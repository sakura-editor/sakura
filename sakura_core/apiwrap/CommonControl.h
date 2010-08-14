#pragma once

namespace ApiWrap
{
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      ステータスバー                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline LRESULT StatusBar_SetText(HWND hwndStatus, WPARAM opt, const TCHAR* str)
	{
		return ::SendMessage( hwndStatus, SB_SETTEXT, opt, (LPARAM)str );
	}

	inline int StatusBar_SetParts(HWND hwndCtl, int num, int* positions)		{ return (int)(DWORD)::SendMessage(hwndCtl, SB_SETPARTS, (WPARAM)num, (LPARAM)positions); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      HotKey コントロール                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline LRESULT HotKey_GetHotKey(HWND hwndCtl)								{ return (LRESULT)::SendMessage(hwndCtl, HKM_GETHOTKEY, 0L, 0L); }
	inline void HotKey_SetHotKey(HWND hwndCtl, DWORD vk_code, DWORD modifier)	{ ::SendMessage(hwndCtl, HKM_SETHOTKEY, MAKEWORD(vk_code, modifier), 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                  プログレスバー コントロール                //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline int Progress_SetRange(HWND hwndCtl, int minimum, int maximum)	{ return (int)(DWORD)::SendMessage(hwndCtl, PBM_SETRANGE, 0L, MAKELPARAM(minimum, maximum)); }
	inline int Progress_SetPos(HWND hwndCtl, int position)					{ return (int)(DWORD)::PostMessage(hwndCtl, PBM_SETPOS, (WPARAM)position, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      Up-Down コントロール                   //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline LRESULT UpDown_SetRange(HWND hwndCtl, int upper, int lower)	{ return (LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, UDM_SETRANGE, 0L, MAKELPARAM(upper, lower)); }
	inline LRESULT UpDown_GetPos(HWND hwndCtl)							{ return (LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, UDM_GETPOS, 0L, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      Rebar コントロール                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline int Rebar_InsertBand(HWND hwndCtl, int index, REBARBANDINFO* info)	{ return (int)(DWORD)::SendMessage(hwndCtl, RB_INSERTBAND, (WPARAM)index, (LPARAM)info); }
	inline int Rebar_SetbarInfo(HWND hwndCtl, REBARINFO* info)					{ return (int)(DWORD)::SendMessage(hwndCtl, RB_SETBARINFO, 0L, (LPARAM)info); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      Toolbar コントロール                   //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline BOOL Toolbar_AddButtons(HWND hwndCtl, int num, TBBUTTON* buttons)		{ return (BOOL)(DWORD)::SendMessage(hwndCtl, TB_ADDBUTTONS, (WPARAM)num, (LPARAM)buttons); }
	inline void Toolbar_ButtonStructSize(HWND hwndCtl, int size)					{ ::SendMessage(hwndCtl, TB_BUTTONSTRUCTSIZE, (WPARAM)size, 0L); }
	inline BOOL Toolbar_CheckButton(HWND hwndCtl, int id, BOOL enable )				{ return (BOOL)(DWORD)::SendMessage(hwndCtl, TB_CHECKBUTTON, (WPARAM)id, MAKELONG(enable, 0)); }
	inline BOOL Toolbar_EnableButton(HWND hwndCtl, int id, BOOL enable )			{ return (BOOL)(DWORD)::SendMessage(hwndCtl, TB_ENABLEBUTTON, (WPARAM)id, MAKELONG(enable, 0)); }
	inline int Toolbar_GetBitmap(HWND hwndCtl, int index)							{ return (int)(DWORD)::SendMessage(hwndCtl, TB_GETBITMAP, (WPARAM)index, 0L); }
	inline DWORD Toolbar_GetButtonSize(HWND hwndCtl)								{ return (DWORD)::SendMessage(hwndCtl, TB_GETBUTTONSIZE, 0L, 0L); }
	inline BOOL Toolbar_GetItemRect(HWND hwndCtl, int index, RECT* rect)			{ return (BOOL)(DWORD)::SendMessage(hwndCtl, TB_GETITEMRECT, (WPARAM)index, (LPARAM)rect); }
	inline int Toolbar_GetRows(HWND hwndCtl)										{ return (int)(DWORD)::SendMessage(hwndCtl, TB_GETROWS, 0L, 0L); }
	inline int Toolbar_Hittest(HWND hwndCtl, POINT* point)							{ return (int)(DWORD)::SendMessage(hwndCtl, TB_HITTEST, 0L, (LPARAM)point); }
	inline int Toolbar_SetButtonInfo(HWND hwndCtl, int index, TBBUTTONINFO* info)	{ return (int)(DWORD)::SendMessage(hwndCtl, TB_SETBUTTONINFO, (WPARAM)index, (LPARAM)info); }
	inline BOOL Toolbar_SetButtonSize(HWND hwndCtl, int width, int height)			{ return (BOOL)(DWORD)::SendMessage(hwndCtl, TB_SETBUTTONSIZE, 0L, MAKELONG(width, height)); }
	inline DWORD Toolbar_SetExtendedStyle(HWND hwndCtl, DWORD styles)				{ return (DWORD)::SendMessage(hwndCtl, TB_SETEXTENDEDSTYLE, 0L, (LPARAM)styles); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      Tooltip コントロール                   //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline BOOL Tooltip_AddTool(HWND hwndCtl, TOOLINFO* info)			{ return (BOOL)(DWORD)::SendMessage(hwndCtl, TTM_ADDTOOL, 0L, (LPARAM)info); }
	inline int Tooltip_SetMaxTipWidth(HWND hwndCtl, int width)			{ return (int)(DWORD)::SendMessage(hwndCtl, TTM_SETMAXTIPWIDTH, 0L, (LPARAM)width); }
	inline void Tooltip_UpdateTipText(HWND hwndCtl, TOOLINFO* info)		{ ::SendMessage(hwndCtl, TTM_UPDATETIPTEXT, 0L, (LPARAM)info); }
}

using namespace ApiWrap;
