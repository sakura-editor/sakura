/*!	@file
	@brief ����v���r���[�Ǘ��N���X

	@author YAZAKI
	@date 2002/1/11 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
class CPrintPreview;

#ifndef _CPRINTPREVIEW_H_
#define _CPRINTPREVIEW_H_
#include "CShareData.h"

class CPrintPreview {
/* �����o�֐��錾 */
public:
	/*
	||  �R���X�g���N�^
	*/
	CPrintPreview( class CEditWnd* pParentWnd );
	~CPrintPreview();
	
	/*
	||	�C�x���g
	*/
	//	Window Messages
	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );	/* �`�揈�� */
	LRESULT OnSize( WPARAM, LPARAM );				/* WM_SIZE ���� */
	LRESULT OnVScroll( WPARAM wParam, LPARAM lParam );
	LRESULT OnHScroll( WPARAM wParam, LPARAM lParam );
	LRESULT OnMouseMove( WPARAM wParam, LPARAM lParam );
	LRESULT OnMouseWheel( WPARAM wParam, LPARAM lParam );

	//	User Messages
	void OnChangePrintSetting( void );
	void OnPreviewGoPage( int nPage );	/* �v���r���[ �y�[�W�w�� */
	void OnPreviewGoPreviousPage(){ OnPreviewGoPage( m_nCurPageNum - 1 ); }		//	�O�̃y�[�W��
	void OnPreviewGoNextPage(){ OnPreviewGoPage( m_nCurPageNum + 1 ); }		//	�O�̃y�[�W��
	void OnPreviewZoom( BOOL bZoomUp );
	void OnPrint( void );	/* ������s */
	BOOL OnPrintPageSetting( void );

	/*
	||	�R���g���[��
	*/
	//	�X�N���[���o�[
	void InitPreviewScrollBar( void );
	
	//	PrintPreview�o�[�i��ʏ㕔�̃R���g���[���j
	void CreatePrintPreviewControls( void );
	void DestroyPrintPreviewControls( void );

	void SetFocusToPrintPreviewBar( void );
	HWND GetPrintPreviewBarHANDLE( void ){ return m_hwndPrintPreviewBar;	}
	
	//	PrintPreview�o�[�̃��b�Z�[�W�����B
	//	�܂�PrintPreviewBar_DlgProc�Ƀ��b�Z�[�W���͂��ADispatchEvent_PPB�ɓ]������d�g��
	static BOOL CALLBACK PrintPreviewBar_DlgProc(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);
	BOOL DispatchEvent_PPB(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam 		// second message parameter
	);

	/*
	||	�`��
	*/
	void DrawPageText( HDC, int, int, int, HFONT, class CDlgCancel* );
	/* ����^�v���r���[ �s�`�� */
	void Print_DrawLine(
		HDC			hdc,
		int			x,
		int			y,
		const char*	pLine,
		int			nLineLen,
		HFONT		hFontZen
	);
	//	�t�H���g��
	static int CALLBACK MyEnumFontFamProc(
		ENUMLOGFONT*	pelf,		// pointer to logical-font data
		NEWTEXTMETRIC*	pntm,		// pointer to physical-font data
		int				nFontType,	// type of font
		LPARAM			lParam 		// address of application-defined data
	);
	
	/*
	||	�A�N�Z�T
	*/
	void SetPrintSetting( PRINTSETTING* pPrintSetting ){ m_pPrintSetting = pPrintSetting; }
	BOOL GetDefaultPrinterInfo(){ return CPrint::GetDefaultPrinterInfo( &m_pPrintSetting->m_mdmDevMode ); };

	/*
	||	�w�b�_�E�t�b�^
	*/
	void SetHeader(char* pszWork[]);	//	&f�Ȃǂ�o�^
	void SetFooter(char* pszWork[]);	//	&p/&P�Ȃǂ�o�^

protected:
	void SetPreviewFontHan( LOGFONT* lf );
	void SetPreviewFontZen( LOGFONT* lf );

/* �����o�ϐ��錾 */
public:
	/* none */

protected:
	CEditWnd*		m_pParentWnd;	//	�e��CEditDoc*�B
	
	/*	�R���g���[������p	*/
	//	����o�[
	HWND			m_hwndPrintPreviewBar;	/* ����v���r���[ ����o�[ */
	//	�X�N���[���o�[
	int				m_nPreviewVScrollPos;	/* ����v���r���[�F�X�N���[���ʒu�c */
	int				m_nPreviewHScrollPos;	/* ����v���r���[�F�X�N���[���ʒu�� */
	BOOL			m_SCROLLBAR_HORZ;
	BOOL			m_SCROLLBAR_VERT;
	HWND			m_hwndVScrollBar;	/* �����X�N���[���o�[�E�B���h�E�n���h�� */
	HWND			m_hwndHScrollBar;	/* �����X�N���[���o�[�E�B���h�E�n���h�� */
	//	�T�C�Y�{�b�N�X
	HWND			m_hwndSizeBox;		/* �T�C�Y�{�b�N�X�E�B���h�E�n���h�� */
	BOOL			m_SizeBoxCanMove;	/* �T�C�Y�{�b�N�X�E�B���h�E�n���h���𓮂����邩�ǂ��� */
	
	//	�\��
	int				m_nPreview_Zoom;	/* ����v���r���[�F�{�� */
	
	//	����ʒu�����肷�邽�߂̕ϐ�
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
//	int				m_nPreview_PaperOffsetRight;	/* �p���]���E�[(1/10mm�P��) */
//	int				m_nPreview_PaperOffsetBottom;	/* �p���]�����[(1/10mm�P��) */
	int				m_bPreview_EnableColms;	/* �󎚉\����/�y�[�W */
	int				m_bPreview_EnableLines;	/* �󎚉\�s��/�y�[�W */
	int				m_nPreview_LineNumberColmns;	/* �s�ԍ��G���A�̕��i�������j */
	int				m_nAllPageNum;	/* �S�y�[�W�� */
	int				m_nCurPageNum;	/* ���݂̃y�[�W */

	PRINTSETTING*	m_pPrintSetting;	/* ���݂̈���ݒ� */
	LOGFONT			m_lfPreviewHan;	/* �v���r���[�p�t�H���g */
	LOGFONT			m_lfPreviewZen;	/* �v���r���[�p�t�H���g */

	class CLayoutMgr*	m_pLayoutMgr_Print;	/* ����p�̃��C�A�E�g�Ǘ���� */

	int				m_pnDx[10240 + 10];	/* ������`��p�������z�� */
};

#endif