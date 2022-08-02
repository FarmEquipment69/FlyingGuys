#include "hook.h"
#include "exported.h"
#include <wingdi.h>

constexpr unsigned char injectedByteCode[64] = {
		0x02, 0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x06, 0x90, 0x90, 0x90, 0x90, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
constexpr char injectedModuleLoc[512] = "\\SystemRoot\\System32\\drivers\\dxgkrnl.sys";
constexpr char injectedModuleName[255] = "NtDxgkGetTrackedWorkloadStatistics";

bool hook::call_kernel_function(void* kernel_function_address)
{
	if (!kernel_function_address)
		return false;

	PVOID* function = reinterpret_cast<PVOID*>(memory::get_system_module_export(injectedModuleLoc, injectedModuleName));
	if (!function)
	{
		return false;
	}
	

	if (injectedByteCode[0] != 0x00 && injectedByteCode[32] != 0x00)
	{
		// TODO: this should be 72
		BYTE result[64];
		// 48 b8 ff ff ff ff ff ff ff ff 90 90 90 90 ff e0
		// 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
		// 2 + 8 = 10 + 6 = 16
		unsigned char s1 = injectedByteCode[0];
		unsigned char s2 = injectedByteCode[32];
		RtlSecureZeroMemory(result, sizeof(result));
		memcpy(&result[0], &injectedByteCode[1], s1);
		uintptr_t hook_address = reinterpret_cast<uintptr_t>(kernel_function_address);
		memcpy(&result[s1], &hook_address, sizeof(uintptr_t));
		memcpy(&result[s1 + sizeof(uintptr_t)], &injectedByteCode[33], s2);
		
		memory::write_to_read_only_memory(function, result, (((size_t)s1 + s2) + sizeof(uintptr_t)));
		exported::Register();
		return true;
	}

	return false;
/*
	BYTE orig[15];

	BYTE shell_code[] = { 0x48, 0xB8 }; // mov eax, xxx
	BYTE shell_mid[] = { 0x90, 0x90, 0x90 }; // nop, nop
	BYTE shell_code_end[] = { 0xFF, 0xE0 }; //jmp eax

	RtlSecureZeroMemory(orig, sizeof(orig));
	memcpy((PVOID)((ULONG_PTR)orig), shell_code, sizeof(shell_code));
	uintptr_t hook_address = reinterpret_cast<uintptr_t>(kernel_function_address);
	memcpy((PVOID)((ULONG_PTR)orig + sizeof(shell_code)), &hook_address, sizeof(uintptr_t));
	memcpy((PVOID)((ULONG_PTR)orig + sizeof(shell_code) + sizeof(uintptr_t)), shell_mid, sizeof(shell_mid));
	memcpy((PVOID)((ULONG_PTR)orig + sizeof(shell_code) + sizeof(uintptr_t) + sizeof(shell_mid)), shell_code_end, sizeof(shell_code_end));

	memory::write_to_read_only_memory(function, orig, sizeof(orig));
	exported::Register();

	return true;
	*/
}

NTSTATUS hook::hook_handler(PVOID called_param)
{
	memory::OP* instructions = (memory::OP*)called_param;

	if (instructions->action == ACTION_REQBASE)
	{
		ANSI_STRING AS;
		UNICODE_STRING ModuleName;

		RtlInitAnsiString(&AS, instructions->module_name);
		RtlAnsiStringToUnicodeString(&ModuleName, &AS, TRUE);
		PEPROCESS process;
		PsLookupProcessByProcessId((HANDLE)instructions->pid, &process);
		memory::ModuleInfo moduleInfo = { NULL };
		moduleInfo = memory::get_module_base_x64(process, ModuleName);
		instructions->module_info = moduleInfo;
		RtlFreeUnicodeString(&ModuleName);
		return STATUS_SUCCESS;
	}
	
	if (instructions->action == ACTION_WRITE_KERNEL)
	{
		if (instructions->addr < 0x7FFFFFFFFFFF && instructions->addr > 0)
		{
			PVOID kernelBuff = ExAllocatePool(NonPagedPool, instructions->size);

			if (!kernelBuff)
			{
				return STATUS_UNSUCCESSFUL;
			}

			if (!memcpy(kernelBuff, instructions->buffer_addr, instructions->size))
			{
				return STATUS_UNSUCCESSFUL;
			}
			PEPROCESS process;
			PsLookupProcessByProcessId((HANDLE)instructions->pid, &process);
			memory::write_kernel_memory((HANDLE)instructions->pid, instructions->addr, kernelBuff, instructions->size);
			ExFreePool(kernelBuff);
		}

		return STATUS_SUCCESS;
	}

	if (instructions->action == ACTION_READ_KERNEL)
	{
		if (instructions->addr < 0x7FFFFFFFFFFF && instructions->addr > 0)
		{
			memory::read_kernel_memory((HANDLE)instructions->pid, instructions->addr, instructions->output, instructions->size);
			return STATUS_SUCCESS;
		}
		return STATUS_UNSUCCESSFUL;
	}
	
	if (instructions->action == ACTION_DRAWBOX)
	{
		HDC hdc = exported::wingdi::NtUserGetDC(NULL);
		if (!hdc)
			return STATUS_UNSUCCESSFUL;

		HBRUSH brush = exported::wingdi::NtGdiCreateSolidBrush(RGB(instructions->r, instructions->g, instructions->b), NULL);
		if (!brush)
			return STATUS_UNSUCCESSFUL;

		RECT rect = { instructions->x, instructions->y, instructions->x + instructions->w, instructions->y + instructions->h };
		exported::wingdi::FrameRect(hdc, &rect, brush, instructions->t);
		exported::wingdi::NtUserReleaseDC(hdc);
		exported::wingdi::NtGdiDeleteObjectApp(brush);

		return STATUS_SUCCESS;
	}
	
	if (instructions->action == ACTION_DRAWTEXT)
	{
		HDC hdc = exported::wingdi::NtUserGetDC(NULL);

		if (!hdc)
		{
			return STATUS_UNSUCCESSFUL;
		}
			
		RECT rect = { instructions->x, instructions->y, instructions->x + instructions->w, instructions->y + instructions->h };

		int res = exported::wingdi::DrawTextW(hdc, L"TEST123", 7, &rect, 0x00);
		//DbgPrint("[KnP] Drawing text %d %d\n", res);
		*(int*)instructions->output = res;

		return STATUS_SUCCESS;
	}

	return STATUS_SUCCESS;
}