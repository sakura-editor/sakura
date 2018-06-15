#include "StdAfx.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CSakuraEnvironment.h"
#include "util/os.h"
#include <limits.h>
#include "window.h"

int CDPI::nDpiX = 96;
int CDPI::nDpiY = 96;
bool CDPI::bInitialized = false;

/**	�w�肵���E�B���h�E�̑c��̃n���h�����擾����

	GetAncestor() API��Win95�Ŏg���Ȃ��̂ł��̂����

	WS_POPUP�X�^�C���������Ȃ��E�B���h�E�iex.CDlgFuncList�_�C�A���O�j���ƁA
	GA_ROOTOWNER�ł͕ҏW�E�B���h�E�܂ők��Ȃ��݂����BGetAncestor() API�ł����l�B
	�{�֐��ŗL�ɗp�ӂ���GA_ROOTOWNER2�ł͑k�邱�Ƃ��ł���B

	@author ryoji
	@date 2007.07.01 ryoji �V�K
	@date 2007.10.22 ryoji �t���O�l�Ƃ���GA_ROOTOWNER2�i�{�֐��ŗL�j��ǉ�
	@date 2008.04.09 ryoji GA_ROOTOWNER2 �͉\�Ȍ���c���k��悤�ɓ���C��
*/
HWND MyGetAncestor( HWND hWnd, UINT gaFlags )
{
	HWND hwndAncestor;
	HWND hwndDesktop = ::GetDesktopWindow();
	HWND hwndWk;

	if( hWnd == hwndDesktop )
		return NULL;

	switch( gaFlags )
	{
	case GA_PARENT:	// �e�E�B���h�E��Ԃ��i�I�[�i�[�͕Ԃ��Ȃ��j
		hwndAncestor = ( (DWORD)::GetWindowLongPtr( hWnd, GWL_STYLE ) & WS_CHILD )? ::GetParent( hWnd ): hwndDesktop;
		break;

	case GA_ROOT:	// �e�q�֌W��k���Ē��ߏ�ʂ̃g�b�v���x���E�B���h�E��Ԃ�
		hwndAncestor = hWnd;
		while( (DWORD)::GetWindowLongPtr( hwndAncestor, GWL_STYLE ) & WS_CHILD )
			hwndAncestor = ::GetParent( hwndAncestor );
		break;

	case GA_ROOTOWNER:	// �e�q�֌W�Ə��L�֌W��GetParent()�ők���ď��L����Ă��Ȃ��g�b�v���x���E�B���h�E��Ԃ�
		hwndWk = hWnd;
		do{
			hwndAncestor = hwndWk;
			hwndWk = ::GetParent( hwndAncestor );
		}while( hwndWk != NULL );
		break;

	case GA_ROOTOWNER2:	// ���L�֌W��GetWindow()�ők���ď��L����Ă��Ȃ��g�b�v���x���E�B���h�E��Ԃ�
		hwndWk = hWnd;
		do{
			hwndAncestor = hwndWk;
			hwndWk = ::GetParent( hwndAncestor );
			if( hwndWk == NULL )
				hwndWk = ::GetWindow( hwndAncestor, GW_OWNER );
		}while( hwndWk != NULL );
		break;

	default:
		hwndAncestor = NULL;
		break;
	}

	return hwndAncestor;
}


/*!
	�������̃��[�U�[������\�ɂ���
	�u���b�L���O�t�b�N(?)�i���b�Z�[�W�z��

	@date 2003.07.04 genta ���̌Ăяo���ŕ������b�Z�[�W����������悤��
*/
BOOL BlockingHook( HWND hwndDlgCancel )
{
	MSG		msg;
	BOOL	ret;
	//	Jun. 04, 2003 genta ���b�Z�[�W�����邾����������悤��
	while(( ret = (BOOL)::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) != 0 ){
		if ( msg.message == WM_QUIT ){
			return FALSE;
		}
		if( NULL != hwndDlgCancel && IsDialogMessage( hwndDlgCancel, &msg ) ){
		}else{
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		}
	}
	return TRUE/*ret*/;
}




bool GetVirtualSnapRect( HWND hWnd, RECT* prcSnap/* = NULL*/, RECT* prcUnsnap/* = NULL*/ )
{
	RECT rcSnap, rcUnsnap;

	// Note. Unsnap �T�C�Y�����łȂ� Snap �T�C�Y�� Window Byte �ɋL�����Ă��闝�R
	//   SetAeroSnap() ����r���̃E�B���h�E�͈ꎞ�I�� Unsnap �T�C�Y�ɂ���邪�A
	//   ���̃^�C�~���O�ŌĂ΂ꂽ�ꍇ�ɂ� Snap �T�C�Y��Ԃ���悤�ɁB
	rcSnap.left = ::GetWindowLong(hWnd, GWL_SNAP_LEFT);
	rcSnap.top = ::GetWindowLong(hWnd, GWL_SNAP_TOP);
	rcSnap.right = ::GetWindowLong(hWnd, GWL_SNAP_RIGHT);
	rcSnap.bottom = ::GetWindowLong(hWnd, GWL_SNAP_BOTTOM);

	rcUnsnap.left = ::GetWindowLong(hWnd, GWL_UNSNAP_LEFT);
	rcUnsnap.top = ::GetWindowLong(hWnd, GWL_UNSNAP_TOP);
	rcUnsnap.right = ::GetWindowLong(hWnd, GWL_UNSNAP_RIGHT);
	rcUnsnap.bottom = ::GetWindowLong(hWnd, GWL_UNSNAP_BOTTOM);

	bool bRet = (!::IsRectEmpty(&rcSnap) && !::IsRectEmpty(&rcUnsnap));
	if (bRet)
	{
		if (prcSnap) *prcSnap = rcSnap;
		if (prcUnsnap) *prcUnsnap = rcUnsnap;
	}
	return bRet;
}

void SetVirtualSnapRect( HWND hWnd, const RECT* prcSnap, const RECT* prcUnsnap )
{
	::SetWindowLong(hWnd, GWL_SNAP_LEFT, prcSnap->left);
	::SetWindowLong(hWnd, GWL_SNAP_TOP, prcSnap->top);
	::SetWindowLong(hWnd, GWL_SNAP_RIGHT, prcSnap->right);
	::SetWindowLong(hWnd, GWL_SNAP_BOTTOM, prcSnap->bottom);

	::SetWindowLong(hWnd, GWL_UNSNAP_LEFT, prcUnsnap->left);
	::SetWindowLong(hWnd, GWL_UNSNAP_TOP, prcUnsnap->top);
	::SetWindowLong(hWnd, GWL_UNSNAP_RIGHT, prcUnsnap->right);
	::SetWindowLong(hWnd, GWL_UNSNAP_BOTTOM, prcUnsnap->bottom);
}

void SetVirtualSnapRectEmpty( HWND hWnd )
{
	RECT rcEmpty;
	::SetRectEmpty(&rcEmpty);
	SetVirtualSnapRect(hWnd, &rcEmpty, &rcEmpty);
}

bool GetAeroSnapRect( HWND hWnd, RECT* prcSnap/* = NULL*/, RECT* prcUnsnap/* = NULL*/, bool bRealOnly/* = false*/ )
{
	if (IsZoomed(hWnd) || IsIconic(hWnd))
		return false;

	if (!bRealOnly)
	{
		if (GetVirtualSnapRect(hWnd, prcSnap, prcUnsnap))
			return true;
	}

	RECT rcWnd, rcWork, rcMon;
	::GetWindowRect(hWnd, &rcWnd);
	GetMonitorWorkRect(hWnd, &rcWork, &rcMon);
	::OffsetRect(&rcWnd, rcMon.left - rcWork.left, rcMon.top - rcWork.top);	// ���[�N�G���A���W�ɕϊ�
	WINDOWPLACEMENT wp = {sizeof(wp)};
	::GetWindowPlacement(hWnd, &wp);
	if (!::EqualRect(&wp.rcNormalPosition, &rcWnd))
	{
		if (prcUnsnap) *prcUnsnap = wp.rcNormalPosition;
		if (prcSnap) *prcSnap = rcWnd;
		return true;
	}

	return false;
}

bool SetAeroSnap( HWND hWnd )
{
	DLLSHAREDATA* pShareData = &GetDllShareData();
	if( !::IsWindowVisible(hWnd) || ::IsZoomed(hWnd) ||
		!(pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin) ){
		// Aero Snap ��Ԃւ̑J�ڂ��s�v�ɂȂ���
		SetVirtualSnapRectEmpty(hWnd);
		return true;
	}

	RECT rcSnap, rcUnsnap;
	if (!GetVirtualSnapRect(hWnd, &rcSnap, &rcUnsnap))	// �i���jrcSnap �͌��݂̈ʒu�Ɠ����̂͂�
		return true;

	if (GetWindowThreadProcessId(::GetForegroundWindow(), NULL) != GetWindowThreadProcessId(hWnd, NULL))
		return false;

	RECT rcWork, rcMon;
	GetMonitorWorkRect(hWnd, &rcWork, &rcMon);

	// Aero Snap ����ł���悤�ɁA�����I�ɃG�f�B�^�E�B���h�E���A�N�e�B�u������
	// �i���b�Z�[�W�{�b�N�X�\�����Ȃ� Aero Snap �s��Ԃł��ꎞ�I�ɉ\�ɂ���j
	HWND hWndActiveOld = ::GetActiveWindow();
	BOOL bEnableOld = ::IsWindowEnabled(hWnd);
	::EnableWindow(hWnd, TRUE);
	::SetActiveWindow(hWnd);

	// �ꎞ�I�� Snap ������̈ʒu�ɖ߂�
	::OffsetRect(&rcSnap, rcWork.left - rcMon.left, rcWork.top - rcMon.top);
	::OffsetRect(&rcUnsnap, rcWork.left - rcMon.left, rcWork.top - rcMon.top);
	::MoveWindow(hWnd, rcUnsnap.left, rcUnsnap.top, rcUnsnap.right - rcUnsnap.left, rcUnsnap.bottom - rcUnsnap.top, FALSE);

	// Shift, Ctrl, Alt �L�[�͗���
	INPUT in[3 + 4];
	ULONG_PTR dwExtraInfo = ::GetMessageExtraInfo();
	in[0].ki.wVk = VK_SHIFT;
	in[1].ki.wVk = VK_CONTROL;
	in[2].ki.wVk = VK_MENU;
	int i;
	for (i = 0; i < 3; i++)
	{
		in[i].type = INPUT_KEYBOARD;
		in[i].ki.wScan = ::MapVirtualKey(in[i].ki.wVk, 0);
		in[i].ki.dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
		in[i].ki.time = 0;
		in[i].ki.dwExtraInfo = dwExtraInfo;
	}
	// Aero Snap �L�[����𒍓�����
	INPUT* pin = &in[i];
	pin[0].ki.wVk = pin[3].ki.wVk = VK_LWIN;
	pin[1].ki.wVk = pin[2].ki.wVk = ((rcSnap.right + rcSnap.left) < (rcWork.right + rcWork.left))? VK_LEFT: VK_RIGHT;
	pin[0].ki.dwFlags = pin[1].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
	pin[2].ki.dwFlags = pin[3].ki.dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
	for (i = 0; i < 4; i++)
	{
		pin[i].type = INPUT_KEYBOARD;
		pin[i].ki.wScan = ::MapVirtualKey(pin[i].ki.wVk, 0);
		pin[i].ki.time = 0;
		pin[i].ki.dwExtraInfo = dwExtraInfo;
	}
	::SendInput(_countof(in), in, sizeof(in[0]));
	BlockingHook(NULL);

	// Snap �ʒu�ɖ߂�
	::ShowWindow(hWnd, SW_HIDE);
	::MoveWindow(hWnd, rcSnap.left, rcSnap.top, rcSnap.right - rcSnap.left, rcSnap.bottom - rcSnap.top, FALSE);
	::ShowWindow(hWnd, SW_SHOW);
	::SetActiveWindow(hWndActiveOld);
	::EnableWindow(hWnd, bEnableOld);

	SetVirtualSnapRectEmpty(hWnd);

	// Win 10 �ł� Aero Snap ����� Foreground ���O���̂ŋ����I�ɖ߂�
	::Sleep(0);	// ���܂��Ȃ��iForeground �������蓮�삵�܂��悤�Ɂj
	::SetForegroundWindow(hWnd);	// for Windows 10

	return true;
}

/** �t���[���E�B���h�E���A�N�e�B�u�ɂ���
	@date 2007.11.07 ryoji �Ώۂ�disable�̂Ƃ��͍ŋ߂̃|�b�v�A�b�v���t�H�A�O���E���h������D
		�i���[�_���_�C�A���O�⃁�b�Z�[�W�{�b�N�X��\�����Ă���悤�ȂƂ��j
*/
void ActivateFrameWindow( HWND hwnd )
{
	// �ҏW�E�B���h�E�Ń^�u�܂Ƃߕ\���̏ꍇ�͕\���ʒu�𕜌�����
	bool bAeroSnap = false;
	DLLSHAREDATA* pShareData = &GetDllShareData();
	if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ) {
		if( IsSakuraMainWindow( hwnd ) ){
			// ���ɃA�N�e�B�u�Ȃ牽�����Ȃ�
			// �� ���d�ɃA�N�e�B�u���iSetWindowPlacement�j����� AeroSnap ����������Ă��܂�
			//    �i��j�u������C/C++�w�b�_�i�\�[�X�j���J���v����s�v�Ȃ̂ɌĂ΂�Ă���
			HWND hwndActivate = ::IsWindowEnabled( hwnd )? hwnd: ::GetLastActivePopup( hwnd );
			if( hwndActivate == ::GetForegroundWindow() )
				return;

			if( pShareData->m_sFlags.m_bEditWndChanging )
				return;	// �ؑւ̍Œ�(busy)�͗v���𖳎�����
			pShareData->m_sFlags.m_bEditWndChanging = TRUE;	// �ҏW�E�B���h�E�ؑ֒�ON	2007.04.03 ryoji

			// �ΏۃE�B���h�E�̃X���b�h�Ɉʒu���킹���˗�����	// 2007.04.03 ryoji
			DWORD_PTR dwResult;
			::SendMessageTimeout(
				hwnd,
				MYWM_TAB_WINDOW_NOTIFY,
				TWNT_WNDPL_ADJUST,
				(LPARAM)NULL,
				SMTO_ABORTIFHUNG | SMTO_BLOCK,
				10000,
				&dwResult
			);
			bAeroSnap = GetVirtualSnapRect(hwnd);
		}
	}

	// �Ώۂ�disable�̂Ƃ��͍ŋ߂̃|�b�v�A�b�v���t�H�A�O���E���h������
	HWND hwndActivate;
	hwndActivate = ::IsWindowEnabled( hwnd )? hwnd: ::GetLastActivePopup( hwnd );
	if( ::IsIconic( hwnd ) ){
		::ShowWindow( hwnd, SW_RESTORE );
	}
	else if ( ::IsZoomed( hwnd ) ){
		::ShowWindow( hwnd, SW_MAXIMIZE );
	}
	else {
		// SW_SHOW -> SW_SHOWNA�ɕύX�iSW_SHOW ���Ɗ��� Aero Snap ��Ԃ̏ꍇ�ɉ�������Ă��܂��j
		// �i��j�t�@�C��1���J�����G�f�B�^1�� Snap ��ԂŋN�����Ă����ԂŁA
		//       �X�ɋN���p�����[�^�Ƀt�@�C��1���w�肵�ăG�f�B�^2���N�������
		//       �G�f�B�^2���G�f�B�^1���A�N�e�B�u�ɂ���ہASW_SHOW ���� Snap ��������Ă��܂����Ƃ�����
		//       ���G�f�B�^1����[�t�@�C��]-[�J��]����Ńt�@�C���_�C�A���O�̃t�@�C�����G�f�B�b�g�{�b�N�X
		//         �Ƀt�@�C��1�A�t�@�C��2��2�t�@�C������͂���[�J��]�{�^���������ƍČ�
		::ShowWindow( hwnd, SW_SHOWNA );
	}
	::SetForegroundWindow( hwndActivate );
	::BringWindowToTop( hwndActivate );

	// Aero Snap ��Ԃ̔��f�҂�
	// �i�܂Ƃ߂ĕ���ꍇ�ł� Aero Snap �������p�����悤�Ɂj
	if( bAeroSnap ){
		DWORD dwTid = GetWindowThreadProcessId(hwnd, NULL);
		if( dwTid != GetCurrentThreadId() && dwTid == GetWindowThreadProcessId(::GetForegroundWindow(), NULL) ){
			for(int iRetry = 0; iRetry < 40; iRetry++){
				if( !GetVirtualSnapRect(hwnd) )
					break;
				::Sleep(50);
			}
		}
	}

	if( pShareData )
		pShareData->m_sFlags.m_bEditWndChanging = FALSE;	// �ҏW�E�B���h�E�ؑ֒�OFF	2007.04.03 ryoji

	return;
}


CTextWidthCalc::CTextWidthCalc(HWND hParent, int nID)
{
	assert_warning(hParent);

	hwnd = ::GetDlgItem(hParent, nID);
	hDC = ::GetDC( hwnd );
	assert(hDC);
	hFont = (HFONT)::SendMessageAny(hwnd, WM_GETFONT, 0, 0);
	hFontOld = (HFONT)::SelectObject(hDC, hFont);
	nCx = 0;
	nExt = 0;
	bHDCComp = false;
	bFromDC = false;
}

CTextWidthCalc::CTextWidthCalc(HWND hwndThis)
{
	assert_warning(hwndThis);

	hwnd = hwndThis;
	hDC = ::GetDC( hwnd );
	assert(hDC);
	hFont = (HFONT)::SendMessageAny(hwnd, WM_GETFONT, 0, 0);
	hFontOld = (HFONT)::SelectObject(hDC, hFont);
	nCx = 0;
	nExt = 0;
	bHDCComp = false;
	bFromDC = false;
}

CTextWidthCalc::CTextWidthCalc(HFONT font)
{
	hwnd = 0;
	HDC hDCTemp = ::GetDC( NULL ); // Desktop
	hDC = ::CreateCompatibleDC( hDCTemp );
	::ReleaseDC( NULL, hDCTemp );
	assert(hDC);
	hFont = font;
	hFontOld = (HFONT)::SelectObject(hDC, hFont);
	nCx = 0;
	nExt = 0;
	bHDCComp = true;
	bFromDC = false;
}

CTextWidthCalc::CTextWidthCalc(HDC hdc)
{
	hwnd = 0;
	hDC = hdc;
	assert(hDC);
	nCx = 0;
	nExt = 0;
	bHDCComp = true;
	bFromDC = true;
}

CTextWidthCalc::~CTextWidthCalc()
{
	if(hDC && !bFromDC){
		::SelectObject(hDC, hFontOld);
		if( bHDCComp ){
			::DeleteDC(hDC);
		}else{
			::ReleaseDC(hwnd, hDC);
		}
		hwnd = 0;
		hDC = 0;
	}
}


bool CTextWidthCalc::SetWidthIfMax(int width)
{
	return SetWidthIfMax(0, INT_MIN);
}

bool CTextWidthCalc::SetWidthIfMax(int width, int extCx)
{
	if( INT_MIN == extCx ){
		extCx = nExt;
	}
	if( nCx < width + extCx ){
		nCx = width + extCx;
		return true;
	}
	return false;
}

bool CTextWidthCalc::SetTextWidthIfMax(LPCTSTR pszText)
{
	return SetTextWidthIfMax(pszText, INT_MIN);
}

bool CTextWidthCalc::SetTextWidthIfMax(LPCTSTR pszText, int extCx)
{
	SIZE size;
	if( ::GetTextExtentPoint32( hDC, pszText, _tcslen(pszText), &size ) ){
		return SetWidthIfMax(size.cx, extCx);
	}
	return false;
}

int CTextWidthCalc::GetTextWidth(LPCTSTR pszText) const
{
	SIZE size;
	if( ::GetTextExtentPoint32( hDC, pszText, _tcslen(pszText), &size ) ){
		return size.cx;
	}
	return 0;
}

int CTextWidthCalc::GetTextHeight() const
{
	TEXTMETRIC tm;
	::GetTextMetrics(hDC, &tm);
	return tm.tmHeight;
}

CFontAutoDeleter::CFontAutoDeleter()
	: m_hFontOld(NULL)
	, m_hFont(NULL)
	, m_hwnd(NULL)
{}

CFontAutoDeleter::~CFontAutoDeleter()
{
	if( m_hFont ){
		DeleteObject( m_hFont );
		m_hFont = NULL;
	}
}

void CFontAutoDeleter::SetFont( HFONT hfontOld, HFONT hfont, HWND hwnd )
{
	if( m_hFont ){
		::DeleteObject( m_hFont );
	}
	if( m_hFont != hfontOld ){
		m_hFontOld = hfontOld;
	}
	m_hFont = hfont;
	m_hwnd = hwnd;
}

/*! �E�B���h�E�̃����[�X(WM_DESTROY�p)
*/
void CFontAutoDeleter::ReleaseOnDestroy()
{
	if( m_hFont ){
		::DeleteObject( m_hFont );
		m_hFont = NULL;
	}
	m_hFontOld = NULL;
}

/*! �E�B���h�E�������̃����[�X
*/
#if 0
void CFontAutoDeleter::Release()
{
	if( m_hwnd && m_hFont ){
		::SendMessageAny( m_hwnd, WM_SETFONT, (WPARAM)m_hFontOld, FALSE );
		::DeleteObject( m_hFont );
		m_hFont = NULL;
		m_hwnd = NULL;
	}
}
#endif
