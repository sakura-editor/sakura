/*!	@file
	@brief �t�@�C���^�C�v�ꗗ�_�C�A���O

	@author Norio Nakatani
	@date 1998/12/23 �V�K�쐬
	@date 1999/12/05 �č쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgTypeList;

#ifndef _CDLGTYPELIST_H_
#define _CDLGTYPELIST_H_

#include "dlg/CDialog.h"
using std::wstring;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �t�@�C���^�C�v�ꗗ�_�C�A���O
*/
class CDlgTypeList : public CDialog
{
public:
	// �^
	struct SResult{
		CTypeConfig	cDocumentType;	//!< �������
		bool			bTempChange;	//!< ��PROP_TEMPCHANGE_FLAG
	};

public:
	// �C���^�[�t�F�[�X
	int DoModal( HINSTANCE, HWND, SResult* );	/* ���[�_���_�C�A���O�̕\�� */

protected:
	// �����w���p�֐�
	BOOL OnLbnDblclk( int );
	BOOL OnBnClicked( int );
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );
	void SetData();	/* �_�C�A���O�f�[�^�̐ݒ� */
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	bool Import( void );			// 2010/4/12 Uchi
	bool Export( void );			// 2010/4/12 Uchi
	bool InitializeType( void );	// 2010/4/12 Uchi
	bool AlertFileAssociation();	// 2011/8/20 syat

private:
	CTypeConfig				m_nSettingType;
	// �֘A�t�����
	bool m_bRegistryChecked[ MAX_TYPES ];	//���W�X�g���m�F ���^��
	bool m_bExtRMenu[ MAX_TYPES ];			//�E�N���b�N�o�^ ���^��
	bool m_bExtDblClick[ MAX_TYPES ];		//�_�u���N���b�N ���^��
	bool m_bAlertFileAssociation;			//�֘A�t���x���̕\���t���O
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGTYPELIST_H_ */



