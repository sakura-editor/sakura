/*!	@file
@brief CViewCommanderクラスのコマンド(挿入系)関数群

	2012/12/15	CViewCommander.cpp,CViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "dlg/CDlgCtrlCode.h" //コントロールコードの入力(ダイアログ)
#include "env/CFormatManager.h"
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"

//日付挿入
void CViewCommander::Command_INS_DATE(void)
{
    // 日付をフォーマット
    WCHAR szText[1024];
    SYSTEMTIME systime;
    ::GetLocalTime(&systime);
    CFormatManager().MyGetDateFormat(systime, szText, _countof(szText) - 1);

    // テキストを貼り付け ver1
    Command_INSTEXT(true, szText, CLogicInt(-1), TRUE);
}

//時刻挿入
void CViewCommander::Command_INS_TIME(void)
{
    // 時刻をフォーマット
    WCHAR szText[1024];
    SYSTEMTIME systime;
    ::GetLocalTime(&systime);
    CFormatManager().MyGetTimeFormat(systime, szText, _countof(szText) - 1);

    // テキストを貼り付け ver1
    Command_INSTEXT(true, szText, CLogicInt(-1), TRUE);
}

//	from CViewCommander_New.cpp
/*!	コントロールコードの入力(ダイアログ)
	@author	MIK
	@date	2002/06/02
*/
void CViewCommander::Command_CtrlCode_Dialog(void)
{
    CDlgCtrlCode cDlgCtrlCode;

    //コントロールコード入力ダイアログを表示する
    if (cDlgCtrlCode.DoModal(G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument()))
    {
        //コントロールコードを入力する
        // 2013.06.11 Command_WCHAR -> HandleCommand マクロ記録対応
        // 2013.12.12 F_WCHAR -> F_CTRL_CODE
        HandleCommand(F_CTRL_CODE, true, cDlgCtrlCode.GetCharCode(), 0, 0, 0);
    }
}

// 最近使ったファイル挿入
void CViewCommander::Command_INS_FILE_USED_RECENTLY(void)
{
    std::wstring eol = GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2();
    std::wstring s;
    CMRUFile cMRUFile;
    auto list = cMRUFile.GetPathList();
    for (auto item : list)
    {
        s += item;
        s += eol;
    }
    Command_INSTEXT(true, s.c_str(), (CLogicInt)s.size(), TRUE);
}

// 最近使ったフォルダ挿入
void CViewCommander::Command_INS_FOLDER_USED_RECENTLY(void)
{
    std::wstring eol = GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2();
    std::wstring s;
    CMRUFolder cMRUFolder;
    auto list = cMRUFolder.GetPathList();
    for (auto item : list)
    {
        s += item;
        s += eol;
    }
    Command_INSTEXT(true, s.c_str(), (CLogicInt)s.size(), TRUE);
}
