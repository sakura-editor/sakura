/*!	@file
	@brief マクロエンジン

	@author genta
	@date 2002.4.29
*/
/*
	Copyright (C) 2002, genta
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "macro/CMacroManagerBase.h"

#include "view/CEditView.h"
#include "cmd/CViewCommander_inline.h"
#include "cmd/COpeBlk.h"

// CMacroBeforeAfter

void CMacroBeforeAfter::ExecKeyMacroBefore(
	CEditView* pcEditView,
	int flags [[maybe_unused]]
)
{
	if (const auto opeBlk = pcEditView->m_cCommander.GetOpeBlk();
		opeBlk)
	{
		m_nOpeBlkCount = opeBlk->GetRefCount();
	}
	else {
		m_nOpeBlkCount = 0;
	}

	m_bDrawSwitchOld = pcEditView->GetDrawSwitch();
}

void CMacroBeforeAfter::ExecKeyMacroAfter(
	CEditView* pcEditView,
	int flags [[maybe_unused]],
	 bool bRet [[maybe_unused]]
)
{
	const auto opeBlk = pcEditView->m_cCommander.GetOpeBlk();
	if (0 < m_nOpeBlkCount) {
		if (!opeBlk) {
			pcEditView->m_cCommander.SetOpeBlk(new COpeBlk());
		}

		if( pcEditView->m_cCommander.GetOpeBlk()->GetRefCount() != m_nOpeBlkCount ){
			pcEditView->m_cCommander.GetOpeBlk()->SetRefCount( m_nOpeBlkCount );
		}
	}
	else if (opeBlk) {
		opeBlk->SetRefCount(1); // 強制的にリセットするため1を指定
		pcEditView->SetUndoBuffer();
	}

	GetEditWnd().SetDrawSwitchOfAllViews(m_bDrawSwitchOld);
}

// CMacroManagerBase

void CMacroManagerBase::ExecKeyMacro2(
	CEditView* pcEditView,
	int flags
)
{
	ExecKeyMacroBefore(pcEditView, flags);

	const auto bRet = ExecKeyMacro(pcEditView, flags);

	ExecKeyMacroAfter(pcEditView, flags, bRet);
}
