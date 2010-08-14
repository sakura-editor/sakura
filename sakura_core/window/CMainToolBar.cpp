#include "StdAfx.h"
#include <WindowsX.h>
#include "CMainToolBar.h"
#include "window/CEditWnd.h"
#include "util/os.h"
#include "CEditApp.h"
#include "util/tchar_receive.h"
#include "util/window.h"

CMainToolBar::CMainToolBar(CEditWnd* pOwner)
: m_pOwner(pOwner)
, m_fontSearchBox(NULL)
, m_hwndReBar(NULL)
, m_hwndToolBar(NULL)
, m_hwndSearchBox(NULL)
{
}

/*! �����{�b�N�X�ł̏��� */
void CMainToolBar::ProcSearchBox( MSG *msg )
{
	if( msg->message == WM_KEYDOWN /* && ::GetParent( msg->hwnd ) == m_hwndSearchBox */ )
	{
		if( msg->wParam == VK_RETURN )  //���^�[���L�[
		{
			//�����L�[���[�h���擾
			wchar_t	szText[_MAX_PATH];
			wmemset( szText, 0, _countof(szText) );
			::SendMessage( m_hwndSearchBox, WM_GETTEXT, _MAX_PATH - 1, (LPARAM)szText );
			if( szText[0] )	//�L�[�����񂪂���
			{
				//�����L�[��o�^
				CSearchKeywordManager().AddToSearchKeyArr( szText );

				//�����{�b�N�X���X�V	// 2010/6/6 Uchi
				AcceptSharedSearchKey();

				//::SetFocus( m_hWnd );	//��Ƀt�H�[�J�X���ړ����Ă����Ȃ��ƃL�����b�g��������
				m_pOwner->GetActiveView().SetFocus();

				// �����J�n���̃J�[�\���ʒu�o�^������ύX 02/07/28 ai start
				m_pOwner->GetActiveView().m_ptSrchStartPos_PHY = m_pOwner->GetActiveView().GetCaret().GetCaretLogicPos();
				// 02/07/28 ai end

				//��������
				m_pOwner->OnCommand( (WORD)0 /*���j���[*/, (WORD)F_SEARCH_NEXT, (HWND)0 );
			}
		}
		else if( msg->wParam == VK_TAB )	//�^�u�L�[
		{
			//�t�H�[�J�X���ړ�
			//	2004.10.27 MIK IME�\���ʒu�̂���C��
			::SetFocus( m_pOwner->GetHwnd()  );
		}
	}
}

/*! �T�u�N���X�������c�[���o�[�̃E�B���h�E�v���V�[�W��
	@author ryoji
	@date 2006.09.06 ryoji
*/
static WNDPROC g_pOldToolBarWndProc;	// �c�[���o�[�̖{���̃E�B���h�E�v���V�[�W��

static LRESULT CALLBACK ToolBarWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	// WinXP Visual Style �̂Ƃ��Ƀc�[���o�[��ł̃}�E�X���E�{�^�����������Ŗ������ɂȂ�
	//�i�}�E�X���L���v�`���[�����܂ܕ����Ȃ��j ����������邽�߂ɉE�{�^���𖳎�����
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		return 0L;				// �E�{�^���� UP/DOWN �͖{���̃E�B���h�E�v���V�[�W���ɓn���Ȃ�

	case WM_DESTROY:
		// �T�u�N���X������
		::SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR)g_pOldToolBarWndProc );
		break;
	}
	return ::CallWindowProc( g_pOldToolBarWndProc, hWnd, msg, wParam, lParam );
}


/* �c�[���o�[�쐬
	@date @@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
	@date 2005.08.29 aroka �c�[���o�[�̐܂�Ԃ�
	@date 2006.06.17 ryoji �r�W���A���X�^�C�����L���̏ꍇ�̓c�[���o�[�� Rebar �ɓ���ăT�C�Y�ύX���̂�����𖳂���
*/
void CMainToolBar::CreateToolBar( void )
{
	if( m_hwndToolBar )return;

	REBARINFO		rbi;
	REBARBANDINFO	rbBand;
	int				nFlag;
	TBBUTTON		tbb;
	int				i;
	int				nIdx;
	UINT			uToolType;
	nFlag = 0;

	// 2006.06.17 ryoji
	// Rebar �E�B���h�E�̍쐬
	if( IsVisualStyle() ){	// �r�W���A���X�^�C���L��
		m_hwndReBar = ::CreateWindowEx(
			WS_EX_TOOLWINDOW,
			REBARCLASSNAME, //���o�[�R���g���[��
			NULL,
			WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |	// 2007.03.08 ryoji WS_VISIBLE ����
			RBS_BANDBORDERS | CCS_NODIVIDER,
			0, 0, 0, 0,
			m_pOwner->GetHwnd(),
			NULL,
			CEditApp::Instance()->GetAppInstance(),
			NULL
		);

		if( NULL == m_hwndReBar ){
			TopWarningMessage( m_pOwner->GetHwnd(), _T("Rebar �̍쐬�Ɏ��s���܂����B") );
			return;
		}

		if( GetDllShareData().m_Common.m_sToolBar.m_bToolBarIsFlat ){	/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */
			PreventVisualStyle( m_hwndReBar );	// �r�W���A���X�^�C����K�p�̃t���b�g�� Rebar �ɂ���
		}

		::ZeroMemory(&rbi, sizeof(rbi));
		rbi.cbSize = sizeof(rbi);
		Rebar_SetbarInfo(m_hwndReBar, &rbi);

		nFlag = CCS_NORESIZE | CCS_NODIVIDER | CCS_NOPARENTALIGN | TBSTYLE_FLAT;	// �c�[���o�[�ւ̒ǉ��X�^�C��
	}

	/* �c�[���o�[�E�B���h�E�̍쐬 */
	m_hwndToolBar = ::CreateWindowEx(
		0,
		TOOLBARCLASSNAME,
		NULL,
		WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPCHILDREN | /*WS_BORDER | */	// 2006.06.17 ryoji WS_CLIPCHILDREN �ǉ�	// 2007.03.08 ryoji WS_VISIBLE ����
/*		WS_EX_WINDOWEDGE| */
		TBSTYLE_TOOLTIPS |
//		TBSTYLE_WRAPABLE |
//		TBSTYLE_ALTDRAG |
//		CCS_ADJUSTABLE |
		nFlag,
		0, 0,
		0, 0,
		m_pOwner->GetHwnd(),
		(HMENU)ID_TOOLBAR,
		CEditApp::Instance()->GetAppInstance(),
		NULL
	);
	if( NULL == m_hwndToolBar ){
		if( GetDllShareData().m_Common.m_sToolBar.m_bToolBarIsFlat ){	/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */
			GetDllShareData().m_Common.m_sToolBar.m_bToolBarIsFlat = FALSE;
		}
		TopWarningMessage( m_pOwner->GetHwnd(), _T("�c�[���o�[�̍쐬�Ɏ��s���܂����B") );
		DestroyToolBar();	// 2006.06.17 ryoji
	}
	else{
		// 2006.09.06 ryoji �c�[���o�[���T�u�N���X������
		g_pOldToolBarWndProc = (WNDPROC)::SetWindowLongPtr(
			m_hwndToolBar,
			GWLP_WNDPROC,
			(LONG_PTR)ToolBarWndProc
		);

		Toolbar_SetButtonSize( m_hwndToolBar, DpiScaleX(22), DpiScaleY(22) );	// 2009.10.01 ryoji ��DPI�Ή��X�P�[�����O
		Toolbar_ButtonStructSize( m_hwndToolBar, sizeof(TBBUTTON) );
		//	Oct. 12, 2000 genta
		//	���ɗp�ӂ���Ă���Image List���A�C�R���Ƃ��ēo�^
		CEditApp::Instance()->GetIcons().SetToolBarImages( m_hwndToolBar );
		/* �c�[���o�[�Ƀ{�^����ǉ� */
		int count = 0;	//@@@ 2002.06.15 MIK
		int nToolBarButtonNum = 0;// 2005/8/29 aroka
		//	From Here 2005.08.29 aroka
		// �͂��߂Ƀc�[���o�[�\���̂̔z�������Ă���
		TBBUTTON *pTbbArr = new TBBUTTON[GetDllShareData().m_Common.m_sToolBar.m_nToolBarButtonNum];
		for( i = 0; i < GetDllShareData().m_Common.m_sToolBar.m_nToolBarButtonNum; ++i ){
			nIdx = GetDllShareData().m_Common.m_sToolBar.m_nToolBarButtonIdxArr[i];
			pTbbArr[nToolBarButtonNum] = m_pOwner->GetMenuDrawer().getButton(nIdx);
			// �Z�p���[�^�������Ƃ��͂ЂƂɂ܂Ƃ߂�
			// �܂�Ԃ��{�^����TBSTYLE_SEP�����������Ă���̂�
			// �܂�Ԃ��̑O�̃Z�p���[�^�͑S�č폜�����D
			if( (pTbbArr[nToolBarButtonNum].fsStyle & TBSTYLE_SEP) && (nToolBarButtonNum!=0)){
				if( (pTbbArr[nToolBarButtonNum-1].fsStyle & TBSTYLE_SEP) ){
					pTbbArr[nToolBarButtonNum-1] = pTbbArr[nToolBarButtonNum];
					nToolBarButtonNum--;
				}
			}
			// ���z�ܕԂ��{�^���������璼�O�̃{�^���ɐܕԂ�������t����
			if( pTbbArr[nToolBarButtonNum].fsState & TBSTATE_WRAP ){
				if( nToolBarButtonNum!=0 ){
					pTbbArr[nToolBarButtonNum-1].fsState |= TBSTATE_WRAP;
				}
				continue;
			}
			nToolBarButtonNum++;
		}
		//	To Here 2005.08.29 aroka

		for( i = 0; i < nToolBarButtonNum; ++i ){
			tbb = pTbbArr[i];

			//@@@ 2002.06.15 MIK start
			switch( tbb.fsStyle )
			{
			case TBSTYLE_DROPDOWN:	//�h���b�v�_�E��
				//�g���X�^�C���ɐݒ�
				Toolbar_SetExtendedStyle( m_hwndToolBar, TBSTYLE_EX_DRAWDDARROWS );
				Toolbar_AddButtons( m_hwndToolBar, 1, &tbb );
				count++;
				break;

			case TBSTYLE_COMBOBOX:	//�R���{�{�b�N�X
				{
					RECT			rc;
					TBBUTTONINFO	tbi;
					TBBUTTON		my_tbb;
					LOGFONT		lf;

					switch( tbb.idCommand )
					{
					case F_SEARCH_BOX:
						if( m_hwndSearchBox )
						{
							break;
						}
						
						//�Z�p���[�^���
						memset_raw( &my_tbb, 0, sizeof(my_tbb) );
						my_tbb.fsStyle   = TBSTYLE_BUTTON;  //�{�^���ɂ��Ȃ��ƕ`�悪����� 2005/8/29 aroka
						my_tbb.idCommand = tbb.idCommand;	//����ID�ɂ��Ă���
						if( tbb.fsState & TBSTATE_WRAP ){   //�܂�Ԃ� 2005/8/29 aroka
							my_tbb.fsState |=  TBSTATE_WRAP;
						}
						Toolbar_AddButtons( m_hwndToolBar, 1, &my_tbb );
						count++;

						//�T�C�Y��ݒ肷��
						tbi.cbSize = sizeof(tbi);
						tbi.dwMask = TBIF_SIZE;
						tbi.cx     = DpiScaleX(160);	//�{�b�N�X�̕�	// 2009.10.01 ryoji ��DPI�Ή��X�P�[�����O
						Toolbar_SetButtonInfo( m_hwndToolBar, tbb.idCommand, &tbi );

						//�T�C�Y���擾����
						rc.right = rc.left = rc.top = rc.bottom = 0;
						Toolbar_GetItemRect( m_hwndToolBar, count-1, &rc );

						//�R���{�{�b�N�X�����
						//	Mar. 8, 2003 genta �����{�b�N�X��1�h�b�g���ɂ��炵��
						m_hwndSearchBox = CreateWindow( _T("COMBOBOX"), _T("Combo"),
								WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN
								/*| CBS_SORT*/ | CBS_AUTOHSCROLL /*| CBS_DISABLENOSCROLL*/,
								rc.left, rc.top + 1, rc.right - rc.left, (rc.bottom - rc.top) * 10,
								m_hwndToolBar, (HMENU)(INT_PTR)tbb.idCommand, CEditApp::Instance()->GetAppInstance(), NULL );
						if( m_hwndSearchBox )
						{
							m_pOwner->SetCurrentFocus(0);

							memset_raw( &lf, 0, sizeof(lf) );
							lf.lfHeight			= DpiPointsToPixels(-9); // Jan. 14, 2003 genta �_�C�A���O�ɂ��킹�Ă�����Ə�����	// 2009.10.01 ryoji ��DPI�Ή��i�|�C���g������Z�o�j
							lf.lfWidth			= 0;
							lf.lfEscapement		= 0;
							lf.lfOrientation	= 0;
							lf.lfWeight			= FW_NORMAL;
							lf.lfItalic			= FALSE;
							lf.lfUnderline		= FALSE;
							lf.lfStrikeOut		= FALSE;
							lf.lfCharSet		= SHIFTJIS_CHARSET;
							lf.lfOutPrecision	= OUT_DEFAULT_PRECIS;
							lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
							lf.lfQuality		= DEFAULT_QUALITY;
							lf.lfPitchAndFamily	= FF_MODERN | DEFAULT_PITCH;
							_tcscpy( lf.lfFaceName, _T("�l�r �o�S�V�b�N") );
							m_fontSearchBox = ::CreateFontIndirect( &lf );
							if( m_fontSearchBox )
							{
								::SendMessage( m_hwndSearchBox, WM_SETFONT, (WPARAM)m_fontSearchBox, MAKELONG (TRUE, 0) );
							}

							//���͒�����
							ComboBox_LimitText( m_hwndSearchBox, (WPARAM)_MAX_PATH - 1 );

							//�����{�b�N�X���X�V	// �֐��� 2010/6/6 Uchi
							AcceptSharedSearchKey();
						}
						break;

					default:
						break;
					}
				}
				break;

			case TBSTYLE_BUTTON:	//�{�^��
			case TBSTYLE_SEP:		//�Z�p���[�^
			default:
				Toolbar_AddButtons( m_hwndToolBar, 1, &tbb );
				count++;
				break;
			}
			//@@@ 2002.06.15 MIK end
		}
		if( GetDllShareData().m_Common.m_sToolBar.m_bToolBarIsFlat ){	/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */
			uToolType = (UINT)::GetWindowLong(m_hwndToolBar, GWL_STYLE);
			uToolType |= (TBSTYLE_FLAT);
			::SetWindowLong(m_hwndToolBar, GWL_STYLE, uToolType);
			::InvalidateRect(m_hwndToolBar, NULL, TRUE);
		}
		delete []pTbbArr;// 2005/8/29 aroka
	}

	// 2006.06.17 ryoji
	// �c�[���o�[�� Rebar �ɓ����
	if( m_hwndReBar && m_hwndToolBar ){
		// �c�[���o�[�̍������擾����
		DWORD dwBtnSize = Toolbar_GetButtonSize( m_hwndToolBar );
		DWORD dwRows = Toolbar_GetRows( m_hwndToolBar );

		// �o���h����ݒ肷��
		// �ȑO�̃v���b�g�t�H�[���� _WIN32_WINNT >= 0x0600 �Œ�`�����\���̂̃t���T�C�Y��n���Ǝ��s����	// 2007.12.21 ryoji
		rbBand.cbSize = CCSIZEOF_STRUCT( REBARBANDINFO, wID );
		rbBand.fMask  = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE;
		rbBand.fStyle = RBBS_CHILDEDGE;
		rbBand.hwndChild  = m_hwndToolBar;	// �c�[���o�[
		rbBand.cxMinChild = 0;
		rbBand.cyMinChild = HIWORD(dwBtnSize) * dwRows;
		rbBand.cx         = 250;

		// �o���h��ǉ�����
		Rebar_InsertBand( m_hwndReBar, -1, &rbBand );
		::ShowWindow( m_hwndToolBar, SW_SHOW );
	}

	return;
}

void CMainToolBar::DestroyToolBar( void )
{
	if( m_hwndToolBar )
	{
		if( m_hwndSearchBox )
		{
			if( m_fontSearchBox )
			{
				::DeleteObject( m_fontSearchBox );
				m_fontSearchBox = NULL;
			}

			::DestroyWindow( m_hwndSearchBox );
			m_hwndSearchBox = NULL;

			m_pOwner->SetCurrentFocus(0);
		}

		::DestroyWindow( m_hwndToolBar );
		m_hwndToolBar = NULL;

		//if( m_cTabWnd.m_pOwner->GetHwnd() ) ::UpdateWindow( m_cTabWnd.m_pOwner->GetHwnd() );
		//if( m_CFuncKeyWnd.m_pOwner->GetHwnd() ) ::UpdateWindow( m_CFuncKeyWnd.m_pOwner->GetHwnd() );
	}

	// 2006.06.17 ryoji Rebar ��j������
	if( m_hwndReBar )
	{
		::DestroyWindow( m_hwndReBar );
		m_hwndReBar = NULL;
	}

	return;
}

//! ���b�Z�[�W�����B�Ȃ񂩏��������Ȃ� true ��Ԃ��B
bool CMainToolBar::EatMessage(MSG* msg)
{
	if( m_hwndSearchBox && ::IsDialogMessage( m_hwndSearchBox, msg ) ){	//�����R���{�{�b�N�X
		ProcSearchBox( msg );
		return true;
	}
	return false;
}


/*!	@brief ToolBar��OwnerDraw

	@param pnmh [in] Owner Draw���

	@note Common Control V4.71�ȍ~��NMTBCUSTOMDRAW�𑗂��Ă��邪�C
	Common Control V4.70��LPNMCUSTOMDRAW���������Ă��Ȃ��̂�
	���S�̂��ߏ��������ɍ��킹�ď������s���D
	
	@author genta
	@date 2003.07.21 �쐬

*/
LPARAM CMainToolBar::ToolBarOwnerDraw( LPNMCUSTOMDRAW pnmh )
{
	switch( pnmh->dwDrawStage ){
	case CDDS_PREPAINT:
		//	�`��J�n�O
		//	�A�C�e�������O�ŕ`�悷��|��ʒm����
		return CDRF_NOTIFYITEMDRAW;
	
	case CDDS_ITEMPREPAINT:
		//	�ʓ|�������̂ŁC�g��Toolbar�ɕ`���Ă��炤
		//	�A�C�R�����o�^����Ă��Ȃ��̂Œ��g�͉����`����Ȃ�
		return CDRF_NOTIFYPOSTPAINT;
	
	case CDDS_ITEMPOSTPAINT:
		{
			//	�`��
			// �R�}���h�ԍ��ipnmh->dwItemSpec�j����A�C�R���ԍ����擾����	// 2007.11.02 ryoji
			int nIconId = Toolbar_GetBitmap( pnmh->hdr.hwndFrom, (WPARAM)pnmh->dwItemSpec );

			int offset = ((pnmh->rc.bottom - pnmh->rc.top) - CEditApp::Instance()->GetIcons().cy()) / 2;		// �A�C�e����`����̉摜�̃I�t�Z�b�g	// 2007.03.25 ryoji
			int shift = pnmh->uItemState & ( CDIS_SELECTED | CDIS_CHECKED ) ? 1 : 0;	//	Aug. 30, 2003 genta �{�^���������ꂽ�炿����Ɖ摜�����炷
			int color = pnmh->uItemState & CDIS_CHECKED ? COLOR_3DHILIGHT : COLOR_3DFACE;

			//	Sep. 6, 2003 genta �������͉E�����łȂ����ɂ����炷
			CEditApp::Instance()->GetIcons().Draw( nIconId, pnmh->hdc, pnmh->rc.left + offset + shift, pnmh->rc.top + offset + shift,
				(pnmh->uItemState & CDIS_DISABLED ) ? ILD_MASK : ILD_NORMAL
			);
		}
		break;
	default:
		break;
	}
	return CDRF_DODEFAULT;
}


/*! �c�[���o�[�X�V�p�^�C�}�[�̏���
	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
	@date 2003.08.29 wmlhq, ryoji nTimerCount�̓���
	@date 2006.01.28 aroka OnTimer���番��
	@date 2007.04.03 ryoji �p�����[�^�����ɂ���
	@date 2008.10.05 nasukoji �c�[���o�[�X�V�������O�ɏo����
*/
void CMainToolBar::OnToolbarTimer( void )
{
	m_pOwner->IncrementTimerCount(10);

	UpdateToolbar();	// 2008.09.23 nasukoji	�c�[���o�[�̕\�����X�V����
}

/*!
	@brief �c�[���o�[�̕\�����X�V����
	
	@note ������Ăׂ�悤��OnToolbarTimer()���؂�o����
	
	@date 2008.10.05 nasukoji
*/
void CMainToolBar::UpdateToolbar( void )
{
	// ����v���r���[���Ȃ�A�������Ȃ��B
	if( m_pOwner->IsInPreviewMode() )return;
	
	// �c�[���o�[�̏�ԍX�V
	if( m_hwndToolBar )
	{
		for( int i = 0; i < GetDllShareData().m_Common.m_sToolBar.m_nToolBarButtonNum; ++i )
		{
			TBBUTTON tbb = m_pOwner->GetMenuDrawer().getButton(
				GetDllShareData().m_Common.m_sToolBar.m_nToolBarButtonIdxArr[i]
			);

			// �@�\�����p�\�����ׂ�
			Toolbar_EnableButton(
				m_hwndToolBar,
				tbb.idCommand,
				IsFuncEnable( &m_pOwner->GetDocument(), &GetDllShareData(), (EFunctionCode)tbb.idCommand )
			);

			// �@�\���`�F�b�N��Ԃ����ׂ�
			Toolbar_CheckButton(
				m_hwndToolBar,
				tbb.idCommand,
				IsFuncChecked( &m_pOwner->GetDocument(), &GetDllShareData(), (EFunctionCode)tbb.idCommand )
			);
		}
	}
}

//�����{�b�N�X���X�V
void CMainToolBar::AcceptSharedSearchKey()
{
	if( m_hwndSearchBox )
	{
		int	i;
		ComboBox_ResetContent( m_hwndSearchBox );
		for( i = 0; i < GetDllShareData().m_sSearchKeywords.m_aSearchKeys.size(); i++ )
		{
			Combo_AddString( m_hwndSearchBox, GetDllShareData().m_sSearchKeywords.m_aSearchKeys[i] );
		}
		ComboBox_SetCurSel( m_hwndSearchBox, 0 );
	}
}

int CMainToolBar::GetSearchKey(wchar_t* pBuf, int nBufCount)
{
	pBuf[0]=L'\0';
	return ::GetWindowText( m_hwndSearchBox, TcharReceiver<wchar_t>(pBuf,nBufCount), nBufCount );
	/*
	wmemset( szText, 0, _countof(szText) );
	::SendMessage( m_hwndSearchBox, WM_GETTEXT, _MAX_PATH - 1, (LPARAM)szText );
	*/
}


/*!
�c�[���o�[�̌����{�b�N�X�Ƀt�H�[�J�X���ړ�����.
	@date 2006.06.04 yukihane �V�K�쐬
*/
void CMainToolBar::SetFocusSearchBox( void ) const
{
	if( m_hwndSearchBox ){
		::SetFocus(m_hwndSearchBox);
	}
}
