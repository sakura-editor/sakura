﻿/*! @file */
/*
	Copyright (C) 2013, novice

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
#pragma once

#include "COpeBlk.h"
#include "doc/CEditDoc.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"

//外部依存
inline CEditDoc *CViewCommander::GetDocument() { return m_pCommanderView->m_pcEditDoc; }
inline CEditWnd *CViewCommander::GetEditWindow() { return m_pCommanderView->m_pcEditWnd; }
inline HWND		 CViewCommander::GetMainWindow() { return ::GetParent(m_pCommanderView->m_hwndParent); }
inline COpeBlk * CViewCommander::GetOpeBlk() { return GetDocument()->m_cDocEditor.m_pcOpeBlk; }
inline void		 CViewCommander::SetOpeBlk(COpeBlk *p)
{
	GetDocument()->m_cDocEditor.m_pcOpeBlk			= p;
	GetDocument()->m_cDocEditor.m_nOpeBlkRedawCount = 0;
}
inline CLayoutRange &CViewCommander::GetSelect() { return m_pCommanderView->GetSelectionInfo().m_sSelect; }
inline CCaret &		 CViewCommander::GetCaret() { return m_pCommanderView->GetCaret(); }
