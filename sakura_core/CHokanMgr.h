//	$Id$
/*!	@file
	�L�[���[�h�⊮
	
	@author Norio Nakatani
	$Revision$
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

class CHokanMgr;

#ifndef _CHOKANMGR_H_
#define _CHOKANMGR_H_

#include "CDialog.h"
#include <windows.h>
#include "CMemory.h"
#include "CShareData.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CHokanMgr : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CHokanMgr();
	~CHokanMgr();

	HWND DoModeless( HINSTANCE, HWND, LPARAM );/* ���[�h���X�_�C�A���O�̕\�� */
	void Hide( void );
	/* ������ */
	int CHokanMgr::Search( 
	//	HFONT		hFont,
		POINT*		ppoWin,
		int			nWinHeight,
		int			nColmWidth,
		const char*	pszCurWord,
	//	void*		pcEditView,
		const char* pszHokanFile,
		int			bHokanLoHiCase,			// ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� 2001/06/19 asa-o
		CMemory*	pcmemHokanWord = NULL	// �⊮��₪�P�̂Ƃ�����Ɋi�[ 2001/06/19 asa-o
	);
	void SetCurKouhoStr( void );
	BOOL DoHokan( int );
	void ChangeView( LPARAM );/* ���[�h���X���F�ΏۂƂȂ�r���[�̕ύX */


	BOOL OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	BOOL OnDestroy( void );
	BOOL OnSize( WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int wID );
	BOOL OnKeyDown( WPARAM wParam, LPARAM lParam );
	BOOL OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL OnLbnDblclk( int wID );
	BOOL OnKillFocus( WPARAM wParam, LPARAM lParam );
//	int OnVKeyToItem( WPARAM wParam, LPARAM lParam );
//	int OnCharToItem( WPARAM wParam, LPARAM lParam );
//	BOOL OnNextDlgCtl( WPARAM, LPARAM );

	int KeyProc( WPARAM, LPARAM );

//	2001/06/18 asa-o
	void ShowTip();	// �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v�̕\��

    
//	HFONT			m_hFont;
//	HFONT			m_hFontOld;
	CMemory			m_cmemCurWord;
	CMemory*		m_pcmemKouho;
	int				m_nKouhoNum;

	int				m_nCurKouhoIdx;
	char*			m_pszCurKouho;

	POINT			m_poWin;
	int				m_nWinHeight;
	int				m_nColmWidth;
	int				m_pnDx[256 + 1];		/* ������`��p�������z�� */
//	void*			m_pcEditView;
	int				m_bTimerFlag;

protected:
	/*
	||  �����w���p�֐�
	*/

};



///////////////////////////////////////////////////////////////////////
#endif /* _CHOKANMGR_H_ */

/*[EOF]*/
