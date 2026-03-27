#include "sol_ImGui.h"

namespace sol_ImGui {
    void Init(sol::state &lua) {
        InitEnums(lua);

        sol::table imGuiTable = lua.create_named_table("ImGui");
        InitWindows(imGuiTable);
        InitLayout(imGuiTable);
        InitInput(imGuiTable);
        InitWidgets(imGuiTable);
        InitPopups(imGuiTable);
        InitTables(imGuiTable);
        InitUtility(imGuiTable);
    }
}
