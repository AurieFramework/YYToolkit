#pragma once
#include "../YYRValue/YYRValue.hpp"

struct RToken
{
	int kind;
	unsigned int type;
	int ind;
	int ind2;
	RValue value;
	int itemnumb;
	RToken* items;
	int position;
};