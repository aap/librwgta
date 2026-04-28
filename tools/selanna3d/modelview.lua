include('editor_util.lua')


function hideDamagedLOD(a)
	local name = a:getFrame():getName()
	if string.find(name, "_dam") or
	   string.find(name, "_vlo") or
	   string.find(name, "_lo_") then
		a:setVisibility(false)
	end
end

clump = nil
texdict = nil
activeCam = nil
rwCamera = nil   -- full-screen camera (ImGui target + showRaster)

-- View3D: 4-up viewport system.
-- Each entry: { cam=Camera, rwcam=rwCamera, proj, name, rect={x,y,w,h} }
views = {}
activeView = nil

clearCol = rw.RGBA(0x80, 0x80, 0x80, 0xFF)
background = rw.RGBA(0xA1, 0xA1, 0xA1, 0xFF)
--filedir = "/u/aap/gta/gta_miami/models/gta3_img"
--filedir = "/u/aap/gta/gta3_re/models/gta3_img"
filedir = "/u/aap/other/gta/gtasa/models/gta3_img"

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

	local origin = rw.V3d(0, 0, 0)
	-- top-left: front (looking along +Y, Z up)
	makeView("Front", rw.Camera_PARALLEL,    rw.V3d(0,-5, 0), origin, true,  false)
	-- top-right: side (looking along -X, Z up)
	makeView("Side",  rw.Camera_PARALLEL,    rw.V3d(5, 0, 0), origin, true,  false)
	-- bottom-left: top (looking down -Z, Y forward)
	local vTop = makeView("Top", rw.Camera_PARALLEL, rw.V3d(0, 0, 5), origin, true, false)
	vTop.cam.up = rw.V3d(0, 1, 0)
	-- bottom-right: perspective
	local vPersp = makeView("Persp", rw.Camera_PERSPECTIVE, rw.V3d(4, 4, 2), origin, false, true)

	activeView = vPersp
	currentLayout = layouts.fourUp
	layoutViews()

	gizmo.stepTrans = 0.1
	gizmo.stepRot   = 5
	gizmo.snapTrans = true
	gizmo.snapRot   = true
	rw.ImageSetSearchPath("/u/aap/other/gta/gtasa/models/generic/generic_txd/")
	clump, texdict = LoadClumpWithTxd(filedir, "cheetah.dff", "cheetah.txd")
--	clump, texdict = LoadClumpWithTxd(filedir, "player.dff", "player.txd")
print("done")
end

function LoadClumpWithTxd(filedir, dff, txd)
	local texdict = LoadTexDict(rw.makePath(filedir .. "/" .. txd))
	rw.setCurrentTexDict(texdict)
	local clump = LoadClump(rw.makePath(filedir .. "/" .. dff))
	return clump, texdict
end

function LoadTexDict(path)
	local txd = rw.readTexDictFile(path, 0, 1000000)
	if txd then
		sk.ConvertTexDict(txd)
	end
	return txd
end

function LoadClump(path)
	local c = rw.readClumpFile(path, 0, 1000000)
	if c then
		sk.ConvertClump(c)
		for a in c:atomics() do
			hideDamagedLOD(a)
			gta.SetupAtomicPipelines(a)
		end
		carColScanClump(c)
	end
	return c
end

function processCam(view)
	local cam = view.cam
	local r = view.rect
	local dx = (sk.mouse.x - sk.prevmouse.x) / r.w
	local dy = (sk.mouse.y - sk.prevmouse.y) / r.h
	local alt = IsAltDown()
	local isOrtho = view.rwcam:getProjection() == rw.Camera_PARALLEL

	local s = 4.5
	if (sk.mouse.btn & 2) ~= 0 then
		if alt then
			cam:orbit(-dx*s, dy*s)
		else
			local d = cam:distanceTo(cam.target)
			cam:pan(-dx*d, dy*d)
		end
	elseif (sk.mouse.btn & 4) ~= 0 then
		if alt then
			cam:zoom(-dy*s*10)
		end
	end

	cam:zoom(sk.mouse.dwheel*2)
end

selection = nil
hovered = nil
drawColorCoded = false

-- Layout constants (pixels).
statusH = 32   -- bottom status bar height
propW   = 320  -- right properties panel initial width (resizable)

-- Frame metatable with gizmo support.
-- gizmo.Process() calls selection:gizmo(phase, pos, rot) when the gizmo is active.
-- pos/rot are the new world-space position and rotation from ImGuizmo.
-- We convert to local space by factoring out the parent LTM.
local frameMeta = {}
frameMeta.__index = frameMeta

local frameGizmoBefore = nil  -- local matrix snapshot for undo

function frameMeta:gizmo(phase, pos, rot)
	local f = self.frame
	if phase == 0 then
		gizmo.InitMatrix(f:getLTM())
		frameGizmoBefore = f:copyMatrix()
	else
		local ltmCur = gizmo.GetMatrix()
		local parent = f:getParent()
		local localM = parent and ltmCur * rw.matInvert(parent:getLTM()) or ltmCur
		f:transform(localM, rw.COMBINEREPLACE)

		if phase == 2 then
			local before = frameGizmoBefore
			frameGizmoBefore = nil
			local after = f:copyMatrix()
			History:push({ undo = function() f:transform(before, rw.COMBINEREPLACE) end,
			               redo = function() f:transform(after,  rw.COMBINEREPLACE) end })
		end
	end
end

function frameSelect(frame)
	selection = setmetatable({ frame = frame }, frameMeta)
	gizmo.InitMatrix(frame:getLTM())
end

function frameHasAtomics(frame)
	for a in frame:atomics() do return true end
	return false
end

function guiHierarchy(frame)
	local flags = ImGuiTreeNodeFlags.OpenOnArrow |
	              ImGuiTreeNodeFlags.OpenOnDoubleClick |
		ImGuiTreeNodeFlags.DefaultOpen
	if frame:count() < 2 then
		flags = flags | ImGuiTreeNodeFlags.Leaf
	end
	local label = frame:getName()
	if frameHasAtomics(frame) then label = label .. "  [M]" end
	local open = ImGui.TreeNodeEx(label, flags)
	if ImGui.IsItemClicked() and not ImGui.IsItemToggledOpen() then
		local selFrame = selection and selection.frame or nil
		if selFrame ~= frame then
			frameSelect(frame)
			selectedMaterial = nil
		end
	end
	if open then
		for child in frame:children() do
			guiHierarchy(child)
		end
		ImGui.TreePop()
	end
end

selectedMaterial = nil
selectedTexture  = nil

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
	if (fmt & rw.RASTER_PAL8)      ~= 0 then s = s .. " PAL8" end
	if (fmt & rw.RASTER_PAL4)      ~= 0 then s = s .. " PAL4" end
	if (fmt & rw.RASTER_MIPMAP)    ~= 0 then s = s .. " MIP" end
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

-- Compact clickable name in the TXD list; 64px hover preview.
function guiTexture(t)
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
end

-- Full detail: metadata + correctly aspect-ratioed image.
-- Ordered combo list helpers: { values={...}, labels={...} }
-- valToIdx returns 0-based ImGui index, idxToVal maps back.
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

local txdZoomIdx = 3  -- default 100% (index into txdZoomLevels)
local txdZoomLevels  = { 0.25, 0.50, 0.75, 1.00, 1.25, 1.50, 1.75, 2.00 }
local txdZoomLabels  = { "25%", "50%", "75%", "100%", "125%", "150%", "175%", "200%" }

function guiTextureFull(t)
	prop.begin("txd_detail_props")
	local mask = t:getMaskName()
	if mask ~= "" then prop.label("Mask", mask) end

	local fi, changed = prop.combo("Filter", comboFind(filterCombo, t:getFilter()), filterCombo.labels)
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

function guiTexDict(txd)
	local avail = ImGui.GetContentRegionAvail()
	ImGui.BeginChild("txd_list", avail/2, 0, ImGuiChildFlags.ResizeX | ImGuiChildFlags.Borders, 0)
	for t in txd:textures() do
		guiTexture(t)
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
end

function guiMatFXProps(m)
	-- MatFX effect combo: always shown so you can switch to Nothing or add an effect.
	-- setEffects handles union clearing internally when switching type.
	prop.header("MatFX")
	local fxCombo = {
		values = { rw.MATFX_NOTHING, rw.MATFX_BUMPMAP, rw.MATFX_ENVMAP, rw.MATFX_BUMPENVMAP,
		           rw.MATFX_DUAL, rw.MATFX_UVTRANSFORM, rw.MATFX_DUALUVTRANSFORM },
		labels = { "Nothing", "BumpMap", "EnvMap", "BumpMap+EnvMap",
		           "Dual", "UVTransform", "Dual UVTransform" },
	}
	local fx = m:getMatFX()
	local fxtype = fx and fx:getType() or rw.MATFX_NOTHING
	local fi, changed = prop.combo("Effect", comboFind(fxCombo, fxtype), fxCombo.labels)
	if changed then
		m:setMatFXEffect(fxCombo.values[fi + 1])
		fx = m:getMatFX()
		fxtype = fx and fx:getType() or rw.MATFX_NOTHING
	end

	if fx and fxtype ~= rw.MATFX_NOTHING then
		-- fx[0]/fx[1] slot layout per type:
		--   BUMPMAP        -> [BUMPMAP, NOTHING]
		--   ENVMAP         -> [ENVMAP,  NOTHING]
		--   BUMPENVMAP     -> [BUMPMAP, ENVMAP]
		--   DUAL           -> [DUAL,    NOTHING]
		--   UVTRANSFORM    -> [UVTRANSFORM, NOTHING]
		--   DUALUVTRANSFORM-> [UVTRANSFORM, DUAL]
		local hasBump = fxtype == rw.MATFX_BUMPMAP    or fxtype == rw.MATFX_BUMPENVMAP
		local hasEnv  = fxtype == rw.MATFX_ENVMAP     or fxtype == rw.MATFX_BUMPENVMAP
		local hasDual = fxtype == rw.MATFX_DUAL       or fxtype == rw.MATFX_DUALUVTRANSFORM
		local hasUV   = fxtype == rw.MATFX_UVTRANSFORM or fxtype == rw.MATFX_DUALUVTRANSFORM

		if hasBump then
			prop.texture("Bump Map", fx:getBumpTexture(), function(t) fx:setBumpTexture(t) end)
			local v, changed = prop.dragFloat("Bump Coeff", fx:getBumpCoefficient(), 0.005, 0, 1)
			if changed then fx:setBumpCoefficient(v) end
		end

		if hasEnv then
			prop.texture("Env Map", fx:getEnvTexture(), function(t) fx:setEnvTexture(t) end)
			local v, changed = prop.dragFloat("Env Coeff", fx:getEnvCoefficient(), 0.005, 0, 1)
			if changed then fx:setEnvCoefficient(v) end
			local fb, changed = prop.check("Env FB Alpha", fx:getEnvFBAlpha())
			if changed then fx:setEnvFBAlpha(fb) end
		end

		if hasDual then
			local blendCombo = {
				values = { rw.BLEND_ZERO, rw.BLEND_ONE, rw.BLEND_SRCCOLOR, rw.BLEND_INVSRCCOLOR,
				           rw.BLEND_SRCALPHA, rw.BLEND_INVSRCALPHA, rw.BLEND_DESTALPHA,
				           rw.BLEND_INVDESTALPHA, rw.BLEND_DESTCOLOR, rw.BLEND_INVDESTCOLOR,
				           rw.BLEND_SRCALPHASAT },
				labels = { "Zero", "One", "SrcColor", "InvSrcColor",
				           "SrcAlpha", "InvSrcAlpha", "DestAlpha",
				           "InvDestAlpha", "DestColor", "InvDestColor",
				           "SrcAlphaSat" },
			}
			prop.texture("Dual Map", fx:getDualTexture(), function(t) fx:setDualTexture(t) end)
			local si, changed = prop.combo("Src Blend",  comboFind(blendCombo, fx:getDualSrcBlend()),  blendCombo.labels)
			if changed then fx:setDualSrcBlend(blendCombo.values[si + 1]) end
			local di, changed = prop.combo("Dest Blend", comboFind(blendCombo, fx:getDualDestBlend()), blendCombo.labels)
			if changed then fx:setDualDestBlend(blendCombo.values[di + 1]) end
		end

		if hasUV then
			-- UV transform matrices are runtime-animated; no static values to edit here.
			prop.label("UV Transform", "(runtime animated)")
		end
	end
end

function guiGTAMatProps(m)
	local env = gta.getEnvMat(m)
	if env then
		prop.header("Env Mat")
		prop.label("Scale",      string.format("%.3f, %.3f", env:scaleX(), env:scaleY()))
		prop.label("Trans Scale", string.format("%.3f, %.3f", env:transScaleX(), env:transScaleY()))
		prop.label("Shininess",  string.format("%.3f", env:shininess()))
		prop.texture("Env Tex",  env.texture)
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
	local v, changed = prop.dragFloat("Ambient",  s.ambient)
	if changed then s.ambient = v end
	v, changed = prop.dragFloat("Diffuse",  s.diffuse)
	if changed then s.diffuse = v end
	v, changed = prop.dragFloat("Specular", s.specular)
	if changed then s.specular = v end

	prop.texture("Texture", m:getTexture(), function(t) m:setTexture(t) end)

	guiMatFXProps(m)
	guiGTAMatProps(m)

	prop.finish()
end

function guiGeometry(g)
	prop.begin("geo_props")
	prop.header("Geometry")
	prop.label("Vertices",     g:getNumVertices())
	prop.label("Triangles",    g:getNumTriangles())
	prop.label("TexCoordSets", g:getNumTexCoordSets())
	prop.flags("Geo Flags", g:getFlags(), geoFlagBits,
		function(v) g:setFlags(v) end,
		rw.GEO_LIGHT | rw.GEO_MODULATE)
	prop.finish()

	if not ImGui.CollapsingHeader("Materials", ImGuiTreeNodeFlags.DefaultOpen) then return end
	local i = 0
	for m in g:materials() do
		local c = m:getColor()
		-- small color swatch before the label
		ImGui.ColorButton("##col"..i, { c.r/255, c.g/255, c.b/255, c.a/255 },
			ImGuiColorEditFlags.NoTooltip | ImGuiColorEditFlags.NoDragDrop, 14, 14)
		ImGui.SameLine()
		local t = m:getTexture()
		local label = i .. ": " .. (t and t:getName() or "(no texture)")
		if ImGui.Selectable(label, m == selectedMaterial) then
			selectedMaterial = m
		end
		i = i + 1
	end

	if selectedMaterial then
		guiMaterialProps(selectedMaterial)
	end
end

function guiAtomic(a)
	if ImGui.CollapsingHeader("Atomic", ImGuiTreeNodeFlags.DefaultOpen) then
		prop.begin("atomic_props")
		prop.flags("Atomic Flags", a:getFlags(), atomicFlagBits,
			function(v) a:setFlags(v) end)
		prop.label("Pipeline ID",   string.format("0x%08X", a:getPipelineID()))
		prop.label("Pipeline Data", string.format("0x%08X", a:getPipelineData()))
		prop.finish()
	end
	guiGeometry(a:getGeometry())
end

-- Per-drag undo state: matrix snapshot before the current drag started.
local frameXformBefore = nil

local function xformDragField(f, dragFn, applyFn)
	local v2, changed = dragFn()
	if changed then
		if not frameXformBefore then frameXformBefore = f:copyMatrix() end
		applyFn(v2)
	end
	if ImGui.IsItemDeactivatedAfterEdit() and frameXformBefore then
		local before = frameXformBefore
		frameXformBefore = nil
		local after = f:copyMatrix()
		History:push({ undo = function() f:transform(before, rw.COMBINEREPLACE) end,
		               redo = function() f:transform(after,  rw.COMBINEREPLACE) end })
	end
end

function guiFrameTransform(f)
	prop.begin("frame_xform")
	prop.header("Local Transform")

	-- Re-decompose each call so each field always reflects the live matrix.
	local function decomp()
		local t, r, s = sk.matDecompose(f:copyMatrix())
		return {t.x,t.y,t.z}, {r.x,r.y,r.z}, {s.x,s.y,s.z}
	end

	xformDragField(f,
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

	xformDragField(f,
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

	xformDragField(f,
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

function guiFrame(f)
	if ImGui.CollapsingHeader("Frame", ImGuiTreeNodeFlags.DefaultOpen) then
		prop.begin("frame_props")
		prop.label("Name", f:getName())
		prop.finish()
		guiFrameTransform(f)
	end
	for a in f:atomics() do
		guiAtomic(a)
	end
end

local showHierarchyWin = true
local showTxdWin = false
local showCarColWin = false

-- ---------------------------------------------------------------------------
-- Vehicle colour slots
-- ---------------------------------------------------------------------------
-- Each slot has a magic material color key (24-bit RGB 0xRRGGBB) that the
-- game bakes into materials to mark which colour slot they belong to.
-- After scanning, s.mats holds all matched materials and s.chosen mirrors
-- the colour currently applied to them (initialized from the first match,
-- or the key itself when none are found).

local carColSlots = {
	{ name = "Body 1",   key = 0x3cff00 },
	{ name = "Body 2",   key = 0xff00af },
	{ name = "Body 3",   key = 0x00ffff },
	{ name = "Body 4",   key = 0xff00ff },
	{ name = "Light FL", key = 0xffaf00 },
	{ name = "Light FR", key = 0x00ffc8 },
	{ name = "Light RL", key = 0xb9ff00 },
	{ name = "Light RR", key = 0xff3c00 },
}

local carColKeyToSlot = {}
for i, s in ipairs(carColSlots) do
	s.mats   = {}
	s.chosen = { ((s.key>>16)&0xff)/255, ((s.key>>8)&0xff)/255, (s.key&0xff)/255, 1 }
	carColKeyToSlot[s.key] = i
end

-- Scan all materials in clump c, bucket by key color.
-- chosen is set from the first matched material's actual current color
-- so the picker always reflects what is in the scene.
function carColScanClump(c)
	for _, s in ipairs(carColSlots) do s.mats = {} end
	if not c then return end
	for a in c:atomics() do
		for m in a:getGeometry():materials() do
			local col = m:getColor()
			local key = col.r * 0x10000 + col.g * 0x100 + col.b
			local si = carColKeyToSlot[key]
			if si then
				local s = carColSlots[si]
				if #s.mats == 0 then
					-- Initialise chosen from scene color of first match.
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
	for _, m in ipairs(s.mats) do
		m:setColor(rw.RGBA(r, g, b, a))
	end
end

local function carColReset()
	for _, s in ipairs(carColSlots) do
		local r = (s.key>>16)&0xff
		local g = (s.key>>8) &0xff
		local b =  s.key     &0xff
		carColSetSlot(s, { r/255, g/255, b/255, 1 })
	end
end

local function guiCarColWin()
	ImGui.SetNextWindowSize(260, 0)
	showCarColWin = ImGui.Begin("Vehicle Colours", showCarColWin,
		ImGuiWindowFlags.AlwaysAutoResize)
	for i, s in ipairs(carColSlots) do
		local n = #s.mats
		local col, changed = ImGui.ColorEdit4(
			s.name .. " (" .. n .. ")##cc" .. i,
			s.chosen,
			ImGuiColorEditFlags.NoInputs | ImGuiColorEditFlags.AlphaBar)
		if changed then carColSetSlot(s, col) end
	end
	ImGui.Spacing()
	if ImGui.Button("Reset to key colours") then carColReset() end
	ImGui.End()
end

function gui()
	if showHierarchyWin then
		showHierarchyWin = ImGui.Begin("Hierarchy", showHierarchyWin)
		if clump then
			guiHierarchy(clump:getFrame())
		end
		ImGui.End()
	end


	if showCarColWin then guiCarColWin() end

	if showTxdWin then
		showTxdWin = ImGui.Begin("Textures", showTxdWin)
		if texdict then
			guiTexDict(texdict)
		end
		ImGui.End()
	end

	-- Right properties panel: fixed to right edge, full height minus status bar.
	-- Only horizontal resize is allowed (drag the left edge).
	ImGui.SetNextWindowPos(gWidth - propW, 0, ImGuiCond.Always)
	ImGui.SetNextWindowSize(propW, gHeight - statusH, ImGuiCond.Always)
	ImGui.SetNextWindowSizeConstraints(120, gHeight - statusH, gWidth - 60, gHeight - statusH)
	local propFlags = ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoTitleBar |
	                  ImGuiWindowFlags.NoBringToFrontOnFocus | ImGuiWindowFlags.NoCollapse
	local _, propVisible = ImGui.Begin("##props", true, propFlags)
	if propVisible then
		-- Sync propW so the panel sticks to the right edge when the user drags.
		local pw, ph = ImGui.GetWindowSize()
		propW = math.floor(pw)

		if selection then
			guiFrame(selection.frame)
		end
	end
	ImGui.End()

	-- Bottom status bar: fixed to bottom edge, full width.
	ImGui.SetNextWindowPos(0, gHeight - statusH, ImGuiCond.Always)
	ImGui.SetNextWindowSize(gWidth, statusH, ImGuiCond.Always)
	local sbFlags = ImGuiWindowFlags.NoMove | ImGuiWindowFlags.NoResize |
	                ImGuiWindowFlags.NoTitleBar | ImGuiWindowFlags.NoBringToFrontOnFocus |
	                ImGuiWindowFlags.NoCollapse | ImGuiWindowFlags.NoScrollbar
	ImGui.Begin("##statusbar", true, sbFlags)  -- returns open, visible; we ignore both

	-- Gizmo op buttons.
	local function opBtn(label, op)
		local active = gizmo.op == op
		if active then ImGui.PushStyleColor(ImGuiCol.Button, 0.3, 0.6, 0.9, 1.0) end
		if ImGui.Button(label) then gizmo.op = op end
		if active then ImGui.PopStyleColor() end
		ImGui.SameLine()
	end
	opBtn("Translate", gizmo.TRANSLATE)
	opBtn("Rotate",    gizmo.ROTATE)

	ImGui.Text("|")
	ImGui.SameLine()

	-- Gizmo mode toggle.
	local modeLocal = gizmo.mode == gizmo.LOCAL
	local modeLabel = modeLocal and "Local" or "World"
	if ImGui.Button(modeLabel) then
		gizmo.mode = modeLocal and gizmo.WORLD or gizmo.LOCAL
	end
	ImGui.SameLine()

	ImGui.Text("|")
	ImGui.SameLine()

	-- Snap toggles.
	gizmo.snapTrans, _ = ImGui.Checkbox("Snap T", gizmo.snapTrans)
	ImGui.SameLine()
	if gizmo.snapTrans then
		ImGui.SetNextItemWidth(60)
		gizmo.stepTrans, _ = ImGui.DragFloat("##snaptval", gizmo.stepTrans, 0.1, 0.01, 100, "%.2f")
		ImGui.SameLine()
	end

	gizmo.snapRot, _ = ImGui.Checkbox("Snap R", gizmo.snapRot)
	ImGui.SameLine()
	if gizmo.snapRot then
		ImGui.SetNextItemWidth(60)
		gizmo.stepRot, _ = ImGui.DragFloat("##snaprval", gizmo.stepRot, 0.5, 0.1, 90, "%.1f")
		ImGui.SameLine()
	end

	ImGui.Text("|")
	ImGui.SameLine()

	activeView.drawGrid, _ = ImGui.Checkbox("Draw Grid", activeView.drawGrid)
	ImGui.SameLine()
	activeView.drawWire, _ = ImGui.Checkbox("Draw Wire", activeView.drawWire)
	ImGui.SameLine()
	activeView.drawShaded, _ = ImGui.Checkbox("Draw Shaded", activeView.drawShaded)
	ImGui.SameLine()

	ImGui.Text("|")
	ImGui.SameLine()

	-- Layout buttons.
	for _, entry in ipairs(layoutList) do
		local active = currentLayout == entry.fn
		if active then ImGui.PushStyleColor(ImGuiCol.Button, 0.3, 0.6, 0.9, 1.0) end
		if ImGui.Button(entry.name) then
			currentLayout = entry.fn
			layoutViews()
		end
		if active then ImGui.PopStyleColor() end
		ImGui.SameLine()
	end

	ImGui.Text("|")
	ImGui.SameLine()

	-- Window toggle buttons.
	local function winBtn(label, flag)
		if flag then ImGui.PushStyleColor(ImGuiCol.Button, 0.3, 0.6, 0.9, 1.0) end
		local clicked = ImGui.Button(label)
		if flag then ImGui.PopStyleColor() end
		ImGui.SameLine()
		return clicked
	end
	if winBtn("Hierarchy", showHierarchyWin) then showHierarchyWin = not showHierarchyWin end
	if winBtn("Textures",  showTxdWin)       then showTxdWin = not showTxdWin end
	if winBtn("Car Cols",  showCarColWin)    then showCarColWin = not showCarColWin end

	ImGui.End()
end

red   = rw.RGBA(255, 0,   0,   255)
blue  = rw.RGBA(0,   0,   255, 255)
black = rw.RGBA(0,   0,   0,   255)
white = rw.RGBA(255, 255, 255, 255)
darkblue = rw.RGBA(0,   4,   96, 255)

-- Atomic colour-coded picking tables.
-- Lazily assigned: first time an atomic is rendered we give it an id.
local atomicToId = {}  -- atomic userdata pointer → integer id (1-based)
local idToAtomic = {}  -- integer id → atomic userdata

local function atomicPickId(a)
	local key = a:ptr()
	local id = atomicToId[key]
	if not id then
		id = #idToAtomic + 1
		atomicToId[key] = id
		idToAtomic[id] = a
	end
	return id
end

-- Render all clump atomics, setting colour code per atomic for picking.
local function renderClumpCoded(c)
	gta.SetRenderColourCoded(1)
	for a in c:atomics() do
		gta.SetColourCode(atomicPickId(a))
		if a:isVisible() then
			a:render()
		end
	end
	gta.SetRenderColourCoded(0)
end

-- Normal render with highlight on the selected frame's atomics.
local function renderClump(view, c)
	local selFrame = selection and selection.frame or nil
	for a in c:atomics() do
		local wirecol = darkblue
		if a:getFrame() == selFrame then
			wirecol = white
		end
		if a:isVisible() then
			if view.drawShaded then a:render() end
			if view.drawWire then gta.renderWireAtomic(a, wirecol) end
		end
	end
end

xaxis = rw.V3d(1,0,0)
yaxis = rw.V3d(0,1,0)
zaxis = rw.V3d(0,0,1)

include('view3d.lua')

local function renderView(view)
	local rwcam = view.rwcam
	rwcam:beginUpdate()

	if clump and sk.isMouseClicked(sk.LMB) and view == activeView and not isDraggingTitle() then
		rwcam:clear(black, rw.Camera_CLEARIMAGE|rw.Camera_CLEARZ)
		renderClumpCoded(clump)
		local code = gta.GetColourCode(sk.curmouse.x, sk.curmouse.y)
		local a = idToAtomic[code]
		if a then
			frameSelect(a:getFrame())
			selectedMaterial = nil
		else
			selection = nil
		end
	end

	rwcam:clear(background, rw.Camera_CLEARIMAGE|rw.Camera_CLEARZ)

	if view.drawGrid then
		gta.renderGrid(20, 20, 0.5)
	end
	if clump then
		renderClump(view, clump)
	end
	gta.renderDebugLines()
	drawViewOverlay(view)

	rwcam:endUpdate()
end

function Draw(timestep)
	sk.updateMouse()

	processViewInput()
	activeView = getActiveView()

	updateViewCameras()
	if activeView and not isDraggingTitle() then
		processCam(activeView)
	end
	activeCam = activeView and activeView.cam or nil

	if sk.isMouseClicked(sk.RMB) then
		selection = nil
	end

	rwCamera:clear(clearCol, rw.Camera_CLEARIMAGE|rw.Camera_CLEARZ)

	-- Render each view (views hidden by the layout have a 1x1 rect and are harmless to render).
	for _, v in ipairs(views) do
		renderView(v)
	end

	-- ImGui renders over the full framebuffer (panels + gizmo overlay).
	rwCamera:beginUpdate()

	sk.ImGuiBeginFrame(timestep)

	hovered = nil
	gui()
	-- Only call Manipulate once (active view). Multiple Manipulate calls per
	-- frame corrupt ImGuizmo's single-instance state machine.
	if activeView then
		local r = activeView.rect
		gizmo.Process(activeView.rwcam, r.x, r.y, r.w, r.h)
	end

	sk.ImGuiEndFrame()

	rwCamera:endUpdate()
	rwCamera:showRaster(1)
end

function KeyDown(k)
	sk.keysdown[k] = true

	local ctrl = IsCtrlDown()
	if k == sk.KEY_SPACE then
		toggleFullscreen(activeView)
	elseif k == sk.KEY_G then
		activeView.drawGrid = not activeView.drawGrid
	elseif k == sk.KEY_Z then
		activeView.drawWire = not activeView.drawWire
	elseif k == sk.KEY_X then
		activeView.drawShaded = not activeView.drawShaded
	elseif k == sk.KEY_W then
		gizmo.op = gizmo.TRANSLATE
	elseif k == sk.KEY_R then
		if ctrl then History:redo() else gizmo.op = gizmo.ROTATE end
	elseif k == sk.KEY_Z then
		if ctrl then History:undo() end
	end
end

function KeyUp(k)
	sk.keysdown[k] = false
end

function Resize(w, h)
	sk.CameraSize(rwCamera, w, h)
	layoutViews()
end
