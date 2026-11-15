/*!	@file
	@brief 文書データ1行

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor, genta
	Copyright (C) 2002, MIK, YAZAKI
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "doc/logic/CDocLine.h"

#include "mem/CMemory.h"

CDocLine::CDocLine()
{
	return;
}

CDocLine::~CDocLine()
{
	return;
}

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

/*!
 * @brief 現在のバッファから改行コードを設定する
 */
void CDocLine::SetEol(
	bool bEnableExtEol
)
{
	// 一旦、改行コード「なし」にする
	m_cEol = EEolType::none;

	// 行データを取り出す
	const auto line = m_cLine.str();

	// 行データが空なら改行コード「なし」で確定
	if (line.empty()) return;

	// 改行コードの開始位置を探す
	const auto found = std::find_if(
		line.rbegin(),
		line.rend(),
		[bEnableExtEol](wchar_t wc) {
			return !WCODE::IsLineDelimiter(wc, bEnableExtEol);
		}
	);

	// 改行コードの開始位置を確定する
	const auto nOffsetToEol = found != line.rend()
		? std::distance(line.begin(), found.base())
		: 0;

	// 改行コードの部分文字列を切り出す
	std::wstring_view eol = line.substr(nOffsetToEol);

	//改行コード設定
	m_cEol.SetTypeByString(eol.data(), eol.length());
}

void CDocLine::SetDocLineString(
	std::wstring_view line,
	bool bEnableExtEol
)
{
	// 行データのバッファを更新する
	m_cLine.SetString(line.data(), line.length());

	// 現在のバッファから改行コードを設定する
	SetEol(bEnableExtEol);
}

void CDocLine::SetDocLineString(
	const wchar_t* pData,
	int nLength,
	bool bEnableExtEol
)
{
	// 引数から文字列参照を構築する
	std::wstring_view line{ pData, static_cast<size_t>(nLength) };

	// 文字列を渡すバージョンを呼び出す
	SetDocLineString(line, bEnableExtEol);
}

void CDocLine::SetDocLineStringMove(CNativeW* pcDataFrom, bool bEnableExtEol)
{
	m_cLine.swap(*pcDataFrom);
	SetEol(bEnableExtEol);
}

void CDocLine::SetEol(const CEol& cEol, [[maybe_unused]] COpeBlk* pcOpeBlk)
{
	//改行コードを削除
	for(int i=0;i<(Int)m_cEol.GetLen();i++){
		m_cLine.Chop();
	}

	//改行コードを挿入
	m_cEol = cEol;
	m_cLine += cEol.GetValue2();
}
