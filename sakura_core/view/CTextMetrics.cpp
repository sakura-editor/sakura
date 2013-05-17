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
}

/*
	�����̑傫���𒲂ׂ�
	
	���r���h��ɂ��A�����ɃT�C�Y���ς��悤�ł����B
	�@�T�C�Y�����킹�邽�߁A�K���ȕ����Œ����B
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
		::GetTextExtentPoint32( hdc, "��", 2, &sz );
#endif
		this->SetHankakuHeight(sz.cy);
		this->SetHankakuWidth(sz.cx / 2);
		::SelectObject( hdc, hFontOld );
	}
	ReleaseDC(NULL,hdc);
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
	int	nTabSpace,					//   [in]  TAB��
	int	nIndent						//   [in]  �C���f���g(TAB�Ή��p)
)
{
	bool bHigh;				// �T���Q�[�g�y�A�i��ʁj

	vResultArray->resize(nLength);
	if(!pText || nLength<=0)return NULL;

	int* p=&(*vResultArray)[0];
	int	 nLayoutCnt = nIndent;
	const wchar_t* q=pText;
	bHigh = false;
	for (int i=0; i<nLength; i++, p++, q++) {
		if (*q == WCODE::TAB) {
			// TAB�Ή�	2013/5/7 Uchi
			if (i > 0 && *(q-1) == WCODE::TAB) {
				*p = nTabSpace * nHankakuDx;
				nLayoutCnt += nTabSpace;
			}
			else {
				*p = (nTabSpace - nLayoutCnt % nTabSpace) * nHankakuDx;
				nLayoutCnt += (nTabSpace - nLayoutCnt % nTabSpace);
			}
			bHigh = false;
		}
		// �T���Q�[�g�`�F�b�N BMP �ȊO�͑S�p����	2008/7/5 Uchi
		else if (IsUTF16High(*q)) {
			*p = nHankakuDx*2;
			nLayoutCnt += 2;
			bHigh = true;
		}
		else if (IsUTF16Low(*q)) {
			// �T���Q�[�g�y�A�i���ʁj�P�Ƃ̏ꍇ�͑S�p����
			//*p = (bHigh) ? 0 : nHankakuDx*2;
			if (bHigh) {
				*p = 0;
			}
			else{
				if (IsBinaryOnSurrogate(*q)) {
					*p = nHankakuDx;
					nLayoutCnt++;
				}
				else{
					*p = nHankakuDx*2;
					nLayoutCnt += 2;
				}
			}
			bHigh = false;
		}
		else if(WCODE::IsHankaku(*q)){
			*p = nHankakuDx;
			nLayoutCnt++;
			bHigh = false;				// �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
		}
		else{
			*p = nHankakuDx*2;
			nLayoutCnt += 2;
			bHigh = false;				// �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
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
	int nHankakuDx        //!< ���p�����̕����Ԋu
)
{
	//�����Ԋu�z��𐶐�
	vector<int> vDxArray;
	const int* pDxArray = CTextMetrics::GenerateDxArray(
		&vDxArray,
		pText,
		nLength,
		nHankakuDx
	);

	//�s�N�Z�������v�Z
	return CalcTextWidth(pText, nLength, pDxArray);
}

