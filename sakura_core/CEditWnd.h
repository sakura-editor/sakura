//	$Id$
/*!	@file
	�ҏW�E�B���h�E�i�O�g�j�Ǘ��N���X

	@author Norio Nakatani
	@date 1998/05/13 �V�K�쐬
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

#ifndef _CEDITWND_H_
#define _CEDITWND_H_

class CEditWnd;

#include "CEditDoc.h"
#include "CKeyBind.h"
#include "CShareData.h"
#include "CFuncKeyWnd.h"
#include "CMenuDrawer.h"
#include "CImageListMgr.h"




/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CEditWnd
{
public:
	/*
	||  Constructors
	*/
	CEditWnd();
	~CEditWnd();

	/*
	|| �����o�֐�
	*/
	HWND Create( HINSTANCE, HWND, const char*, int, BOOL );	/* �쐬 */


	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ���b�Z�[�W���� */
	BOOL DispatchEvent_PPB( HWND, UINT, WPARAM, LPARAM );	/* ����v���r���[ ����o�[ �_�C�A���O�̃��b�Z�[�W���� */

	void PrintPreviewModeONOFF( void );	/* ����v���r���[���[�h�̃I��/�I�t */

	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );	/* �`�揈�� */
	LRESULT OnSize( WPARAM, LPARAM );	/* WM_SIZE ���� */
	LRESULT OnLButtonUp( WPARAM, LPARAM );
	LRESULT OnLButtonDown( WPARAM, LPARAM );
	LRESULT OnMouseMove( WPARAM, LPARAM );
	LRESULT OnMouseWheel( WPARAM, LPARAM );
	LRESULT OnHScroll( WPARAM, LPARAM );
	LRESULT OnVScroll( WPARAM, LPARAM );

	void OnTimer( HWND, UINT, UINT, DWORD );	/* �^�C�}�[�̏��� */
	void OnCommand( WORD, WORD , HWND );

	void CreateToolBar( void );			/* �c�[���o�[�쐬 */
	void CreateStatusBar( void );		/* �X�e�[�^�X�o�[�쐬 */
	void DestroyStatusBar( void );		/* �X�e�[�^�X�o�[�j�� */
	void CreatePrintPreviewBar( void );	/* �c�[���o�[�쐬 */

	void InitMenu( HMENU, UINT, BOOL );
//�����v���Z�X��
	void MessageLoop( void );	/* ���b�Z�[�W���[�v */

	int	OnClose( void );	/* �I�����̏��� */


void CEditWnd::ExecCmd(LPCSTR lpszCmd/*, HANDLE hFile*/);


//	void MyAppendMenu( HMENU, int, int, char* );	/* ���j���[���ڂ�ǉ� */
//#ifdef _DEBUG
	void SetDebugModeON( void );	/* �f�o�b�O���j�^���[�h�ɐݒ� */
//#endif
	/*
	|| �X�^�e�B�b�N�ȃ����o�֐�
	*/
	static int IsFuncEnable( CEditDoc*, DLLSHAREDATA*, int );	/* �@�\�����p�\�����ׂ� */
	static int IsFuncChecked( CEditDoc*, DLLSHAREDATA*, int );	/* �@�\���`�F�b�N��Ԃ����ׂ� */

	static void OnHelp_MenuItem( HWND, int );	/* ���j���[�A�C�e���ɑΉ�����w���v��\�� */
	static int FuncID_To_HelpContextID( int );	/* �@�\ID�ɑΉ����郁�j���[�R���e�L�X�g�ԍ���Ԃ� */

	/*
	|| �����o�ϐ�
	*/
	HINSTANCE		m_hInstance;
	HWND			m_hWnd;
	char*			m_pszAppName;
	CEditDoc		m_cEditDoc;
	HWND			m_hwndParent;
    HWND			m_hwndToolBar;
	HWND			m_hwndStatusBar;
	HWND			m_hwndProgressBar;
	HWND			m_hwndPrintPreviewBar;	/* ����v���r���[ ����o�[ */
	HWND			m_hwndVScrollBar;	/* �����X�N���[���o�[�E�B���h�E�n���h�� */
	HWND			m_hwndHScrollBar;	/* �����X�N���[���o�[�E�B���h�E�n���h�� */
	HWND			m_hwndSizeBox;		/* �T�C�Y�{�b�N�X�E�B���h�E�n���h�� */
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;
	HBITMAP			m_hbmpOPENED;
	HBITMAP			m_hbmpOPENED_THIS;
	CFuncKeyWnd		m_CFuncKeyWnd;
	CMenuDrawer		m_CMenuDrawer;
	int				m_nWinSizeType;	/* �T�C�Y�ύX�̃^�C�v */
	BOOL			m_bDragMode;
	int				m_nDragPosOrgX;
	int				m_nDragPosOrgY;
	BOOL			m_SCROLLBAR_HORZ;
	BOOL			m_SCROLLBAR_VERT;

//	HANDLE			m_hThread;

//	int				m_nChildArrNum;
//	HWND			m_hwndChildArr[32];


	/* ����v���r���[�\����� */
	int				m_nPreviewVScrollPos;	/* ����v���r���[�F�X�N���[���ʒu�c */
	int				m_nPreviewHScrollPos;	/* ����v���r���[�F�X�N���[���ʒu�� */
	int				m_nPreview_Zoom;	/* ����v���r���[�F�{�� */
	int				m_nPreview_CurPage;	/* ����v���r���[�F�y�[�W */
	int				m_nPreview_AllPageNum;	/* ����v���r���[�F�S�y�[�W�� */
	int				m_nPreview_ViewWidth;		/* ����v���r���[�F�r���[��(�s�N�Z��) */
	int				m_nPreview_ViewHeight;		/* ����v���r���[�F�r���[����(�s�N�Z��) */
	int				m_nPreview_ViewMarginLeft;	/* ����v���r���[�F�r���[���[�Ɨp���̊Ԋu(1/10mm�P��) */
	int				m_nPreview_ViewMarginTop;	/* ����v���r���[�F�r���[���[�Ɨp���̊Ԋu(1/10mm�P��) */
	int				m_nPreview_PaperAllWidth;	/* �p����(1/10mm�P��) */
	int				m_nPreview_PaperAllHeight;	/* �p������(1/10mm�P��) */
	int				m_nPreview_PaperWidth;	/* �p������L����(1/10mm�P��) */
	int				m_nPreview_PaperHeight;	/* �p������L������(1/10mm�P��) */
	int				m_nPreview_PaperOffsetLeft;	/* �p���]�����[(1/10mm�P��) */
	int				m_nPreview_PaperOffsetTop;	/* �p���]����[(1/10mm�P��) */
	int				m_nPreview_PaperOffsetRight;	/* �p���]���E�[(1/10mm�P��) */
	int				m_nPreview_PaperOffsetBottom;	/* �p���]�����[(1/10mm�P��) */
	int				m_bPreview_EnableColms;	/* �󎚉\����/�y�[�W */
	int				m_bPreview_EnableLines;	/* �󎚉\�s��/�y�[�W */
	int				m_nPreview_LineNumberColmns;	/* �s�ԍ��G���A�̕��i�������j */
	int				m_nAllPageNum;	/* �S�y�[�W�� */
	int				m_nCurPageNum;	/* ���݂̃y�[�W */

	PRINTSETTING*	m_pPrintSetting;	/* ���݂̈���ݒ� */
	LOGFONT			m_lfPreviewHan;	/* �v���r���[�p�t�H���g */
	LOGFONT			m_lfPreviewZen;	/* �v���r���[�p�t�H���g */

	CLayoutMgr		m_CLayoutMgr_Print;	/* ����p�̃��C�A�E�g�Ǘ���� */
	int				m_pnDx[10240 + 10];	/* ������`��p�������z�� */
	HDC				m_hdcCompatDC;	/* �ĕ`��p�R���p�`�u���c�b */
	HBITMAP			m_hbmpCompatBMP;	/* �ĕ`��p�������a�l�o */
	HBITMAP			m_hbmpCompatBMPOld;	/* �ĕ`��p�������a�l�o(OLD) */

	//	Oct. 12, 2000 genta
	CImageListMgr	m_cIcons;	//	Image List

	/*
	|| �����w���p�n
	*/
protected:
	void OnDropFiles( HDROP );	/* �t�@�C�����h���b�v���ꂽ */
	void InitPreviewScrollBar( void );	/* ����v���r���[ �X�N���[���o�[������ */
	void OnPreviewZoom( BOOL );	/* �v���r���[�g��k�� */
	void OnPreviewGoPage( int );	/* �v���r���[ �y�[�W�w�� */
	/* ����^�v���r���[ �s�`�� */
	void CEditWnd::Print_DrawLine(
		HDC			hdc,
		int			x,
		int			y,
		const char*	pLine,
		int			nLineLen,
		HFONT		hFontZen
	);
	/* ���/����v���r���[ �y�[�W�e�L�X�g�̕`�� */
	void CEditWnd::DrawPageText( HDC, int, int, int, HFONT, CDlgCancel* );
	void OnPrint( void );	/* ������s */
	void OnChangePrintSetting( void );	/* ����ݒ�̔��f */
public:
	BOOL OnPrintPageSetting( void );/* ����y�[�W�ݒ� */

};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITWND_H_ */


/*[EOF]*/
