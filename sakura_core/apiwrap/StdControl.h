#pragma once

/*
2007.09.17 kobake
�����R�[�h��WCHAR�Ȃ̂ŁA�����L�[���[�h�Ȃǂ�WCHAR�ŕێ�����B
���̂��߁A�����_�C�A���O�̃R���{�{�b�N�X�ȂǂɁAWCHAR��ݒ肷���ʂ��o�Ă���B
UNICODE�łł͖�薳�����AANSI�łł͐ݒ�̑O�ɃR�[�h�ϊ�����K�v������B
�Ăяo�����ŕϊ����Ă��ǂ����A�p�x�������̂ŁAWCHAR�𒼐ڎ󂯎��API���b�v�֐���񋟂���B

�܂��ASendMessage�̒��ڌĂяo���́A�ǂ����Ă�WPARAM,LPARAM�ւ̋����L���X�g�������邽�߁A
�R���p�C���̌^�`�F�b�N���������Awchar_t, char�̍��݂���\�[�X�R�[�h�̒��ł̓o�O�̉����ɂȂ�₷���B
�����������Ӗ��ł��A���̃t�@�C�����̃��b�v�֐����g�����Ƃ𐄏�����B
*/

#include "../util/tchar_convert.h"

namespace ApiWrap{

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �E�B���h�E����                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline BOOL Wnd_SetText(HWND hwnd, const ACHAR* str)
	{
		return SetWindowTextA(hwnd, str);
	}
	inline BOOL Wnd_SetText(HWND hwnd, const WCHAR* str)
	{
#ifdef _UNICODE
		return SetWindowTextW(hwnd, str);
#else
		return SetWindowTextA(hwnd, to_achar(str));
#endif
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �R���{�{�b�N�X                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline LRESULT Combo_AddString(HWND hwndCombo, const ACHAR* str)
	{
		return ::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)to_tchar(str) );
	}

	inline LRESULT Combo_AddString(HWND hwndCombo, const WCHAR* str)
	{
		return ::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)to_tchar(str) );
	}

	inline LRESULT Combo_GetLBText(HWND hwndCombo, int nIndex, TCHAR* str)
	{
		return ::SendMessage( hwndCombo, CB_GETLBTEXT, nIndex, (LPARAM)str );
	}

	inline LRESULT Combo_GetText(HWND hwndCombo, TCHAR* str, int cchMax)
	{
		return ::GetWindowText( hwndCombo, str, cchMax );
	}

	inline int Combo_DeleteString(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, CB_DELETESTRING, (WPARAM)index, 0L); }
	inline int Combo_FindStringExact(HWND hwndCtl, int indexStart, const ACHAR* lpszFind)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)to_tchar(lpszFind)); }
	inline int Combo_FindStringExact(HWND hwndCtl, int indexStart, const WCHAR* lpszFind)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)to_tchar(lpszFind)); }
	
	inline int Combo_GetCount(HWND hwndCtl)								{ return (int)(DWORD)::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L); }
	inline int Combo_GetCurSel(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, CB_GETCURSEL, 0L, 0L); }
	inline int Combo_SetCurSel(HWND hwndCtl, int index)					{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETCURSEL, (WPARAM)index, 0L); }
	inline LRESULT Combo_GetItemData(HWND hwndCtl, int index)			{ return ((LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, CB_GETITEMDATA, (WPARAM)index, 0L)); }
	inline int Combo_SetItemData(HWND hwndCtl, int index, int data)		{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int Combo_SetItemData(HWND hwndCtl, int index, void* data)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int Combo_GetLBTextLen(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, CB_GETLBTEXTLEN, (WPARAM)index, 0L); }
	inline int Combo_InsertString(HWND hwndCtl, int index, const ACHAR* lpsz)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_INSERTSTRING, (WPARAM)index, (LPARAM)to_tchar(lpsz)); }
	inline int Combo_InsertString(HWND hwndCtl, int index, const WCHAR* lpsz)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_INSERTSTRING, (WPARAM)index, (LPARAM)to_tchar(lpsz)); }
	inline int Combo_LimitText(HWND hwndCtl, int cchLimit)				{ return (int)(DWORD)::SendMessage(hwndCtl, CB_LIMITTEXT, (WPARAM)cchLimit, 0L); }
	inline int Combo_ResetContent(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, CB_RESETCONTENT, 0L, 0L); }
	inline int Combo_SetEditSel(HWND hwndCtl, int ichStart, int ichEnd)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETEDITSEL, 0L, MAKELPARAM(ichStart, ichEnd)); }
	inline int Combo_SetExtendedUI(HWND hwndCtl, UINT flags)			{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETEXTENDEDUI, (WPARAM)flags, 0L); }
	inline BOOL Combo_ShowDropdown(HWND hwndCtl, BOOL fShow)			{ return (BOOL)(DWORD)::SendMessage(hwndCtl, CB_SHOWDROPDOWN, (WPARAM)fShow, 0L); }
	inline int Combo_SetDroppedWidth(HWND hwndCtl, int width)			{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETDROPPEDWIDTH, (WPARAM)width, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      ���X�g�{�b�N�X                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LRESULT List_GetText(HWND hwndList, int nIndex, ACHAR* str);
	LRESULT List_GetText(HWND hwndList, int nIndex, WCHAR* str);

	inline LRESULT List_AddString(HWND hwndList, const ACHAR* str)
	{
		return ::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)to_tchar(str) );
	}
	inline LRESULT List_AddString(HWND hwndList, const WCHAR* str)
	{
		return ::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)to_tchar(str) );
	}
	inline int List_AddItemData(HWND hwndCtl, int data)					{ return (int)(DWORD)::SendMessage(hwndCtl, LB_ADDSTRING, 0L, (LPARAM)data); }
	inline int List_AddItemData(HWND hwndCtl, void* data)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_ADDSTRING, 0L, (LPARAM)data); }
	inline int List_DeleteString(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_DELETESTRING, (WPARAM)index, 0L); }
	inline int List_FindStringExact(HWND hwndCtl, int indexStart, ACHAR* lpszFind)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)to_tchar(lpszFind)); }
	inline int List_FindStringExact(HWND hwndCtl, int indexStart, WCHAR* lpszFind)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)to_tchar(lpszFind)); }
	inline int List_GetCaretIndex(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCARETINDEX, 0L, 0L); }
	inline int List_GetCount(HWND hwndCtl)								{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCOUNT, 0L, 0L); }
	inline int List_GetCurSel(HWND hwndCtl)								{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCURSEL, 0L, 0L); }
	inline int List_SetCurSel(HWND hwndCtl, int index)					{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETCURSEL, (WPARAM)index, 0L); }
	inline LRESULT List_GetItemData(HWND hwndCtl, int index)			{ return (LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, LB_GETITEMDATA, (WPARAM)index, 0L); }
	inline int List_SetItemData(HWND hwndCtl, int index, int data)		{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int List_SetItemData(HWND hwndCtl, int index, void* data)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int List_GetItemRect(HWND hwndCtl, int index, RECT* lprc)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETITEMRECT, (WPARAM)index, (LPARAM)lprc); }
	inline int List_GetTopIndex(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETTOPINDEX, 0L, 0L); }
	inline int List_InsertItemData(HWND hwndCtl, int index, int data)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, (LPARAM)data); }
	inline int List_InsertItemData(HWND hwndCtl, int index, void* data)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, (LPARAM)data); }
	inline int List_InsertString(HWND hwndCtl, int index, const ACHAR* lpsz)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, (LPARAM)to_tchar(lpsz)); }
	inline int List_InsertString(HWND hwndCtl, int index, const WCHAR* lpsz)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, (LPARAM)to_tchar(lpsz)); }
	inline BOOL List_ResetContent(HWND hwndCtl)							{ return (BOOL)(DWORD)::SendMessage(hwndCtl, LB_RESETCONTENT, 0L, 0L); }
	inline void List_SetHorizontalExtent(HWND hwndCtl, int cxExtent)	{ ::SendMessage(hwndCtl, LB_SETHORIZONTALEXTENT, (WPARAM)cxExtent, 0L); }
	inline int List_GetItemHeight(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETITEMHEIGHT, (WPARAM)index, 0L); }
	inline int List_SetItemHeight(HWND hwndCtl, int index, int cy)		{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETITEMHEIGHT, (WPARAM)index, MAKELPARAM(cy, 0)); }
	inline int List_SetTopIndex(HWND hwndCtl, int indexTop)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETTOPINDEX, (WPARAM)indexTop, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �G�f�B�b�g �R���g���[��                //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline void EditCtl_LimitText(HWND hwndCtl, int cchLimit)			{ ::SendMessage(hwndCtl, EM_LIMITTEXT, (WPARAM)(cchLimit), 0L); }
	inline void EditCtl_SetSel(HWND hwndCtl, int ichStart, int ichEnd)	{ ::SendMessage(hwndCtl, EM_SETSEL, ichStart, ichEnd); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �{�^�� �R���g���[��                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline int BtnCtl_GetCheck(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, BM_GETCHECK, 0L, 0L); }
	inline void BtnCtl_SetCheck(HWND hwndCtl, int check)				{ ::SendMessage(hwndCtl, BM_SETCHECK, (WPARAM)check, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �X�^�e�B�b�N �R���g���[��              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline HICON StCtl_SetIcon(HWND hwndCtl, HICON hIcon)				{ return (HICON)(UINT_PTR)::SendMessage(hwndCtl, STM_SETICON, (WPARAM)hIcon, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �_�C�A���O��                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline BOOL DlgItem_SetText(HWND hwndDlg, int nIDDlgItem, const ACHAR* str)
	{
		return SetDlgItemText(hwndDlg, nIDDlgItem, to_tchar(str));
	}
	inline BOOL DlgItem_SetText(HWND hwndDlg, int nIDDlgItem, const WCHAR* str)
	{
		return SetDlgItemText(hwndDlg, nIDDlgItem, to_tchar(str));
	}

	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, ACHAR* str, int nMaxCount);
	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, WCHAR* str, int nMaxCount);
	//GetDlgItemText

}
using namespace ApiWrap;


