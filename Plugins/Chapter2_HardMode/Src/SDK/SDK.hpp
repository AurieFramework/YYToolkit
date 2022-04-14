#pragma once

// Forward declarations
#include "FwdDecls/FwdDecls.hpp"

// Enums
#include "Enums/Enums.hpp"

// Documented
#include "Structures/Documented/APIVars/APIVars.hpp"
#include "Structures/Documented/CCode/CCode.hpp"
#include "Structures/Documented/CDynamicArray/CDynamicArray.hpp"
#include "Structures/Documented/FunctionInfo/FunctionInfo.hpp"
#include "Structures/Documented/CHashMap/CHashMap.hpp"
#include "Structures/Documented/Math/Math.hpp"
#include "Structures/Documented/RefThing/RefThing.hpp"
#include "Structures/Documented/RToken/RToken.hpp"
#include "Structures/Documented/VMBuffer/VMBuffer.hpp"
#include "Structures/Documented/YYRValue/YYRValue.hpp"

// Undocumented - these may not work on all GMS versions (aka. may require plugin-specific additions / removals)

#include "Structures/Undocumented/CScript/CScript.hpp"
#include "Structures/Undocumented/VMExec/VMExec.hpp"
#include "Structures/Undocumented/YYGMLFuncs/YYGMLFuncs.hpp"
#include "Structures/Undocumented/YYObjectBase/YYObjectBase.hpp"
#include "Structures/Undocumented/YYVAR/YYVAR.hpp"

// Plugins.. duh
#include "Plugins/Plugins.hpp"
#include "Plugins/YYTKEvent/YYTKEvent.hpp"
// This only does shit if we're in a plugin
#include "Plugins/API Definitions/APIDefs.hpp" 