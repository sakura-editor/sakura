﻿/*! @file */
/*
	Copyright (C) 2013, novice
	Copyright (C) 2018-2022, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CVIEWCOMMANDER_INLINE_EF3571A1_FED4_439C_957F_E77B28E2384F_H_
#define SAKURA_CVIEWCOMMANDER_INLINE_EF3571A1_FED4_439C_957F_E77B28E2384F_H_
#pragma once

#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "COpeBlk.h"

//外部依存
inline CEditDoc* CViewCommander::GetDocument()
{
	return m_pCommanderView->m_pcEditDoc;
}
inline CEditWnd* CViewCommander::GetEditWindow()
{
	return &GetEditWnd();
}
inline HWND CViewCommander::GetMainWindow()
{
	return ::GetParent( m_pCommanderView->m_hwndParent );
}
inline COpeBlk* CViewCommander::GetOpeBlk()
{
	return GetDocument()->m_cDocEditor.m_pcOpeBlk;
}
inline void CViewCommander::SetOpeBlk(COpeBlk* p)
{
	GetDocument()->m_cDocEditor.m_pcOpeBlk = p;
	GetDocument()->m_cDocEditor.m_nOpeBlkRedawCount = 0;
}
inline CLayoutRange& CViewCommander::GetSelect()
{
	return m_pCommanderView->GetSelectionInfo().m_sSelect;
}
inline CCaret& CViewCommander::GetCaret()
{
	return m_pCommanderView->GetCaret();
}
#endif /* SAKURA_CVIEWCOMMANDER_INLINE_EF3571A1_FED4_439C_957F_E77B28E2384F_H_ */
