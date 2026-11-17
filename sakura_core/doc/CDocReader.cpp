/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "doc/CDocReader.h"

#include "doc/logic/CDocLine.h"
#include "doc/logic/CDocLineMgr.h"

/*!
 * @brief 全行データを返す
 *
 * @returns 全行データ。
 * @note Debug版のテストにのみ使用している。
 * @note 設定されたデータをそのまま返すよう実装を変更。（改行コードをCRLFに変更する機能はドロップ。）
 */
std::wstring CDocReader::GetAllText() const
{
	std::wstring buffer{};

	// 全行データのサイズを計測する
	size_t allDataSize = 0;
	for (auto pDocLine = m_pcDocLineMgr->GetDocLineTop();
		pDocLine;
		pDocLine = pDocLine->GetNextLine())
	{
		allDataSize += pDocLine->length();
	}

	// データが空でない場合
	if (allDataSize) {
		// バッファサイズを調整する
		buffer.reserve(allDataSize);

		for (auto pDocLine = m_pcDocLineMgr->GetDocLineTop();
			pDocLine;
			pDocLine = pDocLine->GetNextLine())
		{
			buffer.append(pDocLine->data(), pDocLine->length());
		}
	}

	return buffer;
}

const wchar_t* CDocReader::GetLineStr( CLogicInt nLine, CLogicInt* pnLineLen )
{
	const CDocLine* pDocLine;
	pDocLine = m_pcDocLineMgr->GetLine( nLine );
	if( nullptr == pDocLine ){
		*pnLineLen = CLogicInt(0);
		return nullptr;
	}
	// 2002/2/10 aroka CMemory のメンバ変数に直接アクセスしない(inline化されているので速度的な問題はない)
	return pDocLine->GetDocLineStrWithEOL( pnLineLen );
}

/*!
	指定された行番号の文字列と改行コードを除く長さを取得
	
	@author Moca
	@date 2003.06.22
*/
const wchar_t* CDocReader::GetLineStrWithoutEOL( CLogicInt nLine, int* pnLineLen )
{
	const CDocLine* pDocLine = m_pcDocLineMgr->GetLine( nLine );
	if( nullptr == pDocLine ){
		*pnLineLen = 0;
		return nullptr;
	}
	*pnLineLen = pDocLine->GetLengthWithoutEOL();
	return pDocLine->GetPtr();
}

/*! 順アクセスモード：先頭行を得る

	@param pnLineLen [out] 行の長さが返る。
	@return 1行目の先頭へのポインタ。
	データが1行もないときは、長さ0、ポインタNULLが返る。

*/
const wchar_t* CDocReader::GetFirstLinrStr( int* pnLineLen )
{
	if (!pnLineLen) throw std::invalid_argument("pnLineLen is required");

	return m_pcDocLineMgr->GetFirstLinrStr(*pnLineLen);
}

/*!
	順アクセスモード：次の行を得る

	@param pnLineLen [out] 行の長さが返る。
	@return 次行の先頭へのポインタ。
	GetFirstLinrStr()が呼び出されていないとNULLが返る

*/
const wchar_t* CDocReader::GetNextLinrStr( int* pnLineLen )
{
	if (!pnLineLen) throw std::invalid_argument("pnLineLen is required");

	return m_pcDocLineMgr->GetNextLinrStr(*pnLineLen);
}
