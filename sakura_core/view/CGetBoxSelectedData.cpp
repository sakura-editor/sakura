/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#include "StdAfx.h"
#include "view/CGetBoxSelectedData.h"

CGetBoxSelectedData::CGetBoxSelectedData(
	const CEditView* _pcEditView,
	const CLayoutRect& _rcSel,
	const bool _bEnableExtEol
) noexcept
	: m_pcEditView(_pcEditView)
	, rcSel(_rcSel)
	, bEnableExtEol(_bEnableExtEol)
{
}

[[nodiscard]] size_t CGetBoxSelectedData::_CountData() const
{
	// コピーに必要なバッファサイズ
	size_t nBufSize = 0;

	const auto& cLayoutMgr = m_pcEditView->m_pcEditDoc->m_cLayoutMgr;

	// データ計測部
	for( auto nLineNum = rcSel.top; nLineNum <= rcSel.bottom; ++nLineNum ){
		const CLayout* pcLayout = nullptr;
		CLogicInt nLineLen;
		const auto* pLine = cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
		if( !pLine || !pcLayout ){
			break;
		}

		// 行データが1文字以上ある場合
		if (auto [nIdxFrom, nIdxTo] = LineColumnsToIndexes(nLineNum, pcLayout);
			nIdxFrom < nIdxTo) {
			// 選択範囲が改行コードで終わっているとき
			if( WCODE::IsLineDelimiter( pLine[nIdxTo - 1], bEnableExtEol ) ){
				// 行データのサイズから、改行文字の「1桁分」を引く
				nBufSize += nIdxTo - nIdxFrom - 1;
			}
			// 選択範囲が改行コードで終わっていないとき
			else {
				nBufSize += nIdxTo - nIdxFrom;
			}
		}

		// 矩形選択のコピー時は改行コード固定。
		nBufSize += 2; // countof(WCODE::CRLF) - 1
	}

	return nBufSize;
}

[[nodiscard]] bool CGetBoxSelectedData::_GetData( CNativeW& cmemBuf ) const
{
	const auto& cLayoutMgr = m_pcEditView->m_pcEditDoc->m_cLayoutMgr;

	// データ取得部
	for( auto nLineNum = rcSel.top; nLineNum <= rcSel.bottom; ++nLineNum ){
		const CLayout* pcLayout = nullptr;
		CLogicInt nLineLen;
		const auto* pLine = cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
		if( pcLayout == nullptr || pLine == nullptr ){
			break;
		}

		// 行データが1文字以上ある場合
		if (auto [nIdxFrom, nIdxTo] = LineColumnsToIndexes(nLineNum, pcLayout);
			nIdxFrom < nIdxTo) {
			// 選択範囲が改行コードで終わっているとき
			if( WCODE::IsLineDelimiter(pLine[nIdxTo - 1], bEnableExtEol) ){
				// 行データの終端は改行コードの手前までにする
				// ※CRLFも表示上は「1桁」であることに注意。
				cmemBuf.AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
			}
			// 選択範囲が改行コードで終わっていないとき
			else{
				cmemBuf.AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
			}
		}

		// 矩形選択のコピー時は改行コード固定。
		cmemBuf.AppendString(WCODE::CRLF);
	}

	return true;
}
