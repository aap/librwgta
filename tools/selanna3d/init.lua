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
	return rw.Quat(q.w, -q.x, -q.y, -q.z)
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
	for k,v in pairs(t)  do
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
	g:ReadCdImage("models/gta3.img")
	g:ReadDataFile("data/default.dat")
	g:ReadDataFile("data/gta3.dat")
	g:FinishLoading()

	g:InstantiateScene("industsw.ipl")
	g:InstantiateScene("industse.ipl")
	g:InstantiateScene("industnw.ipl")
	g:InstantiateScene("industne.ipl")
	return g
end

function LoadVC()
	local g = gta.make(gta.GameVC, "/u/aap/gta/gta_miami")
	g:ReadCdImage("models/gta3.img")
	g:ReadDataFile("data/default.dat")
	g:ReadDataFile("data/gta_vc.dat")
	g:FinishLoading()

	g:InstantiateScene("oceandn.ipl")
	g:InstantiateScene("oceandrv.ipl")
	return g
end

function LoadLCS()
	local g = gta.make(gta.GameVC, "/u/aap/other/gta/gta_lcs")
	g:ReadCdImage("models/gta3.img")
	g:ReadDataFile("data/gta_lcs.dat")
	g:FinishLoading()

	g:InstantiateScene("indust.ipl")
	return g
end

function LoadVCS()
	local g = gta.make(gta.GameVC, "/u/aap/gta/vcs_map")
	g:ReadCdImage("models/gta3.img")
	g:ReadDataFile("data/gta_vcs.dat")
	g:FinishLoading()

	g:InstantiateScene("oceandn.ipl")
	g:InstantiateScene("oceandrv.ipl")
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
	sceneCam.fov = 80;
	sceneCam.near = 0.1;
	sceneCam.far = 2000;
	sceneCam.position = rw.V3d(679, -1006, 96)
	sceneCam.target = rw.V3d(1700, -455, -274)
	sceneCam:setDistance(30)
	sceneCam:update();

	modelCam = Camera.new()
	modelCam.rwCamera = rwCamera
	modelCam.aspectRatio = gWidth/gHeight
	modelCam.fov = 80;
	modelCam.near = 0.1;
	modelCam.far = 1000;
	modelCam.position = rw.V3d(20, 20, 20)
	modelCam.target = rw.V3d(0, 0, 0)
	modelCam:update();

print("done")
end

function processCam(cam)
	local dx = sk.mouse.x - sk.prevmouse.x
	local dy = sk.mouse.y - sk.prevmouse.y
	local s = 0.01
	if (sk.mouse.btn & 1) ~= 0 then
		cam:turn(-dx*s, -dy*s)
	elseif (sk.mouse.btn & 4) ~= 0 then
		cam:orbit(-dx*s, dy*s)
	end

	local shift = sk.keysdown[sk.KEY_LSHIFT] or sk.keysdown[sk.KEY_RSHIFT]
	s = 1.0
	if shift then s = s*2 end
	if sk.keysdown[sk.KEY_W] then
		cam:dolly(s)
	elseif sk.keysdown[sk.KEY_S] then
		cam:dolly(-s)
	elseif sk.keysdown[sk.KEY_A] then
		cam:pan(-s, 0)
	elseif sk.keysdown[sk.KEY_D] then
		cam:pan(s, 0)
	end
end

function Building:imguiTitle()
	return true, tostring(self.id) .. "\t" .. self.model
end

function Building:imguiDraw()
	if not ImGui.CollapsingHeader("Model", ImGuiTreeNodeFlags.DefaultOpen) then return end

	ImGui.PushID(tostring(self))
	guiTab(self.sourceFile, "File " .. self.sourceFile.fileName)
	local value, used = ImGui.InputInt("Id", self.id)
	local text, selected = ImGui.InputText("Model", self.model, 24)
	local text, selected = ImGui.InputText("Txd", self.texDict, 24)
	for i = 1, self.numAtomics do
		local value, used = ImGui.InputFloat("LOD dist"..i, self["lodDist"..i])
	end
	local value, used = ImGui.InputInt("Flags", self.flags)
	if self.timeOn then
		local value, used = ImGui.InputInt("Time On", self.timeOn)
		local value, used = ImGui.InputInt("Time Off", self.timeOff)
	end

	if ImGui.Button("View") then
		game:LoadAtomics(self)
		modelCam:jumpTo(rw.V3d(0,0,0))
		clump = self.rwClump
	end
	ImGui.PopID()
end

function Instance:imguiTitle()
	return self.show and self.sourceFile.showScene,
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
		local s = self.scale
		local values, used = ImGui.InputFloat3("Scale", {s.x, s.y, s.z})
		local q = self.rotation
		local values, used = ImGui.InputFloat4("Rotation", {q.x, q.y, q.z, q.w})
		if self.area then
			local value, used = ImGui.InputInt("Area", self.area)
		end
		if ImGui.Button("jump to") then
			activeCam:jumpTo(tV3d(p))
		end
		ImGui.Dummy(0, 20)
	end
	ImGui.PopID()

	if self.mdl then
		self.mdl:imguiDraw()
	end
end

function guiTabRaw(t, label)
	label = label or "table"
	if ImGui.TreeNode(label) then
		for k, v in pairs(t) do
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
			for k, v in pairs(t) do
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

function guiScenes(g)
	if not ImGui.TreeNode("Scenes") then return end
	for _, f in ipairs(g.filesOrdered) do
		if f.type == 'IPL' then
			ImGui.PushID(f.fileName)
			local clicked
			f.showScene, clicked = ImGui.Checkbox("visible", f.showScene or false)
			if clicked then
				if f.showScene then
					g:InstantiateScene(f.fileName)
				else
					g:HideScene(f.fileName)
				end
			end
			ImGui.SameLine()
			ImGui.Text(f.fileName)
			ImGui.PopID()
		end
	end
	ImGui.TreePop()
end

selection = nil
hovered = nil
drawColorCoded = false

function guiList(list, label, cullhidden)
	if not ImGui.TreeNode(label) then return end
	for _, v in ipairs(list) do
		local mt = getmetatable(v)
		local show = true
		local title = tostring(v)
		if mt and mt.imguiTitle then
			show, title = v:imguiTitle()
		end
		if show or not cullhidden then
			ImGui.PushID(tostring(v))
			if ImGui.Selectable(title, v == selection) then
				selection = v
			end
			if ImGui.IsItemHovered() then
				hovered = v
			end
			ImGui.PopID()
		end
	end
	ImGui.TreePop()
end

function guiCamera(cam)
	if not ImGui.CollapsingHeader("Camera", ImGuiTreeNodeFlags.DefaultOpen) then return end
	local p = cam.position
	local xyz, used = ImGui.InputFloat3("Position", {p.x, p.y, p.z})
	cam.position = rw.V3d(xyz[1], xyz[2], xyz[3])

	p = cam.target
	xyz, used = ImGui.InputFloat3("Target", {p.x, p.y, p.z})
	cam.target = rw.V3d(xyz[1], xyz[2], xyz[3])
end

function guiSelection()
	if selection then
		local mt = getmetatable(selection)
		if mt and mt.imguiDraw then
			selection:imguiDraw()
		else
			guiTab(selection, "selection")
		end
	end
end

function gui()
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
			guiSelection()
			ImGui.EndTabItem()
		end
		if ImGui.BeginTabItem("View") then
			if game then
				game.hour, _ = ImGui.InputInt("Hour", game.hour)
				game.hour = game.hour % 24
			end

			guiCamera(activeCam)
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

function gta:RenderMap()
	if drawColorCoded then
		gta.SetRenderColourCoded(1)
	end
	local drawLast = {}
	for _, inst in pairs(self.instances) do
		inst.colorId = inst.instId
		setColor(inst)
		local a = self:DrawInstance(inst)
		if a then
			table.insert(drawLast, inst)
		end
	end
	for _, inst in pairs(drawLast) do
		setColor(inst)
		gta.RenderAtomic(inst.rwAtomic)
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
	processCam(activeCam)
	activeCam:update()
	rwCamera:beginUpdate()

	if game and not clump and sk.isMouseClicked(sk.LMB) then
		rwCamera:clear(black, rw.Camera_CLEARIMAGE|rw.Camera_CLEARZ)
		drawColorCoded = true
		game:RenderMap()
		drawColorCoded = false
		local code = gta.GetColourCode(sk.curmouse.x, sk.curmouse.y)
		selection = game.instances[code]
	end

	rwCamera:clear(clearCol, rw.Camera_CLEARIMAGE|rw.Camera_CLEARZ)

	sk.ImGuiBeginFrame(timestep)

	hovered = nil
	gui()

	if clump then
		clump:render()
	elseif game then
		game:RenderMap()
	end

	sk.ImGuiEndFrame()

	rwCamera:endUpdate()
	rwCamera:showRaster(0)
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

function sk.updateMouse()
	sk.prevmouse = sk.mouse
	sk.mouse = copytab(sk.curmouse)

	if sk.clickstate == 2 then sk.clickstate = 0 end
	if sk.clickstate == 0 and (sk.mouse.btn & (sk.mouse.btn ~ sk.prevmouse.btn)) ~= 0 then
		sk.clickstate = 1
		sk.clickx = sk.mouse.x
		sk.clicky = sk.mouse.y
	elseif sk.clickstate == 1 and sk.mouse.btn == 0 then
		if sk.mouse.x == sk.clickx and sk.mouse.y == sk.clicky then
			sk.clickstate = 2
			sk.clickbtn = sk.prevmouse.btn & ~(sk.prevmouse.btn-1)
			local logtab = { 0, 1, 2, 0, 3 }
			sk.clickbtn = logtab[(sk.clickbtn&7)+1]
		else
			sk.clickstate = 0
		end
	end
end

function KeyUp(k)
	sk.keysdown[k] = false
end

function KeyDown(k)
	sk.keysdown[k] = true

	if k == sk.KEY_C then
		local cam = activeCam
		print(cam.position)
		print(cam.target)
		print(cam.fov)
		print(cam.aspectRatio)
		print(cam.near)
		print(cam.far)
	elseif k == sk.KEY_V then
		if vc then
			game = vc
		end
	elseif k == sk.KEY_B then
		if iii then
			game = iii
		end
	elseif k == sk.KEY_N then
		if lcs then
			game = lcs
		end
	elseif k == sk.KEY_M then
		if vcs then
			game = vcs
		end
	end
end

sk.curmouse = { x = 0, y = 0, btn = 0 }
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
