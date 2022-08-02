#pragma once
#include <Windows.h>
#include <string>

#define ACTION_WRITE_KERNEL 0x01
#define ACTION_READ_KERNEL 0x02
#define ACTION_REQBASE 0x03
#define ACTION_DRAWBOX 0x04
#define ACTION_DRAWTEXT 0x05

namespace KnP
{

	namespace memory
	{
		typedef struct _moduleInfo
		{
			ULONG64 base;
			ULONG size;
		} ModuleInfo;

		typedef struct _MEMORY
		{
			void* buffer_addr;
			UINT_PTR addr;
			ULONGLONG size;
			ULONG pid;
			ULONG64 action;
			ModuleInfo module_info;
			void* output;
			const char* module_name;
			char* text;
			int r, g, b, x, y, w, h, t;

		} MEMORY;

		struct HandleDisposer
		{
			using pointer = HANDLE;

			void operator()(HANDLE handle)const
			{
				if (handle != NULL && handle != INVALID_HANDLE_VALUE)
				{
					CloseHandle(handle);
				}
			}
		};

		ModuleInfo get_module_information(const char* module_name, ULONG pid);
		unsigned char* find_memory_pattern(unsigned char* src, size_t size, const char* pattern);
		unsigned char* find_memory_string(unsigned char* src, size_t size, const char* str, size_t strSize);
		uint32_t get_process_id(std::wstring process_name);
	}
}
