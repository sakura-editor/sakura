/*!	@file
	@brief �^�u�E�B���h�E

	@author MIK
	@date 2003.5.30
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2004, MIK, Kazika
	Copyright (C) 2005, ryoji
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, ryoji
	Copyright (C) 2012, Moca, syat

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


#ifndef SAKURA_WINDOW_CTABWND_H_
#define SAKURA_WINDOW_CTABWND_H_

#include "CWnd.h"

class CGraphics;
struct EditNode;
struct DLLSHAREDATA;

//! �^�u�o�[�E�B���h�E
class CTabWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CTabWnd();
	virtual ~CTabWnd();

	/*
	|| �����o�֐�
	*/
	HWND Open( HINSTANCE, HWND );		/*!< �E�B���h�E �I�[�v�� */
	void Close( void );					/*!< �E�B���h�E �N���[�Y */
	void TabWindowNotify( WPARAM wParam, LPARAM lParam );
	void Refresh( BOOL bEnsureVisible = TRUE, BOOL bRebuild = FALSE );			// 2006.02.06 ryoji �����폜
	void NextGroup( void );			/* ���̃O���[�v */			// 2007.06.20 ryoji
	void PrevGroup( void );			/* �O�̃O���[�v */			// 2007.06.20 ryoji
	void MoveRight( void );			/* �^�u���E�Ɉړ� */		// 2007.06.20 ryoji
	void MoveLeft( void );			/* �^�u�����Ɉړ� */		// 2007.06.20 ryoji
	void Separate( void );			/* �V�K�O���[�v */			// 2007.06.20 ryoji
	void JoinNext( void );			/* ���̃O���[�v�Ɉړ� */	// 2007.06.20 ryoji
	void JoinPrev( void );			/* �O�̃O���[�v�Ɉړ� */	// 2007.06.20 ryoji

	LRESULT TabWndDispatchEvent( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	LRESULT TabListMenu( POINT pt, BOOL bSel = TRUE, BOOL bFull = FALSE, BOOL bOtherGroup = TRUE );	/*!< �^�u�ꗗ���j���[�쐬���� */	// 2006.03.23 fon

protected:
	/*
	|| �����w���p�n
	*/
	int FindTabIndexByHWND( HWND hWnd );
	void AdjustWindowPlacement( void );							/*!< �ҏW�E�B���h�E�̈ʒu���킹 */	// 2007.04.03 ryoji
	int SetCarmWindowPlacement( HWND hwnd, const WINDOWPLACEMENT* pWndpl );	/* �A�N�e�B�u���̏��Ȃ� SetWindowPlacement() �����s���� */	// 2007.11.30 ryoji
	void ShowHideWindow( HWND hwnd, BOOL bDisp );
	void HideOtherWindows( HWND hwndExclude );					/*!< ���̕ҏW�E�B���h�E���B�� */	// 2007.05.17 ryoji
	void ForceActiveWindow( HWND hwnd );
	void TabWnd_ActivateFrameWindow( HWND hwnd, bool bForce = true );	//2004.08.27 Kazika �����ǉ�
	HWND GetNextGroupWnd( void );	/* ���̃O���[�v�̐擪�E�B���h�E��T�� */	// 2007.06.20 ryoji
	HWND GetPrevGroupWnd( void );	/* �O�̃O���[�v�̐擪�E�B���h�E��T�� */	// 2007.06.20 ryoji
	void GetTabName( EditNode* pEditNode, BOOL bFull, BOOL bDupamp, LPTSTR pszName, int nLen );	/* �^�u���擾���� */	// 2007.06.28 ryoji �V�K�쐬

	/* ���z�֐� */
	virtual void AfterCreateWindow( void ){}	/*!< �E�B���h�E�쐬��̏��� */	// 2007.03.13 ryoji �������Ȃ�

	/* ���z�֐� ���b�Z�[�W���� */
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );		/*!< WM_SIZE���� */
	virtual LRESULT OnDestroy( HWND, UINT, WPARAM, LPARAM );	/*!< WM_DSESTROY���� */
	virtual LRESULT OnNotify( HWND, UINT, WPARAM, LPARAM );		/*!< WM_NOTIFY���� */
	virtual LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );		/*!< WM_PAINT���� */
	virtual LRESULT OnCaptureChanged( HWND, UINT, WPARAM, LPARAM );	/*!< WM_CAPTURECHANGED ���� */
	virtual LRESULT OnLButtonDown( HWND, UINT, WPARAM, LPARAM );	/*!< WM_LBUTTONDOWN���� */
	virtual LRESULT OnLButtonUp( HWND, UINT, WPARAM, LPARAM );	/*!< WM_LBUTTONUP���� */
	virtual LRESULT OnRButtonDown( HWND, UINT, WPARAM, LPARAM );	/*!< WM_RBUTTONDOWN���� */
	virtual LRESULT OnLButtonDblClk( HWND, UINT, WPARAM, LPARAM );	/*!< WM_LBUTTONDBLCLK���� */
	virtual LRESULT OnMouseMove( HWND, UINT, WPARAM, LPARAM );	/*!< WM_MOUSEMOVE���� */
	virtual LRESULT OnTimer( HWND, UINT, WPARAM, LPARAM );		/*!< WM_TIMER���� */
	virtual LRESULT OnMeasureItem( HWND, UINT, WPARAM, LPARAM );	/*!< WM_MEASUREITEM���� */
	virtual LRESULT OnDrawItem( HWND, UINT, WPARAM, LPARAM );		/*!< WM_DRAWITEM���� */

	// 2005.09.01 ryoji �h���b�O�A���h�h���b�v�Ń^�u�̏����ύX���\��
	/* �T�u�N���X������ Tab �ł̃��b�Z�[�W���� */
	LRESULT OnTabLButtonDown( WPARAM wParam, LPARAM lParam );	/*!< �^�u�� WM_LBUTTONDOWN ���� */
	LRESULT OnTabLButtonUp( WPARAM wParam, LPARAM lParam );		/*!< �^�u�� WM_LBUTTONUP ���� */
	LRESULT OnTabMouseMove( WPARAM wParam, LPARAM lParam );		/*!< �^�u�� WM_MOUSEMOVE ���� */
	LRESULT OnTabCaptureChanged( WPARAM wParam, LPARAM lParam );	/*!< �^�u�� WM_CAPTURECHANGED ���� */
	LRESULT OnTabRButtonDown( WPARAM wParam, LPARAM lParam );	/*!< �^�u�� WM_RBUTTONDOWN ���� */
	LRESULT OnTabRButtonUp( WPARAM wParam, LPARAM lParam );		/*!< �^�u�� WM_RBUTTONUP ���� */
	LRESULT OnTabMButtonDown( WPARAM wParam, LPARAM lParam );	/*!< �^�u�� WM_MBUTTONDOWN ���� */
	LRESULT OnTabMButtonUp( WPARAM wParam, LPARAM lParam );		/*!< �^�u�� WM_MBUTTONUP ���� */
	LRESULT OnTabNotify( WPARAM wParam, LPARAM lParam );		/*!< �^�u�� WM_NOTIFY ���� */

	//�����⏕�C���^�[�t�F�[�X
	void BreakDrag( void ) { if( ::GetCapture() == m_hwndTab ) ::ReleaseCapture(); m_eDragState = DRAG_NONE; }	/*!< �h���b�O��ԉ������� */
	BOOL ReorderTab( int nSrcTab, int nDstTab );	/*!< �^�u�����ύX���� */
	void BroadcastRefreshToGroup( void );
	BOOL SeparateGroup( HWND hwndSrc, HWND hwndDst, POINT ptDrag, POINT ptDrop );	/*!< �^�u�������� */	// 2007.06.20 ryoji
	LRESULT ExecTabCommand( int nId, POINTS pts );	/*!< �^�u�� �R�}���h���s���� */
	void LayoutTab( void );							/*!< �^�u�̃��C�A�E�g�������� */

	HIMAGELIST InitImageList( void );				/*!< �C���[�W���X�g�̏��������� */
	int GetImageIndex( EditNode* pNode );			/*!< �C���[�W���X�g�̃C���f�b�N�X�擾���� */
	HIMAGELIST ImageList_Duplicate( HIMAGELIST himl );	/*!< �C���[�W���X�g�̕������� */

	// 2006.02.01 ryoji �^�u�ꗗ��ǉ�
	void DrawBtnBkgnd( HDC hdc, const LPRECT lprcBtn, BOOL bBtnHilighted );	/*!< �{�^���w�i�`�揈�� */	// 2006.10.21 ryoji
	void DrawListBtn( CGraphics& gr, const LPRECT lprcClient );			/*!< �ꗗ�{�^���`�揈�� */
	void DrawCloseBtn( CGraphics& gr, const LPRECT lprcClient );			/*!< ����{�^���`�揈�� */		// 2006.10.21 ryoji
	void GetListBtnRect( const LPRECT lprcClient, LPRECT lprc );	/*!< �ꗗ�{�^���̋�`�擾���� */
	void GetCloseBtnRect( const LPRECT lprcClient, LPRECT lprc );	/*!< ����{�^���̋�`�擾���� */	// 2006.10.21 ryoji

	HFONT CreateMenuFont( void )
	{
		// ���j���[�p�t�H���g�쐬
		NONCLIENTMETRICS	ncm;
		// �ȑO�̃v���b�g�t�H�[���� WINVER >= 0x0600 �Œ�`�����\���̂̃t���T�C�Y��n���Ǝ��s����	// 2007.12.21 ryoji
		ncm.cbSize = CCSIZEOF_STRUCT( NONCLIENTMETRICS, lfMessageFont );
		::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, ncm.cbSize, (PVOID)&ncm, 0 );
		return ::CreateFontIndirect( &ncm.lfMenuFont );
	}

protected:
	enum DragState { DRAG_NONE, DRAG_CHECK, DRAG_DRAG };
	enum CaptureSrc { CAPT_NONE, CAPT_CLOSE };

	typedef HIMAGELIST (WINAPI *FN_ImageList_Duplicate)(HIMAGELIST himl);

	/*
	|| �����o�ϐ�
	*/
public:
	LPCTSTR			m_pszClassName;	/*!< �N���X�� */
	DLLSHAREDATA*	m_pShareData;	/*!< ���L�f�[�^ */
	HFONT			m_hFont;		/*!< �\���p�t�H���g */
	HWND			m_hwndTab;		/*!< �^�u�R���g���[�� */
	HWND			m_hwndToolTip;	/*!< �c�[���`�b�v�i�{�^���p�j */
	TCHAR			m_szTextTip[1024];	/*!< �c�[���`�b�v�̃e�L�X�g�i�^�u�p�j */

private:
	DragState	m_eDragState;			//!< �h���b�O���
	int			m_nSrcTab;				//!< �ړ����^�u
	POINT		m_ptSrcCursor;			//!< �h���b�O�J�n�J�[�\���ʒu
	HCURSOR		m_hDefaultCursor;		//!< �h���b�O�J�n���̃J�[�\��

	// 2006.01.28 ryoji �^�u�ւ̃A�C�R���\�����\��
	FN_ImageList_Duplicate	m_RealImageList_Duplicate;

	HIMAGELIST	m_hIml;					//!< �C���[�W���X�g
	HICON		m_hIconApp;				//!< �A�v���P�[�V�����A�C�R��
	HICON		m_hIconGrep;			//!< Grep�A�C�R��
	int			m_iIconApp;				//!< �A�v���P�[�V�����A�C�R���̃C���f�b�N�X
	int			m_iIconGrep;			//!< Grep�A�C�R���̃C���f�b�N�X

	BOOL		m_bVisualStyle;			//!< �r�W���A���X�^�C�����ǂ���	// 2007.04.01 ryoji
	BOOL		m_bHovering;
	BOOL		m_bListBtnHilighted;
	BOOL		m_bCloseBtnHilighted;	//!< ����{�^���n�C���C�g���	// 2006.10.21 ryoji
	CaptureSrc	m_eCaptureSrc;			//!< �L���v�`���[��
	BOOL		m_bTabSwapped;			//!< �h���b�O���Ƀ^�u�̓���ւ������������ǂ���
	LONG*		m_nTabBorderArray;		//!< �h���b�O�O�̃^�u���E�ʒu�z��
	LOGFONT		m_lf;					//!< �\���t�H���g�̓������
};

#endif /* SAKURA_WINDOW_CTABWND_H_ */



