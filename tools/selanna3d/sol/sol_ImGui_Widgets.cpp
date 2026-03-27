#include "sol_ImGui.h"

namespace sol_ImGui {
    void TextUnformatted(const std::string &text) { ImGui::TextUnformatted(text.c_str()); }

    void TextUnformatted(const std::string &text, const std::string &textEnd) {
        ImGui::TextUnformatted(text.c_str(), textEnd.c_str());
    }

    void Text(const std::string &text) { ImGui::Text("%s", text.c_str()); }

    void TextColored(float colR, float colG, float colB, float colA, const std::string &text) {
        ImGui::TextColored({colR, colG, colB, colA}, "%s", text.c_str());
    }

    void TextDisabled(const std::string &text) { ImGui::TextDisabled("%s", text.c_str()); }

    void TextWrapped(const std::string text) { ImGui::TextWrapped("%s", text.c_str()); }

    void LabelText(const std::string &label, const std::string &text) {
        ImGui::LabelText(label.c_str(), "%s", text.c_str());
    }

    void BulletText(const std::string &text) { ImGui::BulletText("%s", text.c_str()); }

    void Image() {
        /* TODO: Image(...) ==> UNSUPPORTED (ImTextureID binding) */
    }

    void ImageButton() {
        /* TODO: ImageButton(...) ==> UNSUPPORTED (ImTextureID binding) */
    }

    void ProgressBar(float fraction) { ImGui::ProgressBar(fraction); }

    void ProgressBar(float fraction, float sizeX, float sizeY) { ImGui::ProgressBar(fraction, {sizeX, sizeY}); }

    void ProgressBar(float fraction, float sizeX, float sizeY, const std::string &overlay) {
        ImGui::ProgressBar(fraction, {sizeX, sizeY}, overlay.c_str());
    }

    void Bullet() { ImGui::Bullet(); }

    bool BeginCombo(const std::string &label, const std::string &previewValue) {
        return ImGui::BeginCombo(label.c_str(), previewValue.c_str());
    }

    bool BeginCombo(const std::string &label, const std::string &previewValue, int flags) {
      return ImGui::BeginCombo(label.c_str(), previewValue.c_str(), flags);
    }

    void EndCombo() { ImGui::EndCombo(); }

    std::tuple<int, bool> Combo(const std::string &label, int currentItem, const sol::table &items, int itemsCount) {
        std::vector<std::string> strings;
        for (int i{1}; i <= itemsCount; i++) {
            // Lua tables are 1-indexed
            const auto &stringItem = items.get<sol::optional<std::string> >(i);
            strings.push_back(stringItem.value_or("Missing"));
        }

        std::vector<const char *> cstrings;
        for (auto &string_item: strings)
            cstrings.push_back(string_item.c_str());

        int current_item_ref = currentItem; // Pass by reference to ImGui::Combo
        bool clicked = ImGui::Combo(label.c_str(), &current_item_ref, cstrings.data(), itemsCount);
        return std::make_tuple(current_item_ref, clicked);
    }

    std::tuple<int, bool> Combo(const std::string &label, int currentItem, const sol::table &items, int itemsCount,
                                int popupMaxHeightInItems) {
        std::vector<std::string> strings;
        for (int i{1}; i <= itemsCount; i++) {
            // Lua tables are 1-indexed
            const auto &stringItem = items.get<sol::optional<std::string> >(i);
            strings.push_back(stringItem.value_or("Missing"));
        }

        std::vector<const char *> cstrings;
        for (auto &string_item: strings)
            cstrings.push_back(string_item.c_str());

        int current_item_ref = currentItem; // Pass by reference
        bool clicked = ImGui::Combo(label.c_str(), &current_item_ref, cstrings.data(), itemsCount,
                                    popupMaxHeightInItems);
        return std::make_tuple(current_item_ref, clicked);
    }

    std::tuple<int, bool> Combo(const std::string &label, int currentItem, const std::string &itemsSeparatedByZeros) {
        int current_item_ref = currentItem; // Pass by reference
        bool clicked = ImGui::Combo(label.c_str(), &current_item_ref, itemsSeparatedByZeros.c_str());
        return std::make_tuple(current_item_ref, clicked);
    }

    std::tuple<int, bool> Combo(const std::string &label, int currentItem, const std::string &itemsSeparatedByZeros,
                                int popupMaxHeightInItems) {
        int current_item_ref = currentItem; // Pass by reference
        bool clicked = ImGui::Combo(label.c_str(), &current_item_ref, itemsSeparatedByZeros.c_str(),
                                    popupMaxHeightInItems);
        return std::make_tuple(current_item_ref, clicked);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorEdit3(const std::string &label, const sol::table &col,
                                                                      int flags) {
        // Added default for flags
        const lua_Number r{col[1].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                g{col[2].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                b{col[3].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))};
        float color_arr[3] = {float(r), float(g), float(b)}; // Renamed to avoid conflict
        bool used = ImGui::ColorEdit3(label.c_str(), color_arr, flags);

        sol::as_table_t rgb = sol::as_table(std::vector{color_arr[0], color_arr[1], color_arr[2]});
        return std::make_tuple(rgb, used);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorEdit3(const std::string &label, const sol::table &col) {
        return ColorEdit3(label, col, 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorEdit4(const std::string &label, const sol::table &col,
                                                                      int flags) {
        // Added default for flags
        const lua_Number r{col[1].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                g{col[2].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                b{col[3].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                a{
                    col[4].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(1.0f))
                }; // Default alpha to 1.0f
        float color_arr[4] = {float(r), float(g), float(b), float(a)}; // Renamed
        bool used = ImGui::ColorEdit4(label.c_str(), color_arr, flags);

        sol::as_table_t rgba = sol::as_table(std::vector{color_arr[0], color_arr[1], color_arr[2], color_arr[3]
        });
        return std::make_tuple(rgba, used);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorEdit4(const std::string &label, const sol::table &col) {
        return ColorEdit4(label, col, 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorPicker3(const std::string &label, const sol::table &col,
                                                                        int flags) {
        // Added default for flags
        const lua_Number r{col[1].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                g{col[2].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                b{col[3].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))};
        float color_arr[3] = {float(r), float(g), float(b)}; // Renamed
        bool used = ImGui::ColorPicker3(label.c_str(), color_arr, flags);

        sol::as_table_t rgb = sol::as_table(std::vector{color_arr[0], color_arr[1], color_arr[2]});
        return std::make_tuple(rgb, used);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    ColorPicker3(const std::string &label, const sol::table &col) {
        return ColorPicker3(label, col, 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorPicker4(const std::string &label, const sol::table &col,
                                                                        int flags, const float *ref_col) {
        // Added default for flags & ref_col
        const lua_Number r{col[1].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                g{col[2].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                b{col[3].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                a{
                    col[4].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(1.0f))
                }; // Default alpha to 1.0f
        float color_arr[4] = {float(r), float(g), float(b), float(a)}; // Renamed
        // Note: ref_col is not easily passed from Lua. Keeping it NULL.
        bool used = ImGui::ColorPicker4(label.c_str(), color_arr, flags, ref_col);

        sol::as_table_t rgba = sol::as_table(std::vector{color_arr[0], color_arr[1], color_arr[2], color_arr[3]
        });
        return std::make_tuple(rgba, used);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool>
    ColorPicker4(const std::string &label, const sol::table &col) {
        return ColorPicker4(label, col, 0, NULL);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> ColorPicker4(const std::string &label, const sol::table &col,
                                                                        int flags) {
        return ColorPicker4(label, col, flags, NULL);
    }

    void SetColorEditOptions(int flags) { ImGui::SetColorEditOptions(flags); }

    bool TreeNode(const std::string &label) { return ImGui::TreeNode(label.c_str()); }

    bool TreeNode(const std::string &str_id, const std::string &fmt_or_label) {
        return ImGui::TreeNode(str_id.c_str(), "%s", fmt_or_label.c_str());
    }

    bool TreeNodeEx(const std::string &label) { return ImGui::TreeNodeEx(label.c_str()); }

    bool TreeNodeEx(const std::string &label, int flags) {
        return ImGui::TreeNodeEx(label.c_str(), flags); }

    bool TreeNodeEx(const std::string &str_id, int flags, const std::string &fmt_or_label) {
        return ImGui::TreeNodeEx(str_id.c_str(), flags, "%s", fmt_or_label.c_str());
    }

    void TreePush(const std::string &str_id) { ImGui::TreePush(str_id.c_str()); }

    void TreePop() { ImGui::TreePop(); }

    float GetTreeNodeToLabelSpacing() { return ImGui::GetTreeNodeToLabelSpacing(); }

    bool CollapsingHeader(const std::string &label) { return ImGui::CollapsingHeader(label.c_str()); }

    bool CollapsingHeader(const std::string &label, int flags) {
        return ImGui::CollapsingHeader(label.c_str(), flags); }

    std::tuple<bool, bool> CollapsingHeader(const std::string &label, bool open, int flags) {
        bool p_open_value = open;
        bool notCollapsed = ImGui::CollapsingHeader(label.c_str(), &p_open_value, flags);
        return std::make_tuple(p_open_value, notCollapsed);
    }

    std::tuple<bool, bool> CollapsingHeader(const std::string &label, bool open) {
        return CollapsingHeader(label, open, 0);
    }

    void SetNextItemOpen(bool is_open) { ImGui::SetNextItemOpen(is_open); }

    void SetNextItemOpen(bool is_open, int cond) {
        ImGui::SetNextItemOpen(is_open, cond);
    }

    void Value(const std::string &prefix, bool b) { ImGui::Value(prefix.c_str(), b); }

    void Value(const std::string &prefix, int v) { ImGui::Value(prefix.c_str(), v); }

    void Value(const std::string &prefix, unsigned int v) { ImGui::Value(prefix.c_str(), v); }

    void Value(const std::string &prefix, float v, const char *float_format) {
        ImGui::Value(prefix.c_str(), v, float_format);
    }

    void Value(const std::string &prefix, float v, const std::string &float_format_str) {
        ImGui::Value(prefix.c_str(), v, float_format_str.c_str());
    }

    void SetItemDefaultFocus() { ImGui::SetItemDefaultFocus(); }

    void SetKeyboardFocusHere() { ImGui::SetKeyboardFocusHere(); }

    void SetKeyboardFocusHere(int offset) { ImGui::SetKeyboardFocusHere(offset); }

#pragma region Init
    void Init_WidgetsText(sol::table &imGuiTable) {
        imGuiTable.set_function("TextUnformatted", sol::overload(
                                    sol::resolve<void(const std::string &)>(TextUnformatted),
                                    sol::resolve<void(const std::string &, const std::string &)>(TextUnformatted)
                                ));
        imGuiTable.set_function("Text", Text); // Assumes %s formatting for single string
        imGuiTable.set_function("TextColored", TextColored); // Assumes %s formatting
        imGuiTable.set_function("TextDisabled", TextDisabled); // Assumes %s formatting
        imGuiTable.set_function("TextWrapped", TextWrapped); // Assumes %s formatting
        imGuiTable.set_function("LabelText", LabelText); // Assumes %s formatting
        imGuiTable.set_function("BulletText", BulletText); // Assumes %s formatting
    }

    void Init_WidgetsComboBox(sol::table &imGuiTable) {
        imGuiTable.set_function("BeginCombo", sol::overload(
                                    sol::resolve<bool(const std::string &, const std::string &)>(BeginCombo),
                                    sol::resolve<bool(const std::string &, const std::string &, int)>(BeginCombo)
                                ));
        imGuiTable.set_function("EndCombo", EndCombo);
        imGuiTable.set_function("Combo", sol::overload(
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, const sol::table &,
                                                                       int)>(
                                        Combo),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, const sol::table &,
                                                                       int,
                                                                       int)>(Combo),
                                    sol::resolve<std::tuple<int, bool>
                                        (const std::string &, int, const std::string &)>(Combo),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, const std::string &,
                                                                       int)>(
                                        Combo)
                                ));
    }

    void Init_WidgetsColorEditorPicker(sol::table &imGuiTable) {
        imGuiTable.set_function("ColorEdit3", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &)>(ColorEdit3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, int)>(ColorEdit3)
                                ));
        imGuiTable.set_function("ColorEdit4", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &)>(ColorEdit4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, int)>(ColorEdit4)
                                ));
        imGuiTable.set_function("ColorPicker3", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &)>(ColorPicker3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, int)>(ColorPicker3)
                                ));
        imGuiTable.set_function("ColorPicker4", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &)>(ColorPicker4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, int)>(ColorPicker4)
                                    // Skipping ref_col version for simplicity from Lua
                                ));
        imGuiTable.set_function("ColorButton", sol::overload(
                                    sol::resolve<bool(const std::string &, const sol::table &)>(ColorButton),
                                    sol::resolve<bool(const std::string &, const sol::table &, int)>(ColorButton),
                                    sol::resolve<bool(const std::string &, const sol::table &, int, float, float)>(
                                        ColorButton)
                                ));
        imGuiTable.set_function("SetColorEditOptions", SetColorEditOptions);
    }

    void Init_WidgetsTrees(sol::table &imGuiTable) {
        imGuiTable.set_function("TreeNode", sol::overload(
                                    sol::resolve<bool(const std::string &)>(TreeNode),
                                    sol::resolve<bool(const std::string &, const std::string &)>(TreeNode)
                                ));
        imGuiTable.set_function("TreeNodeEx", sol::overload(
                                    sol::resolve<bool(const std::string &)>(TreeNodeEx),
                                    sol::resolve<bool(const std::string &, int)>(TreeNodeEx),
                                    sol::resolve<bool(const std::string &, int, const std::string &)>(TreeNodeEx)
                                ));
        imGuiTable.set_function("TreePush", TreePush);
        imGuiTable.set_function("TreePop", TreePop); // Added
        imGuiTable.set_function("GetTreeNodeToLabelSpacing", GetTreeNodeToLabelSpacing);
        imGuiTable.set_function("CollapsingHeader", sol::overload(
                                    sol::resolve<bool(const std::string &)>(CollapsingHeader),
                                    sol::resolve<bool(const std::string &, int)>(CollapsingHeader),
                                    sol::resolve<std::tuple<bool, bool>(const std::string &, bool)>(CollapsingHeader),
                                    sol::resolve<std::tuple<bool, bool>(const std::string &, bool, int)>(
                                        CollapsingHeader)
                                ));
        imGuiTable.set_function("SetNextItemOpen", sol::overload(
                                    sol::resolve<void(bool)>(SetNextItemOpen),
                                    sol::resolve<void(bool, int)>(SetNextItemOpen)
                                ));
    }

    void Init_WidgetsValueHelpers(sol::table &imGuiTable) {
        imGuiTable.set_function("Value", sol::overload(
                                    sol::resolve<void(const std::string &, bool)>(Value),
                                    sol::resolve<void(const std::string &, int)>(Value),
                                    sol::resolve<void(const std::string &, unsigned int)>(Value),
                                    sol::resolve<void(const std::string &, float, const char *)>(Value),
                                    sol::resolve<void(const std::string &, float, const std::string &)>(Value)
                                ));
    }

    void Init_FocusActivation(sol::table &imGuiTable) {
        imGuiTable.set_function("SetItemDefaultFocus", SetItemDefaultFocus);
        imGuiTable.set_function("SetKeyboardFocusHere", sol::overload(
                                    sol::resolve<void()>(SetKeyboardFocusHere),
                                    sol::resolve<void(int)>(SetKeyboardFocusHere)
                                ));
    }

    void InitWidgets(sol::table &imGuiTable) {
        Init_WidgetsText(imGuiTable);
        Init_WidgetsComboBox(imGuiTable);
        Init_WidgetsColorEditorPicker(imGuiTable);
        Init_WidgetsTrees(imGuiTable);
        Init_WidgetsValueHelpers(imGuiTable);
        Init_FocusActivation(imGuiTable);
    }
#pragma endregion Init
}
