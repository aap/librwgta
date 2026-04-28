-- alias.lua — Alias-style control strip and marking menu
-- include() after editor_util.lua and view3d.lua.
--
-- Marking menus activate on Shift+Ctrl/CapsLk + any mouse button.
-- Menu descriptions are ordered lists; false = skip that slot.
-- Positions start at N (noon) and step clockwise by 45° per slot.
-- Sectors: 0=E 1=NE 2=N 3=NW 4=W 5=SW 6=S 7=SE

-- Clockwise from N: 2,1,0,7,6,5,4,3
local clockFromN = { 2, 1, 0, 7, 6, 5, 4, 3 }

-- ---------------------------------------------------------------------------
-- Drag state (ithil DragStuff equivalent)
-- drag.button: 0=idle, 1=LMB, 2=MMB, 3=RMB (1-based)
-- ---------------------------------------------------------------------------

-- ---------------------------------------------------------------------------
-- Marking menu state
-- ---------------------------------------------------------------------------

markingMenus = { false, false, false }  -- [1]=LMB [2]=MMB [3]=RMB; false = no menu

markingActive = false   -- true while any marking menu drag is in progress

function markingMenu()
	if not (IsCtrlDown() and IsShiftDown()) then
		markingActive = false
		return
	end

	local btn = drag.button
	if btn >= 1 and btn <= 3 and markingMenus[btn] then
		if drag.active or drag.stopping then
			markingActive = true
			local sx, sy = drag.start.x, drag.start.y
			local ex, ey = drag.stop.x,  drag.stop.y
			local activeSector = al.MarkingDraw(sx, sy, ex, ey, drag.stopping)
			local pos = 1
			for _, entry in ipairs(markingMenus[btn]) do
				if entry ~= false then
					local sector = clockFromN[pos]
					if al.MarkingEntry(entry.label, sx, sy, sector, activeSector, drag.stopping) then
						if entry.fn then entry.fn() end
					end
				end
				pos = pos + 1
				if pos > 8 then pos = 1 end
			end
			return
		end
	end

	markingActive = false
end

-- ---------------------------------------------------------------------------
-- Default menu content for modelview.lua
-- ---------------------------------------------------------------------------

function setupModelviewMenus()
	-- LMB: empty for now
	markingMenus[1] = false

	-- MMB: transform operations
	markingMenus[2] = {
		{ label = "Translate", fn = function() gizmo.op = gizmo.TRANSLATE end },
		{ label = "Rotate",    fn = function() gizmo.op = gizmo.ROTATE    end },
		{ label = "Scale",     fn = function() gizmo.op = gizmo.SCALE     end },
		false,
		false,
		false,
		false,
		false,
	}

	-- RMB: view toggles
	markingMenus[3] = {
		{ label = "Grid",      fn = function() if activeView then activeView.drawGrid   = not activeView.drawGrid   end end },
		{ label = "Wire",      fn = function() if activeView then activeView.drawWire   = not activeView.drawWire   end end },
		{ label = "Shaded",    fn = function() if activeView then activeView.drawShaded = not activeView.drawShaded end end },
		false,
		{ label = "Fullscreen",fn = function() if activeView then activeView:toggleFullscreen() end end },
		false,
		false,
		false,
	}
end
