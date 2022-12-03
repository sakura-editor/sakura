/*!	@file
	@brief 文書データ1行

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor, genta
	Copyright (C) 2002, MIK, YAZAKI
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CDocLine.h"
#include "mem/CMemory.h"

CDocLine::CDocLine()
: m_pPrev( NULL ), m_pNext( NULL )
{
}

CDocLine::~CDocLine() = default;

/* 空行（スペース、タブ、改行記号のみの行）かどうかを取得する
	true：空行だ。
	false：空行じゃないぞ。

	2002/04/26 YAZAKI
*/
bool CDocLine::IsEmptyLine() const
{
	const wchar_t* pLine = GetPtr();
	int nLineLen = GetLengthWithoutEOL();
	int i;
	for ( i = 0; i < nLineLen; i++ ){
		if (pLine[i] != L' ' && pLine[i] != L'\t'){
			return false;	//	スペースでもタブでもない文字があったらfalse。
		}
	}
	return true;	//	すべてスペースかタブだけだったらtrue。
}

void CDocLine::SetEol(bool bEnableExtEol)
{
	const wchar_t* pData = m_cLine.GetStringPtr();
	int nLength = m_cLine.GetStringLength();
	//改行コード設定
	const wchar_t* p = &pData[nLength] - 1;
	while(p>=pData && WCODE::IsLineDelimiter(*p, bEnableExtEol))p--;
	p++;
	if(p>=pData){
		m_cEol.SetTypeByString(p, &pData[nLength]-p);
	}
	else{
		m_cEol = EEolType::none;
	}
}

void CDocLine::SetDocLineString(const wchar_t* pData, int nLength, bool bEnableExtEol)
{
	m_cLine.SetString(pData, nLength);
	SetEol(bEnableExtEol);
}

void CDocLine::SetDocLineString(const CNativeW& cData, bool bEnableExtEol)
{
	SetDocLineString(cData.GetStringPtr(), cData.GetStringLength(), bEnableExtEol);
}

void CDocLine::SetDocLineStringMove(CNativeW* pcDataFrom, bool bEnableExtEol)
{
	m_cLine.swap(*pcDataFrom);
	SetEol(bEnableExtEol);
}

void CDocLine::SetEol(const CEol& cEol, COpeBlk* pcOpeBlk)
{
	//改行コードを削除
	for(int i=0;i<(Int)m_cEol.GetLen();i++){
		m_cLine.Chop();
	}

	//改行コードを挿入
	m_cEol = cEol;
	m_cLine += cEol.GetValue2();
}
