//	$Id$
/************************************************************************

	CPropTypes.h
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/5/8  �V�K�쐬


************************************************************************/

class CPropTypes;

#ifndef _CPROPTYPES_H_
#define _CPROPTYPES_H_

#include <windows.h>
#include "CShareData.h"
#include "CMemory.h"
#include "CKeyBind.h"
//#include "_global_fio.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
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
	BOOL DispatchEvent_p1( HWND, UINT, WPARAM, LPARAM );	/* p1 ���b�Z�[�W���� */
	BOOL DispatchEvent_p3( HWND, UINT, WPARAM, LPARAM );	/* p3 ���b�Z�[�W���� */
	BOOL DispatchEvent_p3_new( HWND, UINT, WPARAM, LPARAM );	/* p3 ���b�Z�[�W���� */
	static BOOL SelectColor( HWND , COLORREF* );	/* �F�I���_�C�A���O */


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
	CKeyWordSetMgr	m_CKeyWordSetMgr;
	int				m_nCurrentColorType;		/* ���ݑI������Ă���F�^�C�v */
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
protected:
	/*
	||  �����w���p�֐�
	*/
	void OnHelp( HWND , int );	/* �w���v */
	void DrawToolBarItemList( DRAWITEMSTRUCT* );	/* �c�[���o�[�{�^�����X�g�̃A�C�e���`�� */
	void DrawColorButton( DRAWITEMSTRUCT* , COLORREF );	/* �F�{�^���̕`�� */
	void SetData_p1( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p1 */
	int  GetData_p1( HWND );	/* �_�C�A���O�f�[�^�̎擾 p1 */
	void SetData_p3( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p3 */
	int  GetData_p3( HWND );	/* �_�C�A���O�f�[�^�̎擾 p3 */
	void SetData_p3_new( HWND );	/* �_�C�A���O�f�[�^�̐ݒ� p3 */
	int  GetData_p3_new( HWND );	/* �_�C�A���O�f�[�^�̎擾 p3 */
	void p3_Import_Colors( HWND );	/* �F�̐ݒ���C���|�[�g */
	void p3_Export_Colors( HWND );	/* �F�̐ݒ���G�N�X�|�[�g */
	void DrawColorListItem( DRAWITEMSTRUCT*);	/* �F��ʃ��X�g �I�[�i�[�`�� */

	//	Sept. 10, 2000 JEPRO ���s��ǉ�
	void EnableTypesPropInput( HWND hwndDlg );	//	�^�C�v�ʐݒ�̃J���[�ݒ��ON/OFF
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPROPTYPES_H_ */

/*[EOF]*/
