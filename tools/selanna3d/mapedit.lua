include('gta.lua')
include('gtadraw.lua')
include('console.lua')

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

function LoadIII()
	local g = gta.make(gta.GameIII, "/u/aap/gta/gta3_re")
	g:AddCdImage("models/gta3.img")
	g:ReadDataFile("data/default.dat")
	g:ReadDataFile("data/gta3.dat")
	g:FinishLoading()

	g:InstantiateScene("industsw")
	g:InstantiateScene("industse")
	g:InstantiateScene("industnw")
	g:InstantiateScene("industne")
	return g
end

function LoadVC()
	local g = gta.make(gta.GameVC, "/u/aap/gta/gta_miami")
	g:AddCdImage("models/gta3.img")
	g:ReadDataFile("data/default.dat")
	g:ReadDataFile("data/gta_vc.dat")
	g:FinishLoading()

	g:InstantiateScene("oceandn")
	g:InstantiateScene("oceandrv")
	return g
end

function LoadLCS()
	local g = gta.make(gta.GameVC, "/u/aap/other/gta/gta_lcs")
	g:AddCdImage("models/gta3.img")
	g:ReadDataFile("data/gta_lcs.dat")
	g:FinishLoading()

	g:InstantiateScene("indust")
	return g
end

function LoadVCS()
	local g = gta.make(gta.GameVC, "/u/aap/gta/vcs_map")
	g:AddCdImage("models/gta3.img")
	g:ReadDataFile("data/gta_vcs.dat")
	g:FinishLoading()

	g:InstantiateScene("oceandn")
	g:InstantiateScene("oceandrv")
	return g
end

function LoadSA()
	local g = gta.make(gta.GameSA, "/u/aap/other/gta/gtasa")
	g.dayNightBalance = 0
	g.wetRoads = 0
	g:AddCdImage("models/gta3.img")
	g:AddCdImage("models/gta_int.img")
	g:ReadDataFile("data/default.dat")
	g:ReadDataFile("data/gta_nopaths.dat")
--	g:ReadDataFile("data/gta.dat")
	g:FinishLoading()

	g:LoadStreamedIPLs()
	g:InstantiateScene("lae", true)

	return g
end

function copyGame(g, dir)
	g:SwitchGameDir(dir)
	g:PopulateGameDir()
	for _, f in pairs(g.files) do
		if f.type == 'IDE' then
			g:WriteIDE(f)
		elseif f.type == 'IPL' then
			g:WriteIPL(f)
		elseif f.type == 'MAPZONE' then
			g:WriteZON(f)
		else
			print("not copying", f.type, f.fileName)
		end
	end
end


function hideDamagedLOD(a)
	local name = a:getFrame():getName()
	if string.find(name, "_dam") or
	   string.find(name, "_vlo") or
	   string.find(name, "_lo_") then
		a:setVisibility(false)
	end
end

clump = nil
game = nil
sceneCam = nil
modelCam = nil
activeCam = nil
rwCamera = nil
-- should go into camera maybe?
camspeed = 0
camspeedSide = 0

clearCol = rw.RGBA(0x80, 0x80, 0x80, 0xFF)

function Init()
	world = rw.WorldCreate(nil)

	local ambient = rw.LightCreate(rw.Light_AMBIENT)
	ambient:setColor(0.2, 0.2, 0.2)
	world:addLight(ambient)

	local xaxis = rw.V3d(1, 0, 0)
	local direct = rw.LightCreate(rw.Light_DIRECTIONAL)
	direct:setColor(0.8, 0.8, 0.8)
	direct:setFrame(rw.FrameCreate())
	direct:getFrame():rotate(xaxis, 180, rw.COMBINEREPLACE)
	world:addLight(direct)

	rwCamera = sk.CameraCreate(gWidth, gHeight, 1)
	world:addCamera(rwCamera)

	sceneCam = Camera.new()
	local activeCam = sceneCam
	sceneCam.rwCamera = rwCamera
	sceneCam.aspectRatio = gWidth/gHeight
	sceneCam.fov = 80
	sceneCam.near = 0.1
	sceneCam.far = 2000
	sceneCam.position = rw.V3d(679, -1006, 96)
	sceneCam.target = rw.V3d(1700, -455, -274)
	sceneCam:setDistance(30)
	sceneCam:update()

	modelCam = Camera.new()
	modelCam.rwCamera = rwCamera
	modelCam.aspectRatio = gWidth/gHeight
	modelCam.fov = 80
	modelCam.near = 0.1
	modelCam.far = 1000
	modelCam.position = rw.V3d(20, 20, 20)
	modelCam.target = rw.V3d(0, 0, 0)
	modelCam:update()

print("done")
end

function clamp(val, min, max)
	if val < min then return min end
	if val > max then return max end
	return val
end

function processCam(cam, timestep)
	timestep = timestep*30

	local shift = IsShiftDown()
	local alt = IsAltDown()
	local dx = (sk.mouse.x - sk.prevmouse.x)/gWidth
	local dy = (sk.mouse.y - sk.prevmouse.y)/gHeight

	local s = 4.5
	if (sk.mouse.btn & 1) ~= 0 then
		cam:turn(-dx*s, -dy*s)
	elseif (sk.mouse.btn & 2) ~= 0 then
		if alt then
			cam:orbit(-dx*s, dy*s)
		else
			local d = cam:distanceTo(cam.target)/5
			cam:pan(-dx*s*d, dy*s*d)
		end
	elseif (sk.mouse.btn & 4) ~= 0 then
		cam:zoom(-dy*s*10)
	end

	cam:zoom(sk.mouse.dwheel*2)

	s = timestep
	if shift then s = s*2 end

	if sk.keysdown[sk.KEY_W] then
		camspeed = camspeed + 0.1
	elseif sk.keysdown[sk.KEY_S] then
		camspeed = camspeed - 0.1
	else
		camspeed = 0
	end
	camspeed = clamp(camspeed, -70, 70)
	cam:dolly(camspeed*s)

	if sk.keysdown[sk.KEY_A] then
		camspeedSide = camspeedSide - 0.1
	elseif sk.keysdown[sk.KEY_D] then
		camspeedSide = camspeedSide + 0.1
	else
		camspeedSide = 0
	end
	camspeedSide = clamp(camspeedSide, -70, 70)
	cam:pan(camspeedSide*s, 0)

	if sk.keysdown[sk.KEY_J] then
		if selection and selection.position then
			activeCam:jumpTo(tV3d(selection.position))
		end
	end
end

function Building:imguiTitle()
	return true, tostring(self.id) .. "\t" .. self.modelName
end

function Building:imguiDraw()
	if not ImGui.CollapsingHeader("Model", ImGuiTreeNodeFlags.DefaultOpen) then return end

	ImGui.PushID(tostring(self))
	guiTab(self.sourceFile, "File " .. self.sourceFile.fileName)
	local value, used = ImGui.InputInt("Id", self.id)
	local text, selected = ImGui.InputText("Model", self.modelName, 24)
	local text, selected = ImGui.InputText("Txd", self.txdName, 24)
	for i = 1, self.numAtomics do
		local value, used = ImGui.InputFloat("LOD dist"..i, self["lodDist"..i])
	end
	if self.timeOn then
		local value, used = ImGui.InputInt("Time On", self.timeOn)
		local value, used = ImGui.InputInt("Time Off", self.timeOff)
	end
	ImGui.Flags("Flags##Object", self.flags)

	if ImGui.Button("View") then
		game:LoadAtomics(self)
		modelCam:jumpTo(rw.V3d(0,0,0))
		clump = self.rwClump
	end
	ImGui.PopID()
end

function Instance:select()
	selection = self
	gizmo.Init(tV3d(self.position), tQuat(self.rotation):conj())
end

function Instance:setXform(pos, rot)
	self.position = pos
	self.rotation = rot
	self:UpdateRW()
	if self == selection then
		gizmo.Init(tV3d(self.position), tQuat(self.rotation):conj())
	end
end

function Instance:gizmo(phase, pos, rot)
	if phase == 0 then
		undoStart = {pos, rot:conj()}
	elseif phase == 2 then
		local posBefore = undoStart[1]
		local rotBefore = undoStart[2]
		local posAfter = pos
		local rotAfter = rot:conj()
		local inst = self
		local undo = function(self) inst:setXform(posBefore, rotBefore) end
		local redo = function(self) inst:setXform(posAfter, rotAfter) end
		History:push({undo=undo, redo=redo})
	end
	self:setXform(pos, rot:conj())
end

function Instance:imguiTitle()
	return self.show and self.sourceFile.scene.showScene,
		tostring(self.id) .. "\t" .. self.name
end

function Instance:imguiDraw()
	ImGui.PushID(tostring(self))
	if ImGui.CollapsingHeader("Instance", ImGuiTreeNodeFlags.DefaultOpen) then 
		guiTab(self.sourceFile, "File " .. self.sourceFile.fileName)
		local value, used = ImGui.InputInt("Id", self.id)
		local text, selected = ImGui.InputText("Name", self.name, 24)
		local p = self.position
		local values, used = ImGui.InputFloat3("Position", {p.x, p.y, p.z})
		if self.scale then
			local s = self.scale
			local values, used = ImGui.InputFloat3("Scale", {s.x, s.y, s.z})
		end
		local q = self.rotation
		local values, used = ImGui.InputFloat4("Rotation", {q.x, q.y, q.z, q.w})
		if self.area then
			local value, used = ImGui.InputInt("Area", self.area)
		end
		if self.flags then
			ImGui.Flags("Flags##Instance", self.flags)
		end
		if ImGui.Button("jump to") then
			local p = tV3d(self.position)
			activeCam:jumpTo(p)
		end
		ImGui.Dummy(0, 20)
	end
	ImGui.PopID()

	if self.mdl then
		self.mdl:imguiDraw()
	end
end

function Scene:imguiTitle()
	return true, self.name
end

function guiTabRaw(t, label)
	label = label or "table"
	if ImGui.TreeNode(label) then
		for k, v in pairsSorted(t) do
			local key = tostring(k)
			ImGui.PushID(key)
			if type(v) == "table" then
				guiTabRaw(v, key)
			else
				ImGui.LabelText(key, tostring(v))
			end
			ImGui.PopID()
		end
		ImGui.TreePop()
	end
end

function guiTab(t, label, cullhidden)
	label = label or "table"
	local mt = getmetatable(t)
	if mt and mt.imguiTitle then
		local show
		show, label = t:imguiTitle()
		if cullhidden and not show then
			return
		end
	end
	if ImGui.TreeNode(label) then
		if mt and mt.imguiDraw then
			t:imguiDraw()
		else
			for k, v in pairsSorted(t) do
				local key = tostring(k)
				ImGui.PushID(key)
				if type(v) == "table" then
					guiTab(v, key, cullhidden)
				else
					ImGui.LabelText(key, tostring(v))
				end
				ImGui.PopID()
			end
		end
		ImGui.TreePop()
	end
end

function Scene:imguiDraw()
	ImGui.PushID(self.name)

	local show, clicked = ImGui.Checkbox("##visible", self.showScene or false)
	if clicked then
		if show then
			game:InstantiateScene(self, true)
		else
			game:HideScene(self, true)
		end
	end
	if #self.streamed > 0 then
		ImGui.SameLine()
		show, clicked = ImGui.Checkbox("##onevisible", self.showScene or false)
		if clicked then
			if show then
				game:InstantiateScene(self, false)
			else
				game:HideScene(self, false)
			end
		end
	end
	ImGui.SameLine()
	if ImGui.TreeNode(self.name) then
		ImGui.Text(self.file.fileName)
		for _, scn in ipairs(self.streamed) do
			scn:imguiDraw()
		end
		guiList(self.instances, "Instances")
		guiList(self.instances, "Visible Instances", true)
		if self.carGenerators then
			guiList(self.carGenerators, "Car Generators")
		end

		ImGui.TreePop()
	end
	ImGui.PopID()
end

function guiScenes(g)
	if not ImGui.TreeNode("Scenes") then return end
	for _, scene in ipairs(g.scenes) do
		scene:imguiDraw()
	end
	ImGui.TreePop()
end

function gizmoUpdate(pos, rot)
	print(pos, rot)
end

selection = nil
hovered = nil
drawColorCoded = false
gizmo.op = gizmo.TRANSLATE
gizmo.mode = gizmo.WORLD
gizmo.wasUsing = false
gizmo.stepTrans = 1
gizmo.stepRot = 5
gizmo.snapRot = true
gizmo.snapTrans = true

viewer = { lodMode = 1, lodMult = 1.5 }

function gizmo.Process()
	local step
	if gizmo.op == gizmo.ROTATE then
		step = gizmo.stepRot
		if not gizmo.snapRot then
			step = 0
		end
	else
		step = gizmo.stepTrans
		if not gizmo.snapTrans then
			step = 0
		end
	end
	gizmo.Use(gizmo.op, gizmo.mode, step)

	local using = gizmo.IsUsing()
	if selection and (using or gizmo.wasUsing) then
		if not gizmo.wasUsing then
			local pos, rot = gizmo.GetXform()
			selection:gizmo(0, pos, rot)
		end
		local mt = getmetatable(selection)
		if mt and mt.gizmo then
			local pos, rot = gizmo.GetXform()
			if using then
				selection:gizmo(1, pos, rot)
			else
				selection:gizmo(2, pos, rot)
			end
		end
	end
	gizmo.wasUsing = using
end

local ListWindows = {}
function NewListWindow(item, title)
	ListWindows[item] = title
end
local ItemWindows = {}
function NewItemWindow(item, title)
	ItemWindows[item] = title
end

function guiWindows()
	for item, title in pairs(ItemWindows) do
		if ImGui.Begin(title, true) then
			guiItem(item, title)
		else
			ItemWindows[item] = nil
		end
		ImGui.End()
	end
	for item, title in pairs(ListWindows) do
		if ImGui.Begin(title, true) then
			ImGui.SetNextItemOpen(true)
			guiList(item, title)
		else
			ListWindows[item] = nil
		end
		ImGui.End()
	end
end

function guiList(list, label, cullhidden)
	local open = ImGui.TreeNode(label)
	if ImGui.IsItemHovered() then
		if ImGui.IsMouseClicked(2) then
			NewListWindow(list, label)
		end
	end
	if not open then return end
	for _, v in ipairs(list) do
		local mt = getmetatable(v)
		local show = true
		local title = tostring(v)
		if mt and mt.imguiTitle then
			show, title = v:imguiTitle()
		end
		if show or not cullhidden then
			ImGui.PushID(tostring(v))
			if ImGui.Selectable(title, v == selection) and v ~= selection then
				local mt = getmetatable(v)
				if mt and mt.select then
					v:select()
				else
					selection = v
				end
			end
			if ImGui.IsItemHovered() then
				if ImGui.IsMouseClicked(2) then
					NewItemWindow(v, title)
				end
				hovered = v
			end
			ImGui.PopID()
		end
	end
	ImGui.TreePop()
end

function guiCamera(cam)
	local p = cam.position
	local xyz, used = ImGui.InputFloat3("Position", {p.x, p.y, p.z})
	cam.position = rw.V3d(xyz[1], xyz[2], xyz[3])

	p = cam.target
	xyz, used = ImGui.InputFloat3("Target", {p.x, p.y, p.z})
	cam.target = rw.V3d(xyz[1], xyz[2], xyz[3])

	cam.fov, _ = ImGui.DragFloat("FOV", cam.fov, 0.1, 10, 130)
end

local undoStart;

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

function ImGui.DragFloatX(title, tab, field, step, lo, hi)
	local used
	tab[field], used = ImGui.DragFloat(title, tab[field], step, lo, hi)
	if ImGui.IsItemActivated() then
		undoStart = tab[field]
	end
	if ImGui.IsItemDeactivatedAfterEdit() then
		History:PushSetValue(tab, field, undoStart, tab[field])
	end
end

function ImGui.Flags(title, flags)
--	ImGui.SetNextItemStorageID(ImGui.GetID(title))	 --don't have it here
	local open = ImGui.TreeNode(title)
	ImGui.SameLine()
	ImGui.Text(string.format("%x", flags.bits))
	if open then
		for i=0,31 do
			local bit = 1<<i
			if flags.desc[bit] then
				local val, changed = ImGui.Checkbox(flags.desc[bit], (flags.bits & bit) ~= 0)
				if changed then
					if val then
						History:PushFlagSet(flags, bit)
					else
						History:PushFlagClear(flags, bit)
					end
				end
			end
		end
		ImGui.TreePop()
	end
end

function guiRendering(g)
	if ImGui.RadioButton("Draw HD", viewer.lodMode == 1) then viewer.lodMode = 1 end
	ImGui.SameLine()
	if ImGui.RadioButton("Draw LOD", viewer.lodMode == 2) then viewer.lodMode = 2 end
	ImGui.SameLine()
	if ImGui.RadioButton("Draw Normal", viewer.lodMode == 3) then viewer.lodMode = 3 end
	ImGui.DragFloatX("LOD multiplier", viewer, "lodMult", 0.05, 0.5, 10.0)

	if g then
		g.hour, _ = ImGui.InputInt("Hour", g.hour)
		g.hour = g.hour % 24

		if g.dayNightBalance then
			g.dayNightBalance, _ = ImGui.DragFloat("Day/Night",
					g.dayNightBalance, 0.005, 0, 1)
		end
		if g.wetRoads then
			g.wetRoads, _ = ImGui.DragFloat("Wet Roads",
					g.wetRoads, 0.005, 0, 1)
		end
	end
end

function guiItem(item, title)
	if item then
		local mt = getmetatable(item)
		if mt and mt.imguiDraw then
			item:imguiDraw()
		else
			guiTab(item, title)
		end
	end
end

test3 = "/u/aap/gta/test3"
testvc = "/u/aap/gta/testvc"
testsa = "/u/aap/gta/testsa"

function gui()
	guiWindows()

	ImGui.Begin("Game")

	if ImGui.Button("III") then
		if not iii then
			iii = LoadIII()
		end
		game = iii
	end
	ImGui.SameLine()
	if ImGui.Button("VC") then
		if not vc then
			vc = LoadVC()
		end
		game = vc
	end
	ImGui.SameLine()
	if ImGui.Button("LCS") then
		if not lcs then
			lcs = LoadLCS()
		end
		game = lcs
	end
	ImGui.SameLine()
	if ImGui.Button("VCS") then
		if not vcs then
			vcs = LoadVCS()
		end
		game = vcs
	end
	ImGui.SameLine()
	if ImGui.Button("SA") then
		if not sa then
			sa = LoadSA()
		end
		game = sa
	end


	if clump then
		ImGui.SameLine()
		if ImGui.Button("Back to Map") then
			clump = nil
		end
	end

	local w, h = ImGui.GetContentRegionAvail()
	ImGui.BeginChild("left", w/2, 0, ImGuiChildFlags.ResizeX | ImGuiChildFlags.Borders, 0)
	if game then
		guiTabRaw(game, "GTA")
		guiList(game.filesOrdered, "Files")
		guiScenes(game)
		ImGui.Separator()
		guiList(game.buildings, "Simple Models")
		guiList(game.timedBuildings, "Timed Buildings")
		ImGui.Separator()
		guiList(game.instances, "Instances")
		guiList(game.instances, "Visible Instances", true)
	end
	ImGui.EndChild()

	ImGui.SameLine()

	ImGui.BeginChild("right", 0, 0, true)
	if ImGui.BeginTabBar("right_tab", 0) then
		if ImGui.BeginTabItem("Selection") then
			guiItem(selection, "selection")
			ImGui.EndTabItem()
		end
		if ImGui.BeginTabItem("Editor") then
			if ImGui.CollapsingHeader("Transformation", ImGuiTreeNodeFlags.DefaultOpen) then
				if ImGui.RadioButton("Local", gizmo.mode == gizmo.LOCAL) then
					gizmo.mode = gizmo.LOCAL
				end
				ImGui.SameLine()
				if ImGui.RadioButton("World", gizmo.mode == gizmo.WORLD) then
					gizmo.mode = gizmo.WORLD
				end
				gizmo.stepTrans, _ = ImGui.DragFloat("##TransStep",
					gizmo.stepTrans, 0.5, 0.0, 1000000)
				ImGui.SameLine()
				gizmo.snapTrans, _ = ImGui.Checkbox("Translation Snap", gizmo.snapTrans)
				gizmo.stepRot, _ = ImGui.DragFloat("##RotStep",
					gizmo.stepRot, 0.5, 0.0, 1000000)
				ImGui.SameLine()
				gizmo.snapRot, _ = ImGui.Checkbox("Rotation Snap", gizmo.snapRot)
			end
			ImGui.EndTabItem()
		end
		if ImGui.BeginTabItem("View") then
--			ImGui.LabelText("Framerate", tostring(gFramerate))
--			ImGui.LabelText("DeltaTime", tostring(gDeltaTime))
			if ImGui.CollapsingHeader("Rendering", ImGuiTreeNodeFlags.DefaultOpen) then
				guiRendering(game)
			end
			if ImGui.CollapsingHeader("Camera", ImGuiTreeNodeFlags.DefaultOpen) then
				guiCamera(activeCam)
			end
			ImGui.EndTabItem()
		end
		if ImGui.BeginTabItem("Console") then
			con.gui()
			ImGui.EndTabItem()
		end
		ImGui.EndTabBar()
	end
	ImGui.EndChild()


	ImGui.End()
end

red = rw.RGBA(255, 0, 0, 255)
green = rw.RGBA(0, 255, 0, 255)
blue = rw.RGBA(0, 0, 255, 255)
cyan = rw.RGBA(0, 255, 255, 255)
magenta = rw.RGBA(255, 0, 255, 255)
yellow = rw.RGBA(255, 255, 0, 255)
black = rw.RGBA(0, 0, 0, 255)

function setColor(obj)
	if obj == hovered then
		gta.SetHighlightColour(blue)
	elseif obj == selection then
		gta.SetHighlightColour(red)
	else
		gta.SetHighlightColour(black)
	end

	if obj.colorId then
		gta.SetColourCode(obj.colorId)
	end
end

function gta:RenderScene(scene, drawLast)
	if not scene.showScene then return drawLast end
	for _, inst in pairs(scene.instances) do
		inst.colorId = inst.instId
		setColor(inst)
		local a = self:DrawInstance(inst)
		if a then
			table.insert(drawLast, inst)
		end
	end
	return drawLast
end

function gta:RenderMap()
	if drawColorCoded then
		gta.SetRenderColourCoded(1)
	end
	local drawLast = {}
	for _, scene in ipairs(self.scenes) do
		drawLast = self:RenderScene(scene, drawLast)
		for _, scn in ipairs(scene.streamed) do
			drawLast = self:RenderScene(scn, drawLast)
		end
	end
	for _, inst in pairs(drawLast) do
		setColor(inst)
		inst.rwAtomic:render()
	end
	gta.SetRenderColourCoded(0)
end

function Draw(timestep)
	sk.updateMouse()

	if clump then
		activeCam = modelCam
	else
		activeCam = sceneCam
	end
	processCam(activeCam, timestep)
	activeCam:update()
	rwCamera:beginUpdate()

	if game and not clump and sk.isMouseClicked(sk.LMB) then
		rwCamera:clear(black, rw.Camera_CLEARIMAGE|rw.Camera_CLEARZ)
		drawColorCoded = true
		game:RenderMap()
		drawColorCoded = false
		local code = gta.GetColourCode(sk.curmouse.x, sk.curmouse.y)
		local inst = game.instances[code]
		if inst then inst:select()
		else selection = nil
		end
	end
	if sk.isMouseClicked(sk.RMB) then
		selection = nil
	end

	rwCamera:clear(clearCol, rw.Camera_CLEARIMAGE|rw.Camera_CLEARZ)

	sk.ImGuiBeginFrame(timestep)

	hovered = nil
	gui()
	gizmo.Process()

	if clump then
		clump:render()
	elseif game then
		if game.dayNightBalance then gta.SetDayNightBalance(game.dayNightBalance) end
		if game.wetRoads then gta.SetWetRoads(game.wetRoads) end
		game:RenderMap()
	end

	sk.ImGuiEndFrame()

	rwCamera:endUpdate()
	rwCamera:showRaster(1)
end

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

function Resize(w, h)
	sk.CameraSize(rwCamera, w, h)
	sceneCam.aspectRatio = w/h
	modelCam.aspectRatio = w/h
end
function MouseWheel(delta)
	sk.curmouse.dwheel = sk.curmouse.dwheel + delta
end
