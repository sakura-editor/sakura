/*!	@file
	@brief CEditView�N���X��grep�֘A�R�}���h�����n�֐��Q

	@author genta
	@date	2005/01/10 �쐬
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
#include "stdafx.h"
#include "sakura_rc.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "CControlTray.h"
#include "charset/charcode.h"
#include "CEditApp.h"
#include "CGrepAgent.h"

/*!
	�R�}���h�R�[�h�̕ϊ�(grep mode��)
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
	if( ! CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode )
		return;

	if( nCommand == F_WCHAR ){
		//	Jan. 23, 2005 genta ��������Y��
		if( WCODE::IsLineDelimiter((wchar_t)lparam1) && GetDllShareData().m_Common.m_sSearch.m_bGTJW_RETURN ){
			nCommand = F_TAGJUMP;
			lparam1 = GetKeyState_Control()?1:0;
		}
	}
}
