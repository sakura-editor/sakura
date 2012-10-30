/*!	@file
	@brief �u���_�C�A���O

	@author Norio Nakatani
	@date 1998/10/02  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, hor
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgReplace;

#ifndef _CDLGREPLACE_H_
#define _CDLGREPLACE_H_

#include "dlg/CDialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �u���_�C�A���O�{�b�N�X
*/
class SAKURA_CORE_API CDlgReplace : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgReplace();
	/*
	||  Attributes & Operations
	*/
	HWND DoModeless( HINSTANCE, HWND, LPARAM, BOOL );	/* ���[�_���_�C�A���O�̕\�� */
	void ChangeView( LPARAM );	/* ���[�h���X���F�u���E�����ΏۂƂȂ�r���[�̕ύX */

	SSearchOption	m_sSearchOption;	// �����I�v�V����
	/*
	int				m_bLoHiCase;	// �p�啶���Ɖp����������ʂ���
	int				m_bWordOnly;	// ��v����P��̂݌�������
	int				m_bRegularExp;	// ���K�\��
	*/
	int				m_bConsecutiveAll;	/* �u���ׂĒu���v�͒u���̌J�Ԃ� */	// 2007.01.16 ryoji
	std::wstring	m_strText;	// ����������
	std::wstring	m_strText2;	// �u���㕶����
	int				m_nReplaceKeySequence;	//�u����V�[�P���X
	BOOL			m_bSelectedArea;	/* �I��͈͓��u�� */
	int				m_bNOTIFYNOTFOUND;				/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	BOOL			m_bSelected;	/* �e�L�X�g�I�𒆂� */
	int				m_nReplaceTarget;	/* �u���Ώ� */	// 2001.12.03 hor
	int				m_nPaste;			/* �\��t���H */	// 2001.12.03 hor
	int				m_nReplaceCnt;		//���ׂĒu���̎��s����		// 2002.02.08 hor
	bool			m_bCanceled;		//���ׂĒu���Œ��f������	// 2002.02.08 hor

	CLogicPoint		m_ptEscCaretPos_PHY;	// ����/�u���J�n���̃J�[�\���ʒu�ޔ��G���A

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	BOOL OnActivate( WPARAM wParam, LPARAM lParam );	// 2009.11.29 ryoji
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData( void );		/* �_�C�A���O�f�[�^�̐ݒ� */
	void SetCombosList( void );	/* ����������/�u���㕶���񃊃X�g�̐ݒ� */
	int GetData( void );		/* �_�C�A���O�f�[�^�̎擾 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGREPLACE_H_ */



