#pragma once
#include "definitions.h"
#include <wingdi.h>

namespace exported
{
	namespace wingdi {
		typedef HBRUSH(*GdiSelectBrush_t)(_In_ HDC 	hdc, _In_ HBRUSH hbr);
		typedef BOOL(*PatBlt_t)(_In_ HDC, _In_ int x, _In_ int y, _In_ int w, _In_ int h, _In_ DWORD);
		typedef HDC(*NtUserGetDC_t)(HWND hWnd);
		typedef HBRUSH(*NtGdiCreateSolidBrush_t)(_In_ COLORREF 	crColor, _In_opt_ HBRUSH hbr);
		typedef int (*ReleaseDC_t)(HDC hdc);
		typedef BOOL(*DeleteObjectApp_t)(HANDLE hobj);
		typedef BOOL (*NtGdiExtTextOutW_t)(IN HDC 	hDC,
			IN INT 	XStart,
			IN INT 	YStart,
			IN UINT 	fuOptions,
			IN OPTIONAL LPRECT 	UnsafeRect,
			IN LPWSTR 	UnsafeString,
			IN INT 	Count,
			IN OPTIONAL LPINT 	UnsafeDx,
			IN DWORD 	dwCodePage
		);

		// Reac OS
		//  GdiSelectFont
		// Win32kFull.sys -> NtGdiExtTextOutW 
		//NtGdiExtTextOutW -> ExtTextOutW -> TextOutW -> TextOutA

		/*
			SetTextAlign(GDI_drawing::HDC_Desktop, TA_CENTER | TA_NOUPDATECP);
			SetBkColor(GDI_drawing::HDC_Desktop, RGB(0, 0, 0));
			SetBkMode(GDI_drawing::HDC_Desktop, TRANSPARENT);
			SetTextColor(GDI_drawing::HDC_Desktop, color);
			SelectObject(GDI_drawing::HDC_Desktop, GDI_drawing::Font);
			TextOutA(GDI_drawing::HDC_Desktop, x, y, text, strlen(text));
			DeleteObject(Font);
		*/

		inline GdiSelectBrush_t GdiSelectBrush = NULL;
		inline PatBlt_t NtGdiPatBlt = NULL;
		inline NtUserGetDC_t NtUserGetDC = NULL;
		inline NtGdiCreateSolidBrush_t NtGdiCreateSolidBrush = NULL;
		inline ReleaseDC_t NtUserReleaseDC = NULL;
		inline DeleteObjectApp_t NtGdiDeleteObjectApp = NULL;
		inline NtGdiExtTextOutW_t NtGdiExtTextOutW = NULL;

		INT FrameRect(HDC hDC, CONST RECT* lprc, HBRUSH hbr, int thickness);
		INT DrawTextW(HDC hdc, LPWSTR str, INT count, LPRECT rect, UINT flags);


		BOOL ExtTextOutW(_In_ HDC 	hdc,
			_In_ INT 	x,
			_In_ INT 	y,
			_In_ UINT 	fuOptions,
			_In_opt_ const RECT* lprc,
			_In_reads_opt_(cwc) LPCWSTR 	lpString,
			_In_ UINT 	cwc,
			_In_reads_opt_(cwc) const INT* lpDx
		);



		void Register();
	}


	void Register();


}


