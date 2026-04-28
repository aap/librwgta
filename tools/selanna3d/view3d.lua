-- view3d.lua — View3D windowing system
-- Each view: { cam=Camera, rwcam, name, rect={x,y,w,h}, drawGrid, drawWire, drawShaded }
-- Views are sub-cameras whose frameBuffers are sub-rasters of rwCamera.
-- The top titleH pixels of each view's rect act as a draggable title bar.

local titleH    = 20  -- height of draggable title bar region (pixels)
local viewBorder = 1
local cornerSize = 14 -- top-right corner hit area for fullscreen toggle

-- Usable view area. Set by the host (ClumpEditor:draw) each frame before layoutViews().
viewRect = { x=0, y=0, w=1, h=1 }

-- View metatable -------------------------------------------------------------

View = {}
View.__index = View

function View.make(world, name, proj, pos, target, drawWire, drawShaded)
	local rwcam = sk.CameraCreate(0, 0, 1)
	rwcam:setNearPlane(0.1)
	rwcam:setFarPlane(500)
	rwcam:setProjection(proj)
	world:addCamera(rwcam)
	local cam = Camera.new()
	cam.rwCamera = rwcam
	cam.near = 0.1
	cam.far = 500
	cam.fov = 70
	cam.position = pos
	cam.target = target
	cam:update()
	local v = setmetatable(
		{ cam=cam, rwcam=rwcam, name=name, rect={x=0,y=0,w=1,h=1},
		  drawGrid=true, drawWire=drawWire, drawShaded=drawShaded,
		  fullscreen=false, savedRect=nil },
		View)
	views[#views+1] = v
	return v
end

function View:begin(cam)
	self.rwcam:frameBuffer():subRaster(cam:frameBuffer(), self.rect.x, self.rect.y, self.rect.w, self.rect.h)
	self.rwcam:zBuffer():subRaster(cam:zBuffer(), self.rect.x, self.rect.y, self.rect.w, self.rect.h)
	self.cam.aspectRatio = self.rect.w / self.rect.h
	self.rwcam:beginUpdate()
end

function View:finish()
	self.rwcam:endUpdate()
end

function View:toggleFullscreen()
	if self.fullscreen then
		self.fullscreen = false
		self.rect = self.savedRect or self.rect
		self.savedRect = nil
	else
		self.fullscreen = true
		self.savedRect = { x=self.rect.x, y=self.rect.y, w=self.rect.w, h=self.rect.h }
		local b = viewBorder
		self.rect = { x=b, y=b, w=viewRect.w-b*2, h=viewRect.h-b*2 }
	end
end

-- Layout machinery -----------------------------------------------------------

layouts = {}
currentLayout = nil

local function applyLayout(rects)
	local vw = math.max(viewRect.w, 2)
	local vh = math.max(viewRect.h, 2)
	local b = viewBorder
	for i, v in ipairs(views) do
		if v.fullscreen then
			v.rect = { x=b, y=b, w=vw-b*2, h=vh-b*2 }
		else
			local q = rects[i] or { x=0, y=0, w=1, h=1 }
			local r = { x=q.x+b, y=q.y+b, w=q.w-b*2, h=q.h-b*2 }
			if r.w < 1 then r.w = 1 end
			if r.h < 1 then r.h = 1 end
			v.rect = r
		end
	end
end

layouts.fourUp = function(vw, vh)
	local hw, hh = math.floor(vw/2), math.floor(vh/2)
	return {
		{ x=0,  y=0,  w=hw,    h=hh    },
		{ x=hw, y=0,  w=vw-hw, h=hh    },
		{ x=0,  y=hh, w=hw,    h=vh-hh },
		{ x=hw, y=hh, w=vw-hw, h=vh-hh },
	}
end

layouts.single = function(vw, vh)
	local idx = 1
	for i, v in ipairs(views) do if v == activeView then idx = i end end
	local rects = {}
	rects[idx] = { x=0, y=0, w=vw, h=vh }
	return rects
end

-- Left third: 3 ortho views stacked; right two-thirds: perspective.
layouts.threeLeft = function(vw, vh)
	local lw = math.floor(vw / 3)
	local h1 = math.floor(vh / 3)
	local h2 = math.floor(vh / 3)
	local h3 = vh - h1 - h2
	return {
		{ x=0,  y=0,     w=lw,    h=h1 },
		{ x=0,  y=h1,    w=lw,    h=h2 },
		{ x=0,  y=h1+h2, w=lw,    h=h3 },
		{ x=lw, y=0,     w=vw-lw, h=vh },
	}
end

-- Named list for UI, in display order.
layoutList = {
	{ name="4-Up",      fn=layouts.fourUp     },
	{ name="3+Persp",   fn=layouts.threeLeft  },
	{ name="Single",    fn=layouts.single     },
}

function layoutViews(resetFullscreen)
	if resetFullscreen then
		for _, v in ipairs(views) do
			v.fullscreen = false
			v.savedRect  = nil
		end
	end
	local vw = math.max(viewRect.w, 2)
	local vh = math.max(viewRect.h, 2)
	applyLayout(currentLayout(vw, vh))
end

-- Per-frame camera update ----------------------------------------------------

local function updateOrthoViewWindow(v)
	if v.rwcam:getProjection() == rw.Camera_PARALLEL then
		local d = v.cam:distanceTo(v.cam.target)
		local aspect = v.rect.w / v.rect.h
		v.rwcam:setViewWindow(d*0.5*aspect, d*0.5)
	end
end

function updateViewCameras()
	for _, v in ipairs(views) do
		v.cam:update()
		updateOrthoViewWindow(v)
	end
end

-- Hit testing ----------------------------------------------------------------

local function pointInRect(r, x, y)
	return x >= r.x and x < r.x+r.w and y >= r.y and y < r.y+r.h
end

local function titleBarRect(v)
	local r = v.rect
	return { x=r.x, y=r.y, w=r.w, h=titleH }
end

local function fullscreenCornerRect(v)
	local r = v.rect
	return { x=r.x+r.w-cornerSize, y=r.y, w=cornerSize, h=titleH }
end

local function contentRect(v)
	local r = v.rect
	return { x=r.x, y=r.y+titleH, w=r.w, h=r.h-titleH }
end

local function viewAt(x, y)
	-- Iterate in reverse so topmost (last) views are hit first.
	for i = #views, 1, -1 do
		if pointInRect(views[i].rect, x, y) then return views[i] end
	end
end

-- Input handling -------------------------------------------------------------

local dragView = nil   -- view being dragged by its title bar
local dragOX, dragOY  -- offset from view origin to mouse at drag start

function processViewInput()
	local mx, my = sk.mouse.x, sk.mouse.y
	local lmb = (sk.mouse.btn & 1) ~= 0
	local prevLmb = (sk.prevmouse.btn & 1) ~= 0

	if sk.mouse.btn ~= 0 and sk.prevmouse.btn == 0 then
		local v = viewAt(mx, my)
		if v then
			setActiveView(v)
		end
	end
	local v = activeView
	if v and sk.isMouseClicked(sk.LMB) and pointInRect(fullscreenCornerRect(v), mx, my) then
		v:toggleFullscreen()
		sk.clickstate = 0  -- consume click so picking doesn't fire
	end
	if v and lmb and not prevLmb and pointInRect(titleBarRect(v), mx, my) then
		dragView = v
		dragOX = mx - v.rect.x
		dragOY = my - v.rect.y
	end

	-- Drag: move view rect (subRaster applied on next begin()).
	if dragView and lmb then
		local r = dragView.rect
		r.x = mx - dragOX
		r.y = my - dragOY
	elseif not lmb then
		dragView = nil
	end
end

-- Content-area mouse check (for camera input / picking).
function mouseInContent(v)
	return pointInRect(contentRect(v), sk.mouse.x, sk.mouse.y)
end

function setActiveView(v)
	if v == activeView then return end
	activeView = v
	for i = 1, #views do
		if views[i] == v then
			table.remove(views, i)
			table.insert(views, v)
			break
		end
	end
end

-- 2D overlay for a single view (title bar, border, corner button).
-- Call inside view:begin()/view:finish(), after 3D rendering.
-- Coordinates are in the sub-camera's local pixel space (0,0 = top-left of view).
function drawViewOverlay(v)
	local titleCol    = rw.RGBA(0xA1, 0xA1, 0xA1, 0xFF)
	local borderCol   = rw.RGBA(0x22, 0x22, 0x22, 0xFF)
	local borderActiv = rw.RGBA(0xFF, 0xFF, 0xFF, 0xFF)

	local w, h = v.rect.w, v.rect.h
	sk.DrawRect(0, 0, w, titleH, titleCol)
	sk.DrawRectLines(0, 0, w, titleH, borderCol)
	local vsp = math.floor((titleH - cornerSize) / 2)
	sk.DrawRectLines(w-cornerSize-vsp, vsp, w-vsp, titleH-vsp, borderCol)
	local col = (v == activeView) and borderActiv or borderCol
	sk.DrawRectLines(1, 1, w, h, col)
end

function getActiveView()    return activeView        end
function getViews()         return views             end
function isDraggingTitle()  return dragView ~= nil   end
