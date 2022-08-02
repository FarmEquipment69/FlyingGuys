#pragma once
#include <Windows.h>
#include <memory>
#include <tlhelp32.h>
#include <iostream>
#include "memory.h"

namespace KnP
{
	namespace hook {
		
		extern char lib[255];
		extern char module_name[255];

		template<typename ... Args>
		inline uint64_t call_hook(const Args ... args)
		{
			LoadLibraryA("user32.dll");
			void* hooked_func = GetProcAddress(LoadLibraryA(lib), module_name);

			const auto func = static_cast<uint64_t(__stdcall*)(Args...)>(hooked_func);

			return func(args ...);
		}

		namespace wingdi
		{
			int draw_box(int x, int y, int w, int h, int t, int r, int g, int b);
			int draw_text(const char* str, int x, int y, int w, int h);
		}

		namespace memory
		{

			std::string read_memory_string(UINT_PTR readAddr, ULONG pid);
			std::wstring read_memory_wstring(UINT_PTR readAddr, ULONG pid);
			void read_memory(UINT_PTR readAddr, void* output, size_t size, ULONG pid);
			void write_memory(UINT_PTR writeAddr, void* input, size_t size, ULONG pid);
			UINT_PTR scan_signature(UINT_PTR readAddr, ULONG size, const char* mask, ULONG pid);
			KnP::memory::ModuleInfo get_module_information(const char* module_name, ULONG pid);

			template<class T>
			inline T read_memory(UINT_PTR readAddr, ULONG pid)
			{
				T result;

				KnP::memory::MEMORY instructions;
				instructions.pid = pid;
				instructions.size = sizeof(T);

				instructions.addr = readAddr;
				instructions.action = ACTION_READ_KERNEL;
				instructions.output = &result;
				KnP::hook::call_hook(&instructions);

				return result;
			}


			template<typename T>
			inline void write_memory(UINT_PTR writeAddr, const T& obj, ULONG pid)
			{
				void* buffer = malloc(sizeof(T));
				memcpy(buffer, &obj, sizeof(T));

				KnP::memory::MEMORY instructions;
				instructions.pid = pid;
				instructions.action = ACTION_WRITE_KERNEL;

				instructions.addr = writeAddr;
				instructions.buffer_addr = buffer;
				instructions.size = sizeof(T);
				KnP::hook::call_hook(&instructions);

				free(buffer);
			}
		}

	}
}