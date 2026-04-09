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

viewer = { lodMode = 1, lodMult = 1.5, drawCollision = false }

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
