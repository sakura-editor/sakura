//	$Id$
/*!	@file
	@brief �c�[���`�b�v

	@author Norio Nakatani
	@date 1998/10/30 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, asa-o

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CTipWnd;

#ifndef _CTIPWND_H_
#define _CTIPWND_H_

#include "CWnd.h"
#include "CMemory.h"
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
	void Show( int, int, char*, RECT* pRect = NULL );	/* Tip��\�� */
	void Hide( void );	/* Tip������ */
	void GetWindowSize(RECT* pRect);		// 2001/06/19 asa-o �E�B���h�E�̃T�C�Y�𓾂�

	void ChangeFont( LOGFONT* lf ){
		if ( m_hFont ){
			::DeleteObject( m_hFont );
		}
		m_hFont = ::CreateFontIndirect( lf );
	};

protected: // 2002/2/10 aroka �A�N�Z�X���ύX
	char*		m_pszClassName;	/* Mutex�쐬�p�E�E�B���h�E�N���X�� */
	HFONT		m_hFont;

public:
	CMemory		m_cKey;			/* �L�[�̓��e�f�[�^ */
	BOOL		m_KeyWasHit;	/* �L�[���q�b�g������ */

	CMemory		m_cInfo;		/* Tip�̓��e�f�[�^ */

protected:
	/*
	||  �����w���p�֐�
	*/
	void ComputeWindowSize( HDC, HFONT, const char*, RECT* );	/* �E�B���h�E�̃T�C�Y�����߂� */
	void DrawTipText( HDC, HFONT, const char* );	/* �E�B���h�E�̃e�L�X�g��\�� */

	/* ���z�֐� */

	/* ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q�� */
	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );/* �`�揈�� */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CTIPWND_H_ */


/*[EOF]*/
