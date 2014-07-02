/*!	@file
	@brief �����_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/12/02 �č쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "dlg/CDialog.h"
#include "recent/CRecentSearch.h"
#include "util/window.h"
#ifndef SAKURA_CDLGFIND_H_
#define SAKURA_CDLGFIND_H_


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDlgFind : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgFind();
	/*
	||  Attributes & Operations
	*/
//	int DoModal( HINSTANCE, HWND, LPARAM );	/* ���[�_���_�C�A���O�̕\�� */
	HWND DoModeless( HINSTANCE, HWND, LPARAM );	/* ���[�h���X�_�C�A���O�̕\�� */

	void ChangeView( LPARAM );

	SSearchOption m_sSearchOption;	// �����I�v�V����
	int		m_bNOTIFYNOTFOUND;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	std::wstring	m_strText;	// ����������

	CLogicPoint	m_ptEscCaretPos_PHY;	// �����J�n���̃J�[�\���ʒu�ޔ��G���A

	CRecentSearch			m_cRecentSearch;
	SComboBoxItemDeleter	m_comboDel;
	CFontAutoDeleter		m_cFontText;

protected:
//@@@ 2002.2.2 YAZAKI CShareData�Ɉړ�
//	void AddToSearchKeyArr( const char* );
	/* �I�[�o�[���C�h? */
	BOOL OnCbnDropDown( HWND hwndCtl, int wID );
	int GetData( void );		/* �_�C�A���O�f�[�^�̎擾 */
	void SetCombosList( void );	/* ����������/�u���㕶���񃊃X�g�̐ݒ� */
	void SetData( void );		/* �_�C�A���O�f�[�^�̐ݒ� */
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnDestroy();
	BOOL OnBnClicked( int );
	BOOL OnActivate( WPARAM wParam, LPARAM lParam );	// 2009.11.29 ryoji

	// virtual BOOL OnKeyDown( WPARAM wParam, LPARAM lParam );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_CDLGFIND_H_ */



