//	$Id$
/*!	@file
	���j���[�Ǘ����\��

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
#include "CMenuDrawer.h"
#include "sakura_rc.h"
#include "global.h"
#include "debug.h"
#include "CSplitBoxWnd.h"
#include "CEditWnd.h"


void FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr)
{
//	ASSERT_VALID(this);
//	ASSERT(m_hDC != NULL);

	RECT rect;
	::SetBkColor( hdc, clr );
	::SetRect( &rect, x, y, x + cx, y + cy );
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL );
}


CMenuDrawer::CMenuDrawer()
{
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_cShareData.Init();
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );

	m_nMenuItemNum = 0;
	m_nMenuHeight = 0;
	m_hFontMenu = NULL;
	m_hFontMenuUndelLine = NULL;
	return;
}


CMenuDrawer::~CMenuDrawer()
{
	if( NULL != m_hFontMenu ){
		::DeleteObject( m_hFontMenu );
		m_hFontMenu = NULL;
	}
	if( NULL != m_hFontMenuUndelLine ){
		::DeleteObject( m_hFontMenuUndelLine );
		m_hFontMenuUndelLine = NULL;
	}
	return;
}

void CMenuDrawer::Create( HINSTANCE hInstance, HWND hWndOwner, CImageListMgr* pcIcons )
{
	m_hInstance = hInstance;
	m_hWndOwner = hWndOwner;
	m_pcIcons = pcIcons;

	return;
}


void CMenuDrawer::ResetContents( void )
{
	int		i;
	LOGFONT	lf;
	for( i = 0; i < m_nMenuItemNum; ++i ){
		m_cmemMenuItemStrArr[i].SetData( "", 0 );
		m_nMenuItemFuncArr[i] = 0;
	}
	m_nMenuItemNum = 0;

	NONCLIENTMETRICS	ncm;
	ncm.cbSize = sizeof( NONCLIENTMETRICS );
	::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), (PVOID)&ncm, 0 );

	m_nMenuHeight = ncm.iMenuHeight;
	if( 21 > m_nMenuHeight ){
		m_nMenuHeight = 21;
	}

	if( NULL != m_hFontMenu ){
		::DeleteObject( m_hFontMenu );
		m_hFontMenu = NULL;
	}
	if( NULL != m_hFontMenuUndelLine ){
		::DeleteObject( m_hFontMenuUndelLine );
		m_hFontMenuUndelLine = NULL;
	}
	lf = ncm.lfMenuFont;
	m_hFontMenu = ::CreateFontIndirect( &lf );
	lf.lfUnderline = TRUE;
	m_hFontMenuUndelLine = ::CreateFontIndirect( &lf );
	m_nMaxTab = 0;
	m_nMaxTabLen = 0;
	return;
}




/* ���j���[�A�C�e���̕`��T�C�Y���v�Z */
int CMenuDrawer::MeasureItem( int nFuncID, int* pnItemHeight )
{


	*pnItemHeight = m_nMenuHeight;
	const char* pszLabel;
	RECT rc;
	HDC hdc;
	HFONT hFontOld;

	if( NULL == ( pszLabel = GetLabel( nFuncID ) ) ){
		return 0;
	}
	hdc = ::GetDC( m_hWndOwner );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontMenu );
	::DrawText( hdc, pszLabel, strlen( pszLabel ), &rc, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_CALCRECT );
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWndOwner, hdc );


//	*pnItemHeight = 20;
	*pnItemHeight = 2 + 15 + 1;

	return rc.right - rc.left + 20 + 8;
//	return m_nMaxTab + 16 + m_nMaxTabLen;

}

/* ���j���[���ڂ�ǉ� */
void CMenuDrawer::MyAppendMenu( HMENU hMenu, int nFlag, int nFuncId, const char* pszLabel, BOOL bAddKeyStr )
{
	char		szLabel[256];
	int			nFlagAdd;
	int			i;

	szLabel[0] = '\0';
	if( NULL != pszLabel ){
		strcpy( szLabel, pszLabel );
	}
	if( nFuncId != 0 ){
		/* ���j���[���x���̍쐬 */
		CKeyBind::GetMenuLabel(
			m_hInstance,
			m_pShareData->m_nKeyNameArrNum,
			m_pShareData->m_pKeyNameArr,
			nFuncId,
			szLabel,
			bAddKeyStr
		 );
	}
	nFlagAdd = 0;
	/* �A�C�R���p�r�b�g�}�b�v�������̂́A�I�[�i�[�h���E�ɂ��� */
	if( 0 != nFuncId ){
		if( m_nMenuItemNum + 1 > MAX_MENUITEMS ){
			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
				"CMenuDrawer::MyAppendMenu()�G���[\n\nCMenuDrawer���Ǘ��ł��郁�j���[�A�C�e���̏����CMenuDrawer::MAX_MENUITEMS==%d�ł��B\n ", MAX_MENUITEMS
			);
		}else{

			m_nMenuItemBitmapIdxArr[m_nMenuItemNum] = -1;
			m_nMenuItemFuncArr[m_nMenuItemNum] = nFuncId;
			m_cmemMenuItemStrArr[m_nMenuItemNum].SetData( szLabel, strlen( szLabel ) );
//#ifdef _DEBUG
			/* ���j���[���ڂ��I�[�i�[�`��ɂ��� */
			/* ���j���[�ɃA�C�R����\������ */
			if( m_pShareData->m_Common.m_bMenuIcon ){
				nFlagAdd = MF_OWNERDRAW;
			}
//#endif
			/* �@�\�̃r�b�g�}�b�v�����邩�ǂ������ׂĂ��� */
			for( i = 0; i < m_cShareData.m_nMyButtonNum; ++i ){
				if( nFuncId == m_cShareData.m_tbMyButton[i].idCommand ){
					/* �@�\�̃r�b�g�}�b�v�̏����o���Ă��� */
					m_nMenuItemBitmapIdxArr[m_nMenuItemNum] = m_cShareData.m_tbMyButton[i].iBitmap;
					break;
				}
			}
			m_nMenuItemNum++;
		}
	}

	MENUITEMINFO mii;
	memset( &mii, 0, sizeof( MENUITEMINFO ) );
	mii.cbSize = sizeof( MENUITEMINFO );
	mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
	mii.fType = 0;
	if( MF_OWNERDRAW	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_OWNERDRAW;
	if( MF_SEPARATOR	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_SEPARATOR;
	if( MF_STRING		& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_STRING;
	if( MF_MENUBREAK	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_MENUBREAK;
	if( MF_MENUBARBREAK	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_MENUBARBREAK;

	mii.fState = 0;
	if( MF_GRAYED		& ( nFlag | nFlagAdd ) ) mii.fState |= MFS_GRAYED;
	if( MF_CHECKED		& ( nFlag | nFlagAdd ) ) mii.fState |= MFS_CHECKED;

	mii.wID = nFuncId;
	mii.hSubMenu = (nFlag&MF_POPUP)?((HMENU)nFuncId):NULL;
	mii.hbmpChecked = NULL;
	mii.hbmpUnchecked = NULL;
	mii.dwItemData = (DWORD)this;
	mii.dwTypeData = (LPTSTR)szLabel;
	mii.cch = 0;

	// ���j���[���̎w�肳�ꂽ�ʒu�ɁA�V�������j���[���ڂ�}�����܂��B
	::InsertMenuItem( hMenu, 0xFFFFFFFF, TRUE, &mii );
	return;
}




/* ���j���[�A�C�e���`�� */
void CMenuDrawer::DrawItem( DRAWITEMSTRUCT* lpdis )
{

	int			i;
	int			j;
	int			nItemIndex;
	HDC			hdc;
	const char*	pszItemStr;
	int			nItemStrLen;
//	TEXTMETRIC	tm;
//	SIZE		sz;
	HFONT		hFontOld;
	int			nIndentLeft;
	int			nIndentRight;
	int			nTextTopMargin;
	RECT		rc1;
	HBRUSH		hBrush;
	RECT		rcText;
	int			nBkModeOld;

	nIndentLeft = 26;
	nIndentRight = 8;
	nTextTopMargin = 3;

	for( i = 0; i < m_nMenuItemNum; ++i ){
		if( (int)lpdis->itemID == m_nMenuItemFuncArr[i] ){
			break;
		}
	}
	if( i >= m_nMenuItemNum ){
		return;
	}
	nItemIndex = i;
	pszItemStr = m_cmemMenuItemStrArr[nItemIndex].GetPtr( &nItemStrLen );

//	hdc = ::GetDC( m_hWndOwner );
	hdc = lpdis->hDC;
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontMenu );


	/* �A�C�e����`�h��Ԃ� */
	hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_MENU ) );
	::FillRect( hdc, &lpdis->rcItem, hBrush );
	::DeleteObject( hBrush );


	/* �A�C�e�����I������Ă��� */
	nBkModeOld = ::SetBkMode( hdc, TRANSPARENT );
	if( lpdis->itemState & ODS_SELECTED ){
		hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_HIGHLIGHT/*COLOR_3DHIGHLIGHT*/ ) );
		rc1 = lpdis->rcItem;
		if( -1 != m_nMenuItemBitmapIdxArr[nItemIndex] || lpdis->itemState & ODS_CHECKED ){
			rc1.left += (nIndentLeft - 2);
		}
		/* �I���n�C���C�g��` */
		::FillRect( hdc, &rc1, hBrush );
		::DeleteObject( hBrush );

		/* �A�C�e�����g�p�s�� */
		if( lpdis->itemState & ODS_DISABLED ){
			::SetTextColor( hdc, ::GetSysColor( COLOR_MENU/*COLOR_3DSHADOW*/ ) );
		}else{
			::SetTextColor( hdc, ::GetSysColor( COLOR_HIGHLIGHTTEXT/*COLOR_MENUTEXT*//*COLOR_3DHIGHLIGHT*/ ) );
		}
	}else{
		/* �A�C�e�����g�p�s�� */
		if( lpdis->itemState & ODS_DISABLED ){
			::SetTextColor( hdc, ::GetSysColor( COLOR_3DSHADOW ) );
		}else{
			::SetTextColor( hdc, ::GetSysColor( COLOR_MENUTEXT ) );
		}
	}

#ifdef _DEBUG
	char	szText[1024];
	MENUITEMINFO mii;
	// ���j���[���ڂɊւ�������擾���܂��B
	memset( &mii, 0, sizeof( MENUITEMINFO ) );
	mii.cbSize = sizeof( MENUITEMINFO );
	mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
	mii.fType = MFT_STRING;
	strcpy( szText, "--unknown--" );
	mii.dwTypeData = (LPTSTR)szText;
	mii.cch = sizeof( szText ) - 1;
	if( 0 != ::GetMenuItemInfo( (HMENU)lpdis->hwndItem, lpdis->itemID, FALSE, (MENUITEMINFO*)&mii )
	 && NULL == mii.hSubMenu
	 && 0 == CEditWnd::FuncID_To_HelpContextID( lpdis->itemID ) 	/* �@�\ID�ɑΉ����郁�j���[�R���e�L�X�g�ԍ���Ԃ� */
	){
		::SetTextColor( hdc, RGB( 0, 0, 255 ) );
	}
#endif

	rcText = lpdis->rcItem;
	rcText.left += nIndentLeft;
	rcText.right -= nIndentRight;

	/* TAB�����̑O�ƌ��ɕ������ăe�L�X�g��`�悷�� */
	for( j = 0; j < nItemStrLen; ++j ){
		if( pszItemStr[j] == '\t' ){
			break;
		}
	}
	/* TAB�����̌�둤�̃e�L�X�g��`�悷�� */
	if( j < nItemStrLen ){
		/* �A�C�e�����g�p�s�� */
		if( lpdis->itemState & ODS_DISABLED && !(lpdis->itemState & ODS_SELECTED)  ){
			COLORREF colOld = ::SetTextColor( hdc, ::GetSysColor( COLOR_3DHIGHLIGHT ) );
				rcText.left++;
				rcText.top++;
				rcText.right++;
				rcText.bottom++;
				::DrawText( hdc, &pszItemStr[j + 1], strlen( &pszItemStr[j + 1] ), &rcText, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_RIGHT );
				rcText.left--;
				rcText.top--;
				rcText.right--;
				rcText.bottom--;
				::SetTextColor( hdc, colOld );
		}
		::DrawText( hdc, &pszItemStr[j + 1], strlen( &pszItemStr[j + 1] ), &rcText, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_RIGHT );
	}
	/* TAB�����̑O���̃e�L�X�g��`�悷�� */
	/* �A�C�e�����g�p�s�� */
	if( lpdis->itemState & ODS_DISABLED && !(lpdis->itemState & ODS_SELECTED)  ){
		COLORREF colOld = ::SetTextColor( hdc, ::GetSysColor( COLOR_3DHIGHLIGHT ) );
			rcText.left++;
			rcText.top++;
			rcText.right++;
			rcText.bottom++;
			::DrawText( hdc, pszItemStr, j, &rcText, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_LEFT );
			rcText.left--;
			rcText.top--;
			rcText.right--;
			rcText.bottom--;
			::SetTextColor( hdc, colOld );
	}
	::DrawText( hdc, pszItemStr, j, &rcText, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_LEFT );
	::SelectObject( hdc, hFontOld  );
	::SetBkMode( hdc, nBkModeOld );

	/* �`�F�b�N��ԂȂ牚��3D�g��`�悷�� */
	if( lpdis->itemState & ODS_CHECKED ){
		/* �A�C�R�����͂ޘg */
		CSplitBoxWnd::Draw3dRect(
			hdc, lpdis->rcItem.left + 1, lpdis->rcItem.top,
			2 + 16 + 2, lpdis->rcItem.bottom - lpdis->rcItem.top,
			::GetSysColor( COLOR_3DSHADOW ),
			::GetSysColor( COLOR_3DHILIGHT )
		);
		/* �A�C�e�����I������Ă��Ȃ��ꍇ��3D�g�̒��𖾂邭�h��Ԃ� */
		if( lpdis->itemState & ODS_SELECTED ){
		}else{
			HBRUSH hbr = ::CreateSolidBrush( ::GetSysColor( COLOR_3DLIGHT ) );
//			HBRUSH hbr = ::CreateSolidBrush( ::GetSysColor( COLOR_3DHILIGHT ) );
			HBRUSH hbrOld = (HBRUSH)::SelectObject( hdc, hbr );
			RECT rc;
			::SetRect( &rc, lpdis->rcItem.left + 1 + 1, lpdis->rcItem.top + 1, lpdis->rcItem.left + 1 + 1 + 16 + 2, lpdis->rcItem.top + 1+ 15 + 2 );
			::FillRect( hdc, &rc, hbr );
			::SelectObject( hdc, hbrOld );
			::DeleteObject( hbr );
		}
	}


	/* �@�\�̉摜�����݂���Ȃ烁�j���[�A�C�R��?��`�悷�� */
	if( -1 != m_nMenuItemBitmapIdxArr[nItemIndex] ){
		/* 3D�g��`�悷�� */
		/* �A�C�e�����I������Ă��� */
		if( lpdis->itemState & ODS_SELECTED ){
			/* �A�C�e�����g�p�s�� */
			if( lpdis->itemState & ODS_DISABLED /*&& !(lpdis->itemState & ODS_SELECTED)*/  ){
			}else{
				if( lpdis->itemState & ODS_CHECKED ){
				}else{
					/* �A�C�R�����͂ޘg */
					CSplitBoxWnd::Draw3dRect(
						hdc, lpdis->rcItem.left + 1, lpdis->rcItem.top,
						2 + 16 + 2, lpdis->rcItem.bottom - lpdis->rcItem.top,
						::GetSysColor( COLOR_3DHILIGHT ),
						::GetSysColor( COLOR_3DSHADOW )
					 );
				}
			}
		}

		/* �A�C�e�����g�p�s�� */
		if( lpdis->itemState & ODS_DISABLED ){
			/* �W�F�A�C�R�� */
			COLORREF cOld;
//			cOld = SetTextColor( hdc, GetSysColor(COLOR_3DSHADOW) );	//Oct. 24, 2000 ����͕W���ł�RGB(128,128,128)�Ɠ���
			cOld = SetTextColor( hdc, RGB(132,132,132) );	//Oct. 24, 2000 JEPRO ����������������
			m_pcIcons->Draw( m_nMenuItemBitmapIdxArr[nItemIndex],
				hdc,	//	Target DC
				lpdis->rcItem.left + 1,	//	X
				lpdis->rcItem.top + 1,		//	Y
				ILD_MASK
			);
			SetTextColor( hdc, cOld );

		}else{
			COLORREF colBk;
			if( lpdis->itemState & ODS_CHECKED && !( lpdis->itemState & ODS_SELECTED ) ){
				colBk = ::GetSysColor( COLOR_3DLIGHT );
//				colBk = ::GetSysColor( COLOR_3DHILIGHT );
			}else{
				colBk = ::GetSysColor( COLOR_MENU );
			}

			/* �ʏ�̃A�C�R�� */
			m_pcIcons->Draw( m_nMenuItemBitmapIdxArr[nItemIndex],
				hdc,	//	Target DC
				lpdis->rcItem.left + 1 + 1,	//	X
				lpdis->rcItem.top + 1,		//	Y
				ILD_NORMAL
			);
		}

	}else{
		if( lpdis->itemState & ODS_CHECKED ){
			/* �A�C�R�����Ȃ��`�F�b�N�}�[�N�̕\�� */
			int nX, nY;
			nX = lpdis->rcItem.left + 1 + 8;
			nY = lpdis->rcItem.top + 8;
			::MoveToEx( hdc, nX, nY, NULL );
			::LineTo( hdc, nX + 1, nY + 1 );
			::LineTo( hdc, nX + 4, nY - 2 );

			nY++;
			::MoveToEx( hdc, nX, nY, NULL );
			::LineTo( hdc, nX + 1, nY + 1 );
			::LineTo( hdc, nX + 4, nY - 2 );

		}
	}

	//	::ReleaseDC( m_hWndOwner, hdc );
	return;
}





int CMenuDrawer::Find( int nFuncID )
{
	int i;
	for( i = 0; i < m_nMenuItemNum; ++i ){
		if( (int)nFuncID == m_nMenuItemFuncArr[i] ){
			break;
		}
	}
	if( i >= m_nMenuItemNum ){
		return -1;
	}else{
		return i;
	}
}


const char* CMenuDrawer::GetLabel( int nFuncID )
{
	int i;
	if( -1 == ( i = Find( nFuncID ) ) ){
		return NULL;
	}
	return m_cmemMenuItemStrArr[i].GetPtr( NULL );
}

char CMenuDrawer::GetAccelCharFromLabel( const char* pszLabel )
{
	int i;
	for( i = 0; i + 1 < (int)strlen( pszLabel ); ++i ){
		if( '&' == pszLabel[i] ){
			if( '&' == pszLabel[i + 1]  ){
				i++;
			}else{
				return toupper( pszLabel[i + 1] );
			}
		}
	}
	return (char)0;
}





/* ���j���[�A�N�Z�X�L�[�������̏���(WM_MENUCHAR����) */
LRESULT CMenuDrawer::OnMenuChar( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	TCHAR				chUser;
	UINT				fuFlag;
	HMENU				hmenu;
//	HMENU				hMenu;
	MENUITEMINFO		mii;
	int i;
	chUser = (TCHAR) LOWORD(wParam);	// character code
	fuFlag = (UINT) HIWORD(wParam);		// menu flag
	hmenu = (HMENU) lParam;				// handle to menu
//	MYTRACE( "::GetMenuItemCount( %xh )==%d\n", hmenu, ::GetMenuItemCount( hmenu ) );

	//	Oct. 27, 2000 genta
	if( 0 <= chUser && chUser < ' '){
		chUser += '@';
	}
	else {
		chUser = toupper( chUser );
	}

	struct WorkData{
		int				idx;
		MENUITEMINFO	mii;
	};

	WorkData vecAccel[100];
	int nAccelNum;
	int nAccelSel;
	nAccelNum = 0;
	nAccelSel = 99999;
	for( i = 0; i < ::GetMenuItemCount( hmenu ); i++ ){
		char	szText[1024];
		// ���j���[���ڂɊւ�������擾���܂��B
		memset( &mii, 0, sizeof( MENUITEMINFO ) );
		mii.cbSize = sizeof( MENUITEMINFO );
		mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
		mii.fType = MFT_STRING;
		strcpy( szText, "--unknown--" );
		mii.dwTypeData = (LPTSTR)szText;
		mii.cch = sizeof( szText ) - 1;
		if( 0 == ::GetMenuItemInfo( hmenu, i, TRUE, (MENUITEMINFO*)&mii ) ){
			continue;
		}
		const char* pszLabel;
		if( NULL == ( pszLabel = GetLabel( mii.wID ) ) ){
			continue;
		}
		if( chUser == GetAccelCharFromLabel( pszLabel ) ){
			vecAccel[nAccelNum].idx = i;
			vecAccel[nAccelNum].mii = mii;
			if( /*-1 == nAccelSel ||*/ MFS_HILITE & mii.fState ){
				nAccelSel = nAccelNum;
			}
			nAccelNum++;
		}
	}
//	MYTRACE( "%d\n", (int)mapAccel.size() );
	if( 0 == nAccelNum ){
		return  MAKELONG( 0, MNC_IGNORE );
	}
	if( 1 == nAccelNum ){
		return  MAKELONG( vecAccel[0].idx, MNC_EXECUTE );
	}
//	MYTRACE( "nAccelSel=%d nAccelNum=%d\n", nAccelSel, nAccelNum );
	if( nAccelSel + 1 >= nAccelNum ){
//		MYTRACE( "vecAccel[0].idx=%d\n", vecAccel[0].idx );
		return  MAKELONG( vecAccel[0].idx, MNC_SELECT );
	}else{
//		MYTRACE( "vecAccel[nAccelSel + 1].idx=%d\n", vecAccel[nAccelSel + 1].idx );
		return  MAKELONG( vecAccel[nAccelSel + 1].idx, MNC_SELECT );
	}
}








/* �r�b�g�}�b�v�̕\�� �D�F�𓧖��`�� */
void CMenuDrawer::MyBitBlt(
	HDC drawdc,
	int nXDest,
	int nYDest,
	int nWidth,
	int nHeight,
	HBITMAP bmp,
	int nXSrc,
	int nYSrc,
	COLORREF colToTransParent, //bmp�̒��̓����ɂ������F
	COLORREF colBkColor //�`���̔w�i�F
)
{
//	COLORREF colToTransParent = RGB( 192, 192, 192 );	/* BMP�̒��̓����ɂ���F */
//	HBRUSH	brShadow, brHilight;
	HDC		hdcMask;
	HBITMAP bmpMask;
	HBITMAP bmpMaskOld;
	HDC		hdcMem;
	HBITMAP	bmpMemOld;
	HDC		hdcMem2;
	HBITMAP bmpMem2;
	HBITMAP bmpMem2Old;
	// create a monochrome memory DC
	hdcMask = CreateCompatibleDC( 0 );
	bmpMask = CreateCompatibleBitmap( hdcMask, nWidth, nHeight );
	bmpMaskOld = (HBITMAP)SelectObject( hdcMask, bmpMask );
	/* ���r�b�g�}�b�v�pDC */
	hdcMem = ::CreateCompatibleDC( drawdc );
	bmpMemOld = (HBITMAP)::SelectObject( hdcMem, bmp );
	/* ��ƗpDC */
	hdcMem2 = ::CreateCompatibleDC( drawdc );
	bmpMem2 = CreateCompatibleBitmap( drawdc, nWidth, nHeight );
	bmpMem2Old = (HBITMAP)SelectObject( hdcMem2, bmpMem2 );

	// build a mask
	PatBlt( hdcMask, 0, 0, nWidth, nHeight, WHITENESS );
	SetBkColor( hdcMem, colToTransParent );
	BitBlt( hdcMask, 0, 0, nWidth, nHeight, hdcMem, nXSrc,nYSrc, SRCCOPY );

	/* �}�X�N�`��(�����ɂ��Ȃ��������������`��) */
	::SetBkColor( drawdc, colBkColor/*::GetSysColor( COLOR_MENU )*/ );
	::SetTextColor( drawdc, RGB( 0, 0, 0 ) );
	::BitBlt( drawdc, nXDest, nYDest, nWidth, nHeight, hdcMask, 0, 0, SRCCOPY );

	/* �r�b�g�}�b�v�`��(�����ɂ���F���������ă}�X�N��OR�`��) */
	::SetBkColor( hdcMem2, colToTransParent/*RGB( 0, 0, 0 )*/ );
	::SetTextColor( hdcMem2, RGB( 0, 0, 0 ) );
	::BitBlt( hdcMem2, 0, 0, nWidth, nHeight, hdcMask, 0, 0, SRCCOPY );
	::BitBlt( hdcMem2, 0, 0, nWidth, nHeight, hdcMem, nXSrc,nYSrc, SRCINVERT/*SRCPAINT*/ );
	::BitBlt( drawdc, nXDest, nYDest, nWidth, nHeight, hdcMem2,  0, 0, /*SRCCOPY*/SRCPAINT );

	::SelectObject( hdcMask, bmpMaskOld );
	::DeleteObject( bmpMask );
	::DeleteDC( hdcMask );
	::SelectObject( hdcMem, bmpMemOld );
	::DeleteDC( hdcMem );
	::SelectObject( hdcMem2, bmpMem2Old );
	::DeleteObject( bmpMem2 );
	::DeleteDC( hdcMem2 );
	return;
}





/* ���j���[�A�C�R���̒W�F�\�� */
void CMenuDrawer::DitherBlt2( HDC drawdc, int nXDest, int nYDest, int nWidth,
							int nHeight, HBITMAP bmp, int nXSrc, int nYSrc)
{
	HBRUSH brShadow, brHilight;
	HDC		hdcMask;
	HBITMAP	bmpMask;
	HBITMAP	bmpMaskOld;
	HDC		hdcMem;
	HBITMAP	bmpMemOld;
	HBRUSH pOldBrush;

	COLORREF colToTransParent = RGB( 192, 192, 192 );	/* BMP�̒��̓����ɂ���F */

	// create a monochrome memory DC
	hdcMask = CreateCompatibleDC( 0 );
	bmpMask = CreateCompatibleBitmap( hdcMask, nWidth, nHeight );
	bmpMaskOld = (HBITMAP)SelectObject( hdcMask, bmpMask );

	hdcMem = CreateCompatibleDC( 0 );
	bmpMemOld = (HBITMAP)SelectObject( hdcMem, bmp );

	// build a mask
	PatBlt( hdcMask, 0, 0, nWidth, nHeight, WHITENESS );
	SetBkColor( hdcMem, colToTransParent );
	BitBlt( hdcMask, 0, 0, nWidth, nHeight, hdcMem, nXSrc,nYSrc, SRCCOPY );
	SetBkColor( hdcMem, RGB( 255, 255, 255 ) );
	BitBlt( hdcMask, 0, 0, nWidth, nHeight, hdcMem, nXSrc,nYSrc, SRCPAINT );

	// Copy the image from the toolbar into the memory DC
	// and draw it (grayed) back into the toolbar.
	FillSolidRect( hdcMem, 0,0, nWidth, nHeight, GetSysColor( COLOR_MENU ) );
	//SK: Looks better on the old shell
	SetBkColor( hdcMem, RGB( 0, 0, 0) );
	SetTextColor( hdcMem, RGB( 255, 255, 255 ) );
	brHilight = CreateSolidBrush( GetSysColor( COLOR_BTNHILIGHT ) );
	brShadow = CreateSolidBrush( GetSysColor( COLOR_BTNSHADOW ) );
	pOldBrush = (HBRUSH)SelectObject( hdcMem, brHilight );
	BitBlt( hdcMem, 0, 0, nWidth, nHeight, hdcMask, 0, 0, 0x00E20746L );
	BitBlt( drawdc, nXDest+1,nYDest+1,nWidth, nHeight, hdcMem,0,0,SRCCOPY );

	BitBlt( hdcMem, 1, 1, nWidth, nHeight, hdcMask, 0, 0, 0x00E20746L );
	SelectObject( hdcMem, brShadow);
	BitBlt( hdcMem, 0, 0, nWidth, nHeight, hdcMask, 0, 0, 0x00E20746L );
	BitBlt( drawdc, nXDest,nYDest,nWidth, nHeight, hdcMem,0,0,SRCCOPY );

	// reset DCs
	SelectObject( hdcMask, bmpMaskOld );
	DeleteDC( hdcMask );

	SelectObject( hdcMem, pOldBrush );
	SelectObject( hdcMem, bmpMemOld );
	DeleteDC( hdcMem );

	DeleteObject( bmpMask );
	DeleteObject( brHilight );
	DeleteObject( brShadow );
	return;

}


/*[EOF]*/
