/*!	@file
	@brief CEditViewクラスのgrep関連コマンド処理系関数群

	@author genta
	@date	2005/01/10 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "_main/CControlTray.h"
#include "charset/charcode.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "sakura_rc.h"

/*!
	コマンドコードの変換(grep mode時)
*/
void CEditView::TranslateCommand_grep(
	EFunctionCode&	nCommand,
	bool&			bRedraw,
	LPARAM&			lparam1,
	LPARAM&			lparam2,
	LPARAM&			lparam3,
	LPARAM&			lparam4
)
{
	if( ! CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode )
		return;

	if( nCommand == F_WCHAR ){
		//	Jan. 23, 2005 genta 文字判定忘れ
		if( WCODE::IsLineDelimiter((wchar_t)lparam1, GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol)
				&& GetDllShareData().m_Common.m_sSearch.m_bGTJW_RETURN ){
			nCommand = F_TAGJUMP;
			lparam1 = GetKeyState_Control()?1:0;
		}
	}
}
