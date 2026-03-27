function gta:LoadClump(mdl)
	if mdl.rwClump then return mdl.rwClump end
	local txd = self:GetTexDictionary(mdl.txd)
	rw.setCurrentTexDict(txd)
	mdl.rwClump = self:GetModelClump(mdl)
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
	mdl.rwClump = self:GetModelClump(mdl)
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

function gta:Instantiate(inst)
	if inst.rwAtomic then 
		inst.show = true
		return
	end
	if inst.mdl.lodDist1 > 300 then return end
	self:LoadAtomics(inst.mdl)
	if not inst.mdl.rwAtomics then
		return
	end
	local atomic = inst.mdl.rwAtomics[1]:clone()
	local frm = rw.FrameCreate()
	local pos = tV3d(inst.position)
	local rot = tQuat(inst.rotation)
	frm:rotateQuat(rot, rw.COMBINEREPLACE)
	frm:translate(pos, rw.COMBINEPOSTCONCAT)
	atomic:setFrame(frm)
	inst.rwAtomic = atomic
	inst.show = true
end

function gta:InstantiateScene(name)
	local file = self:FindFileFuzzy(name)
	if file == nil then return end
	for _, inst in pairs(self.instances) do
		if inst.sourceFile == file then
			self:Instantiate(inst)
		end
	end
print("done instancing")
	file.showScene = true
end

function gta:HideScene(name)
	local file = self:FindFileFuzzy(name)
	if file == nil then return end
	file.showScene = false
end

function Building:IsTimeInRange(hour)
	if self.timeOn > self.timeOff then
		return hour >= self.timeOn or hour < self.timeOff
	else
		return hour >= self.timeOn and hour < self.timeOff
	end
end

function gta:DrawInstance(inst)
	if not (inst.show and inst.sourceFile.showScene) then return nil end
	local mdl = inst.mdl
	if mdl.timeOn then
		if not mdl:IsTimeInRange(self.hour) then
			return nil
		end
	end
	if (mdl.flags & (4|8)) ~= 0 then
		return inst.rwAtomic
	end
	gta.RenderAtomic(inst.rwAtomic)
	return nil
end

