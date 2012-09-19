/*!	@file
	@brief �c�[���`�b�v

	@author Norio Nakatani
	@date 1998/10/30 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, aroka
	Copyright (C) 2006, genta, fon

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CTipWnd;

#ifndef _CTIPWND_H_
#define _CTIPWND_H_

#include "CWnd.h"
#include "mem/CMemory.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CTipWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CTipWnd();
	~CTipWnd();
	void Create( HINSTANCE, HWND );	/* ������ */

	/*
	||  Attributes & Operations
	*/
	void Show( int, int, const TCHAR*, RECT* pRect = NULL );	/* Tip��\�� */
	void Hide( void );	/* Tip������ */
	void GetWindowSize(LPRECT pRect);		// 2001/06/19 asa-o �E�B���h�E�̃T�C�Y�𓾂�

	void ChangeFont( LOGFONT* lf ){
		if ( m_hFont ){
			::DeleteObject( m_hFont );
		}
		m_hFont = ::CreateFontIndirect( lf );
	}

protected: // 2002/2/10 aroka �A�N�Z�X���ύX
	TCHAR*		m_pszClassName;	/* Mutex�쐬�p�E�E�B���h�E�N���X�� */
	HFONT		m_hFont;

public:
	CNativeW	m_cKey;			/* �L�[�̓��e�f�[�^ */
	BOOL		m_KeyWasHit;	/* �L�[���q�b�g������ */
	int			m_nSearchLine;	/* �����̃q�b�g�s */	// 2006.04.10 fon
	int			m_nSearchDict;	/* �q�b�g�����ԍ� */	// 2006.04.10 fon

	CNativeT	m_cInfo;		/* Tip�̓��e�f�[�^ */

protected:
	/*
	||  �����w���p�֐�
	*/
	void ComputeWindowSize( HDC, HFONT, const TCHAR*, RECT* );	/* �E�B���h�E�̃T�C�Y�����߂� */
	void DrawTipText( HDC, HFONT, const TCHAR* );	/* �E�B���h�E�̃e�L�X�g��\�� */

	/* ���z�֐� */
	//	Jan. 9, 2006 genta
	virtual void AfterCreateWindow( void );

	/* ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q�� */
	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );/* �`�揈�� */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CTIPWND_H_ */



