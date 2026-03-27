#include "sol_ImGui.h"

namespace sol_ImGui {
    void LogToTTY() { ImGui::LogToTTY(); }

    void LogToTTY(int auto_open_depth) { ImGui::LogToTTY(auto_open_depth); }

    void LogToFile() { ImGui::LogToFile(); }

    void LogToFile(int auto_open_depth) { ImGui::LogToFile(auto_open_depth); }

    void LogToFile(int auto_open_depth, const std::string &filename) {
        ImGui::LogToFile(auto_open_depth, filename.c_str());
    }

    void LogToClipboard() { ImGui::LogToClipboard(); }

    void LogToClipboard(int auto_open_depth) { ImGui::LogToClipboard(auto_open_depth); }

    void LogFinish() { ImGui::LogFinish(); }

    void LogButtons() { ImGui::LogButtons(); }

    void LogText(const std::string &text) { ImGui::LogText("%s", text.c_str()); }

    double GetTime() { return ImGui::GetTime(); }

    int GetFrameCount() { return ImGui::GetFrameCount(); }

    std::string GetStyleColorName(int idx) {
        return std::string(ImGui::GetStyleColorName(idx));
    }

    std::tuple<float, float> CalcTextSize(const std::string &text, bool hide_text_after_double_hash, float wrap_width) {
        const auto vec2{ImGui::CalcTextSize(text.c_str(), NULL, hide_text_after_double_hash, wrap_width)};
        return std::make_tuple(vec2.x, vec2.y);
    }

    std::tuple<float, float> CalcTextSize(const std::string &text) {
        return CalcTextSize(text, false, -1.0f);
    }

    std::tuple<float, float> CalcTextSize(const std::string &text, bool hide_text_after_double_hash) {
        return CalcTextSize(text, hide_text_after_double_hash, -1.0f);
    }


#ifdef SOL_IMGUI_USE_COLOR_U32
    sol::as_table_t<std::vector<float>> ColorConvertU32ToFloat4(unsigned int in)
    {
        const auto vec4 = ImGui::ColorConvertU32ToFloat4(in);
        sol::as_table_t rgba = sol::as_table(std::vector<float>{vec4.x, vec4.y, vec4.z, vec4.w});
        return rgba;
    }
    unsigned int ColorConvertFloat4ToU32(const sol::table& rgba)
    {
        const lua_Number r{ rgba[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                         g{ rgba[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                         b{ rgba[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                         a{ rgba[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(1.0f)) }; // Default alpha 1.0f
        return ImGui::ColorConvertFloat4ToU32({ float(r), float(g), float(b), float(a) });
    }
#endif

    std::tuple<float, float, float> ColorConvertRGBtoHSV(float r, float g, float b) {
        float h{}, s{}, v{};
        ImGui::ColorConvertRGBtoHSV(r, g, b, h, s, v);
        return std::make_tuple(h, s, v);
    }

    std::tuple<float, float, float> ColorConvertHSVtoRGB(float h, float s, float v) {
        float r{}, g{}, b{};
        ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);
        return std::make_tuple(r, g, b);
    }

#pragma region Init
    void InitUtility(sol::table &imGuiTable) {
        imGuiTable.set_function("LogToTTY", sol::overload(
                                    sol::resolve<void()>(LogToTTY),
                                    sol::resolve<void(int)>(LogToTTY)
                                ));
        imGuiTable.set_function("LogToFile", sol::overload(
                                    sol::resolve<void()>(LogToFile), // Added
                                    sol::resolve<void(int)>(LogToFile),
                                    sol::resolve<void(int, const std::string &)>(LogToFile)
                                ));
        imGuiTable.set_function("LogToClipboard", sol::overload(
                                    sol::resolve<void()>(LogToClipboard),
                                    sol::resolve<void(int)>(LogToClipboard)
                                ));
        imGuiTable.set_function("LogFinish", LogFinish);
        imGuiTable.set_function("LogButtons", LogButtons);
        imGuiTable.set_function("LogText", LogText);
        imGuiTable.set_function("CalcTextSize", sol::overload(
                                    sol::resolve<std::tuple<float, float>(const std::string &)>(CalcTextSize),
                                    sol::resolve<std::tuple<float, float>(const std::string &, bool)>(CalcTextSize),
                                    sol::resolve<std::tuple<float, float>(const std::string &, bool, float)>(
                                        CalcTextSize)
                                ));
#ifdef SOL_IMGUI_USE_COLOR_U32
        ImGui.set_function("ColorConvertU32ToFloat4"		, ColorConvertU32ToFloat4);
        ImGui.set_function("ColorConvertFloat4ToU32"		, ColorConvertFloat4ToU32);
#endif
        imGuiTable.set_function("ColorConvertRGBtoHSV", ColorConvertRGBtoHSV);
        imGuiTable.set_function("ColorConvertHSVtoRGB", ColorConvertHSVtoRGB);
    }
#pragma endregion Init
}
