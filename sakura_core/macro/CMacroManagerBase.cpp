/*!	@file
	@brief マクロエンジン

	@author genta
	@date 2002.4.29
*/
/*
	Copyright (C) 2002, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CMacroManagerBase.h"
#include "view/CEditView.h"
#include "cmd/CViewCommander_inline.h"
#include "COpeBlk.h"

// CMacroBeforeAfter

void CMacroBeforeAfter::ExecKeyMacroBefore( class CEditView* pcEditView, int flags )
{
	COpeBlk* opeBlk = pcEditView->m_cCommander.GetOpeBlk();
	if( opeBlk ){
		m_nOpeBlkCount = opeBlk->GetRefCount();
	}else{
		m_nOpeBlkCount = 0;
	}
	m_bDrawSwitchOld = pcEditView->GetDrawSwitch();
}

void CMacroBeforeAfter::ExecKeyMacroAfter( class CEditView* pcEditView, int flags, bool bRet )
{
	if( 0 < m_nOpeBlkCount ){
		COpeBlk* opeBlk = pcEditView->m_cCommander.GetOpeBlk();
		if( opeBlk == NULL ){
			pcEditView->m_cCommander.SetOpeBlk(new COpeBlk());
		}
		if( pcEditView->m_cCommander.GetOpeBlk()->GetRefCount() != m_nOpeBlkCount ){
			pcEditView->m_cCommander.GetOpeBlk()->SetRefCount( m_nOpeBlkCount );
		}
	}else{
		COpeBlk* opeBlk = pcEditView->m_cCommander.GetOpeBlk();
		if( opeBlk ){
			opeBlk->SetRefCount(1); // 強制的にリセットするため1を指定
			pcEditView->SetUndoBuffer();
		}
	}
	GetEditWnd().SetDrawSwitchOfAllViews(m_bDrawSwitchOld);
}

// CMacroManagerBase
//	デフォルトのコンストラクタ・デストラクタ

CMacroManagerBase::CMacroManagerBase()
 : m_nReady( false )
{}

CMacroManagerBase::~CMacroManagerBase()
{}

void CMacroManagerBase::ExecKeyMacro2( class CEditView* pcEditView, int flags )
{
	ExecKeyMacroBefore(pcEditView, flags);
	bool b = ExecKeyMacro(pcEditView, flags);
	ExecKeyMacroAfter(pcEditView, flags, b);
}
