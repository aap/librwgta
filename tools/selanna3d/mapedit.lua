include('editor_util.lua')
include('gta.lua')
include('gtadraw.lua')
include('console.lua')

function LoadIII()
	local g = gta.make(gta.GameIII, "/u/aap/gta/gta3_re")
--	local g = gta.make(gta.GameIII, "/u/aap/other/gta/gta3_upstate")
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

selection = nil
hovered = nil
drawColorCoded = false
viewer = { lodMode = 1, lodMult = 1.5, drawCollision = false }

local undoStart;

test3 = "/u/aap/gta/test3"
testvc = "/u/aap/gta/testvc"
testsa = "/u/aap/gta/testsa"

include('gui.lua')

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
		local a = self:DrawInstance(inst, inst == selection)
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

xaxis = rw.V3d(1,0,0)
yaxis = rw.V3d(0,1,0)
zaxis = rw.V3d(0,0,1)

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

	gta.renderAxesWidget(activeCam.target, xaxis, yaxis, zaxis)

	if clump then
		clump:render()
	elseif game then
		if game.dayNightBalance then gta.SetDayNightBalance(game.dayNightBalance) end
		if game.wetRoads then gta.SetWetRoads(game.wetRoads) end
		game:RenderMap()
	end

	gta.renderDebugLines()

	sk.ImGuiEndFrame()

	rwCamera:endUpdate()
	rwCamera:showRaster(1)
end

function Resize(w, h)
	sk.CameraSize(rwCamera, w, h)
	sceneCam.aspectRatio = w/h
	modelCam.aspectRatio = w/h
end
