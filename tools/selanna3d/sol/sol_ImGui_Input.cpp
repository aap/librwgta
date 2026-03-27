#include "sol_ImGui.h"

namespace sol_ImGui {
    void PushID(const std::string &stringID) { ImGui::PushID(stringID.c_str()); }

    void PushID(const std::string &stringIDBegin, const std::string &stringIDEnd) {
        ImGui::PushID(stringIDBegin.c_str(), stringIDEnd.c_str());
    }

    void PushID(const void *) {
        /* TODO: PushID(void*) ==> UNSUPPORTED */
    }

    void PushID(int intID) { ImGui::PushID(intID); }

    void PopID() { ImGui::PopID(); }

    int GetID(const std::string &stringID) { return ImGui::GetID(stringID.c_str()); }

    int GetID(const std::string &stringIDBegin, const std::string &stringIDEnd) {
        return ImGui::GetID(stringIDBegin.c_str(), stringIDEnd.c_str());
    }

    int GetID(const void *) {
        return 0; /* TODO: GetID(void*) ==> UNSUPPORTED */
    }

    std::tuple<float, bool> DragFloat(const std::string &label, float v, float v_speed, float v_min, float v_max,
                                      const char *format, int flags) {
        float current_v = v; // Pass by reference
        bool used = ImGui::DragFloat(label.c_str(), &current_v, v_speed, v_min, v_max, format,
                                     flags);
        return std::make_tuple(current_v, used);
    }

    bool Button(const std::string &label) { return ImGui::Button(label.c_str()); }

    bool Button(const std::string &label, float sizeX, float sizeY) {
        return ImGui::Button(label.c_str(), {sizeX, sizeY});
    }

    bool SmallButton(const std::string &label) { return ImGui::SmallButton(label.c_str()); }

    bool InvisibleButton(const std::string &stringID, float sizeX, float sizeY) {
        return ImGui::InvisibleButton(stringID.c_str(), {sizeX, sizeY});
    }

    bool InvisibleButton(const std::string &str_id, float sizeX, float sizeY, int flags) {
        return ImGui::InvisibleButton(str_id.c_str(), {sizeX, sizeY}, flags);
    }

    bool ArrowButton(const std::string &stringID, int dir) {
        return ImGui::ArrowButton(stringID.c_str(), static_cast<ImGuiDir>(dir));
    }

    std::tuple<bool, bool> Checkbox(const std::string &label, bool v) {
        bool value{v};
        bool pressed = ImGui::Checkbox(label.c_str(), &value);

        return std::make_tuple(value, pressed);
    }

    std::tuple<int, bool> CheckboxFlags(const std::string &label, int current_flags, int flags_value) {
        int temp_flags = current_flags;
        bool changed = ImGui::CheckboxFlags(label.c_str(), &temp_flags, flags_value);
        return std::make_tuple(temp_flags, changed);
    }

    std::tuple<unsigned int, bool> CheckboxFlags(const std::string &label, unsigned int current_flags,
                                                 unsigned int flags_value) {
        unsigned int temp_flags = current_flags;
        bool changed = ImGui::CheckboxFlags(label.c_str(), &temp_flags, flags_value);
        return std::make_tuple(temp_flags, changed);
    }

    bool RadioButton(const std::string &label, bool active) { return ImGui::RadioButton(label.c_str(), active); }

    std::tuple<int, bool> RadioButton(const std::string &label, int v, int vButton) {
        // ImGui::RadioButton takes int* v, so we need to manage the value.
        int current_v = v;
        bool ret{ImGui::RadioButton(label.c_str(), &current_v, vButton)};
        return std::make_tuple(current_v, ret);
    }

    std::tuple<float, bool> DragFloat(const std::string &label, float v) {
        return DragFloat(label, v, 1.0f, 0.0f, 0.0f, "%.3f", 0);
    }

    std::tuple<float, bool> DragFloat(const std::string &label, float v, float v_speed) {
        return DragFloat(label, v, v_speed, 0.0f, 0.0f, "%.3f", 0);
    }

    std::tuple<float, bool> DragFloat(const std::string &label, float v, float v_speed, float v_min) {
        return DragFloat(label, v, v_speed, v_min, 0.0f, "%.3f", 0);
    }

    std::tuple<float, bool> DragFloat(const std::string &label, float v, float v_speed, float v_min, float v_max) {
        return DragFloat(label, v, v_speed, v_min, v_max, "%.3f", 0);
    }

    std::tuple<float, bool> DragFloat(const std::string &label, float v, float v_speed, float v_min, float v_max,
                                      const std::string &format) {
        return DragFloat(label, v, v_speed, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<float, bool> DragFloat(const std::string &label, float v, float v_speed, float v_min, float v_max,
                                      const std::string &format, int flags) {
        float current_v = v; // Pass by reference
        bool used = ImGui::DragFloat(label.c_str(), &current_v, v_speed, v_min, v_max, format.c_str(),
                                     flags);
        return std::make_tuple(current_v, used);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat2(const std::string &label, const sol::table &v) {
        return DragFloatN_Internal<2>(label, v, 1.0f, 0.0f, 0.0f, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat2(const std::string &label, const sol::table &v,
                                                                      float v_speed) {
        return DragFloatN_Internal<2>(label, v, v_speed, 0.0f, 0.0f, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat2(const std::string &label, const sol::table &v,
                                                                      float v_speed, float v_min) {
        return DragFloatN_Internal<2>(label, v, v_speed, v_min, 0.0f, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat2(const std::string &label, const sol::table &v,
                                                                      float v_speed, float v_min, float v_max) {
        return DragFloatN_Internal<2>(label, v, v_speed, v_min, v_max, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat2(const std::string &label, const sol::table &v,
                                                                      float v_speed, float v_min, float v_max,
                                                                      const std::string &format) {
        return DragFloatN_Internal<2>(label, v, v_speed, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat2(const std::string &label, const sol::table &v,
                                                                      float v_speed, float v_min, float v_max,
                                                                      const std::string &format, int flags) {
        return DragFloatN_Internal<2>(label, v, v_speed, v_min, v_max, format.c_str(), flags);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat3(const std::string &label, const sol::table &v) {
        return DragFloatN_Internal<3>(label, v, 1.0f, 0.0f, 0.0f, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat3(const std::string &label, const sol::table &v,
                                                                      float v_speed) {
        return DragFloatN_Internal<3>(label, v, v_speed, 0.0f, 0.0f, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat3(const std::string &label, const sol::table &v,
                                                                      float v_speed, float v_min) {
        return DragFloatN_Internal<3>(label, v, v_speed, v_min, 0.0f, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat3(const std::string &label, const sol::table &v,
                                                                      float v_speed, float v_min, float v_max) {
        return DragFloatN_Internal<3>(label, v, v_speed, v_min, v_max, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat3(const std::string &label, const sol::table &v,
                                                                      float v_speed, float v_min, float v_max,
                                                                      const std::string &format) {
        return DragFloatN_Internal<3>(label, v, v_speed, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat3(const std::string &label, const sol::table &v,
                                                                      float v_speed, float v_min, float v_max,
                                                                      const std::string &format, int flags) {
        return DragFloatN_Internal<3>(label, v, v_speed, v_min, v_max, format.c_str(), flags);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat4(const std::string &label, const sol::table &v) {
        return DragFloatN_Internal<4>(label, v, 1.0f, 0.0f, 0.0f, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat4(const std::string &label, const sol::table &v,
                                                                      float v_speed) {
        return DragFloatN_Internal<4>(label, v, v_speed, 0.0f, 0.0f, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat4(const std::string &label, const sol::table &v,
                                                                      float v_speed, float v_min) {
        return DragFloatN_Internal<4>(label, v, v_speed, v_min, 0.0f, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat4(const std::string &label, const sol::table &v,
                                                                      float v_speed, float v_min, float v_max) {
        return DragFloatN_Internal<4>(label, v, v_speed, v_min, v_max, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat4(const std::string &label, const sol::table &v,
                                                                      float v_speed, float v_min, float v_max,
                                                                      const std::string &format) {
        return DragFloatN_Internal<4>(label, v, v_speed, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> DragFloat4(const std::string &label, const sol::table &v,
                                                                      float v_speed, float v_min, float v_max,
                                                                      const std::string &format, int flags) {
        return DragFloatN_Internal<4>(label, v, v_speed, v_min, v_max, format.c_str(), flags);
    }

    void DragFloatRange2() {
        /* TODO: DragFloatRange2(...) ==> UNSUPPORTED */
    }

    std::tuple<int, bool> DragInt(const std::string &label, int v, float v_speed, int v_min, int v_max,
                                  const char *format, int flags) {
        int current_v = v; // Pass by reference
        bool used = ImGui::DragInt(label.c_str(), &current_v, v_speed, v_min, v_max, format,
                                   flags);
        return std::make_tuple(current_v, used);
    }

    std::tuple<int, bool> DragInt(const std::string &label, int v) {
        return DragInt(label, v, 1.0f, 0, 0, "%d", 0);
    }

    std::tuple<int, bool> DragInt(const std::string &label, int v, float v_speed) {
        return DragInt(label, v, v_speed, 0, 0, "%d", 0);
    }

    std::tuple<int, bool> DragInt(const std::string &label, int v, float v_speed, int v_min) {
        return DragInt(label, v, v_speed, v_min, 0, "%d", 0);
    }

    std::tuple<int, bool> DragInt(const std::string &label, int v, float v_speed, int v_min, int v_max) {
        return DragInt(label, v, v_speed, v_min, v_max, "%d", 0);
    }

    std::tuple<int, bool> DragInt(const std::string &label, int v, float v_speed, int v_min, int v_max,
                                  const std::string &format) {
        return DragInt(label, v, v_speed, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<int, bool> DragInt(const std::string &label, int v, float v_speed, int v_min, int v_max,
                                  const std::string &format, int flags) {
        int current_v = v; // Pass by reference
        bool used = ImGui::DragInt(label.c_str(), &current_v, v_speed, v_min, v_max, format.c_str(),
                                   flags);
        return std::make_tuple(current_v, used);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt2(const std::string &label, const sol::table &v) {
        return DragIntN_Internal<2>(label, v, 1.0f, 0, 0, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt2(const std::string &label, const sol::table &v,
                                                                  float v_speed) {
        return DragIntN_Internal<2>(label, v, v_speed, 0, 0, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt2(const std::string &label, const sol::table &v,
                                                                  float v_speed, int v_min) {
        return DragIntN_Internal<2>(label, v, v_speed, v_min, 0, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt2(const std::string &label, const sol::table &v,
                                                                  float v_speed, int v_min, int v_max) {
        return DragIntN_Internal<2>(label, v, v_speed, v_min, v_max, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt2(const std::string &label, const sol::table &v,
                                                                  float v_speed, int v_min, int v_max,
                                                                  const std::string &format) {
        return DragIntN_Internal<2>(label, v, v_speed, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt2(const std::string &label, const sol::table &v,
                                                                  float v_speed, int v_min, int v_max,
                                                                  const std::string &format, int flags) {
        return DragIntN_Internal<2>(label, v, v_speed, v_min, v_max, format.c_str(), flags);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt3(const std::string &label, const sol::table &v) {
        return DragIntN_Internal<3>(label, v, 1.0f, 0, 0, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt3(const std::string &label, const sol::table &v,
                                                                  float v_speed) {
        return DragIntN_Internal<3>(label, v, v_speed, 0, 0, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt3(const std::string &label, const sol::table &v,
                                                                  float v_speed, int v_min) {
        return DragIntN_Internal<3>(label, v, v_speed, v_min, 0, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt3(const std::string &label, const sol::table &v,
                                                                  float v_speed, int v_min, int v_max) {
        return DragIntN_Internal<3>(label, v, v_speed, v_min, v_max, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt3(const std::string &label, const sol::table &v,
                                                                  float v_speed, int v_min, int v_max,
                                                                  const std::string &format) {
        return DragIntN_Internal<3>(label, v, v_speed, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt3(const std::string &label, const sol::table &v,
                                                                  float v_speed, int v_min, int v_max,
                                                                  const std::string &format, int flags) {
        return DragIntN_Internal<3>(label, v, v_speed, v_min, v_max, format.c_str(), flags);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt4(const std::string &label, const sol::table &v) {
        return DragIntN_Internal<4>(label, v, 1.0f, 0, 0, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt4(const std::string &label, const sol::table &v,
                                                                  float v_speed) {
        return DragIntN_Internal<4>(label, v, v_speed, 0, 0, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt4(const std::string &label, const sol::table &v,
                                                                  float v_speed, int v_min) {
        return DragIntN_Internal<4>(label, v, v_speed, v_min, 0, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt4(const std::string &label, const sol::table &v,
                                                                  float v_speed, int v_min, int v_max) {
        return DragIntN_Internal<4>(label, v, v_speed, v_min, v_max, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt4(const std::string &label, const sol::table &v,
                                                                  float v_speed, int v_min, int v_max,
                                                                  const std::string &format) {
        return DragIntN_Internal<4>(label, v, v_speed, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> DragInt4(const std::string &label, const sol::table &v,
                                                                  float v_speed, int v_min, int v_max,
                                                                  const std::string &format, int flags) {
        return DragIntN_Internal<4>(label, v, v_speed, v_min, v_max, format.c_str(), flags);
    }

    void DragIntRange2() {
        /* TODO: DragIntRange2(...) ==> UNSUPPORTED */
    }

    void DragScalar() {
        /* TODO: DragScalar(...) ==> UNSUPPORTED */
    }

    void DragScalarN() {
        /* TODO: DragScalarN(...) ==> UNSUPPORTED */
    }

    std::tuple<float, bool> SliderFloat(const std::string &label, float v, float v_min, float v_max, const char *format,
                                        int flags) {
        float current_v = v; // Pass by reference
        bool used = ImGui::SliderFloat(label.c_str(), &current_v, v_min, v_max, format,
                                       flags);
        return std::make_tuple(current_v, used);
    }

    std::tuple<float, bool> SliderFloat(const std::string &label, float v, float v_min, float v_max) {
        return SliderFloat(label, v, v_min, v_max, "%.3f", 0);
    }

    std::tuple<float, bool> SliderFloat(const std::string &label, float v, float v_min, float v_max,
                                        const std::string &format) {
        return SliderFloat(label, v, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<float, bool> SliderFloat(const std::string &label, float v, float v_min, float v_max,
                                        const std::string &format, int flags) {
        float current_v = v; // Pass by reference
        bool used = ImGui::SliderFloat(label.c_str(), &current_v, v_min, v_max, format.c_str(),
                                       flags);
        return std::make_tuple(current_v, used);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat2(const std::string &label, const sol::table &v,
                                                                        float v_min, float v_max) {
        return SliderFloatN_Internal<2>(label, v, v_min, v_max, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat2(const std::string &label, const sol::table &v,
                                                                        float v_min, float v_max,
                                                                        const std::string &format) {
        return SliderFloatN_Internal<2>(label, v, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat2(const std::string &label, const sol::table &v,
                                                                        float v_min, float v_max,
                                                                        const std::string &format, int flags) {
        return SliderFloatN_Internal<2>(label, v, v_min, v_max, format.c_str(), flags);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat3(const std::string &label, const sol::table &v,
                                                                        float v_min, float v_max) {
        return SliderFloatN_Internal<3>(label, v, v_min, v_max, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat3(const std::string &label, const sol::table &v,
                                                                        float v_min, float v_max,
                                                                        const std::string &format) {
        return SliderFloatN_Internal<3>(label, v, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat3(const std::string &label, const sol::table &v,
                                                                        float v_min, float v_max,
                                                                        const std::string &format, int flags) {
        return SliderFloatN_Internal<3>(label, v, v_min, v_max, format.c_str(), flags);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat4(const std::string &label, const sol::table &v,
                                                                        float v_min, float v_max) {
        return SliderFloatN_Internal<4>(label, v, v_min, v_max, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat4(const std::string &label, const sol::table &v,
                                                                        float v_min, float v_max,
                                                                        const std::string &format) {
        return SliderFloatN_Internal<4>(label, v, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> SliderFloat4(const std::string &label, const sol::table &v,
                                                                        float v_min, float v_max,
                                                                        const std::string &format, int flags) {
        return SliderFloatN_Internal<4>(label, v, v_min, v_max, format.c_str(), flags);
    }

    std::tuple<float, bool> SliderAngle(const std::string &label, float v_rad, float v_degrees_min, float v_degrees_max,
                                        const char *format, int flags) {
        float current_v_rad = v_rad; // Pass by reference
        bool used = ImGui::SliderAngle(label.c_str(), &current_v_rad, v_degrees_min, v_degrees_max, format,
                                       flags);
        return std::make_tuple(current_v_rad, used);
    }

    std::tuple<float, bool> SliderAngle(const std::string &label, float v_rad) {
        return SliderAngle(label, v_rad, -360.0f, 360.0f, "%.0f deg", 0);
    }

    std::tuple<float, bool> SliderAngle(const std::string &label, float v_rad, float v_degrees_min) {
        return SliderAngle(label, v_rad, v_degrees_min, 360.0f, "%.0f deg", 0);
    }

    std::tuple<float, bool> SliderAngle(const std::string &label, float v_rad, float v_degrees_min,
                                        float v_degrees_max) {
        return SliderAngle(label, v_rad, v_degrees_min, v_degrees_max, "%.0f deg", 0);
    }

    std::tuple<float, bool> SliderAngle(const std::string &label, float v_rad, float v_degrees_min, float v_degrees_max,
                                        const std::string &format) {
        return SliderAngle(label, v_rad, v_degrees_min, v_degrees_max, format.c_str(), 0);
    }

    std::tuple<float, bool> SliderAngle(const std::string &label, float v_rad, float v_degrees_min, float v_degrees_max,
                                        const std::string &format, int flags) {
        float current_v_rad = v_rad; // Pass by reference
        bool used = ImGui::SliderAngle(label.c_str(), &current_v_rad, v_degrees_min, v_degrees_max, format.c_str(),
                                       flags);
        return std::make_tuple(current_v_rad, used);
    }

    std::tuple<int, bool> SliderInt(const std::string &label, int v, int v_min, int v_max, const char *format,
                                    int flags) {
        int current_v = v; // Pass by reference
        bool used = ImGui::SliderInt(label.c_str(), &current_v, v_min, v_max, format,
                                     flags);
        return std::make_tuple(current_v, used);
    }

    std::tuple<int, bool> SliderInt(const std::string &label, int v, int v_min, int v_max) {
        return SliderInt(label, v, v_min, v_max, "%d", 0);
    }

    std::tuple<int, bool> SliderInt(const std::string &label, int v, int v_min, int v_max, const std::string &format) {
        return SliderInt(label, v, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<int, bool> SliderInt(const std::string &label, int v, int v_min, int v_max, const std::string &format,
                                    int flags) {
        int current_v = v; // Pass by reference
        bool used = ImGui::SliderInt(label.c_str(), &current_v, v_min, v_max, format.c_str(),
                                     flags);
        return std::make_tuple(current_v, used);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt2(const std::string &label, const sol::table &v,
                                                                    int v_min, int v_max) {
        return SliderIntN_Internal<2>(label, v, v_min, v_max, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt2(const std::string &label, const sol::table &v,
                                                                    int v_min, int v_max, const std::string &format) {
        return SliderIntN_Internal<2>(label, v, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt2(const std::string &label, const sol::table &v,
                                                                    int v_min, int v_max, const std::string &format,
                                                                    int flags) {
        return SliderIntN_Internal<2>(label, v, v_min, v_max, format.c_str(), flags);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt3(const std::string &label, const sol::table &v,
                                                                    int v_min, int v_max) {
        return SliderIntN_Internal<3>(label, v, v_min, v_max, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt3(const std::string &label, const sol::table &v,
                                                                    int v_min, int v_max, const std::string &format) {
        return SliderIntN_Internal<3>(label, v, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt3(const std::string &label, const sol::table &v,
                                                                    int v_min, int v_max, const std::string &format,
                                                                    int flags) {
        return SliderIntN_Internal<3>(label, v, v_min, v_max, format.c_str(), flags);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt4(const std::string &label, const sol::table &v,
                                                                    int v_min, int v_max) {
        return SliderIntN_Internal<4>(label, v, v_min, v_max, "%d", 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt4(const std::string &label, const sol::table &v,
                                                                    int v_min, int v_max, const std::string &format) {
        return SliderIntN_Internal<4>(label, v, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> SliderInt4(const std::string &label, const sol::table &v,
                                                                    int v_min, int v_max, const std::string &format,
                                                                    int flags) {
        return SliderIntN_Internal<4>(label, v, v_min, v_max, format.c_str(), flags);
    }

    void SliderScalar() {
        /* TODO: SliderScalar(...) ==> UNSUPPORTED */
    }

    void SliderScalarN() {
        /* TODO: SliderScalarN(...) ==> UNSUPPORTED */
    }

    std::tuple<float, bool> VSliderFloat(const std::string &label, float sizeX, float sizeY, float v, float v_min,
                                         float v_max, const char *format, int flags) {
        float current_v = v; // Pass by reference
        bool used = ImGui::VSliderFloat(label.c_str(), {sizeX, sizeY}, &current_v, v_min, v_max, format,
                                        flags);
        return std::make_tuple(current_v, used);
    }

    std::tuple<float, bool> VSliderFloat(const std::string &label, float sizeX, float sizeY, float v, float v_min,
                                         float v_max) {
        return VSliderFloat(label, sizeX, sizeY, v, v_min, v_max, "%.3f", 0);
    }

    std::tuple<float, bool> VSliderFloat(const std::string &label, float sizeX, float sizeY, float v, float v_min,
                                         float v_max, const std::string &format) {
        return VSliderFloat(label, sizeX, sizeY, v, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<float, bool> VSliderFloat(const std::string &label, float sizeX, float sizeY, float v, float v_min,
                                         float v_max, const std::string &format, int flags) {
        float current_v = v; // Pass by reference
        bool used = ImGui::VSliderFloat(label.c_str(), {sizeX, sizeY}, &current_v, v_min, v_max, format.c_str(),
                                        flags);
        return std::make_tuple(current_v, used);
    }

    std::tuple<int, bool> VSliderInt(const std::string &label, float sizeX, float sizeY, int v, int v_min, int v_max,
                                     const char *format, int flags) {
        int current_v = v; // Pass by reference
        bool used = ImGui::VSliderInt(label.c_str(), {sizeX, sizeY}, &current_v, v_min, v_max, format,
                                      flags);
        return std::make_tuple(current_v, used);
    }

    std::tuple<int, bool> VSliderInt(const std::string &label, float sizeX, float sizeY, int v, int v_min, int v_max) {
        return VSliderInt(label, sizeX, sizeY, v, v_min, v_max, "%d", 0);
    }

    std::tuple<int, bool> VSliderInt(const std::string &label, float sizeX, float sizeY, int v, int v_min, int v_max,
                                     const std::string &format) {
        return VSliderInt(label, sizeX, sizeY, v, v_min, v_max, format.c_str(), 0);
    }

    std::tuple<int, bool> VSliderInt(const std::string &label, float sizeX, float sizeY, int v, int v_min, int v_max,
                                     const std::string &format, int flags) {
        int current_v = v; // Pass by reference
        bool used = ImGui::VSliderInt(label.c_str(), {sizeX, sizeY}, &current_v, v_min, v_max, format.c_str(),
                                      flags);
        return std::make_tuple(current_v, used);
    }

    void VSliderScalar() {
        /* TODO: VSliderScalar(...) ==> UNSUPPORTED */
    }

    std::tuple<std::string, bool> InputText(const std::string &label, std::string current_text, size_t buf_size,
                                            int flags) {
        std::vector<char> buffer(buf_size);
        strncpy_s(buffer.data(), buf_size, current_text.c_str(), current_text.length());
        // strncpy_s should null terminate if there's space, but ensure last char is null if string is >= buf_size-1
        buffer[std::min(buf_size - 1, current_text.length())] = '\0';

        bool changed = ImGui::InputText(label.c_str(), buffer.data(), buf_size,
                                        flags);
        return std::make_tuple(std::string(buffer.data()), changed);
    }

    std::tuple<std::string, bool> InputText(const std::string &label, std::string current_text, size_t buf_size) {
        return InputText(label, current_text, buf_size, 0);
    }

    std::tuple<std::string, bool> InputTextMultiline(const std::string &label, std::string current_text,
                                                     size_t buf_size, const ImVec2 &size, int flags) {
        std::vector<char> buffer(buf_size);
        strncpy_s(buffer.data(), buf_size, current_text.c_str(), current_text.length());
        buffer[std::min(buf_size - 1, current_text.length())] = '\0';

        bool changed = ImGui::InputTextMultiline(label.c_str(), buffer.data(), buf_size, size,
                                                 flags);
        return std::make_tuple(std::string(buffer.data()), changed);
    }

    std::tuple<std::string, bool> InputTextMultiline(const std::string &label, std::string current_text,
                                                     size_t buf_size) {
        return InputTextMultiline(label, current_text, buf_size, {0, 0}, 0);
    }

    std::tuple<std::string, bool> InputTextMultiline(const std::string &label, std::string current_text,
                                                     size_t buf_size, float sizeX, float sizeY) {
        return InputTextMultiline(label, current_text, buf_size, {sizeX, sizeY}, 0);
    }

    std::tuple<std::string, bool> InputTextMultiline(const std::string &label, std::string current_text,
                                                     size_t buf_size, float sizeX, float sizeY, int flags) {
        return InputTextMultiline(label, current_text, buf_size, {sizeX, sizeY}, flags);
    }

    std::tuple<std::string, bool> InputTextWithHint(const std::string &label, const std::string &hint,
                                                    std::string current_text, size_t buf_size, int flags) {
        std::vector<char> buffer(buf_size);
        strncpy_s(buffer.data(), buf_size, current_text.c_str(), current_text.length());
        buffer[std::min(buf_size - 1, current_text.length())] = '\0';

        bool changed = ImGui::InputTextWithHint(label.c_str(), hint.c_str(), buffer.data(), buf_size,
                                                flags);
        return std::make_tuple(std::string(buffer.data()), changed);
    }

    std::tuple<std::string, bool> InputTextWithHint(const std::string &label, const std::string &hint,
                                                    std::string current_text, size_t buf_size) {
        return InputTextWithHint(label, hint, current_text, buf_size, 0);
    }

    std::tuple<float, bool> InputFloat(const std::string &label, float v, float step, float step_fast,
                                       const char *format, int flags) {
        float current_v = v; // Pass by reference
        bool changed = ImGui::InputFloat(label.c_str(), &current_v, step, step_fast, format,
                                         flags);
        return std::make_tuple(current_v, changed);
    }

    std::tuple<float, bool> InputFloat(const std::string &label, float v) {
        return InputFloat(label, v, 0.0f, 0.0f, "%.3f", 0);
    }

    std::tuple<float, bool> InputFloat(const std::string &label, float v, float step) {
        return InputFloat(label, v, step, 0.0f, "%.3f", 0);
    }

    std::tuple<float, bool> InputFloat(const std::string &label, float v, float step, float step_fast) {
        return InputFloat(label, v, step, step_fast, "%.3f", 0);
    }

    std::tuple<float, bool> InputFloat(const std::string &label, float v, float step, float step_fast,
                                       const std::string &format) {
        return InputFloat(label, v, step, step_fast, format.c_str(), 0);
    }

    std::tuple<float, bool> InputFloat(const std::string &label, float v, float step, float step_fast,
                                       const std::string &format, int flags) {
        float current_v = v; // Pass by reference
        bool changed = ImGui::InputFloat(label.c_str(), &current_v, step, step_fast, format.c_str(),
                                         flags);
        return std::make_tuple(current_v, changed);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat2(const std::string &label, const sol::table &v) {
        return InputFloatN_Internal<2>(label, v, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat2(const std::string &label, const sol::table &v,
                                                                       const std::string &format) {
        return InputFloatN_Internal<2>(label, v, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat2(const std::string &label, const sol::table &v,
                                                                       const std::string &format, int flags) {
        return InputFloatN_Internal<2>(label, v, format.c_str(), flags);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat3(const std::string &label, const sol::table &v) {
        return InputFloatN_Internal<3>(label, v, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat3(const std::string &label, const sol::table &v,
                                                                       const std::string &format) {
        return InputFloatN_Internal<3>(label, v, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat3(const std::string &label, const sol::table &v,
                                                                       const std::string &format, int flags) {
        return InputFloatN_Internal<3>(label, v, format.c_str(), flags);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat4(const std::string &label, const sol::table &v) {
        return InputFloatN_Internal<4>(label, v, "%.3f", 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat4(const std::string &label, const sol::table &v,
                                                                       const std::string &format) {
        return InputFloatN_Internal<4>(label, v, format.c_str(), 0);
    }

    std::tuple<sol::as_table_t<std::vector<float> >, bool> InputFloat4(const std::string &label, const sol::table &v,
                                                                       const std::string &format, int flags) {
        return InputFloatN_Internal<4>(label, v, format.c_str(), flags);
    }

    std::tuple<int, bool> InputInt(const std::string &label, int v, int step, int step_fast, int flags) {
        int current_v = v; // Pass by reference
        bool changed = ImGui::InputInt(label.c_str(), &current_v, step, step_fast,
                                       flags);
        return std::make_tuple(current_v, changed);
    }

    std::tuple<int, bool> InputInt(const std::string &label, int v) { return InputInt(label, v, 1, 100, 0); }

    std::tuple<int, bool> InputInt(const std::string &label, int v, int step) {
        return InputInt(label, v, step, 100, 0);
    }

    std::tuple<int, bool> InputInt(const std::string &label, int v, int step, int step_fast) {
        return InputInt(label, v, step, step_fast, 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputInt2(const std::string &label, const sol::table &v) {
        return InputIntN_Internal<2>(label, v, 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputInt2(const std::string &label, const sol::table &v,
                                                                   int flags) {
        return InputIntN_Internal<2>(label, v, flags);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputInt3(const std::string &label, const sol::table &v) {
        return InputIntN_Internal<3>(label, v, 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputInt3(const std::string &label, const sol::table &v,
                                                                   int flags) {
        return InputIntN_Internal<3>(label, v, flags);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputInt4(const std::string &label, const sol::table &v) {
        return InputIntN_Internal<4>(label, v, 0);
    }

    std::tuple<sol::as_table_t<std::vector<int> >, bool> InputInt4(const std::string &label, const sol::table &v,
                                                                   int flags) {
        return InputIntN_Internal<4>(label, v, flags);
    }

    std::tuple<double, bool> InputDouble(const std::string &label, double v, double step, double step_fast,
                                         const char *format, int flags) {
        double current_v = v; // Pass by reference
        bool changed = ImGui::InputDouble(label.c_str(), &current_v, step, step_fast, format,
                                          flags);
        return std::make_tuple(current_v, changed);
    }

    std::tuple<double, bool> InputDouble(const std::string &label, double v) {
        return InputDouble(label, v, 0.0, 0.0, "%.6f", 0);
    }

    std::tuple<double, bool> InputDouble(const std::string &label, double v, double step) {
        return InputDouble(label, v, step, 0.0, "%.6f", 0);
    }

    std::tuple<double, bool> InputDouble(const std::string &label, double v, double step, double step_fast) {
        return InputDouble(label, v, step, step_fast, "%.6f", 0);
    }

    std::tuple<double, bool> InputDouble(const std::string &label, double v, double step, double step_fast,
                                         const std::string &format) {
        return InputDouble(label, v, step, step_fast, format.c_str(), 0);
    }

    std::tuple<double, bool> InputDouble(const std::string &label, double v, double step, double step_fast,
                                         const std::string &format, int flags) {
        double current_v = v; // Pass by reference
        bool changed = ImGui::InputDouble(label.c_str(), &current_v, step, step_fast, format.c_str(),
                                          flags);
        return std::make_tuple(current_v, changed);
    }

    void InputScalar() {
        /* TODO: InputScalar(...) ==> UNSUPPORTED */
    }

    void InputScalarN() {
        /* TODO: InputScalarN(...) ==> UNSUPPORTED */
    }

    bool ColorButton(const std::string &desc_id, const sol::table &col, int flags, const ImVec2 &size) {
        // Added default for flags and size
        const lua_Number r{col[1].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                g{col[2].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                b{col[3].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(0))},
                a{
                    col[4].get<std::optional<lua_Number> >().value_or(static_cast<lua_Number>(1.0f))
                }; // Default alpha to 1.0f
        const ImVec4 color_vec{float(r), float(g), float(b), float(a)};
        return ImGui::ColorButton(desc_id.c_str(), color_vec, flags, size);
    }

    bool ColorButton(const std::string &desc_id, const sol::table &col) {
        return ColorButton(desc_id, col, 0, {0, 0});
    }

    bool ColorButton(const std::string &desc_id, const sol::table &col, int flags) {
        return ColorButton(desc_id, col, flags, {0, 0});
    }

    bool ColorButton(const std::string &desc_id, const sol::table &col, int flags, float sizeX, float sizeY) {
        return ColorButton(desc_id, col, flags, {sizeX, sizeY});
    }

    bool Selectable(const std::string &label, bool selected, int flags, const ImVec2 &size) {
        bool p_selected = selected;
        bool activated = ImGui::Selectable(label.c_str(), &p_selected, flags, size);
        // If activated, return new state of p_selected. Otherwise, return original selected state.
        return activated ? p_selected : selected;
    }

    bool Selectable(const std::string &label) { return ImGui::Selectable(label.c_str()); }

    bool Selectable(const std::string &label, bool selected) {
        return Selectable(label, selected, 0, {0, 0});
    }

    bool Selectable(const std::string &label, bool selected, int flags) {
        return Selectable(label, selected, flags, {0, 0});
    }

    bool Selectable(const std::string &label, bool selected, int flags, float sizeX, float sizeY) {
        return Selectable(label, selected, flags, {sizeX, sizeY});
    }

    std::tuple<int, bool> ListBox(const std::string &label, int current_item, const sol::table &items, int items_count,
                                  int height_in_items) {
        std::vector<std::string> strings;
        for (int i{1}; i <= items_count; i++) {
            const auto &stringItem = items.get<sol::optional<std::string> >(i);
            strings.push_back(stringItem.value_or("Missing"));
        }

        std::vector<const char *> cstrings;
        for (auto &string_item: strings)
            cstrings.push_back(string_item.c_str());

        int current_item_ref = current_item;
        bool clicked = ImGui::ListBox(label.c_str(), &current_item_ref, cstrings.data(), items_count, height_in_items);
        return std::make_tuple(current_item_ref, clicked);
    }

    std::tuple<int, bool> ListBox(const std::string &label, int current_item, const sol::table &items,
                                  int items_count) {
        return ListBox(label, current_item, items, items_count, -1);
    }

    bool ListBoxHeader(const std::string &label, float sizeX, float sizeY) {
        return ImGui::BeginListBox(label.c_str(), {sizeX, sizeY});
    }

    bool ListBoxHeader(const std::string &label, int items_count, int height_in_items) {
        float height = ImGui::GetTextLineHeightWithSpacing() * ((height_in_items < 0
                                                                     ? ImMin(items_count, 7)
                                                                     : height_in_items) + 0.25f) + ImGui::GetStyle().
                       FramePadding.y * 2.0f;
        return ImGui::BeginListBox(label.c_str(), ImVec2(0.0f, height));
    }

    void ListBoxFooter() { ImGui::EndListBox(); }

    bool BeginMenuBar() { return ImGui::BeginMenuBar(); }

    void EndMenuBar() { ImGui::EndMenuBar(); }

    bool BeginMainMenuBar() { return ImGui::BeginMainMenuBar(); }

    void EndMainMenuBar() { ImGui::EndMainMenuBar(); }

    bool BeginMenu(const std::string &label) { return ImGui::BeginMenu(label.c_str()); }

    bool BeginMenu(const std::string &label, bool enabled) { return ImGui::BeginMenu(label.c_str(), enabled); }

    void EndMenu() { ImGui::EndMenu(); }

    bool MenuItem(const std::string &label) {
        return ImGui::MenuItem(label.c_str());
    }

    bool MenuItem(const std::string &label, const std::string &shortcut) {
        return ImGui::MenuItem(label.c_str(), shortcut.c_str());
    }

    bool MenuItem(const std::string &label, const std::string &shortcut, bool selected) {
        return ImGui::MenuItem(label.c_str(), shortcut.c_str(), selected);
    }

    bool MenuItem(const std::string &label, const std::string &shortcut, bool selected, bool enabled) {
        return ImGui::MenuItem(label.c_str(), shortcut.c_str(), selected, enabled);
    }

    std::tuple<bool, bool> MenuItemToggle(const std::string &label, const std::string &shortcut, bool selected_val,
                                          bool enabled) {
        bool p_selected = selected_val;
        bool activated = ImGui::MenuItem(label.c_str(), shortcut.c_str(), &p_selected, enabled);
        return std::make_tuple(p_selected, activated);
    }

    int GetKeyIndex(int imgui_key_enum_val) { return static_cast<ImGuiKey>(imgui_key_enum_val); }

    bool IsKeyDown(int user_key_index) { return ImGui::IsKeyDown(static_cast<ImGuiKey>(user_key_index)); }

    bool IsKeyPressed(int user_key_index) { return ImGui::IsKeyPressed(static_cast<ImGuiKey>(user_key_index)); }

    bool IsKeyPressed(int user_key_index, bool repeat) {
        return ImGui::IsKeyPressed(static_cast<ImGuiKey>(user_key_index), repeat);
    }

    bool IsKeyReleased(int user_key_index) {
        return ImGui::IsKeyReleased(static_cast<ImGuiKey>(user_key_index));
    }

    int GetKeyPressedAmount(int key_index, float repeat_delay, float rate) {
        return ImGui::GetKeyPressedAmount(static_cast<ImGuiKey>(key_index), repeat_delay, rate);
    }

    void CaptureKeyboardFromApp() { ImGui::SetNextFrameWantCaptureKeyboard(true); }

    void CaptureKeyboardFromApp(bool want_capture_keyboard_value) {
        ImGui::SetNextFrameWantCaptureKeyboard(want_capture_keyboard_value);
    }

    bool IsMouseDown(int button) { return ImGui::IsMouseDown(button); }

    bool IsMouseClicked(int button) { return ImGui::IsMouseClicked(button); }

    bool IsMouseClicked(int button, bool repeat) {
        return ImGui::IsMouseClicked(button, repeat);
    }

    bool IsMouseReleased(int button) { return ImGui::IsMouseReleased(button); }

    bool IsMouseDoubleClicked(int button) {
        return ImGui::IsMouseDoubleClicked(button);
    }

    bool IsMouseHoveringRect(float min_x, float min_y, float max_x, float max_y) {
        return ImGui::IsMouseHoveringRect({min_x, min_y}, {max_x, max_y});
    }

    bool IsMouseHoveringRect(float min_x, float min_y, float max_x, float max_y, bool clip) {
        return ImGui::IsMouseHoveringRect({min_x, min_y}, {max_x, max_y}, clip);
    }

    bool IsMousePosValid() { return ImGui::IsMousePosValid(NULL); }

    bool IsMousePosValid(float x, float y) {
        ImVec2 p(x, y);
        return ImGui::IsMousePosValid(&p);
    }

    bool IsAnyMouseDown() { return ImGui::IsAnyMouseDown(); }

    std::tuple<float, float> GetMousePos() {
        const auto vec2{ImGui::GetMousePos()};
        return std::make_tuple(vec2.x, vec2.y);
    }

    std::tuple<float, float> GetMousePosOnOpeningCurrentPopup() {
        const auto vec2{ImGui::GetMousePosOnOpeningCurrentPopup()};
        return std::make_tuple(vec2.x, vec2.y);
    }

    bool IsMouseDragging(int button) { return ImGui::IsMouseDragging(button); }

    bool IsMouseDragging(int button, float lock_threshold) {
        return ImGui::IsMouseDragging(button, lock_threshold);
    }

    std::tuple<float, float> GetMouseDragDelta() {
        const auto vec2{ImGui::GetMouseDragDelta()};
        return std::make_tuple(vec2.x, vec2.y);
    }

    std::tuple<float, float> GetMouseDragDelta(int button) {
        const auto vec2{ImGui::GetMouseDragDelta(button)};
        return std::make_tuple(vec2.x, vec2.y);
    }

    std::tuple<float, float> GetMouseDragDelta(int button, float lock_threshold) {
        const auto vec2{ImGui::GetMouseDragDelta(button, lock_threshold)};
        return std::make_tuple(vec2.x, vec2.y);
    }

    void ResetMouseDragDelta() { ImGui::ResetMouseDragDelta(); }

    void ResetMouseDragDelta(int button) { ImGui::ResetMouseDragDelta(button); }

    int GetMouseCursor() { return ImGui::GetMouseCursor(); }

    void SetMouseCursor(int cursor_type) { ImGui::SetMouseCursor(cursor_type); }

    void CaptureMouseFromApp() { ImGui::SetNextFrameWantCaptureMouse(true); }

    void CaptureMouseFromApp(bool want_capture_mouse_value) {
        ImGui::SetNextFrameWantCaptureMouse(want_capture_mouse_value);
    }

    std::string GetClipboardText() {
        return std::string(ImGui::GetClipboardText() ? ImGui::GetClipboardText() : "");
    }

    void SetClipboardText(const std::string &text) { ImGui::SetClipboardText(text.c_str()); }

#pragma region Init
    void Init_IDStackScopes(sol::table &imGuiTable) {
        imGuiTable.set_function("PushID", sol::overload(
                                    sol::resolve<void(const std::string &)>(PushID),
                                    sol::resolve<void(const std::string &, const std::string &)>(PushID),
                                    sol::resolve<void(int)>(PushID)
                                    // Skipping void* version for Lua
                                ));
        imGuiTable.set_function("PopID", PopID);
        imGuiTable.set_function("GetID", sol::overload(
                                    sol::resolve<int(const std::string &)>(GetID),
                                    sol::resolve<int(const std::string &, const std::string &)>(GetID)
                                    // Skipping void* version for Lua
                                ));
    }

    void Init_WidgetsMain(sol::table &imGuiTable) {
        imGuiTable.set_function("Button", sol::overload(
                                    sol::resolve<bool(const std::string &)>(Button),
                                    sol::resolve<bool(const std::string &, float, float)>(Button)
                                ));
        imGuiTable.set_function("SmallButton", SmallButton);
        imGuiTable.set_function("InvisibleButton", sol::overload(
                                    sol::resolve<bool(const std::string &, float, float)>(InvisibleButton),
                                    sol::resolve<bool(const std::string &, float, float, int)>(InvisibleButton)
                                ));
        imGuiTable.set_function("ArrowButton", ArrowButton);
        imGuiTable.set_function("Checkbox", Checkbox);
        imGuiTable.set_function("CheckboxFlags", sol::overload(
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, int)>(CheckboxFlags),
                                    sol::resolve<std::tuple<unsigned int, bool>(
                                        const std::string &, unsigned int, unsigned int)>(CheckboxFlags)
                                ));
        imGuiTable.set_function("RadioButton", sol::overload(
                                    sol::resolve<bool(const std::string &, bool)>(RadioButton),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, int)>(RadioButton)
                                ));
        imGuiTable.set_function("ProgressBar", sol::overload(
                                    sol::resolve<void(float)>(ProgressBar),
                                    sol::resolve<void(float, float, float)>(ProgressBar),
                                    sol::resolve<void(float, float, float, const std::string &)>(ProgressBar)
                                ));
        imGuiTable.set_function("Bullet", Bullet);
    }

    void Init_WidgetsDrags(sol::table &imGuiTable) {
        imGuiTable.set_function("DragFloat", sol::overload(
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float)>(DragFloat),
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float, float)>(DragFloat),
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float, float, float)>(
                                        DragFloat),
                                    sol::resolve<std::tuple<float, bool>(
                                        const std::string &, float, float, float, float)>(
                                        DragFloat),
                                    sol::resolve<std::tuple<float, bool>(
                                        const std::string &, float, float, float, float,
                                        const std::string &)>(DragFloat),
                                    sol::resolve<std::tuple<float, bool>(
                                        const std::string &, float, float, float, float,
                                        const std::string &,
                                        int)>(DragFloat) // power -> flags
                                ));
        imGuiTable.set_function("DragFloat2", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &)>(DragFloat2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float)>(DragFloat2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float)>(DragFloat2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, float)>(DragFloat2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, float,
                                        const std::string &)>(
                                        DragFloat2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, float,
                                        const std::string &,
                                        int)>(DragFloat2) // power -> flags
                                ));
        imGuiTable.set_function("DragFloat3", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &)>(DragFloat3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float)>(DragFloat3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float)>(DragFloat3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, float)>(DragFloat3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, float,
                                        const std::string &)>(
                                        DragFloat3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, float,
                                        const std::string &,
                                        int)>(DragFloat3) // power -> flags
                                ));
        imGuiTable.set_function("DragFloat4", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &)>(DragFloat4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float)>(DragFloat4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float)>(DragFloat4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, float)>(DragFloat4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, float,
                                        const std::string &)>(
                                        DragFloat4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, float,
                                        const std::string &,
                                        int)>(DragFloat4) // power -> flags
                                ));
        imGuiTable.set_function("DragInt", sol::overload(
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int)>(DragInt),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, float)>(DragInt),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, float, int)>(DragInt),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, float, int, int)>(
                                        DragInt),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, float, int, int,
                                                                       const std::string &)>(DragInt),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, float, int, int,
                                                                       const std::string &,
                                                                       int)>(DragInt) // new overload with flags
                                ));
        imGuiTable.set_function("DragInt2", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &)>(DragInt2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float)>(DragInt2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float, int)>(DragInt2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float, int, int)>(DragInt2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float, int, int, const std::string &)>(
                                        DragInt2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float, int, int, const std::string &,
                                        int)>(DragInt2) // new overload with flags
                                ));
        imGuiTable.set_function("DragInt3", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &)>(DragInt3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float)>(DragInt3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float, int)>(DragInt3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float, int, int)>(DragInt3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float, int, int, const std::string &)>(
                                        DragInt3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float, int, int, const std::string &,
                                        int)>(DragInt3) // new overload with flags
                                ));
        imGuiTable.set_function("DragInt4", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &)>(DragInt4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float)>(DragInt4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float, int)>(DragInt4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float, int, int)>(DragInt4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float, int, int, const std::string &)>(
                                        DragInt4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, float, int, int, const std::string &,
                                        int)>(DragInt4) // new overload with flags
                                ));
    }

    void Init_WidgetsSliders(sol::table &imGuiTable) {
        imGuiTable.set_function("SliderFloat", sol::overload(
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float, float, float)>(
                                        SliderFloat),
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float, float, float,
                                                                         const std::string &)>(SliderFloat),
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float, float, float,
                                                                         const std::string &,
                                                                         int)>(SliderFloat) // power -> flags
                                ));
        imGuiTable.set_function("SliderFloat2", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float)>(SliderFloat2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, const std::string &)>(
                                        SliderFloat2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, const std::string &,
                                        int)>(SliderFloat2) // power -> flags
                                ));
        imGuiTable.set_function("SliderFloat3", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float)>(SliderFloat3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, const std::string &)>(
                                        SliderFloat3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, const std::string &,
                                        int)>(SliderFloat3) // power -> flags
                                ));
        imGuiTable.set_function("SliderFloat4", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float)>(SliderFloat4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, const std::string &)>(
                                        SliderFloat4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, float, float, const std::string &,
                                        int)>(SliderFloat4) // power -> flags
                                ));
        imGuiTable.set_function("SliderAngle", sol::overload(
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float)>(SliderAngle),
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float, float)>(
                                        SliderAngle),
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float, float, float)>(
                                        SliderAngle),
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float, float, float,
                                                                         const std::string &)>(SliderAngle),
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float, float, float,
                                                                         const std::string &,
                                                                         int)>(SliderAngle) // new overload with flags
                                ));
        imGuiTable.set_function("SliderInt", sol::overload(
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, int, int)>(SliderInt),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, int, int,
                                                                       const std::string &)>(SliderInt),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, int, int,
                                                                       const std::string &,
                                                                       int)>(SliderInt) // new overload with flags
                                ));
        imGuiTable.set_function("SliderInt2", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, int, int)>(SliderInt2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, int, int, const std::string &)>(
                                        SliderInt2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, int, int, const std::string &,
                                        int)>(SliderInt2) // new overload with flags
                                ));
        imGuiTable.set_function("SliderInt3", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, int, int)>(SliderInt3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, int, int, const std::string &)>(
                                        SliderInt3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, int, int, const std::string &,
                                        int)>(SliderInt3) // new overload with flags
                                ));
        imGuiTable.set_function("SliderInt4", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, int, int)>(SliderInt4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, int, int, const std::string &)>(
                                        SliderInt4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, int, int, const std::string &,
                                        int)>(SliderInt4) // new overload with flags
                                ));
        imGuiTable.set_function("VSliderFloat", sol::overload(
                                    sol::resolve<std::tuple<float, bool>(
                                        const std::string &, float, float, float, float,
                                        float)>(VSliderFloat),
                                    sol::resolve<std::tuple<float, bool>(
                                        const std::string &, float, float, float, float,
                                        float, const std::string &)>(VSliderFloat),
                                    sol::resolve<std::tuple<float, bool>(
                                        const std::string &, float, float, float, float,
                                        float, const std::string &,
                                        int)>(VSliderFloat) // power -> flags
                                ));
        imGuiTable.set_function("VSliderInt", sol::overload(
                                    sol::resolve<std::tuple<int, bool>
                                        (const std::string &, float, float, int, int, int)>(
                                        VSliderInt),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, float, float, int, int, int,
                                                                       const std::string &)>(VSliderInt),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, float, float, int, int, int,
                                                                       const std::string &,
                                                                       int)>(VSliderInt) // new overload with flags
                                ));
    }

    void Init_WidgetsInputKeyboard(sol::table &imGuiTable) {
        imGuiTable.set_function("InputText", sol::overload(
                                    sol::resolve<std::tuple<std::string, bool>
                                        (const std::string &, std::string, size_t)>(
                                        InputText),
                                    sol::resolve<std::tuple<std::string, bool>
                                        (const std::string &, std::string, size_t, int)>(InputText)
                                ));
        imGuiTable.set_function("InputTextMultiline", sol::overload(
                                    sol::resolve<std::tuple<std::string, bool>
                                        (const std::string &, std::string, size_t)>(
                                        InputTextMultiline),
                                    sol::resolve<std::tuple<std::string, bool>(
                                        const std::string &, std::string, size_t, float, float)>(InputTextMultiline),
                                    sol::resolve<std::tuple<std::string, bool>(
                                        const std::string &, std::string, size_t, float, float, int)>(
                                        InputTextMultiline)
                                ));
        imGuiTable.set_function("InputTextWithHint", sol::overload(
                                    sol::resolve<std::tuple<std::string, bool>(
                                        const std::string &, const std::string &, std::string, size_t)>(
                                        InputTextWithHint),
                                    sol::resolve<std::tuple<std::string, bool>(
                                        const std::string &, const std::string &, std::string, size_t, int)>(
                                        InputTextWithHint)
                                ));
        imGuiTable.set_function("InputFloat", sol::overload(
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float)>(InputFloat),
                                    sol::resolve<std::tuple<float, bool>
                                        (const std::string &, float, float)>(InputFloat),
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float, float, float)>(
                                        InputFloat),
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float, float, float,
                                                                         const std::string &)>(InputFloat),
                                    sol::resolve<std::tuple<float, bool>(const std::string &, float, float, float,
                                                                         const std::string &, int)>(InputFloat)
                                ));
        imGuiTable.set_function("InputFloat2", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &)>(InputFloat2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, const std::string &)>(InputFloat2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, const std::string &, int)>(InputFloat2)
                                ));
        imGuiTable.set_function("InputFloat3", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &)>(InputFloat3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, const std::string &)>(InputFloat3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, const std::string &, int)>(InputFloat3)
                                ));
        imGuiTable.set_function("InputFloat4", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &)>(InputFloat4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, const std::string &)>(InputFloat4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<float> >, bool>(
                                        const std::string &, const sol::table &, const std::string &, int)>(InputFloat4)
                                ));
        imGuiTable.set_function("InputInt", sol::overload(
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int)>(InputInt),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, int)>(InputInt),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, int, int)>(InputInt),
                                    // sol::resolve<std::tuple<int, bool>(const std::string &, int, int, int)>(InputInt), // Duplicate
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, int, int, int)>(
                                        InputInt)
                                ));
        imGuiTable.set_function("InputInt2", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &)>(InputInt2),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, int)>(InputInt2)
                                ));
        imGuiTable.set_function("InputInt3", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &)>(InputInt3),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, int)>(InputInt3)
                                ));
        imGuiTable.set_function("InputInt4", sol::overload(
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &)>(InputInt4),
                                    sol::resolve<std::tuple<sol::as_table_t<std::vector<int> >, bool>(
                                        const std::string &, const sol::table &, int)>(InputInt4)
                                ));
        imGuiTable.set_function("InputDouble", sol::overload(
                                    sol::resolve<std::tuple<double, bool>(const std::string &, double)>(InputDouble),
                                    sol::resolve<std::tuple<double, bool>(const std::string &, double, double)>(
                                        InputDouble),
                                    sol::resolve<std::tuple<double, bool>(const std::string &, double, double, double)>(
                                        InputDouble),
                                    sol::resolve<std::tuple<double, bool>(const std::string &, double, double, double,
                                                                          const std::string &)>(InputDouble),
                                    sol::resolve<std::tuple<double, bool>(const std::string &, double, double, double,
                                                                          const std::string &, int)>(InputDouble)
                                ));
    }

    void Init_Selectables(sol::table &imGuiTable) {
        imGuiTable.set_function("Selectable", sol::overload(
                                    sol::resolve<bool(const std::string &)>(Selectable), // Read-only
                                    sol::resolve<bool(const std::string &, bool)>(Selectable), // In-out
                                    sol::resolve<bool(const std::string &, bool, int)>(Selectable), // In-out with flags
                                    sol::resolve<bool(const std::string &, bool, int, float, float)>(Selectable)
                                    // In-out with flags and size
                                ));
    }

    void Init_WidgetsListBoxes(sol::table &imGuiTable) {
        imGuiTable.set_function("ListBox", sol::overload(
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, const sol::table &,
                                                                       int)>(
                                        ListBox),
                                    sol::resolve<std::tuple<int, bool>(const std::string &, int, const sol::table &,
                                                                       int,
                                                                       int)>(ListBox)
                                ));
        imGuiTable.set_function("BeginListBox", sol::overload( // Renamed from ListBoxHeader
                                    sol::resolve<bool(const std::string &, float, float)>(ListBoxHeader),
                                    // Kept ListBoxHeader name for sol binding for old scripts
                                    sol::resolve<bool(const std::string &, int, int)>(ListBoxHeader)
                                ));
        imGuiTable.set_function("EndListBox", ListBoxFooter); // Renamed from ListBoxFooter for sol binding
    }

    void Init_WidgetsMenu(sol::table &imGuiTable) {
        imGuiTable.set_function("BeginMenuBar", BeginMenuBar);
        imGuiTable.set_function("EndMenuBar", EndMenuBar);
        imGuiTable.set_function("BeginMainMenuBar", BeginMainMenuBar);
        imGuiTable.set_function("EndMainMenuBar", EndMainMenuBar);
        imGuiTable.set_function("BeginMenu", sol::overload(
                                    sol::resolve<bool(const std::string &)>(BeginMenu),
                                    sol::resolve<bool(const std::string &, bool)>(BeginMenu)
                                ));
        imGuiTable.set_function("EndMenu", EndMenu);
        imGuiTable.set_function("MenuItem", sol::overload(
                                    sol::resolve<bool(const std::string &)>(MenuItem),
                                    sol::resolve<bool(const std::string &, const std::string &)>(MenuItem),
                                    sol::resolve<bool(const std::string &, const std::string &, bool)>(MenuItem),
                                    // Read-only selected
                                    sol::resolve<bool(const std::string &, const std::string &, bool, bool)>(MenuItem)
                                    // Read-only selected + enabled
                                ));
        imGuiTable.set_function("MenuItemToggle",
                                sol::resolve<std::tuple<bool, bool>
                                    (const std::string &, const std::string &, bool, bool)>(MenuItemToggle));
    }

    void Init_InputsUtilitiesKeyboard(sol::table &imGuiTable) {
        imGuiTable.set_function("GetKeyIndex", GetKeyIndex);
        imGuiTable.set_function("IsKeyDown", IsKeyDown);
        imGuiTable.set_function("IsKeyPressed", sol::overload(
                                    sol::resolve<bool(int)>(IsKeyPressed),
                                    sol::resolve<bool(int, bool)>(IsKeyPressed)
                                ));
        imGuiTable.set_function("IsKeyReleased", IsKeyReleased);
        imGuiTable.set_function("GetKeyPressedAmount", GetKeyPressedAmount);
        imGuiTable.set_function("CaptureKeyboardFromApp", sol::overload( // Uses SetNextFrameWantCaptureKeyboard
                                    sol::resolve<void()>(CaptureKeyboardFromApp),
                                    sol::resolve<void(bool)>(CaptureKeyboardFromApp)
                                ));
    }

    void Init_InputsUtilitiesMouse(sol::table &imGuiTable) {
        imGuiTable.set_function("IsMouseDown", IsMouseDown);
        imGuiTable.set_function("IsMouseClicked", sol::overload(
                                    sol::resolve<bool(int)>(IsMouseClicked),
                                    sol::resolve<bool(int, bool)>(IsMouseClicked)
                                ));
        imGuiTable.set_function("IsMouseReleased", IsMouseReleased);
        imGuiTable.set_function("IsMouseDoubleClicked", IsMouseDoubleClicked);
        imGuiTable.set_function("IsMouseHoveringRect", sol::overload(
                                    sol::resolve<bool(float, float, float, float)>(IsMouseHoveringRect),
                                    sol::resolve<bool(float, float, float, float, bool)>(IsMouseHoveringRect)
                                ));
        imGuiTable.set_function("IsMousePosValid", sol::overload(
                                    sol::resolve<bool()>(IsMousePosValid),
                                    sol::resolve<bool(float, float)>(IsMousePosValid)
                                ));
        imGuiTable.set_function("IsAnyMouseDown", IsAnyMouseDown);
        imGuiTable.set_function("GetMousePos", GetMousePos);
        imGuiTable.set_function("GetMousePosOnOpeningCurrentPopup", GetMousePosOnOpeningCurrentPopup);
        imGuiTable.set_function("IsMouseDragging", sol::overload(
                                    sol::resolve<bool(int)>(IsMouseDragging),
                                    sol::resolve<bool(int, float)>(IsMouseDragging)
                                ));
        imGuiTable.set_function("GetMouseDragDelta", sol::overload(
                                    sol::resolve<std::tuple<float, float>()>(GetMouseDragDelta),
                                    sol::resolve<std::tuple<float, float>(int)>(GetMouseDragDelta),
                                    sol::resolve<std::tuple<float, float>(int, float)>(GetMouseDragDelta)
                                ));
        imGuiTable.set_function("ResetMouseDragDelta", sol::overload(
                                    sol::resolve<void()>(ResetMouseDragDelta),
                                    sol::resolve<void(int)>(ResetMouseDragDelta)
                                ));
        imGuiTable.set_function("GetMouseCursor", GetMouseCursor);
        imGuiTable.set_function("SetMouseCursor", SetMouseCursor);
        imGuiTable.set_function("CaptureMouseFromApp", sol::overload( // Uses SetNextFrameWantCaptureMouse
                                    sol::resolve<void()>(CaptureMouseFromApp),
                                    sol::resolve<void(bool)>(CaptureMouseFromApp)
                                ));
    }

    void Init_ClipboardUtilities(sol::table &imGuiTable) {
        imGuiTable.set_function("GetClipboardText", GetClipboardText);
        imGuiTable.set_function("SetClipboardText", SetClipboardText);
    }

    void InitInput(sol::table &imGuiTable) {
        Init_IDStackScopes(imGuiTable);
        Init_WidgetsMain(imGuiTable);
        Init_WidgetsDrags(imGuiTable);
        Init_WidgetsSliders(imGuiTable);
        Init_WidgetsInputKeyboard(imGuiTable);
        Init_Selectables(imGuiTable);
        Init_WidgetsListBoxes(imGuiTable);
        Init_WidgetsMenu(imGuiTable);
        Init_InputsUtilitiesKeyboard(imGuiTable);
        Init_InputsUtilitiesMouse(imGuiTable);
        Init_ClipboardUtilities(imGuiTable);
    }
#pragma endregion Init
}
