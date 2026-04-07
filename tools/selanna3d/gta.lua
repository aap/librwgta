function isspace(c)
	return c == ' ' or c == '\t'
end

function split(line)
    local fields = {}
    local i = 1
    local len = #line

    while i <= len do
        while i <= len and isspace(line:sub(i, i)) do
            i = i + 1
        end
        if i > len then break end

        if line:sub(i, i) == '"' then
            i = i + 1
            local field = {}
            while i <= len do
                local c = line:sub(i, i)
                if c == '"' then
                    i = i + 1
                    break
                end
                field[#field + 1] = c
                i = i + 1
            end
            fields[#fields + 1] = table.concat(field)
        else
            local start = i
            while i <= len and not isspace(line:sub(i, i)) do
                i = i + 1
            end
            fields[#fields + 1] = line:sub(start, i - 1)
        end
    end
    return fields
end

TokenStream = {}
TokenStream.__index = TokenStream

function TokenStream.make(line)
	local fields = split(line)
	local t = { fields = fields, i = 1 }
	setmetatable(t, TokenStream)
	return t
end

function TokenStream:next()
	local f = self.fields[self.i]
	self.i = self.i + 1
	return f
end

function TokenStream:nextInt()
	return tonumber(self:next())
end

function TokenStream:nextFloat()
	return tonumber(self:next())
end

function TokenStream:nextXY()
	local x = self:nextFloat()
	local y = self:nextFloat()
	return { x = x, y = y }
end

function TokenStream:nextXYZ()
	local x = self:nextFloat()
	local y = self:nextFloat()
	local z = self:nextFloat()
	return { x = x, y = y, z = z }
end

function TokenStream:nextXYZW()
	local x = self:nextFloat()
	local y = self:nextFloat()
	local z = self:nextFloat()
	local w = self:nextFloat()
	return { x = x, y = y, z = z, w = w }
end

function TokenStream:nextRGBA()
	local r = self:nextFloat()
	local g = self:nextFloat()
	local b = self:nextFloat()
	local a = self:nextFloat()
	return { r = r, g = g, b = b, a = a }
end

BinaryStream = {}
BinaryStream.__index = BinaryStream

function BinaryStream.make(data)
	local s = { data = data, pos = 1 }
	setmetatable(s, BinaryStream)
	return s
end

function BinaryStream:nextU32()
	local n = string.unpack("<I4", self.data, self.pos)
	self.pos = self.pos + 4
	return n
end

function BinaryStream:nextI32()
	local n = string.unpack("<i4", self.data, self.pos)
	self.pos = self.pos + 4
	return n
end

function BinaryStream:nextI16()
	local n = string.unpack("<i2", self.data, self.pos)
	self.pos = self.pos + 2
	return n
end

function BinaryStream:nextFloat()
	local n = string.unpack("<f", self.data, self.pos)
	self.pos = self.pos + 4
	return n
end

-- identical to text tokens actually....
function BinaryStream:nextXYZ()
	local x = self:nextFloat()
	local y = self:nextFloat()
	local z = self:nextFloat()
	return { x = x, y = y, z = z }
end
function BinaryStream:nextXYZW()
	local x = self:nextFloat()
	local y = self:nextFloat()
	local z = self:nextFloat()
	local w = self:nextFloat()
	return { x = x, y = y, z = z, w = w }
end


function addString(line, s)
	if line == "" then
		return s
	else
		return line .. ", " .. s
	end
end
function addInt(line, i)
	return addString(line, tostring(i))
end
function addHex(line, i)
	return addString(line, string.format("%x", (i)))
end
function addFloat(line, f)
	return addString(line, tostring(f))
end
function addXY(line, xy)
	line = addFloat(line, xy.x)
	return addFloat(line, xy.y)
end
function addXYZ(line, xyz)
	line = addFloat(line, xyz.x)
	line = addFloat(line, xyz.y)
	return addFloat(line, xyz.z)
end
function addXYZW(line, xyzw)
	line = addFloat(line, xyzw.x)
	line = addFloat(line, xyzw.y)
	line = addFloat(line, xyzw.z)
	return addFloat(line, xyzw.w)
end
function addRGBA(line, rgba)
	line = addFloat(line, rgba.r)
	line = addFloat(line, rgba.g)
	line = addFloat(line, rgba.b)
	return addFloat(line, rgba.a)
end


Building = {}
Building.__index = Building
function Building.make()
	return setmetatable({}, Building)
end

Instance = {}
Instance.__index = Instance
function Instance.make()
	return setmetatable({}, Instance)
end

Scene = {}
Scene.__index = Scene
function Scene.make(name, file)
	local scene = setmetatable({ name = name, file = file,
		instances = {}, streamed = {} }, Scene)
	file.scene = scene
	return scene
end

--gta = {} -- done in C++
gta.__index = gta

gta.GameIII = 0
gta.GameVC = 1
gta.GameSA = 2

-- model types
gta.MT_ATOMIC = 1
gta.MT_CLUMP = 2
gta.MT_PED = 3
gta.MT_VEHICLE = 4
gta.MT_WEAPON = 5

function gta.make(game, gameDir)
	local g = {}
	setmetatable(g, gta)
	g.game = game
	g.gameDir = gameDir
	g.files = {}
	g.filesOrdered = {}
	g.imagePath = "none"

	g.scenes = {}
	g.scenesByName = {}

	g.genericTxds = {}
	g.cdImageFiles = {}
	g.streamedFiles = {}
	g.streamIplsByName = {}

	-- IDE
	g.modelsById = {}
	g.modelsByName = {}
	g.txdsByName = {}
	g.buildings = {}
	g.timedBuildings = {}
	g.clumps = {}
	g.animClumps = {}
	g.peds = {}
	g.vehicles = {}
	g.weapons = {}
	g.effects = {}
	g.txdParents = {}

	-- IPL
	g.instances = {}
	g.cullZones = {}
	g.zones = {}
	g.mapZones = {}
	g.occluders = {}
	g.garages = {}
	g.entryExits = {}
	g.pickups = {}
	g.carGenerators = {}
	g.stuntJumps = {}
	g.timecycMods = {}
	g.audioZones = {}

	g.pathSegments = {}

	g.hour = 12
	return g
end

function gta:GetFileFs(file)
	file = file:gsub("\\", "/")
	if self.files[file] then
		return self.files[file]
	end
	local path = rw.makePath(self.gameDir .. "/" .. file)
	local f = { fileName = file, fullPath = path, modified = false }
	self.files[file] = f
	table.insert(self.filesOrdered, f)
	return f
end

function gta:SwitchGameDir(dir)
	self.gameDir = dir
	for i, f in pairs(self.files) do
		f.fullPath = rw.makePath(self.gameDir .. "/" .. f.fileName)
	end
end

function gta:MkDir(dir)
	os.execute("mkdir -p " .. dir)
end

function gta:PopulateGameDir()
	local dirs = {}
	for _, f in pairs(self.files) do
		f.fileName = f.fileName:lower()
		f.fullPath = rw.makePath(self.gameDir .. "/" .. f.fileName)
		local dir = f.fullPath:match("^(.*)/[^/]*$")
		if dir and dir ~= "" then
			dirs[dir] = true
		end
	end
	for dir in pairs(dirs) do
		gta:MkDir(dir)
	end
end

function gta:AddScene(name, file)
	local scene = Scene.make(name, file)
	table.insert(self.scenes, scene)
	self.scenesByName[name:lower()] = scene
	return scene
end

function gta:AddModel(obj, tab, type)
	obj.sourceFile = self.currentFile
	obj.type = type
	table.insert(tab, obj)
	self.modelsById[obj.id] = obj
	self.modelsByName[obj.modelName:lower()] = obj
end

function gta:AddBuilding(obj)
	self:AddModel(obj, self.buildings, gta.MT_ATOMIC)
end

function gta:AddTimedBuilding(obj)
	self:AddModel(obj, self.timedBuildings, gta.MT_ATOMIC)
end

function gta:AddAnimClump(obj)
	self:AddModel(obj, self.animClumps, gta.MT_CLUMP)
end

function gta:AddClump(obj)
	self:AddModel(obj, self.clumps, gta.MT_CLUMP)
end

function gta:AddPed(obj)
	self:AddModel(obj, self.peds, gta.MT_PED)
end

function gta:AddVehicle(obj)
	self:AddModel(obj, self.vehicles, gta.MT_VEHICLE)
end

function gta:AddWeapon(obj)
	self:AddModel(obj, self.weapons, gta.MT_WEAPON)
end

function gta:AddEffect(e)
	e.sourceFile = self.currentFile
	table.insert(self.effects, e)
end

function gta:AddInstance(inst, file)
	inst.sourceFile = file
	inst.instId = #self.instances + 1
	table.insert(self.instances, inst)

	local scene = inst.sourceFile.scene
	table.insert(scene.instances, inst)
end

function gta:AddCullZone(zone)
	zone.sourceFile = self.currentFile
	table.insert(self.cullZones, zone)
end

function gta:AddZone(zone)
	zone.sourceFile = self.currentFile
	table.insert(self.zones, zone)
end

function gta:AddMapZone(zone)
	zone.sourceFile = self.currentFile
	table.insert(self.mapZones, zone)
end

function gta:AddOccluder(occl)
	occl.sourceFile = self.currentFile
	table.insert(self.occluders, occl)
end

function gta:AddPathSegment(seg)
	local obj = self.modelsById[seg.id]
	seg.sourceFile = self.currentFile
	table.insert(self.pathSegments, seg)
end


function gta:AddTxdParent(txdp)
	txdp.sourceFile = self.currentFile
	table.insert(self.txdParents, txdp)
end

function gta:AddGarage(grge)
	grge.sourceFile = self.currentFile
	table.insert(self.garages, grge)
end

function gta:AddEntryExit(enex)
	enex.sourceFile = self.currentFile
	table.insert(self.entryExits, enex)
end

function gta:AddPickup(pick)
	pick.sourceFile = self.currentFile
	table.insert(self.pickups, pick)
end

function gta:AddCarGenerator(cargen, file)
	cargen.sourceFile = file
	table.insert(self.carGenerators, cargen)

	local scene = cargen.sourceFile.scene
	if not scene.carGenerators then scene.carGenerators = {} end
	table.insert(scene.carGenerators, cargen)
end

function gta:AddStuntJump(jump)
	jump.sourceFile = self.currentFile
	table.insert(self.stuntJumps, jump)
end

function gta:AddTimeCycleMod(tcyc)
	tcyc.sourceFile = self.currentFile
	table.insert(self.timecycMods, tcyc)
end

function gta:AddAudioZone(auzo)
	auzo.sourceFile = self.currentFile
	table.insert(self.audioZones, auzo)
end

function gta:LinkStreamingInfo()
	for _, s in pairs(self.streamedFiles) do
		if s.ext == "DFF" then
			local mdl = self.modelsByName[s.name:lower()]
			if mdl then
				mdl.streamingInfo = s
			else
--				print("did not find model", s.name)
			end
		elseif s.ext == "TXD" then
			local txd = self.txdsByName[s.name:lower()]
			if txd then
				txd.streamingInfo = s
			else
--				print("did not find txd", s.name)
			end
		elseif s.ext == "IPL" then
			local ipl = { streamingInfo = s }
			self.streamIplsByName[s.name:lower()] = ipl
		end
	end
end

function gta:AddTxdSlot(name)
	local txd = { name = name, refCount = 1 }
	self.txdsByName[name] = txd
	return txd
end

function gta:GetTxdSlot(name)
	local txd = self.txdsByName[name]
	if not txd then
		txd = self:AddTxdSlot(name)
	end
	return txd
end

function gta:GetStreamBuffer(si)
	local f = io.open(si.container.fullPath, "rb")
	if not f then
		print("Could not open file: " .. si.container.fullPath)
		return nil
	end
	f:seek("set", si.offset*2048)
	local data = f:read(si.size*2048)
	f:close()
	return BinaryStream.make(data)
end

function gta:FinishLoading()
	self.currentFile = nil

	-- link TXDs to models
	local generic = self:AddTxdSlot('generic')
	for id, mdl in pairs(self.modelsById) do
		local name = mdl.txdName:lower()
		local txd = self:GetTxdSlot(name)
		mdl.txd = txd
		txd.refCount = txd.refCount+1
	end

	-- link TXD parents
	for _, txdp in pairs(self.txdParents) do
		local txd = self:GetTxdSlot(txdp.txdName:lower())
		local parent = self:GetTxdSlot(txdp.parentName:lower())
		txd.parent = parent
	end

	-- load CD images
	for _, f in pairs(self.cdImageFiles) do
		self:ReadCdImage(f.fileName)
	end
	self:LinkStreamingInfo()

	-- load generic textures
	generic.rwTxd = rw.TexDictionaryCreate()
	for _, f in pairs(self.genericTxds) do
		local txd = rw.readTexDictFile(f.fullPath, 0, 0)
		sk.ConvertTexDict(txd)
		for t in txd:textures() do
			generic.rwTxd:addFront(t)
		end
		txd:destroy()
	end

	-- link models to instances
	-- TODO: put this into AddInstance?
	for i, inst in pairs(self.instances) do
		inst.mdl = self.modelsById[inst.id]
	end
end

function gta:GetModelClump(mdl)
	if mdl.rwClump then return mdl.rwClump end
	local si = mdl.streamingInfo
	if not si or si.ext ~= "DFF" then return nil end
	rw.ImageSetSearchPath(self.imagePath)
	mdl.rwClump = rw.readClumpFile(si.container.fullPath, si.offset*2048, si.size*2048)
	if mdl.rwClump then
		sk.ConvertClump(mdl.rwClump)
		for a in mdl.rwClump:atomics() do
			gta.SetupAtomicPipelines(a)
		end
	end
	return mdl.rwClump
end

function gta:GetTexDictionary(txd)
	if txd.rwTxd then return txd.rwTxd end
	if txd.parent then
		self:GetTexDictionary(txd.parent)
	end
	local si = txd.streamingInfo
	if not si or si.ext ~= "TXD" then
		txd.rwTxd = rw.TexDictionaryCreate()
		return nil
	end
	txd.rwTxd = rw.readTexDictFile(si.container.fullPath, si.offset*2048, si.size*2048)
	sk.ConvertTexDict(txd.rwTxd)
	if txd.parent then
		gta.TxdSetParent(txd.rwTxd, txd.parent.rwTxd)
	end
	return txd.rwTxd
end


FlagSet = {}
FlagSet.__index = FlagSet

function FlagSet.make(bits, desc)
	local flags = { bits = bits, desc=desc }
	return flags
end

objflagsIII = {}
objflagsIII[1] = "Normal Cull"
objflagsIII[2] = "No Fade"
objflagsIII[4] = "Draw Last"
objflagsIII[8] = "Additive Blend"
objflagsIII[0x10] = "Is Subway"
objflagsIII[0x20] = "Ignore Light"
objflagsIII[0x40] = "No Z-Write"

objflagsVC = {}
objflagsVC[1] = "Wet Road Effect"
objflagsVC[2] = "No Fade"
objflagsVC[4] = "Draw Last"
objflagsVC[8] = "Additive Blend"
objflagsVC[0x10] = "Is Subway"
objflagsVC[0x20] = "Ignore Light"
objflagsVC[0x40] = "No Z-Write"
objflagsVC[0x80] = "No Shadows"
objflagsVC[0x100] = "Ingore Draw Dist"
objflagsVC[0x200] = "Code Glass"
objflagsVC[0x400] = "Artist Glass"

objflagsSA = {}
atmflagsSA = {}
clpflagsSA = {}
objflagsSA[4] = "Draw Last"
objflagsSA[8] = "Additive Blend"
objflagsSA[0x40] = "No Z-Write"
objflagsSA[0x80] = "No Shadows"
objflagsSA[0x200000] = "No Backface Culling"
for i=0,31 do
	local bit = 1<<i
	if objflagsSA[bit] then
		atmflagsSA[bit] = objflagsSA[bit]
		clpflagsSA[bit] = objflagsSA[bit]
	end
end
atmflagsSA[1] = "Wet Road Effect"
atmflagsSA[0x200] = "Code Glass"
atmflagsSA[0x400] = "Artist Glass"
atmflagsSA[0x800] = "Garage Door"
atmflagsSA[0x2000] = "Tree"
atmflagsSA[0x4000] = "Palm Tree"
atmflagsSA[0x8000] = "Don't collide with Flyer"
atmflagsSA[0x80000] = "Wet Only"
atmflagsSA[0x100000] = "Tag"
atmflagsSA[0x400000] = "No Cover"
clpflagsSA[0x20] = "Door"

objflags = {}
objflags[gta.GameIII] = objflagsIII
objflags[gta.GameVC] = objflagsVC
objflags[gta.GameSA] = atmflagsSA

instflagsSA = {}
instflagsSA[1] = "Unimportant"
instflagsSA[4] = "Under Water"
instflagsSA[8] = "Tunnel"
instflagsSA[0x10] = "Tunnel Transition"


-- file loading

function gta:ReadNothing(line)
end

function gta:ReadObjLine(line)
	local t = TokenStream.make(line)
	local obj = Building.make()
	obj.id = t:nextInt()
	obj.modelName = t:next()
	obj.txdName = t:next()
	if self.game == gta.GameSA and #t.fields == 5 then
		obj.numAtomics = 1
	else
		obj.numAtomics = t:nextInt()
	end
	for i = 1,obj.numAtomics do
		obj["lodDist" .. i] = t:nextFloat()
	end
	obj.flags = FlagSet.make(t:nextInt(), objflags[self.game])
	self:AddBuilding(obj)
end

function gta:WriteObjLine(obj)
	local line = ""
	line = addInt(line, obj.id)
	line = addString(line, obj.modelName)
	line = addString(line, obj.txdName)
	if self.game < gta.GameSA or obj.numAtomics > 1 then
		line = addInt(line, obj.numAtomics)
	end
	for i = 1,obj.numAtomics do
		line = addFloat(line, obj["lodDist" .. i])
	end
	line = addInt(line, obj.flags.bits)
	return line
end


function gta:ReadTObjLine(line)
	local t = TokenStream.make(line)
	local obj = Building.make()
	obj.id = t:nextInt()
	obj.modelName = t:next()
	obj.txdName = t:next()
	if self.game == gta.GameSA and #t.fields == 7 then
		obj.numAtomics = 1
	else
		obj.numAtomics = t:nextInt()
	end
	for i = 1,obj.numAtomics do
		obj["lodDist" .. i] = t:nextFloat()
	end
	obj.flags = FlagSet.make(t:nextInt(), objflags[self.game])
	obj.timeOn = t:nextInt()
	obj.timeOff = t:nextInt()
	self:AddTimedBuilding(obj)
end

function gta:WriteTObjLine(obj)
	local line = ""
	line = addInt(line, obj.id)
	line = addString(line, obj.modelName)
	line = addString(line, obj.txdName)
	if self.game < gta.GameSA or obj.numAtomics > 1 then
		line = addInt(line, obj.numAtomics)
	end
	for i = 1,obj.numAtomics do
		line = addFloat(line, obj["lodDist" .. i])
	end
	line = addInt(line, obj.flags.bits)
	line = addInt(line, obj.timeOn)
	line = addInt(line, obj.timeOff)
	return line
end

function gta:ReadAnimLine(line)
	local t = TokenStream.make(line)
	local obj = {}
	obj.id = t:nextInt()
	obj.modelName = t:next()
	obj.txdName = t:next()
	obj.animName = t:next()
	obj.lodDist = t:nextFloat()
	obj.flags = FlagSet.make(t:nextInt(), clpflagsSA)
	self:AddAnimClump(obj)
end

function gta:WriteAnimLine(obj)
	local line = ""
	line = addInt(line, obj.id)
	line = addString(line, obj.modelName)
	line = addString(line, obj.txdName)
	line = addString(line, obj.animName)
	line = addFloat(line, obj.lodDist)
	line = addInt(line, obj.flags.bits)
	return line
end


function gta:ReadHierLine(line)
	local t = TokenStream.make(line)
	local clp = {}
	clp.id = t:nextInt()
	clp.modelName = t:next()
	clp.txdName = t:next()
	self:AddClump(clp)
end

function gta:WriteHierLine(clp)
	local line = ""
	line = addInt(line, clp.id)
	line = addString(line, clp.modelName)
	line = addString(line, clp.txdName)
	return line
end


function gta:ReadPedLine(line)
	local t = TokenStream.make(line)
	local ped = {}
	ped.id = t:nextInt()
	ped.modelName = t:next()
	ped.txdName = t:next()
	ped.pedType = t:next()
	ped.pedStats = t:next()
	ped.animGroup = t:next()
	ped.carMask = tonumber("0x" .. t:next())
	if self.game > gta.GameIII then
		if self.game == gta.GameSA then
			ped.flags = tonumber("0x" .. t:next())
		end
		ped.animFile = t:next()
		ped.radio1 = t:nextInt()
		ped.radio2 = t:nextInt()
		if self.game == gta.GameSA then
			ped.audioType = t:next()
			ped.firstVoice = t:next()
			ped.lastVoice = t:next()
		end
	end
	self:AddPed(ped)
end

function gta:WritePedLine(ped)
	local line = ""
	line = addInt(line, ped.id)
	line = addString(line, ped.modelName)
	line = addString(line, ped.txdName)
	line = addString(line, ped.pedType)
	line = addString(line, ped.pedStats)
	line = addString(line, ped.animGroup)
	line = addHex(line, ped.carMask)
	if self.game > gta.GameIII then
		if self.game == gta.GameSA then
			line = addHex(line, ped.flags)
		end
		line = addString(line, ped.animFile)
		line = addInt(line, ped.radio1)
		line = addInt(line, ped.radio2)
		if self.game == gta.GameSA then
			line = addString(line, ped.audioType)
			line = addString(line, ped.firstVoice)
			line = addString(line, ped.lastVoice)
		end
	end
	return line
end

function gta:ReadVehicleLine(line)
	local t = TokenStream.make(line)
	local veh = {}
	veh.id = t:nextInt()
	veh.modelName = t:next()
	veh.txdName = t:next()
	veh.vehicleType = t:next()
	veh.handlingName = t:next()
	veh.gameName = t:next()
	if self.game > gta.GameIII then
		veh.animFile = t:next()
	end
	veh.rating = t:next()
	veh.freq = t:nextInt()
	veh.levels = t:nextInt()
	veh.compRules = tonumber("0x" .. t:next())
	if self.game < gta.GameSA then
		if veh.vehicleType == "car" then
			veh.wheelId = t:nextInt()
			veh.wheelScale = t:nextFloat()
		elseif veh.vehicleType == "plane" then
			veh.planeLodId = t:nextInt()
		elseif veh.vehicleType == "bike" then
			veh.steerAngle = t:nextInt()
			veh.wheelScale = t:nextFloat()
		end
	else
		if veh.vehicleType == "car" or
		   veh.vehicleType == "mtruck" or
		   veh.vehicleType == "quad" or
		   veh.vehicleType == "heli" or
		   veh.vehicleType == "plane" or
		   veh.vehicleType == "trailer" then
			veh.wheelId = t:nextInt()
			veh.wheelScale = t:nextFloat()
			veh.wheelScaleRear = t:nextFloat()
			veh.wheelClass = t:nextInt()
		elseif veh.vehicleType == "bike" or
		       veh.vehicleType == "bmx" then
			veh.steerAngle = t:nextInt()
			veh.wheelScale = t:nextFloat()
			veh.wheelScaleRear = t:nextFloat()
			veh.wheelClass = t:nextInt()
		end
	end
	self:AddVehicle(veh)
end

function gta:WriteVehicleLine(veh)
	local line = ""
	line = addInt(line, veh.id)
	line = addString(line, veh.modelName)
	line = addString(line, veh.txdName)
	line = addString(line, veh.vehicleType)
	line = addString(line, veh.handlingName)
	line = addString(line, veh.gameName)
	if self.game > gta.GameIII then
		line = addString(line, veh.animFile)
	end
	line = addString(line, veh.rating)
	line = addInt(line, veh.freq)
	line = addInt(line, veh.levels)
	line = addHex(line, veh.compRules)
	if self.game < gta.GameSA then
		if veh.vehicleType == "car" then
			line = addInt(line, veh.wheelId)
			line = addFloat(line, veh.wheelScale)
		elseif veh.vehicleType == "plane" then
			line = addInt(line, veh.planeLodId)
		elseif veh.vehicleType == "bike" then
			line = addInt(line, veh.steerAngle)
			line = addFloat(line, veh.wheelScale)
		end
	else
		if veh.vehicleType == "car" or
		   veh.vehicleType == "mtruck" or
		   veh.vehicleType == "quad" or
		   veh.vehicleType == "heli" or
		   veh.vehicleType == "plane" or
		   veh.vehicleType == "trailer" then
			line = addInt(line, veh.wheelId)
			line = addFloat(line, veh.wheelScale)
			line = addFloat(line, veh.wheelScaleRear)
			line = addInt(line, veh.wheelClass)
		elseif veh.vehicleType == "bike" or
		       veh.vehicleType == "bmx" then
			line = addInt(line, veh.steerAngle)
			line = addFloat(line, veh.wheelScale)
			line = addFloat(line, veh.wheelScaleRear)
			line = addInt(line, veh.wheelClass)
		end
	end
	return line
end

function gta:ReadWeaponLine(line)
	local t = TokenStream.make(line)
	local weap = {}
	weap.id = t:nextInt()
	weap.modelName = t:next()
	weap.txdName = t:next()
	weap.animFile = t:next()
	t:next()	-- unused - 1
	weap.lodDist = t:nextFloat()
	self:AddWeapon(weap)
end

function gta:WriteWeaponLine(weap)
	local line = ""
	line = addInt(line, weap.id)
	line = addString(line, weap.modelName)
	line = addString(line, weap.txdName)
	line = addString(line, weap.animFile)
	line = addInt(line, 1)
	line = addFloat(line, weap.loadDist)
	return line
end

function gta:ReadPathLine(line)
	local t = TokenStream.make(line)

	if not self._pathSegment then
		self._pathSegment = { }
		local seg = self._pathSegment
		if self.game == gta.GameIII then
			if t:next() == "ped" then
				seg.type = 0
			else
				seg.type = 1
			end
		else
			seg.type = t:nextInt()
		end
		seg.id = t:nextInt()
		if seg.id >= 0 then
			seg.name = t:next()
		end
		seg.nodes = {}
	else
		local node = {}
		node.type = t:nextInt()
		node.link = t:nextInt()
		node.cross = t:nextInt()
		node.pos = t:nextXYZ()
		node.width = t:nextFloat()
		node.lanesIn = t:nextInt()
		node.lanesOut = t:nextInt()
		if self.game > gta.GameIII then
			node.speed = t:nextInt()
			node.flags = t:nextInt()
			node.density = t:nextFloat()
		end
		if self.game == gta.GameSA then
			node.special = t:nextInt()
		end

		local seg = self._pathSegment
		table.insert(seg.nodes, node)
		if #seg.nodes == 12 then
			self:AddPathSegment(seg)
			self._pathSegment = nil
		end
	end
end

local pathtypestr = { "ped", "car" }

function gta:WritePathNodeLine(node)
	local line = ""
	line = addInt(line, node.type)
	line = addInt(line, node.link)
	line = addInt(line, node.cross)
	line = addXYZ(line, node.pos)
	line = addFloat(line, node.width)
	line = addInt(line, node.lanesIn)
	line = addInt(line, node.lanesOut)
	if self.game > gta.GameIII then
		line = addInt(line, node.speed)
		line = addFloat(line, node.flags)
		line = addFloat(line, node.density)
	end
	if self.game == gta.GameSA then
		line = addInt(line, node.special)
	end
	return "\t" .. line
end

function gta:WritePathSegment(seg)
	local line = ""
	if self.game == gta.GameIII then
		line = addString(line, pathtypestr[seg.type+1])
	else
		line = addInt(line, seg.type)
	end
	line = addInt(line, seg.id)
	if seg.name then
		line = addString(line, seg.name)
	end
	for i = 1,12 do
		line = line .. "\n"
		line = line .. self:WritePathNodeLine(seg.nodes[i])
	end
	return line
end

local FX_LIGHT = 0
local FX_PARTICLE = 1
local FX_LOOKATPOINT = 2
-- VC
local FX_PEDQUEUE = 3
local FX_SUNGLARE = 4
-- SA
local FX_INTERIOR = 5
local FX_ENTRYEXIT = 6
local FX_ROADSIGN = 7
local FX_TRIGGERPOINT = 8
local FX_COVERPOINT = 9
local FX_ESCALATOR = 10

function gta:Read2dfxLine(line)
	local t = TokenStream.make(line)

	local e = {}
	e.id = t:nextInt()
	e.position = t:nextXYZ(t)
	if self.game < gta.GameSA then
		e.color = t:nextRGBA(t)
	end
	e.type = t:nextInt()

	if e.type == FX_LIGHT then
		if self.game == gta.GameSA then
			e.color = t:nextRGBA(t)
		end
		e.coronaTex = t:next()
		e.shadowTex = t:next()
		e.lodDist = t:nextFloat()
		e.size = t:nextFloat()
		e.coronaSize = t:nextFloat()
		e.shadowSize = t:nextFloat()
		e.shadowAlpha = t:nextInt()
		e.flashiness = t:nextInt()
		e.reflection = t:nextInt()
		e.lensFlareType = t:nextInt()
		e.flags = t:nextInt()
	elseif e.type == FX_PARTICLE then
		if self.game < gta.GameSA then
			e.particleType = t:nextInt()
			e.dir = t:nextXYZ()
			e.size = t:nextFloat()
		else
			e.particleName = t:next()
		end
	elseif e.type == FX_LOOKATPOINT then
		-- gone in SA
		e.lookType = t:nextInt()
		e.dir = t:nextXYZ()
		e.probability = t:nextInt()
	elseif e.type == FX_PEDQUEUE then
		e.queueType = t:nextInt()
		e.queueDir = t:nextXYZ()
		e.useDir = t:nextXYZ()
		if self.game == gta.GameSA then
			e.forwardDir = t:nextXYZ()
			e.interest = t:nextInt()
			e.lookAt = t:nextInt()
			e.scriptName = t:next(t)
		end
	elseif e.type == FX_INTERIOR then
		e.interiorType = t:nextInt()
		e.width = t:nextFloat()
		e.depth = t:nextFloat()
		e.height = t:nextFloat()
		e.rot = t:nextFloat()
		e.door = t:nextInt()
		e.seed = t:nextInt()
		e.group = t:nextInt()
		e.status = t:nextInt()
		e.lDoorStart = t:nextInt()
		e.lDoorEnd = t:nextInt()
		e.tDoorStart = t:nextInt()
		e.tDoorEnd = t:nextInt()
		e.rDoorStart = t:nextInt()
		e.rDoorEnd = t:nextInt()
		e.lWindowStart = t:nextInt()
		e.lWindowEnd = t:nextInt()
		e.tWindowStart = t:nextInt()
		e.tWindowEnd = t:nextInt()
		e.rWindowStart = t:nextInt()
		e.rWindowEnd = t:nextInt()
		e.noGoLeft = { -1, -1, -1 }
		e.noGoBottom = { -1, -1, -1 }
		e.noGoWidth = { -1, -1, -1 }
		e.noGoDepth = { -1, -1, -1 }
		for i=1,3 do
			e.noGoLeft[i] = t:nextInt()
			e.noGoBottom[i] = t:nextInt()
			e.noGoWidth[i] = t:nextInt()
			e.noGoDepth[i] = t:nextInt()
		end
	elseif e.type == FX_ENTRYEXIT then
		e.prot = t:nextFloat()
		e.wx = t:nextFloat()
		e.wy = t:nextFloat()
		e.spawnPos = t:nextXYZ()
		e.spawnRot = t:nextFloat()
		e.area = t:nextInt()
		e.flags = t:nextInt()
		e.name = t:next()
		e.extracol = t:nextInt()
	elseif e.type == FX_ROADSIGN then
		e.width = t:nextFloat()
		e.height = t:nextFloat()
		e.rotation = t:nextXYZ()
		e.flags = t:nextInt()
		e.text1 = t:next()
		e.text2 = t:next()
		e.text3 = t:next()
		e.text4 = t:next()
	elseif e.type == FX_TRIGGERPOINT then
		e.index = t:nextInt()
	elseif e.type == FX_COVERPOINT then
		e.dir = t:nextXY()
		e.usage = t:nextInt()
	elseif e.type == FX_ESCALATOR then
		e.coords1 = t:nextXYZ()
		e.coords2 = t:nextXYZ()
		e.coords3 = t:nextXYZ()
		e.goingUp = t:nextInt()
	end
	self:AddEffect(e)
end

function gta:Write2dfxLine(e)
	local line = ""
	line = addInt(line, e.id)
	line = addXYZ(line, e.position)
	if self.game < gta.GameSA then
		line = addRGBA(line, e.color)
	end
	line = addInt(line, e.type)

	if e.type == FX_LIGHT then
		if self.game == gta.GameSA then
			line = addRGBA(line, e.color)
		end
		line = addString(line, '"' .. e.coronaTex .. '"')
		line = addString(line, '"' .. e.shadowTex .. '"')
		line = addFloat(line, e.lodDist)
		line = addFloat(line, e.size)
		line = addFloat(line, e.coronaSize)
		line = addFloat(line, e.shadowSize)
		line = addInt(line, e.shadowAlpha)
		line = addInt(line, e.flashiness)
		line = addInt(line, e.reflection)
		line = addInt(line, e.lensFlareType)
		line = addInt(line, e.flags)
	elseif e.type == FX_PARTICLE then
		if self.game < gta.GameSA then
			line = addInt(line, e.particleType)
			line = addXYZ(line, e.dir)
			line = addFloat(line, e.size)
		else
			line = addString(line, e.particleName)
		end
	elseif e.type == FX_LOOKATPOINT then
		line = addInt(line, e.lookType)
		line = addXYZ(line, e.dir)
		line = addInt(line, e.probability)
	elseif e.type == FX_PEDQUEUE then
		line = addInt(line, e.queueType)
		line = addXYZ(line, e.queueDir)
		line = addXYZ(line, e.useDir)
		if self.game == gta.GameSA then
			line = addXYZ(line, e.forwardDir)
			line = addInt(line, e.interest)
			line = addInt(line, e.lookAt)
			line = addString(line, e.scriptName)
		end
	elseif e.type == FX_INTERIOR then
		line = addInt(line, e.interiorType)
		line = addFloat(line, e.width)
		line = addFloat(line, e.depth)
		line = addFloat(line, e.height)
		line = addFloat(line, e.rot)
		line = addInt(line, e.door)
		line = addInt(line, e.seed)
		line = addInt(line, e.group)
		line = addInt(line, e.status)
		line = addInt(line, e.lDoorStart)
		line = addInt(line, e.lDoorEnd)
		line = addInt(line, e.tDoorStart)
		line = addInt(line, e.tDoorEnd)
		line = addInt(line, e.rDoorStart)
		line = addInt(line, e.rDoorEnd)
		line = addInt(line, e.lWindowStart)
		line = addInt(line, e.lWindowEnd)
		line = addInt(line, e.tWindowStart)
		line = addInt(line, e.tWindowEnd)
		line = addInt(line, e.rWindowStart)
		line = addInt(line, e.rWindowEnd)
		for i=1,3 do
			line = addInt(line, e.noGoLeft[i])
			line = addInt(line, e.noGoBottom[i])
			line = addInt(line, e.noGoWidth[i])
			line = addInt(line, e.noGoDepth[i])
		end
	elseif e.type == FX_ENTRYEXIT then
		line = addFloat(line, e.prot)
		line = addFloat(line, e.wx)
		line = addFloat(line, e.wy)
		line = addXYZ(line, e.spawnPos)
		line = addFloat(line, e.spawnRot)
		line = addInt(line, e.area)
		line = addInt(line, e.flags)
		line = addString(line, e.name)
		line = addInt(line, e.extracol)
	elseif e.type == FX_ROADSIGN then
		line = addFloat(line, e.width)
		line = addFloat(line, e.height)
		line = addXYZ(line, e.rotation)
		line = addInt(line, e.flags)
		line = addString(line, e.text1)
		line = addString(line, e.text2)
		line = addString(line, e.text3)
		line = addString(line, e.text4)
	elseif e.type == FX_TRIGGERPOINT then
		line = addInt(line, e.index)
	elseif e.type == FX_COVERPOINT then
		line = addXY(line, e.dir)
		line = addInt(line, e.usage)
	elseif e.type == FX_ESCALATOR then
		line = addXYZ(line, e.coords1)
		line = addXYZ(line, e.coords2)
		line = addXYZ(line, e.coords3)
		line = addInt(line, e.goingUp)
	end
	return line
end

function gta:ReadTxdParentLine(line)
	local t = TokenStream.make(line)
	local txdp = {}
	txdp.txdName = t:next()
	txdp.parentName = t:next()
	self:AddTxdParent(txdp)
end

function gta:WriteTxdParentLine(txdp)
	local line = ""
	line = addString(line, txdp.txdName)
	line = addString(line, txdp.parentName)
	return line
end

local IDEdesc = {}
IDEdesc["objs"] = gta.ReadObjLine
IDEdesc["tobj"] = gta.ReadTObjLine
IDEdesc["hier"] = gta.ReadHierLine
IDEdesc["peds"] = gta.ReadPedLine
IDEdesc["cars"] = gta.ReadVehicleLine
IDEdesc["path"] = gta.ReadPathLine
IDEdesc["2dfx"] = gta.Read2dfxLine
--VC
IDEdesc["weap"] = gta.ReadWeaponLine
--SA
IDEdesc["txdp"] = gta.ReadTxdParentLine
IDEdesc["anim"] = gta.ReadAnimLine



function gta:ReadInstLine(line)
	local t = TokenStream.make(line)

	local inst = Instance.make()
	inst.id = t:nextInt()
	inst.name = t:next()
	if self.game > gta.GameIII then
		inst.area = t:nextInt()
	end
	inst.position = t:nextXYZ()
	if self.game < gta.GameSA then
		inst.scale = t:nextXYZ()
		inst.rotation = t:nextXYZW()
	else
		inst.rotation = t:nextXYZW()
		inst.lodIndex = t:nextInt()
		inst.flags = FlagSet.make(inst.area>>8, instflagsSA)
		inst.area = inst.area & 0xFF
	end
	self:AddInstance(inst, self.currentFile)
end

function gta:WriteInstLine(inst)
	local line = ""
	line = addInt(line, inst.id)
	line = addString(line, inst.name)
	if self.game < gta.GameSA then
		if self.game > gta.GameIII then
			line = addInt(line, inst.area)
		end
		line = addXYZ(line, inst.position)
		line = addXYZ(line, inst.scale)
		line = addXYZW(line, inst.rotation)
	else
		line = addInt(line, inst.area | (inst.flags.bits << 8))
		line = addXYZ(line, inst.position)
		line = addXYZW(line, inst.rotation)
		line = addInt(line, inst.lodIndex)
	end
	return line
end

function gta:ReadCullLine(line)
	local t = TokenStream.make(line)
	local cull = {}
	cull.pos = t:nextXYZ()
	cull.min = t:nextXYZ()
	cull.max = t:nextXYZ()
	cull.flags = t:nextInt()
	cull.wantedLevelDrop = t:nextInt()
	self:AddCullZone(cull)
end

function gta:WriteCullLine(cull)
	local line = ""
	line = addXYZ(line, cull.pos)
	line = addXYZ(line, cull.min)
	line = addXYZ(line, cull.max)
	line = addInt(line, cull.flags)
	line = addInt(line, cull.wantedLevelDrop)
	return line
end

function gta:ReadZoneLine(line)
	local t = TokenStream.make(line)
	local zone = {}
	zone.name = t:next()
	zone.type = t:nextInt()
	zone.min = t:nextXYZ()
	zone.max = t:nextXYZ()
	zone.level = t:nextInt()
	self:AddZone(zone)
end

function gta:ReadMapZoneLine(line)
	local t = TokenStream.make(line)
	local zone = {}
	zone.name = t:next()
	zone.type = t:nextInt()
	zone.min = t:nextXYZ()
	zone.max = t:nextXYZ()
	zone.level = t:nextInt()
	self:AddMapZone(zone)
end

function gta:WriteZoneLine(zone)
	local line = ""
	line = addString(line, zone.name)
	line = addInt(line, zone.type)
	line = addXYZ(line, zone.min)
	line = addXYZ(line, zone.max)
	line = addInt(line, zone.level)
	return line
end

function gta:ReadOccluderLine(line)
	local t = TokenStream.make(line)
	local occl = {}
	occl.pos = t:nextXYZ()
	occl.dimension = t:nextXYZ()
	occl.angle = t:nextFloat()
	self:AddOccluder(occl)
end

function gta:WriteOccluderLine(occl)
	local line = ""
	line = addXYZ(line, occl.pos)
	line = addXYZ(line, occl.dimension)
	line = addFloat(line, occl.angle)
	return line
end

function gta:ReadGarageLine(line)
	local t = TokenStream.make(line)
	local grge = {}
	grge.pos1 = t:nextXY()
	grge.zbot = t:nextFloat()
	grge.pos2 = t:nextXY()
	grge.pos3 = t:nextXY()
	grge.ztop = t:nextFloat()
	grge.flag = t:nextInt()
	grge.type = t:nextInt()
	grge.name = t:next()
	if not grge.name then
		grge.name = ""
	end
	self:AddGarage(grge)
end

function gta:WriteGarageLine(grge)
	local line = ""
	line = addXY(line, grge.pos1)
	line = addFloat(line, grge.zbot)
	line = addXY(line, grge.pos2)
	line = addXY(line, grge.pos3)
	line = addFloat(line, grge.ztop)
	line = addInt(line, grge.flag)
	line = addInt(line, grge.type)
	line = addString(line, grge.name)
	return line
end

function gta:ReadEntryExitLine(line)
	local t = TokenStream.make(line)
	local enex = {}
	enex.position = t:nextXYZ()
	enex.prot = t:nextFloat()
	enex.wx = t:nextFloat()
	enex.wy = t:nextFloat()
	enex.wz = t:nextFloat()
	enex.spawnPos = t:nextXYZ()
	enex.spawnRot = t:nextFloat()
	enex.area = t:nextInt()
	enex.flags = t:nextInt()
	enex.name = t:next()
	enex.extracol = t:nextInt()
	enex.numRandomPeds = t:nextInt()
	enex.openTime = t:nextInt()
	enex.shutTime = t:nextInt()
	self:AddEntryExit(enex)
end

function gta:WriteEntryExitLine(enex)
	local line = ""
	line = addXYZ(line, enex.position)
	line = addFloat(line, enex.prot)
	line = addFloat(line, enex.wx)
	line = addFloat(line, enex.wy)
	line = addFloat(line, enex.wz)
	line = addXYZ(line, enex.spawnPos)
	line = addFloat(line, enex.spawnRot)
	line = addInt(line, enex.area)
	line = addString(line, '"' .. enex.name .. '"')
	line = addInt(line, enex.extracol)
	line = addInt(line, enex.numRandomPeds)
	line = addInt(line, enex.openTime)
	line = addInt(line, enex.shutTime)
	return line
end

function gta:ReadPickupLine(line)
	local t = TokenStream.make(line)
	local pick = {}
	pick.type = t:nextInt()
	pick.position = t:nextXYZ()
	self:AddPickup(pick)
end

function gta:WritePickupLine(pick)
	local line = ""
	line = addInt(line, pick.type)
	line = addXYZ(line, pick.position)
	return line
end

function gta:ReadCarGenLine(line)
	-- doesn't occur
	local t = TokenStream.make(line)
	local cargen = {}
	cargen.position = t:nextXYZ()
	cargen.rot = t:nextFloat()
	cargen.modelId = t:nextInt()
	cargen.col1 = t:nextInt()
	cargen.col2 = t:nextInt()
	cargen.flags = t:nextInt()
	cargen.alarmChance = t:nextInt()
	cargen.lockedChance = t:nextInt()
	cargen.minDelay = t:nextInt()
	cargen.maxDelay = t:nextInt()
	self:AddCarGenerator(cargen, self.currentFile)
end

function gta:WriteCarGenLine(cargen)
	local line = ""
	line = addXYZ(line, cargen.position)
	line = addFloat(line, cargen.rot)
	line = addInt(line, cargen.modelId)
	line = addInt(line, cargen.col1)
	line = addInt(line, cargen.col2)
	line = addInt(line, cargen.flags)
	line = addInt(line, cargen.alarmChance)
	line = addInt(line, cargen.lockedChance)
	line = addInt(line, cargen.minDelay)
	line = addInt(line, cargen.maxDelay)
	return line
end

function gta:ReadStuntJumpLine(line)
	-- doesn't occur
	local t = TokenStream.make(line)
	local jump = {}
	jump.startMin = t:nextXYZ()
	jump.startMax = t:nextXYZ()
	jump.endMin = t:nextXYZ()
	jump.endMax = t:nextXYZ()
	jump.cam = t:nextXYZ()
	jump.score = t:nextInt()
	self:AddStuntJump(jump)
end

function gta:WriteStuntJumpLine(jump)
	local line = ""
	line = addXYZ(line, jump.startMin)
	line = addXYZ(line, jump.startMax)
	line = addXYZ(line, jump.endMin)
	line = addXYZ(line, jump.endMax)
	line = addXYZ(line, jump.cam)
	line = addInt(line, jump.score)
	return line
end

function gta:ReadTimeCycleModLine(line)
	local t = TokenStream.make(line)
	local tcyc = {}
	tcyc.min = t:nextXYZ()
	tcyc.max = t:nextXYZ()
	tcyc.farClip = t:nextInt()
	tcyc.index = t:nextInt()
	tcyc.percentage = t:nextFloat()
	tcyc.range = t:nextFloat()
	tcyc.dirLightMult = t:nextFloat()
	tcyc.lodMult = t:nextFloat()
	self:AddTimeCycleMod(tcyc)
end

function gta:WriteTimeCycleModLine(tcyc)
	local line = ""
	line = addXYZ(line, tcyc.min)
	line = addXYZ(line, tcyc.max)
	line = addInt(line, tcyc.farClip)
	line = addInt(line, tcyc.index)
	line = addFloat(line, tcyc.percentage)
	line = addFloat(line, tcyc.range)
	line = addFloat(line, tcyc.dirLightMult)
	line = addFloat(line, tcyc.lodMult)
	return line
end

function gta:ReadAudioZoneLine(line)
	local t = TokenStream.make(line)
	local auzo = {}
	auzo.name = t:next()
	auzo.active = t:nextInt() ~= 0
	auzo.sound = t:nextInt()
	if #t.fields > 7 then
		auzo.min = t:nextXYZ()
		auzo.max = t:nextXYZ()
	else
		auzo.position = t:nextXYZ()
		auzo.range = t:nextFloat()
	end
	self:AddAudioZone(auzo)
end

function bool2int(b)
	if b then return 1
	else return 0 end
end

function gta:WriteAudioZoneLine(auzo)
	local line = ""
	line = addString(line, auzo.name)
	line = addInt(line, bool2int(auzo.active))
	if auzo.range then
		line = addXYZ(line, auzo.position)
		line = addFloat(line, auzo.range)
	else
		line = addXYZ(line, auzo.min)
		line = addXYZ(line, auzo.max)
	end
	return line
end


local IPLdesc = {}
IPLdesc["inst"] = gta.ReadInstLine
IPLdesc["cull"] = gta.ReadCullLine
IPLdesc["zone"] = gta.ReadZoneLine
-- VC
IPLdesc["path"] = gta.ReadPathLine
IPLdesc["occl"] = gta.ReadOccluderLine
-- SA
IPLdesc["grge"] = gta.ReadGarageLine
IPLdesc["enex"] = gta.ReadEntryExitLine
IPLdesc["pick"] = gta.ReadPickupLine
IPLdesc["cars"] = gta.ReadCarGenLine
IPLdesc["jump"] = gta.ReadStuntJumpLine
IPLdesc["tcyc"] = gta.ReadTimeCycleModLine
IPLdesc["auzo"] = gta.ReadAudioZoneLine
IPLdesc["mult"] = gta.ReadNothing	-- multi-building, unused

local ZONdesc = {}
ZONdesc["zone"] = gta.ReadMapZoneLine


function gta:ReadBinaryIPLHeader(strm)
	local header = {}
	header.numInstances = strm:nextI16()
	header.numMultiBuildings = strm:nextI16()
	header.numZones = strm:nextI16()
	header.numCullZones = strm:nextI16()
	header.numOcclusionVolumes = strm:nextI16()
	header.numEntryExit = strm:nextI16()
	header.numGarages = strm:nextI16()
	header.numPickups = strm:nextI16()
	header.numCarGenerators = strm:nextI16()
	header.numStuntJumps = strm:nextI16()
	header.numTimeCycleMods = strm:nextI16()
	header.numPaths = strm:nextI16()

	header.offInstances = strm:nextI32()
	header.offMultiBuildings = strm:nextI32()
	header.offZones = strm:nextI32()
	header.offCullZones = strm:nextI32()
	header.offOcclusionVolumes = strm:nextI32()
	header.offEntryExit = strm:nextI32()
	header.offGarages = strm:nextI32()
	header.offPickups = strm:nextI32()
	header.offCarGenerators = strm:nextI32()
	header.offStuntJumps = strm:nextI32()
	header.offTimeCycleMods = strm:nextI32()
	header.offPaths = strm:nextI32()
	return header
end

function gta:ReadBinaryInstance(strm)
	local inst = Instance.make()

	inst.position = strm:nextXYZ()
	inst.rotation = strm:nextXYZW()
	inst.id = strm:nextI32()
	inst.area = strm:nextI32()
	inst.lodIndex = strm:nextI32()

	inst.flags = FlagSet.make(inst.area>>8, instflagsSA)
	inst.area = inst.area & 0xFF
	local mdl = self.modelsById[inst.id]
	if mdl then
		inst.mdl = mdl
		inst.name = mdl.modelName
	end
	return inst
end

function gta:ReadBinaryCarGen(strm)
	local cargen = {}
	cargen.position = strm:nextXYZ()
	cargen.rot = strm:nextFloat()
	cargen.modelId = strm:nextI32()
	cargen.col1 = strm:nextI32()
	cargen.col2 = strm:nextI32()
	cargen.flags = strm:nextI32()
	cargen.alarmChance = strm:nextI32()
	cargen.lockedChance = strm:nextI32()
	cargen.minDelay = strm:nextI32()
	cargen.maxDelay = strm:nextI32()
	return cargen
end

function gta:LoadStreamedIPLs()
	for name, ipl in pairs(self.streamIplsByName) do
		local scene = Scene.make(name, ipl.streamingInfo)
		local strm = self:GetStreamBuffer(ipl.streamingInfo)
		local type = strm:nextU32()
		if type ~= 0x79726e62 then
			print("not binary IPL", name)
		end
		local header = self:ReadBinaryIPLHeader(strm)
		strm.pos = header.offInstances + 1
		for i = 1,header.numInstances do
			self:AddInstance(self:ReadBinaryInstance(strm), ipl.streamingInfo)
		end
		strm.pos = header.offCarGenerators + 1
		for i = 1,header.numCarGenerators do
			self:AddCarGenerator(self:ReadBinaryCarGen(strm), ipl.streamingInfo)
		end
	end

	-- link streamed scenes to parents
	for _, scene in ipairs(self.scenes) do
		for i=0,10000 do
			local name = scene.name .. "_stream" .. tostring(i)
			local ipl = self.streamIplsByName[name:lower()]
			if not ipl then break end
			local scn = ipl.streamingInfo.scene
			table.insert(scene.streamed, scn)
			scn.parentScene = scene
		end
	end
end

function gta:ReadFileByDesc(file, desc)
	self.currentFile = self:GetFileFs(file)
	local f = io.open(self.currentFile.fullPath, "r")
	if not f then
		print("Could not open file: " .. self.currentFile.fullPath)
		return
	end
	local sect = 'end'
	self._pathSegment = nil
	for line in f:lines() do
		line = line:gsub(","," "):gsub("#.*", ""):gsub("\r",""):gsub("[\t ]*$", "")
		if line == "" then goto continue end
		local fields = split(line)
		if sect == 'end' then
			sect = fields[1]
		elseif fields[1] == 'end' then
			sect = 'end'
		elseif desc[sect] then
			desc[sect](self, line)
		else
			print('unknown', sect, line, desc[sect])
		end
		::continue::
	end
	f:close()
end

function gta:ReadCdImage(imgfile)
	local img = self:GetFileFs(imgfile)
	if self.game == gta.GameSA then
		self.currentFile = img
	else
		local dirfile = string.sub(imgfile, 0, -4) .. "dir"
		self.currentFile = self:GetFileFs(dirfile)
	end
	local f = io.open(self.currentFile.fullPath, "rb")
	if not f then error("couldn't open " .. self.currentFile.fullPath) end
	local num = 100000000	 -- dumb, whatever
	if self.game == gta.GameSA then
		local data = f:read(8)
		local ver, n = string.unpack("<I4I4", data)
		num = n
	end
	for i=1,num do
		local data = f:read(32)
		if not data or #data < 32 then break end
		-- this kinda doubles as a file as well
		local item = {}
		item.offset, item.size = string.unpack("<I4I4", data)
		item.fileName = data:sub(9):match("^([^%z]+)")
		item.name = item.fileName:match("[^.]+")
		item.ext = item.fileName:sub(string.len(item.name)+2):upper()
		item.container = img
		item.sourceFile = self.currentFile
		table.insert(self.streamedFiles, item)
	end
	f:close()
end

function gta:AddCdImage(imgfile)
	table.insert(self.cdImageFiles, self:GetFileFs(imgfile))
end

function JustFileName(path)
	return path:gsub(".*[/\\]", ""):gsub("[.].*", " "):gsub(" ", "")
end

function gta:ReadDataFile(file)
	self.currentFile = self:GetFileFs(file)
	local f = io.open(self.currentFile.fullPath, "r")
	if not f then
		print("Could not open file: " .. filename)
		return
	end

	for line in f:lines() do
		line = line:gsub("#.*", ""):gsub("\r","")
		if line == "" then goto continue end
		local fields = split(line)

		print(line)
		if fields[1] == "IDE" then
			self:ReadFileByDesc(fields[2], IDEdesc)
		elseif fields[1] == "IPL" then
			local scene = self:AddScene(JustFileName(fields[2]), self:GetFileFs(fields[2]))
			self:ReadFileByDesc(fields[2], IPLdesc)
		elseif fields[1] == "MAPZONE" then
			self:ReadFileByDesc(fields[2], ZONdesc)
		elseif fields[1] == "COLFILE" then
--			print("COL FILE", fields[2], fields[3])
			self.currentFile = self:GetFileFs(fields[3])
		elseif fields[1] == "CDIMAGE" or fields[1] == 'IMG' then
			self:AddCdImage(fields[2])
		elseif fields[1] == "SPLASH" then
			self.currentFile = {}
		elseif fields[1] == "MODELFILE" then
			self.currentFile = self:GetFileFs(fields[2])
-- TODO
		elseif fields[1] == "TEXDICTION" then
			self.currentFile = self:GetFileFs(fields[2])
			table.insert(self.genericTxds, self.currentFile)
		elseif fields[1] == "IMAGEPATH" then
			self.currentFile = {}
			self.imagePath = rw.makePath(self.gameDir .. "/" .. fields[2]) .. "/"
		else
print("unknown", fields[1])
			self.currentFile = {}
		end
		self.currentFile.type = fields[1]
		::continue::
	end
	f:close()
end

function gta:WriteDataSection(f, source, name, list, func)
	local content = ""
	for _, elt in pairs(list) do
		if elt.sourceFile == source then
			content = content .. func(self, elt) .. "\n"
		end
	end
	if content ~= "" then
		f:write(name .. "\n")
		f:write(content)
		f:write("end\n")
	end
end

function gta:WriteIDE(file)
	local f = io.open(file.fullPath, "w")
	if not f then error("couldn't open " .. file.fullPath) end

	self:WriteDataSection(f, file, "peds", self.peds, self.WritePedLine)
	self:WriteDataSection(f, file, "cars", self.vehicles, self.WriteVehicleLine)
	self:WriteDataSection(f, file, "objs", self.buildings, self.WriteObjLine)
	self:WriteDataSection(f, file, "tobj", self.timedBuildings, self.WriteTObjLine)
	self:WriteDataSection(f, file, "path", self.pathSegments, self.WritePathSegment)
	self:WriteDataSection(f, file, "2dfx", self.effects, self.Write2dfxLine)
	if self.game > gta.GameIII then
		self:WriteDataSection(f, file, "weap", self.weapons, self.WriteWeaponLine)
	end
	self:WriteDataSection(f, file, "hier", self.clumps, self.WriteHierLine)
	if self.game == gta.GameSA then
		self:WriteDataSection(f, file, "txdp", self.txdParents, self.WriteTxdParentLine)
		self:WriteDataSection(f, file, "anim", self.animClumps, self.WriteAnimLine)
	end
	f:close()
end

function gta:WriteIPL(file)
	local f = io.open(file.fullPath, "w")
	if not f then error("couldn't open " .. file.fullPath) end

	self:WriteDataSection(f, file, "inst", self.instances, self.WriteInstLine)
	self:WriteDataSection(f, file, "cull", self.cullZones, self.WriteCullLine)
	self:WriteDataSection(f, file, "zone", self.zones, self.WriteZoneLine)
	if self.game > gta.GameIII then
		self:WriteDataSection(f, file, "path", self.pathSegments, self.WritePathSegment)
		self:WriteDataSection(f, file, "occl", self.occluders, self.WriteOccluderLine)
	end
	if self.game == gta.GameSA then
		self:WriteDataSection(f, file, "grge", self.garages, self.WriteGarageLine)
		self:WriteDataSection(f, file, "enex", self.entryExits, self.WriteEntryExitLine)
		self:WriteDataSection(f, file, "pick", self.pickups, self.WritePickupLine)
		self:WriteDataSection(f, file, "cars", self.carGenerators, self.WriteCarGenLine)
		self:WriteDataSection(f, file, "jump", self.stuntJumps, self.WriteStuntJumpLine)
		self:WriteDataSection(f, file, "tcyc", self.timecycMods, self.WriteTimeCycleModLine)
		self:WriteDataSection(f, file, "auzo", self.audioZones, self.WriteAudioZoneLine)
	end

	f:close()
end

function gta:WriteZON(file)
	local f = io.open(file.fullPath, "w")
	if not f then error("couldn't open " .. file.fullPath) end

	self:WriteDataSection(f, file, "zone", self.mapZones, self.WriteZoneLine)

	f:close()
end


function gta.readDir(dirpath)
	local entries = {}
	local f = io.open(dirpath, "rb")
	if not f then error("couldn't open " .. dirpath) end
	while true do
		local data = f:read(32)
		if not data or #data < 32 then break end

		local offset, size = string.unpack("<I4I4", data)
		-- trim null bytes from filename
		local name = data:sub(9):match("^([^%z]+)")

		 entries[name:lower()] = {
			 offset = offset,
			 size   = size,
			 name   = name,
		 }
	end
	f:close()
	return entries
end

function gta.printDir(dir)
	for name, entry in pairs(dir) do
		print(string.format("%-32s  offset: %6d  size: %d", name, entry.offset, entry.size))
	end
end

