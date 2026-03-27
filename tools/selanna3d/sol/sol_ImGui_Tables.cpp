#include "sol_ImGui.h"

namespace sol_ImGui {
    void Columns() { ImGui::Columns(); }

    void Columns(int count) { ImGui::Columns(count); }

    void Columns(int count, const std::string &id) { ImGui::Columns(count, id.c_str()); }

    void Columns(int count, const std::string &id, bool border) { ImGui::Columns(count, id.c_str(), border); }

    void NextColumn() { ImGui::NextColumn(); }

    int GetColumnIndex() { return ImGui::GetColumnIndex(); }

    float GetColumnWidth() { return ImGui::GetColumnWidth(); }

    float GetColumnWidth(int column_index) { return ImGui::GetColumnWidth(column_index); }

    void SetColumnWidth(int column_index, float width) { ImGui::SetColumnWidth(column_index, width); }

    float GetColumnOffset() { return ImGui::GetColumnOffset(); }

    float GetColumnOffset(int column_index) { return ImGui::GetColumnOffset(column_index); }

    void SetColumnOffset(int column_index, float offset_x) { ImGui::SetColumnOffset(column_index, offset_x); }

    int GetColumnsCount() { return ImGui::GetColumnsCount(); }

#pragma region Init
    void InitTables(sol::table &imGuiTable) {
        imGuiTable.set_function("Columns", sol::overload(
                                    sol::resolve<void()>(Columns),
                                    sol::resolve<void(int)>(Columns),
                                    sol::resolve<void(int, const std::string &)>(Columns),
                                    sol::resolve<void(int, const std::string &, bool)>(Columns)
                                ));
        imGuiTable.set_function("NextColumn", NextColumn);
        imGuiTable.set_function("GetColumnIndex", GetColumnIndex);
        imGuiTable.set_function("GetColumnWidth", sol::overload(
                                    sol::resolve<float()>(GetColumnWidth),
                                    sol::resolve<float(int)>(GetColumnWidth)
                                ));
        imGuiTable.set_function("SetColumnWidth", SetColumnWidth);
        imGuiTable.set_function("GetColumnOffset", sol::overload(
                                    sol::resolve<float()>(GetColumnOffset),
                                    sol::resolve<float(int)>(GetColumnOffset)
                                ));
        imGuiTable.set_function("SetColumnOffset", SetColumnOffset);
        imGuiTable.set_function("GetColumnsCount", GetColumnsCount);
    }
#pragma endregion Init
}
