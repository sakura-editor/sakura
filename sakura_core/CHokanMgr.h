/*!	@file
	@brief キーワード補完

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, asa-o
	Copyright (C) 2003, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Windows.h>
#include "dlg/CDialog.h"
#include "util/container.h"

/*! @brief キーワード補完

	@date 2003.06.25 Moca ファイル内からの補完機能を追加
*/
class CHokanMgr final : public CDialog
{
  public:
    /*
	||  Constructors
	*/
    CHokanMgr();
    ~CHokanMgr();

    HWND DoModeless(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam); /* モードレスダイアログの表示 */
    void Hide(void);
    /* 初期化 */
    int Search(
        POINT *ppoWin,
        int nWinHeight,
        int nColumnWidth,
        const wchar_t *pszCurWord,
        const WCHAR *pszHokanFile,
        bool bHokanLoHiCase, // 入力補完機能：英大文字小文字を同一視する 2001/06/19 asa-o
        bool bHokanByFile, // 編集中データから候補を探す。 2003.06.23 Moca
        int nHokanType,
        bool bHokanByKeyword,
        CNativeW *pcmemHokanWord = NULL // 補完候補が１つのときこれに格納 2001/06/19 asa-o
    );
    void HokanSearchByKeyword(
        const wchar_t *pszCurWord,
        bool bHokanLoHiCase,
        vector_ex<std::wstring> &vKouho);
    //	void SetCurKouhoStr( void );
    BOOL DoHokan(int nVKey);
    void ChangeView(LPARAM pcEditView); /* モードレス時：対象となるビューの変更 */

    INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) override;
    BOOL OnInitDialog(HWND, WPARAM wParam, LPARAM lParam) override;
    BOOL OnDestroy(void) override;
    BOOL OnSize(WPARAM wParam, LPARAM lParam) override;
    BOOL OnLbnSelChange(HWND hwndCtl, int wID) override;

    int KeyProc(WPARAM wParam, LPARAM lParam);

    //	2001/06/18 asa-o
    void ShowTip(); // 補完ウィンドウで選択中の単語にキーワードヘルプの表示

    static bool AddKouhoUnique(vector_ex<std::wstring> &, const std::wstring &);

    CNativeW m_cmemCurWord;
    vector_ex<std::wstring> m_vKouho;
    int m_nKouhoNum;

    int m_nCurKouhoIdx;

    POINT m_poWin;
    int m_nWinHeight;
    int m_nColumnWidth;
    int m_bTimerFlag;

  protected:
    /*
	||  実装ヘルパ関数
	*/
    LPVOID GetHelpIdTable(void) override; //@@@ 2002.01.18 add
};
