#pragma once
#include "imgui/imgui.h"
#include <string>
#include <vector> // For InputText buffer
#if !defined(_MSC_VER)
#include <cstring> // For memcpy, memset, strlen
#endif
#include <sol/sol.hpp>

// Helper for ImMin since it's not standard C++ and might not be in global scope from imgui.h
#ifndef ImMin
#define ImMin(A, B)            (((A) < (B)) ? (A) : (B))
#endif

// Helper for strncpy_s (might not be available on all compilers, provide a basic alternative)
#if defined(_MSC_VER)
// Use built-in strncpy_s for MSVC
#else
    // Basic implementation or placeholder for other compilers
    inline int strncpy_s(char* dest, size_t destsz, const char* src, size_t count) {
        if (!dest || destsz == 0 || !src) return 1; // EINVAL
        size_t src_len = strlen(src);
        size_t len_to_copy = std::min({count, destsz - 1, src_len});
        memcpy(dest, src, len_to_copy);
        dest[len_to_copy] = '\0';
        return 0;
    }
#endif


namespace sol_ImGui {
    // Windows
    bool Begin(const std::string &name);

    std::tuple<bool, bool> Begin(const std::string &name, bool open);

    std::tuple<bool, bool> Begin(const std::string &name, bool open, int flags);

    void End();

    // Child Windows
    bool BeginChild(const std::string &name);

    bool BeginChild(const std::string &name, float sizeX);

    bool BeginChild(const std::string &name, float sizeX, float sizeY);

    bool BeginChild(const std::string &name, float sizeX, float sizeY, bool border);

    // Matches the new ImGui::BeginChild signature with child_flags and window_flags
    bool BeginChild(const std::string &name, float sizeX, float sizeY, int child_flags, int window_flags);

    // Kept for backward compatibility if scripts used 'bool border, int window_flags'
    bool BeginChildEx(const std::string &name, float sizeX, float sizeY, bool border, int window_flags);

    void EndChild();

    // Windows Utilities
    bool IsWindowAppearing();

    bool IsWindowCollapsed();

    bool IsWindowFocused();

    bool IsWindowFocused(int flags);

    bool IsWindowHovered();

    bool IsWindowHovered(int flags);

    ImDrawList *GetWindowDrawList();

    ImGuiViewport *GetWindowViewport();

    std::tuple<float, float> GetWindowPos();

    std::tuple<float, float> GetWindowSize();

    float GetWindowWidth();

    float GetWindowHeight();

    // Prefer using SetNext...
    void SetNextWindowPos(float posX, float posY);

    void SetNextWindowPos(float posX, float posY, int cond);

    void SetNextWindowPos(float posX, float posY, int cond, float pivotX, float pivotY);

    void SetNextWindowSize(float sizeX, float sizeY);

    void SetNextWindowSize(float sizeX, float sizeY, int cond);

    void SetNextWindowSizeConstraints(float minX, float minY, float maxX, float maxY);

    void SetNextWindowContentSize(float sizeX, float sizeY);

    void SetNextWindowCollapsed(bool collapsed);

    void SetNextWindowCollapsed(bool collapsed, int cond);

    void SetNextWindowFocus();

    void SetNextWindowBgAlpha(float alpha);

    void SetWindowPos(float posX, float posY);

    void SetWindowPos(float posX, float posY, int cond);

    void SetWindowSize(float sizeX, float sizeY);

    void SetWindowSize(float sizeX, float sizeY, int cond);

    void SetWindowCollapsed(bool collapsed);

    void SetWindowCollapsed(bool collapsed, int cond);

    void SetWindowFocus();

    void SetWindowFontScale(float scale);

    // Obsolete in ImGui, but function still exists

    void SetWindowPos(const std::string &name, float posX, float posY);

    void SetWindowPos(const std::string &name, float posX, float posY, int cond);

    void SetWindowSize(const std::string &name, float sizeX, float sizeY);

    void SetWindowSize(const std::string &name, float sizeX, float sizeY, int cond);

    void SetWindowCollapsed(const std::string &name, bool collapsed);

    void SetWindowCollapsed(const std::string &name, bool collapsed, int cond);

    void SetWindowFocus(const std::string &name);

    // Content Region
    // GetContentRegionMax(), GetWindowContentRegionMin(), GetWindowContentRegionMax(), GetWindowContentRegionWidth() are obsolete.
    // Lua users should use GetCursorScreenPos() and GetContentRegionAvail().
    std::tuple<float, float> GetContentRegionAvail();

    // Windows Scrolling
    float GetScrollX();

    float GetScrollY();

    float GetScrollMaxX();

    float GetScrollMaxY();

    void SetScrollX(float scrollX);

    void SetScrollY(float scrollY);

    void SetScrollHereX();

    void SetScrollHereX(float centerXRatio);

    void SetScrollHereY();

    void SetScrollHereY(float centerYRatio);

    void SetScrollFromPosX(float localX);

    void SetScrollFromPosX(float localX, float centerXRatio);

    void SetScrollFromPosY(float localY);

    void SetScrollFromPosY(float localY, float centerYRatio);

    // Parameters stacks (shared)
    void PushFont(ImFont *pFont);

    void PopFont();
#ifdef SOL_IMGUI_USE_COLOR_U32
	void PushStyleColor(int idx, int col);
#endif
    void PushStyleColor(int idx, float colR, float colG, float colB, float colA);

    void PopStyleColor();

    void PopStyleColor(int count);

    void PushStyleVar(int idx, float val);

    void PushStyleVar(int idx, float valX, float valY);

    void PopStyleVar();

    void PopStyleVar(int count);

    std::tuple<float, float, float, float> GetStyleColorVec4(int idx);

    ImFont *GetFont();

    float GetFontSize();

    std::tuple<float, float> GetFontTexUvWhitePixel();
#ifdef SOL_IMGUI_USE_COLOR_U32
	int GetColorU32(int idx, float alphaMul);
	int GetColorU32(float colR, float colG, float colB, float colA);
	int GetColorU32(int col);
#endif

    // Parameters stacks (current window)
    void PushItemWidth(float itemWidth);

    void PopItemWidth();

    void SetNextItemWidth(float itemWidth);

    float CalcItemWidth();

    void PushTextWrapPos();

    void PushTextWrapPos(float wrapLocalPosX);

    void PopTextWrapPos();

    void PushAllowKeyboardFocus(bool allowKeyboardFocus);

    void PopAllowKeyboardFocus();

    void PushButtonRepeat(bool repeat);

    void PopButtonRepeat();


    // Cursor / Layout
    void Separator();

    void SameLine();

    void SameLine(float offsetFromStartX);

    void SameLine(float offsetFromStartX, float spacing);

    void NewLine();

    void Spacing();

    void Dummy(float sizeX, float sizeY);

    void Indent();

    void Indent(float indentW);

    void Unindent();

    void Unindent(float indentW);

    void BeginGroup();

    void EndGroup();

    std::tuple<float, float> GetCursorPos();

    float GetCursorPosX();

    float GetCursorPosY();

    void SetCursorPos(float localX, float localY);

    void SetCursorPosX(float localX);

    void SetCursorPosY(float localY);

    std::tuple<float, float> GetCursorStartPos();

    std::tuple<float, float> GetCursorScreenPos();

    void SetCursorScreenPos(float posX, float posY);

    void AlignTextToFramePadding();

    float GetTextLineHeight();

    float GetTextLineHeightWithSpacing();

    float GetFrameHeight();

    float GetFrameHeightWithSpacing();

    // ID stack / scopes
    void PushID(const std::string &stringID);

    void PushID(const std::string &stringIDBegin, const std::string &stringIDEnd);

    void PushID(const void *);

    void PushID(int intID);

    void PopID();

    int GetID(const std::string &stringID);

    int GetID(const std::string &stringIDBegin, const std::string &stringIDEnd);

    int GetID(const void *);

    // Widgets: Text
    void TextUnformatted(const std::string &text);

    void TextUnformatted(const std::string &text, const std::string &textEnd);

    // Note: ImGui::Text is variadic. This binding supports only a single string.
    void Text(const std::string &text);

    // Note: ImGui::TextColored is variadic. This binding supports only a single string.
    void TextColored(float colR, float colG, float colB, float colA, const std::string &text);

    // Note: ImGui::TextDisabled is variadic. This binding supports only a single string.
    void TextDisabled(const std::string &text);

    // Note: ImGui::TextWrapped is variadic. This binding supports only a single string.
    void TextWrapped(std::string text);

    // Note: ImGui::LabelText is variadic. This binding supports only a single string.
    void LabelText(const std::string &label, const std::string &text);

    // Note: ImGui::BulletText is variadic. This binding supports only a single string.
    void BulletText(const std::string &text);

    // Widgets: Main
    bool Button(const std::string &label);

    bool Button(const std::string &label, float sizeX, float sizeY);

    bool SmallButton(const std::string &label);

    bool InvisibleButton(const std::string &stringID, float sizeX, float sizeY);

    // Overload for InvisibleButton with flags
    bool InvisibleButton(const std::string &str_id, float sizeX, float sizeY, int flags);


    bool ArrowButton(const std::string &stringID, int dir);

    void Image();

    void ImageButton();

    std::tuple<bool, bool> Checkbox(const std::string &label, bool v);

    std::tuple<int, bool> CheckboxFlags(const std::string &label, int current_flags, int flags_value);

    std::tuple<unsigned int, bool> CheckboxFlags(const std::string &label, unsigned int current_flags,
                                                 unsigned int flags_value);


    bool RadioButton(const std::string &label, bool active);

    std::tuple<int, bool> RadioButton(const std::string &label, int v, int vButton);

    void ProgressBar(float fraction);

    void ProgressBar(float fraction, float sizeX, float sizeY);

    void ProgressBar(float fraction, float sizeX, float sizeY, const std::string &overlay);

    void Bullet();

    // Widgets: Combo Box
    bool BeginCombo(const std::string &label, const std::string &previewValue);

    bool BeginCombo(const std::string &label, const std::string &previewValue, int flags);

    void EndCombo();

    std::tuple<int, bool> Combo(const std::string &label, int currentItem, const sol::table &items,
                                int itemsCount);

    std::tuple<int, bool> Combo(const std::string &label, int currentItem, const sol::table &items,
                                int itemsCount, int popupMaxHeightInItems);

    std::tuple<int, bool> Combo(const std::string &label, int currentItem,
                                const std::string &itemsSeparatedByZeros);

    std::tuple<int, bool> Combo(const std::string &label, int currentItem,
                                const std::string &itemsSeparatedByZeros, int popupMaxHeightInItems);

    // Widgets: Drags (float power argument changed to ImGuiSliderFlags flags)
    std::tuple<float, bool> DragFloat(const std::string &label, float v, float v_speed = 1.0f,
                                      float v_min = 0.0f, float v_max = 0.0f, const char *format = "%.3f",
                                      int flags = 0);

    // Keep existing overloads by forwarding to the one with flags
    std::tuple<float, bool> DragFloat(const std::string &label, float v);

    std::tuple<float, bool> DragFloat(const std::string &label, float v, float v_speed);

    std::tuple<float, bool> DragFloat(const std::string &label, float v, float v_speed, float v_min);

    std::tuple<float, bool>
    DragFloat(const std::string &label, float v, float v_speed, float v_min, float v_max);

    std::tuple<float, bool> DragFloat(const std::string &label, float v, float v_speed, float v_min, float v_max,
                                      const std::string &format);

    // The overload that previously took 'power' now takes 'flags'
    std::tuple<float, bool> DragFloat(const std::string &label, float v, float v_speed, float v_min, float v_max,
                                      const std::string &format, int flags);


    // DragFloat2, DragFloat3, DragFloat4 need similar treatment if they had 'power' (they did)
    // Base version for DragFloatN now takes flags
    template<int N>
    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloatN_Internal(
        const std::string &label, const sol::table &v_table, float v_speed, float v_min, float v_max,
        const char *format, int flags) {
        float value[N];
        for (int i = 0; i < N; ++i) {
            value[i] = v_table[i + 1].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0));
        }
        bool used = false;
        if (N == 2)
          used = ImGui::DragFloat2(label.c_str(), value, v_speed, v_min, v_max, format, flags);
        else if (N == 3)
          used = ImGui::DragFloat3(label.c_str(), value, v_speed, v_min, v_max, format, flags);
        else if (N == 4)
          used = ImGui::DragFloat4(label.c_str(), value, v_speed, v_min, v_max, format, flags);

        std::vector<float> result_vec;
        for (int i = 0; i < N; ++i) result_vec.push_back(value[i]);
        return std::make_tuple(sol::as_table(result_vec), used);
    }

    // DragFloat2
    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    DragFloat2(const std::string &label, const sol::table &v);

    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    DragFloat2(const std::string &label, const sol::table &v, float v_speed);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat2(
        const std::string &label, const sol::table &v, float v_speed, float v_min);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat2(
        const std::string &label, const sol::table &v, float v_speed, float v_min, float v_max);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat2(
        const std::string &label, const sol::table &v, float v_speed, float v_min, float v_max,
        const std::string &format);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat2(
        const std::string &label, const sol::table &v, float v_speed, float v_min, float v_max,
        const std::string &format, int flags);

    // DragFloat3
    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    DragFloat3(const std::string &label, const sol::table &v);

    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    DragFloat3(const std::string &label, const sol::table &v, float v_speed);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat3(
        const std::string &label, const sol::table &v, float v_speed, float v_min);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat3(
        const std::string &label, const sol::table &v, float v_speed, float v_min, float v_max);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat3(
        const std::string &label, const sol::table &v, float v_speed, float v_min, float v_max,
        const std::string &format);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat3(
        const std::string &label, const sol::table &v, float v_speed, float v_min, float v_max,
        const std::string &format, int flags);

    // DragFloat4
    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    DragFloat4(const std::string &label, const sol::table &v);

    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    DragFloat4(const std::string &label, const sol::table &v, float v_speed);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat4(
        const std::string &label, const sol::table &v, float v_speed, float v_min);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat4(
        const std::string &label, const sol::table &v, float v_speed, float v_min, float v_max);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat4(
        const std::string &label, const sol::table &v, float v_speed, float v_min, float v_max,
        const std::string &format);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat4(
        const std::string &label, const sol::table &v, float v_speed, float v_min, float v_max,
        const std::string &format, int flags);


    void DragFloatRange2();

    // DragInt does not have a power argument, takes flags directly
    std::tuple<int, bool> DragInt(const std::string &label, int v, float v_speed = 1.0f, int v_min = 0,
                                  int v_max = 0, const char *format = "%d", int flags = 0);

    // Keep existing overloads by forwarding
    std::tuple<int, bool> DragInt(const std::string &label, int v);

    std::tuple<int, bool> DragInt(const std::string &label, int v, float v_speed);

    std::tuple<int, bool> DragInt(const std::string &label, int v, float v_speed, int v_min);

    std::tuple<int, bool> DragInt(const std::string &label, int v, float v_speed, int v_min, int v_max);

    std::tuple<int, bool> DragInt(const std::string &label, int v, float v_speed, int v_min, int v_max,
                                  const std::string &format);

    // The overload that previously had format only (now explicitly adding flags)
    std::tuple<int, bool> DragInt(const std::string &label, int v, float v_speed, int v_min, int v_max,
                                  const std::string &format, int flags);


    template<int N>
    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragIntN_Internal(
        const std::string &label, const sol::table &v_table, float v_speed, int v_min, int v_max, const char *format,
        int flags) {
        int value[N];
        for (int i = 0; i < N; ++i) {
            value[i] = static_cast<int>(v_table[i + 1].get<std::optional<lua_Number> >().value_or(
                static_cast<lua_Number>(0)));
        }
        bool used = false;
        if (N == 2)
            used = ImGui::DragInt2(label.c_str(), value, v_speed, v_min, v_max, format,
                                   flags);
        else if (N == 3)
          used = ImGui::DragInt3(label.c_str(), value, v_speed, v_min, v_max, format, flags);
        else if (N == 4)
          used = ImGui::DragInt4(label.c_str(), value, v_speed, v_min, v_max, format, flags);

        std::vector<int> result_vec;
        for (int i = 0; i < N; ++i) result_vec.push_back(value[i]);
        return std::make_tuple(sol::as_table(result_vec), used);
    }

    // DragInt2
    std::tuple<sol::as_table_t<std::vector<int> >, bool>
    DragInt2(const std::string &label, const sol::table &v);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt2(
        const std::string &label, const sol::table &v, float v_speed);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt2(
        const std::string &label, const sol::table &v, float v_speed, int v_min);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt2(
        const std::string &label, const sol::table &v, float v_speed, int v_min, int v_max);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt2(
        const std::string &label, const sol::table &v, float v_speed, int v_min, int v_max, const std::string &format);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt2(
        const std::string &label, const sol::table &v, float v_speed, int v_min, int v_max, const std::string &format,
        int flags);

    // DragInt3
    std::tuple<sol::as_table_t<std::vector<int> >, bool>
    DragInt3(const std::string &label, const sol::table &v);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt3(
        const std::string &label, const sol::table &v, float v_speed);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt3(
        const std::string &label, const sol::table &v, float v_speed, int v_min);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt3(
        const std::string &label, const sol::table &v, float v_speed, int v_min, int v_max);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt3(
        const std::string &label, const sol::table &v, float v_speed, int v_min, int v_max, const std::string &format);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt3(
        const std::string &label, const sol::table &v, float v_speed, int v_min, int v_max, const std::string &format,
        int flags);

    // DragInt4
    std::tuple<sol::as_table_t<std::vector<int> >, bool>
    DragInt4(const std::string &label, const sol::table &v);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt4(
        const std::string &label, const sol::table &v, float v_speed);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt4(
        const std::string &label, const sol::table &v, float v_speed, int v_min);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt4(
        const std::string &label, const sol::table &v, float v_speed, int v_min, int v_max);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt4(
        const std::string &label, const sol::table &v, float v_speed, int v_min, int v_max, const std::string &format);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt4(
        const std::string &label, const sol::table &v, float v_speed, int v_min, int v_max, const std::string &format,
        int flags);


    void DragIntRange2();

    void DragScalar();

    void DragScalarN();

    // Widgets: Sliders
    std::tuple<float, bool> SliderFloat(const std::string &label, float v, float v_min, float v_max,
                                        const char *format = "%.3f", int flags = 0);

    // Keep existing overloads
    std::tuple<float, bool> SliderFloat(const std::string &label, float v, float v_min, float v_max);

    std::tuple<float, bool> SliderFloat(const std::string &label, float v, float v_min, float v_max,
                                        const std::string &format);

    // The overload that previously took 'power' now takes 'flags'
    std::tuple<float, bool> SliderFloat(const std::string &label, float v, float v_min, float v_max,
                                        const std::string &format, int flags);


    template<int N>
    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloatN_Internal(
        const std::string &label, const sol::table &v_table, float v_min, float v_max, const char *format, int flags) {
        float value[N];
        for (int i = 0; i < N; ++i) {
            value[i] = v_table[i + 1].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0));
        }
        bool used = false;
        if (N == 2)
            used = ImGui::SliderFloat2(label.c_str(), value, v_min, v_max, format,
                                       flags);
        else if (N == 3)
          used = ImGui::SliderFloat3(label.c_str(), value, v_min, v_max, format, flags);
        else if (N == 4)
          used = ImGui::SliderFloat4(label.c_str(), value, v_min, v_max, format,
                                       flags);

        std::vector<float> result_vec;
        for (int i = 0; i < N; ++i) result_vec.push_back(value[i]);
        // No correction for SliderFloat3 needed here as value is directly from ImGui
        return std::make_tuple(sol::as_table(result_vec), used);
    }

    // SliderFloat2
    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat2(
        const std::string &label, const sol::table &v, float v_min, float v_max);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat2(
        const std::string &label, const sol::table &v, float v_min, float v_max, const std::string &format);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat2(
        const std::string &label, const sol::table &v, float v_min, float v_max, const std::string &format, int flags);

    // SliderFloat3
    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat3(
        const std::string &label, const sol::table &v, float v_min, float v_max);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat3(
        const std::string &label, const sol::table &v, float v_min, float v_max, const std::string &format);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat3(
        const std::string &label, const sol::table &v, float v_min, float v_max, const std::string &format, int flags);

    // SliderFloat4
    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat4(
        const std::string &label, const sol::table &v, float v_min, float v_max);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat4(
        const std::string &label, const sol::table &v, float v_min, float v_max, const std::string &format);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat4(
        const std::string &label, const sol::table &v, float v_min, float v_max, const std::string &format, int flags);


    // SliderAngle does not take power, takes flags
    std::tuple<float, bool> SliderAngle(const std::string &label, float v_rad, float v_degrees_min = -360.0f,
                                        float v_degrees_max = +360.0f, const char *format = "%.0f deg",
                                        int flags = 0);

    // Keep existing overloads
    std::tuple<float, bool> SliderAngle(const std::string &label, float v_rad);

    std::tuple<float, bool> SliderAngle(const std::string &label, float v_rad, float v_degrees_min);

    std::tuple<float, bool> SliderAngle(const std::string &label, float v_rad, float v_degrees_min,
                                        float v_degrees_max);

    std::tuple<float, bool> SliderAngle(const std::string &label, float v_rad, float v_degrees_min,
                                        float v_degrees_max, const std::string &format);

    // Add overload with flags
    std::tuple<float, bool> SliderAngle(const std::string &label, float v_rad, float v_degrees_min,
                                        float v_degrees_max, const std::string &format, int flags);


    // SliderInt family, format then flags
    std::tuple<int, bool> SliderInt(const std::string &label, int v, int v_min, int v_max,
                                    const char *format = "%d", int flags = 0);

    // Keep existing overloads
    std::tuple<int, bool> SliderInt(const std::string &label, int v, int v_min, int v_max);

    std::tuple<int, bool> SliderInt(const std::string &label, int v, int v_min, int v_max,
                                    const std::string &format);

    // Add overload with flags
    std::tuple<int, bool> SliderInt(const std::string &label, int v, int v_min, int v_max,
                                    const std::string &format, int flags);


    template<int N>
    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderIntN_Internal(
        const std::string &label, const sol::table &v_table, int v_min, int v_max, const char *format, int flags) {
        int value[N];
        for (int i = 0; i < N; ++i) {
            value[i] = static_cast<int>(v_table[i + 1].get<std::optional<lua_Number> >().value_or(
                static_cast<lua_Number>(0)));
        }
        bool used = false;
        if (N == 2)
            used = ImGui::SliderInt2(label.c_str(), value, v_min, v_max, format,
                                     flags);
        else if (N == 3)
            used = ImGui::SliderInt3(label.c_str(), value, v_min, v_max, format,
                                     flags);
        else if (N == 4)
            used = ImGui::SliderInt4(label.c_str(), value, v_min, v_max, format,
                                     flags);

        std::vector<int> result_vec;
        for (int i = 0; i < N; ++i) result_vec.push_back(value[i]);
        return std::make_tuple(sol::as_table(result_vec), used);
    }

    // SliderInt2
    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt2(
        const std::string &label, const sol::table &v, int v_min, int v_max);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt2(
        const std::string &label, const sol::table &v, int v_min, int v_max, const std::string &format);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt2(
        const std::string &label, const sol::table &v, int v_min, int v_max, const std::string &format, int flags);

    // SliderInt3
    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt3(
        const std::string &label, const sol::table &v, int v_min, int v_max);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt3(
        const std::string &label, const sol::table &v, int v_min, int v_max, const std::string &format);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt3(
        const std::string &label, const sol::table &v, int v_min, int v_max, const std::string &format, int flags);

    // SliderInt4
    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt4(
        const std::string &label, const sol::table &v, int v_min, int v_max);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt4(
        const std::string &label, const sol::table &v, int v_min, int v_max, const std::string &format);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt4(
        const std::string &label, const sol::table &v, int v_min, int v_max, const std::string &format, int flags);


    void SliderScalar();

    void SliderScalarN();

    // VSliderFloat - power to flags
    std::tuple<float, bool> VSliderFloat(const std::string &label, float sizeX, float sizeY, float v,
                                         float v_min, float v_max, const char *format = "%.3f", int flags = 0);

    // Keep existing overloads
    std::tuple<float, bool> VSliderFloat(const std::string &label, float sizeX, float sizeY, float v,
                                         float v_min, float v_max);

    std::tuple<float, bool> VSliderFloat(const std::string &label, float sizeX, float sizeY, float v,
                                         float v_min, float v_max, const std::string &format);

    // The overload that previously took 'power' now takes 'flags'
    std::tuple<float, bool> VSliderFloat(const std::string &label, float sizeX, float sizeY, float v,
                                         float v_min, float v_max, const std::string &format, int flags);

    // VSliderInt takes format then flags
    std::tuple<int, bool> VSliderInt(const std::string &label, float sizeX, float sizeY, int v, int v_min,
                                     int v_max, const char *format = "%d", int flags = 0);

    // Keep existing overloads
    std::tuple<int, bool> VSliderInt(const std::string &label, float sizeX, float sizeY, int v, int v_min,
                                     int v_max);

    std::tuple<int, bool> VSliderInt(const std::string &label, float sizeX, float sizeY, int v, int v_min,
                                     int v_max, const std::string &format);

    // Add overload with flags
    std::tuple<int, bool> VSliderInt(const std::string &label, float sizeX, float sizeY, int v, int v_min,
                                     int v_max, const std::string &format, int flags);

    void VSliderScalar();

    // Widgets: Input with Keyboard (CRITICAL FIX for buffer handling)
    std::tuple<std::string, bool> InputText(const std::string &label, std::string current_text, size_t buf_size,
                                            int flags = 0);

    // Convenience overload without flags
    std::tuple<std::string, bool> InputText(const std::string &label, std::string current_text, size_t buf_size);


    std::tuple<std::string, bool> InputTextMultiline(const std::string &label, std::string current_text,
                                                     size_t buf_size, const ImVec2 &size = ImVec2(0, 0),
                                                     int flags = 0);

    // Convenience overloads
    std::tuple<std::string, bool> InputTextMultiline(const std::string &label, std::string current_text,
                                                     size_t buf_size);

    std::tuple<std::string, bool> InputTextMultiline(const std::string &label, std::string current_text,
                                                     size_t buf_size, float sizeX, float sizeY);

    std::tuple<std::string, bool> InputTextMultiline(const std::string &label, std::string current_text,
                                                     size_t buf_size, float sizeX, float sizeY, int flags);


    std::tuple<std::string, bool> InputTextWithHint(const std::string &label, const std::string &hint,
                                                    std::string current_text, size_t buf_size, int flags = 0);

    // Convenience overload
    std::tuple<std::string, bool> InputTextWithHint(const std::string &label, const std::string &hint,
                                                    std::string current_text, size_t buf_size);

    // InputFloat takes flags
    std::tuple<float, bool> InputFloat(const std::string &label, float v, float step = 0.0f,
                                       float step_fast = 0.0f, const char *format = "%.3f", int flags = 0);

    // Keep existing overloads
    std::tuple<float, bool> InputFloat(const std::string &label, float v);

    std::tuple<float, bool> InputFloat(const std::string &label, float v, float step);

    std::tuple<float, bool> InputFloat(const std::string &label, float v, float step, float step_fast);

    std::tuple<float, bool> InputFloat(const std::string &label, float v, float step, float step_fast,
                                       const std::string &format);

    // Overload that takes flags
    std::tuple<float, bool> InputFloat(const std::string &label, float v, float step, float step_fast,
                                       const std::string &format, int flags);

    template<int N>
    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloatN_Internal(
        const std::string &label, const sol::table &v_table, const char *format, int flags) {
        float value[N];
        for (int i = 0; i < N; ++i) {
            value[i] = v_table[i + 1].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0));
        }
        bool used = false;
        if (N == 2) used = ImGui::InputFloat2(label.c_str(), value, format, flags);
        else if (N == 3)
          used = ImGui::InputFloat3(label.c_str(), value, format,
                                      flags);
        else if (N == 4)
            used = ImGui::InputFloat4(label.c_str(), value, format, flags);

        std::vector<float> result_vec;
        for (int i = 0; i < N; ++i) result_vec.push_back(value[i]);
        return std::make_tuple(sol::as_table(result_vec), used);
    }

    // InputFloat2
    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    InputFloat2(const std::string &label, const sol::table &v);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat2(
        const std::string &label, const sol::table &v, const std::string &format);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat2(
        const std::string &label, const sol::table &v, const std::string &format, int flags);

    // InputFloat3
    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    InputFloat3(const std::string &label, const sol::table &v);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat3(
        const std::string &label, const sol::table &v, const std::string &format);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat3(
        const std::string &label, const sol::table &v, const std::string &format, int flags);

    // InputFloat4
    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    InputFloat4(const std::string &label, const sol::table &v);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat4(
        const std::string &label, const sol::table &v, const std::string &format);

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat4(
        const std::string &label, const sol::table &v, const std::string &format, int flags);

    // InputInt takes flags
    std::tuple<int, bool> InputInt(const std::string &label, int v, int step = 1, int step_fast = 100,
                                   int flags = 0);

    // Keep existing overloads
    std::tuple<int, bool> InputInt(const std::string &label, int v);

    std::tuple<int, bool> InputInt(const std::string &label, int v, int step);

    std::tuple<int, bool> InputInt(const std::string &label, int v, int step, int step_fast);

    // The overload that took flags (already correct)
    // inline std::tuple<int, bool> InputInt(const std::string &label, int v, int step, int step_fast, int flags) { ... } // already exists and is correct

    template<int N>
    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputIntN_Internal(
        const std::string &label, const sol::table &v_table, int flags) {
        int value[N];
        for (int i = 0; i < N; ++i) {
            value[i] = static_cast<int>(v_table[i + 1].get<std::optional<lua_Number> >().value_or(
                static_cast<lua_Number>(0)));
        }
        bool used = false;
        if (N == 2) used = ImGui::InputInt2(label.c_str(), value, flags);
        else if (N == 3) used = ImGui::InputInt3(label.c_str(), value, flags);
        else if (N == 4)
          used = ImGui::InputInt4(label.c_str(), value, flags);

        std::vector<int> result_vec;
        for (int i = 0; i < N; ++i) result_vec.push_back(value[i]);
        return std::make_tuple(sol::as_table(result_vec), used);
    }

    // InputInt2
    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputInt2(const std::string &label, const sol::table &v);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputInt2(const std::string &label, const sol::table &v,
                                                                   int flags);

    // InputInt3
    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputInt3(const std::string &label, const sol::table &v);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputInt3(const std::string &label, const sol::table &v,
                                                                   int flags);

    // InputInt4
    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputInt4(const std::string &label, const sol::table &v);

    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputInt4(const std::string &label, const sol::table &v,
                                                                   int flags);


    // InputDouble takes flags
    std::tuple<double, bool> InputDouble(const std::string &label, double v, double step = 0.0,
                                         double step_fast = 0.0, const char *format = "%.6f", int flags = 0);

    // Keep existing overloads
    std::tuple<double, bool> InputDouble(const std::string &label, double v);

    std::tuple<double, bool> InputDouble(const std::string &label, double v, double step);

    std::tuple<double, bool> InputDouble(const std::string &label, double v, double step, double step_fast);

    std::tuple<double, bool> InputDouble(const std::string &label, double v, double step, double step_fast,
                                         const std::string &format);

    // Overload that takes flags
    std::tuple<double, bool> InputDouble(const std::string &label, double v, double step, double step_fast,
                                         const std::string &format, int flags);

    void InputScalar();

    void InputScalarN();

    // Widgets: Color Editor / Picker
    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorEdit3(const std::string &label, const sol::table &col,
                                                                      int flags = 0);

    // Overload without flags
    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorEdit3(const std::string &label, const sol::table &col);


    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorEdit4(const std::string &label, const sol::table &col,
                                                                      int flags = 0);

    // Overload without flags
    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorEdit4(const std::string &label, const sol::table &col);


    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorPicker3(const std::string &label, const sol::table &col,
                                                                        int flags = 0);

    // Overload without flags
    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    ColorPicker3(const std::string &label, const sol::table &col);


    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorPicker4(
        const std::string &label, const sol::table &col, int flags = 0, const float *ref_col = NULL);

    // Overload without flags
    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    ColorPicker4(const std::string &label, const sol::table &col);

    // Overload with flags, no ref_col
    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorPicker4(const std::string &label, const sol::table &col,
                                                                        int flags);


    bool ColorButton(const std::string &desc_id, const sol::table &col, int flags = 0,
                     const ImVec2 &size = ImVec2(0, 0));

    // Overloads
    bool ColorButton(const std::string &desc_id, const sol::table &col);

    bool ColorButton(const std::string &desc_id, const sol::table &col, int flags);

    bool ColorButton(const std::string &desc_id, const sol::table &col, int flags, float sizeX, float sizeY);

    void SetColorEditOptions(int flags);

    // Widgets: Trees
    bool TreeNode(const std::string &label);

    bool TreeNode(const std::string &str_id, const std::string &fmt_or_label);

    bool TreeNodeEx(const std::string &label);

    bool TreeNodeEx(const std::string &label, int flags);

    bool TreeNodeEx(const std::string &str_id, int flags, const std::string &fmt_or_label);

    void TreePush(const std::string &str_id);

    void TreePop();

    float GetTreeNodeToLabelSpacing();

    bool CollapsingHeader(const std::string &label);

    bool CollapsingHeader(const std::string &label, int flags);

    std::tuple<bool, bool> CollapsingHeader(const std::string &label, bool open, int flags = 0);

    std::tuple<bool, bool> CollapsingHeader(const std::string &label, bool open);


    void SetNextItemOpen(bool is_open);

    void SetNextItemOpen(bool is_open, int cond);

    // Widgets: Selectables
    bool Selectable(const std::string &label, bool selected, int flags, const ImVec2 &size);

    // Overloads for Lua convenience
    bool Selectable(const std::string &label);

    // Read-only version
    bool Selectable(const std::string &label, bool selected);

    bool Selectable(const std::string &label, bool selected, int flags);

    bool Selectable(const std::string &label, bool selected, int flags, float sizeX, float sizeY);


    // Widgets: List Boxes
    std::tuple<int, bool> ListBox(const std::string &label, int current_item, const sol::table &items,
                                  int items_count, int height_in_items = -1);

    std::tuple<int, bool> ListBox(const std::string &label, int current_item, const sol::table &items,
                                  int items_count);

    // ListBoxHeader/Footer replaced by BeginListBox/EndListBox
    bool ListBoxHeader(const std::string &label, float sizeX, float sizeY);

    bool ListBoxHeader(const std::string &label, int items_count, int height_in_items = -1);

    void ListBoxFooter();

    // Widgets: Data Plotting
    /* TODO: Widgets Data Plotting ==> UNSUPPORTED (barely used and quite long functions) */

    // Widgets: Value() helpers
    void Value(const std::string &prefix, bool b);

    void Value(const std::string &prefix, int v);

    void Value(const std::string &prefix, unsigned int v);

    void Value(const std::string &prefix, float v, const char *float_format = NULL);

    void Value(const std::string &prefix, float v, const std::string &float_format_str);

    // Widgets: Menus
    bool BeginMenuBar();

    void EndMenuBar();

    bool BeginMainMenuBar();

    void EndMainMenuBar();

    bool BeginMenu(const std::string &label);

    bool BeginMenu(const std::string &label, bool enabled);

    void EndMenu();

    // MenuItem
    // Simple version returns true if activated
    bool MenuItem(const std::string &label);

    // Version with a shortcut returns true if activated
    bool MenuItem(const std::string &label, const std::string &shortcut);

    // Version with bool selected (read-only from Lua PoV), returns true if activated
    bool MenuItem(const std::string &label, const std::string &shortcut, bool selected);

    // Version with bool selected (read-only from Lua PoV) and enabled, returns true if activated
    bool MenuItem(const std::string &label, const std::string &shortcut, bool selected, bool enabled);

    // Version for Lua where `selected` is in-out, returns (new_selected_state, activated)
    std::tuple<bool, bool> MenuItemToggle(const std::string &label, const std::string &shortcut,
                                          bool selected_val, bool enabled = true);


    // Tooltips
    void BeginTooltip();

    void EndTooltip();

    void SetTooltip(const std::string &fmt_or_text);

    // New Item Tooltips
    bool BeginItemTooltip();

    void SetItemTooltip(const std::string &text);

    // Popups, Modals
    bool BeginPopup(const std::string &str_id);

    bool BeginPopup(const std::string &str_id, int flags);

    // BeginPopupModal now returns tuple if p_open is involved
    bool BeginPopupModal(const std::string &name);

    bool BeginPopupModal(const std::string &name, int flags);

    std::tuple<bool, bool> BeginPopupModal(const std::string &name, bool open_val, int flags = 0);

    // Backward compatible overload if Lua only passes name and open
    std::tuple<bool, bool> BeginPopupModal(const std::string &name, bool open_val);


    void EndPopup();

    void OpenPopup(const std::string &str_id);

    void OpenPopup(const std::string &str_id, int popup_flags);

    void OpenPopupOnItemClick(const std::string &str_id = "", int popup_flags = 1);

    void OpenPopupOnItemClick();


    // OpenPopupContextItem is obsolete, use BeginPopupContextItem
    // Keeping old signatures for BeginPopupContextItem for compatibility if possible (they now take flags)
    bool BeginPopupContextItem();

    bool BeginPopupContextItem(const std::string &str_id);

    bool BeginPopupContextItem(const std::string &str_id, int popup_flags);

    bool BeginPopupContextWindow();

    bool BeginPopupContextWindow(const std::string &str_id);

    bool BeginPopupContextWindow(const std::string &str_id, int popup_flags);

    bool BeginPopupContextVoid();

    bool BeginPopupContextVoid(const std::string &str_id);

    bool BeginPopupContextVoid(const std::string &str_id, int popup_flags);

    void CloseCurrentPopup();

    bool IsPopupOpen(const std::string &str_id);

    bool IsPopupOpen(const std::string &str_id, int popup_flags);

    // Columns (Legacy API)
    void Columns();

    void Columns(int count);

    void Columns(int count, const std::string &id);

    void Columns(int count, const std::string &id, bool border);

    void NextColumn();

    int GetColumnIndex();

    float GetColumnWidth();

    float GetColumnWidth(int column_index);

    void SetColumnWidth(int column_index, float width);

    float GetColumnOffset();

    float GetColumnOffset(int column_index);

    void SetColumnOffset(int column_index, float offset_x);

    int GetColumnsCount();

    // Tab Bars, Tabs
    bool BeginTabBar(const std::string &str_id);

    bool BeginTabBar(const std::string &str_id, int flags);

    void EndTabBar();

    // BeginTabItem tuple return for p_open
    bool BeginTabItem(const std::string &label);

    bool BeginTabItem(const std::string &label, int flags);

    std::tuple<bool, bool> BeginTabItem(const std::string &label, bool open_val, int flags = 0);

    // Keep original for compatibility if possible
    std::tuple<bool, bool> BeginTabItem(const std::string &label, bool open_val);


    void EndTabItem();

    void SetTabItemClosed(const std::string &tab_or_docked_window_label);

    // Docking
    // Note: Docking API might be part of imgui_internal.h or a specific branch (like docking branch)
    // These bindings assume they are available in the linked ImGui version.
    // If using standard master branch ImGui without docking, these might not compile or link.
#if (defined(IMGUI_HAS_DOCK) || defined(CUSTOM_IMGUI_HAS_DOCK)) // Heuristic check
    void DockSpace(ImGuiID id);
    void DockSpace(ImGuiID id, const ImVec2& size);
    void DockSpace(ImGuiID id, float sizeX, float sizeY);
    void DockSpace(ImGuiID id, float sizeX, float sizeY, int flags);
    // DockSpaceOverViewport might not be in all versions or require specific setup
    // inline unsigned int DockSpaceOverViewport() { return ImGui::DockSpaceOverViewport(); } // Requires Viewport* argument
    void SetNextWindowDockID(ImGuiID dock_id);
    void SetNextWindowDockID(ImGuiID dock_id, int cond);
    // inline void SetNextWindowClass(const ImGuiWindowClass* window_class) { ImGui::SetNextWindowClass(window_class); } // Complex type
    ImGuiID GetWindowDockID();
    bool IsWindowDocked();
#else
    // Provide stubs if docking is not available to avoid compile errors, though they won't do anything.
    void DockSpace(unsigned int);

    void DockSpace(unsigned int, float, float);

    void DockSpace(unsigned int, float, float, int);

    unsigned int DockSpaceOverViewport();

    void SetNextWindowDockID(unsigned int);

    void SetNextWindowDockID(unsigned int, int);

    void SetNextWindowClass();

    unsigned int GetWindowDockID();

    bool IsWindowDocked();
#endif

    // Logging
    void LogToTTY();

    void LogToTTY(int auto_open_depth);

    void LogToFile();

    void LogToFile(int auto_open_depth);

    // This overload was missing
    void LogToFile(int auto_open_depth, const std::string &filename);

    void LogToClipboard();

    void LogToClipboard(int auto_open_depth);

    void LogFinish();

    void LogButtons();

    void LogText(const std::string &text);

    // Drag and Drop
    // TODO: Drag and Drop ==> UNSUPPORTED (complex with payload handling, especially from Lua)

    // Clipping
    void PushClipRect(float min_x, float min_y, float max_x, float max_y, bool intersect_current);

    void PopClipRect();

    // Focus, Activation
    void SetItemDefaultFocus();

    void SetKeyboardFocusHere();

    void SetKeyboardFocusHere(int offset);

    // Item/Widgets Utilities
    bool IsItemHovered();

    bool IsItemHovered(int flags);

    bool IsItemActive();

    bool IsItemFocused();

    bool IsItemClicked();

    bool IsItemClicked(int mouse_button);

    bool IsItemVisible();

    bool IsItemEdited();

    bool IsItemActivated();

    bool IsItemDeactivated();

    bool IsItemDeactivatedAfterEdit();

    bool IsItemToggledOpen();

    bool IsAnyItemHovered();

    bool IsAnyItemActive();

    bool IsAnyItemFocused();

    std::tuple<float, float> GetItemRectMin();

    std::tuple<float, float> GetItemRectMax();

    std::tuple<float, float> GetItemRectSize();

    // SetItemAllowOverlap is obsolete, use SetNextItemAllowOverlap
    void SetNextItemAllowOverlap();


    // Miscellaneous Utilities
    bool IsRectVisible(float sizeX, float sizeY);

    bool IsRectVisible(float minX, float minY, float maxX, float maxY);

    double GetTime();

    int GetFrameCount();

    /* TODO: GetBackgroundDrawList(), GetForeGroundDrawList(), GetDrawListSharedData() ==> UNSUPPORTED */
    std::string GetStyleColorName(int idx);

    /* TODO: SetStateStorage(), GetStateStorage(), CalcListClipping() ==> UNSUPPORTED */

    // BeginChildFrame updated to use BeginChild
    bool BeginChildFrame(unsigned int id, float sizeX, float sizeY);

    bool BeginChildFrame(unsigned int id, float sizeX, float sizeY, int flags);

    void EndChildFrame();


    // Text Utilities
    // Simplified CalcTextSize for Lua to avoid const char* text_end complexity from Lua
    std::tuple<float, float> CalcTextSize(const std::string &text, bool hide_text_after_double_hash = false,
                                          float wrap_width = -1.0f);

    // Overloads for CalcTextSize
    std::tuple<float, float> CalcTextSize(const std::string &text);

    std::tuple<float, float> CalcTextSize(const std::string &text, bool hide_text_after_double_hash);


    // Color Utilities
#ifdef SOL_IMGUI_USE_COLOR_U32
	sol::as_table_t<std::vector<float>> ColorConvertU32ToFloat4(unsigned int in);
	unsigned int ColorConvertFloat4ToU32(const sol::table& rgba);
#endif
    std::tuple<float, float, float> ColorConvertRGBtoHSV(float r, float g, float b);

    std::tuple<float, float, float> ColorConvertHSVtoRGB(float h, float s, float v);

    // Inputs Utilities: Keyboard
    // GetKeyIndex is obsolete if using new key system. ImGuiKey is the index.
    // The binding `GetKeyIndex(int imgui_key)` where `imgui_key` is an `ImGuiKey` enum cast to int is fine.
    int GetKeyIndex(int imgui_key_enum_val);

    // Effectively a cast
    bool IsKeyDown(int user_key_index);

    bool IsKeyPressed(int user_key_index);

    bool IsKeyPressed(int user_key_index, bool repeat);

    bool IsKeyReleased(int user_key_index);

    int GetKeyPressedAmount(int key_index, float repeat_delay, float rate);

    // CaptureKeyboardFromApp -> SetNextFrameWantCaptureKeyboard
    void CaptureKeyboardFromApp();

    void CaptureKeyboardFromApp(bool want_capture_keyboard_value);

    // Inputs Utilities: Mouse
    bool IsMouseDown(int button);

    bool IsMouseClicked(int button);

    bool IsMouseClicked(int button, bool repeat);

    bool IsMouseReleased(int button);

    bool IsMouseDoubleClicked(int button);

    bool IsMouseHoveringRect(float min_x, float min_y, float max_x, float max_y);

    bool IsMouseHoveringRect(float min_x, float min_y, float max_x, float max_y, bool clip);

    bool IsMousePosValid();

    bool IsMousePosValid(float x, float y);

    bool IsAnyMouseDown();

    std::tuple<float, float> GetMousePos();

    std::tuple<float, float> GetMousePosOnOpeningCurrentPopup();

    bool IsMouseDragging(int button);

    bool IsMouseDragging(int button, float lock_threshold);

    std::tuple<float, float> GetMouseDragDelta();

    std::tuple<float, float> GetMouseDragDelta(int button);

    std::tuple<float, float> GetMouseDragDelta(int button, float lock_threshold);

    void ResetMouseDragDelta();

    void ResetMouseDragDelta(int button);

    int GetMouseCursor();

    void SetMouseCursor(int cursor_type);

    // CaptureMouseFromApp -> SetNextFrameWantCaptureMouse
    void CaptureMouseFromApp();

    void CaptureMouseFromApp(bool want_capture_mouse_value);

    // Clipboard Utilities
    std::string GetClipboardText();

    void SetClipboardText(const std::string &text);

    void InitEnums(sol::state &lua);

    void InitWindows(sol::table &imGuiTable);

    void InitLayout(sol::table &imGuiTable);

    void InitInput(sol::table &imGuiTable);

    void InitWidgets(sol::table &imGuiTable);

    void InitPopups(sol::table &imGuiTable);

    void InitTables(sol::table &imGuiTable);

    void InitUtility(sol::table &imGuiTable);

    void Init(sol::state &lua);
} // End of namespace sol_ImGui
