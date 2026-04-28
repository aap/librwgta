-- clumpedit.lua — ClumpEditor: DFF/TXD inspector and frame editor.
-- include() after editor_util.lua.
-- Provides ClumpEditor (inherits Editor) for use by both modelview.lua
-- and gtaedit.lua.

-- ---------------------------------------------------------------------------
-- ClumpEditor
-- ---------------------------------------------------------------------------

ClumpEditor = setmetatable({}, { __index = Editor })
ClumpEditor.__index = ClumpEditor

function ClumpEditor.new()
	local self = Editor.new()
	setmetatable(self, ClumpEditor)
	self.clump            = nil
	self.texdict          = nil
	self.selection        = nil
	self.selectedMaterial = nil
	self.selectedTexture  = nil
	self.atomicToId       = {}
	self.idToAtomic       = {}
	self.onExit           = nil   -- set by host (e.g. gtaedit) to return to map mode
	self.showHierarchy    = true
	self.showTxd          = false
	self.showCarCol       = false
	self.frameXformBefore = nil
	self.frameGizmoBefore = nil
	-- Layout / UI
	self.propW    = 320
	self.statusH  = 100
	self.clearCol  = rw.RGBA(0x80, 0x80, 0x80, 0xFF)
	self.background = rw.RGBA(0xA1, 0xA1, 0xA1, 0xFF)
	-- Gizmo defaults
	self.stepTrans = 0.1
	self.stepRot   = 5
	self.snapTrans = true
	self.snapRot   = true
	return self
end

-- Called after rwCamera and world are ready.
-- Sets up gizmo defaults and the 4-up view layout.
function ClumpEditor:initWorld()
	self.world = rw.WorldCreate(nil)

	local ambient = rw.LightCreate(rw.Light_AMBIENT)
	ambient:setColor(0.2, 0.2, 0.2)
	self.world:addLight(ambient)

	local xax = rw.V3d(1, 0, 0)
	local direct = rw.LightCreate(rw.Light_DIRECTIONAL)
	direct:setColor(0.8, 0.8, 0.8)
	direct:setFrame(rw.FrameCreate())
	direct:getFrame():rotate(xax, 180, rw.COMBINEREPLACE)
	self.world:addLight(direct)

	self.rwCamera = sk.CameraCreate(gWidth, gHeight, 1)

	gizmo.stepTrans = self.stepTrans
	gizmo.stepRot   = self.stepRot
	gizmo.snapTrans = self.snapTrans
	gizmo.snapRot   = self.snapRot
	self:initViews()
	self.viewsReady = true
end

-- ---------------------------------------------------------------------------
-- Load helpers
-- ---------------------------------------------------------------------------

function ClumpEditor:loadTexDict(path)
	local txd = rw.readTexDictFile(path, 0, 1000000)
	if txd then sk.ConvertTexDict(txd) end
	return txd
end

function ClumpEditor:loadClump(path)
	local c = rw.readClumpFile(path, 0, 1000000)
	if c then
		sk.ConvertClump(c)
		for a in c:atomics() do
			hideDamagedLOD(a)
			gta.SetupAtomicPipelines(a)
		end
		self:carColScan(c)
	end
	return c
end

function ClumpEditor:load(filedir, dff, txd)
	local t = self:loadTexDict(rw.makePath(filedir .. "/" .. txd))
	rw.setCurrentTexDict(t)
	local c = self:loadClump(rw.makePath(filedir .. "/" .. dff))
	self.clump   = c
	self.texdict = t
	self.selection = nil
	self.selectedMaterial = nil
	self.selectedTexture  = nil
	self.atomicToId = {}
	self.idToAtomic = {}
end

function ClumpEditor:openClump(c, txd)
	self.clump   = c
	self.texdict = txd
	self.selection = nil
	self.selectedMaterial = nil
	self.selectedTexture  = nil
	self.atomicToId = {}
	self.idToAtomic = {}
	if txd then rw.setCurrentTexDict(txd) end
	self:carColScan(c)
end

-- ---------------------------------------------------------------------------
-- Frame selection + gizmo
-- ---------------------------------------------------------------------------

local frameMeta = {}
frameMeta.__index = frameMeta

function frameMeta:gizmo(phase, pos, rot)
	local f    = self.frame
	local ed   = self.editor
	if phase == 0 then
		gizmo.InitMatrix(f:getLTM())
		ed.frameGizmoBefore = f:copyMatrix()
	else
		local ltmCur = gizmo.GetMatrix()
		local parent = f:getParent()
		local localM = parent and ltmCur * rw.matInvert(parent:getLTM()) or ltmCur
		f:transform(localM, rw.COMBINEREPLACE)
		if phase == 2 then
			local before = ed.frameGizmoBefore
			ed.frameGizmoBefore = nil
			local after = f:copyMatrix()
			ed.history:push({
				undo = function() f:transform(before, rw.COMBINEREPLACE) end,
				redo = function() f:transform(after,  rw.COMBINEREPLACE) end,
			})
		end
	end
end

function ClumpEditor:frameSelect(frame)
	self.selection = setmetatable({ frame = frame, editor = self }, frameMeta)
	gizmo.InitMatrix(frame:getLTM())
end

-- ---------------------------------------------------------------------------
-- Atomic colour-coded picking
-- ---------------------------------------------------------------------------

function ClumpEditor:atomicPickId(a)
	local key = a:ptr()
	local id  = self.atomicToId[key]
	if not id then
		id = #self.idToAtomic + 1
		self.atomicToId[key] = id
		self.idToAtomic[id]  = a
	end
	return id
end

function ClumpEditor:renderCoded(c)
	gta.SetRenderColourCoded(1)
	for a in c:atomics() do
		gta.SetColourCode(self:atomicPickId(a))
		if a:isVisible() then a:render() end
	end
	gta.SetRenderColourCoded(0)
end

-- ---------------------------------------------------------------------------
-- Render
-- ---------------------------------------------------------------------------

function ClumpEditor:renderClump(view, c)
	local selFrame = self.selection and self.selection.frame or nil
	for a in c:atomics() do
		local wirecol = darkblue
		if a:getFrame() == selFrame then wirecol = white end
		if a:isVisible() then
			if view.drawShaded then a:render() end
			if view.drawWire   then gta.renderWireAtomic(a, wirecol) end
		end
	end
end


-- ---------------------------------------------------------------------------
-- Vehicle colour slots
-- ---------------------------------------------------------------------------

local carColSlotDefs = {
	{ name = "Body 1",   key = 0x3cff00 },
	{ name = "Body 2",   key = 0xff00af },
	{ name = "Body 3",   key = 0x00ffff },
	{ name = "Body 4",   key = 0xff00ff },
	{ name = "Light FL", key = 0xffaf00 },
	{ name = "Light FR", key = 0x00ffc8 },
	{ name = "Light RL", key = 0xb9ff00 },
	{ name = "Light RR", key = 0xff3c00 },
}

function ClumpEditor:initCarColSlots()
	self.carColSlots = {}
	self.carColKeyToSlot = {}
	for i, def in ipairs(carColSlotDefs) do
		local s = { name = def.name, key = def.key, mats = {},
		            chosen = { ((def.key>>16)&0xff)/255,
		                       ((def.key>>8) &0xff)/255,
		                        (def.key     &0xff)/255, 1 } }
		self.carColSlots[i]             = s
		self.carColKeyToSlot[def.key]   = i
	end
end

function ClumpEditor:carColScan(c)
	if not self.carColSlots then self:initCarColSlots() end
	for _, s in ipairs(self.carColSlots) do s.mats = {} end
	if not c then return end
	for a in c:atomics() do
		for m in a:getGeometry():materials() do
			local col = m:getColor()
			local key = col.r * 0x10000 + col.g * 0x100 + col.b
			local si  = self.carColKeyToSlot[key]
			if si then
				local s = self.carColSlots[si]
				if #s.mats == 0 then
					s.chosen = { col.r/255, col.g/255, col.b/255, col.a/255 }
				end
				table.insert(s.mats, m)
			end
		end
	end
end

local function carColSetSlot(s, rgba)
	s.chosen = rgba
	local r = math.floor(rgba[1]*255 + 0.5)
	local g = math.floor(rgba[2]*255 + 0.5)
	local b = math.floor(rgba[3]*255 + 0.5)
	local a = math.floor(rgba[4]*255 + 0.5)
	for _, m in ipairs(s.mats) do m:setColor(rw.RGBA(r, g, b, a)) end
end

local function carColReset(slots)
	for _, s in ipairs(slots) do
		local r = (s.key>>16)&0xff
		local g = (s.key>>8) &0xff
		local b =  s.key     &0xff
		carColSetSlot(s, { r/255, g/255, b/255, 1 })
	end
end

-- ---------------------------------------------------------------------------
-- GUI helpers
-- ---------------------------------------------------------------------------

filterNames = {
	[rw.FILTER_NEAREST]          = "Nearest",
	[rw.FILTER_LINEAR]           = "Linear",
	[rw.FILTER_MIPNEAREST]       = "MipNearest",
	[rw.FILTER_MIPLINEAR]        = "MipLinear",
	[rw.FILTER_LINEARMIPNEAREST] = "LinearMipNearest",
	[rw.FILTER_LINEARMIPLINEAR]  = "LinearMipLinear",
}
addressNames = {
	[rw.ADDRESS_WRAP]   = "Wrap",
	[rw.ADDRESS_MIRROR] = "Mirror",
	[rw.ADDRESS_CLAMP]  = "Clamp",
	[rw.ADDRESS_BORDER] = "Border",
}
rasterFormatNames = {
	[rw.RASTER_DEFAULT] = "Default",
	[rw.RASTER_C1555]   = "C1555",
	[rw.RASTER_C565]    = "C565",
	[rw.RASTER_C4444]   = "C4444",
	[rw.RASTER_LUM8]    = "LUM8",
	[rw.RASTER_C8888]   = "C8888",
	[rw.RASTER_C888]    = "C888",
	[rw.RASTER_D16]     = "D16",
	[rw.RASTER_D24]     = "D24",
	[rw.RASTER_D32]     = "D32",
	[rw.RASTER_C555]    = "C555",
}

function rasterFormatStr(ras)
	local fmt = ras:format()
	local s = rasterFormatNames[fmt & 0xFF00] or string.format("0x%X", fmt & 0xFF00)
	if (fmt & rw.RASTER_PAL8)      ~= 0 then s = s .. " PAL8"    end
	if (fmt & rw.RASTER_PAL4)      ~= 0 then s = s .. " PAL4"    end
	if (fmt & rw.RASTER_MIPMAP)    ~= 0 then s = s .. " MIP"     end
	if (fmt & rw.RASTER_AUTOMIPMAP)~= 0 then s = s .. " AUTOMIP" end
	return s
end

geoFlagBits = {
	[rw.GEO_TRISTRIP]       = "TriStrip",
	[rw.GEO_POSITIONS]      = "Positions",
	[rw.GEO_TEXTURED]       = "Textured",
	[rw.GEO_PRELIT]         = "Prelit",
	[rw.GEO_NORMALS]        = "Normals",
	[rw.GEO_LIGHT]          = "Light",
	[rw.GEO_MODULATE]       = "Modulate",
	[rw.GEO_TEXTURED2]      = "Textured2",
	[rw.GEO_NATIVE]         = "Native",
	[rw.GEO_NATIVEINSTANCE] = "NativeInstance",
}
atomicFlagBits = {
	[rw.ATOMIC_COLLISIONTEST] = "CollisionTest",
	[rw.ATOMIC_RENDER]        = "Render",
}

filterCombo = {
	values = { rw.FILTER_NEAREST, rw.FILTER_LINEAR, rw.FILTER_MIPNEAREST,
	           rw.FILTER_MIPLINEAR, rw.FILTER_LINEARMIPNEAREST, rw.FILTER_LINEARMIPLINEAR },
	labels = { "Nearest", "Linear", "MipNearest", "MipLinear", "LinearMipNearest", "LinearMipLinear" },
}
addressCombo = {
	values = { rw.ADDRESS_WRAP, rw.ADDRESS_MIRROR, rw.ADDRESS_CLAMP, rw.ADDRESS_BORDER },
	labels = { "Wrap", "Mirror", "Clamp", "Border" },
}
function comboFind(c, val)
	for i, v in ipairs(c.values) do if v == val then return i - 1 end end
	return 0
end

-- Compact clickable name in the TXD list; 64px hover preview.
function guiTexture(t, selectedTexture)
	if ImGui.Selectable(t:getName(), t == selectedTexture) then
		selectedTexture = t
	end
	if ImGui.IsItemHovered() and t.raster then
		local ras = t.raster
		local w, h = ras:width(), ras:height()
		if w > 0 and h > 0 then
			ImGui.BeginTooltip()
			rw.imageTexture(t, 64, 64 * h / w)
			ImGui.EndTooltip()
		end
	end
	return selectedTexture
end

local txdZoomIdx    = 3
local txdZoomLevels = { 0.25, 0.50, 0.75, 1.00, 1.25, 1.50, 1.75, 2.00 }
local txdZoomLabels = { "25%", "50%", "75%", "100%", "125%", "150%", "175%", "200%" }

function guiTextureFull(t)
	prop.begin("txd_detail_props")
	local mask = t:getMaskName()
	if mask ~= "" then prop.label("Mask", mask) end
	local fi, changed = prop.combo("Filter",    comboFind(filterCombo,  t:getFilter()),    filterCombo.labels)
	if changed then t:setFilter(filterCombo.values[fi + 1]) end
	local au, changed = prop.combo("Address U", comboFind(addressCombo, t:getAddressU()), addressCombo.labels)
	if changed then t:setAddressU(addressCombo.values[au + 1]) end
	local av, changed = prop.combo("Address V", comboFind(addressCombo, t:getAddressV()), addressCombo.labels)
	if changed then t:setAddressV(addressCombo.values[av + 1]) end
	local ras = t.raster
	if ras then
		prop.label("Size",   string.format("%d x %d", ras:width(), ras:height()))
		prop.label("Depth",  tostring(ras:depth()))
		prop.label("Format", rasterFormatStr(ras))
	end
	local zi, zc = prop.combo("Zoom", txdZoomIdx, txdZoomLabels)
	if zc then txdZoomIdx = zi end
	prop.finish()
	if ras then
		local w, h = ras:width(), ras:height()
		if w > 0 and h > 0 then
			local scale = txdZoomLevels[txdZoomIdx + 1]
			rw.imageTexture(t, w * scale, h * scale)
		end
	end
end

function guiTexDict(txd, selectedTexture)
	local avail = ImGui.GetContentRegionAvail()
	ImGui.BeginChild("txd_list", avail/2, 0, ImGuiChildFlags.ResizeX | ImGuiChildFlags.Borders, 0)
	for t in txd:textures() do
		selectedTexture = guiTexture(t, selectedTexture)
	end
	ImGui.EndChild()
	ImGui.SameLine()
	ImGui.BeginChild("txd_detail", 0, 0, true)
	if selectedTexture then
		ImGui.TextUnformatted(selectedTexture:getName())
		ImGui.Separator()
		ImGui.Spacing()
		guiTextureFull(selectedTexture)
	else
		ImGui.TextDisabled("Select a texture")
	end
	ImGui.EndChild()
	return selectedTexture
end

function guiMatFXProps(m)
	prop.header("MatFX")
	local fxCombo = {
		values = { rw.MATFX_NOTHING, rw.MATFX_BUMPMAP, rw.MATFX_ENVMAP, rw.MATFX_BUMPENVMAP,
		           rw.MATFX_DUAL, rw.MATFX_UVTRANSFORM, rw.MATFX_DUALUVTRANSFORM },
		labels = { "Nothing", "BumpMap", "EnvMap", "BumpMap+EnvMap",
		           "Dual", "UVTransform", "Dual UVTransform" },
	}
	local fx     = m:getMatFX()
	local fxtype = fx and fx:getType() or rw.MATFX_NOTHING
	local fi, changed = prop.combo("Effect", comboFind(fxCombo, fxtype), fxCombo.labels)
	if changed then
		m:setMatFXEffect(fxCombo.values[fi + 1])
		fx     = m:getMatFX()
		fxtype = fx and fx:getType() or rw.MATFX_NOTHING
	end
	if fx and fxtype ~= rw.MATFX_NOTHING then
		local hasBump = fxtype == rw.MATFX_BUMPMAP     or fxtype == rw.MATFX_BUMPENVMAP
		local hasEnv  = fxtype == rw.MATFX_ENVMAP      or fxtype == rw.MATFX_BUMPENVMAP
		local hasDual = fxtype == rw.MATFX_DUAL        or fxtype == rw.MATFX_DUALUVTRANSFORM
		local hasUV   = fxtype == rw.MATFX_UVTRANSFORM or fxtype == rw.MATFX_DUALUVTRANSFORM
		if hasBump then
			prop.texture("Bump Map", fx:getBumpTexture(), function(t) fx:setBumpTexture(t) end)
			local v, c = prop.dragFloat("Bump Coeff", fx:getBumpCoefficient(), 0.005, 0, 1)
			if c then fx:setBumpCoefficient(v) end
		end
		if hasEnv then
			prop.texture("Env Map", fx:getEnvTexture(), function(t) fx:setEnvTexture(t) end)
			local v, c = prop.dragFloat("Env Coeff", fx:getEnvCoefficient(), 0.005, 0, 1)
			if c then fx:setEnvCoefficient(v) end
			local fb, c = prop.check("Env FB Alpha", fx:getEnvFBAlpha())
			if c then fx:setEnvFBAlpha(fb) end
		end
		if hasDual then
			local blendCombo = {
				values = { rw.BLEND_ZERO, rw.BLEND_ONE, rw.BLEND_SRCCOLOR, rw.BLEND_INVSRCCOLOR,
				           rw.BLEND_SRCALPHA, rw.BLEND_INVSRCALPHA, rw.BLEND_DESTALPHA,
				           rw.BLEND_INVDESTALPHA, rw.BLEND_DESTCOLOR, rw.BLEND_INVDESTCOLOR,
				           rw.BLEND_SRCALPHASAT },
				labels = { "Zero", "One", "SrcColor", "InvSrcColor",
				           "SrcAlpha", "InvSrcAlpha", "DestAlpha",
				           "InvDestAlpha", "DestColor", "InvDestColor", "SrcAlphaSat" },
			}
			prop.texture("Dual Map", fx:getDualTexture(), function(t) fx:setDualTexture(t) end)
			local si, c = prop.combo("Src Blend",  comboFind(blendCombo, fx:getDualSrcBlend()),  blendCombo.labels)
			if c then fx:setDualSrcBlend(blendCombo.values[si + 1]) end
			local di, c = prop.combo("Dest Blend", comboFind(blendCombo, fx:getDualDestBlend()), blendCombo.labels)
			if c then fx:setDualDestBlend(blendCombo.values[di + 1]) end
		end
		if hasUV then
			prop.label("UV Transform", "(runtime animated)")
		end
	end
end

function guiGTAMatProps(m)
	local env = gta.getEnvMat(m)
	if env then
		prop.header("Env Mat")
		prop.label("Scale",       string.format("%.3f, %.3f", env:scaleX(), env:scaleY()))
		prop.label("Trans Scale", string.format("%.3f, %.3f", env:transScaleX(), env:transScaleY()))
		prop.label("Shininess",   string.format("%.3f", env:shininess()))
		prop.texture("Env Tex",   env.texture)
	end
	local spec = gta.getSpecMat(m)
	if spec then
		prop.header("Spec Mat")
		prop.label("Specularity", string.format("%.3f", spec.specularity))
		prop.texture("Spec Tex",  spec.texture)
	end
end

function guiMaterialProps(m)
	prop.begin("mat_props")
	prop.header("Material")
	local c, changed = prop.color("Color", m:getColor())
	if changed then m:setColor(c) end
	local s = m:getSurfaceProps()
	local v, ch = prop.dragFloat("Ambient",  s.ambient)
	if ch then s.ambient  = v end
	v, ch = prop.dragFloat("Diffuse",  s.diffuse)
	if ch then s.diffuse  = v end
	v, ch = prop.dragFloat("Specular", s.specular)
	if ch then s.specular = v end
	prop.texture("Texture", m:getTexture(), function(t) m:setTexture(t) end)
	guiMatFXProps(m)
	guiGTAMatProps(m)
	prop.finish()
end

function guiGeometry(g, selectedMaterial)
	prop.begin("geo_props")
	prop.header("Geometry")
	prop.label("Vertices",     g:getNumVertices())
	prop.label("Triangles",    g:getNumTriangles())
	prop.label("TexCoordSets", g:getNumTexCoordSets())
	prop.flags("Geo Flags", g:getFlags(), geoFlagBits,
		function(v) g:setFlags(v) end,
		rw.GEO_LIGHT | rw.GEO_MODULATE)
	prop.finish()
	if not ImGui.CollapsingHeader("Materials", ImGuiTreeNodeFlags.DefaultOpen) then
		return selectedMaterial
	end
	local i = 0
	for m in g:materials() do
		local col = m:getColor()
		ImGui.ColorButton("##col"..i, { col.r/255, col.g/255, col.b/255, col.a/255 },
			ImGuiColorEditFlags.NoTooltip | ImGuiColorEditFlags.NoDragDrop, 14, 14)
		ImGui.SameLine()
		local t = m:getTexture()
		local label = i .. ": " .. (t and t:getName() or "(no texture)")
		if ImGui.Selectable(label, m == selectedMaterial) then selectedMaterial = m end
		i = i + 1
	end
	if selectedMaterial then guiMaterialProps(selectedMaterial) end
	return selectedMaterial
end

function guiAtomic(a, selectedMaterial)
	if ImGui.CollapsingHeader("Atomic", ImGuiTreeNodeFlags.DefaultOpen) then
		prop.begin("atomic_props")
		prop.flags("Atomic Flags", a:getFlags(), atomicFlagBits,
			function(v) a:setFlags(v) end)
		prop.label("Pipeline ID",   string.format("0x%08X", a:getPipelineID()))
		prop.label("Pipeline Data", string.format("0x%08X", a:getPipelineData()))
		prop.finish()
	end
	return guiGeometry(a:getGeometry(), selectedMaterial)
end

function ClumpEditor:xformDragField(f, dragFn, applyFn)
	local v2, changed = dragFn()
	if changed then
		if not self.frameXformBefore then self.frameXformBefore = f:copyMatrix() end
		applyFn(v2)
	end
	if ImGui.IsItemDeactivatedAfterEdit() and self.frameXformBefore then
		local before = self.frameXformBefore
		self.frameXformBefore = nil
		local after = f:copyMatrix()
		self.history:push({
			undo = function() f:transform(before, rw.COMBINEREPLACE) end,
			redo = function() f:transform(after,  rw.COMBINEREPLACE) end,
		})
	end
end

function ClumpEditor:guiFrameTransform(f)
	prop.begin("frame_xform")
	prop.header("Local Transform")

	local function decomp()
		local t, r, s = sk.matDecompose(f:copyMatrix())
		return {t.x,t.y,t.z}, {r.x,r.y,r.z}, {s.x,s.y,s.z}
	end

	self:xformDragField(f,
		function()
			local t, r, s = decomp()
			return prop.dragFloat3("Translation", t, 0.01, -1e9, 1e9, "%.3f")
		end,
		function(v)
			local t, r, s = decomp()
			local m = f:copyMatrix()
			sk.matRecompose(m, rw.V3d(v[1],v[2],v[3]), rw.V3d(r[1],r[2],r[3]), rw.V3d(s[1],s[2],s[3]))
			f:transform(m, rw.COMBINEREPLACE)
		end)

	self:xformDragField(f,
		function()
			local t, r, s = decomp()
			return prop.dragFloat3("Rotation", r, 0.5, -360, 360, "%.2f")
		end,
		function(v)
			local t, r, s = decomp()
			local m = f:copyMatrix()
			sk.matRecompose(m, rw.V3d(t[1],t[2],t[3]), rw.V3d(v[1],v[2],v[3]), rw.V3d(s[1],s[2],s[3]))
			f:transform(m, rw.COMBINEREPLACE)
		end)

	self:xformDragField(f,
		function()
			local t, r, s = decomp()
			return prop.dragFloat3("Scale", s, 0.005, -1e6, 1e6, "%.4f")
		end,
		function(v)
			local t, r, s = decomp()
			local m = f:copyMatrix()
			sk.matRecompose(m, rw.V3d(t[1],t[2],t[3]), rw.V3d(r[1],r[2],r[3]), rw.V3d(v[1],v[2],v[3]))
			f:transform(m, rw.COMBINEREPLACE)
		end)

	prop.finish()
end

function ClumpEditor:guiFrame(f)
	if ImGui.CollapsingHeader("Frame", ImGuiTreeNodeFlags.DefaultOpen) then
		prop.begin("frame_props")
		prop.label("Name", f:getName())
		prop.finish()
		self:guiFrameTransform(f)
	end
	for a in f:atomics() do
		self.selectedMaterial = guiAtomic(a, self.selectedMaterial)
	end
end

function ClumpEditor:guiHierarchy(frame)
	local flags = ImGuiTreeNodeFlags.OpenOnArrow |
	              ImGuiTreeNodeFlags.OpenOnDoubleClick |
	              ImGuiTreeNodeFlags.DefaultOpen
	if frame:count() < 2 then flags = flags | ImGuiTreeNodeFlags.Leaf end
	local label = frame:getName()
	local hasAtomics = false
	for _ in frame:atomics() do hasAtomics = true; break end
	if hasAtomics then label = label .. "  [M]" end
	local open = ImGui.TreeNodeEx(label, flags)
	if ImGui.IsItemClicked() and not ImGui.IsItemToggledOpen() then
		local selFrame = self.selection and self.selection.frame or nil
		if selFrame ~= frame then
			self:frameSelect(frame)
			self.selectedMaterial = nil
		end
	end
	if open then
		for child in frame:children() do self:guiHierarchy(child) end
		ImGui.TreePop()
	end
end

function ClumpEditor:guiCarColWin()
	ImGui.SetNextWindowSize(260, 0)
	self.showCarCol = ImGui.Begin("Vehicle Colours", self.showCarCol,
		ImGuiWindowFlags.AlwaysAutoResize)
	if self.carColSlots then
		for i, s in ipairs(self.carColSlots) do
			local n = #s.mats
			local col, changed = ImGui.ColorEdit4(
				s.name .. " (" .. n .. ")##cc" .. i,
				s.chosen,
				ImGuiColorEditFlags.NoInputs | ImGuiColorEditFlags.AlphaBar)
			if changed then carColSetSlot(s, col) end
		end
		ImGui.Spacing()
		if ImGui.Button("Reset to key colours") then carColReset(self.carColSlots) end
	end
	ImGui.End()
end

-- ---------------------------------------------------------------------------
-- GUI windows / panels
-- ---------------------------------------------------------------------------

function ClumpEditor:guiSidebar(propW, statusH)
	ImGui.SetNextWindowPos(gWidth - propW, 0, ImGuiCond.Always)
	ImGui.SetNextWindowSize(propW, gHeight - statusH, ImGuiCond.Always)
	ImGui.SetNextWindowSizeConstraints(120, gHeight - statusH, gWidth - 60, gHeight - statusH)
	local propFlags = ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoTitleBar |
	                  ImGuiWindowFlags.NoBringToFrontOnFocus | ImGuiWindowFlags.NoCollapse
	local _, visible = ImGui.Begin("##props", true, propFlags)
	if visible then
		local pw, _ = ImGui.GetWindowSize()
		propW = math.floor(pw)
		if self.selection then self:guiFrame(self.selection.frame) end
	end
	ImGui.End()
	return propW
end

function ClumpEditor:guiWindows()
	if self.showHierarchy then
		self.showHierarchy = ImGui.Begin("Hierarchy", self.showHierarchy)
		if self.clump then self:guiHierarchy(self.clump:getFrame()) end
		ImGui.End()
	end

	if self.showCarCol then self:guiCarColWin() end

	if self.showTxd then
		self.showTxd = ImGui.Begin("Textures", self.showTxd)
		if self.texdict then
			self.selectedTexture = guiTexDict(self.texdict, self.selectedTexture)
		end
		ImGui.End()
	end
end

-- Bottom command bar.  Draws window-toggle buttons and, if onExit is set,
-- a "← Map" button that calls it.
function ClumpEditor:guiCommandBar(statusH)
	ImGui.SetNextWindowPos(0, gHeight - statusH, ImGuiCond.Always)
	ImGui.SetNextWindowSize(gWidth, statusH, ImGuiCond.Always)
	local flags = ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoResize |
	              ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoBringToFrontOnFocus |
	              ImGuiWindowFlags.NoCollapse | ImGuiWindowFlags.NoScrollbar
	ImGui.Begin("##commandbar", true, flags)

	if al.BeginMenu("Xform") then
		if al.MenuEntry("Translate") then gizmo.op = gizmo.TRANSLATE end
		if al.MenuEntry("Rotate")    then gizmo.op = gizmo.ROTATE    end
		al.EndMenu()
	end
	ImGui.SameLine()
	if al.BeginMenu("View") then
		local av = activeView
		if al.MenuEntryToggle("Grid", av.drawGrid)     then if av then av.drawGrid   = not av.drawGrid   end end
		if al.MenuEntryToggle("Wire", av.drawWire)     then if av then av.drawWire   = not av.drawWire   end end
		if al.MenuEntryToggle("Shaded", av.drawShaded) then if av then av.drawShaded = not av.drawShaded end end
		al.EndMenu()
	end
	ImGui.SameLine()
	if al.BeginMenu("Layout") then
		for _, entry in ipairs(layoutList) do
			if al.MenuEntry(entry.name) then
				currentLayout = entry.fn
				self:updateViewRect()
				layoutViews(true)
			end
		end
		al.EndMenu()
	end
	ImGui.SameLine()
	if al.BeginMenu("Window") then
		if al.MenuEntryToggle("Hierarchy",      self.showHierarchy) then self.showHierarchy = not self.showHierarchy end
		if al.MenuEntryToggle("Textures",       self.showTxd)       then self.showTxd       = not self.showTxd       end
		if al.MenuEntryToggle("Vehicle Colors", self.showCarCol)    then self.showCarCol    = not self.showCarCol    end
		al.EndMenu()
	end

	al.MenuIndicator("Xform")
	ImGui.SameLine()
	al.MenuIndicator("View")
	ImGui.SameLine()
	al.MenuIndicator("Layout")
	ImGui.SameLine()
	al.MenuIndicator("Window")

	if self.onExit then
		ImGui.SameLine()
		ImGui.Text("|")
		ImGui.SameLine()
		if ImGui.Button("Back to Map") then self.onExit() end
	end

	ImGui.End()
end

-- ---------------------------------------------------------------------------
-- View setup
-- ---------------------------------------------------------------------------

-- Call once after rwCamera is ready.  Populates the global views[] array.
function ClumpEditor:initViews()
	views    = {}
	activeView = nil
	local origin = rw.V3d(0, 0, 0)
	View.make(self.world, "Front", rw.Camera_PARALLEL,    rw.V3d(0,-5, 0), origin, true,  false)
	View.make(self.world, "Side",  rw.Camera_PARALLEL,    rw.V3d(5, 0, 0), origin, true,  false)
	local vTop  = View.make(self.world, "Top",   rw.Camera_PARALLEL,    rw.V3d(0, 0, 5), origin, true,  false)
	vTop.cam.up = rw.V3d(0, 1, 0)
	local vPersp = View.make(self.world, "Persp", rw.Camera_PERSPECTIVE, rw.V3d(4, 4, 2), origin, false, true)
	activeView   = vPersp
	currentLayout = layouts.fourUp
	self:updateViewRect()
	layoutViews()
end

function ClumpEditor:updateViewRect()
	viewRect = { x=0, y=0, w=gWidth - self.propW, h=gHeight - self.statusH }
end

-- Per-view camera input (same as modelview processCam).
function ClumpEditor:processCam(view)
	local cam = view.cam
	local r   = view.rect
	local dx  = (sk.mouse.x - sk.prevmouse.x) / r.w
	local dy  = (sk.mouse.y - sk.prevmouse.y) / r.h
	local alt = IsAltDown()
	local s   = 4.5
	if (sk.mouse.btn & 2) ~= 0 then
		if alt then cam:orbit(-dx*s, dy*s)
		else local d = cam:distanceTo(cam.target); cam:pan(-dx*d, dy*d) end
	elseif (sk.mouse.btn & 4) ~= 0 then
		if alt then cam:zoom(-dy*s*10) end
	end
	cam:zoom(sk.mouse.dwheel*2)
end

-- Full per-view render.
function ClumpEditor:renderView(view)
	local c = self.clump
	view:begin(self.rwCamera)

	-- Colour-coded pick pass: before the normal clear, while last frame's image is still there.
	if c and sk.isMouseClicked(sk.LMB) and view == activeView and not isDraggingTitle() then
		view.rwcam:clear(black, rw.Camera_CLEARIMAGE|rw.Camera_CLEARZ)
		self:renderCoded(c)
		local code = gta.GetColourCode(sk.curmouse.x, sk.curmouse.y)
		local a = self.idToAtomic[code]
		if a then
			self:frameSelect(a:getFrame())
			self.selectedMaterial = nil
		else
			self.selection = nil
		end
	end

	-- Normal render pass.
	view.rwcam:clear(self.background, rw.Camera_CLEARIMAGE|rw.Camera_CLEARZ)
	if view.drawGrid then gta.renderGrid(20, 20, 0.5) end
	if c then self:renderClump(view, c) end
	gta.renderDebugLines()
	drawViewOverlay(view)
	view:finish()
end

-- Full draw loop: call from the host script's Draw() when in clump mode.
function ClumpEditor:draw(timestep)
	self:updateViewRect()

	processViewInput()
	activeView = getActiveView()
	updateViewCameras()
	if activeView and not isDraggingTitle() and not markingActive then
		self:processCam(activeView)
	end
	activeCam = activeView and activeView.cam or nil

	if sk.isMouseClicked(sk.RMB) and not markingActive then
		self.selection = nil
	end

	self.rwCamera:clear(self.clearCol, rw.Camera_CLEARIMAGE|rw.Camera_CLEARZ)
	for _, v in ipairs(views) do self:renderView(v) end

	self.rwCamera:beginUpdate()
	sk.ImGuiBeginFrame(timestep)

	hovered = nil
	handleDrag()
	markingMenu()
	self:guiWindows()
	local newPropW = self:guiSidebar(self.propW, self.statusH)
	if newPropW ~= self.propW then
		self.propW = newPropW
		self:updateViewRect()
		layoutViews()
	end
	self:guiCommandBar(self.statusH)

	if activeView then
		local r = activeView.rect
		gizmo.Process(self.selection, activeView.rwcam, r.x, r.y, r.w, r.h)
	end

	sk.ImGuiEndFrame()
	self.rwCamera:endUpdate()
	self.rwCamera:showRaster(1)
end

-- ---------------------------------------------------------------------------
-- Editor method overrides
-- ---------------------------------------------------------------------------

function ClumpEditor:keyDown(k)
	local ctrl = IsCtrlDown()
	if k == sk.KEY_SPACE and activeView then
		activeView:toggleFullscreen()
	elseif k == sk.KEY_G and activeView then
		activeView.drawGrid = not activeView.drawGrid
	elseif k == sk.KEY_Z and activeView then
		activeView.drawWire = not activeView.drawWire
	elseif k == sk.KEY_X and activeView then
		activeView.drawShaded = not activeView.drawShaded
	elseif k == sk.KEY_W then
		gizmo.op = gizmo.TRANSLATE
	elseif k == sk.KEY_R then
		if ctrl then self:redo() else gizmo.op = gizmo.ROTATE end
	elseif k == sk.KEY_Z then
		if ctrl then self:undo() end
	end
end

function ClumpEditor:resize(w, h)
	sk.CameraSize(self.rwCamera, w, h)
	self:updateViewRect()
	layoutViews()
end
