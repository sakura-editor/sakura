/*!	@file
	@brief ����v���r���[�Ǘ��N���X

	@author YAZAKI
	@date 2002/1/11 �V�K�쐬
*/
/*
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, �����

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef SAKURA_CPRINTPREVIEW_4FBD8BE8_4E93_4714_A3F2_F69081A2EDBDR_H_
#define SAKURA_CPRINTPREVIEW_4FBD8BE8_4E93_4714_A3F2_F69081A2EDBDR_H_

#include <Windows.h> // 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "CPrint.h" // 2002/2/10 aroka

class CColorStrategy;
class CColorStrategyPool;
class CEditWnd;
class CLayout;
class CLayoutMgr;

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
	void OnPreviewGoDirectPage( void );
	void OnPreviewZoom( BOOL bZoomUp );
	void OnPrint( void );	/* ������s */
	BOOL OnPrintPageSetting( void );
	void OnCheckAntialias( void );

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
	HWND GetPrintPreviewBarHANDLE_Safe() const{ if(!this)return NULL; else return m_hwndPrintPreviewBar; } //!< this��NULL�ł����s�ł���ŁB2007.10.29 kobake
	
	//	PrintPreview�o�[�̃��b�Z�[�W�����B
	//	�܂�PrintPreviewBar_DlgProc�Ƀ��b�Z�[�W���͂��ADispatchEvent_PPB�ɓ]������d�g��
	static INT_PTR CALLBACK PrintPreviewBar_DlgProc(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);
	INT_PTR DispatchEvent_PPB(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam 		// second message parameter
	);

protected:
	/*
	||	�`��B
	||	DrawXXXXX()�́A���݂̃t�H���g�𔼊p�t�H���g�ɐݒ肵�Ă���Ăяo�����ƁB
	||	�܂��ADrawXXXXX()���甲���Ă����Ƃ��́A���p�t�H���g�ɐݒ肳��Ă��邱�Ƃ����҂��Ă悢�B
	||	�t�H���g�́A���p�t�H���g�ƑS�p�t�H���g�����Ȃ����Ƃ����҂��Ă悢�B
	*/
	void DrawHeader( HDC hdc, const CMyRect& rect );
	void DrawPageText( HDC, int, int, int, class CDlgCancel* );
	void DrawFooter( HDC hdc, const CMyRect& rect );

	// ����^�v���r���[ �s�`��
	CColorStrategy* Print_DrawLine(
		HDC				hdc,
		POINT			ptDraw,		//!< �`����W�BHDC�����P�ʁB
		const wchar_t*	pLine,
		int				nDocLineLen,
		int				nLineStart,
		int				nLineLen,
		CLayoutInt		nIndent,	//!< �܂�Ԃ��C���f���g���� // 2006.08.14 Moca
		const CLayout*	pcLayout = NULL,	//!< �F�t�pLayout
		CColorStrategy*	pStrategyStart = NULL
	);

	// ����^�v���r���[ �u���b�N�`��
	void Print_DrawBlock(
		HDC				hdc,
		POINT			ptDraw,		//!< �`����W�BHDC�����P�ʁB
		const wchar_t*	pPhysicalLine,
		int				nBlockLen,
		int				nKind,		//< 0:���p, 1:�S�p
		const CLayout*	pcLayout,	//!< �F�ݒ�pLayout
		const CColorStrategy*	pStrategy,
		int				nBgnPhysical,
		CLayoutInt		nLayoutX,
		int				nDx,
		const int*		pDxArray
	);

	// �w�胍�W�b�N�ʒu��CColorStrategy���擾
	CColorStrategy* GetColorStrategy(
		const CStringRef&	cStringLine,
		int					iLogic,
		CColorStrategy*		pStrategy
	);

	// ����p�t�H���g���쐬����
	void CtrateFonts(HDC hdc);
	// ����p�t�H���g��j������
	void DestructFonts();

public:
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
	BOOL GetDefaultPrinterInfo(){ return m_cPrint.GetDefaultPrinter( &m_pPrintSetting->m_mdmDevMode ); }
	int  GetCurPageNum(){ return m_nCurPageNum; }	/* ���݂̃y�[�W */
	int  GetAllPageNum(){ return m_nAllPageNum; }	/* ���݂̃y�[�W */

	
	/*
	||	�w�b�_�E�t�b�^
	*/
	void SetHeader(char* pszWork[]);	//	&f�Ȃǂ�o�^
	void SetFooter(char* pszWork[]);	//	&p/&P�Ȃǂ�o�^

protected:
	void SetPreviewFontHan( const LOGFONT* lf );
	void SetPreviewFontZen( const LOGFONT* lf );

/* �����o�ϐ��錾 */
public:
	/* none */

protected:
	CEditWnd*		m_pParentWnd;	//	�e��CEditDoc*�B

	// 2006.08.17 Moca YAZAKI����̃����̒ʂ�DC/BMP��CEditDoc����CPrintPreview�ֈړ�
	HDC				m_hdcCompatDC;	//!< �ĕ`��p�R���p�`�u��DC
	HBITMAP			m_hbmpCompatBMP;	//!< �ĕ`��p������BMP
	HBITMAP			m_hbmpCompatBMPOld;	//!< �ĕ`��p������BMP(OLD)
	int				m_nbmpCompatScale;	//!< BMP�̉�ʂ�10(COMPAT_BMP_BASE)�s�N�Z�����������BMP�̃s�N�Z����

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
	short			m_nPreview_PaperAllWidth;	/* �p����(1/10mm�P��) */
	short			m_nPreview_PaperAllHeight;	/* �p������(1/10mm�P��) */
	short			m_nPreview_PaperWidth;		/* �p������L����(1/10mm�P��) */
	short			m_nPreview_PaperHeight;		/* �p������L������(1/10mm�P��) */
	short			m_nPreview_PaperOffsetLeft;	/* �p���]�����[(1/10mm�P��) */
	short			m_nPreview_PaperOffsetTop;	/* �p���]����[(1/10mm�P��) */
	CLayoutInt		m_bPreview_EnableColms;		/* �󎚉\����/�y�[�W */
	int				m_bPreview_EnableLines;		/* �󎚉\�s��/�y�[�W */
	int				m_nPreview_LineNumberColmns;	/* �s�ԍ��G���A�̕��i�������j */
	WORD			m_nAllPageNum;				/* �S�y�[�W�� */
	WORD			m_nCurPageNum;				/* ���݂̃y�[�W */

	PRINTSETTING*	m_pPrintSetting;			/* ���݂̈���ݒ� */
	LOGFONT			m_lfPreviewHan;				/* �v���r���[�p�t�H���g */
	LOGFONT			m_lfPreviewZen;				/* �v���r���[�p�t�H���g */

	HFONT			m_hFontHan;					// ����p���p�t�H���g�n���h��
	HFONT			m_hFontHan_b;				// ����p���p�t�H���g�n���h�� ����
	HFONT			m_hFontHan_u;				// ����p���p�t�H���g�n���h�� ����
	HFONT			m_hFontHan_bu;				// ����p���p�t�H���g�n���h�� �����A����
	HFONT			m_hFontZen;					// ����p�S�p�t�H���g�n���h��
	HFONT			m_hFontZen_b;				// ����p�S�p�t�H���g�n���h�� ����
	HFONT			m_hFontZen_u;				// ����p�S�p�t�H���g�n���h�� ����
	HFONT			m_hFontZen_bu;				// ����p�S�p�t�H���g�n���h�� �����A����
	int				m_nAscentHan;				// ���p�����̃A�Z���g�i������/����C������̍����j
	int				m_nAscentZen;				// �S�p�����̃A�Z���g�i������/����C������̍����j

	CColorStrategyPool*	m_pool;					// �F��`�Ǘ����

	class CLayoutMgr*	m_pLayoutMgr_Print;		/* ����p�̃��C�A�E�g�Ǘ���� */

	// �v���r���[����o�Ă����݂̃v�����^�����L�����Ă�����悤��static�ɂ��� 2003.05.02 ����� 
	static CPrint	m_cPrint;					//!< ���݂̃v�����^���
};

#endif /* SAKURA_CPRINTPREVIEW_4FBD8BE8_4E93_4714_A3F2_F69081A2EDBDR_H_ */
