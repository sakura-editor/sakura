#include "stdafx.h"
#include "CTextMetrics.h"
#include "charcode.h"
#include <vector>
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
}

/*
	文字の大きさを調べる
	
	※ビルド種により、微妙にサイズが変わるようでした。
	　サイズを合わせるため、適当な文字で調整。
*/
void CTextMetrics::Update(HFONT hFont)
{
	HDC hdc = GetDC(NULL);
	{
		HFONT hFontOld = (HFONT)::SelectObject( hdc, hFont );
		SIZE  sz;
#ifdef _UNICODE
		::GetTextExtentPoint32( hdc, L"xx", 2, &sz );
#else
		::GetTextExtentPoint32( hdc, "大", 2, &sz );
#endif
		this->SetHankakuHeight(sz.cy);
		this->SetHankakuWidth(sz.cx / 2);
		::SelectObject( hdc, hFontOld );
	}
	ReleaseDC(NULL,hdc);
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
	int nHankakuDx                  //!< [in]  半角文字の文字間隔
)
{
	using namespace WCODE;

	vResultArray->resize(nLength);
	if(!pText || nLength<=0)return NULL;

	int* p=&(*vResultArray)[0];
	const wchar_t* q=pText;
	for(int i=0;i<nLength;i++){
		if(isHankaku(*q)){
			*p = nHankakuDx;
		}
		else{
			*p = nHankakuDx*2;
		}
		p++;
		q++;
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
	int nHankakuDx        //!< 半角文字の文字間隔
)
{
	//文字間隔配列を生成
	vector<int> vDxArray;
	const int* pDxArray = CTextMetrics::GenerateDxArray(
		&vDxArray,
		pText,
		nLength,
		nHankakuDx
	);

	//ピクセル幅を計算
	return CalcTextWidth(pText, nLength, pDxArray);
}

