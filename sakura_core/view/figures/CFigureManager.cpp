/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CFigureManager.h"
#include "CFigure_Tab.h"
#include "CFigure_Comma.h"
#include "CFigure_HanSpace.h"
#include "CFigure_ZenSpace.h"
#include "CFigure_Eol.h"
#include "CFigure_CtrlCode.h"

CFigureManager::CFigureManager()
{
	m_vFigures.push_back(new CFigure_Tab());
	m_vFigures.push_back(new CFigure_Comma());
	m_vFigures.push_back(new CFigure_HanSpace());
	m_vFigures.push_back(new CFigure_ZenSpace());
	m_vFigures.push_back(new CFigure_Eol());
	m_vFigures.push_back(new CFigure_CtrlCode());
	m_vFigures.push_back(new CFigure_Text());

	OnChangeSetting();
}

CFigureManager::~CFigureManager()
{
	m_vFiguresDisp.clear();

	int size = (int)m_vFigures.size();
	for(int i = 0; i < size; i++){
		SAFE_DELETE(m_vFigures[i]);
	}
	m_vFigures.clear();
}

//$$ 高速化可能
CFigure& CFigureManager::GetFigure(const wchar_t* pText, int nTextLen)
{
	int size = (int)m_vFiguresDisp.size();
	for(int i = 0; i < size; i++){
		CFigure* pcFigure = m_vFiguresDisp[i];
		if(pcFigure->Match(pText, nTextLen)){
			return *pcFigure;
		}
	}

	assert(0);
	return *m_vFiguresDisp.back();
}

/*! 設定更新
*/
void CFigureManager::OnChangeSetting(void)
{
	m_vFiguresDisp.clear();

	int size = (int)m_vFigures.size();
	int i;
	for(i = 0; i < size; i++){
		m_vFigures[i]->Update();
		// 色分け表示対象のみを登録
		if( m_vFigures[i]->Disp() ){
			m_vFiguresDisp.push_back(m_vFigures[i]);
		}
	}
}
