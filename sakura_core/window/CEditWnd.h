/*!	@file
	@brief �ҏW�E�B���h�E�i�O�g�j�Ǘ��N���X

	@author Norio Nakatani
	@date 1998/05/13 �V�K�쐬
	@date 2002/01/14 YAZAKI PrintPreview�̕���
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001-2002, YAZAKI
	Copyright (C) 2002, aroka, genta, MIK
	Copyright (C) 2003, MIK, genta, wmlhq
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta, Moca
	Copyright (C) 2006, ryoji, aroka, fon, yukihane
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITWND_H_
#define _CEDITWND_H_

class CEditWnd;

#include <shellapi.h>// HDROP
#include "doc/CEditDoc.h"
#include "env/CShareData.h"
#include "func/CFuncKeyWnd.h"
#include "CTabWnd.h"	//@@@ 2003.05.31 MIK
#include "CMenuDrawer.h"
#include "CMainToolBar.h"
#include "CMainStatusBar.h"
#include "view/CEditView.h"
#include "window/CSplitterWnd.h"
#include "dlg/CDlgFind.h"
#include "dlg/CDlgReplace.h"
#include "dlg/CDlgJump.h"
#include "dlg/CDlgGrep.h"
#include "outline/CDlgFuncList.h"
#include "dlg/CDlgOpenFile.h"
#include "CHokanMgr.h"
#include "util/design_template.h"
#include "doc/CDocListener.h"

//by �S
#include"CDropTarget.h"

const static int MENUBAR_MESSAGE_MAX_LEN = 30;

//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
class CPrintPreview;// 2002/2/10 aroka
class CDropTarget;


//���C���E�B���h�E���R���g���[��ID
#define IDT_EDIT		455  // 20060128 aroka
#define IDT_TOOLBAR		456
#define IDT_CAPTION		457
#define IDT_FIRST_IDLE	458
#define IDT_SYSMENU		1357
#define ID_TOOLBAR		100

struct STabGroupInfo{
	HWND			hwndTop;
	WINDOWPLACEMENT	wpTop;

	STabGroupInfo() : hwndTop(NULL) { }
	bool IsValid() const{ return hwndTop!=NULL; }
};

//! �ҏW�E�B���h�E�i�O�g�j�Ǘ��N���X
// 2002.02.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
// 2007.10.30 kobake IsFuncEnable,IsFuncChecked��Funccode.h�Ɉړ�
// 2007.10.30 kobake OnHelp_MenuItem��CEditApp�Ɉړ�
class SAKURA_CORE_API CEditWnd
: public TSingleInstance<CEditWnd> //###
, public CDocListenerEx
{
public:
	/*
	||  Constructors
	*/
	CEditWnd();
	~CEditWnd();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �쐬                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	Mar. 7, 2002 genta �����^�C�v�p�����ǉ�
	// 2007.06.26 ryoji �O���[�v�w������ǉ�
	//! �쐬
	HWND Create(
		int				nGroup				//!< [in] �O���[�vID
	);
	void _GetTabGroupInfo(STabGroupInfo* pTabGroupInfo, int& nGroup);
	void _GetWindowRectForInit(CMyRect* rcResult, int nGroup, const STabGroupInfo& sTabGroupInfo);	//!< �E�B���h�E�����p�̋�`���擾
	HWND _CreateMainWindow(int nGroup, const STabGroupInfo& sTabGroupInfo);
	void _AdjustInMonitor(const STabGroupInfo& sTabGroupInfo);

	void OpenDocumentWhenStart(
		const SLoadInfo& sLoadInfo		//!< [in]
	);

	void SetDocumentTypeWhenCreate(
		ECodeType		nCharCode,							//!< [in] �����R�[�h
		bool			bViewMode,							//!< [in] �r���[���[�h�ŊJ�����ǂ���
		CTypeConfig	nDocumentType = CTypeConfig(-1)	//!< [in] �����^�C�v�D-1�̂Ƃ������w�薳���D
	);
	void UpdateCaption();
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �C�x���g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//�h�L�������g�C�x���g
	void OnAfterSave(const SSaveInfo& sSaveInfo);

	//�Ǘ�
	void MessageLoop( void );								/* ���b�Z�[�W���[�v */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ���b�Z�[�W���� */

	//�e��C�x���g
	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );	/* �`�揈�� */
	LRESULT OnSize( WPARAM, LPARAM );	/* WM_SIZE ���� */
	LRESULT OnLButtonUp( WPARAM, LPARAM );
	LRESULT OnLButtonDown( WPARAM, LPARAM );
	LRESULT OnMouseMove( WPARAM, LPARAM );
	LRESULT OnMouseWheel( WPARAM, LPARAM );
	BOOL DoMouseWheel( WPARAM wParam, LPARAM lParam );	// �}�E�X�z�C�[������	// 2007.10.16 ryoji
	LRESULT OnHScroll( WPARAM, LPARAM );
	LRESULT OnVScroll( WPARAM, LPARAM );
	int	OnClose();	/* �I�����̏��� */
	void OnDropFiles( HDROP );	/* �t�@�C�����h���b�v���ꂽ */
	BOOL OnPrintPageSetting( void );/* ����y�[�W�ݒ� */
	LRESULT OnTimer( WPARAM, LPARAM );	// WM_TIMER ����	// 2007.04.03 ryoji
	void OnEditTimer( void );	/* �^�C�}�[�̏��� */
	void OnCaptionTimer( void );
	void OnSysMenuTimer( void );
	void OnCommand( WORD, WORD , HWND );
	LRESULT OnNcLButtonDown(WPARAM, LPARAM);
	LRESULT OnNcLButtonUp(WPARAM, LPARAM);
	LRESULT OnLButtonDblClk(WPARAM, LPARAM);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ʒm                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//�t�@�C�����ύX�ʒm
	void ChangeFileNameNotify( const TCHAR* pszTabCaption, const TCHAR* pszFilePath, bool bIsGrep );	//@@@ 2003.05.31 MIK, 2006.01.28 ryoji �t�@�C�����AGrep���[�h�p�����[�^��ǉ�


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         ���j���[                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void InitMenu( HMENU, UINT, BOOL );
	void InitMenubarMessageFont(void);	//	���j���[�o�[�ւ̃��b�Z�[�W�\���@�\��CEditWnd���ڊ�	//	Dec. 4, 2002 genta
	LRESULT WinListMenu( HMENU hMenu, EditNode* pEditNodeArr, int nRowNum, BOOL bFull );	/*!< �E�B���h�E�ꗗ���j���[�쐬���� */	// 2006.03.23 fon
	LRESULT PopupWinList( bool bMousePos );	/*!< �E�B���h�E�ꗗ�|�b�v�A�b�v�\������ */	// 2006.03.23 fon	// 2007.02.28 ryoji �t���p�X�w��̃p�����[�^���폜


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ���`                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void LayoutToolBar( void );			/* �c�[���o�[�̔z�u���� */			// 2006.12.19 ryoji
	void LayoutFuncKey( void );			/* �t�@���N�V�����L�[�̔z�u���� */	// 2006.12.19 ryoji
	void LayoutTabBar( void );			/* �^�u�o�[�̔z�u���� */			// 2006.12.19 ryoji
	void LayoutStatusBar( void );		/* �X�e�[�^�X�o�[�̔z�u���� */		// 2006.12.19 ryoji
	void EndLayoutBars( BOOL bAdjust = TRUE );	/* �o�[�̔z�u�I������ */	// 2006.12.19 ryoji


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void PrintPreviewModeONOFF( void );	/* ����v���r���[���[�h�̃I��/�I�t */
	
	//�A�C�R��
	void SetWindowIcon( HICON, int);	//	Sep. 10, 2002 genta
	void GetDefaultIcon( HICON* hIconBig, HICON* hIconSmall ) const;	//	Sep. 10, 2002 genta
	bool GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const;	//	Sep. 10, 2002 genta
	void SetPageScrollByWheel( BOOL bState ) { m_bPageScrollByWheel = bState; };	// �z�C�[������ɂ��y�[�W�X�N���[���L����ݒ肷��iTRUE=����, FALSE=�Ȃ��j	// 2009.01.17 nasukoji
	void SetHScrollByWheel( BOOL bState ) { m_bHorizontalScrollByWheel = bState; };	// �z�C�[������ɂ�鉡�X�N���[���L����ݒ肷��iTRUE=����, FALSE=�Ȃ��j	// 2009.01.17 nasukoji
	void ClearMouseState( void );		// 2009.01.17 nasukoji	�}�E�X�̏�Ԃ��N���A����i�z�C�[���X�N���[���L����Ԃ��N���A�j


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ���                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//! ���A�v�����A�N�e�B�u���ǂ���	// 2007.03.08 ryoji
	BOOL IsActiveApp() const { return m_bIsActiveApp; };

	//!�c�[���`�b�v�̃e�L�X�g���擾�B2007.09.08 kobake �ǉ�
	void GetTooltipText(TCHAR* wszBuf, size_t nBufCount, int nID) const;

	//!����v���r���[�����ǂ���
	bool IsInPreviewMode()
	{
		return m_pPrintPreview!=NULL;
	}

	BOOL IsPageScrollByWheel() const { return m_bPageScrollByWheel; };		// �z�C�[������ɂ��y�[�W�X�N���[���L��	// 2009.01.17 nasukoji
	BOOL IsHScrollByWheel() const { return m_bHorizontalScrollByWheel; };	// �z�C�[������ɂ�鉡�X�N���[���L��		// 2009.01.17 nasukoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �\��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void PrintMenubarMessage( const TCHAR* msg );
	void SendStatusMessage( const TCHAR* msg );		//	Dec. 4, 2002 genta ���̂�CEditView����ړ�


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �E�B���h�E����                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void WindowTopMost( int ); // 2004.09.21 Moca

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �r���[�Ǘ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LRESULT Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void Views_RedrawAll();
	void Views_Redraw();
	void SetActivePane( int );	/* �A�N�e�B�u�ȃy�C����ݒ� */
	int GetActivePane( void ) const;	/* �A�N�e�B�u�ȃy�C�����擾 */ //2007.08.26 kobake const�ǉ�
	void SetDrawSwitchOfAllViews( bool bDraw );					/* ���ׂẴy�C���̕`��X�C�b�`��ݒ肷�� */	// 2008.06.08 ryoji
	void RedrawAllViews( CEditView* pcViewExclude );				/* ���ׂẴy�C����Redraw���� */
	void Views_DisableSelectArea(bool bRedraw);
	BOOL DetectWidthOfLineNumberAreaAllPane( bool bRedraw );	/* ���ׂẴy�C���ŁA�s�ԍ��\���ɕK�v�ȕ����Đݒ肷��i�K�v�Ȃ�ĕ`�悷��j */
	BOOL WrapWindowWidth( int nPane );	/* �E�[�Ő܂�Ԃ� */	// 2008.06.08 ryoji
	BOOL UpdateTextWrap( void );		/* �܂�Ԃ����@�֘A�̍X�V */	// 2008.06.10 ryoji
	//	Aug. 14, 2005 genta TAB���Ɛ܂�Ԃ��ʒu�̍X�V
	void ChangeLayoutParam( bool bShowProgress, CLayoutInt nTabSize, CLayoutInt nMaxLineKetas );
	//	Aug. 14, 2005 genta
	CLogicPoint* SavePhysPosOfAllView();
	void RestorePhysPosOfAllView( CLogicPoint* pptPosArray );
	// �݊�BMP�ɂ���ʃo�b�t�@ 2007.09.09 Moca
	void Views_DeleteCompatibleBitmap(); //!< CEditView�̉�ʃo�b�t�@���폜


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �e��A�N�Z�T                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	HWND			GetHwnd()		const	{ return this?m_hWnd:NULL; }
	CMenuDrawer&	GetMenuDrawer()			{ return m_CMenuDrawer; }
	CEditDoc&		GetDocument();
	const CEditDoc&	GetDocument() const;

	//�r���[
	const CEditView&	GetActiveView() const { return *m_pcEditViewArr[m_nActivePaneIndex]; }
	CEditView&			GetActiveView()       { return *m_pcEditViewArr[m_nActivePaneIndex]; }
	CEditView*			GetDragSourceView() const					{ return m_pcDragSourceView; }
	void				SetDragSourceView( CEditView* pcDragSourceView )	{ m_pcDragSourceView = pcDragSourceView; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �����⏕                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//by �S
protected:
	enum EIconClickStatus{
		icNone,
		icDown,
		icClicked,
		icDoubleClicked
	};

protected:
	//�h���b�v�_�E�����j���[
	int	CreateFileDropDownMenu( HWND );	//�J��(�h���b�v�_�E��)	//@@@ 2002.06.15 MIK

	//�^�C�}�[
	void Timer_ONOFF( BOOL ); /* �X�V�̊J�n�^��~ 20060128 aroka */

public:
	//! ��������m_nTimerCount���C���N�������g
	void IncrementTimerCount(int nInterval)
	{
		m_nTimerCount++;
		if( nInterval < m_nTimerCount ){
			m_nTimerCount = 0;
		}
	}

	//D&D�t���O�Ǘ�
	void SetDragPosOrg(CMyPoint ptDragPosOrg){ m_ptDragPosOrg=ptDragPosOrg; }
	void SetDragMode(bool bDragMode){ m_bDragMode = bDragMode; }
	bool GetDragMode() const{ return m_bDragMode; }
	const CMyPoint& GetDragPosOrg() const{ return m_ptDragPosOrg; }

	/* IDropTarget���� */	// 2008.06.20 ryoji
	STDMETHODIMP DragEnter( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragOver( DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragLeave( void );
	STDMETHODIMP Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );

	//�t�H�[�J�X�Ǘ�
	int GetCurrentFocus() const{ return m_nCurrentFocus; }
	void SetCurrentFocus(int n){ m_nCurrentFocus = n; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����o�ϐ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	//���E�B���h�E
	HWND			m_hWnd;

	//�e�E�B���h�E
	HWND			m_hwndParent;

public:
	//�q�E�B���h�E
	CMainToolBar	m_cToolbar;			//!< �c�[���o�[
	CTabWnd			m_cTabWnd;			//!< �^�u�E�C���h�E	//@@@ 2003.05.31 MIK
	CFuncKeyWnd		m_CFuncKeyWnd;		//!< �t�@���N�V�����o�[
	CMainStatusBar	m_cStatusBar;		//!< �X�e�[�^�X�o�[
	CPrintPreview*	m_pPrintPreview;	//!< ����v���r���[�\�����B�K�v�ɂȂ����Ƃ��̂݃C���X�^���X�𐶐�����B

	CSplitterWnd	m_cSplitterWnd;		//!< �����t���[��
	CEditView*		m_pcEditViewArr[4];	//!< �r���[
	int				m_nActivePaneIndex;	//!< �A�N�e�B�u�ȃr���[
	CEditView*		m_pcDragSourceView;	//!< �h���b�O���̃r���[

	//�_�C�A���O�B
	CDlgFind		m_cDlgFind;			// �u�����v�_�C�A���O
	CDlgReplace		m_cDlgReplace;		// �u�u���v�_�C�A���O
	CDlgJump		m_cDlgJump;			// �u�w��s�փW�����v�v�_�C�A���O
	CDlgGrep		m_cDlgGrep;			// Grep�_�C�A���O
	CDlgFuncList	m_cDlgFuncList;		// �A�E�g���C����͌��ʃ_�C�A���O
	CDlgOpenFile	m_cDlgOpenFile;		// �t�@�C���I�[�v���_�C�A���O
	CHokanMgr		m_cHokanMgr;		// ���͕⊮

private:
	//���L�f�[�^
	DLLSHAREDATA*	m_pShareData;

	//�w���p
	CMenuDrawer		m_CMenuDrawer;

	//���b�Z�[�WID
	UINT			m_uMSIMEReconvertMsg;
	UINT			m_uATOKReconvertMsg;

	//���
	bool			m_bIsActiveApp;		//!< ���A�v�����A�N�e�B�u���ǂ���	// 2007.03.08 ryoji
	LPTSTR			m_pszLastCaption;
	LPTSTR			m_pszMenubarMessage; //!< ���j���[�o�[�E�[�ɕ\�����郁�b�Z�[�W
	int				m_nTimerCount;		//!< OnTimer�p 2003.08.29 wmlhq
	int				m_nCurrentFocus;	//!< ���݂̃t�H�[�J�X���
	int				m_nWinSizeType;		//!< �T�C�Y�ύX�̃^�C�v�BSIZE_MAXIMIZED, SIZE_MINIMIZED ���B
	BOOL			m_bPageScrollByWheel;		//!< �z�C�[������ɂ��y�[�W�X�N���[������	// 2009.01.17 nasukoji
	BOOL			m_bHorizontalScrollByWheel;	//!< �z�C�[������ɂ�鉡�X�N���[������		// 2009.01.17 nasukoji

	//�t�H���g�E�C���[�W
	HFONT			m_hFontCaretPosInfo;		//!< �L�����b�g�̍s���ʒu�\���p�t�H���g
	int				m_nCaretPosInfoCharWidth;	//!< �L�����b�g�̍s���ʒu�\���p�t�H���g�̕�
	int				m_nCaretPosInfoCharHeight;	//!< �L�����b�g�̍s���ʒu�\���p�t�H���g�̍���

	//D&D�t���O
	bool			m_bDragMode;
	CMyPoint		m_ptDragPosOrg;
	CDropTarget*	m_pcDropTarget;

	//���̑��t���O
	BOOL				m_bUIPI;		// �G�f�B�^�|�g���C�Ԃł�UI���������m�F�p�t���O	// 2007.06.07 ryoji
	EIconClickStatus	m_IconClicked;

public:
	ESelectCountMode	m_nSelectCountMode; // �I�𕶎��J�E���g���@

};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITWND_H_ */





