#include "stdafx.h"
#include "window.h"
#include "CShareData.h"
#include "mymessage.h"


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




/** �t���[���E�B���h�E���A�N�e�B�u�ɂ���
	@date 2007.11.07 ryoji �Ώۂ�disable�̂Ƃ��͍ŋ߂̃|�b�v�A�b�v���t�H�A�O���E���h������D
		�i���[�_���_�C�A���O�⃁�b�Z�[�W�{�b�N�X��\�����Ă���悤�ȂƂ��j
*/
void ActivateFrameWindow( HWND hwnd )
{
	// �ҏW�E�B���h�E�Ń^�u�܂Ƃߕ\���̏ꍇ�͕\���ʒu�𕜌�����
	CShareData* pInstance = NULL;
	DLLSHAREDATA* pShareData = NULL;
	if( (pInstance = CShareData::getInstance()) && (pShareData = pInstance->GetShareData()) ){
		if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ) {
			if( pInstance->IsEditWnd( hwnd ) ){
				if( pShareData->m_bEditWndChanging )
					return;	// �ؑւ̍Œ�(busy)�͗v���𖳎�����
				pShareData->m_bEditWndChanging = TRUE;	// �ҏW�E�B���h�E�ؑ֒�ON	2007.04.03 ryoji

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
			}
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
		::ShowWindow( hwnd, SW_SHOW );
	}
	::SetForegroundWindow( hwndActivate );
	::BringWindowToTop( hwndActivate );

	if( pShareData )
		pShareData->m_bEditWndChanging = FALSE;	// �ҏW�E�B���h�E�ؑ֒�OFF	2007.04.03 ryoji

	return;
}
