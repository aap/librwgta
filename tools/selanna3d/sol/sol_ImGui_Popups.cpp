#include "sol_ImGui.h"

namespace sol_ImGui {
    void BeginTooltip() { ImGui::BeginTooltip(); }

    void EndTooltip() { ImGui::EndTooltip(); }

    void SetTooltip(const std::string &fmt_or_text) { ImGui::SetTooltip("%s", fmt_or_text.c_str()); }

    bool BeginItemTooltip() { return ImGui::BeginItemTooltip(); }

    void SetItemTooltip(const std::string &text) { ImGui::SetItemTooltip("%s", text.c_str()); }

    bool BeginPopup(const std::string &str_id) { return ImGui::BeginPopup(str_id.c_str()); }

    bool BeginPopup(const std::string &str_id, int flags) { return ImGui::BeginPopup(str_id.c_str(), flags); }

    bool BeginPopupModal(const std::string &name) { return ImGui::BeginPopupModal(name.c_str(), nullptr, 0); }

    bool BeginPopupModal(const std::string &name, int flags) {
        // Overload with flags, no p_open
        return ImGui::BeginPopupModal(name.c_str(), nullptr, flags);
    }

    std::tuple<bool, bool> BeginPopupModal(const std::string &name, bool open_val, int flags) {
        bool p_open = open_val;
        bool displayed = ImGui::BeginPopupModal(name.c_str(), &p_open, flags);
        return std::make_tuple(p_open, displayed);
    }

    std::tuple<bool, bool> BeginPopupModal(const std::string &name, bool open_val) {
        return BeginPopupModal(name, open_val, 0);
    }

    void EndPopup() { ImGui::EndPopup(); }

    void OpenPopup(const std::string &str_id) { ImGui::OpenPopup(str_id.c_str()); }

    void OpenPopup(const std::string &str_id, int popup_flags) { ImGui::OpenPopup(str_id.c_str(), popup_flags); }

    void OpenPopupOnItemClick(const std::string &str_id, int popup_flags) {
        ImGui::OpenPopupOnItemClick(str_id.empty() ? nullptr : str_id.c_str(), popup_flags);
    }

    void OpenPopupOnItemClick() { ImGui::OpenPopupOnItemClick(nullptr, 1); }

    bool BeginPopupContextItem() {
        return ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight); }

    bool BeginPopupContextItem(const std::string &str_id) {
        return ImGui::BeginPopupContextItem(str_id.c_str(), ImGuiPopupFlags_MouseButtonRight);
    }

    bool BeginPopupContextItem(const std::string &str_id, int popup_flags) {
        return ImGui::BeginPopupContextItem(str_id.c_str(), popup_flags);
    }

    bool BeginPopupContextWindow() {
        return ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight); }

    bool BeginPopupContextWindow(const std::string &str_id) {
        return ImGui::BeginPopupContextWindow(str_id.c_str(), ImGuiPopupFlags_MouseButtonRight);
    }

    bool BeginPopupContextWindow(const std::string &str_id, int popup_flags) {
        return ImGui::BeginPopupContextWindow(str_id.c_str(), popup_flags);
    }

    bool BeginPopupContextVoid() {
        return ImGui::BeginPopupContextVoid(nullptr, ImGuiPopupFlags_MouseButtonRight); }

    bool BeginPopupContextVoid(const std::string &str_id) {
        return ImGui::BeginPopupContextVoid(str_id.c_str(), ImGuiPopupFlags_MouseButtonRight);
    }

    bool BeginPopupContextVoid(const std::string &str_id, int popup_flags) {
        return ImGui::BeginPopupContextVoid(str_id.c_str(), popup_flags);
    }

    void CloseCurrentPopup() { ImGui::CloseCurrentPopup(); }

    bool IsPopupOpen(const std::string &str_id) { return ImGui::IsPopupOpen(str_id.c_str()); }

    bool IsPopupOpen(const std::string &str_id, int popup_flags) {
        return ImGui::IsPopupOpen(str_id.c_str(), popup_flags);
    }

#pragma region Init
    void Init_Tooltips(sol::table &imGuiTable) {
        imGuiTable.set_function("BeginTooltip", BeginTooltip);
        imGuiTable.set_function("EndTooltip", EndTooltip);
        imGuiTable.set_function("SetTooltip", SetTooltip);
        imGuiTable.set_function("BeginItemTooltip", BeginItemTooltip);
        imGuiTable.set_function("SetItemTooltip", SetItemTooltip);
    }

    void Init_PopupsModals(sol::table &imGuiTable) {
        imGuiTable.set_function("BeginPopup", sol::overload(
                                    sol::resolve<bool(const std::string &)>(BeginPopup),
                                    sol::resolve<bool(const std::string &, int)>(BeginPopup)
                                ));
        imGuiTable.set_function("BeginPopupModal", sol::overload(
                                    sol::resolve<bool(const std::string &)>(BeginPopupModal), // No p_open
                                    sol::resolve<bool(const std::string &, int)>(BeginPopupModal),
                                    // No p_open, with flags
                                    sol::resolve<std::tuple<bool, bool>(const std::string &, bool)>(BeginPopupModal),
                                    // With p_open
                                    sol::resolve<std::tuple<bool, bool>(const std::string &, bool, int)>(
                                        BeginPopupModal)
                                    // With p_open and flags
                                ));
        imGuiTable.set_function("EndPopup", EndPopup);
        imGuiTable.set_function("OpenPopup", sol::overload(
                                    sol::resolve<void(const std::string &)>(OpenPopup),
                                    sol::resolve<void(const std::string &, int)>(OpenPopup)
                                ));
        imGuiTable.set_function("OpenPopupOnItemClick", sol::overload(
                                    sol::resolve<void()>(OpenPopupOnItemClick),
                                    sol::resolve<void(const std::string &, int)>(OpenPopupOnItemClick)
                                ));
        imGuiTable.set_function("CloseCurrentPopup", CloseCurrentPopup);
        imGuiTable.set_function("BeginPopupContextItem", sol::overload(
                                    sol::resolve<bool()>(BeginPopupContextItem),
                                    sol::resolve<bool(const std::string &)>(BeginPopupContextItem),
                                    sol::resolve<bool(const std::string &, int)>(BeginPopupContextItem)
                                ));
        imGuiTable.set_function("BeginPopupContextWindow", sol::overload(
                                    sol::resolve<bool()>(BeginPopupContextWindow),
                                    sol::resolve<bool(const std::string &)>(BeginPopupContextWindow),
                                    sol::resolve<bool(const std::string &, int)>(BeginPopupContextWindow)
                                ));
        imGuiTable.set_function("BeginPopupContextVoid", sol::overload(
                                    sol::resolve<bool()>(BeginPopupContextVoid),
                                    sol::resolve<bool(const std::string &)>(BeginPopupContextVoid),
                                    sol::resolve<bool(const std::string &, int)>(BeginPopupContextVoid)
                                ));
        imGuiTable.set_function("IsPopupOpen", sol::overload(
                                    sol::resolve<bool(const std::string &)>(IsPopupOpen),
                                    sol::resolve<bool(const std::string &, int)>(IsPopupOpen)
                                ));
    }

    void InitPopups(sol::table &imGuiTable) {
        Init_Tooltips(imGuiTable);
        Init_PopupsModals(imGuiTable);
    }
#pragma endregion Init
}
