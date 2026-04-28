#include <cstdio>
#include <cstring>
#include <cmath>
#include <map>

#include <rw.h>
#include <skeleton.h>

#include "imgui/imgui.h"
#include "sol/sol.hpp"

#include "stuff.h"

// ---------------------------------------------------------------------------
// AlMenu — Alias-style command strip
//
// Each named menu has a fixed-size button in the control strip.  Clicking
// (or holding) the button opens a popup *above* it; the popup closes on
// mouse release.  The last selected item is remembered and shown in an
// indicator button next to the menu button.  Selecting an item also fires
// that item's Lua callback immediately.
//
// Lua API (exposed on the "al" table):
//   al.BeginMenu(id)       → bool   open/draw the menu button + popup
//   al.EndMenu()                    close the popup
//   al.Entry(id)           → bool   a selectable item; true = fired this frame
//   al.EntryToggle(id, v)  → bool, bool  item with toggle; returns fired, new-v
//   al.Indicator(id)               draw the current-selection indicator button
// ---------------------------------------------------------------------------

struct AlMenuState
{
	bool        closeAtEnd;
	int         frame;         // counts up while popup is open; position hidden until >= 10
	ImGuiID     lastSelection;
	const char *label;         // label of currently selected entry (nullptr = not yet set)
	ImVec2      pos;           // screen pos of button when popup was opened
	ImVec2      size;          // measured popup size (updated each open frame)
	bool        buttonHovered;

	AlMenuState() : closeAtEnd(false), frame(0), lastSelection(0), label(nullptr) {}
};

static std::map<ImGuiID, AlMenuState> alMenus;
static AlMenuState *currentAlMenu = nullptr;

static void
DummyWidth(float w)
{
	ImVec2 cursor = ImGui::GetCursorPos();
	ImGui::Dummy(ImVec2(w, 10));
	ImGui::SetCursorPos(cursor);
}

// BeginAlMenu returns true when the popup is open and entries should be drawn.
// The first AlMenuEntry call each frame sets lastSelection/label if not yet set,
// so no silent init-open is needed.
static bool
BeginAlMenu(const char *id)
{
	ImGuiID gid = ImGui::GetID(id);
	currentAlMenu = &alMenus[gid];
	AlMenuState &s = *currentAlMenu;

	ImVec2 sz(76, 60);
	ImVec2 btnPos = ImGui::GetCursorScreenPos();
	ImGui::Button(id, sz);

	s.buttonHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)
	                     && s.label != nullptr;

	s.pos = btnPos;
	// simulate delayed click: hide popup off-screen for first 10 frames
	if(s.frame < 10)
		ImGui::SetNextWindowPos(ImVec2(-10000, -10000));
	else
		ImGui::SetNextWindowPos(ImVec2(s.pos.x, s.pos.y - s.size.y));

	bool initOpen = currentAlMenu->label == nullptr;
	if(initOpen || ImGui::IsItemActive()) {
		s.frame = 0;
		ImGui::OpenPopup(id);
		currentAlMenu->closeAtEnd = initOpen;
	}

	if(ImGui::BeginPopup(id)) {
		s.size = ImGui::GetWindowSize();
		s.frame++;
		DummyWidth(200.0f);
		return true;
	}
	return false;
}

static void
EndAlMenu()
{
	AlMenuState &s = *currentAlMenu;
	if(s.closeAtEnd || ImGui::IsMouseReleased(0))
		ImGui::CloseCurrentPopup();
	s.closeAtEnd = false;
	ImGui::EndPopup();
}

// Returns true if the entry was activated (selected as default action, or clicked).
// If pToggle is non-null, it behaves as a toggle item.
static bool
AlMenuEntry(const char *id, bool *pToggle = nullptr)
{
	bool ret = false;
	ImGuiID nID = ImGui::GetID(id);

	if(currentAlMenu->lastSelection == 0) {
		currentAlMenu->lastSelection = nID;
		currentAlMenu->label = id;
	}

	bool selected = currentAlMenu->lastSelection == nID;

	if(currentAlMenu->buttonHovered && selected && ImGui::IsMouseReleased(0)) {
		if(pToggle) *pToggle = !*pToggle;
		ret = true;
	}

	if(selected) {
		ImVec2 rmin = ImGui::GetCursorScreenPos();
		rmin.x -= 4; rmin.y -= 2;
		ImVec2 rmax(rmin.x + ImGui::GetContentRegionAvail().x + 8,
		            rmin.y + ImGui::GetFrameHeight() - 2);
		ImGui::GetWindowDrawList()->AddRectFilled(rmin, rmax,
			ImGui::GetColorU32(ImGuiCol_Header));
	}

	ImGui::MenuItem(id, nullptr, pToggle);
	if(!ret && ImGui::IsItemHovered() && ImGui::IsMouseReleased(0)) {
		currentAlMenu->lastSelection = nID;
		currentAlMenu->label = id;
		ret = true;
		currentAlMenu->closeAtEnd = true;
	}

	return ret;
}

static void
AlMenuIndicator(const char *id)
{
	ImGuiID gid = ImGui::GetID(id);
	AlMenuState *menu = &alMenus[gid];
	ImVec2 sz(76, 20);
	if(menu->label)
		ImGui::Button(menu->label, sz);
	else
		ImGui::Button("##", sz);
	ImGui::SameLine();
}

// ---------------------------------------------------------------------------
// MarkingMenu — drag-activated radial menu
//
// Call MarkingBegin() each frame with the mouse button to watch (1=LMB,
// 2=MMB, 3=RMB).  If a drag on that button is in progress, returns true
// and the caller should call MarkingEntry() for each sector label.
// MarkingEnd() must always be called after MarkingBegin() returns true.
//
// Lua API (on the "al" table):
//   al.MarkingBegin(btn)    → bool   true while drag in progress
//   al.MarkingEntry(label, sector) → bool  true on release in that sector
//   al.MarkingEnd()
// ---------------------------------------------------------------------------

// Draws a floating label button anchored relative to origin point.
// px, py are pivot fractions: (0,0)=top-left of button at origin, (1,1)=bottom-right.
// active=true → highlight as selected candidate. stopping=true → fire if active.
static bool
MarkingButton(const char *id, float ox, float oy, float px, float py, bool active, bool stopping)
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;

	ImGuiStyle &style = ImGui::GetStyle();
	ImVec2 pad  = style.FramePadding;
	ImVec2 tsz  = ImGui::CalcTextSize(id);
	ImVec2 btnsz(tsz.x + 2*pad.x, tsz.y + 2*pad.y);

	ImGui::SetNextWindowPos(ImVec2(ox, oy), 0, ImVec2(px, py));

	char winid[256];
	snprintf(winid, sizeof(winid), "%s##mmwin", id);
	ImGui::Begin(winid, nullptr, flags);

	ImVec2 oldPos = ImGui::GetCursorPos();
	ImGui::SetCursorPos(oldPos);
	ImVec2 screenPos = ImGui::GetCursorScreenPos();

	ImGui::InvisibleButton(id, btnsz);
	bool hovered = active || ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped);
	ImU32 col = hovered ? ImGui::GetColorU32(ImGuiCol_ButtonHovered) : 0xFF222222u;
	ImGui::GetWindowDrawList()->AddRectFilled(
		screenPos,
		ImVec2(screenPos.x + btnsz.x, screenPos.y + btnsz.y),
		col);
	ImGui::SetCursorPos(ImVec2(oldPos.x + pad.x, oldPos.y + pad.y));
	ImGui::TextUnformatted(id);

	ImGui::End();

	return active && stopping;
}

// Sector → (pivotX, pivotY) anchor pairs for 8 directions.
// Sector 0=E, 1=NE, 2=N, 3=NW, 4=W, 5=SW, 6=S, 7=SE  (same as ithil)
static const float sectorPX[8] = { -1.0f,  -1.0f,  0.5f,  2.0f, 2.0f,  2.0f,  0.5f, -1.0f };
static const float sectorPY[8] = {  0.5f,   2.0f,  2.0f,  2.0f, 0.5f, -1.0f, -1.0f, -1.0f };

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

void
registerAlias(sol::state &lua)
{
	sol::table al = lua["al"].get_or_create<sol::table>();

	// --- AlMenu ---
	al.set_function("BeginMenu", [](const char *id) -> bool {
		return BeginAlMenu(id);
	});
	al.set_function("EndMenu", []() {
		EndAlMenu();
	});
	al.set_function("MenuEntry", [](const char *id) -> bool {
		return AlMenuEntry(id, nullptr);
	});
	al.set_function("MenuEntryToggle", [](const char *id, bool v) -> std::tuple<bool,bool> {
		bool toggle = v;
		bool fired = AlMenuEntry(id, &toggle);
		return { fired, toggle };
	});
	al.set_function("MenuIndicator", [](const char *id) {
		AlMenuIndicator(id);
	});

	// al.MarkingDraw(sx, sy, ex, ey, stopping)
	// Draw the drag line and all entry buttons for a marking menu that is active.
	// sx,sy = drag start; ex,ey = current end; stopping = release frame.
	// Returns the active sector (0..7) or -1 if drag too short.
	al.set_function("MarkingDraw", [](float sx, float sy, float ex, float ey, bool stopping)
	                               -> int {
		// Drag line.
		ImGui::GetForegroundDrawList()->AddLine(
			ImVec2(sx, sy), ImVec2(ex, ey), 0xFF00FFFFu, 2.0f);

		// Compute sector from angle.
		float dx = ex - sx, dy = ey - sy;
		float len = sqrtf(dx*dx + dy*dy);
		if(len < 20.0f) return -1;
		const float TAU = 6.28318530718f;
		float angle = atan2f(-dy, dx);
		angle += TAU / 16.0f;
		if(angle < 0.0f) angle += TAU;
		return (int)(angle / TAU * 8);
	});

	// al.MarkingEntry(label, sx, sy, sector, activeSector, stopping)
	// Draw one entry button. Returns true if fired (stopping and sector matches).
	al.set_function("MarkingEntry", [](const char *label, float sx, float sy,
	                                   int sector, int activeSector, bool stopping) -> bool {
		bool active = (activeSector == sector);
		return MarkingButton(label, sx, sy,
		                     sectorPX[sector & 7], sectorPY[sector & 7],
		                     active, stopping);
	});
}
