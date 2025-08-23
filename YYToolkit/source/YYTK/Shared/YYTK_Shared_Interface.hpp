// File: YYTK_Shared_Interface.hpp
// 
// Defines the interface, and a GetInterface() function, which returns the YYTKInterface instance.

#include "YYTK_Shared_Base.hpp"
#include "YYTK_Shared_Types.hpp"
#include <FunctionWrapper/FunctionWrapper.hpp>
#include <d3d11.h>

namespace YYTK
{
	// ExecuteIt
	using FWCodeEvent = FunctionWrapper<bool(CInstance*, CInstance*, CCode*, int, RValue*)>;
	// IDXGISwapChain::Present
	using FWFrame = FunctionWrapper<HRESULT(IDXGISwapChain*, UINT, UINT)>;
	// IDXGISwapChain::ResizeBuffers
	using FWResize = FunctionWrapper<HRESULT(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)>;
	// WndProc calls
	using FWWndProc = FunctionWrapper<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

	struct YYTKInterface : public Aurie::AurieInterfaceBase
	{
		// === Interface Functions ===
		virtual Aurie::AurieStatus Create() = 0;

		virtual void Destroy() = 0;

		virtual void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		) = 0;

		/**
		 * \brief Looks up a named object in various runner data structures.
		 * \param FunctionName Name of the object to look up. Either a GameMaker built-in function, or a script.
		 * \param FunctionIndex A buffer into which the index is stored.
		 * \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if the object does not exist.
		 */
		virtual Aurie::AurieStatus GetNamedRoutineIndex(
			IN const char* FunctionName,
			OUT int* FunctionIndex
		) = 0;

		/**
		 * \brief Retrieves a pointer to a named object.
		 * \param FunctionName Case-sensitive name of the object to look up. Either a GameMaker built-in function, or a script.
		 * \param FunctionPointer A buffer into which the pointer is stored. Points to a TRoutine for GameMaker built-in functions, an to a CScript for scripts.
		 * \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if the object does not exist. AURIE_ACCESS_DENIED for extension functions.
		 */
		virtual Aurie::AurieStatus GetNamedRoutinePointer(
			IN const char* FunctionName,
			OUT PVOID* FunctionPointer
		) = 0;

		/**
		 * \brief Returns a pointer to the instance representing the GameMaker global namespace.
		 * \param Instance A buffer into which the pointer to the global instance is written.
		 * \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if the runner is incompatible.
		 */
		virtual Aurie::AurieStatus GetGlobalInstance(
			OUT CInstance** Instance
		) = 0;

		/**
		 * \brief Calls a built-in function from within the global context.
		 * \param FunctionName The name of the built-in function to call.
		 * \param Arguments A vector of all parameters passed to the built-in function. Order is same as in native GameMaker.
		 * \return An RValue representing the function's result. Unset on failure.
		 */
		virtual RValue CallBuiltin(
			IN const char* FunctionName,
			IN std::vector<RValue> Arguments
		) = 0;

		/**
		 * \brief Calls a built-in function from within user-defined context.
		 * \param Result Buffer into which the result of the call is written.
		 * \param FunctionName The name of the built-in function to call.
		 * \param SelfInstance The instance object representing "self".
		 * \param OtherInstance The instance object representing "other".
		 * \param Arguments A vector of all parameters passed to the built-in function. Order is same as in native GameMaker.
		 * \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if the function does not exist.
		 */
		virtual Aurie::AurieStatus CallBuiltinEx(
			OUT RValue& Result,
			IN const char* FunctionName,
			IN CInstance* SelfInstance,
			IN CInstance* OtherInstance,
			IN std::vector<RValue> Arguments
		) = 0;

		/**
		 * \brief Defines a callback function for intercepting game events.
		 * \param Module Specify g_ArSelfModule.
		 * \param Trigger The event to listen for.
		 * \param Routine A pointer to the callback function of the correct type.
		 * \param Priority Specifies the importance of the callback. The higher this number, the earlier the method is called in the chain. Default is 0.
		 * \return AURIE_SUCCESS on success. AURIE_OBJECT_ALREADY_EXISTS if the Routine is already registered.
		 */
		virtual Aurie::AurieStatus CreateCallback(
			IN Aurie::AurieModule* Module,
			IN EventTriggers Trigger,
			IN PVOID Routine,
			IN int32_t Priority
		) = 0;

		/**
		 * \brief Removes a callback registered with the CreateCallback function.
		 * \param Module Specify g_ArSelfModule.
		 * \param Routine A pointer to the callback function of the correct type that will be unregistered.
		 * \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if the Routine is not registered.
		 */
		virtual Aurie::AurieStatus RemoveCallback(
			IN Aurie::AurieModule* Module,
			IN PVOID Routine
		) = 0;

		/**
		 * \brief Accesses variables for a given instance / GameMaker struct.
		 * \param Instance An RValue of type VALUE_OBJECT.
		 * \param MemberName The name of the member variable to access.
		 * \param Member A buffer into which an engine-managed pointer to the variable is written.
		 * \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if the variable does not exist. AURIE_INVALID_PARAMETER if Instance is invalid.
		 */
		virtual Aurie::AurieStatus GetInstanceMember(
			IN RValue Instance,
			IN const char* MemberName,
			OUT RValue*& Member
		) = 0;

		/**
		 * \brief Enumerates the members for a given GameMaker struct / instance pointer.
		 * \param Instance An RValue of type VALUE_OBJECT.
		 * \param EnumFunction A callback function called for each top-level member of the struct.
		 * \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if EnumFunction returned false for all members.
		 */
		virtual Aurie::AurieStatus EnumInstanceMembers(
			IN RValue Instance,
			IN std::function<bool(IN const char* MemberName, RValue* Value)> EnumFunction
		) = 0;

		/**
		* \brief Deprecated. Use .ToString() on the RValue.
		*/
		virtual Aurie::AurieStatus RValueToString(
			IN const RValue& Value,
			OUT std::string& String
		) = 0;

		/**
		* \brief Deprecated. Construct the RValue directly.
		*/
		virtual Aurie::AurieStatus StringToRValue(
			IN const std::string_view String,
			OUT RValue& Value
		) = 0;

		/**
		* \brief Reserved for internal use.
		* \return An interface reserved for internal use.
		*/
		virtual const YYRunnerInterface& GetRunnerInterface() = 0;

		/**
		* \brief Invalidates function and variable lookup tables kept by YYTK.
		*/
		virtual void InvalidateAllCaches() = 0;

		/**
		* \brief Retrieves a pointer to the CScript object for a given script ID.
		* \param Index The index of an element within the runner's script array, the data of which to retrieve. To convert a script ID to an index, subtract 100 000 from it.
		* \param Script A buffer into which a pointer to the CScript object is written.
		* \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if index is out of bounds.
		*/
		virtual Aurie::AurieStatus GetScriptData(
			IN int Index,
			OUT CScript*& Script
		) = 0;

		/**
		* \brief Retrieves the index for an instance built-in or global built-in variable.
		* \param Name The name of the built-in to look up.
		* \param Index A reference to a buffer, into which the index to use with GetBuiltinVariableInformation is written.
		* \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if the variable does not exist.
		*/
		virtual Aurie::AurieStatus GetBuiltinVariableIndex(
			IN std::string_view Name,
			OUT size_t& Index
		) = 0;

		/**
		* \brief Retrieves the engine entry for a built-in variable.
		* \param Index The array index of the built-in variable. 
		* \param VariableInformation A read-only struct describing the built-in variable.
		* \return AURIE_SUCCESS on success. AURIE_INVALID_PARAMETER if the index is out of bounds.
		*/
		virtual Aurie::AurieStatus GetBuiltinVariableInformation(
			IN size_t Index,
			OUT RVariableRoutine*& VariableInformation
		) = 0;

		/**
		* \brief Retrieves the value of a local built-in or a global built-in variable.
		* \param Name The name of the built-in variable.
		* \param TargetInstance The instance from which to fetch the variable. See the wiki for more details.
		* \param ArrayIndex The index for array accesses. For non-array access, specify NULL_INDEX.
		* \param Value A buffer into which the value of the built-in is written.
		* \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if the variable does not exist.
		*/
		virtual Aurie::AurieStatus GetBuiltin(
			IN std::string_view Name,
			IN CInstance* TargetInstance,
			OPTIONAL IN int ArrayIndex,
			OUT RValue& Value
		) = 0;

		/**
		* \brief Retrieves the value of a local built-in or a global built-in variable.
		* \param Name The name of the built-in variable.
		* \param TargetInstance The instance from which to fetch the variable. See the wiki for more details.
		* \param ArrayIndex The index for array accesses. For non-array access, specify NULL_INDEX.
		* \param Value The value to write into the built-in variable.
		* \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if the variable does not exist.
		*/
		virtual Aurie::AurieStatus SetBuiltin(
			IN std::string_view Name,
			IN CInstance* TargetInstance,
			OPTIONAL IN int ArrayIndex,
			IN RValue& Value
		) = 0;

		/**
		* \brief Retrieves the value of a local built-in or a global built-in variable.
		* \param Value An RValue of type VALUE_ARRAY.
		* \param ArrayIndex The index of the element to retrieve from the array.
		* \param ArrayElement A buffer into which the pointer to the element is written.
		* \return AURIE_SUCCESS on success. AURIE_INVALID_PARAMETER if reading out of bounds, or if Value isn't an array.
		*/
		virtual Aurie::AurieStatus GetArrayEntry(
			IN RValue& Value,
			IN size_t ArrayIndex,
			OUT RValue*& ArrayElement
		) = 0;

		/**
		* \brief Retrieves the value of a local built-in or a global built-in variable.
		* \param Value An RValue of type VALUE_ARRAY.
		* \param Size A buffer into which the size of the array is written.
		* \return AURIE_SUCCESS on success. AURIE_INVALID_PARAMETER if Value isn't an array.
		*/
		virtual Aurie::AurieStatus GetArraySize(
			IN RValue& Value,
			OUT size_t& Size
		) = 0;

		/**
		* \brief Internal structure access required. See the wiki for more details.
		*/
		virtual Aurie::AurieStatus GetRoomData(
			IN int32_t RoomID,
			OUT CRoom*& Room
		) = 0;

		/**
		* \brief Internal structure access required. See the wiki for more details.
		*/
		virtual Aurie::AurieStatus GetCurrentRoomData(
			OUT CRoom*& CurrentRoom
		) = 0;

		/**
		* \brief Converts an instance ID into an instance object.
		* \param InstanceID The ID of the instance.
		* \param Instance A buffer into which a pointer to the instance object is written.
		* \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if InstanceID is not an active instance in the current room.
		*/
		virtual Aurie::AurieStatus GetInstanceObject(
			IN int32_t InstanceID,
			OUT CInstance*& Instance
		) = 0;

		/**
		* \brief Invokes a callback for each instance matching Object. See the wiki for more details.
		* \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if no instances of Object exist.
		*/
		virtual Aurie::AurieStatus InvokeWithObject(
			IN const RValue& Object,
			IN std::function<void(CInstance* Self, CInstance* Other)> Method
		) = 0;

		/**
		* \brief Retrieves a variable's hash from an object's hashmap.
		* \param Object Reserved for internal use.
		* \param VariableName The name of the variable whose name to convert into a slot ID.
		* \param Hash A buffer into which the slot ID is written.
		* \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if no instances of Object exist.
		*/
		virtual Aurie::AurieStatus GetVariableSlot(
			IN const RValue& Object,
			IN const char* VariableName,
			OUT int32_t& Hash
		) = 0;

		/**
		* \brief Gets the number of top-level variables in a struct or instance.
		* \param Object The instance whose variables are to be counted.
		* \param Count A buffer into which the total count is written.
		* \return AURIE_SUCCESS on success. AURIE_INVALID_PARAMETER if Object is an invalid type.
		*/
		virtual Aurie::AurieStatus GetInstanceMemberCount(
			IN RValue Object,
			OUT int32_t& Count
		) = 0;

		/**
		* \brief Calls a game script in the global context.
		* \param ScriptName The name of the script to call, prefixed with gml_Script. Case-sensitive.
		* \param Arguments The arguments to pass into the script.
		* \return An RValue representing the return value of the script.
		*/
		virtual RValue CallGameScript(
			IN std::string_view ScriptName,
			IN const std::vector<RValue>& Arguments
		) = 0;


		/**
		 * \brief Calls a script from within user-defined context.
		 * \param Result Buffer into which the result of the call is written.
		 * \param ScriptName The name of the script to call, prefixed with gml_Script. Case-sensitive.
		 * \param SelfInstance The instance object representing "self".
		 * \param OtherInstance The instance object representing "other".
		 * \param Arguments A vector of all parameters passed to the script. Order is same as in native GameMaker.
		 * \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if the function does not exist.
		 */
		virtual Aurie::AurieStatus CallGameScriptEx(
			OUT RValue& Result,
			IN std::string_view ScriptName,
			IN CInstance* SelfInstance,
			IN CInstance* OtherInstance,
			IN const std::vector<RValue>& Arguments
		) = 0;

		/**
		 * \brief Determines if an object is an instance of another.
		 * \param Instance An object instance.
		 * \param ObjectName The name of the object to compare Instance against.
		 * \return TRUE if Instance is an instance of ObjectName.
		 */
		virtual bool IsInstanceOfObject(
			IN const RValue& Instance,
			IN std::string_view ObjectName
		) = 0;

		/**
		 * \brief Reserved for internal use. Do not use.
		 * \param MethodName The name of the method whose argument count is extracted.
		 * \return AURIE_SUCCESS on success. AURIE_OBJECT_NOT_FOUND if the function does not exist.
		 */
		virtual Aurie::AurieStatus GetMethodParameterCount(
			IN std::string_view MethodName,
			OUT int32_t& Count
		) = 0;
	};

	struct YYTKPrivateInterface : public Aurie::AurieInterfaceBase
	{
		/* Aurie Boilerplate */

		virtual Aurie::AurieStatus Create() = 0;

		virtual void Destroy() = 0;

		virtual void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		) = 0;

		/* RValue conversions */

		virtual double RV_ToDouble(
			IN const RValue* Value
		) = 0;

		virtual int32_t RV_ToInt32(
			IN const RValue* Value
		) = 0;

		virtual int64_t RV_ToInt64(
			IN const RValue* Value
		) = 0;

		virtual PVOID RV_ToPointer(
			IN const RValue* Value
		) = 0;

		virtual bool RV_ToBoolean(
			IN const RValue* Value
		) = 0;

		virtual const char* RV_GetKindName(
			IN const RValue* Value
		) = 0;

		virtual const char* RV_GetObjectSpecificKind(
			IN const RValue* Value
		) = 0;

		virtual YYObjectBase* RV_ToObject(
			IN const RValue* Value
		) = 0;

		virtual CInstance* RV_ToInstance(
			IN const RValue* Value
		) = 0;

		virtual const char* RV_ToCString(
			IN const RValue* Value
		) = 0;

		virtual std::string RV_ToString(
			IN const RValue* Value
		) = 0;

		virtual std::u8string RV_ToU8String(
			IN const RValue* Value
		) = 0;

		virtual std::map<std::string, RValue> RV_ToMap(
			IN const RValue* Value
		) = 0;

		virtual std::map<std::string, RValue*> RV_ToRefMap(
			IN RValue* Value
		) = 0;

		virtual std::vector<RValue> RV_ToVector(
			IN const RValue* Value
		) = 0;

		virtual std::vector<RValue*> RV_ToRefVector(
			IN RValue* Value
		) = 0;

		virtual int32_t RV_GetMemberCount(
			IN const RValue* Value
		) = 0;

		virtual RValue* RV_ToCArray(
			IN RValue* Value
		) = 0;

		virtual RValue RV_IndexByNumber(
			IN const RValue* Value,
			IN size_t Index
		) = 0;

		virtual RValue* RV_IndexByNumberRef(
			IN RValue* Value,
			IN size_t Index
		) = 0;

		virtual RValue RV_IndexByName(
			IN const RValue* Value,
			IN std::string_view Index
		) = 0;

		virtual RValue* RV_IndexByNameRef(
			IN RValue* Value,
			IN std::string_view Index
		) = 0;

		virtual bool RV_ContainsNestedValue(
			IN const RValue* Value,
			IN std::string_view Index
		) = 0;

		virtual bool RV_IsUndefined(
			IN const RValue* Value
		) = 0;

		virtual bool RV_IsUnset(
			IN const RValue* Value
		) = 0;

		virtual bool RV_IsStruct(
			IN const RValue* Value
		) = 0;

		virtual bool RV_IsNumberCompatible(
			IN const RValue* Value
		) = 0;

		virtual bool RV_IsString(
			IN const RValue* Value
		) = 0;

		virtual bool RV_IsArray(
			IN const RValue* Value
		) = 0;

		/* RValue initializers */

		virtual void RV_CreateEmpty(
			IN RValue* Value
		) = 0;

		virtual void RV_CreateFromDouble(
			IN RValue* Value,
			IN double Contents
		) = 0;

		virtual void RV_CreateFromInteger(
			IN RValue* Value,
			IN int64_t Contents
		) = 0;

		virtual void RV_CreateFromPointer(
			IN RValue* Value,
			IN void* Contents
		) = 0;

		virtual void RV_CreateFromObjectPointer(
			IN RValue* Value,
			IN void* Contents
		) = 0;

		virtual void RV_CreateFromVector(
			IN RValue* Value,
			IN const std::vector<RValue>& Contents
		) = 0;

		virtual void RV_CreateFromAnsiString(
			IN RValue* Value,
			IN const std::string_view Contents
		) = 0;

		virtual void RV_CreateFromU8String(
			IN RValue* Value,
			IN const std::u8string_view Contents
		) = 0;

		virtual void RV_CreateFromBoolean(
			IN RValue* Value,
			IN bool Contents
		) = 0;

		virtual void RV_CreateFromMap(
			IN RValue* Value,
			IN const std::map<std::string, RValue>& Contents
		) = 0;

		virtual void RV_Copy(
			IN RValue* Destination,
			IN const RValue* Source
		) = 0;

		virtual void RV_Free(
			IN RValue* Value
		) = 0;

		virtual const char* CCode_GetName(
			IN const CCode* Object
		) = 0;

		virtual const char* CScript_GetName(
			IN const CScript* Object
		) = 0;

		virtual CRoomInternal* CRoom_GetInternalData(
			IN CRoom* Object
		) = 0;

		virtual bool YYObjectBase_Add(
			IN YYObjectBase* Object,
			IN const char* Name,
			IN const RValue& Value,
			IN int Flags
		) = 0;

		virtual RValue* YYObjectBase_FindOrAllocateValue(
			IN YYObjectBase* Object,
			IN const char* Name
		) = 0;

		virtual CInstanceInternal* CInstance_GetInternalData(
			IN CInstance* Instance
		) = 0;

		virtual CInstance* CInstance_FromID(
			IN int32_t InstanceID
		) = 0;

		virtual Aurie::AurieStatus YkSetRuntimeFlags(
			uint8_t NewFlags
		) = 0;
	};

	inline YYTKInterface* GetInterface()
	{
		using namespace Aurie;
		static YYTKInterface* module_interface = nullptr;

		// Try getting the interface
		// If we error, we return nullptr.
		if (!module_interface)
		{
			AurieStatus last_status = ObGetInterface(
				"YYTK_ZeusMain",
				reinterpret_cast<AurieInterfaceBase*&>(module_interface)
			);

			if (!AurieSuccess(last_status))
				printf("[%s : %d] FATAL: Failed to get YYTK Interface (%s)!\n", __FILE__, __LINE__, AurieStatusToString(last_status));
		}

		return module_interface;
	}

	inline YYTKPrivateInterface* GetPrivateInterface()
	{
		using namespace Aurie;
		static YYTKPrivateInterface* private_interface = nullptr;

		// Try getting the interface
		// If we error, we return nullptr.
		if (!private_interface)
		{
			AurieStatus last_status = ObGetInterface(
				"YYTK_ZeusPrivate",
				reinterpret_cast<AurieInterfaceBase*&>(private_interface)
			);

			if (!AurieSuccess(last_status))
				printf("[%s : %d] FATAL: Failed to get YYTK Interface (%s)!\n", __FILE__, __LINE__, AurieStatusToString(last_status));
		}

		return private_interface;
	}
}


