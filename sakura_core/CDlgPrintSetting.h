//	$Id$
/*!	@file
	@brief ����ݒ�_�C�A���O

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgPrintSetting;

#ifndef _CDLGPRINTSETTING_H_
#define _CDLGPRINTSETTING_H_

#include "CDialog.h"
//#include <windows.h>
//#include "CShareData.h"

//! ����ݒ�_�C�A���O
class CDlgPrintSetting : public CDialog
{
public:
	/*
	||  Constructors
	*/
//	CDlgPrintSetting();
//	~CDlgPrintSetting();
//	void Create( HINSTANCE, HWND );	/* ������ */

	/*
	||  Attributes & Operations
	*/
//	BOOL DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */
//	int DoModal( int*, PRINTSETTING* );	/* ���[�_���_�C�A���O�̕\�� */
	int DoModal( HINSTANCE, HWND, int*, PRINTSETTING* );	/* ���[�_���_�C�A���O�̕\�� */

//	HINSTANCE		m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
//	HWND			m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
//	HWND			m_hWnd;			/* ���̃_�C�A���O�̃n���h�� */
//	CShareData		m_cShareData;
//	DLLSHAREDATA*	m_pShareData;
//	char			m_szHelpFile[_MAX_PATH + 1];
	int				m_nCurrentPrintSetting;
	PRINTSETTING	m_PrintSettingArr[MAX_PRINTSETTINGARR];


protected:
	/*
	||  �����w���p�֐�
	*/
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnDestroy( void );
	BOOL OnNotify( WPARAM,  LPARAM );
	BOOL OnCbnSelChange( HWND, int );
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void OnChangeSettingType( BOOL );	/* �ݒ�̃^�C�v���ς���� */
	void OnSpin( int , BOOL );	/* �X�s���R���g���[���̏��� */
	int DataCheckAndCrrect( int , int );	/* ���͒l(���l)�̃G���[�`�F�b�N�����Đ������l��Ԃ� */
	BOOL OnTimer( WPARAM );	/* �^�C�}�[���� */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGPRINTSETTING_H_ */


/*[EOF]*/
