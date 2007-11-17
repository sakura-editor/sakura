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

	inline LRESULT Combo_GetText(HWND hwndCombo, int nIndex, TCHAR* str)
	{
		return ::SendMessage( hwndCombo, CB_GETLBTEXT, nIndex, (LPARAM)str );
	}

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


