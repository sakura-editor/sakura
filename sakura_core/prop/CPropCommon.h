/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�̏���

	@author	Norio Nakatani
	@date 1998/12/24 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, jepro
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, aroka, Moca
	Copyright (C) 2005, MIK, Moca, aroka
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

class CPropCommon;

#ifndef _CPROP1_H_
#define _CPROP1_H_
#include <windows.h>
#include "env/CShareData.h"
#include "sakura_rc.h"
#include "func/CFuncLookup.h" //MacroRec
class CImageListMgr;
class CSMacroMgr;
class CMenuDrawer;// 2002/2/10 aroka to here

//	#define -> enum �ɕύX	2008/6/22 Uchi
//	�����ύX Win,Toolbar,Tab,Statusbar�̏��ɁAFile,FileName ����	2008/6/22 Uchi
#if 1
enum ComPropSheetOrder {
	ID_PAGENUM_GENERAL = 0,		// �S��
	ID_PAGENUM_WIN,				// �E�B���h�E
	ID_PAGENUM_TOOLBAR,			// �c�[���o�[
	ID_PAGENUM_TAB,				// �^�u�o�[
	ID_PAGENUM_STATUSBAR,		// �X�e�[�^�X�o�[
	ID_PAGENUM_EDIT,			// �ҏW
	ID_PAGENUM_FILE,			// �t�@�C��
	ID_PAGENUM_FILENAME,		// �t�@�C�����\��
	ID_PAGENUM_BACKUP,			// �o�b�N�A�b�v
	ID_PAGENUM_FORMAT,			// ����
	ID_PAGENUM_GREP,			// ����
	ID_PAGENUM_KEYBOARD,		// �L�[���蓖��
	ID_PAGENUM_CUSTMENU,		// �J�X�^�����j���[
	ID_PAGENUM_KEYWORD,			// �����L�[���[�h
	ID_PAGENUM_HELPER,			// �x��
	ID_PAGENUM_MACRO,			// �}�N��
};
#else
#define ID_PAGENUM_ZENPAN		0	//Oct. 25, 2000 JEPRO ZENPAN1��ZENPAN �ɕύX
#define ID_PAGENUM_WIN			1	//Oct. 25, 2000 JEPRO  5�� 1 �ɕύX
#define ID_PAGENUM_TAB			2	//Feb. 11, 2007 genta URL�Ɠ��ꊷ��	// 2007.02.13 �����ύX�iTAB��WIN�̎��Ɂj
#define ID_PAGENUM_EDIT			3
#define ID_PAGENUM_FILE			4
#define ID_PAGENUM_BACKUP		5
#define ID_PAGENUM_FORMAT		6	//Oct. 25, 2000 JEPRO  1�� 5 �ɕύX
#define ID_PAGENUM_GREP			7
#define ID_PAGENUM_KEYBOARD		8
#define ID_PAGENUM_CUSTMENU		9	//Oct. 25, 2000 JEPRO 11�� 9 �ɕύX
#define ID_PAGENUM_TOOLBAR		10	//Oct. 25, 2000 JEPRO  9��10 �ɕύX
#define ID_PAGENUM_KEYWORD		11	//Oct. 25, 2000 JEPRO 10��11 �ɕύX
#define ID_PAGENUM_HELPER		12
#define ID_PAGENUM_MACRO		13	//Oct. 25, 2000 JEPRO 10��11 �ɕύX
#define ID_PAGENUM_FILENAME		14	// Moca �ǉ�
#define ID_PAGENUM_STATUSBAR	15	// 2008/6/21 Uchi �ǉ�
#endif
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�N���X

	1�̃_�C�A���O�{�b�N�X�ɕ����̃v���p�e�B�y�[�W���������\����
	�Ȃ��Ă���ADialog procedure��Event Dispatcher���y�[�W���Ƃɂ���D

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class SAKURA_CORE_API CPropCommon
{
public:
	/*
	||  Constructors
	*/
	CPropCommon();
	~CPropCommon();
	//	Sep. 29, 2001 genta �}�N���N���X��n���悤��;
//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
	void Create( HWND, CImageListMgr*, CSMacroMgr* , CMenuDrawer* );	/* ������ */

	/*
	||  Attributes & Operations
	*/
	int DoPropertySheet( int/*, int*/ );	/* �v���p�e�B�V�[�g�̍쐬 */

	// 2002.12.11 Moca �ǉ�
	void InitData( void );		//!< DLLSHAREDATA����ꎞ�f�[�^�̈�ɐݒ�𕡐�����
	void ApplyData( void );		//!< �ꎞ�f�[�^�̈悩���DLLSHAREDATA�ݒ���R�s�[����

	//	Jun. 2, 2001 genta
	//	�����ɂ�����Event Handler��protected�G���A�Ɉړ������D

	HWND				m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	HWND				m_hwndThis;		/* ���̃_�C�A���O�̃n���h�� */
	ComPropSheetOrder	m_nPageNum;
	DLLSHAREDATA*		m_pShareData;
	int					m_nSettingType;
//	int				m_nActiveItem;

	//	Oct. 16, 2000 genta
	CImageListMgr*	m_pcIcons;	//	Image List
	
	//	Oct. 2, 2001 genta �O���}�N���ǉ��ɔ����C�Ή������̕ʃN���X��
	//	Oct. 15, 2001 genta Lookup�̓_�C�A���O�{�b�N�X���ŕʃC���X�^���X�����悤��
	//	(�����ΏۂƂ��āC�ݒ�pcommon�̈���w���悤�ɂ��邽�߁D)
	CFuncLookup			m_cLookup;
	CSMacroMgr*			m_pcSMacro;

	CMenuDrawer*		m_pcMenuDrawer;
	/*
	|| �_�C�A���O�f�[�^
	*/
	SFilePath		m_szHelpFile;
	CommonSetting	m_Common;

	//2002/04/25 YAZAKI STypeConfig�S�̂�ێ�����K�v�͂Ȃ��B
	//STypeConfig			m_Types[MAX_TYPES];
	// 2005.01.13 MIK �Z�b�g������
	int				m_Types_nKeyWordSetIdx[MAX_TYPES][MAX_KEYWORDSET_PER_TYPE];

protected:
	/*
	||  �����w���p�֐�
	*/
	void OnHelp( HWND, int );	/* �w���v */
	int	SearchIntArr( int , int* , int );
//	void DrawToolBarItemList( DRAWITEMSTRUCT* );	/* �c�[���o�[�{�^�����X�g�̃A�C�e���`�� */
//	void DrawColorButton( DRAWITEMSTRUCT* , COLORREF );	/* �F�{�^���̕`�� */ // 2002.11.09 Moca ���g�p
//	BOOL SelectColor( HWND , COLORREF* );	/* �F�I���_�C�A���O */

	//	Jun. 2, 2001 genta
	//	Event Handler, Dialog Procedure�̌�����
	//	Global�֐�������Dialog procedure��class��static method�Ƃ���
	//	�g�ݍ��񂾁D
	//	��������ȉ� Macro�܂Ŕz�u�̌�������static method�̒ǉ�

	//! �ėp�_�C�A���O�v���V�[�W��
	static INT_PTR DlgProc(
		INT_PTR (CPropCommon::*DispatchPage)( HWND, UINT, WPARAM, LPARAM ),
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

	//==============================================================
	//!	�S�ʃy�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_GENERAL(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR DispatchEvent_p1( HWND, UINT, WPARAM, LPARAM );
	void SetData_p1( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p1 */
	int  GetData_p1( HWND );	/* �_�C�A���O�f�[�^�̎擾 p1 */

	//==============================================================
	//!	�t�@�C���y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_FILE(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for File page
	INT_PTR DispatchEvent_p2( HWND, UINT, WPARAM, LPARAM );
	void SetData_p2( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p2 */
	int  GetData_p2( HWND );	/* �_�C�A���O�f�[�^�̎擾 p2 */

	//==============================================================
	//!	�L�[���蓖�ăy�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_KEYBIND(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Key Bind page
	INT_PTR DispatchEvent_p5( HWND, UINT, WPARAM, LPARAM );
	void SetData_p5( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p5 */
	int  GetData_p5( HWND );	/* �_�C�A���O�f�[�^�̎擾 p5 */
	void p5_ChangeKeyList( HWND ); /* p5: �L�[���X�g���`�F�b�N�{�b�N�X�̏�Ԃɍ��킹�čX�V����*/
	void p5_Import_KeySetting( HWND );	/* p5:�L�[���蓖�Đݒ���C���|�[�g���� */
	void p5_Export_KeySetting( HWND );	/* p5:�L�[���蓖�Đݒ���G�N�X�|�[�g���� */

	//==============================================================
	//!	�c�[���o�[�y�[�W��Dialog Procedure
	// 20050809 aroka _p6����_PROP_TOOLBAR�ɖ��O�ύX
	static INT_PTR CALLBACK DlgProc_PROP_TOOLBAR(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Toolbar page
	INT_PTR DispatchEvent_PROP_TOOLBAR( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_TOOLBAR( HWND );	/* �c�[���o�[ �_�C�A���O�f�[�^�̐ݒ� */
	int  GetData_PROP_TOOLBAR( HWND );	/* �c�[���o�[ �_�C�A���O�f�[�^�̎擾 */
	void DrawToolBarItemList( DRAWITEMSTRUCT* );	/* �c�[���o�[�{�^�����X�g�̃A�C�e���`�� */

	//==============================================================
	//!	�L�[���[�h�y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_KEYWORD(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Keyword page
	INT_PTR DispatchEvent_p7( HWND, UINT, WPARAM, LPARAM );
	void SetData_p7( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p7 */
	void SetData_p7_KeyWordSet( HWND , int );	/* �_�C�A���O�f�[�^�̐ݒ� p7 �w��L�[���[�h�Z�b�g�̐ݒ� */
	int  GetData_p7( HWND );	/* �_�C�A���O�f�[�^�̎擾 p7 */
	void GetData_p7_KeyWordSet( HWND , int );	/* �_�C�A���O�f�[�^�̎擾 p7 �w��L�[���[�h�Z�b�g�̎擾 */
	void DispKeywordCount( HWND hwndDlg );

	//==============================================================
	//!	�J�X�^�����j���[�y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_CUSTMENU(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Custom Menu page
	INT_PTR DispatchEvent_p8( HWND, UINT, WPARAM, LPARAM );
	void SetData_p8( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p8 */
	int  GetData_p8( HWND );	/* �_�C�A���O�f�[�^�̎擾 p8 */

	//==============================================================
	//!	�����y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_FORMAT(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Format page
	INT_PTR DispatchEvent_p9( HWND, UINT, WPARAM, LPARAM );
	void SetData_p9( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p9 */
	int  GetData_p9( HWND );	/* �_�C�A���O�f�[�^�̎擾 p9 */
	void ChangeDateExample( HWND hwndDlg );
	void ChangeTimeExample( HWND hwndDlg );

	//==============================================================
	//!	�x���y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_HELPER(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Custom Menu page
	INT_PTR DispatchEvent_p10( HWND, UINT, WPARAM, LPARAM );
	void SetData_p10( HWND );
	int  GetData_p10( HWND );

	//==============================================================
	//!	�o�b�N�A�b�v�y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_BACKUP(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Backup page
	INT_PTR DispatchEvent_PROP_BACKUP( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_BACKUP( HWND );
	int GetData_PROP_BACKUP( HWND );

	//==============================================================
	//!	�E�B���h�E�y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_WIN(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Window page
	INT_PTR DispatchEvent_PROP_WIN( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_WIN( HWND );
	int GetData_PROP_WIN( HWND );

	//==============================================================
	//!	�^�u����y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_TAB(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for TAB page
	INT_PTR DispatchEvent_PROP_TAB( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_TAB( HWND );
	int GetData_PROP_TAB( HWND );
	void EnableTabPropInput(HWND hwndDlg);

	//==============================================================
	//!	�ҏW�y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_EDIT(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for edit page
	INT_PTR DispatchEvent_PROP_EDIT( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_EDIT( HWND );
	int GetData_PROP_EDIT( HWND );

	//==============================================================
	//!	GREP�y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_GREP(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Grep page
	INT_PTR DispatchEvent_PROP_GREP( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_GREP( HWND );
	int GetData_PROP_GREP( HWND );
	void SetRegexpVersion( HWND ); // 2007.08.12 genta �o�[�W�����\��

	//	From Here Jun. 2, 2001 genta
	//==============================================================
	//!	�}�N���y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_MACRO(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Macro page
	INT_PTR DispatchEvent_PROP_Macro( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_Macro( HWND );//!<�_�C�A���O�f�[�^�̐ݒ� Macro
	int GetData_PROP_Macro( HWND );//!<�_�C�A���O�f�[�^�̎擾 Macro
	void InitDialog_PROP_Macro( HWND hwndDlg );//!< Macro�y�[�W�̏�����
	//	To Here Jun. 2, 2001 genta
	void SetMacro2List_Macro( HWND hwndDlg );//!< Macro�f�[�^�̐ݒ�
	void SelectBaseDir_Macro( HWND hwndDlg );//!< Macro�f�B���N�g���̑I��
	void OnFileDropdown_Macro( HWND hwndDlg );//!< �t�@�C���h���b�v�_�E�����J�����Ƃ�
	void CheckListPosition_Macro( HWND hwndDlg );//!< ���X�g�r���[��Focus�ʒu�m�F
	static int CALLBACK DirCallback_Macro( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData );

	//==============================================================
	//!	�t�@�C�����\���y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_FILENAME(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for FileName page
	INT_PTR DispatchEvent_PROP_FILENAME( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_FILENAME( HWND );
	int  GetData_PROP_FILENAME( HWND );
	static int SetListViewItem_FILENAME( HWND hListView, int, LPTSTR, LPTSTR, bool );//!<ListView�̃A�C�e����ݒ�
	static void GetListViewItem_FILENAME( HWND hListView, int, LPTSTR, LPTSTR );//!<ListView�̃A�C�e�����擾
	static int MoveListViewItem_FILENAME( HWND hListView, int, int );//!<ListView�̃A�C�e�����ړ�����

	//==============================================================
	//!	�X�e�[�^�X�o�[�y�[�W��Dialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_STATUSBAR(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for edit page
	INT_PTR DispatchEvent_PROP_STATUSBAR( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_STATUSBAR( HWND );
	int GetData_PROP_STATUSBAR( HWND );


	int nLastPos_Macro; //!< �O��t�H�[�J�X�̂������ꏊ
	int m_nLastPos_FILENAME; //!< �O��t�H�[�J�X�̂������ꏊ �t�@�C�����^�u�p


	void p7_Edit_List_KeyWord( HWND, HWND );	/* p7:���X�g���őI������Ă���L�[���[�h��ҏW���� */
	void p7_Delete_List_KeyWord( HWND , HWND );	/* p7:���X�g���őI������Ă���L�[���[�h���폜���� */
	void p7_Import_List_KeyWord( HWND , HWND );	/* p7:���X�g���̃L�[���[�h���C���|�[�g���� */
	void p7_Export_List_KeyWord( HWND , HWND );	/* p7:���X�g���̃L�[���[�h���G�N�X�|�[�g���� */
	void p7_Clean_List_KeyWord( HWND , HWND );	//! p7:���X�g���̃L�[���[�h�𐮗����� 2005.01.26 Moca

	void p8_Import_CustMenuSetting( HWND );	/* p8:�J�X�^�����j���[�ݒ���C���|�[�g���� */
	void p8_Export_CustMenuSetting( HWND );	/* p8:�J�X�^�����j���[�ݒ���G�N�X�|�[�g���� */

	//	Aug. 16, 2000 genta
	void EnableBackupInput(HWND hwndDlg);	//	�o�b�N�A�b�v�ݒ��ON/OFF
	//	20051107 aroka
	void UpdateBackupFile(HWND hwndDlg);	//	�o�b�N�A�b�v�t�@�C���̏ڍאݒ�

	//	Aug. 21, 2000 genta
	void EnableFilePropInput(HWND hwndDlg);	//	�t�@�C���ݒ��ON/OFF
	//	Sept. 9, 2000 JEPRO		���s��ǉ�
	void EnableWinPropInput( HWND hwndDlg) ;	//	�E�B���h�E�ݒ��ON/OFF
	//	Sept. 10, 2000 JEPRO	���s��ǉ�
	void CPropCommon::EnableFormatPropInput( HWND hwndDlg );	//	�����ݒ��ON/OFF
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPROP1_H_ */



