/*!	@file
	@brief �������E�B���h�E�N���X

	@author Norio Nakatani
	@date 1998/07/07 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
class CSplitterWnd;


#ifndef _CSPLITTERWND_H_
#define _CSPLITTERWND_H_

#include "CWnd.h"
#include "env/CShareData.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/

#define MAXCOUNTOFVIEW	4

/*!
	@brief �������E�B���h�E�N���X
	
	�S�����E�B���h�E�̊Ǘ��ƕ������̕`����s���B
	
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class SAKURA_CORE_API CSplitterWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CSplitterWnd();
	~CSplitterWnd();
private: // 2002/2/3 aroka
	/*
	||  Attributes & Operations
	*/
	TCHAR*			m_pszClassName;			/* �N���X�� */
	DLLSHAREDATA*	m_pShareData;
	void*			m_pCEditWnd;
	int				m_nAllSplitRows;		/* �����s�� */
	int				m_nAllSplitCols;		/* �������� */
	int				m_nVSplitPos;			/* ���������ʒu */
	int				m_nHSplitPos;			/* ���������ʒu */
	HWND			m_ChildWndArr[MAXCOUNTOFVIEW];		/* �q�E�B���h�E�z�� */
	HCURSOR			m_hcurOld;				/* ���Ƃ̃}�E�X�J�[�\�� */
	int				m_bDragging;			/* �����o�[���h���b�O���� */
	int				m_nDragPosX;			/* �h���b�O�ʒu�w */
	int				m_nDragPosY;			/* �h���b�O�ʒu�x */
	int				m_nActivePane;			/* �A�N�e�B�u�ȃy�C�� */
public: // 2002/2/3 aroka
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
	int GetAllSplitRows(){ return m_nAllSplitRows;} // 2002/2/3 aroka
	int GetAllSplitCols(){ return m_nAllSplitCols;} // 2002/2/3 aroka
protected:
	/* ���z�֐� */
	virtual LRESULT DispatchEvent_WM_APP( HWND, UINT, WPARAM, LPARAM );/* �A�v���P�[�V������`�̃��b�Z�[�W(WM_APP <= msg <= 0xBFFF) */

	/* ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q�� */
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );	/* �E�B���h�E�T�C�Y�̕ύX���� */
	virtual LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );	/* �`�揈�� */
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



