#include "exported.h"
#include "memory.h"
#include <wingdi.h>

namespace exported
{
	void Register()
	{
		wingdi::Register();
	}

    namespace wingdi
    {
        void Register()
        {
            exported::wingdi::GdiSelectBrush = (exported::wingdi::GdiSelectBrush_t)memory::get_system_module_export_w(L"win32kfull.sys", "NtGdiSelectBrush");
            exported::wingdi::NtGdiExtTextOutW = (exported::wingdi::NtGdiExtTextOutW_t)memory::get_system_module_export_w(L"win32kfull.sys", "NtGdiExtTextOutW");
            exported::wingdi::NtGdiCreateSolidBrush = (exported::wingdi::NtGdiCreateSolidBrush_t)memory::get_system_module_export_w(L"win32kfull.sys", "NtGdiCreateSolidBrush");
            exported::wingdi::NtGdiPatBlt = (exported::wingdi::PatBlt_t)memory::get_system_module_export_w(L"win32kfull.sys", "NtGdiPatBlt");
            exported::wingdi::NtUserGetDC = (exported::wingdi::NtUserGetDC_t)memory::get_system_module_export_w(L"win32kbase.sys", "NtUserGetDC");
            exported::wingdi::NtUserReleaseDC = (exported::wingdi::ReleaseDC_t)memory::get_system_module_export_w(L"win32kbase.sys", "NtUserReleaseDC");
            exported::wingdi::NtGdiDeleteObjectApp = (exported::wingdi::DeleteObjectApp_t)memory::get_system_module_export_w(L"win32kbase.sys", "NtGdiDeleteObjectApp");
        }

        INT FrameRect(HDC hDC, CONST RECT* lprc, HBRUSH hbr, int thickness)
        {

            RECT r = *lprc;
            HBRUSH oldbrush = GdiSelectBrush(hDC, hbr);

            if (!oldbrush) return 0;

            NtGdiPatBlt(hDC, r.left, r.top, thickness, r.bottom - r.top, PATCOPY);
            NtGdiPatBlt(hDC, r.right - thickness, r.top, thickness, r.bottom - r.top, PATCOPY);
            NtGdiPatBlt(hDC, r.left, r.top, r.right - r.left, thickness, PATCOPY);
            NtGdiPatBlt(hDC, r.left, r.bottom - thickness, r.right - r.left, thickness, PATCOPY);

            GdiSelectBrush(hDC, oldbrush);
            return TRUE;
        }

        INT DrawTextW(HDC hdc, LPWSTR str, INT count, LPRECT rect, UINT flags)
        {
            DRAWTEXTPARAMS dtp;
            memset(&dtp, 0, sizeof(dtp));
            dtp.cbSize = sizeof(dtp);
            if (flags & 0x00000080) //DT_TABSTOP
            {
                dtp.iTabLength = (flags >> 8) & 0xff;
                flags &= 0xffff00ff;
            }

            INT Dx[10] = { 10, -5, 10, 5, 10, -10, 10, 5, 10, 5 };
           // NtGdiExtTextOutW(hdc, 0, 0, 0, rect, str, count, Dx, 0);
            //DrawTextExWorker
           
            /*
            
			HFONT hf = CreateFont(20, 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
            SelectObject(hdc, hf);
           / bool b = ExtTextOutW(hdc, 50, 50, 0, rect, L"Hello World!", 12, 0);
            DeleteObject(hf);
            */
            return -1;
            //return NtGdiDrawTextExWorker(hdc, (LPWSTR)str, count, rect, flags, &dtp);
        }


		BOOL ExtTextOutW(_In_ HDC hdc, _In_ INT x, _In_ INT y, _In_ UINT fuOptions, _In_opt_ const RECT* lprc, _In_reads_opt_(cwc) LPCWSTR lpString, _In_ UINT cwc, _In_reads_opt_(cwc) const INT* lpDx)
		{/*
			PDC_ATTR pdcattr;

			// Need both, should return a parameter error? No they don't!
			if (!lpDx && fuOptions & ETO_PDY)
				return FALSE;

			// Now sorting out rectangle.

			// Here again, need both.
			if (lprc && !(fuOptions & (ETO_CLIPPED | ETO_OPAQUE)))
			{
				lprc = NULL; // No flags, no rectangle.
			}
			else if (!lprc) // No rectangle, force clear flags if set and continue.
			{
				fuOptions &= ~(ETO_CLIPPED | ETO_OPAQUE);
			}

			

			if (GdiConvertAndCheckDC(hdc) == NULL) return FALSE;

			if (!(fuOptions & (ETO_GLYPH_INDEX | ETO_IGNORELANGUAGE)))
			{
				bBypassETOWMF = TRUE;

				if (LoadLPK(LPK_ETO))
					return LpkExtTextOut(hdc, x, y, fuOptions, lprc, lpString, cwc, lpDx, 0);
			}
			else
			{
				bBypassETOWMF = FALSE;
			}

			pdcattr = GdiGetDcAttr(hdc);
			if (pdcattr &&
				!(pdcattr->ulDirty_ & DC_DIBSECTION) &&
				!(pdcattr->lTextAlign & TA_UPDATECP))
			{
				if (lprc && !cwc)
				{
					if (fuOptions & ETO_OPAQUE)
					{
						PGDIBSEXTTEXTOUT pgO;

						pgO = GdiAllocBatchCommand(hdc, GdiBCExtTextOut);
						if (pgO)
						{
							pdcattr->ulDirty_ |= DC_MODE_DIRTY;
							pgO->Count = cwc;
							pgO->Rect = *lprc;
							pgO->Options = fuOptions;
							
							pgO->ulBackgroundClr = pdcattr->ulBackgroundClr;
							pgO->ptlViewportOrg = pdcattr->ptlViewportOrg;
							return TRUE;
						}
					}
					else // Do nothing, old explorer pops this off.
					{
						DPRINT1("GdiBCExtTextOut nothing\n");
						return TRUE;
					}
				}         // Max 580 wchars, if offset 0
				else if (cwc <= ((GDIBATCHBUFSIZE - sizeof(GDIBSTEXTOUT)) / sizeof(WCHAR)))
				{
					PGDIBSTEXTOUT pgO;
					PTEB pTeb = NtCurrentTeb();

					pgO = GdiAllocBatchCommand(hdc, GdiBCTextOut);
					if (pgO)
					{
						USHORT cjSize = 0;
						ULONG DxSize = 0;

						if (cwc > 2) cjSize = (cwc * sizeof(WCHAR)) - sizeof(pgO->String);

						
						if (lpDx)
						{
							
							DxSize = (cwc * sizeof(INT)) * (fuOptions & ETO_PDY ? 2 : 1);
							cjSize += DxSize;
							// The structure buffer holds 4 bytes. Store Dx data then string.
							// Result one wchar -> Buf[ Dx ]Str[wC], [4][2][X] one extra unused wchar
							// to assure alignment of 4.
						}

						if ((pTeb->GdiTebBatch.Offset + cjSize) <= GDIBATCHBUFSIZE)
						{
							pdcattr->ulDirty_ |= DC_MODE_DIRTY | DC_FONTTEXT_DIRTY;
							pgO->cbCount = cwc;
							pgO->x = x;
							pgO->y = y;
							pgO->Options = fuOptions;
							pgO->iCS_CP = 0;

							if (lprc) pgO->Rect = *lprc;
							else
							{
								pgO->Options |= GDIBS_NORECT; // Tell the other side lprc is nill.
							}

							
							pgO->crForegroundClr = pdcattr->crForegroundClr;
							pgO->crBackgroundClr = pdcattr->crBackgroundClr;
							pgO->ulForegroundClr = pdcattr->ulForegroundClr;
							pgO->ulBackgroundClr = pdcattr->ulBackgroundClr;
							pgO->lBkMode = pdcattr->lBkMode == OPAQUE ? OPAQUE : TRANSPARENT;
							pgO->hlfntNew = pdcattr->hlfntNew;
							pgO->flTextAlign = pdcattr->flTextAlign;
							pgO->ptlViewportOrg = pdcattr->ptlViewportOrg;

							pgO->Size = DxSize; // of lpDx then string after.
							
							if (lpDx) RtlCopyMemory(&pgO->Buffer, lpDx, DxSize);

							if (cwc) RtlCopyMemory(&pgO->String[DxSize / sizeof(WCHAR)], lpString, cwc * sizeof(WCHAR));

							// Recompute offset and return size
							pTeb->GdiTebBatch.Offset += cjSize;
							((PGDIBATCHHDR)pgO)->Size += cjSize;
							return TRUE;
						}
						// Reset offset and count then fall through
						pTeb->GdiTebBatch.Offset -= sizeof(GDIBSTEXTOUT);
						pTeb->GdiBatchCount--;
					}
				}
			}
			return NtGdiExtTextOutW(hdc,
				x,
				y,
				fuOptions,
				(LPRECT)lprc,
				(LPWSTR)lpString,
				cwc,
				(LPINT)lpDx,
				0);
			*/
			return 0;
		}

	}
}