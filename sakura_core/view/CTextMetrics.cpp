/*
	Copyright (C) 2007, kobake

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
#include <vector>
#include "CTextMetrics.h"
#include "charset/codechecker.h"

using namespace std;

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CTextMetrics::CTextMetrics()
{
	//$ 適当な仮値で初期化。実際には使う側でSet～を呼ぶので、これらの仮値が参照されることは無い。
	SetHankakuWidth(10);
	SetHankakuHeight(18);
	SetHankakuDx(12);
	SetHankakuDy(24);
}

CTextMetrics::~CTextMetrics()
{
}

void CTextMetrics::CopyTextMetricsStatus(CTextMetrics* pDst) const
{
	pDst->SetHankakuWidth			(GetHankakuWidth());		/* 半角文字の幅 */
	pDst->SetHankakuHeight			(GetHankakuHeight());		/* 文字の高さ */
	pDst->m_aFontHeightMargin = m_aFontHeightMargin;
}

/*
	文字の大きさを調べる
	
	※ビルド種により、微妙にサイズが変わるようでした。
	　サイズを合わせるため、適当な文字で調整。
*/
void CTextMetrics::Update(HDC hdc, HFONT hFont, int nLineSpace, int nColmSpace)
{
	int size = 1; //暫定
	HFONT hFontArray[1] = { hFont };

	this->SetHankakuHeight(1);
	this->SetHankakuWidth(1);
	int tmAscent[1];
	int tmAscentMaxHeight;
	m_aFontHeightMargin.resize(size);
	for( int i = 0; i < size; i++ ){
		HFONT hFontOld = (HFONT)::SelectObject( hdc, hFontArray[i] );
 		SIZE  sz;
		// LocalCache::m_han_size と一致していなければならない
		{
			// KB145994
			// tmAveCharWidth は不正確(半角か全角なのかも不明な値を返す)
			// ただしこのコードはカーニングの影響を受ける
			GetTextExtentPoint32W_AnyBuild(hdc, L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &sz);
			sz.cx = (sz.cx / 26 + 1) / 2;
		}
		TEXTMETRIC tm;
		GetTextMetrics(hdc, &tm);
		if( GetHankakuHeight() < sz.cy ){
			SetHankakuHeight(sz.cy);
			tmAscentMaxHeight = tm.tmAscent;
		}
		if( i == 0 && GetHankakuWidth() < sz.cx ){
			SetHankakuWidth(sz.cx);
		}
		tmAscent[i] = tm.tmAscent;
		::SelectObject( hdc, hFontOld );
	}
	int minMargin = 0;
	for(int i = 0; i < size; i++){
		if( tmAscentMaxHeight - tmAscent[i] < minMargin ){
			minMargin = tmAscentMaxHeight - tmAscent[i];
		}
	}
	if( minMargin < 0 ){
		minMargin *= -1;
		SetHankakuHeight( GetHankakuHeight() + minMargin );
	}
	int nOrgHeight = GetHankakuHeight();
	if( nLineSpace < 0 ){
		// マイナスの場合は文字の高さも引く
		SetHankakuHeight( std::max(1, GetHankakuHeight() + nLineSpace) );
	}
	for( int i = 0; i < size; i++ ){
		m_aFontHeightMargin[i] = tmAscentMaxHeight - tmAscent[i] + minMargin;
	}
	
	// Dx/Dyも設定
	SetHankakuDx( GetHankakuWidth() + nColmSpace );
	SetHankakuDy( std::max(1, nOrgHeight + nLineSpace) );
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           設定                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextMetrics::SetHankakuWidth(int nHankakuWidth)
{
	m_nCharWidth=nHankakuWidth;
}

//! 半角文字の縦幅を設定。単位はピクセル。
void CTextMetrics::SetHankakuHeight(int nHankakuHeight)
{
	m_nCharHeight=nHankakuHeight;
}


//!文字間隔基準設定。nDxBasisは半角文字の基準ピクセル幅。SetHankakuDx
void CTextMetrics::SetHankakuDx(int nDxBasis)
{
	m_nDxBasis=nDxBasis;
	for(int i=0;i<_countof(m_anHankakuDx);i++)m_anHankakuDx[i]=GetHankakuDx();
	for(int i=0;i<_countof(m_anZenkakuDx);i++)m_anZenkakuDx[i]=GetZenkakuDx();
}
void CTextMetrics::SetHankakuDy(int nDyBasis)
{
	m_nDyBasis=nDyBasis;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           取得                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 指定した文字列により文字間隔配列を生成する。
const int* CTextMetrics::GenerateDxArray(
	std::vector<int>* vResultArray, //!< [out] 文字間隔配列の受け取りコンテナ
	const wchar_t* pText,           //!< [in]  文字列
	int nLength,                    //!< [in]  文字列長
	int	nHankakuDx,					//!< [in]  半角文字の文字間隔
	int	nTabSpace,					//   [in]  TAB幅(CLayoutXInt)
	int	nIndent,					//   [in]  インデント(TAB対応用)(CLayoutXInt)
	int nCharSpacing				//!< [in]  文字隙間
)
{

	vResultArray->resize(nLength);
	if(!pText || nLength<=0)return NULL;

	int* p=&(*vResultArray)[0];
	int	 nLayoutCnt = nIndent;
	const wchar_t* x=pText;
	for (int i=0; i<nLength; i++, p++, x++) {
		// サロゲートチェック
		if (*x == WCODE::TAB) {
			// TAB対応	2013/5/7 Uchi
			if (i > 0 && *(x-1) == WCODE::TAB) {
				*p = nTabSpace;
				nLayoutCnt += *p;
			}
			else {
				*p = (nTabSpace + nHankakuDx - 1) - ((nLayoutCnt + nHankakuDx - 1) % nTabSpace);
				nLayoutCnt += *p;
			}
		}else
		if(IsUTF16High(*x)){
			if(i+1 < nLength && IsUTF16Low(x[1])){
				int n = 0;
				if(nCharSpacing){
					n = CNativeW::GetKetaOfChar(pText, nLength, i) * nCharSpacing;
				}
				*p = WCODE::CalcPxWidthByFont2(x) + n;
				p++;
				x++;
				i++;
				*p = 0;
			}else{
				int n = 0;
				if(nCharSpacing){
					n = CNativeW::GetKetaOfChar(pText, nLength, i) * nCharSpacing;
				}
				*p = WCODE::CalcPxWidthByFont(*x) + n;
				nLayoutCnt += *p;
			}
		}else{
			int n = 0;
			if(nCharSpacing){
				n = CNativeW::GetKetaOfChar(pText, nLength, i) * nCharSpacing;
			}
			*p = WCODE::CalcPxWidthByFont(*x) + n;
			nLayoutCnt += *p;
		}
	}

	if(vResultArray->size())
		return &(*vResultArray)[0];
	else
		return NULL;
}

//!文字列のピクセル幅を返す。
int CTextMetrics::CalcTextWidth(
	const wchar_t* pText, //!< 文字列
	int nLength,          //!< 文字列長
	const int* pnDx       //!< 文字間隔の入った配列
)
{
	//ANSI時代の動作 ※pnDxにはすべて同じ値が入っていた
	//return pnDx[0] * nLength;

	//UNICODE時代の動作
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
	int nCharSpacing      //!< 文字の隙間
)
{
	//文字間隔配列を生成
	vector<int> vDxArray;
	const int* pDxArray = CTextMetrics::GenerateDxArray(
		&vDxArray,
		pText,
		nLength,
		nHankakuDx,
		8,
		0,
		nCharSpacing
	);

	//ピクセル幅を計算
	return CalcTextWidth(pText, nLength, pDxArray);
}

int CTextMetrics::CalcTextWidth3(
	const wchar_t* pText, //!< 文字列
	int nLength          //!< 文字列長
) const
{
	return CalcTextWidth2(pText, nLength, GetCharPxWidth(), GetCharSpacing());
}
