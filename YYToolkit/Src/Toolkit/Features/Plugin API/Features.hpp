#pragma once
#ifdef __cplusplus
#pragma warning(disable : 26812)
#define DllExport extern "C" __declspec(dllexport)
#else
#define DllExport __declspec(dllexport)
#endif

// Enable / Disable the toggling of the internal UI.
DllExport void SetNewUIState(bool State);

