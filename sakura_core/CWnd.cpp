/*!	@file
	@brief �E�B���h�E�̊�{�N���X

	@author Norio Nakatani
	@date 2000/01/11 �V�K�쐬
*/
/*
	Copyright (C) 2000-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CWnd.h"

CWnd* gm_pCWnd = NULL;


#ifndef	WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif


/* CWnd�E�B���h�E���b�Z�[�W�̃R�[���o�b�N�֐� */
LRESULT CALLBACK CWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CWnd* pCWnd;
//	CREATESTRUCT* lpcs;
	if( NULL != gm_pCWnd
	 && NULL == ::GetWindowLongPtr( hwnd, GWLP_USERDATA ) // Modified by KEITA for WIN64 2003.9.6
	){
		pCWnd = gm_pCWnd;
		/* �N���X�I�u�W�F�N�g�̃|�C���^���g���E�B���h�E�������Ɋi�[���Ă��� */
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)pCWnd );
		pCWnd->m_hWnd = hwnd;
		gm_pCWnd = NULL;
	}else{
		/* �N���X�I�u�W�F�N�g�̃|�C���^���g���E�B���h�E������������o�� */
		// Modified by KEITA for WIN64 2003.9.6
		pCWnd = (CWnd*)::GetWindowLongPtr( hwnd, GWLP_USERDATA );
	}
	if( NULL != pCWnd ){
		/* �N���X�I�u�W�F�N�g�̃|�C���^���g���ă��b�Z�[�W��z������ */
		return pCWnd->DispatchEvent( hwnd, uMsg, wParam, lParam );
	}else{
		/* �ӂ��͂����ɂ͗��Ȃ� */
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}


CWnd::CWnd()
{
	strcpy( m_szClassInheritances, "CWnd" );
	m_hInstance = NULL;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = NULL;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	m_hWnd = NULL;			/* ���̃E�B���h�E�̃n���h�� */
	return;
}

CWnd::~CWnd()
{
	if( ::IsWindow( m_hWnd ) ){
		/* �N���X�I�u�W�F�N�g�̃|�C���^��NULL�ɂ��Ċg���E�B���h�E�������Ɋi�[���Ă��� */
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( m_hWnd, GWLP_USERDATA, (LONG_PTR)NULL );
		::DestroyWindow( m_hWnd );
	}
	m_hWnd = NULL;
	return;
}


/* ������ */
void CWnd::Init(
	HINSTANCE	hInstance,		// handle to application instance
	HWND		hwndParent // handle to parent or owner window
)
{
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;
	return;
}


/* �E�B���h�E�N���X�쐬 */
ATOM CWnd::RegisterWC(
	/* WNDCLASS�p */
	HICON		hIcon,			// Handle to the class icon.
	HICON		hIconSm,		// Handle to a small icon
	HCURSOR		hCursor,		// Handle to the class cursor.
	HBRUSH		hbrBackground,	// Handle to the class background brush.
	LPCTSTR		lpszMenuName,	// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
	LPCTSTR		lpszClassName	// Pointer to a null-terminated string or is an atom.
)
{
	/* �E�B���h�E�N���X�̓o�^ */
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	//	Apr. 27, 2000 genta
	//	�T�C�Y�ύX���̂������}���邽��CS_HREDRAW | CS_VREDRAW ���O����
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = CWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 32;
	wc.hInstance = m_hInstance;
	wc.hIcon = hIcon;
	wc.hCursor = hCursor;
	wc.hbrBackground = hbrBackground;
	wc.lpszMenuName = lpszMenuName;
	wc.lpszClassName = lpszClassName;
	wc.hIconSm = hIconSm;
	return ::RegisterClassEx( &wc );
}

/* �쐬 */
HWND CWnd::Create(
	/* CreateWindowEx()�p */
	DWORD		dwExStyle, // extended window style
	LPCTSTR		lpszClassName,	// Pointer to a null-terminated string or is an atom.
	LPCTSTR		lpWindowName, // pointer to window name
	DWORD		dwStyle, // window style
	int			x, // horizontal position of window
	int			y, // vertical position of window
	int			nWidth, // window width
	int			nHeight, // window height
	HMENU		hMenu // handle to menu, or child-window identifier
)
{
	/* �E�B���h�E�쐬�O�̏���(�N���X�o�^�O) ( virtual )*/
	PreviCreateWindow();

	/* �����E�B���h�E�T�C�Y */
	/* �E�B���h�E�̍쐬 */
	gm_pCWnd = this;
	m_hWnd = ::CreateWindowEx(
		dwExStyle, // extended window style
		lpszClassName, // pointer to registered class name
		lpWindowName, // pointer to window name
		dwStyle, // window style
		x, // horizontal position of window
		y, // vertical position of window
		nWidth, // window width
		nHeight, // window height
		m_hwndParent, // handle to parent or owner window
		hMenu, // handle to menu, or child-window identifier
		m_hInstance, // handle to application instance
		(LPVOID)this	// pointer to window-creation data
	);
	if( NULL == m_hWnd ){
		::MessageBox( m_hwndParent, "CWnd::Create()\n\n::CreateWindowEx failed.", "error", MB_OK );
		return NULL;
	}

	/* �E�B���h�E�쐬��̏��� */
	AfterCreateWindow();
	return m_hWnd;
}


/* ���b�Z�[�W�z�� */
LRESULT CWnd::DispatchEvent( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	#define CALLH(message, method) case message: return method( hwnd, msg, wp, lp )
	switch( msg ){
	CALLH( WM_CREATE			, OnCreate			);
	CALLH( WM_CLOSE				, OnClose			);
	CALLH( WM_DESTROY			, OnDestroy			);
	CALLH( WM_SIZE				, OnSize			);
	CALLH( WM_MOVE				, OnMove			);
	CALLH( WM_COMMAND			, OnCommand			);
	CALLH( WM_LBUTTONDOWN		, OnLButtonDown		);
	CALLH( WM_LBUTTONUP			, OnLButtonUp		);
	CALLH( WM_LBUTTONDBLCLK		, OnLButtonDblClk	);
	CALLH( WM_RBUTTONDOWN		, OnRButtonDown		);
	CALLH( WM_RBUTTONUP			, OnRButtonUp		);
	CALLH( WM_RBUTTONDBLCLK		, OnRButtonDblClk	);
	CALLH( WM_MBUTTONDOWN		, OnMButtonDown		);
	CALLH( WM_MBUTTONUP			, OnMButtonUp		);
	CALLH( WM_MBUTTONDBLCLK		, OnMButtonDblClk	);
	CALLH( WM_MOUSEMOVE			, OnMouseMove		);
	CALLH( WM_MOUSEWHEEL		, OnMouseWheel		);
	CALLH( WM_PAINT				, OnPaint			);
	CALLH( WM_TIMER				, OnTimer			);
	CALLH( WM_QUERYENDSESSION	, OnQueryEndSession	);

	CALLH( WM_MEASUREITEM		, OnMeasureItem		);
	CALLH( WM_MENUCHAR			, OnMenuChar		);
	CALLH( WM_NOTIFY			, OnNotify			);	//@@@ 2003.05.31 MIK
	CALLH( WM_DRAWITEM			, OnDrawItem		);	// 2006.02.01 ryoji
	CALLH( WM_CAPTURECHANGED	, OnCaptureChanged	);	// 2006.11.30 ryoji

//	CALLH( WM_NCCREATE			, OnNcCreate		);
	CALLH( WM_NCDESTROY			, OnNcDestroy		);
//	CALLH( WM_NCCALCSIZE		, OnNcCalcSize		);
//	CALLH( WM_NCHITTEST			, OnNcHitTest		);
//	CALLH( WM_NCPAINT			, OnNcPaint			);
//	CALLH( WM_NCACTIVATE		, OnNcActivate		);
//	CALLH( WM_NCMOUSEMOVE		, OnNcMouseMove		);
//	CALLH( WM_NCLBUTTONDOWN		, OnNcLButtonDown	);
//	CALLH( WM_NCLBUTTONUP		, OnNcLButtonUp		);
//	CALLH( WM_NCLBUTTONDBLCLK	, OnNcLButtonDblClk	);
//	CALLH( WM_NCRBUTTONDOWN		, OnNcRButtonDown	);
//	CALLH( WM_NCRBUTTONUP		, OnNcRButtonUp		);
//	CALLH( WM_NCRBUTTONDBLCLK	, OnNcRButtonDblClk	);
//	CALLH( WM_NCMBUTTONDOWN		, OnNcMButtonDown	);
//	CALLH( WM_NCMBUTTONUP		, OnNcMButtonUp		);
//	CALLH( WM_NCMBUTTONDBLCLK	, OnNcMButtonDblClk	);


	/* MDI�p */
	CALLH( WM_MDIACTIVATE		, OnMDIActivate		);
//	CALLH( WM_MDICASCADE		, OnMDICascade		);
//	CALLH( WM_MDICREATE			, OnMDICreate		);
//	CALLH( WM_MDIDESTROY		, OnMDIDestroy		);
//	CALLH( WM_MDIGETACTIVE		, OnMDIGetActive	);
//	CALLH( WM_MDIICONARRANGE	, OnMDIIconArrange	);
//	CALLH( WM_MDIMAXIMIZE		, OnMDIMaximize		);
//	CALLH( WM_MDINEXT			, OnMDINext			);
//	CALLH( WM_MDIREFRESHMENU	, OnMDIRefreshMenu	);
//	CALLH( WM_MDIRESTORE		, OnMDIRestore		);
//	CALLH( WM_MDISETMENU		, OnMDISetMenu		);
//	CALLH( WM_MDITILE			, OnMDITile			);
	default:
		if( WM_APP <= msg && msg <= 0xBFFF ){
			/* �A�v���P�[�V������`�̃��b�Z�[�W(WM_APP <= msg <= 0xBFFF) */
			return DispatchEvent_WM_APP( hwnd, msg, wp, lp );
		}
		break;	/* default */
	}
	return CallDefWndProc( hwnd, msg, wp, lp );
}

/* �A�v���P�[�V������`�̃��b�Z�[�W(WM_APP <= msg <= 0xBFFF) */
LRESULT CWnd::DispatchEvent_WM_APP( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	return CallDefWndProc( hwnd, msg, wp, lp );
}



// WM_NCDESTROY
LRESULT CWnd::OnNcDestroy( HWND, UINT, WPARAM, LPARAM )
{
	m_hWnd = NULL;
	return 0L;
}

/* �f�t�H���g���b�Z�[�W���� */
LRESULT CWnd::CallDefWndProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	return ::DefWindowProc( hwnd, msg, wp, lp );
}


/*[EOF]*/
