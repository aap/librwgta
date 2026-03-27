#include "sol_ImGui.h"

namespace sol_ImGui {
    std::tuple<float, float> GetContentRegionAvail() {
        const auto vec2{ImGui::GetContentRegionAvail()};
        return std::make_tuple(vec2.x, vec2.y);
    }


    void PushFont(ImFont *pFont) { ImGui::PushFont(pFont); }

    void PopFont() { ImGui::PopFont(); }

#ifdef SOL_IMGUI_USE_COLOR_U32
    void PushStyleColor(int idx, int col) { ImGui::PushStyleColor(static_cast<ImGuiCol>(idx), ImU32(col)); }
#endif
    void PushStyleColor(int idx, float colR, float colG, float colB, float colA) {
        ImGui::PushStyleColor(idx, {colR, colG, colB, colA});
    }

    void PopStyleColor() { ImGui::PopStyleColor(); }

    void PopStyleColor(int count) { ImGui::PopStyleColor(count); }

    void PushStyleVar(int idx, float val) { ImGui::PushStyleVar(idx, val); }

    void PushStyleVar(int idx, float valX, float valY) {
        ImGui::PushStyleVar(idx, {valX, valY});
    }

    void PopStyleVar() { ImGui::PopStyleVar(); }

    void PopStyleVar(int count) { ImGui::PopStyleVar(count); }

    std::tuple<float, float, float, float> GetStyleColorVec4(int idx) {
        const auto col{ImGui::GetStyleColorVec4(idx)};
        return std::make_tuple(col.x, col.y, col.z, col.w);
    }

    ImFont *GetFont() { return ImGui::GetFont(); }

    float GetFontSize() { return ImGui::GetFontSize(); }

    std::tuple<float, float> GetFontTexUvWhitePixel() {
        const auto vec2{ImGui::GetFontTexUvWhitePixel()};
        return std::make_tuple(vec2.x, vec2.y);
    }

#ifdef SOL_IMGUI_USE_COLOR_U32
    int GetColorU32(int idx, float alphaMul) { return ImGui::GetColorU32(static_cast<ImGuiCol>(idx), alphaMul); }
    int GetColorU32(float colR, float colG, float colB, float colA) { return ImGui::GetColorU32({ colR, colG, colB, colA }); }
    int GetColorU32(int col { return ImGui::GetColorU32(ImU32(col)); }
#endif

    void PushItemWidth(float itemWidth) { ImGui::PushItemWidth(itemWidth); }

    void PopItemWidth() { ImGui::PopItemWidth(); }

    void SetNextItemWidth(float itemWidth) { ImGui::SetNextItemWidth(itemWidth); }

    float CalcItemWidth() { return ImGui::CalcItemWidth(); }

    void PushTextWrapPos() { ImGui::PushTextWrapPos(); }

    void PushTextWrapPos(float wrapLocalPosX) { ImGui::PushTextWrapPos(wrapLocalPosX); }

    void PopTextWrapPos() { ImGui::PopTextWrapPos(); }

    void PushAllowKeyboardFocus(bool allowKeyboardFocus) {
        ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, !allowKeyboardFocus);
    }

    void PopAllowKeyboardFocus() { ImGui::PopItemFlag(); }

    void PushButtonRepeat(bool repeat) { ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, repeat); }

    void PopButtonRepeat() { ImGui::PopItemFlag(); }

    void Separator() { ImGui::Separator(); }

    void SameLine() { ImGui::SameLine(); }

    void SameLine(float offsetFromStartX) { ImGui::SameLine(offsetFromStartX); }

    void SameLine(float offsetFromStartX, float spacing) { ImGui::SameLine(offsetFromStartX, spacing); }

    void NewLine() { ImGui::NewLine(); }

    void Spacing() { ImGui::Spacing(); }

    void Dummy(float sizeX, float sizeY) { ImGui::Dummy({sizeX, sizeY}); }

    void Indent() { ImGui::Indent(); }

    void Indent(float indentW) { ImGui::Indent(indentW); }

    void Unindent() { ImGui::Unindent(); }

    void Unindent(float indentW) { ImGui::Unindent(indentW); }

    void BeginGroup() { ImGui::BeginGroup(); }

    void EndGroup() { ImGui::EndGroup(); }

    std::tuple<float, float> GetCursorPos() {
        const auto vec2{ImGui::GetCursorPos()};
        return std::make_tuple(vec2.x, vec2.y);
    }

    float GetCursorPosX() { return ImGui::GetCursorPosX(); }

    float GetCursorPosY() { return ImGui::GetCursorPosY(); }

    void SetCursorPos(float localX, float localY) { ImGui::SetCursorPos({localX, localY}); }

    void SetCursorPosX(float localX) { ImGui::SetCursorPosX(localX); }

    void SetCursorPosY(float localY) { ImGui::SetCursorPosY(localY); }

    std::tuple<float, float> GetCursorStartPos() {
        const auto vec2{ImGui::GetCursorStartPos()};
        return std::make_tuple(vec2.x, vec2.y);
    }

    std::tuple<float, float> GetCursorScreenPos() {
        const auto vec2{ImGui::GetCursorScreenPos()};
        return std::make_tuple(vec2.x, vec2.y);
    }

    void SetCursorScreenPos(float posX, float posY) { ImGui::SetCursorScreenPos({posX, posY}); }

    void AlignTextToFramePadding() { ImGui::AlignTextToFramePadding(); }

    float GetTextLineHeight() { return ImGui::GetTextLineHeight(); }

    float GetTextLineHeightWithSpacing() { return ImGui::GetTextLineHeightWithSpacing(); }

    float GetFrameHeight() { return ImGui::GetFrameHeight(); }

    float GetFrameHeightWithSpacing() { return ImGui::GetFrameHeightWithSpacing(); }

    bool BeginTabBar(const std::string &str_id) { return ImGui::BeginTabBar(str_id.c_str()); }

    bool BeginTabBar(const std::string &str_id, int flags) {
        return ImGui::BeginTabBar(str_id.c_str(), flags);
    }

    void EndTabBar() { ImGui::EndTabBar(); }

    bool BeginTabItem(const std::string &label) {
        // No p_open
        return ImGui::BeginTabItem(label.c_str(), NULL, 0);
    }

    bool BeginTabItem(const std::string &label, int flags) {
        // No p_open, with flags
        return ImGui::BeginTabItem(label.c_str(), NULL, flags);
    }

    std::tuple<bool, bool> BeginTabItem(const std::string &label, bool open_val, int flags) {
        bool p_open = open_val;
        bool selected = ImGui::BeginTabItem(label.c_str(), &p_open, flags);
        return std::make_tuple(p_open, selected);
    }

    std::tuple<bool, bool> BeginTabItem(const std::string &label, bool open_val) {
        return BeginTabItem(label, open_val, 0);
    }

    void EndTabItem() { ImGui::EndTabItem(); }

    void SetTabItemClosed(const std::string &tab_or_docked_window_label) {
        ImGui::SetTabItemClosed(tab_or_docked_window_label.c_str());
    }

#if (defined(IMGUI_HAS_DOCK) || defined(CUSTOM_IMGUI_HAS_DOCK)) // Heuristic check
    void DockSpace(ImGuiID id) { ImGui::DockSpace(id); }
    void DockSpace(ImGuiID id, const ImVec2& size) { ImGui::DockSpace(id, size); }
    void DockSpace(ImGuiID id, float sizeX, float sizeY) { ImGui::DockSpace(id, {sizeX, sizeY}); }
    void DockSpace(ImGuiID id, float sizeX, float sizeY, int flags) {
        ImGui::DockSpace(id, {sizeX, sizeY}, static_cast<ImGuiDockNodeFlags>(flags));
    }
    // DockSpaceOverViewport might not be in all versions or require specific setup
    // inline unsigned int DockSpaceOverViewport() { return ImGui::DockSpaceOverViewport(); } // Requires Viewport* argument
    void SetNextWindowDockID(ImGuiID dock_id) { ImGui::SetNextWindowDockID(dock_id); }
    void SetNextWindowDockID(ImGuiID dock_id, int cond) {
        ImGui::SetNextWindowDockID(dock_id, static_cast<ImGuiCond>(cond));
    }
    // inline void SetNextWindowClass(const ImGuiWindowClass* window_class) { ImGui::SetNextWindowClass(window_class); } // Complex type
    ImGuiID GetWindowDockID() { return ImGui::GetWindowDockID(); }
    bool IsWindowDocked() { return ImGui::IsWindowDocked(); }
#else
    void DockSpace(unsigned int) {
        /* UNSUPPORTED: Docking not enabled/available */
    }

    void DockSpace(unsigned int, float, float) {
        /* UNSUPPORTED: Docking not enabled/available */
    }

    void DockSpace(unsigned int, float, float, int) {
        /* UNSUPPORTED: Docking not enabled/available */
    }

    unsigned int DockSpaceOverViewport() {
        return 0; /* UNSUPPORTED: Docking not enabled/available */
    }

    void SetNextWindowDockID(unsigned int) {
        /* UNSUPPORTED: Docking not enabled/available */
    }

    void SetNextWindowDockID(unsigned int, int) {
        /* UNSUPPORTED: Docking not enabled/available */
    }

    void SetNextWindowClass() {
        /* UNSUPPORTED: Docking not enabled/available */
    }

    unsigned int GetWindowDockID() {
        return 0; /* UNSUPPORTED: Docking not enabled/available */
    }

    bool IsWindowDocked() {
        return false; /* UNSUPPORTED: Docking not enabled/available */
    }
#endif

    void PushClipRect(float min_x, float min_y, float max_x, float max_y, bool intersect_current) {
        ImGui::PushClipRect({min_x, min_y}, {max_x, max_y}, intersect_current);
    }

    void PopClipRect() { ImGui::PopClipRect(); }

    bool IsItemHovered() { return ImGui::IsItemHovered(); }

    bool IsItemHovered(int flags) { return ImGui::IsItemHovered(flags); }

    bool IsItemActive() { return ImGui::IsItemActive(); }

    bool IsItemFocused() { return ImGui::IsItemFocused(); }

    bool IsItemClicked() { return ImGui::IsItemClicked(); }

    bool IsItemClicked(int mouse_button) {
        return ImGui::IsItemClicked(mouse_button);
    }

    bool IsItemVisible() { return ImGui::IsItemVisible(); }

    bool IsItemEdited() { return ImGui::IsItemEdited(); }

    bool IsItemActivated() { return ImGui::IsItemActivated(); }

    bool IsItemDeactivated() { return ImGui::IsItemDeactivated(); }

    bool IsItemDeactivatedAfterEdit() { return ImGui::IsItemDeactivatedAfterEdit(); }

    bool IsItemToggledOpen() { return ImGui::IsItemToggledOpen(); }

    bool IsAnyItemHovered() { return ImGui::IsAnyItemHovered(); }

    bool IsAnyItemActive() { return ImGui::IsAnyItemActive(); }

    bool IsAnyItemFocused() { return ImGui::IsAnyItemFocused(); }

    std::tuple<float, float> GetItemRectMin() {
        const auto vec2{ImGui::GetItemRectMin()};
        return std::make_tuple(vec2.x, vec2.y);
    }

    std::tuple<float, float> GetItemRectMax() {
        const auto vec2{ImGui::GetItemRectMax()};
        return std::make_tuple(vec2.x, vec2.y);
    }

    std::tuple<float, float> GetItemRectSize() {
        const auto vec2{ImGui::GetItemRectSize()};
        return std::make_tuple(vec2.x, vec2.y);
    }

    void SetNextItemAllowOverlap() { ImGui::SetNextItemAllowOverlap(); }

    bool IsRectVisible(float sizeX, float sizeY) { return ImGui::IsRectVisible({sizeX, sizeY}); }

    bool IsRectVisible(float minX, float minY, float maxX, float maxY) {
        return ImGui::IsRectVisible({minX, minY}, {maxX, maxY});
    }

#pragma region Init
    void Init_ParameterStacks(sol::table &imGuiTable) {
#pragma region Parameters stacks (shared)
        imGuiTable.set_function("PushFont", PushFont);
        imGuiTable.set_function("PopFont", PopFont);
#ifdef SOL_IMGUI_USE_COLOR_U32
		ImGui.set_function("PushStyleColor"					, sol::overload(
																sol::resolve<void(int, int)>(PushStyleColor),
																sol::resolve<void(int, float, float, float, float)>(PushStyleColor)
															));
#else
        imGuiTable.set_function("PushStyleColor", sol::resolve<void(int, float, float, float, float)>(PushStyleColor));
#endif
        imGuiTable.set_function("PopStyleColor", sol::overload(
                                    sol::resolve<void()>(PopStyleColor),
                                    sol::resolve<void(int)>(PopStyleColor)
                                ));
        imGuiTable.set_function("PushStyleVar", sol::overload(
                                    sol::resolve<void(int, float)>(PushStyleVar),
                                    sol::resolve<void(int, float, float)>(PushStyleVar)
                                ));
        imGuiTable.set_function("PopStyleVar", sol::overload(
                                    sol::resolve<void()>(PopStyleVar),
                                    sol::resolve<void(int)>(PopStyleVar)
                                ));
        imGuiTable.set_function("GetStyleColorVec4", GetStyleColorVec4);
        imGuiTable.set_function("GetFont", GetFont);
        imGuiTable.set_function("GetFontSize", GetFontSize);
        imGuiTable.set_function("GetFontTexUvWhitePixel", GetFontTexUvWhitePixel);
#ifdef SOL_IMGUI_USE_COLOR_U32
		ImGui.set_function("GetColorU32"					, sol::overload(
																sol::resolve<int(int, float)>(GetColorU32),
																sol::resolve<int(float, float, float, float)>(GetColorU32),
																sol::resolve<int(int)>(GetColorU32)
															));
#endif
#pragma endregion Parameters stacks (shared)

#pragma region Parameters stacks (current window)
        imGuiTable.set_function("PushItemWidth", PushItemWidth);
        imGuiTable.set_function("PopItemWidth", PopItemWidth);
        imGuiTable.set_function("SetNextItemWidth", SetNextItemWidth);
        imGuiTable.set_function("CalcItemWidth", CalcItemWidth);
        imGuiTable.set_function("PushTextWrapPos", sol::overload(
                                    sol::resolve<void()>(PushTextWrapPos),
                                    sol::resolve<void(float)>(PushTextWrapPos)
                                ));
        imGuiTable.set_function("PopTextWrapPos", PopTextWrapPos);
        imGuiTable.set_function("PushAllowKeyboardFocus", PushAllowKeyboardFocus); // Uses PushItemFlag
        imGuiTable.set_function("PopAllowKeyboardFocus", PopAllowKeyboardFocus); // Uses PopItemFlag
        imGuiTable.set_function("PushButtonRepeat", PushButtonRepeat); // Uses PushItemFlag
        imGuiTable.set_function("PopButtonRepeat", PopButtonRepeat); // Uses PopItemFlag
#pragma endregion Parameters stacks (current window)
    }

    void Init_CursorLayout(sol::table &imGuiTable) {
        imGuiTable.set_function("Separator", Separator);
        imGuiTable.set_function("SameLine", sol::overload(
                                    sol::resolve<void()>(SameLine),
                                    sol::resolve<void(float)>(SameLine),
                                    sol::resolve<void(float, float)>(SameLine) // Added new overload
                                ));
        imGuiTable.set_function("NewLine", NewLine);
        imGuiTable.set_function("Spacing", Spacing);
        imGuiTable.set_function("Dummy", Dummy);
        imGuiTable.set_function("Indent", sol::overload(
                                    sol::resolve<void()>(Indent),
                                    sol::resolve<void(float)>(Indent)
                                ));
        imGuiTable.set_function("Unindent", sol::overload(
                                    sol::resolve<void()>(Unindent),
                                    sol::resolve<void(float)>(Unindent)
                                ));
        imGuiTable.set_function("BeginGroup", BeginGroup);
        imGuiTable.set_function("EndGroup", EndGroup);
        imGuiTable.set_function("GetCursorPos", GetCursorPos);
        imGuiTable.set_function("GetCursorPosX", GetCursorPosX);
        imGuiTable.set_function("GetCursorPosY", GetCursorPosY);
        imGuiTable.set_function("SetCursorPos", SetCursorPos);
        imGuiTable.set_function("SetCursorPosX", SetCursorPosX);
        imGuiTable.set_function("SetCursorPosY", SetCursorPosY);
        imGuiTable.set_function("GetCursorStartPos", GetCursorStartPos);
        imGuiTable.set_function("GetCursorScreenPos", GetCursorScreenPos);
        imGuiTable.set_function("SetCursorScreenPos", SetCursorScreenPos);
        imGuiTable.set_function("AlignTextToFramePadding", AlignTextToFramePadding);
        imGuiTable.set_function("GetTextLineHeight", GetTextLineHeight);
        imGuiTable.set_function("GetTextLineHeightWithSpacing", GetTextLineHeightWithSpacing);
        imGuiTable.set_function("GetFrameHeight", GetFrameHeight);
        imGuiTable.set_function("GetFrameHeightWithSpacing", GetFrameHeightWithSpacing);
    }

    void Init_TabBars(sol::table &imGuiTable) {
        imGuiTable.set_function("BeginTabBar", sol::overload(
                                    sol::resolve<bool(const std::string &)>(BeginTabBar),
                                    sol::resolve<bool(const std::string &, int)>(BeginTabBar)
                                ));
        imGuiTable.set_function("EndTabBar", EndTabBar);
        imGuiTable.set_function("BeginTabItem", sol::overload(
                                    sol::resolve<bool(const std::string &)>(BeginTabItem), // No p_open
                                    sol::resolve<bool(const std::string &, int)>(BeginTabItem), // No p_open, with flags
                                    sol::resolve<std::tuple<bool, bool>(const std::string &, bool)>(BeginTabItem),
                                    // With p_open
                                    sol::resolve<std::tuple<bool, bool>(const std::string &, bool, int)>(BeginTabItem)
                                    // With p_open and flags
                                ));
        imGuiTable.set_function("EndTabItem", EndTabItem);
        imGuiTable.set_function("SetTabItemClosed", SetTabItemClosed);
    }

    void Init_Docking(sol::table &imGuiTable) {
        imGuiTable.set_function("DockSpace", sol::overload(
                                    sol::resolve<void(unsigned int)>(DockSpace),
                                    sol::resolve<void(unsigned int, float, float)>(DockSpace),
                                    sol::resolve<void(unsigned int, float, float, int)>(DockSpace)
                                ));
        imGuiTable.set_function("SetNextWindowDockID", sol::overload(
                                    sol::resolve<void(unsigned int)>(SetNextWindowDockID), // unsigned int for ImGuiID
                                    sol::resolve<void(unsigned int, int)>(SetNextWindowDockID)
                                ));
        imGuiTable.set_function("GetWindowDockID", GetWindowDockID);
        imGuiTable.set_function("IsWindowDocked", IsWindowDocked);
    }

    void Init_Clipping(sol::table &imGuiTable) {
        imGuiTable.set_function("PushClipRect", PushClipRect);
        imGuiTable.set_function("PopClipRect", PopClipRect);
    }

    void Init_ItemWidgetsUtilities(sol::table &imGuiTable) {
        imGuiTable.set_function("IsItemHovered", sol::overload(
                                    sol::resolve<bool()>(IsItemHovered),
                                    sol::resolve<bool(int)>(IsItemHovered)
                                ));
        imGuiTable.set_function("IsItemActive", IsItemActive);
        imGuiTable.set_function("IsItemFocused", IsItemFocused);
        imGuiTable.set_function("IsItemClicked", sol::overload(
                                    sol::resolve<bool()>(IsItemClicked),
                                    sol::resolve<bool(int)>(IsItemClicked)
                                ));
        imGuiTable.set_function("IsItemVisible", IsItemVisible);
        imGuiTable.set_function("IsItemEdited", IsItemEdited);
        imGuiTable.set_function("IsItemActivated", IsItemActivated);
        imGuiTable.set_function("IsItemDeactivated", IsItemDeactivated);
        imGuiTable.set_function("IsItemDeactivatedAfterEdit", IsItemDeactivatedAfterEdit);
        imGuiTable.set_function("IsItemToggledOpen", IsItemToggledOpen);
        imGuiTable.set_function("IsAnyItemHovered", IsAnyItemHovered);
        imGuiTable.set_function("IsAnyItemActive", IsAnyItemActive);
        imGuiTable.set_function("IsAnyItemFocused", IsAnyItemFocused);
        imGuiTable.set_function("GetItemRectMin", GetItemRectMin);
        imGuiTable.set_function("GetItemRectMax", GetItemRectMax);
        imGuiTable.set_function("GetItemRectSize", GetItemRectSize);
        imGuiTable.set_function("SetNextItemAllowOverlap", SetNextItemAllowOverlap); // Updated from SetItemAllowOverlap
    }

    void Init_MiscUtilities(sol::table &imGuiTable) {
        imGuiTable.set_function("IsRectVisible", sol::overload(
                                    sol::resolve<bool(float, float)>(IsRectVisible),
                                    sol::resolve<bool(float, float, float, float)>(IsRectVisible)
                                ));
        imGuiTable.set_function("GetTime", GetTime);
        imGuiTable.set_function("GetFrameCount", GetFrameCount);
        imGuiTable.set_function("GetStyleColorName", GetStyleColorName);
        imGuiTable.set_function("BeginChildFrame", sol::overload( // Updated to use BeginChild
                                    sol::resolve<bool(unsigned int, float, float)>(BeginChildFrame),
                                    sol::resolve<bool(unsigned int, float, float, int)>(BeginChildFrame)
                                ));
        imGuiTable.set_function("EndChildFrame", EndChildFrame); // Updated to use EndChild
    }

    void InitLayout(sol::table &imGuiTable) {
        Init_ParameterStacks(imGuiTable);
        Init_CursorLayout(imGuiTable);
        Init_TabBars(imGuiTable);
        Init_Docking(imGuiTable);
        Init_Clipping(imGuiTable);
        Init_ItemWidgetsUtilities(imGuiTable);
        Init_MiscUtilities(imGuiTable);

        // GetContentRegionMax, GetWindowContentRegionMin, GetWindowContentRegionMax, GetWindowContentRegionWidth are obsolete
        // Lua users should use GetCursorScreenPos and GetContentRegionAvail
        imGuiTable.set_function("GetContentRegionAvail", GetContentRegionAvail);
    }
#pragma endregion Init
}
