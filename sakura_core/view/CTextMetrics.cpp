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
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CTextMetrics::CTextMetrics()
{
	//$ �K���ȉ��l�ŏ������B���ۂɂ͎g������Set�`���ĂԂ̂ŁA�����̉��l���Q�Ƃ���邱�Ƃ͖����B
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
	pDst->SetHankakuWidth			(GetHankakuWidth());		/* ���p�����̕� */
	pDst->SetHankakuHeight			(GetHankakuHeight());		/* �����̍��� */
	pDst->m_aFontHeightMargin = m_aFontHeightMargin;
}

/*
	�����̑傫���𒲂ׂ�
	
	���r���h��ɂ��A�����ɃT�C�Y���ς��悤�ł����B
	�@�T�C�Y�����킹�邽�߁A�K���ȕ����Œ����B
*/
void CTextMetrics::Update(HDC hdc, HFONT hFont, int nLineSpace, int nColmSpace)
{
	int size = 1; //�b��
	HFONT hFontArray[1] = { hFont };

	this->SetHankakuHeight(1);
	this->SetHankakuWidth(1);
	int tmAscent[1];
	int tmAscentMaxHeight;
	m_aFontHeightMargin.resize(size);
	for( int i = 0; i < size; i++ ){
		HFONT hFontOld = (HFONT)::SelectObject( hdc, hFontArray[i] );
 		SIZE  sz;
		// LocalCache::m_han_size �ƈ�v���Ă��Ȃ���΂Ȃ�Ȃ�
		{
			// KB145994
			// tmAveCharWidth �͕s���m(���p���S�p�Ȃ̂����s���Ȓl��Ԃ�)
			// ���������̃R�[�h�̓J�[�j���O�̉e�����󂯂�
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
		// �}�C�i�X�̏ꍇ�͕����̍���������
		SetHankakuHeight( std::max(1, GetHankakuHeight() + nLineSpace) );
	}
	for( int i = 0; i < size; i++ ){
		m_aFontHeightMargin[i] = tmAscentMaxHeight - tmAscent[i] + minMargin;
	}
	
	// Dx/Dy���ݒ�
	SetHankakuDx( GetHankakuWidth() + nColmSpace );
	SetHankakuDy( std::max(1, nOrgHeight + nLineSpace) );
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �ݒ�                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextMetrics::SetHankakuWidth(int nHankakuWidth)
{
	m_nCharWidth=nHankakuWidth;
}

//! ���p�����̏c����ݒ�B�P�ʂ̓s�N�Z���B
void CTextMetrics::SetHankakuHeight(int nHankakuHeight)
{
	m_nCharHeight=nHankakuHeight;
}


//!�����Ԋu��ݒ�BnDxBasis�͔��p�����̊�s�N�Z�����BSetHankakuDx
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
//                           �擾                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �w�肵��������ɂ�蕶���Ԋu�z��𐶐�����B
const int* CTextMetrics::GenerateDxArray(
	std::vector<int>* vResultArray, //!< [out] �����Ԋu�z��̎󂯎��R���e�i
	const wchar_t* pText,           //!< [in]  ������
	int nLength,                    //!< [in]  ������
	int	nHankakuDx,					//!< [in]  ���p�����̕����Ԋu
	int	nTabSpace,					//   [in]  TAB��(CLayoutXInt)
	int	nIndent,					//   [in]  �C���f���g(TAB�Ή��p)(CLayoutXInt)
	int nCharSpacing				//!< [in]  ��������
)
{

	vResultArray->resize(nLength);
	if(!pText || nLength<=0)return NULL;

	int* p=&(*vResultArray)[0];
	int	 nLayoutCnt = nIndent;
	const wchar_t* x=pText;
	for (int i=0; i<nLength; i++, p++, x++) {
		// �T���Q�[�g�`�F�b�N
		if (*x == WCODE::TAB) {
			// TAB�Ή�	2013/5/7 Uchi
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

//!������̃s�N�Z������Ԃ��B
int CTextMetrics::CalcTextWidth(
	const wchar_t* pText, //!< ������
	int nLength,          //!< ������
	const int* pnDx       //!< �����Ԋu�̓������z��
)
{
	//ANSI����̓��� ��pnDx�ɂ͂��ׂē����l�������Ă���
	//return pnDx[0] * nLength;

	//UNICODE����̓���
	int w=0;
	for(int i=0;i<nLength;i++){
		w+=pnDx[i];
	}
	return w;
}

//!������̃s�N�Z������Ԃ��B
int CTextMetrics::CalcTextWidth2(
	const wchar_t* pText, //!< ������
	int nLength,          //!< ������
	int nHankakuDx,       //!< ���p�����̕����Ԋu
	int nCharSpacing      //!< �����̌���
)
{
	//�����Ԋu�z��𐶐�
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

	//�s�N�Z�������v�Z
	return CalcTextWidth(pText, nLength, pDxArray);
}

int CTextMetrics::CalcTextWidth3(
	const wchar_t* pText, //!< ������
	int nLength          //!< ������
) const
{
	return CalcTextWidth2(pText, nLength, GetCharPxWidth(), GetCharSpacing());
}
