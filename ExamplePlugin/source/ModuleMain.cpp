#include <YYToolkit/YYTK_Shared.hpp>
using namespace Aurie;
using namespace YYTK;

#define MAX_OBJECT_LENGTH 8192
#define MAX_ARRAY_LENGTH 1024
#define DUMP_MAX_DEPTH 16

static YYTKInterface* g_ModuleInterface = nullptr;

void DumpInstanceRecursive(
	IN const RValue& Instance,
	IN int32_t Depth
);

void DumpArrayRecursive(
	IN const char* Name,
	IN const RValue& Value,
	IN int32_t Depth
);

// ==== CHATGPT CODE BEGIN ====
#include <fstream>
std::string FormatString(std::string_view Format, va_list args) {
	// Determine size of the formatted string
	int size = vsnprintf(nullptr, 0, Format.data(), args) + 1;
	if (size <= 0) return "";

	// Allocate a buffer to hold the formatted string
	std::vector<char> buffer(size);
	vsnprintf(buffer.data(), size, Format.data(), args);

	return std::string(buffer.data());
}

void PrintInfo(std::string_view Format, ...) {
	// Handle variadic arguments
	va_list args;
	va_start(args, Format);

	// Format the string
	std::string formattedString = FormatString(Format, args);

	va_end(args);

	// Append the formatted string to a file "output.txt"
	std::ofstream file("output.txt", std::ios::app);
	if (file.is_open()) {
		file << formattedString << std::endl;
		file.close();
	}
}
// ==== CHATGPT CODE END ====

CScript* TryLookupScriptByFunctionPointer(
	IN PFUNC_YYGMLScript ScriptFunction
)
{
	AurieStatus last_status = AURIE_SUCCESS;

	int script_index = 0;
	while (AurieSuccess(last_status))
	{
		CScript* script = nullptr;

		last_status = g_ModuleInterface->GetScriptData(
			script_index,
			script
		);

		if (!AurieSuccess(last_status))
			break;

		if (script->m_Functions)
		{
			if (script->m_Functions->m_ScriptFunction == ScriptFunction)
				return script;
		}

		script_index++;
	}
}

void DumpRValue(
	IN int32_t Depth,
	IN const char* Name,
	IN const RValue& Value
)
{
	std::string prefix = std::string(Depth * 2, ' ');

	switch (Value.m_Kind)
	{
	case VALUE_REAL:
	case VALUE_INT64:
	case VALUE_INT32:
	case VALUE_BOOL:
		PrintInfo("%s - [%s] %s = %.2f", prefix.c_str(), Value.GetKindName().c_str(), Name, Value.ToDouble());
		break;
	case VALUE_OBJECT:
	{
		YYObjectBase* object = Value.ToObject();
		switch (object->m_ObjectKind)
		{
		case OBJECT_KIND_SCRIPTREF:
		{
			CScriptRef* ref = Value.ToPointer<CScriptRef*>();
			CScript* script = TryLookupScriptByFunctionPointer(
				ref->m_CallYYC
			);

			if (script)
				PrintInfo("%s - [%s] %s = script '%s'", prefix.c_str(), Value.GetKindName().c_str(), Name, script->GetName());
			else
				PrintInfo("%s - [%s] %s", prefix.c_str(), Value.GetKindName().c_str(), Name);
			break;
		}
		case OBJECT_KIND_CINSTANCE:
		{
			CInstance* inst = Value.ToInstance();

			if (inst->m_Object && inst->m_Object->m_Name)
			{
				PrintInfo(
					"%s - [%s] %s = %p (instance of object %s)", 
					prefix.c_str(), 
					Value.GetKindName().c_str(), 
					Name, 
					Value.ToPointer(), 
					inst->m_Object->m_Name
				);
			}
			else
			{
				PrintInfo(
					"%s - [%s] %s = %p (instance of unknown object)",
					prefix.c_str(),
					Value.GetKindName().c_str(),
					Name,
					Value.ToPointer()
				);
			}

			if (Depth < DUMP_MAX_DEPTH)
				DumpInstanceRecursive(Value, Depth + 1);

			break;
		}
		default:
		{
			PrintInfo("%s - [%s] %s = %p", prefix.c_str(), Value.GetKindName().c_str(), Name, Value.ToPointer());
			if (Depth < DUMP_MAX_DEPTH)
				DumpInstanceRecursive(Value, Depth + 1);
			break;
		}
		}

		break;
	}
	case VALUE_STRING:
		PrintInfo("%s - [%s] %s = '%s'", prefix.c_str(), Value.GetKindName().c_str(), Name, Value.ToCString());
		break;
	case VALUE_ARRAY:
		PrintInfo("%s - [%s] %s", prefix.c_str(), Value.GetKindName().c_str(), Name);
		if (Depth < DUMP_MAX_DEPTH)
			DumpArrayRecursive(Name, Value, Depth + 1);
		break;
	default:
		PrintInfo("%s - [%s] %s", prefix.c_str(), Value.GetKindName().c_str(), Name);
		break;
	}
}

void DumpArrayRecursive(
	IN const char* Name,
	IN const RValue& Value,
	IN int32_t Depth
)
{
	auto vector = Value.ToVector();

	if (vector.size() >= MAX_ARRAY_LENGTH)
	{
		std::string prefix = std::string((Depth + 1) * 2, ' ');
		std::string name = Name;

		PrintInfo("%s - Skipping, array has over 1024 elements", prefix.c_str());
		return;
	}

	for (int i = 0; i < vector.size(); i++)
	{
		std::string name = Name;
		name.append("[");
		name.append(std::to_string(i));
		name.append("]");

		DumpRValue(Depth, name.c_str(), vector[i]);
	}
}

void DumpInstanceRecursive(
	IN const RValue& Instance,
	IN int32_t Depth
)
{
	auto key_value_map = Instance.ToMap();

	if (key_value_map.size() >= MAX_OBJECT_LENGTH)
	{
		std::string prefix = std::string((Depth + 1) * 2, ' ');
		PrintInfo("%s - Skipping, struct has over 1024 elements", prefix.c_str());
		return;
	}

	for (auto& [key, value] : key_value_map)
	{
		if (key == "__STORAGE_NODES")
		{
			std::string prefix = std::string((Depth) * 2, ' ');

			PrintInfo("%s - __STORAGE_NODES - skipping, this struct is a behemoth that doesn't contain useful info.", prefix.c_str());
			continue;
		}

		if (key == "__current_main_tree")
		{
			std::string prefix = std::string((Depth) * 2, ' ');

			PrintInfo("%s - __current_main_tree - skipping, this struct is a behemoth that doesn't contain useful info.", prefix.c_str());
			continue;
		}

		if (key == "__current_running_tree")
		{
			std::string prefix = std::string((Depth) * 2, ' ');

			PrintInfo("%s - __current_main_tree - skipping, this struct is a behemoth that doesn't contain useful info.", prefix.c_str());
			continue;
		}


		DumpRValue(Depth, key.c_str(), value);
	}
}

static bool g_Dumped = false;

void ObjectCallback(
	IN FWCodeEvent& CallContext
)
{
	auto& [self, other, code, argc, args] = CallContext.Arguments();

	if (GetAsyncKeyState(VK_F11) & 1)
	{
		g_Dumped = false;

		CInstance* global;
		g_ModuleInterface->GetGlobalInstance(&global);

		PrintInfo("==== BEGIN DUMP ====");
		DumpInstanceRecursive(global->ToRValue(), 0);
		PrintInfo("==== END DUMP ====");

		g_Dumped = true;

		g_ModuleInterface->InvokeWithObject(
			"obj_ari",
			[](CInstance* Self, CInstance* Other)
			{
				if (g_Dumped)
					return;

				PrintInfo("==== BEGIN DUMP ====");
				DumpInstanceRecursive(Self, 0);
				PrintInfo("==== END DUMP ====");

			}
		);
	}
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	// Gets a handle to the interface exposed by YYTK
	// You can keep this pointer for future use, as it will not change unless YYTK is unloaded.
	last_status = ObGetInterface(
		"YYTK_Main",
		(AurieInterfaceBase*&)(g_ModuleInterface)
	);

	// If we can't get the interface, we fail loading.
	if (!AurieSuccess(last_status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[Example Plugin] - Hello from PluginEntry!");

	last_status = g_ModuleInterface->CreateCallback(
		Module,
		EVENT_OBJECT_CALL,
		ObjectCallback,
		0
	);

	if (!AurieSuccess(last_status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[Example Plugin] - Failed to register callback!");
	}

	return AURIE_SUCCESS;
}