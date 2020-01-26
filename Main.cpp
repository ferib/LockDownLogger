#include "stdafx.h"
#include <cstdarg>

namespace Main
{
	using namespace std;

	uintptr_t ModuleBase;
	uintptr_t CriticalLogFunc;

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
		CriticalLogFunc = ModuleBase + 0x62760; //pattern: 
		std::cout << "CriticalLogFunc 0x" << std::hex << CriticalLogFunc << std::endl;

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

		if (ctx->Eip == CriticalLogFunc && (cer->ExceptionCode == STATUS_SINGLE_STEP || cer->ExceptionCode == STATUS_BREAKPOINT))
		{
			char* format = *(char**)(ctx->Esp + 0x04); //obtain string ptr from stack

			//prtinf source
			char* traverse;
			signed long i;
			char* s;

			int ArgUsed = 0;

			bool BlackList = true;
			char BadWord[] = "OnBeforeNavigationCanvas";	//blacklist this one, somehow it crashes here
			uintptr_t CharPtr = *(uintptr_t*)(ctx->Esp + 0x04);
			for (int i = 0; i < 10; i++)
			{
				//cout << *(char*)(CharPtr+i) << " != " << BadWord[i] << endl; //debugging it is
				if (*(char*)(CharPtr + i) != BadWord[i])
				{
					BlackList = false;
					break;
				}
			}

			for (traverse = format; *traverse != '\0'; traverse++)
			{
				if (BlackList)
					break;

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

					case 'o': i = *(int*)(ctx->Esp + 0x08 + (ArgUsed * 0x04));			//Fetch Octal representation
						ArgUsed++;
						cout << convert(i, 8);
						break;

					case 's': s = *(char**)(ctx->Esp + 0x08 + (ArgUsed * 0x04)); 		//Fetch string
						ArgUsed++;
						cout << s;
						break;

					case 'x': i = *(int*)(ctx->Esp + 0x08 + (ArgUsed * 0x04));			 //Fetch Hexadecimal representation
						ArgUsed++;
						cout << convert(i, 16);
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
		else {
			cout << "Unknown at 0x" << hex << ctx->Eip << " Esp: 0x" << ctx->Esp <<  endl;
			Sleep(5000000);
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

}