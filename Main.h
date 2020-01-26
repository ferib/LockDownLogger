#pragma once
#include "stdafx.h"


namespace Main
{
	void WINAPI InjectHook();
	bool Hook_SendTo(INT64, uintptr_t, INT64);
	LONG WINAPI vehHandler(_EXCEPTION_POINTERS* exceptionInfo);
	DWORD WINAPI Init(void* const(param));
	char* convert(unsigned int num, int base);
}