include('editor_util.lua')
include('alias.lua')
include('view3d.lua')
include('clumpedit.lua')

-- ---------------------------------------------------------------------------
-- Globals needed by clumpedit / view3d / rendering
-- ---------------------------------------------------------------------------

views      = {}
activeView = nil
activeCam  = nil

red      = rw.RGBA(255, 0,   0,   255)
blue     = rw.RGBA(0,   0,   255, 255)
black    = rw.RGBA(0,   0,   0,   255)
white    = rw.RGBA(255, 255, 255, 255)
darkblue = rw.RGBA(0,   4,   96,  255)
xaxis    = rw.V3d(1,0,0)
yaxis    = rw.V3d(0,1,0)
zaxis    = rw.V3d(0,0,1)

filedir = "/u/aap/other/gta/gtasa/models/gta3_img"

clumpEd = ClumpEditor.new()

function hideDamagedLOD(a)
	local name = a:getFrame():getName()
	if string.find(name, "_dam") or
	   string.find(name, "_vlo") or
	   string.find(name, "_lo_") then
		a:setVisibility(false)
	end
end

-- ---------------------------------------------------------------------------
-- Init / callbacks
-- ---------------------------------------------------------------------------

function Init()
	rw.ImageSetSearchPath("/u/aap/other/gta/gtasa/models/generic/generic_txd/")
	clumpEd:initWorld()
	clumpEd:load(filedir, "cheetah.dff", "cheetah.txd")
	setupModelviewMenus()
	print("done")
end

function Draw(timestep)
	sk.updateMouse()
	clumpEd:draw(timestep)
end

function KeyDown(k)
	sk.keysdown[k] = true
	clumpEd:keyDown(k)
end

function KeyUp(k)
	sk.keysdown[k] = false
end

function Resize(w, h)
	clumpEd:resize(w, h)
end
