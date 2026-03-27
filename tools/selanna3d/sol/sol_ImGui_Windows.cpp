#include "sol_ImGui.h"

namespace sol_ImGui {
    bool Begin(const std::string &name) { return ImGui::Begin(name.c_str()); }

    std::tuple<bool, bool> Begin(const std::string &name, bool open) {
        bool p_open_value = open;
        bool should_draw = ImGui::Begin(name.c_str(), open ? &p_open_value : NULL);
        return std::make_tuple(p_open_value, should_draw);
    }

    std::tuple<bool, bool> Begin(const std::string &name, bool open, int flags) {
        bool p_open_value = open;
        bool should_draw = ImGui::Begin(name.c_str(), open ? &p_open_value : NULL,
                                        flags);
        return std::make_tuple(p_open_value, should_draw);
    }

    void End() { ImGui::End(); }

    bool BeginChild(const std::string &name) {
        return ImGui::BeginChild(name.c_str(), {0, 0}, ImGuiChildFlags_None, 0);
    }

    bool BeginChild(const std::string &name, float sizeX) {
        return ImGui::BeginChild(name.c_str(), {sizeX, 0}, ImGuiChildFlags_None, 0);
    }

    bool BeginChild(const std::string &name, float sizeX, float sizeY) {
        return ImGui::BeginChild(name.c_str(), {sizeX, sizeY}, ImGuiChildFlags_None, 0);
    }

    bool BeginChild(const std::string &name, float sizeX, float sizeY, bool border) {
        return ImGui::BeginChild(name.c_str(), {sizeX, sizeY}, border ? ImGuiChildFlags_Borders : ImGuiChildFlags_None,
                                 0);
    }

    bool BeginChild(const std::string &name, float sizeX, float sizeY, int child_flags, int window_flags) {
        return ImGui::BeginChild(name.c_str(), {sizeX, sizeY}, child_flags,
                                 window_flags);
    }

    bool BeginChildEx(const std::string &name, float sizeX, float sizeY, bool border, int window_flags) {
        return ImGui::BeginChild(name.c_str(), {sizeX, sizeY}, border ? ImGuiChildFlags_Borders : ImGuiChildFlags_None,
                                 window_flags);
    }

    void EndChild() { ImGui::EndChild(); }

    bool IsWindowAppearing() { return ImGui::IsWindowAppearing(); }

    bool IsWindowCollapsed() { return ImGui::IsWindowCollapsed(); }

    bool IsWindowFocused() { return ImGui::IsWindowFocused(); }

    bool IsWindowFocused(int flags) { return ImGui::IsWindowFocused(flags); }

    bool IsWindowHovered() { return ImGui::IsWindowHovered(); }

    bool IsWindowHovered(int flags) { return ImGui::IsWindowHovered(flags); }

    ImDrawList *GetWindowDrawList() {
        return nullptr; /* TODO: GetWindowDrawList() ==> UNSUPPORTED (exposing ImDrawList is complex) */
    }

    ImGuiViewport *GetWindowViewport() {
        return nullptr; /* TODO: GetWindowViewport() ==> UNSUPPORTED (exposing ImGuiViewport is complex) */
    }

    std::tuple<float, float> GetWindowPos() {
        const auto vec2{ImGui::GetWindowPos()};
        return std::make_tuple(vec2.x, vec2.y);
    }

    std::tuple<float, float> GetWindowSize() {
        const auto vec2{ImGui::GetWindowSize()};
        return std::make_tuple(vec2.x, vec2.y);
    }

    float GetWindowWidth() { return ImGui::GetWindowWidth(); }

    float GetWindowHeight() { return ImGui::GetWindowHeight(); }

    void SetNextWindowPos(float posX, float posY) { ImGui::SetNextWindowPos({posX, posY}); }

    void SetNextWindowPos(float posX, float posY, int cond) {
        ImGui::SetNextWindowPos({posX, posY}, cond);
    }

    void SetNextWindowPos(float posX, float posY, int cond, float pivotX, float pivotY) {
        ImGui::SetNextWindowPos({posX, posY}, cond, {pivotX, pivotY});
    }

    void SetNextWindowSize(float sizeX, float sizeY) { ImGui::SetNextWindowSize({sizeX, sizeY}); }

    void SetNextWindowSize(float sizeX, float sizeY, int cond) {
        ImGui::SetNextWindowSize({sizeX, sizeY}, cond);
    }

    void SetNextWindowSizeConstraints(float minX, float minY, float maxX, float maxY) {
        ImGui::SetNextWindowSizeConstraints({minX, minY}, {maxX, maxY});
    }

    void SetNextWindowContentSize(float sizeX, float sizeY) { ImGui::SetNextWindowContentSize({sizeX, sizeY}); }

    void SetNextWindowCollapsed(bool collapsed) { ImGui::SetNextWindowCollapsed(collapsed); }

    void SetNextWindowCollapsed(bool collapsed, int cond) {
        ImGui::SetNextWindowCollapsed(collapsed, cond);
    }

    void SetNextWindowFocus() { ImGui::SetNextWindowFocus(); }

    void SetNextWindowBgAlpha(float alpha) { ImGui::SetNextWindowBgAlpha(alpha); }

    void SetWindowPos(float posX, float posY) { ImGui::SetWindowPos({posX, posY}); }

    void SetWindowPos(float posX, float posY, int cond) {
        ImGui::SetWindowPos({posX, posY}, cond);
    }

    void SetWindowSize(float sizeX, float sizeY) { ImGui::SetWindowSize({sizeX, sizeY}); }

    void SetWindowSize(float sizeX, float sizeY, int cond) {
        ImGui::SetWindowSize({sizeX, sizeY}, cond);
    }

    void SetWindowCollapsed(bool collapsed) { ImGui::SetWindowCollapsed(collapsed); }

    void SetWindowCollapsed(bool collapsed, int cond) {
        ImGui::SetWindowCollapsed(collapsed, cond);
    }

    void SetWindowFocus() { ImGui::SetWindowFocus(); }

    void SetWindowFontScale(float scale) { ImGui::SetWindowFontScale(scale); }

    void SetWindowPos(const std::string &name, float posX, float posY) {
        ImGui::SetWindowPos(name.c_str(), {posX, posY});
    }

    void SetWindowPos(const std::string &name, float posX, float posY, int cond) {
        ImGui::SetWindowPos(name.c_str(), {posX, posY}, cond);
    }

    void SetWindowSize(const std::string &name, float sizeX, float sizeY) {
        ImGui::SetWindowSize(name.c_str(), {sizeX, sizeY});
    }

    void SetWindowSize(const std::string &name, float sizeX, float sizeY, int cond) {
        ImGui::SetWindowSize(name.c_str(), {sizeX, sizeY}, cond);
    }

    void SetWindowCollapsed(const std::string &name, bool collapsed) {
        ImGui::SetWindowCollapsed(name.c_str(), collapsed);
    }

    void SetWindowCollapsed(const std::string &name, bool collapsed, int cond) {
        ImGui::SetWindowCollapsed(name.c_str(), collapsed, cond);
    }

    void SetWindowFocus(const std::string &name) { ImGui::SetWindowFocus(name.c_str()); }

    bool BeginChildFrame(unsigned int id, float sizeX, float sizeY) {
        return ImGui::BeginChild(id, {sizeX, sizeY}, ImGuiChildFlags_FrameStyle, 0);
    }

    bool BeginChildFrame(unsigned int id, float sizeX, float sizeY, int flags) {
        return ImGui::BeginChild(id, {sizeX, sizeY}, ImGuiChildFlags_FrameStyle, flags);
    }

    void EndChildFrame() { ImGui::EndChild(); }

    float GetScrollX() { return ImGui::GetScrollX(); }

    float GetScrollY() { return ImGui::GetScrollY(); }

    float GetScrollMaxX() { return ImGui::GetScrollMaxX(); }

    float GetScrollMaxY() { return ImGui::GetScrollMaxY(); }

    void SetScrollX(float scrollX) { ImGui::SetScrollX(scrollX); }

    void SetScrollY(float scrollY) { ImGui::SetScrollY(scrollY); }

    void SetScrollHereX() { ImGui::SetScrollHereX(); }

    void SetScrollHereX(float centerXRatio) { ImGui::SetScrollHereX(centerXRatio); }

    void SetScrollHereY() { ImGui::SetScrollHereY(); }

    void SetScrollHereY(float centerYRatio) { ImGui::SetScrollHereY(centerYRatio); }

    void SetScrollFromPosX(float localX) { ImGui::SetScrollFromPosX(localX); }

    void SetScrollFromPosX(float localX, float centerXRatio) { ImGui::SetScrollFromPosX(localX, centerXRatio); }

    void SetScrollFromPosY(float localY) { ImGui::SetScrollFromPosY(localY); }

    void SetScrollFromPosY(float localY, float centerYRatio) { ImGui::SetScrollFromPosY(localY, centerYRatio); }

#pragma region Init
    void Init_Windows(sol::table &imGuiTable) {
        imGuiTable.set_function("Begin", sol::overload(
                                    sol::resolve<bool(const std::string &)>(Begin),
                                    sol::resolve<std::tuple<bool, bool>(const std::string &, bool)>(Begin),
                                    sol::resolve<std::tuple<bool, bool>(const std::string &, bool, int)>(Begin)
                                ));
        imGuiTable.set_function("End", End);
    }

    void Init_ChildWindows(sol::table &imGuiTable) {
        imGuiTable.set_function("BeginChild", sol::overload(
                                    sol::resolve<bool(const std::string &)>(BeginChild),
                                    sol::resolve<bool(const std::string &, float)>(BeginChild),
                                    sol::resolve<bool(const std::string &, float, float)>(BeginChild),
                                    sol::resolve<bool(const std::string &, float, float, bool)>(BeginChild),
                                    // old border version
                                    sol::resolve<bool(const std::string &, float, float, int, int)>(BeginChild)
                                    // new child_flags, window_flags version
                                ));
        imGuiTable.set_function("BeginChildEx", BeginChildEx); // Specific for bool border, int window_flags
        imGuiTable.set_function("EndChild", EndChild);
    }

    void Init_WindowUtilities(sol::table &imGuiTable) {
        imGuiTable.set_function("IsWindowAppearing", IsWindowAppearing);
        imGuiTable.set_function("IsWindowCollapsed", IsWindowCollapsed);
        imGuiTable.set_function("IsWindowFocused", sol::overload(
                                    sol::resolve<bool()>(IsWindowFocused),
                                    sol::resolve<bool(int)>(IsWindowFocused)
                                ));
        imGuiTable.set_function("IsWindowHovered", sol::overload(
                                    sol::resolve<bool()>(IsWindowHovered),
                                    sol::resolve<bool(int)>(IsWindowHovered)
                                ));
        imGuiTable.set_function("GetWindowPos", GetWindowPos);
        imGuiTable.set_function("GetWindowSize", GetWindowSize);
        imGuiTable.set_function("GetWindowWidth", GetWindowWidth);
        imGuiTable.set_function("GetWindowHeight", GetWindowHeight);

        // Prefer  SetNext...
        imGuiTable.set_function("SetNextWindowPos", sol::overload(
                                    sol::resolve<void(float, float)>(SetNextWindowPos),
                                    sol::resolve<void(float, float, int)>(SetNextWindowPos),
                                    sol::resolve<void(float, float, int, float, float)>(SetNextWindowPos)
                                ));
        imGuiTable.set_function("SetNextWindowSize", sol::overload(
                                    sol::resolve<void(float, float)>(SetNextWindowSize),
                                    sol::resolve<void(float, float, int)>(SetNextWindowSize)
                                ));
        imGuiTable.set_function("SetNextWindowSizeConstraints", SetNextWindowSizeConstraints);
        imGuiTable.set_function("SetNextWindowContentSize", SetNextWindowContentSize);
        imGuiTable.set_function("SetNextWindowCollapsed", sol::overload(
                                    sol::resolve<void(bool)>(SetNextWindowCollapsed),
                                    sol::resolve<void(bool, int)>(SetNextWindowCollapsed)
                                ));
        imGuiTable.set_function("SetNextWindowFocus", SetNextWindowFocus);
        imGuiTable.set_function("SetNextWindowBgAlpha", SetNextWindowBgAlpha);
        imGuiTable.set_function("SetWindowPos", sol::overload(
                                    sol::resolve<void(float, float)>(SetWindowPos),
                                    sol::resolve<void(float, float, int)>(SetWindowPos),
                                    sol::resolve<void(const std::string &, float, float)>(SetWindowPos),
                                    sol::resolve<void(const std::string &, float, float, int)>(SetWindowPos)
                                ));
        imGuiTable.set_function("SetWindowSize", sol::overload(
                                    sol::resolve<void(float, float)>(SetWindowSize),
                                    sol::resolve<void(float, float, int)>(SetWindowSize),
                                    sol::resolve<void(const std::string &, float, float)>(SetWindowSize),
                                    sol::resolve<void(const std::string &, float, float, int)>(SetWindowSize)
                                ));
        imGuiTable.set_function("SetWindowCollapsed", sol::overload(
                                    sol::resolve<void(bool)>(SetWindowCollapsed),
                                    sol::resolve<void(bool, int)>(SetWindowCollapsed),
                                    sol::resolve<void(const std::string &, bool)>(SetWindowCollapsed),
                                    sol::resolve<void(const std::string &, bool, int)>(SetWindowCollapsed)
                                ));
        imGuiTable.set_function("SetWindowFocus", sol::overload(
                                    sol::resolve<void()>(SetWindowFocus),
                                    sol::resolve<void(const std::string &)>(SetWindowFocus)
                                ));
        imGuiTable.set_function("SetWindowFontScale", SetWindowFontScale);
    }

    void Init_WindowsScrolling(sol::table &imGuiTable) {
        imGuiTable.set_function("GetScrollX", GetScrollX);
        imGuiTable.set_function("GetScrollY", GetScrollY);
        imGuiTable.set_function("GetScrollMaxX", GetScrollMaxX);
        imGuiTable.set_function("GetScrollMaxY", GetScrollMaxY);
        imGuiTable.set_function("SetScrollX", SetScrollX);
        imGuiTable.set_function("SetScrollY", SetScrollY);
        imGuiTable.set_function("SetScrollHereX", sol::overload(
                                    sol::resolve<void()>(SetScrollHereX),
                                    sol::resolve<void(float)>(SetScrollHereX)
                                ));
        imGuiTable.set_function("SetScrollHereY", sol::overload(
                                    sol::resolve<void()>(SetScrollHereY),
                                    sol::resolve<void(float)>(SetScrollHereY)
                                ));
        imGuiTable.set_function("SetScrollFromPosX", sol::overload(
                                    sol::resolve<void(float)>(SetScrollFromPosX),
                                    sol::resolve<void(float, float)>(SetScrollFromPosX)
                                ));
        imGuiTable.set_function("SetScrollFromPosY", sol::overload(
                                    sol::resolve<void(float)>(SetScrollFromPosY),
                                    sol::resolve<void(float, float)>(SetScrollFromPosY)
                                ));
    }

    void InitWindows(sol::table &imGuiTable) {
        Init_Windows(imGuiTable);
        Init_ChildWindows(imGuiTable);
        Init_WindowUtilities(imGuiTable);
        Init_WindowsScrolling(imGuiTable);
    }
#pragma endregion Init
}
