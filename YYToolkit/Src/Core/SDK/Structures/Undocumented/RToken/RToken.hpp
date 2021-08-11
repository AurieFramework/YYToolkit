#pragma once
#include "../../Documented/YYRValue/YYRValue.hpp"
#include "../../../Enums/Enums.hpp"

struct RToken
{
	int kind;
	eGMLKind type;
	int ind;
	int ind2;
	RValue value;
	int itemnumb;
	RToken* items;
	int position;
};