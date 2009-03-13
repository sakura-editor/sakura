#include "stdafx.h"
#include "CTextMetrics.h"
//#include "charset/charcode.h"
#include "charset/codechecker.h"
#include <vector>
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
	int nHankakuDx                  //!< [in]  ���p�����̕����Ԋu
)
{
	using namespace WCODE;
	bool bHigh;				// �T���Q�[�g�y�A�i��ʁj

	vResultArray->resize(nLength);
	if(!pText || nLength<=0)return NULL;

	int* p=&(*vResultArray)[0];
	const wchar_t* q=pText;
	bHigh = false;
	for(int i=0;i<nLength;i++){
		// �T���Q�[�g�`�F�b�N BMP �ȊO�͑S�p����	2008/7/5 Uchi
		if (IsUTF16High(*q)) {
			*p = nHankakuDx*2;
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
				}
				else{
					*p = nHankakuDx*2;
				}
			}
			bHigh = false;
		}
		else  if(IsHankaku(*q)){
			*p = nHankakuDx;
			bHigh = false;				// �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
		}
		else{
			*p = nHankakuDx*2;
			bHigh = false;				// �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
		}
		p++;
		q++;
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

