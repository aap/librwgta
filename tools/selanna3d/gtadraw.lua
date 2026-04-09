function gta:LoadClump(mdl)
	if mdl.rwClump then return mdl.rwClump end
	local txd = self:GetTexDictionary(mdl.txd)
	rw.setCurrentTexDict(txd)
	self:GetModelClump(mdl)
	for a in mdl.rwClump:atomics() do
		hideDamagedLOD(a)
	--	printAtomic(a)
	end
	return mdl.rwClump
end

function gta:LoadAtomics(mdl)
	if mdl.rwAtomics then return end
	local txd = self:GetTexDictionary(mdl.txd)
	rw.setCurrentTexDict(txd)
	self:GetModelClump(mdl)
	if not mdl.rwClump then return end
	mdl.rwAtomics = {}
	for a in mdl.rwClump:atomics() do
		local nodename = a:getFrame():getName()
		local name, lod = nodename:match("^(.-)_[Ll](%d+)$")
		if not name then
			name = nodename
			lod = 0
		end
		mdl.rwAtomics[lod+1] = a:clone()
	end
end

function gta:FindFileFuzzy(name)
	local lname = name:lower()
	for _, f in pairs(self.files) do
		if f.fileName:lower():find(lname) then
			return f
		end
	end
	return nil
end

function Instance:UpdateRW()
	local pos = tV3d(self.position)
	local rot = tQuat(self.rotation):conj()
	local frm = self.rwAtomic:getFrame()
	frm:rotateQuat(rot, rw.COMBINEREPLACE)
	frm:translate(pos, rw.COMBINEPOSTCONCAT)
end

function gta:Instantiate(inst)
	if inst.rwAtomic then 
		inst.show = true
		return
	end
-- quick sanity check
	if not inst.mdl.lodDist1 then return end

--	if inst.mdl.lodDist1 > 300 then return end
	self:LoadAtomics(inst.mdl)
	if not inst.mdl.rwAtomics or
	   #inst.mdl.rwAtomics < 1 then
		return
	end
	local atomic = inst.mdl.rwAtomics[1]:clone()
	local frm = rw.FrameCreate()
	atomic:setFrame(frm)
	inst.rwAtomic = atomic
	inst:UpdateRW()
	inst.show = true
end

function gta:InstantiateScene(scene, deep)
	if type(scene) == "string" then
		scene = self.scenesByName[scene:lower()]
		if not scene then return end
	end
	for _, inst in pairs(scene.instances) do
		self:Instantiate(inst)
	end
	if deep then
		for _, scn in ipairs(scene.streamed) do
			self:InstantiateScene(scn, deep)
		end
	end
	scene.showScene = true
end

function gta:InstantiateStreamedScene(name)
	local ipl = self.streamIplsByName[name:lower()]
	if not ipl then return end
	local scene = ipl.streamingInfo.scene
	for _, inst in pairs(scene.instances) do
		self:Instantiate(inst)
	end
	scene.showScene = true
end


function gta:HideScene(scene, deep)
	if type(scene) == "string" then
		local file = self:FindFileFuzzy(scene)
		if file == nil then return end
		scene = file.scene
	end
	if deep then
		for _, scn in ipairs(scene.streamed) do
			self:HideScene(scn, deep)
		end
	end
	scene.showScene = false
end

function AtomicModel:IsTimeInRange(hour)
	if self.timeOn > self.timeOff then
		return hour >= self.timeOn or hour < self.timeOff
	else
		return hour >= self.timeOn and hour < self.timeOff
	end
end

function Instance:GetMatrix() return self.rwAtomic:getFrame():getLTM() end

function Instance:IsOnScreen()
	if not self.rwAtomic then return true end
	local matrix = self:GetMatrix()
	return not col or activeCam:isSphereVisible(col.boundingSphere, self:GetMatrix())
end

function Instance:DrawColModel()
	if not self.mdl.colModel then return end
	gta.renderColModelWire(self.mdl.colModel, self:GetMatrix(), false)
end

function gta:IsInstanceVisible(inst, forceshow)
	if forceshow then return true end
	if not (inst.show and inst.sourceFile.scene.showScene) then return false end
	local mdl = inst.mdl

	local dist = activeCam:distanceTo(tV3d(inst.position))
	local isLod = inst.children or mdl.lodDist1 > 300
	-- TODO
	if viewer.lodMode == 1 then
		-- render HD
		if isLod then return false end
	elseif viewer.lodMode == 2 then
		-- render LOD
		if not isLod then return false end
	else
		if dist > mdl.lodDist1*viewer.lodMult then
			return false
		end
	end

	if mdl.timeOn then
		if not mdl:IsTimeInRange(self.hour) then return false end
	end
	if not inst:IsOnScreen() then return false end

	return true
end

function gta:DrawInstance(inst, forceshow)
	if not self:IsInstanceVisible(inst, forceshow) then return nil end

	local dist = activeCam:distanceTo(tV3d(inst.position))
	if dist < 100 and viewer.drawCollision then
		inst:DrawColModel()
	end

	if (inst.mdl.flags.bits & (4|8)) ~= 0 then
		return inst.rwAtomic
	end
	inst.rwAtomic:render()
	return nil
end

