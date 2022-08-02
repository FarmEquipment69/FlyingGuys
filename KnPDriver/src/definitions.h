#pragma once
#include <ntdef.h>
#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>
#include <ntstrsafe.h>
#include <wdm.h>
#pragma comment(lib, "ntoskrnl.lib")

typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation = 0x0B
} SYSTEM_INFORMATION_CLASS,
* PSYSTEM_INFORMATION_CLASS;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

typedef struct _PEB_LDR_DATA {
	ULONG Length;
	BOOLEAN Initialized;
	PVOID SsHandle;
	LIST_ENTRY ModuleListLoadOrder;
	LIST_ENTRY ModuleListMemoryOrder;
	LIST_ENTRY ModuleListInitOrder;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;  // in bytes
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;  // LDR_*
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	PVOID SectionPointer;
	ULONG CheckSum;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef unsigned short GLYPH;

typedef struct
{
	WORD    message[0x100][0x100];

	int     msgindex;

	int     counter;
	WORD    random_reg1;

	int     width, height;

} MATRIX_MESSAGE;

typedef struct
{
	BOOL    state;
	int     countdown;

	BOOL    started;
	int     runlen;

	int     blippos;
	int     bliplen;

	int     length;
	GLYPH* glyph;

} MATRIX_COLUMN;

typedef struct
{
	int             width;
	int             height;
	int             numcols;
	int             numrows;

	// bitmap containing glyphs.
	HDC             hdcBitmap;
	HBITMAP         hbmBitmap;

	MATRIX_MESSAGE* message;

	MATRIX_COLUMN   column[1];

} MATRIX;

typedef FLOAT FLOATOBJ, * PFLOATOBJ;


typedef struct _RGN_ATTR
{
	ULONG AttrFlags;
	ULONG iComplexity;     /* Clipping region's complexity. NULL, SIMPLE & COMPLEXREGION */
	RECTL Rect;
} RGN_ATTR, * PRGN_ATTR;

typedef struct _DC_ATTR
{
	PVOID pvLDC;
	ULONG ulDirty_;
	HANDLE hbrush;
	HANDLE hpen;
	COLORREF crBackgroundClr;
	ULONG ulBackgroundClr;
	COLORREF crForegroundClr;
	ULONG ulForegroundClr;
	COLORREF crBrushClr;
	ULONG ulBrushClr;
	COLORREF crPenClr;
	ULONG ulPenClr;
	DWORD iCS_CP;
	INT iGraphicsMode;
	BYTE jROP2;
	BYTE jBkMode;
	BYTE jFillMode;
	BYTE jStretchBltMode;
	POINTL ptlCurrent;
	POINTL ptfxCurrent;
	LONG lBkMode;
	LONG lFillMode;
	LONG lStretchBltMode;
	FLONG flFontMapper;
	LONG lIcmMode;
	HANDLE hcmXform;
	HCOLORSPACE hColorSpace;
	FLONG flIcmFlags;
	INT IcmBrushColor;
	INT IcmPenColor;
	PVOID pvLIcm;
	FLONG flTextAlign;
	LONG lTextAlign;
	LONG lTextExtra;
	LONG lRelAbs;
	LONG lBreakExtra;
	LONG cBreak;
	HANDLE hlfntNew;
	MATRIX mxWorldToDevice;
	MATRIX mxDeviceToWorld;
	MATRIX mxWorldToPage;
	FLOATOBJ efM11PtoD;
	FLOATOBJ efM22PtoD;
	FLOATOBJ efDxPtoD;
	FLOATOBJ efDyPtoD;
	INT iMapMode;
	DWORD dwLayout;
	LONG lWindowOrgx;
	POINTL ptlWindowOrg;
	SIZEL szlWindowExt;
	POINTL ptlViewportOrg;
	SIZEL szlViewportExt;
	FLONG flXform;
	SIZEL szlVirtualDevicePixel;
	SIZEL szlVirtualDeviceMm;
	SIZEL szlVirtualDeviceSize;
	POINTL ptlBrushOrigin;
	RGN_ATTR VisRectRegion;
} DC_ATTR, * PDC_ATTR;


typedef struct _ellipsis_data_
{
	int before;
	int len;
	int under;
	int after;
} ellipsis_data, pelipsis_data;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
	BYTE Reserved1[16];
	PVOID Reserved2[10];
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef void(__stdcall* PPS_POST_PROCESS_INIT_ROUTINE)(void); // not exported

typedef struct _PEB {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[1];
	PVOID Reserved3[2];
	PPEB_LDR_DATA Ldr;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	PVOID Reserved4[3];
	PVOID AtlThunkSListPtr;
	PVOID Reserved5;
	ULONG Reserved6;
	PVOID Reserved7;
	ULONG Reserved8;
	ULONG AtlThunkSListPtr32;
	PVOID Reserved9[45];
	BYTE Reserved10[96];
	PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
	BYTE Reserved11[128];
	PVOID Reserved12[1];
	ULONG SessionId;
} PEB, * PPEB;

extern "C" __declspec(dllimport)
NTSTATUS NTAPI ZwProtectVirtualMemory(
	HANDLE ProcessHandle,
	PVOID * BaseAddress,
	PULONG ProtectSize,
	ULONG NewProtect,
	PULONG OldProtect
);

extern "C" NTKERNELAPI
PVOID
NTAPI
RtlFindExportedRoutineByName(
	_In_ PVOID ImageBase,
	_In_ PCCH RoutineNam
);

extern "C" NTSTATUS ZwQuerySystemInformation(ULONG InfoClass, PVOID Buffer, ULONG Length, PULONG ReturnLength);

extern "C" NTKERNELAPI
PPEB
PsGetProcessPeb(
	IN PEPROCESS Process
);

typedef struct {
	UINT cbSize;
	int iTabLength;
	int iLeftMargin;
	int iRightMargin;
	UINT uiLengthDrawn;
} DRAWTEXTPARAMS, * LPDRAWTEXTPARAMS;

extern "C" NTSTATUS NTAPI MmCopyVirtualMemory
(
	PEPROCESS SourceProcess,
	PVOID SourceAddress,
	PEPROCESS TargetProcess,
	PVOID TargetAddress,
	SIZE_T BufferSize,
	KPROCESSOR_MODE PreviousMode,
	PSIZE_T ReturnSize
);

typedef struct _IMAGE_EXPORT_DIRECTORY {
	ULONG   Characteristics;
	ULONG   TimeDateStamp;
	USHORT  MajorVersion;
	USHORT  MinorVersion;
	ULONG   Name;
	ULONG   Base;
	ULONG   NumberOfFunctions;
	ULONG   NumberOfNames;
	ULONG   AddressOfFunctions;     // RVA from base of image
	ULONG   AddressOfNames;         // RVA from base of image
	ULONG   AddressOfNameOrdinals;  // RVA from base of image
} IMAGE_EXPORT_DIRECTORY, * PIMAGE_EXPORT_DIRECTORY;

extern "C" __declspec(dllimport)
PVOID
NTAPI
RtlImageDirectoryEntryToData(
	PVOID ImageBase,
	BOOLEAN MappedAsImage,
	USHORT DirectoryEntry,
	PULONG Size
);