//	$Id$
/************************************************************************

	CSplitterWnd.h
	Copyright (C) 1998-2000, Norio Nakatani


    CREATE: 1998/7/7  �V�K�쐬
************************************************************************/
class CSplitterWnd;


#ifndef _CSPLITTERWND_H_
#define _CSPLITTERWND_H_

#include "CWnd.h"
//#include <windows.h>
#include "CShareData.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CSplitterWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CSplitterWnd();
	~CSplitterWnd();
	/*
	||  Attributes & Operations
	*/
	char*			m_pszClassName;			/* �N���X�� */
//	HINSTANCE		m_hInstance;			/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
//	HWND			m_hwndParent;			/* �I�[�i�[�E�B���h�E�̃n���h�� */
//	HWND			m_hWnd;					/* ���̃_�C�A���O�̃n���h�� */
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
	void*			m_pCEditWnd;	
	int				m_nAllSplitRows;		/* �����s�� */
	int				m_nAllSplitCols;		/* �������� */
	int				m_nVSplitPos;			/* ���������ʒu */
	int				m_nHSplitPos;			/* ���������ʒu */
	HWND			m_ChildWndArr[4];		/* �q�E�B���h�E�z�� */
	HCURSOR			m_hcurOld;				/* ���Ƃ̃}�E�X�J�[�\�� */
	int				m_bDragging;			/* �����o�[���h���b�O���� */
	int				m_nDragPosX;			/* �h���b�O�ʒu�w */
	int				m_nDragPosY;			/* �h���b�O�ʒu�x */
	int				m_nActivePane;			/* �A�N�e�B�u�ȃy�C�� */

	HWND Create( HINSTANCE, HWND, void* pCEditWnd );	/* ������ */
	void SetChildWndArr( HWND* );	/* �q�E�B���h�E�̐ݒ� */
	void DoSplit( int, int );	/* �E�B���h�E�̕��� */
	void SetActivePane( int );	/* �A�N�e�B�u�y�C���̐ݒ� */
	int GetPrevPane( void );	/* �O�̃y�C����Ԃ� */ 
	int GetNextPane( void );	/* ���̃y�C����Ԃ� */ 
	int GetFirstPane( void );	/* �ŏ��̃y�C����Ԃ� */
	int GetLastPane( void );	/* �Ō�̃y�C����Ԃ� */

	void VSplitOnOff( void );	/* �c�����n�m�^�n�e�e */
	void HSplitOnOff( void );	/* �������n�m�^�n�e�e */
	void VHSplitOnOff( void );	/* �c�������n�m�^�n�e�e */
//	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */
protected:
	/* ���z�֐� */
	virtual LRESULT DispatchEvent_WM_APP( HWND, UINT, WPARAM, LPARAM );/* �A�v���P�[�V������`�̃��b�Z�[�W(WM_APP <= msg <= 0xBFFF) */
	
	/* ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q�� */
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );	/* �E�B���h�E�T�C�Y�̕ύX���� */
	virtual LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );	/* �`�揈�� */
//	virtual LRESULT OnMove( HWND, UINT, WPARAM, LPARAM );	/* �E�B���h�E�ړ����̏��� */
	virtual LRESULT OnMouseMove( HWND, UINT, WPARAM, LPARAM ); /* �}�E�X�ړ����̏��� */
	virtual LRESULT OnLButtonDown( HWND, UINT, WPARAM, LPARAM );	/* �}�E�X���{�^���������̏��� */
	virtual LRESULT OnLButtonUp( HWND, UINT, WPARAM, LPARAM );	/* �}�E�X���{�^��������̏��� */
	virtual LRESULT OnLButtonDblClk( HWND, UINT, WPARAM, LPARAM );	/* �}�E�X���{�^���_�u���N���b�N���̏��� */
	/*
	||  �����w���p�֐�
	*/
	void DrawFrame( HDC , RECT* );	/* �����t���[���`�� */
	int HitTestSplitter( int , int );	/* �����o�[�ւ̃q�b�g�e�X�g */
	void DrawSplitter( int , int , int );	/* �����g���b�J�[�̕\�� */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CSPLITTERWND_H_ */

/*[EOF]*/
