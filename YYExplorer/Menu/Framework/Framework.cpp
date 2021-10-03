#include "Framework.hpp"

namespace Framework
{
	namespace GUI
	{
		void ApplyStylingDark()
		{
			ImVec4* colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.67f, 0.67f, 0.67f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.11f, 0.98f);
			colors[ImGuiCol_Border] = ImVec4(0.09f, 0.44f, 0.71f, 0.82f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.39f, 0.69f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.62f, 0.62f, 0.62f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.73f, 1.00f, 0.78f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.91f, 1.00f, 0.86f);
			colors[ImGuiCol_Button] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.47f, 0.78f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.47f, 0.78f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.35f, 0.35f, 0.35f, 0.47f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
			colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
			colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.47f, 0.78f, 1.00f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 1.00f, 0.16f, 0.99f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.03f, 0.38f, 1.00f, 0.56f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(0.47f, 0.24f, 0.89f, 0.74f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.65f);
		}

		void StyleFonts(ImGuiIO& _refIO)
		{
			char Buffer[MAX_PATH];
			if (!GetEnvironmentVariableA("systemroot", Buffer, MAX_PATH))
				printf("[YYExplorer] ERROR: Cannot get %%systemroot%% envvar!\n");

			_refIO.Fonts->AddFontFromFileTTF(strncat(Buffer, "\\Fonts\\Segoeui.ttf", MAX_PATH), 18.0f);
			_refIO.Fonts->Build();
		}
	}

	namespace Wrappers
	{
		std::string RValueKind_ToString(const YYRValue& _Value)
		{
			RValue& Value = _Value.As<RValue>();

			switch (Value.Kind & VALUE_UNSET)
			{
			case VALUE_REAL:
				return "Real (" + std::to_string(Value.Real) + ")";
			case VALUE_STRING:
				return "String " + std::string(Value.String->Get());
			case VALUE_ARRAY:
				return "Array";
			case VALUE_PTR:
				return "Pointer";
			case VALUE_VEC3:
				return "3D Vector";
			case VALUE_UNDEFINED:
				return "Undefined";
			case VALUE_OBJECT:
				if (Value.Object->m_class)
					return "Object (" + std::string(Value.Object->m_class) + ")";
				return "Object";
			case VALUE_INT32:
				return "32-bit Integer (" + std::to_string(Value.I32) + ")";
			case VALUE_VEC4:
				return "4D Vector";
			case VALUE_INT64:
				return "64-bit Integer (" + std::to_string(Value.I64) + ")";
			case VALUE_ACCESSOR:
				return "Accessor";
			case VALUE_NULL:
				return "Javascript NULL";
			case VALUE_BOOL:
				return "Boolean (" + std::to_string(Value.Real) + ")";
			case VALUE_ITERATOR:
				return "Iterator";
			case VALUE_REF:
				return "Reference";
			case VALUE_UNSET:
				return "Unset";
			default:
				return "Unknown";
			}
		}

		YYRValue GetGlobal(const char* _Name)
		{
			using PFN_CALLBUILTIN = YYTKStatus(__cdecl*)(const char* _name, int _argc, YYRValue& _result, YYRValue* _args);
			static auto Global_CallBuiltin = g_pCurrentPlugin->GetCoreExport<PFN_CALLBUILTIN>("Global_CallBuiltin");

			YYRValue Result; YYRValue Name = _Name;
			Global_CallBuiltin("variable_global_get", 1, Result, &Name);

			return Result;
		}

		void SetGlobal(const char* _Name, const YYRValue& ref)
		{
			using PFN_CALLBUILTIN = YYTKStatus(__cdecl*)(const char* _name, int _argc, YYRValue& _result, YYRValue* _args);
			static auto Global_CallBuiltin = g_pCurrentPlugin->GetCoreExport<PFN_CALLBUILTIN>("Global_CallBuiltin");

			YYRValue Result; YYRValue Args[2] = { _Name, ref }; // Copies ref
			Global_CallBuiltin("variable_global_set", 2, Result, Args);
		}
	}
}