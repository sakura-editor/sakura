#pragma once

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
