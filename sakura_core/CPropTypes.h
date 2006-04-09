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

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

class CPropTypes;

#ifndef _CPROPTYPES_H_
#define _CPROPTYPES_H_

#include <windows.h>
#include "CShareData.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �^�C�v�ʐݒ�_�C�A���O�{�b�N�X

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class SAKURA_CORE_API CPropTypes
{
public:
	/*
	||  Constructors
	*/
	CPropTypes();
	~CPropTypes();
	void Create( HINSTANCE, HWND );	/* ������ */

	/*
	||  Attributes & Operations
	*/
	int DoPropertySheet( int );	/* �v���p�e�B�V�[�g�̍쐬 */
	INT_PTR DispatchEvent_p1( HWND, UINT, WPARAM, LPARAM );	/* p1 ���b�Z�[�W���� */
	INT_PTR DispatchEvent_p2( HWND, UINT, WPARAM, LPARAM );	/* p2 ���b�Z�[�W���� �x���^�u */ // 2001/06/14 asa-o
	INT_PTR DispatchEvent_p3( HWND, UINT, WPARAM, LPARAM );	/* p3 ���b�Z�[�W���� */
	INT_PTR DispatchEvent_p3_new( HWND, UINT, WPARAM, LPARAM );	/* p3 ���b�Z�[�W���� */
	static BOOL SelectColor( HWND , COLORREF*, DWORD* );	/* �F�I���_�C�A���O */
	INT_PTR DispatchEvent_Regex( HWND, UINT, WPARAM, LPARAM );	/* ���b�Z�[�W���� ���K�\���L�[���[�h */	//@@@ 2001.11.17 add MIK

private:
	HINSTANCE	m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	HWND		m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	HWND		m_hwndThis;		/* ���̃_�C�A���O�̃n���h�� */
	int			m_nPageNum;

	/*
	|| �_�C�A���O�f�[�^
	*/
	char			m_szHelpFile[_MAX_PATH + 1];
	int				m_nMaxLineSize_org;					/* �܂�Ԃ������� */
	Types			m_Types;
	CKeyWordSetMgr*	m_pCKeyWordSetMgr;	// Mar. 31, 2003 genta �������팸�̂��߃|�C���^��
	int				m_nCurrentColorType;		/* ���ݑI������Ă���F�^�C�v */
	DLLSHAREDATA*	m_pShareData;
	int		m_nSet[ MAX_KEYWORDSET_PER_TYPE ];	//	2005.01.13 MIK keyword set index

	// �t�H���gDialog�J�X�^���p���b�g
	DWORD			m_dwCustColors[16];

protected:
	/*
	||  �����w���p�֐�
	*/
	void OnHelp( HWND , int );	/* �w���v */
	//void DrawToolBarItemList( DRAWITEMSTRUCT* );	/* �c�[���o�[�{�^�����X�g�̃A�C�e���`�� */// 20050809 aroka ���g�p
	void DrawColorButton( DRAWITEMSTRUCT* , COLORREF );	/* �F�{�^���̕`�� */
	void SetData_p1( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p1 */
	int  GetData_p1( HWND );	/* �_�C�A���O�f�[�^�̎擾 p1 */

	// 2001/06/14 asa-o
	void SetData_p2( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p2 �x���^�u */
	int  GetData_p2( HWND );	/* �_�C�A���O�f�[�^�̎擾 p2 �x���^�u */

	void SetData_p3( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p3 */
	int  GetData_p3( HWND );	/* �_�C�A���O�f�[�^�̎擾 p3 */
	void SetData_p3_new( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p3 */
	int  GetData_p3_new( HWND );	/* �_�C�A���O�f�[�^�̎擾 p3 */
	void p3_Import_Colors( HWND );	/* �F�̐ݒ���C���|�[�g */
	void p3_Export_Colors( HWND );	/* �F�̐ݒ���G�N�X�|�[�g */
	void DrawColorListItem( DRAWITEMSTRUCT*);	/* �F��ʃ��X�g �I�[�i�[�`�� */

	//	Sept. 10, 2000 JEPRO ���s��ǉ�
	void EnableTypesPropInput( HWND hwndDlg );	//	�^�C�v�ʐݒ�̃J���[�ݒ��ON/OFF

	void SetData_Regex( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� ���K�\���L�[���[�h */	//@@@ 2001.11.17 add MIK
	int  GetData_Regex( HWND );	/* �_�C�A���O�f�[�^�̎擾 ���K�\���L�[���[�h */	//@@@ 2001.11.17 add MIK
	BOOL Import_Regex( HWND );	//@@@ 2001.11.17 add MIK
	BOOL Export_Regex( HWND );	//@@@ 2001.11.17 add MIK
	static INT_PTR CALLBACK PropTypesRegex( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );	//@@@ 2001.11.17 add MIK
	BOOL RegexKakomiCheck(const char *s);	//@@@ 2001.11.17 add MIK

	void RearrangeKeywordSet( HWND );	// Jan. 23, 2005 genta �L�[���[�h�Z�b�g�Ĕz�u

public:
	//	Jan. 23, 2005 genta
	//	�^�C�v�ʐݒ�f�[�^�̐ݒ�E�擾
	void SetTypeData( const Types& t ){ m_Types = t; }
	void GetTypeData( Types& t ) const { t = m_Types; }
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPROPTYPES_H_ */


/*[EOF]*/
