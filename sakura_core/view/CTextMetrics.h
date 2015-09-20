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
#ifndef SAKURA_CTEXTMETRICS_815A7F9E_8E38_4FA5_9F68_7CA776A18F1F_H_
#define SAKURA_CTEXTMETRICS_815A7F9E_8E38_4FA5_9F68_7CA776A18F1F_H_

//2007.08.25 kobake �ǉ�

#include <vector>

class CTextMetrics;

class CTextMetrics{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CTextMetrics();
	virtual ~CTextMetrics();
	void CopyTextMetricsStatus(CTextMetrics* pDst) const;
	void Update(HDC hdc, HFONT hFont, int nLineSpace, int nColmSpace);

	//�ݒ�
private:
	void SetHankakuWidth(int nHankakuWidth);   //!< ���p�����̕���ݒ�B�P�ʂ̓s�N�Z���B
	void SetHankakuHeight(int nHankakuHeight); //!< ���p�����̏c����ݒ�B�P�ʂ̓s�N�Z���B
	void SetHankakuDx(int nHankakuDx);         //!< ���p�����̕����Ԋu��ݒ�B�P�ʂ̓s�N�Z���B
	void SetHankakuDy(int nHankakuDy);         //!< ���p�����̍s�Ԋu��ݒ�B�P�ʂ̓s�N�Z���B

	//�擾
public:
	int GetHankakuWidth() const{ return m_nCharWidth; }		//!< ���p�����̉������擾�B�P�ʂ̓s�N�Z���B
	int GetHankakuHeight() const{ return m_nCharHeight; }	//!< ���p�����̏c�����擾�B�P�ʂ̓s�N�Z���B
	int GetHankakuDx() const{ return m_nDxBasis; }			//!< ���p�����̕����Ԋu���擾�B�P�ʂ̓s�N�Z���B
	int GetZenkakuDx() const{ return m_nDxBasis*2; }		//!< �S�p�����̕����Ԋu���擾�B�P�ʂ̓s�N�Z���B
	int GetHankakuDy() const{ return m_nDyBasis; }			//!< Y���������Ԋu�B�����c���{�s�Ԋu�B�P�ʂ̓s�N�Z���B

	CPixelXInt GetCharSpacing() const {
		return GetHankakuDx() - GetHankakuWidth();
	}
	// ���C�A�E�g�����̃s�N�Z�������擾����
	int GetCharPxWidth(CLayoutXInt col) const{
		return (Int)col;
	}

	int GetCharPxWidth() const{
		return 1;
	}
	int GetCharHeightMarginByFontNo(int n) const{
		return m_aFontHeightMargin[n];
	}

	// �Œ蕶��x���̃��C�A�E�g�����擾����
	CLayoutXInt GetLayoutXDefault(CKetaXInt chars) const{
		return CLayoutXInt(GetHankakuDx() * (Int)chars);
	}
	// �Œ蕶��1��������̃��C�A�E�g�����擾����
	CLayoutXInt GetLayoutXDefault() const{ return GetLayoutXDefault(CKetaXInt(1));}
	

	//�����Ԋu�z����擾
	const int* GetDxArray_AllHankaku() const{ return m_anHankakuDx; } //!<���p������̕����Ԋu�z����擾�B�v�f����64�B
	const int* GetDxArray_AllZenkaku() const{ return m_anZenkakuDx; } //!<���p������̕����Ԋu�z����擾�B�v�f����64�B

	const int* GenerateDxArray2(
		std::vector<int>* vResultArray, //!< [out] �����Ԋu�z��̎󂯎��R���e�i
		const wchar_t* pText,           //!< [in]  ������
		int nLength                     //!< [in]  ������
	) const {
		return GenerateDxArray(vResultArray, pText, nLength, GetHankakuDx(), 8, 0, GetCharSpacing());
	}

	//! �w�肵��������ɂ�蕶���Ԋu�z��𐶐�����B
	static const int* GenerateDxArray(
		std::vector<int>* vResultArray, //!< [out] �����Ԋu�z��̎󂯎��R���e�i
		const wchar_t* pText,           //!< [in]  ������
		int nLength,                    //!< [in]  ������
		int	nHankakuDx,					//!< [in]  ���p�����̕����Ԋu
		int	nTabSpace = 8,				//   [in]  TAB��
		int	nIndent = 0,				//   [in]  �C���f���g
		int nCharSpacing = 0			//   [in]  �����̊Ԋu
	);

	//!������̃s�N�Z������Ԃ��B
	static int CalcTextWidth(
		const wchar_t* pText, //!< ������
		int nLength,          //!< ������
		const int* pnDx       //!< �����Ԋu�̓������z��
	);

	//!������̃s�N�Z������Ԃ��B
	static int CalcTextWidth2(
		const wchar_t* pText, //!< ������
		int nLength,          //!< ������
		int nHankakuDx,       //!< ���p�����̕����Ԋu
		int nCharSpacing
	);

	int CalcTextWidth3(
		const wchar_t* pText, //!< ������
		int nLength           //!< ������
	) const;

private:
//	HDC m_hdc; //!< �v�Z�ɗp����f�o�C�X�R���e�L�X�g
	int	m_nCharWidth;      //!< ���p�����̉���
	int m_nCharHeight;     //!< ���p�����̏c��
	int m_nDxBasis;        //!< ���p�����̕����Ԋu (����+��)
	int m_nDyBasis;        //!< ���p�����̍s�Ԋu (�c��+��)
	int m_anHankakuDx[64]; //!< ���p�p�����Ԋu�z��
	int m_anZenkakuDx[64]; //!< �S�p�p�����Ԋu�z��
	std::vector<int> m_aFontHeightMargin;
};

#endif /* SAKURA_CTEXTMETRICS_815A7F9E_8E38_4FA5_9F68_7CA776A18F1F_H_ */
/*[EOF]*/
