#include "sol_ImGui.h"

namespace sol_ImGui {
    void InitWindowFlags(sol::state &lua) {
        lua.new_enum("ImGuiWindowFlags",
                     "None", ImGuiWindowFlags_None,
                     "NoTitleBar", ImGuiWindowFlags_NoTitleBar,
                     "NoResize", ImGuiWindowFlags_NoResize,
                     "NoMove", ImGuiWindowFlags_NoMove,
                     "NoScrollbar", ImGuiWindowFlags_NoScrollbar,
                     "NoScrollWithMouse", ImGuiWindowFlags_NoScrollWithMouse,
                     "NoCollapse", ImGuiWindowFlags_NoCollapse,
                     "AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize,
                     "NoBackground", ImGuiWindowFlags_NoBackground,
                     "NoSavedSettings", ImGuiWindowFlags_NoSavedSettings,
                     "NoMouseInputs", ImGuiWindowFlags_NoMouseInputs,
                     "MenuBar", ImGuiWindowFlags_MenuBar,
                     "HorizontalScrollbar", ImGuiWindowFlags_HorizontalScrollbar,
                     "NoFocusOnAppearing", ImGuiWindowFlags_NoFocusOnAppearing,
                     "NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus,
                     "AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar,
                     "AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar,
                     "NoNavInputs", ImGuiWindowFlags_NoNavInputs,
                     "NoNavFocus", ImGuiWindowFlags_NoNavFocus,
                     "UnsavedDocument", ImGuiWindowFlags_UnsavedDocument,
                     "NoNav", ImGuiWindowFlags_NoNav,
                     "NoDecoration", ImGuiWindowFlags_NoDecoration,
                     "NoInputs", ImGuiWindowFlags_NoInputs,
                     // Internal flags, usually not set by user but part of the enum
                     "ChildWindow", ImGuiWindowFlags_ChildWindow,
                     "Tooltip", ImGuiWindowFlags_Tooltip,
                     "Popup", ImGuiWindowFlags_Popup,
                     "Modal", ImGuiWindowFlags_Modal,
                     "ChildMenu", ImGuiWindowFlags_ChildMenu
                     // Obsolete NavFlattened and AlwaysUseWindowPadding removed from public API,
                     // but they are still in imgui.h for internal/obsolete reasons.
                     // We don't bind them to avoid confusion for new users.
#ifdef IMGUI_HAS_DOCK // This preprocessor might not be standard for all ImGui versions
                     // If CUSTOM_IMGUI implies docking, those flags from original might be relevant
                     // For now, assuming these are available if a custom imgui with docking is used
                     // , "DockNodeHost", ImGuiWindowFlags_DockNodeHost
                     // These two seem custom and not standard ImGui docking flags
                     // , "NoClose", ImGuiWindowFlags_NoClose,
                     // "NoDockTab", ImGuiWindowFlags_NoDockTab
#endif
        );
    }

    void InitChildFlags(sol::state &lua) {
        lua.new_enum("ImGuiChildFlags",
                     "None", ImGuiChildFlags_None,
                     "Borders", ImGuiChildFlags_Borders,
                     "AlwaysUseWindowPadding", ImGuiChildFlags_AlwaysUseWindowPadding,
                     "ResizeX", ImGuiChildFlags_ResizeX,
                     "ResizeY", ImGuiChildFlags_ResizeY,
                     "AutoResizeX", ImGuiChildFlags_AutoResizeX,
                     "AutoResizeY", ImGuiChildFlags_AutoResizeY,
                     "AlwaysAutoResize", ImGuiChildFlags_AlwaysAutoResize,
                     "FrameStyle", ImGuiChildFlags_FrameStyle,
                     "NavFlattened", ImGuiChildFlags_NavFlattened
                     // Obsolete alias
        );
    }

    void InitItemFlags(sol::state &lua) {
        lua.new_enum("ImGuiItemFlags",
                     "None", ImGuiItemFlags_None,
                     "NoTabStop", ImGuiItemFlags_NoTabStop,
                     "NoNav", ImGuiItemFlags_NoNav,
                     "NoNavDefaultFocus", ImGuiItemFlags_NoNavDefaultFocus,
                     "ButtonRepeat", ImGuiItemFlags_ButtonRepeat,
                     "AutoClosePopups", ImGuiItemFlags_AutoClosePopups,
                     "AllowDuplicateId", ImGuiItemFlags_AllowDuplicateId
        );
    }

    void InitPopupFlags(sol::state &lua) {
        lua.new_enum("ImGuiPopupFlags",
                     "None", ImGuiPopupFlags_None,
                     "MouseButtonLeft", ImGuiPopupFlags_MouseButtonLeft,
                     "MouseButtonRight", ImGuiPopupFlags_MouseButtonRight,
                     "MouseButtonMiddle", ImGuiPopupFlags_MouseButtonMiddle,
                     "MouseButtonMask_", ImGuiPopupFlags_MouseButtonMask_,
                     "NoReopen", ImGuiPopupFlags_NoReopen,
                     "NoOpenOverExistingPopup", ImGuiPopupFlags_NoOpenOverExistingPopup,
                     "NoOpenOverItems", ImGuiPopupFlags_NoOpenOverItems,
                     "AnyPopupId", ImGuiPopupFlags_AnyPopupId,
                     "AnyPopupLevel", ImGuiPopupFlags_AnyPopupLevel,
                     "AnyPopup", ImGuiPopupFlags_AnyPopup
        );
    }

    void InitInputTextFlags(sol::state &lua) {
        lua.new_enum("ImGuiInputTextFlags",
                     "None", ImGuiInputTextFlags_None,
                     "CharsDecimal", ImGuiInputTextFlags_CharsDecimal,
                     "CharsHexadecimal", ImGuiInputTextFlags_CharsHexadecimal,
                     "CharsScientific", ImGuiInputTextFlags_CharsScientific,
                     "CharsUppercase", ImGuiInputTextFlags_CharsUppercase,
                     "CharsNoBlank", ImGuiInputTextFlags_CharsNoBlank,
                     "AllowTabInput", ImGuiInputTextFlags_AllowTabInput,
                     "EnterReturnsTrue", ImGuiInputTextFlags_EnterReturnsTrue,
                     "EscapeClearsAll", ImGuiInputTextFlags_EscapeClearsAll,
                     "CtrlEnterForNewLine", ImGuiInputTextFlags_CtrlEnterForNewLine,
                     "ReadOnly", ImGuiInputTextFlags_ReadOnly,
                     "Password", ImGuiInputTextFlags_Password,
                     "AlwaysOverwrite", ImGuiInputTextFlags_AlwaysOverwrite,
                     "AutoSelectAll", ImGuiInputTextFlags_AutoSelectAll,
                     "ParseEmptyRefVal", ImGuiInputTextFlags_ParseEmptyRefVal,
                     "DisplayEmptyRefVal", ImGuiInputTextFlags_DisplayEmptyRefVal,
                     "NoHorizontalScroll", ImGuiInputTextFlags_NoHorizontalScroll,
                     "NoUndoRedo", ImGuiInputTextFlags_NoUndoRedo,
                     "ElideLeft", ImGuiInputTextFlags_ElideLeft,
                     "CallbackCompletion", ImGuiInputTextFlags_CallbackCompletion,
                     "CallbackHistory", ImGuiInputTextFlags_CallbackHistory,
                     "CallbackAlways", ImGuiInputTextFlags_CallbackAlways,
                     "CallbackCharFilter", ImGuiInputTextFlags_CallbackCharFilter,
                     "CallbackResize", ImGuiInputTextFlags_CallbackResize,
                     "CallbackEdit", ImGuiInputTextFlags_CallbackEdit
        );
    }

    void InitTreeNodeFlags(sol::state &lua) {
        lua.new_enum("ImGuiTreeNodeFlags",
                     "None", ImGuiTreeNodeFlags_None,
                     "Selected", ImGuiTreeNodeFlags_Selected,
                     "Framed", ImGuiTreeNodeFlags_Framed,
                     "AllowOverlap", ImGuiTreeNodeFlags_AllowOverlap,
                     "NoTreePushOnOpen", ImGuiTreeNodeFlags_NoTreePushOnOpen,
                     "NoAutoOpenOnLog", ImGuiTreeNodeFlags_NoAutoOpenOnLog,
                     "DefaultOpen", ImGuiTreeNodeFlags_DefaultOpen,
                     "OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick,
                     "OpenOnArrow", ImGuiTreeNodeFlags_OpenOnArrow,
                     "Leaf", ImGuiTreeNodeFlags_Leaf,
                     "Bullet", ImGuiTreeNodeFlags_Bullet,
                     "FramePadding", ImGuiTreeNodeFlags_FramePadding,
                     "SpanAvailWidth", ImGuiTreeNodeFlags_SpanAvailWidth,
                     "SpanFullWidth", ImGuiTreeNodeFlags_SpanFullWidth,
                     "SpanLabelWidth", ImGuiTreeNodeFlags_SpanLabelWidth,
                     "SpanAllColumns", ImGuiTreeNodeFlags_SpanAllColumns,
                     "LabelSpanAllColumns", ImGuiTreeNodeFlags_LabelSpanAllColumns,
                     "NavLeftJumpsBackHere", ImGuiTreeNodeFlags_NavLeftJumpsBackHere,
                     "CollapsingHeader", ImGuiTreeNodeFlags_CollapsingHeader
        );
    }

    void InitSelectableFlags(sol::state &lua) {
        lua.new_enum("ImGuiSelectableFlags",
                     "None", ImGuiSelectableFlags_None,
                     "NoAutoClosePopups", ImGuiSelectableFlags_NoAutoClosePopups,
                     "SpanAllColumns", ImGuiSelectableFlags_SpanAllColumns,
                     "AllowDoubleClick", ImGuiSelectableFlags_AllowDoubleClick,
                     "Disabled", ImGuiSelectableFlags_Disabled,
                     "AllowOverlap", ImGuiSelectableFlags_AllowOverlap,
                     "Highlight", ImGuiSelectableFlags_Highlight
        );
    }

    void InitComboFlags(sol::state &lua) {
        lua.new_enum("ImGuiComboFlags",
                     "None", ImGuiComboFlags_None,
                     "PopupAlignLeft", ImGuiComboFlags_PopupAlignLeft,
                     "HeightSmall", ImGuiComboFlags_HeightSmall,
                     "HeightRegular", ImGuiComboFlags_HeightRegular,
                     "HeightLarge", ImGuiComboFlags_HeightLarge,
                     "HeightLargest", ImGuiComboFlags_HeightLargest,
                     "NoArrowButton", ImGuiComboFlags_NoArrowButton,
                     "NoPreview", ImGuiComboFlags_NoPreview,
                     "WidthFitPreview", ImGuiComboFlags_WidthFitPreview,
                     "HeightMask", ImGuiComboFlags_HeightMask_
        );
    }

    void InitTabBarFlags(sol::state &lua) {
        lua.new_enum("ImGuiTabBarFlags",
                     "None", ImGuiTabBarFlags_None,
                     "Reorderable", ImGuiTabBarFlags_Reorderable,
                     "AutoSelectNewTabs", ImGuiTabBarFlags_AutoSelectNewTabs,
                     "TabListPopupButton", ImGuiTabBarFlags_TabListPopupButton,
                     "NoCloseWithMiddleMouseButton", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton,
                     "NoTabListScrollingButtons", ImGuiTabBarFlags_NoTabListScrollingButtons,
                     "NoTooltip", ImGuiTabBarFlags_NoTooltip,
                     "DrawSelectedOverline", ImGuiTabBarFlags_DrawSelectedOverline,
                     "FittingPolicyResizeDown", ImGuiTabBarFlags_FittingPolicyResizeDown,
                     "FittingPolicyScroll", ImGuiTabBarFlags_FittingPolicyScroll,
                     "FittingPolicyMask_", ImGuiTabBarFlags_FittingPolicyMask_,
                     "FittingPolicyDefault_", ImGuiTabBarFlags_FittingPolicyDefault_
        );
    }

    void InitTabItemFlags(sol::state &lua) {
        lua.new_enum("ImGuiTabItemFlags",
                     "None", ImGuiTabItemFlags_None,
                     "UnsavedDocument", ImGuiTabItemFlags_UnsavedDocument,
                     "SetSelected", ImGuiTabItemFlags_SetSelected,
                     "NoCloseWithMiddleMouseButton", ImGuiTabItemFlags_NoCloseWithMiddleMouseButton,
                     "NoPushId", ImGuiTabItemFlags_NoPushId,
                     "NoTooltip", ImGuiTabItemFlags_NoTooltip,
                     "NoReorder", ImGuiTabItemFlags_NoReorder,
                     "Leading", ImGuiTabItemFlags_Leading,
                     "Trailing", ImGuiTabItemFlags_Trailing,
                     "NoAssumedClosure", ImGuiTabItemFlags_NoAssumedClosure
        );
    }

    void InitDirFlags(sol::state &lua) {
        lua.new_enum("ImGuiDir",
                     "None", ImGuiDir_None,
                     "Left", ImGuiDir_Left,
                     "Right", ImGuiDir_Right,
                     "Up", ImGuiDir_Up,
                     "Down", ImGuiDir_Down,
                     "COUNT", ImGuiDir_COUNT
        );
    }

    void InitMouseCursor(sol::state &lua) {
        lua.new_enum("ImGuiMouseCursor",
                     "None", ImGuiMouseCursor_None,
                     "Arrow", ImGuiMouseCursor_Arrow,
                     "TextInput", ImGuiMouseCursor_TextInput,
                     "ResizeAll", ImGuiMouseCursor_ResizeAll,
                     "ResizeNS", ImGuiMouseCursor_ResizeNS,
                     "ResizeEW", ImGuiMouseCursor_ResizeEW,
                     "ResizeNESW", ImGuiMouseCursor_ResizeNESW,
                     "ResizeNWSE", ImGuiMouseCursor_ResizeNWSE,
                     "Hand", ImGuiMouseCursor_Hand,
                     "NotAllowed", ImGuiMouseCursor_NotAllowed,
                     "COUNT", ImGuiMouseCursor_COUNT
        );
    }

    void InitCondFlags(sol::state &lua) {
        lua.new_enum("ImGuiCond",
                     "None", ImGuiCond_None,
                     "Always", ImGuiCond_Always,
                     "Once", ImGuiCond_Once,
                     "FirstUseEver", ImGuiCond_FirstUseEver,
                     "Appearing", ImGuiCond_Appearing
        );
    }

    void InitKeyFlags(sol::state &lua) {
        lua.new_enum("ImGuiKey",
                     "None", ImGuiKey_None,
                     "Tab", ImGuiKey_Tab,
                     "LeftArrow", ImGuiKey_LeftArrow,
                     "RightArrow", ImGuiKey_RightArrow,
                     "UpArrow", ImGuiKey_UpArrow,
                     "DownArrow", ImGuiKey_DownArrow,
                     "PageUp", ImGuiKey_PageUp,
                     "PageDown", ImGuiKey_PageDown,
                     "Home", ImGuiKey_Home,
                     "End", ImGuiKey_End,
                     "Insert", ImGuiKey_Insert,
                     "Delete", ImGuiKey_Delete,
                     "Backspace", ImGuiKey_Backspace,
                     "Space", ImGuiKey_Space,
                     "Enter", ImGuiKey_Enter,
                     "Escape", ImGuiKey_Escape,
                     "KeypadEnter", ImGuiKey_KeypadEnter, // Renamed from KeyPadEnter
                     "A", ImGuiKey_A,
                     "C", ImGuiKey_C,
                     "V", ImGuiKey_V,
                     "X", ImGuiKey_X,
                     "Y", ImGuiKey_Y,
                     "Z", ImGuiKey_Z,
                     // Modifiers (can also be OR'd)
                     "ModCtrl", ImGuiMod_Ctrl,
                     "ModShift", ImGuiMod_Shift,
                     "ModAlt", ImGuiMod_Alt,
                     "ModSuper", ImGuiMod_Super,
                     // Adding some gamepad keys as they are common
                     "GamepadFaceDown", ImGuiKey_GamepadFaceDown, // A or Cross
                     "GamepadFaceRight", ImGuiKey_GamepadFaceRight, // B or Circle
                     "GamepadFaceLeft", ImGuiKey_GamepadFaceLeft, // X or Square
                     "GamepadFaceUp", ImGuiKey_GamepadFaceUp, // Y or Triangle
                     "GamepadStart", ImGuiKey_GamepadStart,
                     "GamepadBack", ImGuiKey_GamepadBack
                     // ImGuiKey_COUNT is not part of the user-facing enum
        );
    }

    void InitFocusedFlags(sol::state &lua) {
        lua.new_enum("ImGuiFocusedFlags",
                     "None", ImGuiFocusedFlags_None,
                     "ChildWindows", ImGuiFocusedFlags_ChildWindows,
                     "RootWindow", ImGuiFocusedFlags_RootWindow,
                     "AnyWindow", ImGuiFocusedFlags_AnyWindow,
                     "NoPopupHierarchy", ImGuiFocusedFlags_NoPopupHierarchy,
                     "RootAndChildWindows", ImGuiFocusedFlags_RootAndChildWindows
        );
    }

    void InitHoveredFlags(sol::state &lua) {
        lua.new_enum("ImGuiHoveredFlags",
                     "None", ImGuiHoveredFlags_None,
                     "ChildWindows", ImGuiHoveredFlags_ChildWindows,
                     "RootWindow", ImGuiHoveredFlags_RootWindow,
                     "AnyWindow", ImGuiHoveredFlags_AnyWindow,
                     "NoPopupHierarchy", ImGuiHoveredFlags_NoPopupHierarchy,
                     "AllowWhenBlockedByPopup", ImGuiHoveredFlags_AllowWhenBlockedByPopup,
                     "AllowWhenBlockedByActiveItem", ImGuiHoveredFlags_AllowWhenBlockedByActiveItem,
                     "AllowWhenOverlappedByItem", ImGuiHoveredFlags_AllowWhenOverlappedByItem,
                     "AllowWhenOverlappedByWindow", ImGuiHoveredFlags_AllowWhenOverlappedByWindow,
                     "AllowWhenDisabled", ImGuiHoveredFlags_AllowWhenDisabled,
                     "NoNavOverride", ImGuiHoveredFlags_NoNavOverride,
                     "RectOnly", ImGuiHoveredFlags_RectOnly, // Combination flag
                     "RootAndChildWindows", ImGuiHoveredFlags_RootAndChildWindows,
                     "ForTooltip", ImGuiHoveredFlags_ForTooltip,
                     "Stationary", ImGuiHoveredFlags_Stationary,
                     "DelayNone", ImGuiHoveredFlags_DelayNone,
                     "DelayShort", ImGuiHoveredFlags_DelayShort,
                     "DelayNormal", ImGuiHoveredFlags_DelayNormal,
                     "NoSharedDelay", ImGuiHoveredFlags_NoSharedDelay
        );
    }

    void InitCol(sol::state &lua) {
        lua.new_enum("ImGuiCol",
                     "Text", ImGuiCol_Text,
                     "TextDisabled", ImGuiCol_TextDisabled,
                     "WindowBg", ImGuiCol_WindowBg,
                     "ChildBg", ImGuiCol_ChildBg,
                     "PopupBg", ImGuiCol_PopupBg,
                     "Border", ImGuiCol_Border,
                     "BorderShadow", ImGuiCol_BorderShadow,
                     "FrameBg", ImGuiCol_FrameBg,
                     "FrameBgHovered", ImGuiCol_FrameBgHovered,
                     "FrameBgActive", ImGuiCol_FrameBgActive,
                     "TitleBg", ImGuiCol_TitleBg,
                     "TitleBgActive", ImGuiCol_TitleBgActive,
                     "TitleBgCollapsed", ImGuiCol_TitleBgCollapsed,
                     "MenuBarBg", ImGuiCol_MenuBarBg,
                     "ScrollbarBg", ImGuiCol_ScrollbarBg,
                     "ScrollbarGrab", ImGuiCol_ScrollbarGrab,
                     "ScrollbarGrabHovered", ImGuiCol_ScrollbarGrabHovered,
                     "ScrollbarGrabActive", ImGuiCol_ScrollbarGrabActive,
                     "CheckMark", ImGuiCol_CheckMark,
                     "SliderGrab", ImGuiCol_SliderGrab,
                     "SliderGrabActive", ImGuiCol_SliderGrabActive,
                     "Button", ImGuiCol_Button,
                     "ButtonHovered", ImGuiCol_ButtonHovered,
                     "ButtonActive", ImGuiCol_ButtonActive,
                     "Header", ImGuiCol_Header,
                     "HeaderHovered", ImGuiCol_HeaderHovered,
                     "HeaderActive", ImGuiCol_HeaderActive,
                     "Separator", ImGuiCol_Separator,
                     "SeparatorHovered", ImGuiCol_SeparatorHovered,
                     "SeparatorActive", ImGuiCol_SeparatorActive,
                     "ResizeGrip", ImGuiCol_ResizeGrip,
                     "ResizeGripHovered", ImGuiCol_ResizeGripHovered,
                     "ResizeGripActive", ImGuiCol_ResizeGripActive,
                     "Tab", ImGuiCol_Tab,
                     "TabHovered", ImGuiCol_TabHovered,
                     "TabSelected", ImGuiCol_TabSelected,
                     "TabSelectedOverline", ImGuiCol_TabSelectedOverline,
                     "TabDimmed", ImGuiCol_TabDimmed,
                     "TabDimmedSelected", ImGuiCol_TabDimmedSelected,
                     "TabDimmedSelectedOverline", ImGuiCol_TabDimmedSelectedOverline,
                     "PlotLines", ImGuiCol_PlotLines,
                     "PlotLinesHovered", ImGuiCol_PlotLinesHovered,
                     "PlotHistogram", ImGuiCol_PlotHistogram,
                     "PlotHistogramHovered", ImGuiCol_PlotHistogramHovered,
                     "TableHeaderBg", ImGuiCol_TableHeaderBg,
                     "TableBorderStrong", ImGuiCol_TableBorderStrong,
                     "TableBorderLight", ImGuiCol_TableBorderLight,
                     "TableRowBg", ImGuiCol_TableRowBg,
                     "TableRowBgAlt", ImGuiCol_TableRowBgAlt,
                     "TextLink", ImGuiCol_TextLink,
                     "TextSelectedBg", ImGuiCol_TextSelectedBg,
                     "DragDropTarget", ImGuiCol_DragDropTarget,
                     "NavCursor", ImGuiCol_NavCursor,
                     "NavWindowingHighlight", ImGuiCol_NavWindowingHighlight,
                     "NavWindowingDimBg", ImGuiCol_NavWindowingDimBg,
                     "ModalWindowDimBg", ImGuiCol_ModalWindowDimBg,
                     "ModalWindowDarkening", ImGuiCol_ModalWindowDimBg, // Alias for ModalWindowDimBg
                     "COUNT", ImGuiCol_COUNT
        );
    }

    void InitStyleVar(sol::state &lua) {
        lua.new_enum("ImGuiStyleVar",
                     "Alpha", ImGuiStyleVar_Alpha,
                     "DisabledAlpha", ImGuiStyleVar_DisabledAlpha,
                     "WindowPadding", ImGuiStyleVar_WindowPadding,
                     "WindowRounding", ImGuiStyleVar_WindowRounding,
                     "WindowBorderSize", ImGuiStyleVar_WindowBorderSize,
                     "WindowMinSize", ImGuiStyleVar_WindowMinSize,
                     "WindowTitleAlign", ImGuiStyleVar_WindowTitleAlign,
                     "ChildRounding", ImGuiStyleVar_ChildRounding,
                     "ChildBorderSize", ImGuiStyleVar_ChildBorderSize,
                     "PopupRounding", ImGuiStyleVar_PopupRounding,
                     "PopupBorderSize", ImGuiStyleVar_PopupBorderSize,
                     "FramePadding", ImGuiStyleVar_FramePadding,
                     "FrameRounding", ImGuiStyleVar_FrameRounding,
                     "FrameBorderSize", ImGuiStyleVar_FrameBorderSize,
                     "ItemSpacing", ImGuiStyleVar_ItemSpacing,
                     "ItemInnerSpacing", ImGuiStyleVar_ItemInnerSpacing,
                     "IndentSpacing", ImGuiStyleVar_IndentSpacing,
                     "CellPadding", ImGuiStyleVar_CellPadding,
                     "ScrollbarSize", ImGuiStyleVar_ScrollbarSize,
                     "ScrollbarRounding", ImGuiStyleVar_ScrollbarRounding,
                     "GrabMinSize", ImGuiStyleVar_GrabMinSize,
                     "GrabRounding", ImGuiStyleVar_GrabRounding,
                     "ImageBorderSize", ImGuiStyleVar_ImageBorderSize,
                     "TabRounding", ImGuiStyleVar_TabRounding,
                     "TabBorderSize", ImGuiStyleVar_TabBorderSize,
                     "TabBarBorderSize", ImGuiStyleVar_TabBarBorderSize,
                     "TabBarOverlineSize", ImGuiStyleVar_TabBarOverlineSize,
                     "TableAngledHeadersAngle", ImGuiStyleVar_TableAngledHeadersAngle,
                     "TableAngledHeadersTextAlign", ImGuiStyleVar_TableAngledHeadersTextAlign,
                     "ButtonTextAlign", ImGuiStyleVar_ButtonTextAlign,
                     "SelectableTextAlign", ImGuiStyleVar_SelectableTextAlign,
                     "SeparatorTextBorderSize", ImGuiStyleVar_SeparatorTextBorderSize,
                     "SeparatorTextAlign", ImGuiStyleVar_SeparatorTextAlign,
                     "SeparatorTextPadding", ImGuiStyleVar_SeparatorTextPadding,
                     "COUNT", ImGuiStyleVar_COUNT
        );
    }

    void InitColorEditFlags(sol::state &lua) {
        lua.new_enum("ImGuiColorEditFlags",
                     "None", ImGuiColorEditFlags_None,
                     "NoAlpha", ImGuiColorEditFlags_NoAlpha,
                     "NoPicker", ImGuiColorEditFlags_NoPicker,
                     "NoOptions", ImGuiColorEditFlags_NoOptions,
                     "NoSmallPreview", ImGuiColorEditFlags_NoSmallPreview,
                     "NoInputs", ImGuiColorEditFlags_NoInputs,
                     "NoTooltip", ImGuiColorEditFlags_NoTooltip,
                     "NoLabel", ImGuiColorEditFlags_NoLabel,
                     "NoSidePreview", ImGuiColorEditFlags_NoSidePreview,
                     "NoDragDrop", ImGuiColorEditFlags_NoDragDrop,
                     "NoBorder", ImGuiColorEditFlags_NoBorder,
                     "AlphaOpaque", ImGuiColorEditFlags_AlphaOpaque,
                     "AlphaNoBg", ImGuiColorEditFlags_AlphaNoBg,
                     "AlphaPreviewHalf", ImGuiColorEditFlags_AlphaPreviewHalf,
                     "AlphaBar", ImGuiColorEditFlags_AlphaBar,
                     "HDR", ImGuiColorEditFlags_HDR,
                     "DisplayRGB", ImGuiColorEditFlags_DisplayRGB,
                     "DisplayHSV", ImGuiColorEditFlags_DisplayHSV,
                     "DisplayHex", ImGuiColorEditFlags_DisplayHex,
                     "Uint8", ImGuiColorEditFlags_Uint8,
                     "Float", ImGuiColorEditFlags_Float,
                     "PickerHueBar", ImGuiColorEditFlags_PickerHueBar,
                     "PickerHueWheel", ImGuiColorEditFlags_PickerHueWheel,
                     "InputRGB", ImGuiColorEditFlags_InputRGB,
                     "InputHSV", ImGuiColorEditFlags_InputHSV,
                     "_DefaultOptions", ImGuiColorEditFlags_DefaultOptions_,
                     "_AlphaMask", ImGuiColorEditFlags_AlphaMask_,
                     "_DisplayMask", ImGuiColorEditFlags_DisplayMask_,
                     "_DataTypeMask", ImGuiColorEditFlags_DataTypeMask_,
                     "_PickerMask", ImGuiColorEditFlags_PickerMask_,
                     "_InputMask", ImGuiColorEditFlags_InputMask_
        );
    }

    void InitMouseButtonFlags(sol::state &lua) {
        lua.new_enum("ImGuiMouseButton",
                     "Left", ImGuiMouseButton_Left,
                     "Right", ImGuiMouseButton_Right,
                     "Middle", ImGuiMouseButton_Middle,
                     "COUNT", ImGuiMouseButton_COUNT
        );
    }

    void InitEnums(sol::state &lua) {
        InitWindowFlags(lua);
        InitChildFlags(lua);
        InitItemFlags(lua);
        InitFocusedFlags(lua);
        InitHoveredFlags(lua);
        InitCondFlags(lua);
        InitCol(lua);
        InitStyleVar(lua);
        InitDirFlags(lua);
        InitComboFlags(lua);
        InitInputTextFlags(lua);
        InitColorEditFlags(lua);
        InitTreeNodeFlags(lua);
        InitSelectableFlags(lua);
        InitPopupFlags(lua);
        InitTabBarFlags(lua);;
        InitTabItemFlags(lua);
        InitMouseButtonFlags(lua);
        InitKeyFlags(lua);
        InitMouseCursor(lua);
    }
}