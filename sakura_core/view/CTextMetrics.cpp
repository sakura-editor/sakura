/*! @file */
/*
	Copyright (C) 2007, kobake
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
#include <algorithm>
#include <vector>
#include "CTextMetrics.h"
#include "charset/codechecker.h"

void CTextMetrics::CopyTextMetricsStatus(CTextMetrics* pDst) const
{
	pDst->m_nCharWidth = m_nCharWidth;		/* 半角文字の幅 */
	pDst->m_nCharHeight = m_nCharHeight;	/* 文字の高さ */
}

/*
	文字の大きさを調べる
	
	※ビルド種により、微妙にサイズが変わるようでした。
	　サイズを合わせるため、適当な文字で調整。
*/
void CTextMetrics::Update(HDC hdc, HFONT hFont, int nLineSpace, int nColmSpace)
{
	// CCharWidthCache::m_han_size と一致していなければならない
	//
	// KB145994
	// tmAveCharWidth は不正確(半角か全角なのかも不明な値を返す)
	// ただしこのコードはカーニングの影響を受ける
	auto hFontOld = (HFONT)::SelectObject( hdc, hFont );
	SIZE  sz;
	GetTextExtentPoint32(hdc, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &sz);
	::SelectObject( hdc, hFontOld );

	m_nCharWidth = (sz.cx / 26 + 1) / 2;
	if( nLineSpace < 0 ){
		// マイナスの場合は文字の高さも引く
		m_nCharHeight = std::max(1, static_cast<int>(sz.cy) + nLineSpace);
	} else {
		m_nCharHeight = sz.cy;
	}
	m_nDxBasis = m_nCharWidth + nColmSpace;
	std::fill(m_anHankakuDx.begin(), m_anHankakuDx.end(), m_nDxBasis);
	m_nDyBasis = std::max(1, static_cast<int>(sz.cy) + nLineSpace);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           取得                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 指定した文字列により文字間隔配列を生成する。
const int* CTextMetrics::GenerateDxArray(
	std::vector<int>* pvResultArray, //!< [out] 文字間隔配列の受け取りコンテナ
	const wchar_t* pText,           //!< [in]  文字列
	int nLength,                    //!< [in]  文字列長
	int	nHankakuDx,					//!< [in]  半角文字の文字間隔
	int	nTabSpace,					//   [in]  TAB幅(CLayoutXInt)
	int	nIndent,					//   [in]  インデント(TAB対応用)(CLayoutXInt)
	int nCharSpacing,				//!< [in]  文字隙間
	CCharWidthCache& cache
)
{
	if(!pText || nLength<=0)return nullptr;
	std::vector<int>& vResultArray = *pvResultArray;
	vResultArray.clear();

	for (int i = 0; i < nLength; ++i) {
		if (pText[i] == WCODE::TAB) {
			// TAB対応	2013/5/7 Uchi
			if (i > 0 && pText[i - 1] == WCODE::TAB) {
				vResultArray.push_back(nTabSpace);
				nIndent += nTabSpace;
				continue;
			}
			vResultArray.push_back((nTabSpace + nHankakuDx - 1) - ((nIndent + nHankakuDx - 1) % nTabSpace));
			nIndent += vResultArray.back();
			continue;
		}

		const int spacing = CNativeW::GetKetaOfChar(pText, nLength, i, cache) * nCharSpacing;
		if(IsUTF16High(pText[i]) && i + 1 < nLength && IsUTF16Low(pText[i + 1])) {
			vResultArray.push_back(cache.CalcPxWidthByFont2(pText + i) + spacing);
			vResultArray.push_back(0);
			i++;
			continue;
		}
		vResultArray.push_back(cache.CalcPxWidthByFont(pText[i]) + spacing);
		nIndent += vResultArray.back();
	}
	return vResultArray.data();
}

//!文字列のピクセル幅を返す。
int CTextMetrics::CalcTextWidth(
	const wchar_t*, //!< 文字列
	int nLength,          //!< 文字列長
	const int* pnDx       //!< 文字間隔の入った配列
)
{
	int w=0;
	for(int i=0;i<nLength;i++){
		w+=pnDx[i];
	}
	return w;
}

//!文字列のピクセル幅を返す。
int CTextMetrics::CalcTextWidth2(
	const wchar_t* pText, //!< 文字列
	int nLength,          //!< 文字列長
	int nHankakuDx,       //!< 半角文字の文字間隔
	int nCharSpacing,     //!< 文字の隙間
	std::vector<int>& vDxArray, //!< [out] 文字間隔配列
	CCharWidthCache& cache
)
{
	const int* pDxArray = CTextMetrics::GenerateDxArray(
		&vDxArray,
		pText,
		nLength,
		nHankakuDx,
		8,
		0,
		nCharSpacing,
		cache
	);

	//ピクセル幅を計算
	return CalcTextWidth(pText, nLength, pDxArray);
}

int CTextMetrics::CalcTextWidth3(
	const wchar_t* pText, //!< 文字列
	int nLength,          //!< 文字列長
	CCharWidthCache& cache
) const
{
	static std::vector<int> dxArray;
	return CalcTextWidth2(pText, nLength, GetCharPxWidth(), GetHankakuDx() - GetHankakuWidth(), dxArray, cache);
}
