/*!	@file
	@brief テキストのレイアウト情報

	@author Norio Nakatani
	@date 1998/3/11 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CLayout.h"
#include "debug.h"
#include "charcode.h"
#include "CLayoutMgr.h"



CLayout::~CLayout()
{
	return;
}

void CLayout::DUMP( void )
{
#ifdef _DEBUG
	MYTRACE_A( "\n\n■CLayout::DUMP()======================\n" );
	MYTRACE_A( "m_ptLogicPos.y=%d\t\t対応する論理行番号\n", m_ptLogicPos.y );
	MYTRACE_A( "m_ptLogicPos.x=%d\t\t対応する論理行の先頭からのオフセット\n", m_ptLogicPos.x );
	MYTRACE_A( "m_nLength=%d\t\t対応する論理行のハイト数\n", m_nLength.GetValue() );
	MYTRACE_A( "m_nTypePrev=%d\t\tタイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 \n", m_nTypePrev );
	MYTRACE_A( "======================\n" );
#endif
	return;
}

//! オフセット値をレイアウト単位に変換して取得。2007.10.17 kobake
CLayoutInt CLayout::CalcLayoutOffset(const CLayoutMgr& pLayoutMgr) const
{
	CLayoutInt nRet(0);
	if(this->GetLogicOffset()){
		const wchar_t* pLine = this->m_pCDocLine->GetPtr();
		int nLineLen = this->m_pCDocLine->GetLength();
		for(int i=0;i<GetLogicOffset();i++){
			if(pLine[i]==WCODE::TAB){
				nRet+=pLayoutMgr.GetActualTabSpace(nRet);
			}
			else{
				nRet+=CNativeW2::GetKetaOfChar(pLine,nLineLen,i);
			}
		}
	}
	return nRet;
}

/*[EOF]*/
