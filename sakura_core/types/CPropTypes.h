/*!	@file
	@brief �^�C�v�ʐݒ�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998/05/08  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, MIK, asa-o
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, genta
	Copyright (C) 2005, MIK, aroka, genta
	Copyright (C) 2006, fon

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

class CPropTypes;

#ifndef _CPROPTYPES_H_
#define _CPROPTYPES_H_

#include <windows.h>
#include "CShareData.h"

/*-----------------------------------------------------------------------
�萔
-----------------------------------------------------------------------*/
//#define STR_COLORDATA_HEAD2	" �e�L�X�g�G�f�B�^�F�ݒ� Ver2"
//#define STR_COLORDATA_HEAD21	" �e�L�X�g�G�f�B�^�F�ݒ� Ver2.1"	//Nov. 2, 2000 JEPRO �ύX [��]. 0.3.9.0:ur3��10�ȍ~�A�ݒ荀�ڂ̔ԍ������ւ�������
#define STR_COLORDATA_HEAD3	" �e�L�X�g�G�f�B�^�F�ݒ� Ver3"		//Jan. 15, 2001 Stonee  �F�ݒ�Ver3�h���t�g(�ݒ�t�@�C���̃L�[��A�ԁ��������)	//Feb. 11, 2001 JEPRO �L���ɂ���
//#define STR_COLORDATA_HEAD4		" �e�L�X�g�G�f�B�^�F�ݒ� Ver4"		//2007.10.02 kobake UNICODE���ɍۂ��ăJ���[�t�@�C���d�l���ύX
#define STR_COLORDATA_SECTION	"SakuraColor"


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �^�C�v�ʐݒ�_�C�A���O�{�b�N�X

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class SAKURA_CORE_API CPropTypes{
public:
	//�����Ɣj��
	CPropTypes();
	~CPropTypes();
	void Create( HINSTANCE, HWND );	//!< ������
	int DoPropertySheet( int );		//!< �v���p�e�B�V�[�g�̍쐬

	//�C���^�[�t�F�[�X	
	void SetTypeData( const STypeConfig& t ){ m_Types = t; }	//!< �^�C�v�ʐݒ�f�[�^�̐ݒ�  Jan. 23, 2005 genta
	void GetTypeData( STypeConfig& t ) const { t = m_Types; }	//!< �^�C�v�ʐݒ�f�[�^�̎擾  Jan. 23, 2005 genta

protected:
	//�C�x���g
	void OnHelp( HWND , int );	//!< �w���v

private:
	//�e��Q��
	HINSTANCE	m_hInstance;	//!< �A�v���P�[�V�����C���X�^���X�̃n���h��
	HWND		m_hwndParent;	//!< �I�[�i�[�E�B���h�E�̃n���h��
	HWND		m_hwndThis;		//!< ���̃_�C�A���O�̃n���h��
	SFilePath	m_szHelpFile;

	//�_�C�A���O�f�[�^
	int			m_nPageNum;
	STypeConfig		m_Types;


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �X�N���[��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent_Screen( HWND, UINT, WPARAM, LPARAM );		//!< p1 ���b�Z�[�W����
protected:
	void SetData_p1( HWND );	//!< �_�C�A���O�f�[�^�̐ݒ� p1
	int  GetData_p1( HWND );	//!< �_�C�A���O�f�[�^�̎擾 p1


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          �J���[                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent_Color( HWND, UINT, WPARAM, LPARAM );		//!< p3 ���b�Z�[�W����
protected:
	void SetData_Color( HWND );								//!< �_�C�A���O�f�[�^�̐ݒ� p3
	int  GetData_Color( HWND );								//!< �_�C�A���O�f�[�^�̎擾 p3
	void _Import_Colors( HWND );							//!< �F�̐ݒ���C���|�[�g
	void _Export_Colors( HWND );							//!< �F�̐ݒ���G�N�X�|�[�g
	void DrawColorListItem( DRAWITEMSTRUCT*);				//!< �F��ʃ��X�g �I�[�i�[�`��
	void EnableTypesPropInput( HWND hwndDlg );				//!< �^�C�v�ʐݒ�̃J���[�ݒ��ON/OFF
	void RearrangeKeywordSet( HWND );						//!< �L�[���[�h�Z�b�g�Ĕz�u  Jan. 23, 2005 genta
	void _DrawColorButton( DRAWITEMSTRUCT* , COLORREF );	//!< �F�{�^���̕`��
public:
	static BOOL SelectColor( HWND , COLORREF*, DWORD* );	//!< �F�I���_�C�A���O
private:
	DWORD			m_dwCustColors[16];						//!< �t�H���gDialog�J�X�^���p���b�g
	int				m_nSet[ MAX_KEYWORDSET_PER_TYPE ];		//!< keyword set index  2005.01.13 MIK
	int				m_nCurrentColorType;					//!< ���ݑI������Ă���F�^�C�v
	CKeyWordSetMgr*	m_pCKeyWordSetMgr;						//!< �������팸�̂��߃|�C���^��  Mar. 31, 2003 genta


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �x��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent_Support( HWND, UINT, WPARAM, LPARAM );	//!< p2 ���b�Z�[�W���� �x���^�u // 2001/06/14 asa-o
protected:
	void SetData_Support( HWND );	//!< �_�C�A���O�f�[�^�̐ݒ� p2 �x���^�u
	int  GetData_Support( HWND );	//!< �_�C�A���O�f�[�^�̎擾 p2 �x���^�u


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                    ���K�\���L�[���[�h                       //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent_Regex( HWND, UINT, WPARAM, LPARAM );		//!< ���b�Z�[�W���� ���K�\���L�[���[�h	//@@@ 2001.11.17 add MIK
protected:
	void SetData_Regex( HWND );	//!< �_�C�A���O�f�[�^�̐ݒ� ���K�\���L�[���[�h	//@@@ 2001.11.17 add MIK
	int  GetData_Regex( HWND );	//!< �_�C�A���O�f�[�^�̎擾 ���K�\���L�[���[�h	//@@@ 2001.11.17 add MIK
	BOOL Import_Regex( HWND );	//@@@ 2001.11.17 add MIK
	BOOL Export_Regex( HWND );	//@@@ 2001.11.17 add MIK
	BOOL RegexKakomiCheck(const wchar_t *s);	//@@@ 2001.11.17 add MIK


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     �L�[���[�h�w���v                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent_KeyHelp( HWND, UINT, WPARAM, LPARAM );	//!< ���b�Z�[�W���� �L�[���[�h�����I��	//@@@ 2006.04.10 fon
protected:
	void SetData_KeyHelp( HWND );	//!< �_�C�A���O�f�[�^�̐ݒ� �L�[���[�h�����I��
	int  GetData_KeyHelp( HWND );	//!< �_�C�A���O�f�[�^�̎擾 �L�[���[�h�����I��
	BOOL Import_KeyHelp( HWND );	//@@@ 2006.04.10 fon
	BOOL Export_KeyHelp( HWND );	//@@@ 2006.04.10 fon
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPROPTYPES_H_ */



