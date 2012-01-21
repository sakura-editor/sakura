/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CTEXTAREA_BE5C17FA_E8D8_4659_9AA4_552DF90288CC9_H_
#define SAKURA_CTEXTAREA_BE5C17FA_E8D8_4659_9AA4_552DF90288CC9_H_

class CViewFont;
class CEditView;
#include "DispPos.h"


class CTextArea{
public:
	CTextArea(CEditView* pEditView);
	virtual ~CTextArea();
	void CopyTextAreaStatus(CTextArea* pDst) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     �r���[�����擾                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//!�\�������ŏ��̍s
	CLayoutInt GetViewTopLine() const
	{
		return m_nViewTopLine;
	}
	void SetViewTopLine(CLayoutInt nLine)
	{
		m_nViewTopLine=nLine;
	}

	//!�\����̈�ԍ��̌�
	CLayoutInt GetViewLeftCol() const
	{
		return m_nViewLeftCol;
	}
	void SetViewLeftCol(CLayoutInt nLeftCol)
	{
		m_nViewLeftCol=nLeftCol;
	}

	//!�E�ɂ͂ݏo�����ŏ��̗��Ԃ�
	CLayoutInt GetRightCol() const
	{
		return m_nViewLeftCol + m_nViewColNum;
	}

	//!���ɂ͂ݏo�����ŏ��̍s��Ԃ�
	CLayoutInt GetBottomLine() const
	{
		return m_nViewTopLine + m_nViewRowNum;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                   �̈���擾(�s�N�Z��)                      //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int GetAreaLeft() const
	{
		return m_nViewAlignLeft;
	}
	int GetAreaTop() const
	{
		return m_nViewAlignTop;
	}
	int GetAreaRight() const
	{
		return m_nViewAlignLeft + m_nViewCx;
	}
	int GetAreaBottom() const
	{
		return m_nViewAlignTop + m_nViewCy;
	}
	CMyRect GetAreaRect() const
	{
		return CMyRect(GetAreaLeft(), GetAreaTop(), GetAreaRight(), GetAreaBottom());
	}

	int GetAreaWidth() const
	{
		return m_nViewCx;
	}
	int GetAreaHeight() const
	{
		return m_nViewCy;
	}

	int GetTopYohaku() const
	{
		return m_nTopYohaku;
	}
	void SetTopYohaku(int nPixel)
	{
		m_nTopYohaku=nPixel;
	}
	int GetLeftYohaku() const
	{
		return m_nLeftYohaku;
	}
	void SetLeftYohaku(int nPixel)
	{
		m_nLeftYohaku=nPixel;
	}
	// �s�ԍ��̕�(�]���Ȃ�)
	int GetLineNumberWidth() const
	{
		return m_nViewAlignLeft - m_nLeftYohaku;
	}

	//! �N���C�A���g�T�C�Y�X�V
	void TextArea_OnSize(
		const CMySize& sizeClient, //!< �E�B���h�E�̃N���C�A���g�T�C�Y
		int nCxVScroll,            //!< �����X�N���[���o�[�̉���
		int nCyHScroll             //!< �����X�N���[���o�[�̏c��
	);

	//! �s�ԍ��\���ɕK�v�ȕ���ݒ�
	bool DetectWidthOfLineNumberArea( bool bRedraw );

	//! �s�ԍ��\���ɕK�v�Ȍ������v�Z
	int  DetectWidthOfLineNumberArea_calculate() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool IsRectIntersected(const RECT& rc) const
	{
		//rc�������܂��̓[���̈�̏ꍇ��false
		if( rc.left >= rc.right )return false;
		if( rc.top  >= rc.bottom )return false;

		if( rc.left >= this->GetAreaRight() )return false; //�E�O
		if( rc.right <= this->GetAreaLeft() )return false; //���O
		if( rc.top >= this->GetAreaBottom() )return false; //���O
		if( rc.bottom <= this->GetAreaTop() )return false; //��O
		
		return true;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        ���̑��擾                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int GetRulerHeight() const
	{
		return m_nViewAlignTop - GetTopYohaku();
	}
	//! �h�L�������g���[�̃N���C�A���g���W���擾 (�܂�A�X�N���[�����ꂽ��Ԃł���΁A�}�C�i�X��Ԃ�)
	int GetDocumentLeftClientPointX() const;

	//�v�Z
	//! �N���C�A���g���W���烌�C�A�E�g�ʒu�ɕϊ�����
	void ClientToLayout(CMyPoint ptClient, CLayoutPoint* pptLayout) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void UpdateAreaMetrics(HDC hdc);
	void SetAreaLeft(int nAreaLeft)
	{
		m_nViewAlignLeft = nAreaLeft;
	}
	void SetAreaTop(int nAreaTop)
	{
		m_nViewAlignTop = nAreaTop;
	}
	void OffsetViewTopLine(CLayoutInt nOff)
	{
		m_nViewTopLine += nOff;
	}
protected:
	void UpdateViewColRowNums();

public:


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �T�|�[�g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//$ Generate�Ȃ�Ă����傰���Ȗ��O����Ȃ��āAGet�`�ŗǂ��C�����Ă���
	//!�N���b�s���O��`���쐬�B�\���͈͊O�������ꍇ��false��Ԃ��B
	void GenerateCharRect(RECT* rc,const DispPos& sPos,int nHankakuNum) const;
	bool TrimRectByArea(RECT* rc) const;
	bool GenerateClipRect(RECT* rc,const DispPos& sPos,int nHankakuNum) const;
	bool GenerateClipRectRight(RECT* rc,const DispPos& sPos) const; //!< �E�[�܂őS��
	bool GenerateClipRectLine(RECT* rc,const DispPos& sPos) const;  //!< �s�S��

	void GenerateTopRect   (RECT* rc, CLayoutInt nLineCount) const;
	void GenerateBottomRect(RECT* rc, CLayoutInt nLineCount) const;
	void GenerateLeftRect  (RECT* rc, CLayoutInt nColCount ) const;
	void GenerateRightRect (RECT* rc, CLayoutInt nColCount ) const;

	void GenerateLineNumberRect(RECT* rc) const;

	void GenerateTextAreaRect(RECT* rc) const;

private:
	//�Q��
	CEditView*	m_pEditView;

public:
	/* ��ʏ�� */
	//�s�N�Z��
private:
	int		m_nViewAlignLeft;		/* �\����̍��[���W */
	int		m_nViewAlignTop;		/* �\����̏�[���W */
private:
	int		m_nTopYohaku;
	int		m_nLeftYohaku;
private:
	int		m_nViewCx;				/* �\����̕� */
	int		m_nViewCy;				/* �\����̍��� */

	//�e�L�X�g
private:
	CLayoutInt	m_nViewTopLine;			/* �\����̈�ԏ�̍s(0�J�n) */
public:
	CLayoutInt	m_nViewRowNum;			/* �\����̍s�� */

private:
	CLayoutInt	m_nViewLeftCol;			/* �\����̈�ԍ��̌�(0�J�n) */
public:
	CLayoutInt	m_nViewColNum;			/* �\����̌��� */

	//���̑�
	int		m_nViewAlignLeftCols;	/* �s�ԍ���̌��� */
};

#endif /* SAKURA_CTEXTAREA_BE5C17FA_E8D8_4659_9AA4_552DF90288CC9_H_ */
/*[EOF]*/
