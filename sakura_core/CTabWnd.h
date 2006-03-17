//	$Id$
/*!	@file
	@brief �^�u�E�B���h�E

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2004, MIK
	Copyright (C) 2005, ryoji

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

class CTabWnd;

#ifndef _CTABWND_H_
#define _CTABWND_H_

#include "CWnd.h"
#include "CEditDoc.h"
#include "CShareData.h"
#include <string>
#include <map>

//! �t�@���N�V�����L�[�E�B���h�E
class SAKURA_CORE_API CTabWnd : public CWnd
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
	void ForceActiveWindow( HWND hwnd );
	void TabWnd_ActivateFrameWindow( HWND hwnd, bool bForce = true );	//2004.08.27 Kazika �����ǉ�

	LRESULT TabWndDispatchEvent( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	/*
	|| �����o�ϐ�
	*/
	LPCTSTR			m_pszClassName;	/*!< �N���X�� */
	DLLSHAREDATA*	m_pShareData;	/*!< ���L�f�[�^ */
	HFONT			m_hFont;		/*!< �\���p�t�H���g */
	HWND			m_hwndTab;		/*!< �^�u�R���g���[�� */
	HWND			m_hwndToolTip;	/*!< �c�[���`�b�v */
	TCHAR			m_szTextTip1[1024];
#ifdef UNICODE
	//�����݈����Ă��镶���R�[�h�Ƃ͋t�̕����R�[�h���w�肷��B
	char			m_szTextTip2[1024];	//!< SJIS������ł̃c�[���`�b�v
#else
	//�����݈����Ă��镶���R�[�h�Ƃ͋t�̕����R�[�h���w�肷��B
	wchar_t			m_szTextTip2[1024];	//!< UNICODE������ł̃c�[���`�b�v
#endif	//UNICODE

protected:
	/*
	|| �����w���p�n
	*/
	void Refresh( void );	// 2006.02.06 ryoji �����폜
	int FindTabIndexByHWND( HWND hWnd );
	void ShowHideWindow( HWND hwnd, BOOL bDisp );
	int GetFirstOpenedWindow( void );

	/* ���z�֐� ���b�Z�[�W���� */
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );		/*!< WM_SIZE���� */
	virtual LRESULT OnDestroy( HWND, UINT, WPARAM, LPARAM );	/*!< WM_DSESTROY���� */
	virtual LRESULT OnNotify( HWND, UINT, WPARAM, LPARAM );		/*!< WM_NOTIFY���� */
	virtual LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );		/*!< WM_PAINT���� */
	virtual LRESULT OnLButtonDown( HWND, UINT, WPARAM, LPARAM );	/*!< WM_LBUTTONDOWN���� */
	virtual LRESULT OnRButtonDown( HWND, UINT, WPARAM, LPARAM );	/*!< WM_RBUTTONDOWN���� */
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

	void BreakDrag( void ) { if( ::GetCapture() == m_hwndTab ) ::ReleaseCapture(); m_eDragState = DRAG_NONE; }	/*!< �h���b�O��ԉ������� */
	BOOL ReorderTab( int nSrcTab, int nDstTab );	/*!< �^�u�����ύX���� */
	LRESULT ExecTabCommand( int nId, POINTS pts );	/*!< �^�u�� �R�}���h���s���� */
	void LayoutTab( void );							/*!< �^�u�̃��C�A�E�g�������� */

	enum DragState { DRAG_NONE, DRAG_CHECK, DRAG_DRAG };

	DragState m_eDragState;		 //!< �h���b�O���
	int	m_nSrcTab;				 //!< �ړ����^�u

	// 2006.01.28 ryoji �^�u�ւ̃A�C�R���\�����\��
	HIMAGELIST (WINAPI *m_RealImageList_Duplicate)(HIMAGELIST himl);
	HIMAGELIST m_hIml;								/*!< �C���[�W���X�g */
	HICON m_hIconApp;								/*!< �A�v���P�[�V�����A�C�R�� */
	HICON m_hIconGrep;								/*!< Grep�A�C�R�� */
	int m_iIconApp;									/*!< �A�v���P�[�V�����A�C�R���̃C���f�b�N�X */
	int m_iIconGrep;								/*!< Grep�A�C�R���̃C���f�b�N�X */
	HIMAGELIST InitImageList( void );				/*!< �C���[�W���X�g�̏��������� */
	int GetImageIndex( EditNode* pNode );			/*!< �C���[�W���X�g�̃C���f�b�N�X�擾���� */
	HIMAGELIST ImageList_Duplicate( HIMAGELIST himl );	/*!< �C���[�W���X�g�̕������� */

	// 2006.02.01 ryoji �^�u�ꗗ��ǉ�
	void DrawListBtn( HDC hdc, const LPRECT lprcClient );			/*!< �ꗗ�{�^���`�揈�� */
	void GetListBtnRect( const LPRECT lprcClient, LPRECT lprc );	/*!< �ꗗ�{�^���̋�`�擾���� */
	LRESULT OnListBtnClick( POINTS pts, BOOL bLeft );				/*!< �ꗗ�{�^���N���b�N���� */
	BOOL m_bHovering;
	BOOL m_bListBtnHilighted;
	HFONT CreateMenuFont( void )
	{
		// ���j���[�p�t�H���g�쐬
		NONCLIENTMETRICS	ncm;
		ncm.cbSize = sizeof( NONCLIENTMETRICS );
		::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), (PVOID)&ncm, 0 );
		return ::CreateFontIndirect( &ncm.lfMenuFont );
	}
};

#endif /* _CTABWND_H_ */

/*[EOF]*/
