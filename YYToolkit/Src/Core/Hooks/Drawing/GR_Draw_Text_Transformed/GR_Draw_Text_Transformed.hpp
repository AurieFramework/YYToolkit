#pragma once

namespace Hooks::GR_Draw_Text_Transformed
{
	void Function(float x, float y, const char* str, int linesep, int linewidth, float xsc, float ysc, float angle);
	void* GetTargetAddress();

	inline decltype(&Function) pfnOriginal;
}