//	$Id$
/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�̏���

	@author	Norio Nakatani
	@date 1998/12/24 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class CPropCommon;

#ifndef _CPROP1_H_
#define _CPROP1_H_

#include <windows.h>
#include "CShareData.h"
#include "CMemory.h"
#include "CKeyBind.h"
#include "CKeyWordSetMgr.h"
#include "CImageListMgr.h"
#include "sakura_rc.h"

#define ID_PAGENUM_ZENPAN	0	//Oct. 25, 2000 JEPRO ZENPAN1��ZENPAN �ɕύX
#define ID_PAGENUM_WIN		1	//Oct. 25, 2000 JEPRO  5�� 1 �ɕύX
#define ID_PAGENUM_EDIT		2
#define ID_PAGENUM_FILE		3
#define ID_PAGENUM_BACKUP	4
#define ID_PAGENUM_FORMAT	5	//Oct. 25, 2000 JEPRO  1�� 5 �ɕύX
#define ID_PAGENUM_URL		6
#define ID_PAGENUM_GREP		7
#define ID_PAGENUM_KEYBOARD	8
#define ID_PAGENUM_CUSTMENU	9	//Oct. 25, 2000 JEPRO 11�� 9 �ɕύX
#define ID_PAGENUM_TOOLBAR	10	//Oct. 25, 2000 JEPRO  9��10 �ɕύX
#define ID_PAGENUM_KEYWORD	11	//Oct. 25, 2000 JEPRO 10��11 �ɕύX
#define ID_PAGENUM_HELPER	12
#define ID_PAGENUM_MACRO	13	//Oct. 25, 2000 JEPRO 10��11 �ɕύX


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�N���X

	1�̃_�C�A���O�{�b�N�X�ɕ����̃v���p�e�B�y�[�W���������\����
	�Ȃ��Ă���ADialog procedure��Event Dispatcher���y�[�W���Ƃɂ���D
*/
class SAKURA_CORE_API CPropCommon
{
public:
	/*
	||  Constructors
	*/
	CPropCommon();
	~CPropCommon();
	void Create( HINSTANCE, HWND, CImageListMgr* );	/* ������ */

	/*
	||  Attributes & Operations
	*/
	int DoPropertySheet( int/*, int*/ );	/* �v���p�e�B�V�[�g�̍쐬 */

	//	Jun. 2, 2001 genta
	//	�����ɂ�����Event Handler��protected�G���A�Ɉړ������D

	HINSTANCE		m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	HWND			m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	HWND			m_hwndThis;		/* ���̃_�C�A���O�̃n���h�� */
	int				m_nPageNum;
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
	int				m_nSettingType;
//	int				m_nActiveItem;

	//	Oct. 16, 2000 genta
	CImageListMgr*	m_pcIcons;	//	Image List

	/*
	|| �_�C�A���O�f�[�^
	*/
	char			m_szHelpFile[_MAX_PATH + 1];
	Common			m_Common;
	short			m_nKeyNameArrNum;				/* �L�[���蓖�ĕ\�̗L���f�[�^�� */
	KEYDATA			m_pKeyNameArr[100];				/* �L�[���蓖�ĕ\ */

	CKeyWordSetMgr	m_CKeyWordSetMgr;
	Types			m_Types[MAX_TYPES];
protected:
	/*
	||  �����w���p�֐�
	*/
	void OnHelp( HWND, int );	/* �w���v */
	int	SearchIntArr( int , int* , int );
	void DrawToolBarItemList( DRAWITEMSTRUCT* );	/* �c�[���o�[�{�^�����X�g�̃A�C�e���`�� */
	void DrawColorButton( DRAWITEMSTRUCT* , COLORREF );	/* �F�{�^���̕`�� */
	BOOL SelectColor( HWND , COLORREF* );	/* �F�I���_�C�A���O */

	//	Jun. 2, 2001 genta
	//	Event Handler, Dialog Procedure�̌�����
	//	Global�֐�������Dialog procedure��class��static method�Ƃ���
	//	�g�ݍ��񂾁D
	//	��������ȉ� Macro�܂Ŕz�u�̌�������static method�̒ǉ�

	//! �ėp�_�C�A���O�v���V�[�W��
	static BOOL DlgProc(
		BOOL (CPropCommon::*DispatchPage)( HWND, UINT, WPARAM, LPARAM ),
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

	//==============================================================
	//!	�S�ʃy�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_GENERAL(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	BOOL DispatchEvent_p1( HWND, UINT, WPARAM, LPARAM );
	void SetData_p1( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p1 */
	int  GetData_p1( HWND );	/* �_�C�A���O�f�[�^�̎擾 p1 */

	//==============================================================
	//!	�t�@�C���y�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_FILE(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for File page
	BOOL DispatchEvent_p2( HWND, UINT, WPARAM, LPARAM );
	void SetData_p2( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p2 */
	int  GetData_p2( HWND );	/* �_�C�A���O�f�[�^�̎擾 p2 */

	//==============================================================
	//!	�L�[���蓖�ăy�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_KEYBIND(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Key Bind page
	BOOL DispatchEvent_p5( HWND, UINT, WPARAM, LPARAM );
	void SetData_p5( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p5 */
	int  GetData_p5( HWND );	/* �_�C�A���O�f�[�^�̎擾 p5 */
	void p5_Import_KeySetting( HWND );	/* p5:�L�[���蓖�Đݒ���C���|�[�g���� */
	void p5_Export_KeySetting( HWND );	/* p5:�L�[���蓖�Đݒ���G�N�X�|�[�g���� */

	//==============================================================
	//!	�c�[���o�[�y�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_TOOLBAR(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Toolbar page
	BOOL DispatchEvent_p6( HWND, UINT, WPARAM, LPARAM );
	void SetData_p6( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p6 */
	int  GetData_p6( HWND );	/* �_�C�A���O�f�[�^�̎擾 p6 */

	//==============================================================
	//!	�L�[���[�h�y�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_KEYWORD(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Keyword page
	BOOL DispatchEvent_p7( HWND, UINT, WPARAM, LPARAM );
	void SetData_p7( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p7 */
	void SetData_p7_KeyWordSet( HWND , int );	/* �_�C�A���O�f�[�^�̐ݒ� p7 �w��L�[���[�h�Z�b�g�̐ݒ� */
	int  GetData_p7( HWND );	/* �_�C�A���O�f�[�^�̎擾 p7 */
	void GetData_p7_KeyWordSet( HWND , int );	/* �_�C�A���O�f�[�^�̎擾 p7 �w��L�[���[�h�Z�b�g�̎擾 */

	//==============================================================
	//!	�J�X�^�����j���[�y�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_CUSTMENU(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Custom Menu page
	BOOL DispatchEvent_p8( HWND, UINT, WPARAM, LPARAM );
	void SetData_p8( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p8 */
	int  GetData_p8( HWND );	/* �_�C�A���O�f�[�^�̎擾 p8 */

	//==============================================================
	//!	�����y�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_FORMAT(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Format page
	BOOL DispatchEvent_p9( HWND, UINT, WPARAM, LPARAM );
	void SetData_p9( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p9 */
	int  GetData_p9( HWND );	/* �_�C�A���O�f�[�^�̎擾 p9 */
	void ChangeDateExample( HWND hwndDlg );
	void ChangeTimeExample( HWND hwndDlg );

	//==============================================================
	//!	�x���y�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_HELPER(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Custom Menu page
	BOOL DispatchEvent_p10( HWND, UINT, WPARAM, LPARAM );
	void SetData_p10( HWND );
	int  GetData_p10( HWND );

	//==============================================================
	//!	�o�b�N�A�b�v�y�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_BACKUP(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Backup page
	BOOL DispatchEvent_PROP_BACKUP( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_BACKUP( HWND );
	int GetData_PROP_BACKUP( HWND );

	//==============================================================
	//!	�E�B���h�E�y�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_WIN(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Window page
	BOOL DispatchEvent_PROP_WIN( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_WIN( HWND );
	int GetData_PROP_WIN( HWND );

	//==============================================================
	//!	�N���b�J�u��URL�y�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_URL(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for clickable URL page
	BOOL DispatchEvent_PROP_URL( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_URL( HWND );
	int GetData_PROP_URL( HWND );

	//==============================================================
	//!	�ҏW�y�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_EDIT(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for edit page
	BOOL DispatchEvent_PROP_EDIT( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_EDIT( HWND );
	int GetData_PROP_EDIT( HWND );

	//==============================================================
	//!	GREP�y�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_GREP(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Grep page
	BOOL DispatchEvent_PROP_GREP( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_GREP( HWND );
	int GetData_PROP_GREP( HWND );

	//	From Here Jun. 2, 2001 genta
	//==============================================================
	//!	�}�N���y�[�W��Dialog Procedure
	static BOOL CALLBACK DlgProc_PROP_MACRO(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Macro page
	BOOL DispatchEvent_PROP_Macro( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_Macro( HWND );//!<�_�C�A���O�f�[�^�̐ݒ� Macro
	int GetData_PROP_Macro( HWND );//!<�_�C�A���O�f�[�^�̎擾 Macro
	//	To Here Jun. 2, 2001 genta

	void p7_Edit_List_KeyWord( HWND, HWND );	/* p7:���X�g���őI������Ă���L�[���[�h��ҏW���� */
	void p7_Delete_List_KeyWord( HWND , HWND );	/* p7:���X�g���őI������Ă���L�[���[�h���폜���� */
	void p7_Import_List_KeyWord( HWND , HWND );	/* p7:���X�g���̃L�[���[�h���C���|�[�g���� */
	void p7_Export_List_KeyWord( HWND , HWND );	/* p7:���X�g���̃L�[���[�h���G�N�X�|�[�g���� */

	void p8_Import_CustMenuSetting( HWND );	/* p8:�J�X�^�����j���[�ݒ���C���|�[�g���� */
	void p8_Export_CustMenuSetting( HWND );	/* p8:�J�X�^�����j���[�ݒ���G�N�X�|�[�g���� */

	//	Aug. 16, 2000 genta
	void EnableBackupInput(HWND hwndDlg);	//	�o�b�N�A�b�v�ݒ��ON/OFF
	//	Aug. 21, 2000 genta
	void EnableFilePropInput(HWND hwndDlg);	//	�t�@�C���ݒ��ON/OFF
	//	Sept. 9, 2000 JEPRO		���s��ǉ�
	void EnableWinPropInput( HWND hwndDlg) ;	//	�E�B���h�E�ݒ��ON/OFF
	//	Sept. 10, 2000 JEPRO	���s��ǉ�
	void CPropCommon::EnableFormatPropInput( HWND hwndDlg );	//	�����ݒ��ON/OFF
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPROP1_H_ */


/*[EOF]*/
