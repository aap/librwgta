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
		guiList(game.atomicModels, "Atomic Models")
		guiList(game.timedModels, "Timed Models")
		guiList(game.weapons, "Weapon Models")
		guiList(game.clumps, "Clump Models")
		guiList(game.animClumps, "Animated Clump Models")
		guiList(game.peds, "Peds")
		guiList(game.vehicles, "Vehicles")
		ImGui.Separator()
		guiList(game.instances, "Instances")
		guiList(game.instances, "Visible Instances", true)
		guiList(game.zones, "Zones")
		guiList(game.mapZones, "Map Zones")
		guiList(game.cullZones, "Cull/Attrib Zones")
		guiList(game.audioZones, "Audio Zones")
		guiList(game.occluders, "Occluders")
		guiList(game.garages, "Garages")
		guiList(game.entryExits, "Entry/Exits")
		guiList(game.pickups, "Pickups")
		guiList(game.carGenerators, "Car Generators")
		guiList(game.stuntJumps, "Stunt Jumps")
		guiList(game.timecycMods, "TimeCycle Mods")
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
				guiTransformation()
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

function guiScenes(g)
	if not ImGui.TreeNode("Scenes") then return end
	for _, scene in ipairs(g.scenes) do
		scene:imguiDraw()
	end
	ImGui.TreePop()
end

function guiTransformation(g)
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

function guiRendering(g)
	if ImGui.RadioButton("Draw HD", viewer.lodMode == 1) then viewer.lodMode = 1 end
	ImGui.SameLine()
	if ImGui.RadioButton("Draw LOD", viewer.lodMode == 2) then viewer.lodMode = 2 end
	ImGui.SameLine()
	if ImGui.RadioButton("Draw Normal", viewer.lodMode == 3) then viewer.lodMode = 3 end
	ImGui.DragFloatX("LOD multiplier", viewer, "lodMult", 0.05, 0.5, 10.0)
	viewer.drawCollision, _ = ImGui.Checkbox("Draw Collision", viewer.drawCollision)

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

function guiCamera(cam)
	local p = cam.position
	local xyz, used = ImGui.InputFloat3("Position", {p.x, p.y, p.z})
	cam.position = rw.V3d(xyz[1], xyz[2], xyz[3])

	p = cam.target
	xyz, used = ImGui.InputFloat3("Target", {p.x, p.y, p.z})
	cam.target = rw.V3d(xyz[1], xyz[2], xyz[3])

	cam.fov, _ = ImGui.DragFloat("FOV", cam.fov, 0.1, 10, 130)
end




-- object GUIs

function ColModel:imguiDraw()
	if ImGui.TreeNode("Col Model") then
		local c = self.boundingSphere.center
		local r = self.boundingSphere.radius
		local values, used = ImGui.InputFloat4("Bounding Sphere", {c.x, c.y, c.z, r})

		local min = self.boundingBox.min
		local max = self.boundingBox.max
		values, used = ImGui.InputFloat3("Bounding Box Min", {min.x, min.y, min.z})
		values, used = ImGui.InputFloat3("Bounding Box Max", {max.x, max.y, max.z})
		-- TODO

		ImGui.TreePop()
	end
end


function AtomicModel:imguiTitle()
	return true, tostring(self.id) .. "\t" .. self.modelName
end

function AtomicModel:imguiDraw()
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

	if self.colModel then
		self.colModel:imguiDraw()
	end

	if ImGui.Button("View") then
		game:LoadAtomics(self)
		clumpEd:openClump(self.rwClump, self.txd.rwTxd)
		if clumpEd.cam then clumpEd.cam:jumpTo(rw.V3d(0,0,0)) end
		mode = "clump"
	end
	ImGui.PopID()
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
		if self.lodInst then
			guiListItem(self.lodInst)
		end
		if self.children then
			for _, c in ipairs(self.children) do
				guiListItem(c)
			end
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



-- helpers

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

function guiListItem(v, cullhidden)
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

function guiList(list, label, cullhidden)
	if #list < 1 then
		return
	end
	local open = ImGui.TreeNode(label)
	if ImGui.IsItemHovered() then
		if ImGui.IsMouseClicked(2) then
			NewListWindow(list, label)
		end
	end
	if not open then return end
	for _, v in ipairs(list) do
		guiListItem(v, cullhidden)
	end
	ImGui.TreePop()
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

