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
#include "view/CGetLinearSelectedData.h"

CGetLinearSelectedData::CGetLinearSelectedData(
	const CEditView* _pcEditView,
	const CViewSelect& cSelection,
	std::wstring_view _quoteMark,
	size_t _nLineNumCols,
	bool _bInsertEolAtWrap,
	EEolType _newEolType
) noexcept
	: m_pcEditView(_pcEditView)
	, ptSelectFrom(cSelection.m_sSelect.GetFrom())
	, ptSelectTo(cSelection.m_sSelect.GetTo())
	, quoteMark(_quoteMark)
	, nLineNumCols(_nLineNumCols)
	, bInsertEolAtWrap(_bInsertEolAtWrap)
	, newEolType(_newEolType)
{
}

[[nodiscard]] size_t CGetLinearSelectedData::_CountData() const
{
	// コピーに必要なバッファサイズ
	size_t nBufSize = 0;

	const auto& cLayoutMgr = m_pcEditView->m_pcEditDoc->m_cLayoutMgr;

	// データ計測部
	for( auto nLineNum = ptSelectFrom.y; nLineNum <= ptSelectTo.y; ++nLineNum ){
		const CLayout* pcLayout = nullptr;
		if( CLogicInt nLineLen(0);
			!cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout ) || !pcLayout ){
			break;
		}

		// 行データが1文字以上ある場合
		if( auto [nIdxFrom, nIdxTo] = LineColumnsToIndexes(nLineNum, pcLayout);
			nIdxFrom < nIdxTo ){
			// 引用部分を表す文字列（「> 」など）を付与する
			if( quoteMark.length() > 0 ){
				nBufSize += quoteMark.length();
			}

			// 行番号を付与する
			if( nLineNumCols > 0 ){
				nBufSize += nLineNumCols + 2;
			}

			// 行データが改行コードで終わっているとき
			if( pcLayout->GetLayoutEol().IsValid() ){
				// 行データのサイズから、改行文字の「1桁分」を引く
				nBufSize += nIdxTo - nIdxFrom - 1;
				// 改行コードの「文字数」を足す
				nBufSize += newEolType == EEolType::none
					? pcLayout->GetLayoutEol().GetLen()
					: CEol(newEolType).GetLen();
			}
			// 行データが改行コードで終わっていない、かつ、折り返し改行を付けるとき
			else if (bInsertEolAtWrap){
				nBufSize += nIdxTo - nIdxFrom;
				// 改行コードの「文字数」を足す
				nBufSize += newEolType == EEolType::none
					? m_pcEditView->m_pcEditDoc->m_cDocEditor.GetNewLineCode().GetLen()
					: CEol(newEolType).GetLen();
			}
			// 行データが改行コードで終わっていないとき
			else{
				nBufSize += nIdxTo - nIdxFrom;
			}
		}
	}

	return nBufSize;
}

[[nodiscard]] bool CGetLinearSelectedData::_GetData( CNativeW& cmemBuf ) const
{
	const auto& cLayoutMgr = m_pcEditView->m_pcEditDoc->m_cLayoutMgr;

	// 行番号整形バッファ(L" 1234:"を出力できるよう桁数+2桁分確保する)
	std::wstring lineNumBuf(nLineNumCols + 2, wchar_t());

	// データ取得部
	for( auto nLineNum = ptSelectFrom.y; nLineNum <= ptSelectTo.y; ++nLineNum ){
		const CLayout* pcLayout = nullptr;
		CLogicInt nLineLen;
		const auto *pLine = cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
		if( pLine == nullptr || pcLayout == nullptr){
			break;
		}

		// 行データが1文字以上ある場合
		if( auto [nIdxFrom, nIdxTo] = LineColumnsToIndexes(nLineNum, pcLayout);
			nIdxFrom < nIdxTo ){
			// 引用部分を表す文字列（「> 」など）を付与する
			if( quoteMark.length() > 0 ){
				cmemBuf.AppendString( quoteMark.data() );
			}

			// 行番号を付与する
			if( nLineNumCols > 0 ){
				// 行番号は L" 1234:" 形式で出力する
				::swprintf_s(lineNumBuf.data(), lineNumBuf.capacity(), L"% *d:", static_cast<uint32_t>(nLineNumCols), (int)(Int)(nLineNum + 1));
				cmemBuf.AppendString(lineNumBuf.data());
			}

			// 行データが改行コードで終わっているとき
			if( pcLayout->GetLayoutEol().IsValid() ){
				// 行データの終端は改行コードの手前までにする
				// ※CRLFも表示上は「1桁」であることに注意。
				cmemBuf.AppendString( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 ); //改行文字の1桁分を引く
				// 変換指定に従い、改行コードを付与する
				cmemBuf.AppendString(newEolType == EEolType::none
					? pcLayout->GetLayoutEol().GetValue2()						//	コード保存
					: CEol(newEolType).GetValue2());							//	新規改行コード
			}
			// 行データが改行コードで終わっていない、かつ、折り返し改行を付けるとき
			else if (bInsertEolAtWrap){
				// 行データは終端まで出力する
				cmemBuf.AppendString(&pLine[nIdxFrom], nIdxTo - nIdxFrom);
				// ドキュメントの改行コードまたは指定された改行コードを付与する
				cmemBuf.AppendString(newEolType == EEolType::none
					? m_pcEditView->m_pcEditDoc->m_cDocEditor.GetNewLineCode().GetValue2()	//	コード保存
					: CEol(newEolType).GetValue2());										//	新規改行コード
			}
			// 行データが改行コードで終わっていないとき
			else{
				// 行データは終端まで出力する
				cmemBuf.AppendString(&pLine[nIdxFrom], nIdxTo - nIdxFrom);
			}
		}
	}

	return true;
}
