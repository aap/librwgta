-- editor_util.lua
-- Shared utilities for all editor scripts (mapedit, modelview, …).
-- include() this file before script-specific code.

-- ---------------------------------------------------------------------------
-- General helpers
-- ---------------------------------------------------------------------------

function copytab(t)
	local t2 = {}
	for k,v in pairs(t) do
		t2[k] = v
	end
	return t2
end

function tV3d(v)
	return rw.V3d(v.x, v.y, v.z)
end

function tQuat(q)
	return rw.Quat(q.w, q.x, q.y, q.z)
end

function clamp(val, min, max)
	if val < min then return min end
	if val > max then return max end
	return val
end

function pairsSorted(t, f)
	local a = {}
	for n in pairs(t) do table.insert(a, n) end
	table.sort(a, f)
	local i = 0
	local iter = function()
		i = i + 1
		if a[i] == nil then return nil
		else return a[i], t[a[i]]
		end
	end
	return iter
end

function printHierarchy(frame, indent)
	indent = indent or 0
	print(string.rep("  ", indent) .. frame:getName() .. "  " .. frame:count())
	for child in frame:children() do
		printHierarchy(child, indent + 1)
	end
end

function printAtomic(a)
	local g = a:getGeometry()
	print("atomic ", a:getFrame():getName())
	print(" geo", g:getNumVertices(), g:getNumTriangles())
	for m in g:materials() do
		local t = m:getTexture()
		local fx = m:getMatFX()
		print("  material", m:getColor(), m:getSurfaceProps())
		if t then
			print("      tex", t:getName(), t:getMaskName())
		end
		if fx then
			print("	matfx", fx:getEnvTexture(), fx:getEnvCoefficient())
		end
	end
end

function printtab(t, indent)
	indent = indent or 2
	local pref = string.rep("  ", indent)
	for k,v in pairsSorted(t)  do
		if type(v)=="table" then
			print(pref .. k)
			printtab(v, indent+1)
		else
			print(pref .. k,v)
		end
	end
end

-- ---------------------------------------------------------------------------
-- Undo/redo history
-- ---------------------------------------------------------------------------

History = {}
History.undoStack = {}
History.redoStack = {}
History.maxDepth = 200

function History:push(action)
	table.insert(self.undoStack, action)
	table.redoStack = {}
	if #self.undoStack > self.maxDepth then
		table.remove(self.undoStack, 1)
	end
	action:redo()
end

function History:PushSetValue(tab, field, before, after)
	local undo = function(self) tab[field] = before; end
	local redo = function(self) tab[field] = after; end
	self:push({undo=undo, redo=redo})
end

function History:PushFlagSet(flags, bit)
	local undo = function(self) flags.bits = flags.bits & ~bit; end
	local redo = function(self) flags.bits = flags.bits | bit; end
	self:push({undo=undo, redo=redo})
end

function History:PushFlagClear(flags, bit)
	local undo = function(self) flags.bits = flags.bits | bit; end
	local redo = function(self) flags.bits = flags.bits & ~bit; end
	self:push({undo=undo, redo=redo})
end

function History:undo()
	local a = table.remove(self.undoStack)
	if a then
		a:undo()
		table.insert(self.redoStack, a)
	end
end

function History:redo()
	local a = table.remove(self.redoStack)
	if a then
		a:redo()
		table.insert(self.undoStack, a)
	end
end

-- ---------------------------------------------------------------------------
-- Mouse / keyboard input
-- ---------------------------------------------------------------------------

sk.keysdown = {}

sk.LMB = 1
sk.MMB = 2
sk.RMB = 4

sk.clickstate = 0
sk.clickx = 0
sk.clicky = 0
sk.clickbtn = 0

function sk.isMouseClicked(btn)
	return sk.clickstate == 2 and sk.clickbtn == btn
end

function sk.mouseNotMoved()
	local dx = sk.mouse.x - sk.clickx
	local dy = sk.mouse.y - sk.clicky
	return dx*dx + dy*dy < 3*3
end

function sk.updateMouse()
	sk.prevmouse = sk.mouse
	sk.mouse = copytab(sk.curmouse)
	sk.curmouse.dwheel = 0

	if sk.clickstate == 2 then sk.clickstate = 0 end
	if sk.clickstate == 0 and (sk.mouse.btn & (sk.mouse.btn ~ sk.prevmouse.btn)) ~= 0 then
		sk.clickstate = 1
		sk.clickx = sk.mouse.x
		sk.clicky = sk.mouse.y
	elseif sk.clickstate == 1 and sk.mouse.btn == 0 then
		if sk.mouseNotMoved() then
			sk.clickstate = 2
			sk.clickbtn = sk.prevmouse.btn & ~(sk.prevmouse.btn-1)
		else
			sk.clickstate = 0
		end
	end
end

function IsCtrlDown()
	return sk.keysdown[sk.KEY_CAPSLK] or sk.keysdown[sk.KEY_LCTRL] or sk.keysdown[sk.KEY_RCTRL]
end

function IsShiftDown()
	return sk.keysdown[sk.KEY_LSHIFT] or sk.keysdown[sk.KEY_RSHIFT]
end

function IsAltDown()
	return sk.keysdown[sk.KEY_LALT] or sk.keysdown[sk.KEY_RALT]
end

function KeyUp(k)
	sk.keysdown[k] = false
end

function KeyDown(k)
	sk.keysdown[k] = true

	local ctrl = IsCtrlDown()
	if k == sk.KEY_G then
		gizmo.op = gizmo.TRANSLATE
	elseif k == sk.KEY_R then
		if ctrl then
			History:redo()
		else
			gizmo.op = gizmo.ROTATE
		end
	elseif k == sk.KEY_Z then
		if ctrl then
			History:undo()
		end
	end
end

sk.curmouse = { x = 0, y = 0, btn = 0, dwheel = 0 }
sk.mouse = copytab(sk.curmouse)
sk.prevmouse = copytab(sk.curmouse)

function MouseBtn(btn)
	sk.curmouse.btn = btn
end
function MouseMotion(x, y)
	sk.curmouse.x = x
	sk.curmouse.y = y
end
function MouseWheel(delta)
	sk.curmouse.dwheel = sk.curmouse.dwheel + delta
end

-- ---------------------------------------------------------------------------
-- Gizmo (ImGuizmo) — shared state and per-frame processing
-- ---------------------------------------------------------------------------
-- Requires C++-side gizmo table to be populated before editor_util is loaded.

gizmo.op       = gizmo.TRANSLATE
gizmo.mode     = gizmo.WORLD
gizmo.wasUsing = false
gizmo.stepTrans = 1
gizmo.stepRot   = 5
gizmo.snapTrans = true
gizmo.snapRot   = true

function gizmo.Process()
	local step
	if gizmo.op == gizmo.ROTATE then
		step = gizmo.snapRot   and gizmo.stepRot   or 0
	else
		step = gizmo.snapTrans and gizmo.stepTrans or 0
	end
	gizmo.Use(gizmo.op, gizmo.mode, step)

	local using = gizmo.IsUsing()
	if selection and (using or gizmo.wasUsing) then
		local mt = getmetatable(selection)
		if mt and mt.gizmo then
			local pos, rot = gizmo.GetXform()
			if not gizmo.wasUsing then
				selection:gizmo(0, pos, rot)
			end
			if using then
				selection:gizmo(1, pos, rot)
			else
				selection:gizmo(2, pos, rot)
			end
		end
	end
	gizmo.wasUsing = using
end

-- ---------------------------------------------------------------------------
-- prop.* — Maya-style two-column property sheet
-- ---------------------------------------------------------------------------
-- Usage:
--   prop.begin("unique_id")
--   prop.header("Section") / prop.label / prop.dragFloat / prop.color / …
--   prop.finish()
--
-- prop.flags name must be unique within the window (used as ImGui ID).

prop = {}
prop.labelWidth = 120

function prop.begin(id)
	ImGui.PushStyleVar(ImGuiStyleVar.ItemSpacing, 4, 2)
	ImGui.Columns(2, id, false)
	ImGui.SetColumnWidth(0, prop.labelWidth)
end

function prop.finish()
	ImGui.Columns(1)
	ImGui.PopStyleVar()
end

function prop.header(title)
	ImGui.Columns(1)
	ImGui.Spacing()
	ImGui.PushStyleColor(ImGuiCol.Text, 0.55, 0.55, 0.55, 1.0)
	ImGui.Separator()
	ImGui.TextUnformatted(title)
	ImGui.PopStyleColor()
	ImGui.Columns(2, "prop_"..title, false)
	ImGui.SetColumnWidth(0, prop.labelWidth)
end

local function propLabel(name)
	ImGui.AlignTextToFramePadding()
	ImGui.TextUnformatted(name)
	ImGui.NextColumn()
	ImGui.SetNextItemWidth(-1)
end

local function propNext()
	ImGui.NextColumn()
end

function prop.label(name, value)
	propLabel(name)
	ImGui.AlignTextToFramePadding()
	ImGui.TextUnformatted(tostring(value))
	propNext()
end

function prop.dragFloat(name, v, speed, lo, hi, fmt)
	propLabel(name)
	local v2, changed = ImGui.DragFloat("##"..name, v, speed or 0.005, lo or 0.0, hi or 1.0, fmt or "%.3f")
	propNext()
	return v2, changed
end

function prop.dragFloat3(name, t, speed, lo, hi, fmt)
	propLabel(name)
	local t2, changed = ImGui.DragFloat3("##"..name, t, speed or 0.005, lo or 0.0, hi or 1.0, fmt or "%.3f")
	propNext()
	return t2, changed
end

function prop.color(name, rgba)
	propLabel(name)
	local col, changed = ImGui.ColorEdit4("##"..name,
		{ rgba.r/255, rgba.g/255, rgba.b/255, rgba.a/255 })
	propNext()
	if changed then
		return rw.RGBA(
			math.floor(col[1]*255 + 0.5),
			math.floor(col[2]*255 + 0.5),
			math.floor(col[3]*255 + 0.5),
			math.floor(col[4]*255 + 0.5)), true
	end
	return rgba, false
end

function prop.check(name, v)
	propLabel(name)
	local v2, changed = ImGui.Checkbox("##"..name, v)
	propNext()
	return v2, changed
end

-- Dropdown. items = {"Label1","Label2",...} (1-based). current = 0-based ImGui index.
-- Returns new index (0-based), changed.
function prop.combo(name, current, items)
	propLabel(name)
	local v2, changed = ImGui.Combo("##"..name, current, items, #items)
	propNext()
	return v2, changed
end

-- Bit-flag rows, collapseable.
--   bitdefs      = { [bit]=name, ... }
--   setter(nv)   optional — if nil, all bits are read-only
--   writableMask optional — bitmask of which bits get a checkbox; rest shown as text
--                           (only meaningful when setter is provided)
function prop.flags(name, value, bitdefs, setter, writableMask)
	propLabel(name)
	ImGui.AlignTextToFramePadding()
	local open = ImGui.TreeNode("##pflags_"..name, string.format("0x%X", value))
	propNext()
	if open then
		for i = 0, 31 do
			local bit = 1 << i
			if bitdefs[bit] then
				propLabel("")
				local canEdit = setter and (writableMask == nil or (writableMask & bit) ~= 0)
				if canEdit then
					local v, changed = ImGui.Checkbox("##pf_"..name.."_"..i, (value & bit) ~= 0)
					if changed then setter(v and (value | bit) or (value & ~bit)) end
					ImGui.SameLine()
					ImGui.AlignTextToFramePadding()
					ImGui.TextUnformatted(bitdefs[bit])
				else
					ImGui.AlignTextToFramePadding()
					if (value & bit) ~= 0 then
						ImGui.TextUnformatted(bitdefs[bit])
					else
						ImGui.TextDisabled(bitdefs[bit])
					end
				end
				propNext()
			end
		end
		ImGui.Columns(1)
		ImGui.TreePop()
		ImGui.Columns(2, "prop_flags_"..name, false)
		ImGui.SetColumnWidth(0, prop.labelWidth)
	end
end

-- Texture slot: name in the right column, 64px thumbnail on hover tooltip.
-- setter stub present for future picker; clicking does nothing yet.
function prop.texture(name, tex, setter)
	propLabel(name)
	if tex then
		local label = tex:getName()
		if setter then
			ImGui.Button(label.."##tbtn_"..name)
		else
			ImGui.AlignTextToFramePadding()
			ImGui.TextUnformatted(label)
		end
		if ImGui.IsItemHovered() and tex.raster then
			local ras = tex.raster
			local w, h = ras:width(), ras:height()
			if w > 0 and h > 0 then
				ImGui.BeginTooltip()
				rw.imageTexture(tex, 64, 64 * h / w)
				ImGui.EndTooltip()
			end
		end
	else
		ImGui.AlignTextToFramePadding()
		ImGui.TextDisabled("(none)")
	end
	propNext()
end
