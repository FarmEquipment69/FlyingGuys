#pragma once
#include "definitions.h"

#define ACTION_WRITE_KERNEL 0x01
#define ACTION_READ_KERNEL 0x02
#define ACTION_REQBASE 0x03
#define ACTION_DRAWBOX 0x04
#define ACTION_DRAWTEXT 0x05
#define ACTION_READ 0x06
#define ACTION_WRITE 0x07

namespace memory
{
	typedef struct _moduleInfo
	{
		ULONG64 base;
		ULONG size;
	} ModuleInfo;

	typedef struct _OP
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

	} OP;

	PVOID get_system_module_base(const char* module_name);
	PVOID get_system_module_export(const char* module_name, LPCSTR routine_name);
	bool write_memory(void* address, void* buffer, size_t size);
	bool write_to_read_only_memory(void* address, void* buffer, size_t size);
	ModuleInfo get_module_base_x64(PEPROCESS proc, UNICODE_STRING module_name);
	bool read_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size);
	bool write_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size);
	PVOID get_system_module_export_w(LPCWSTR module_name, LPCSTR routine_name);
}

