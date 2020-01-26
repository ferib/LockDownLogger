#include "stdafx.h"
#include <cstdarg>

namespace Main
{
	using namespace std;

	uintptr_t ModuleBase;
	uintptr_t CriticalLogFunc;
	uintptr_t SomeOtherLogFunc;

	DWORD WINAPI Init(void* const(param))
	{
		AddVectoredExceptionHandler(1, vehHandler);
		AllocConsole();
		SetConsoleTitle(L"LockDownBrowser - Log");
		FILE* stream;
		freopen_s(&stream, "CONOUT$", "w", stdout);
		ShowWindow(GetConsoleWindow(), SW_SHOW);

		cout << "      __            _        ___                      __                              " << endl;
		cout << "     // //  ___   ___| | __   //   \_____      ___ __   // //  ___   __ _  __ _  ___ _ __  " << endl;
		cout << "    // //  // _ \ // __| |// //  // //\ // _ \ \ //\ // // '_ \ // //  // _ \ // _` |// _` |// _ \ '__| " << endl;
		cout << "   // //__| (_) | (__|   <  // //_//// (_) \ V  V //| | | // //__| (_) | (_| | (_| |  __// |    " << endl;
		cout << "   \____//\___// \___|_|\_\//___,' \___// \_//\_// |_| |_\____//\___// \__, |\__, |\___|_|    " << endl;
		cout << "                                                               |___// |___//            " << endl << endl;

		ModuleBase = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
		CriticalLogFunc = ModuleBase + 0x62760; //pattern: 55 8B EC 83 E4 F8 B8 1C 40 00 00 E8 ?? ?? ?? ?? A1 ?? ?? ?? ??
		SomeOtherLogFunc = ModuleBase + 0x62840; //pattern: 
		std::cout << "CriticalLogFunc 0x" << std::hex << CriticalLogFunc << std::endl;
		std::cout << "SomeOtherLogFunc 0x" << std::hex << SomeOtherLogFunc << std::endl;

		InjectHook();

		while(!GetAsyncKeyState(VK_F4) & 1)
		{
			Sleep(250);	
		}
		cout << "Unloading..." << endl;
		FreeLibraryAndExitThread(static_cast<HMODULE>(param), NULL);
	}


	void WINAPI InjectHook()
	{
		DWORD dwOld;
		cout << "InjectHook();" << endl;

		if (CriticalLogFunc != NULL)
		{
			VirtualProtect((LPVOID)(CriticalLogFunc + 1), 1, PAGE_EXECUTE_READWRITE, &dwOld);
			*(BYTE*)(CriticalLogFunc) = 0xCC;
			VirtualProtect((LPVOID)(CriticalLogFunc + 1), 1, dwOld, &dwOld);
			cout << "placing int3 at 0x" << hex << CriticalLogFunc << endl;
		}
		if (SomeOtherLogFunc != NULL)
		{
			/*
			VirtualProtect((LPVOID)(SomeOtherLogFunc + 1), 1, PAGE_EXECUTE_READWRITE, &dwOld);
			*(BYTE*)(SomeOtherLogFunc) = 0xCC;
			VirtualProtect((LPVOID)(SomeOtherLogFunc + 1), 1, dwOld, &dwOld);
			cout << "placing int3 at 0x" << hex << SomeOtherLogFunc << endl;
			*/
		}

		return;
	}

	char* convert(unsigned int num, int base)
	{
		static char Representation[] = "0123456789ABCDEF";
		static char buffer[50];
		char* ptr;

		ptr = &buffer[49];
		*ptr = '\0';

		do
		{
			*--ptr = Representation[num % base];
			num /= base;
		} while (num != 0);

		return(ptr);
	}

	LONG WINAPI vehHandler(_EXCEPTION_POINTERS *exceptionInfo)
	{
		auto ctx = exceptionInfo->ContextRecord;
		auto cer = exceptionInfo->ExceptionRecord;

		if (ctx->Eip == CriticalLogFunc || ctx->Eip == SomeOtherLogFunc && (cer->ExceptionCode == STATUS_SINGLE_STEP || cer->ExceptionCode == STATUS_BREAKPOINT))
		{
			char* format = *(char**)(ctx->Esp + 0x04); //obtain string ptr from stack
			//cout << ">" << format << endl;

			//prtinf source
			char* traverse;
			signed int i; //unsugned int i;
			char* s;

			int ArgUsed = 0;

			for (traverse = format; *traverse != '\0'; traverse++)
			{
				while (*traverse != '%')
				{
					cout << *traverse;
					traverse++;
				}

				traverse++;

				//Module 2: Fetching and executing arguments
				switch (*traverse)
				{
					case 'c': i = (int)(ctx->Esp + 0x08 + (ArgUsed * 0x04));			//Fetch char argument
						ArgUsed++;
						cout << i;
						break;

					case 'd': i = *(int*)(ctx->Esp + 0x08 + (ArgUsed * 0x04)); 			//Fetch Decimal/Integer argument
						ArgUsed++;
						if (i < 0)
						{
							i = -i;
							cout << '-';
						}
						cout << convert(i, 10);
						break;
					case 'u': i = *(unsigned int*)(ctx->Esp + 0x08 + (ArgUsed * 0x04)); 			//Fetch Decimal/Integer argument
						ArgUsed++;
						cout << i;
						break;

					case 'o': i = *(int*)(ctx->Esp + 0x08 + (ArgUsed * 0x04));			//Fetch Octal representation
						ArgUsed++;
						cout << convert(i, 8);
						break;

					case 's': s = *(char**)(ctx->Esp + 0x08 + (ArgUsed * 0x04)); 		//Fetch string, its fucked when there s a wrong ptr given due to a unkown % value
						ArgUsed++;
						if (s != NULL && s > 0x00)
							cout << s;
						else
							cout << "%s";
						break;

					case 'x': i = *(int*)(ctx->Esp + 0x08 + (ArgUsed * 0x04));			 //Fetch Hexadecimal representation
						ArgUsed++;
						cout << convert(i, 16);
						break;
					case 'I': i = *(int*)(ctx->Esp + 0x08 + (ArgUsed * 0x04));			 //%I64d hmmm?
						ArgUsed++;
						cout << convert(i, 10);
						break;
				}
			}
			cout << endl;

			//restore: push ebp
			ctx->Esp -= 4;
			*(DWORD*)ctx->Esp = ctx->Ebp;
			ctx->Eip += 1;

			return EXCEPTION_CONTINUE_EXECUTION;
		}
		else if (ctx->Eip == SomeOtherLogFunc || ctx->Eip == SomeOtherLogFunc && (cer->ExceptionCode == STATUS_SINGLE_STEP || cer->ExceptionCode == STATUS_BREAKPOINT))
		{
			cout << *(wchar_t**)(ctx->Esp + 0x04);
			//restore: push ebp
			ctx->Esp -= 4;
			*(DWORD*)ctx->Esp = ctx->Ebp;
			ctx->Eip += 1;

			return EXCEPTION_CONTINUE_EXECUTION;
		}
		else {
			cout << "Unknown at 0x" << hex << ctx->Eip << " Esp: 0x" << ctx->Esp <<  endl;
			Sleep(5000000);
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

}