/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�J�X�^�����j���[)�֐��Q

	2012/12/20	CViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2006, fon
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"
#include "window/CEditWnd.h"/// 2002/2/3 aroka �ǉ�


/* �E�N���b�N���j���[ */
void CViewCommander::Command_MENU_RBUTTON( void )
{
	int			nId;
	int			nLength;
//	HGLOBAL		hgClip;
//	char*		pszClip;
	int			i;
	/* �|�b�v�A�b�v���j���[(�E�N���b�N) */
	nId = m_pCommanderView->CreatePopUpMenu_R();
	if( 0 == nId ){
		return;
	}
	switch( nId ){
	case IDM_COPYDICINFO:
		const TCHAR*	pszStr;
		pszStr = m_pCommanderView->m_cTipWnd.m_cInfo.GetStringPtr( &nLength );

		TCHAR*		pszWork;
		pszWork = new TCHAR[nLength + 1];
		auto_memcpy( pszWork, pszStr, nLength );
		pszWork[nLength] = _T('\0');

		// �����ڂƓ����悤�ɁA\n �� CR+LF�֕ϊ�����
		for( i = 0; i < nLength ; ++i){
			if( pszWork[i] == _T('\\') && pszWork[i + 1] == _T('n')){
				pszWork[i] =     WCODE::CR;
				pszWork[i + 1] = WCODE::LF;
			}
		}
		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		m_pCommanderView->MySetClipboardData( pszWork, nLength, false );
		delete[] pszWork;

		break;

	case IDM_JUMPDICT:
		/* �L�[���[�h�����t�@�C�����J�� */
		if(GetDocument()->m_cDocType.GetDocumentAttribute().m_bUseKeyWordHelp){		/* �L�[���[�h�����Z���N�g���g�p���� */	// 2006.04.10 fon
			//	Feb. 17, 2007 genta ���΃p�X�����s�t�@�C����ŊJ���悤��
			m_pCommanderView->TagJumpSub(
				GetDocument()->m_cDocType.GetDocumentType()->m_KeyHelpArr[m_pCommanderView->m_cTipWnd.m_nSearchDict].m_szPath,
				CMyPoint(1, m_pCommanderView->m_cTipWnd.m_nSearchLine),
				0,
				true
			);
		}
		break;

	default:
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
//		HandleCommand( nId, TRUE, 0, 0, 0, 0 );
		::PostMessageCmd( GetMainWindow(), WM_COMMAND, MAKELONG( nId, 0 ),  (LPARAM)NULL );
		break;
	}
	return;
}



/* �J�X�^�����j���[�\�� */
int CViewCommander::Command_CUSTMENU( int nMenuIdx )
{
	HMENU		hMenu;
	int			nId;
	POINT		po;
	int			i;
	UINT		uFlags;

	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->GetMenuDrawer().ResetContents();

	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = GetDocument()->m_cFuncLookup;

	if( nMenuIdx < 0 || MAX_CUSTOM_MENU <= nMenuIdx ){
		return 0;
	}
	if( 0 == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx] ){
		return 0;
	}
	hMenu = ::CreatePopupMenu();
	for( i = 0; i < GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx]; ++i ){
		if( F_0 == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			// 2010.07.24 ���j���[�z��ɓ����
			pCEditWnd->GetMenuDrawer().MyAppendMenuSep( hMenu, MF_SEPARATOR, F_0 , _T("") );
		}else{
			//	Oct. 3, 2001 genta
			WCHAR		szLabel[300];
			WCHAR		szLabel2[300];
			const WCHAR*	pszMenuLabel = szLabel2;
			FuncLookup.Funccode2Name( GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
			/* �L�[ */
			if( L'\0' == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i] ){
				pszMenuLabel = szLabel;
			}else{
				auto_sprintf( szLabel2, LTEXT("%ls (&%hc)"),
					szLabel,
					GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i]
				);
			}
			/* �@�\�����p�\�����ׂ� */
			if( IsFuncEnable( GetDocument(), &GetDllShareData(), GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ) ){
				uFlags = MF_STRING | MF_ENABLED;
			}else{
				uFlags = MF_STRING | MF_DISABLED | MF_GRAYED;
			}
			pCEditWnd->GetMenuDrawer().MyAppendMenu(
				hMenu, /*MF_BYPOSITION | MF_STRING*/uFlags,
				GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] , pszMenuLabel, L"" );
		}
	}
	po.x = 0;
	po.y = 0;
	//2002/04/08 YAZAKI �J�X�^�����j���[���}�E�X�J�[�\���̈ʒu�ɕ\������悤�ɕύX�B
	::GetCursorPos( &po );
	po.y -= 4;
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		GetMainWindow(),
		NULL
	);
	::DestroyMenu( hMenu );
	return nId;
}
