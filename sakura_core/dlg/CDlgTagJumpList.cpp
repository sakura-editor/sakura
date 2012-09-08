/*!	@file
	@brief �^�O�W�����v���X�g�_�C�A���O�{�b�N�X

	@author MIK
	@date 2003.4.13
	@date 2005.03.31 MIK �L�[���[�h�w��TagJump�Ή��̂��ߑ啝�ɕύX
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK
	Copyright (C) 2006, genta, ryoji
	Copyright (C) 2010, Moca

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


#include "StdAfx.h"
#include "dlg/CDlgTagJumpList.h"
#include "CSortedTagJumpList.h"
#include "func/Funccode.h"
#include "env/DLLSHAREDATA.h"
#include "recent/CRecent.h"
#include "util/container.h"
#include "util/shell.h"
#include "util/file.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"


const DWORD p_helpids[] = {
	IDC_LIST_TAGJUMP,		HIDC_LIST_TAGJUMPLIST,			//�t�@�C��
	IDOK,					HIDC_TAGJUMPLIST_IDOK,			//OK
	IDCANCEL,				HIDC_TAGJUMPLIST_IDCANCEL,		//�L�����Z��
	IDC_BUTTON_HELP,		HIDC_BUTTON_TAGJUMPLIST_HELP,	//�w���v
	IDC_KEYWORD,			HDIC_TAGJUMPLIST_KEYWORD,		//�L�[���[�h
//	IDC_CHECK_ICASE,		HIDC_CHECK_ICASE,
//	IDC_CHECK_ANYWHERE,		HIDC_CHECK_ANYWHERE,
//	IDC_BUTTON_NEXTTAG,		HIDC_BUTTON_NEXTTAG,
//	IDC_BUTTON_PREVTAG,		HIDC_BUTTON_PREVTAG,
//	IDC_STATIC,				-1,
	0, 0
};

//�^�O�t�@�C���̃t�H�[�}�b�g	//	@@ 2005.03.31 MIK �萔��
//	@@ 2005.04.03 MIK �L�[���[�h�ɋ󔒂��܂܂��ꍇ�̍l��
#define TAG_FORMAT_2_A       "%[^\t\r\n]\t%[^\t\r\n]\t%d;\"\t%s\t%s"
#define TAG_FORMAT_1_A       "%[^\t\r\n]\t%[^\t\r\n]\t%d"
#define TAG_FILE_INFO_A      "%[^\t\r\n]\t%[^\t\r\n]\t%[^\t\r\n]"
// #define TAG_FORMAT_E_FILE_A  "%[^\t\r\n,],%d"
// #define TAG_FORMAT_E_NAME_A  "%[^\x7f\r\n]\x7f%[^\x7ff\r\n\x01]\x01%d,%d"

//	@@ 2005.03.31 MIK
//�L�[���[�h����͂��ĊY���������\������܂ł̎���(�~���b)
#define TAGJUMP_TIMER_DELAY 700
#define TAGJUMP_TIMER_DELAY_SHORT 50


/*
	ctags.exe ���o�͂���A�g���q�ƑΉ�������
*/
static const TCHAR *p_extentions[] = {
	/*asm*/			_T("asm,s"),								_T("d=define,l=label,m=macro,t=type"),
	/*asp*/			_T("asp,asa"),								_T("f=function,s=sub"),
	/*awk*/			_T("awk,gawk,mawk"),						_T("f=function"),
	/*beta*/		_T("bet"),									_T("f=fragment,p=pattern,s=slot,v=virtual"),
	/*c*/			_T("c,h"),									_T("c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar"),
	/*c++*/			_T("c++,cc,cp,cpp,cxx,h++,hh,hp,hpp,hxx"),	_T("c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar"),
	/*java*/		_T("java"),									_T("c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar"),
	/*vera*/		_T("vr,vri,vrh"),							_T("c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar"),
	/*cobol*/		_T("cbl,cob"),								_T("d=data,f=file,g=group,p=paragraph,P=program,s=section"),
	/*eiffel*/		_T("e"),									_T("c=class,f=feature,l=local"),
	/*fortran*/		_T("f,for,ftn,f77,f90,f95"),				_T("b=block data,c=common,e=entry,f=function,i=interface,k=component,l=label,L=local,m=module,n=namelist,p=program,s=subroutine,t=type,v=variable"),
	/*lisp*/		_T("cl,clisp,el,l,lisp,lsp,ml"),			_T("f=function"),
	/*lua*/			_T("lua"),									_T("f=function"),
	/*makefile*/	_T("mak"),									_T("m=macro"),
	/*pascal*/		_T("p,pas"),								_T("f=function,p=procedure"),
	/*perl*/		_T("pl,pm,perl"),							_T("s=subroutine,p=package"),
	/*php*/			_T("php,php3,phtml"),						_T("c=class,f=function"),
	/*python*/		_T("py,python"),							_T("c=class,f=function,m=member"),
	/*rexx*/		_T("cmd,rexx,rx"),							_T("s=subroutine"),
	/*ruby*/		_T("rb"),									_T("c=class,f=method,F=singleton method,m=mixin"),
	/*scheme*/		_T("sch,scheme,scm,sm"),					_T("f=function,s=set"),
	/*sh*/			_T("sh,bsh,bash,ksh,zsh"),					_T("f=function"),
	/*slang*/		_T("sl"),									_T("f=function,n=namespace"),
	/*sql*/			_T("sql"),									_T("c=cursor,d=prototype,f=function,F=field,l=local,P=package,p=procedure,r=record,s=subtype,t=table,T=trigger,v=variable"),
	/*tcl*/			_T("tcl,tk,wish,itcl"),						_T("p=procedure,c=class,f=method"),
	/*verilog*/		_T("v"),									_T("f=function,m=module,P=parameter,p=port,r=reg,t=task,v=variable,w=wire"),
	/*vim*/			_T("vim"),									_T("f=function,v=variable"),
	/*yacc*/		_T("y"),									_T("l=label"),
//	/*vb*/			_T("bas,cls,ctl,dob,dsr,frm,pag"),			_T("a=attribute,c=class,f=function,l=label,s=procedure,v=variable"),
					NULL,									NULL
};

inline bool CDlgTagJumpList::IsDirectTagJump(){
	return m_bDirectTagJump;
}

inline void CDlgTagJumpList::ClearPrevFindInfo(){
	m_psFindPrev->m_nMatchAll = -1;
	m_psFind0Match->m_nDepth  = -1;
	m_psFind0Match->m_nMatchAll = 0;
}


CDlgTagJumpList::CDlgTagJumpList(bool bDirectTagJump)
	: m_pcList( NULL ),
	  m_bDirectTagJump(bDirectTagJump),
	  m_nIndex( -1 ),
	  m_nLoop( -1 ),
	  m_pszFileName( NULL ),
	  m_pszKeyword( NULL ),
	  m_bTagJumpICase( FALSE ),
	  m_bTagJumpAnyWhere( FALSE ),
	  m_nTimerId( 0 ),
	  m_nTop( 0 ),
	  m_bNextItem( false ),
	  m_strOldKeyword( L"" ),
	  m_psFindPrev( NULL ),
	  m_psFind0Match( NULL )
{
	// 2010.07.22 Moca �y�[�W���O�̗p�� �ő�l��100��50�Ɍ��炷
	m_pcList = new CSortedTagJumpList(50);
	m_psFindPrev = new STagFindState();
	m_psFind0Match = new STagFindState();
	ClearPrevFindInfo();
}

CDlgTagJumpList::~CDlgTagJumpList()
{
	Empty();

	if( m_pszFileName ) free( m_pszFileName );
	m_pszFileName = NULL;
	if( m_pszKeyword ) free( m_pszKeyword );
	m_pszKeyword = NULL;

	StopTimer();
	SAFE_DELETE( m_pcList );
	SAFE_DELETE( m_psFindPrev );
	SAFE_DELETE( m_psFind0Match );
}

/*!
	�^�C�}�[��~

	@author MIK
	@date 2005.03.31 �V�K�쐬
*/
void CDlgTagJumpList::StopTimer( void )
{
	if( m_nTimerId != 0 ){
		::KillTimer( GetHwnd(), m_nTimerId );
		m_nTimerId = 0;
	}
}

/*!
	�^�C�}�[�J�n
	
	�L�[���[�h�w�莞�C�����ԕ������͂��Ȃ���΃��X�g���X�V���邽��
	�u�����ԁv���v��^�C�}�[���K�v

	@author MIK
	@date 2005.03.31 �V�K�쐬
*/
void CDlgTagJumpList::StartTimer( int nDelay = TAGJUMP_TIMER_DELAY )
{
	StopTimer();
	m_nTimerId = ::SetTimer( GetHwnd(), 12345, nDelay, NULL );
}

/*!
	���X�g�̃N���A

	@author MIK
	@date 2005.03.31 �V�K�쐬
*/
void CDlgTagJumpList::Empty( void )
{
	m_nIndex = -1;
	m_pcList->Empty();
}

/*
	���[�_���_�C�A���O�̕\��

	@param[in] lParam 0=�_�C���N�g�^�O�W�����v, 1=�L�[���[�h���w�肵�ă^�O�W�����v
*/
int CDlgTagJumpList::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
)
{
	int ret = (int)CDialog::DoModal( hInstance, hwndParent, IDD_TAGJUMPLIST, lParam );
	StopTimer();
	return ret;
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgTagJumpList::SetData( void )
{
	if( IsDirectTagJump() ){
		m_bTagJumpICase = FALSE;
		::CheckDlgButton( GetHwnd(), IDC_CHECK_ICASE, BST_UNCHECKED );
		m_bTagJumpAnyWhere = FALSE;
		::CheckDlgButton( GetHwnd(), IDC_CHECK_ANYWHERE, BST_UNCHECKED );
		m_bTagJumpExactMatch = TRUE;

		if( m_pszKeyword != NULL ){
			::DlgItem_SetText( GetHwnd(), IDC_KEYWORD, m_pszKeyword );
		}
	}
	//	From Here 2005.04.03 MIK �ݒ�l�̓ǂݍ���
	else{
		HWND hwndKey;
		hwndKey = ::GetDlgItem( GetHwnd(), IDC_KEYWORD );

		m_bTagJumpICase = m_pShareData->m_sTagJump.m_bTagJumpICase;
		::CheckDlgButton( GetHwnd(), IDC_CHECK_ICASE, m_bTagJumpICase ? BST_CHECKED : BST_UNCHECKED );
		m_bTagJumpAnyWhere = m_pShareData->m_sTagJump.m_bTagJumpAnyWhere;
		::CheckDlgButton( GetHwnd(), IDC_CHECK_ANYWHERE, m_bTagJumpAnyWhere ? BST_CHECKED : BST_UNCHECKED );
		m_bTagJumpExactMatch = FALSE;
		Combo_LimitText( hwndKey, _MAX_PATH-1 );
		CRecentTagjumpKeyword cRecentTagJump;
		for( int i = 0; i < cRecentTagJump.GetItemCount(); i++ ){
			Combo_AddString( hwndKey, cRecentTagJump.GetItemText(i) );
		}
		if( m_pszKeyword != NULL ){
			::DlgItem_SetText( GetHwnd(), IDC_KEYWORD, m_pszKeyword );
		}
		else if( cRecentTagJump.GetItemCount() > 0 ){
			Combo_SetCurSel( hwndKey, 0 );
		}
		cRecentTagJump.Terminate();
	}
	//	To Here 2005.04.03 MIK �ݒ�l�̓ǂݍ���
	

	SetTextDir();

	UpdateData(true);

	// �O�̂��ߏォ��UpdateData�̌�Ɉړ�
	if( ! IsDirectTagJump() ){
		StartTimer( TAGJUMP_TIMER_DELAY_SHORT ); // �ŏ��͋K�莞�ԑ҂��Ȃ�
	}
}

/*! @brief Jump���̍X�V

	@date 2005.03.31 MIK 
		�_�C�A���OOpen���ȊO�ɂ��X�V���K�v�Ȃ���SetData()��蕪��
*/
void CDlgTagJumpList::UpdateData( bool bInit )
{
	HWND	hwndList;
	LV_ITEM	lvi;
	int		nIndex;
	int		count;

	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_TAGJUMP );
	ListView_DeleteAllItems( hwndList );

	count = m_pcList->GetCount();

	TCHAR	tmp[32];
	for( nIndex = 0; nIndex < count; nIndex++ )
	{
		CSortedTagJumpList::TagJumpInfo* item;
		item = m_pcList->GetPtr( nIndex );
		if( NULL == item ) break;

		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = nIndex;
		lvi.iSubItem = 0;
		lvi.pszText  = item->keyword;
		ListView_InsertItem( hwndList, &lvi );

		if( item->baseDirId ){
			auto_sprintf( tmp, _T("(%d)"), item->depth );
		}else{
			auto_sprintf( tmp, _T("%d"), item->depth );
		}
		ListView_SetItemText( hwndList, nIndex, 1, tmp );

		auto_sprintf( tmp, _T("%d"), item->no );
		ListView_SetItemText( hwndList, nIndex, 2, tmp );

		TCHAR	*p;
		p = GetNameByType( item->type, item->filename );
		ListView_SetItemText( hwndList, nIndex, 3, p );
		free( p );

		ListView_SetItemText( hwndList, nIndex, 4, item->filename );

		ListView_SetItemText( hwndList, nIndex, 5, item->note );

		ListView_SetItemState( hwndList, nIndex, 0, LVIS_SELECTED | LVIS_FOCUSED );
	}

	/* const */ TCHAR* pszMsgText = NULL;

	//	������������ꍇ�͐؂�̂Ă��|�𖖔��ɑ}��
//	if( m_pcList->IsOverflow() ){
		// 2010.04.03 �u���v�u�O�v�{�^���ǉ����� Overflow���Ȃ��Ȃ���
//		pszMsgText = _T("(����ȍ~�͐؂�̂Ă܂���)");
//	}
	if( (! bInit) && m_pcList->GetCount() == 0 ){
		pszMsgText = _T("(���͌�����܂���ł���)");
	}
	if( pszMsgText ){
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.iItem    = nIndex;
		lvi.iSubItem = 0;
		lvi.pszText  = _T("(�ʒm)");
		lvi.lParam   = -1;
		ListView_InsertItem( hwndList, &lvi );
//		ListView_SetItemText( hwndList, nIndex, 1, _T("") );
//		ListView_SetItemText( hwndList, nIndex, 2, _T("") );
//		ListView_SetItemText( hwndList, nIndex, 3, _T("") );
		ListView_SetItemText( hwndList, nIndex, 4, pszMsgText );
//		ListView_SetItemText( hwndList, nIndex, 5, _T("") );
	}

	if( IsDirectTagJump() && 0 == m_nTop && ! m_bNextItem ){
		// �_�C���N�g�^�O�W�����v�ŁA�y�[�W���O�̕K�v���Ȃ��Ƃ��͔�\��
		::ShowWindow( GetItemHwnd( IDC_BUTTON_NEXTTAG ), SW_HIDE );
		::ShowWindow( GetItemHwnd( IDC_BUTTON_PREVTAG ), SW_HIDE );
	}else{
		::EnableWindow( GetItemHwnd( IDC_BUTTON_NEXTTAG ), m_bNextItem );
		::EnableWindow( GetItemHwnd( IDC_BUTTON_PREVTAG ), (0 < m_nTop) );
	}

	m_nIndex = SearchBestTag();
	if( m_nIndex != -1 )
	{
		ListView_SetItemState( hwndList, m_nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
		ListView_EnsureVisible( hwndList, m_nIndex, FALSE );
	}

	return;
}

/*!	�_�C�A���O�f�[�^�̎擾

	@return TRUE: ����, FALSE: ���̓G���[

	@date 2005.04.03 MIK �ݒ�l�̕ۑ������ǉ�
*/
int CDlgTagJumpList::GetData( void )
{
	HWND	hwndList;

	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_TAGJUMP );
	m_nIndex = ListView_GetNextItem( hwndList, -1, LVIS_SELECTED );

	//	From Here 2005.04.03 MIK �ݒ�l�̕ۑ�
	if( !IsDirectTagJump() )
	{
		m_pShareData->m_sTagJump.m_bTagJumpICase = m_bTagJumpICase;
		m_pShareData->m_sTagJump.m_bTagJumpAnyWhere = m_bTagJumpAnyWhere;
		// 2010.07.22 ��₪��ł��W�����v�ŕ����Ƃ��́A�I�v�V������ۑ�����
		if( m_nIndex == -1 || m_nIndex >= m_pcList->GetCapacity() ){
			return FALSE;
		}
		wchar_t	tmp[MAX_TAG_STRING_LENGTH];
		wcscpy( tmp, L"" );
		::DlgItem_GetText( GetHwnd(), IDC_KEYWORD, tmp, _countof( tmp ) );
		SetKeyword( tmp );

		//�ݒ��ۑ�
		CRecentTagjumpKeyword cRecentTagJumpKeyword;
		cRecentTagJumpKeyword.AppendItem( m_pszKeyword );
		cRecentTagJumpKeyword.Terminate();
	}
	//	To Here 2005.04.03 MIK
	if( m_nIndex == -1 || m_nIndex >= m_pcList->GetCapacity() ) return FALSE;

	return TRUE;
}

/*!
	@date 2005.03.31 MIK
		�K�w�J�����̒ǉ��D�L�[���[�h�w�藓�̒ǉ�
*/
BOOL CDlgTagJumpList::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	HWND		hwndList;
	LV_COLUMN	col;
	RECT		rc;
	long		lngStyle;
	BOOL		bRet;

	_SetHwnd( hwndDlg );

	//���X�g�r���[�̕\���ʒu���擾����B
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_TAGJUMP );
	//ListView_DeleteAllItems( hwndList );
	rc.left = rc.top = rc.right = rc.bottom = 0;
	::GetWindowRect( hwndList, &rc );
	
	int nWidth = (rc.right - rc.left) - ::GetSystemMetrics( SM_CXHSCROLL ) - CTextWidthCalc::WIDTH_MARGIN_SCROLLBER;

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = nWidth * 20 / 100;
	col.pszText  = _T("�L�[���[�h");
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_CENTER;
	col.cx       = nWidth * 7 / 100;
	col.pszText  = _T("�K�w");
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_RIGHT;
	col.cx       = nWidth * 8 / 100;
	col.pszText  = _T("�s�ԍ�");
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = nWidth * 9 / 100;
	col.pszText  = _T("���");
	col.iSubItem = 3;
	ListView_InsertColumn( hwndList, 3, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = nWidth * 35 / 100;
	col.pszText  = _T("�t�@�C����");
	col.iSubItem = 4;
	ListView_InsertColumn( hwndList, 4, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = nWidth * 21 / 100;
	col.pszText  = _T("���l");
	col.iSubItem = 5;
	ListView_InsertColumn( hwndList, 5, &col );

	/* �s�I�� */
	lngStyle = ListView_GetExtendedListViewStyle( hwndList );
	lngStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( hwndList, lngStyle );

	// �_�C���N�g�^�u�W�����v�̎��́A�L�[���[�h���\��
	HWND hwndKey = GetItemHwnd( IDC_KEYWORD );
	int nShowFlag = (IsDirectTagJump() ? SW_HIDE : SW_SHOW);
	::ShowWindow( GetItemHwnd( IDC_STATIC_KEYWORD ), nShowFlag );
	::ShowWindow( hwndKey, nShowFlag );
	::ShowWindow( GetItemHwnd( IDC_CHECK_ICASE ), nShowFlag );
	::ShowWindow( GetItemHwnd( IDC_CHECK_ANYWHERE ), nShowFlag );
	if( IsDirectTagJump() ){
		//�_�C���N�g�^�O�W�����v
		bRet = TRUE;
	}else{
		//�L�[���[�h�w��
		::SetFocus( hwndKey );
		bRet = FALSE;	//for set focus
	}


	/* ���N���X�����o */
	CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
	
	return bRet;
}

BOOL CDlgTagJumpList::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* �w���v */
		MyWinHelp( GetHwnd(), m_pszHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID( F_TAGJUMP_LIST ) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;

	case IDOK:			/* ���E�ɕ\�� */
		StopTimer();
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( GetHwnd(), (BOOL)GetData() );
		return TRUE;

	case IDCANCEL:
		StopTimer();
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;

	// From Here 2005.04.03 MIK ���������ݒ�
	case IDC_CHECK_ICASE:
		m_bTagJumpICase = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_ICASE ) == BST_CHECKED;
		StartTimer( TAGJUMP_TIMER_DELAY_SHORT );
		return TRUE;

	case IDC_CHECK_ANYWHERE:
		m_bTagJumpAnyWhere = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_ANYWHERE ) == BST_CHECKED;
		StartTimer( TAGJUMP_TIMER_DELAY_SHORT );
		return TRUE;
	// To Here 2005.04.03 MIK ���������ݒ�

	case IDC_BUTTON_NEXTTAG:
		m_nTop += m_pcList->GetCapacity();
		StopTimer();
		FindNext( false );
		return TRUE;
	case IDC_BUTTON_PREVTAG:
		m_nTop = std::max(0, m_nTop - m_pcList->GetCapacity());
		StopTimer();
		FindNext( false );
		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgTagJumpList::OnNotify( WPARAM wParam, LPARAM lParam )
{
	NMHDR*	pNMHDR;
	HWND	hwndList;

	pNMHDR = (NMHDR*)lParam;

	hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TAGJUMP );

	//	���ꗗ���X�g�{�b�N�X
	if( hwndList == pNMHDR->hwndFrom )
	{
		switch( pNMHDR->code )
		{
		case NM_DBLCLK:
			StopTimer();
			::EndDialog( GetHwnd(), GetData() );
			return TRUE;
		}
	}

	/* ���N���X�����o */
	return CDialog::OnNotify( wParam, lParam );
}

/*!
	�^�C�}�[�o��

	�^�C�}�[���~���C��⃊�X�g���X�V����
*/
BOOL CDlgTagJumpList::OnTimer( WPARAM wParam )
{
	StopTimer();

	FindNext( true );

	return TRUE;
}

/*!
	�^�C�}�[�o��

	�^�C�}�[���J�n���C��⃊�X�g���X�V���鏀��������
*/
BOOL CDlgTagJumpList::OnCbnEditChange( HWND hwndCtl, int wID )
{
	StartTimer();

	/* ���N���X�����o */
	return CDialog::OnCbnEditChange( hwndCtl, wID );
}

BOOL CDlgTagJumpList::OnCbnSelChange( HWND hwndCtl, int wID )
{
	StartTimer();

	/* ���N���X�����o */
	return CDialog::OnCbnSelChange( hwndCtl, wID );
}

#if 0
BOOL CDlgTagJumpList::OnEnChange( HWND hwndCtl, int wID )
{
	StartTimer();

	/* ���N���X�����o */
	return CDialog::OnEnChange( hwndCtl, wID );
}
#endif

LPVOID CDlgTagJumpList::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

#if 0
bool CDlgTagJumpList::AddParamA( const ACHAR *s0, const ACHAR *s1, int n2, const ACHAR *s3, const ACHAR *s4, int depth, int fileBase )
{
	if( -1 == m_nIndex ) m_nIndex = 0;	//�K��l

	ClearPrevFindInfo();
	m_bNextItem = false;
	m_pcList->AddParamA( s0, s1, n2, s3[0], s4, depth, fileBase );

	return true;
}
#endif

bool CDlgTagJumpList::GetSelectedParam( TCHAR *s0, TCHAR *s1, int *n2, TCHAR *s3, TCHAR *s4, int *depth, TCHAR *baseDir )
{
	if( 1 != m_pcList->GetCount() ){
		if( -1 == m_nIndex || m_nIndex >= m_pcList->GetCount() ) return false;
	}else{
		m_nIndex = 0;
	}

	m_pcList->GetParam( m_nIndex, s0, s1, n2, &s3[0], s4, depth, baseDir );

	return true;
}

bool CDlgTagJumpList::GetSelectedFullPathAndLine( TCHAR *fullPath, int count, int *lineNum, int *depth )
{
	TCHAR path[1024];
	TCHAR fileName[1024];
	TCHAR dirFileName[1024];
	int tempDepth = 0;
	SplitPath_FolderAndFile( GetFilePath(), path, NULL );
	AddLastYenFromDirectoryPath( path );
	
	if( false == GetSelectedParam( NULL, fileName, lineNum, NULL, NULL, &tempDepth, dirFileName ) ){
		return false;
	}
	if( depth ){
		*depth = tempDepth;
	}
	const TCHAR* fileNamePath;
	// �t�@�C�����A�f�B���N�g���w��A��t�@�C���p�X�A�̏��ɓK�p�B�r���Ńt���p�X�Ȃ炻�̂܂܁B
	if( dirFileName[0] ){
		AddLastYenFromDirectoryPath( dirFileName );
		const TCHAR	*p = fileName;
		if( p[0] == _T('\\') ){
			if( p[1] == _T('\\') ){
				auto_strcpy( dirFileName, p );
			}else{
				auto_strcpy( dirFileName, p );
			}
		}else if( _istalpha( p[0] ) && p[1] == _T(':') ){
			auto_strcpy( dirFileName, p );
		}else{
			// ���΃p�X�F�A������
			auto_strcat( dirFileName, p );
		}
		fileNamePath = dirFileName;
	}else{
		fileNamePath = fileName;
	}
	bool ret = NULL != GetFullPathFromDepth( fullPath, count, path, fileNamePath, tempDepth );
	if(ret){
		DEBUG_TRACE( _T("jump to: %ts\n"), static_cast<const TCHAR*>(fullPath) );
	}else{
		DEBUG_TRACE( _T("jump to: error\n") );
	}
	return ret;
}

/*!
	@return �u.ext�v�`���̃^�C�v���B free���邱��
*/
TCHAR *CDlgTagJumpList::GetNameByType( const TCHAR type, const TCHAR *name )
{
	const TCHAR	*p;
	TCHAR	*token;
	int		i;
	//	2005.03.31 MIK
	TCHAR	tmp[MAX_TAG_STRING_LENGTH];

	p = _tcsrchr( name, _T('.') );
	if( ! p ) p = _T(".c");	//������Ȃ��Ƃ��� ".c" �Ƒz�肷��B
	p++;

	for( i = 0; p_extentions[i]; i += 2 )
	{
		_tcscpy( tmp, p_extentions[i] );
		token = _tcstok( tmp, _T(",") );
		while( token )
		{
			if( _tcsicmp( p, token ) == 0 )
			{
				_tcscpy( tmp, p_extentions[i+1] );
				token = _tcstok( tmp, _T(",") );
				while( token )
				{
					if( token[0] == type )
					{
						return _tcsdup( &token[2] );
					}

					token = _tcstok( NULL, _T(",") );
				}

				return _tcsdup( _T("") );
			}

			token = _tcstok( NULL, _T(",") );
		}
	}

	return _tcsdup( _T("") );
}

/*!
	��t�@�C������ݒ�
*/
void CDlgTagJumpList::SetFileName( const TCHAR *pszFileName )
{
	assert_warning( pszFileName );
	if( NULL == pszFileName ) return;

	if( m_pszFileName ) free( m_pszFileName );

	m_pszFileName = _tcsdup( pszFileName );
	
	m_nLoop = CalcMaxUpDirectory( m_pszFileName );
}

/*!
	�����L�[���[�h�̐ݒ�

*/
void CDlgTagJumpList::SetKeyword( const wchar_t *pszKeyword )
{
	if( NULL == pszKeyword ) return;

	if( m_pszKeyword ) free( m_pszKeyword );

	m_pszKeyword = wcsdup( pszKeyword );

	return;
}

/*!
	����ꂽ��₩��ł����҂ɋ߂��Ǝv������̂�
	�I�яo���D(�����I���ʒu����̂���)

	@return �I�����ꂽ�A�C�e����index

*/
int CDlgTagJumpList::SearchBestTag( void )
{
	if( m_pcList->GetCount() <= 0 ) return -1;	//�I�ׂ܂���B
	if( NULL == m_pszFileName ) return 0;

	TCHAR	szFileSrc[1024];
	TCHAR	szFileDst[1024];
	TCHAR	szExtSrc[1024];
	TCHAR	szExtDst[1024];
	int		nMatch = -1;
	int		i;
	int		count;

	_tcscpy( szFileSrc, _T("") );
	_tcscpy( szExtSrc,  _T("") );
	_tsplitpath( m_pszFileName, NULL, NULL, szFileSrc, szExtSrc );

	count = m_pcList->GetCount();

	for( i = 0; i < count; i++ )
	{
		CSortedTagJumpList::TagJumpInfo* item;
		item = m_pcList->GetPtr( i );

		_tcscpy( szFileDst, _T("") );
		_tcscpy( szExtDst,  _T("") );
		_tsplitpath( item->filename, NULL, NULL, szFileDst, szExtDst );
		
		if( _tcsicmp( szFileSrc, szFileDst ) == 0 )
		{
			if( _tcsicmp( szExtSrc, szExtDst ) == 0 ) return i;
			if( nMatch == -1 ) nMatch = i;
		}
	}

	if( nMatch != -1 ) return nMatch;

	return 0;
}

/*!
	@param bNewFind �V������������(���E�O�̂Ƃ�false)
*/
void CDlgTagJumpList::FindNext( bool bNewFind )
{
	wchar_t	szKey[ MAX_TAG_STRING_LENGTH ];
	wcscpy( szKey, L"" );
	::DlgItem_GetText( GetHwnd(), IDC_KEYWORD, szKey, _countof( szKey ) );
	if( bNewFind ){
		// �O��̃L�[���[�h����̍i�������̂Ƃ��ŁAtags���X�L�b�v�ł���Ƃ��̓X�L�b�v
		if( -1 < m_psFind0Match->m_nDepth
			&& (m_bTagJumpAnyWhere == m_bOldTagJumpAnyWhere || FALSE == m_bTagJumpAnyWhere)
			&& (m_bTagJumpICase    == m_bOldTagJumpICase    || FALSE == m_bTagJumpICase)
			&& 0 == wcsncmp( m_strOldKeyword.GetStringPtr(), szKey, m_strOldKeyword.GetStringLength() ) ){
			// ���̃L�[���[�h�łP�����q�b�g���Ȃ�tags������̂Ŕ�΂�
			// �����͓������A�������Ȃ�Ȃ���Ȃ�
		}else{
			ClearPrevFindInfo();
		}
		m_nTop = 0;
	}
	find_key( szKey );
	UpdateData( false );
}

/*!
	�_�C���N�g�^�O�W�����v����(DoModal�O�Ɏ��s)
*/
int CDlgTagJumpList::FindDirectTagJump()
{
	return find_key_core(
		0,	// 0�J�n
		m_pszKeyword,
		false, // ������v
		true,  // ���S��v
		false, // �召�����
		true,  // �������[�h
		1
	);
}

void CDlgTagJumpList::find_key( const wchar_t* keyword )
{
	DlgItem_SetText( GetHwnd(), IDC_STATIC_KEYWORD, _T("�L�[���[�h ������...") );
	::UpdateWindow(GetItemHwnd(IDC_STATIC_KEYWORD));

	find_key_core(
		m_nTop,
		keyword,
		FALSE != m_bTagJumpAnyWhere,
		FALSE != m_bTagJumpExactMatch,
		FALSE != m_bTagJumpICase,
		IsDirectTagJump(),
		IsDirectTagJump() ? 1 : 3
	);
	DlgItem_SetText( GetHwnd(), IDC_STATIC_KEYWORD, _T("�L�[���[�h") );
	::UpdateWindow(GetItemHwnd(IDC_STATIC_KEYWORD));
}

/*!
	�^�O�t�@�C������L�[���[�h�Ƀ}�b�`����f�[�^�𒊏o���Cm_cList�ɐݒ肷��

	@date 2007.03.13 genta �o�b�t�@�I�[�o�[�����b��Ώ��Ńo�b�t�@�T�C�Y�ύX
	@date 2010.04.02 Moca ���낢��ύX�BSJIS�œǂށB�y�[�W���O, format=1�̉��߁A�^�O�t�@�C�����̗��p
		�u�Stags�̌������ʂ��\�[�g���Đ擪����Capaticy�܂Łv���utags�t�@�C����(=depth)���A�L�[���[�h���v�ɕύX
	@date 2010.07.21 find_key��find_key_core�ɂ��āACViewCommander::Command_TagJumpByTagsFile�Ɠ���
*/
int CDlgTagJumpList::find_key_core(
	int  nTop,
	const wchar_t* keyword,
	bool bTagJumpAnyWhere, // ������v
	bool bTagJumpExactMatch, // ���S��v
	bool bTagJumpICase,
	bool bTagJumpICaseByTags, // Tag�t�@�C�����̃\�[�g�ɏ]��
	int  nDefaultNextMode
){
	assert_warning( !(bTagJumpAnyWhere && bTagJumpExactMatch) );

	// to_achar�͈ꎞ�o�b�t�@�Ŕj�󂳂��\��������̂ŃR�s�[
	CNativeA cmemKeyA = CNativeA(to_achar(keyword));
	const ACHAR* paszKeyword = cmemKeyA.GetStringPtr();
	int	length = cmemKeyA.GetStringLength();

	Empty();

	m_strOldKeyword.SetString( keyword );
	m_bOldTagJumpAnyWhere = bTagJumpAnyWhere;
	m_bOldTagJumpICase    = bTagJumpICase;
	m_bNextItem = false;

	if( length == 0 ){
		ClearPrevFindInfo();
		return -1;
	}
	// �����ϊ����Ă݂Ĉ�v���Ȃ�������A�����L�[�ɂ͈�v���Ȃ��Ƃ������Ƃɂ���
	if( 0 != wcscmp( to_wchar( paszKeyword ), keyword ) ){
		ClearPrevFindInfo();
		return -1;
	}
	CSortedTagJumpList& cList = *m_pcList;
	const int nCap = cList.GetCapacity();
	STagFindState state;
	state.m_nDepth    = 0;
	state.m_nMatchAll = 0;
	state.m_nNextMode = nDefaultNextMode;
	state.m_nLoop     = -1;
	state.m_bJumpPath = false;	// �e�ȊO�̃p�X�̈ړ���w��
	state.m_szCurPath[0] = 0;
	
	// �O��̌��ʂ��猟���Ώ�tags���i��
	if( m_psFindPrev->m_nMatchAll <= nTop && -1 < m_psFindPrev->m_nMatchAll ){
		// �w��y�[�W�̌������X�L�b�v
		state = *m_psFindPrev;
		DEBUG_TRACE( _T("skip count  d:%d m:%d n:%d\n"), state.m_nDepth, state.m_nMatchAll, state.m_nNextMode );
	}else if( 0 <= m_psFind0Match->m_nDepth ){
		// depth���󂢏��Ƀq�b�g���Ȃ����������X�L�b�v
		state = *m_psFind0Match;
		DEBUG_TRACE( _T("skip 0match d:%d m:%d n:%d\n"), state.m_nDepth, state.m_nMatchAll, state.m_nNextMode );
	}else{
		// ����or�g���Ȃ��Ƃ��̓N���A
		ClearPrevFindInfo();
		// �t�@�C�������R�s�[�������ƁA�f�B���N�g��(�Ō�\)�݂̂ɂ���
		_tcscpy( state.m_szCurPath, GetFilePath() );
		state.m_szCurPath[ GetFileName() - GetFilePath() ] = _T('\0');
		state.m_nLoop = m_nLoop;
	}
	
	TCHAR	szTagFile[1024];		//�^�O�t�@�C��
	TCHAR	szNextPath[1024];		//�������t�H���_
	ACHAR	szLineData[1024];		//�s�o�b�t�@
	ACHAR	s[4][1024];
	int		n2;
	szNextPath[0] = _T('\0');
	vector_ex<std::tstring> seachDirs;

	// �p�X��Jump�ŏz���Ă���ꍇ�ɍő�l���K������
	for( ; state.m_nDepth <= state.m_nLoop && state.m_nDepth < (_MAX_PATH/2); state.m_nDepth++ )
	{
		// 0 ���̃t�@�C���͌������Ȃ�
		// 1 1�ł��q�b�g�����玟�͌������Ȃ�
		// 2 ���S��v�̂Ƃ���1�ɓ����B ����ȊO��3�ɓ���
		// 3 �K����������
		if( 0 == state.m_nNextMode ) break;
		if( 1 == state.m_nNextMode && 0 < state.m_nMatchAll ) break;
		if( 2 == state.m_nNextMode && bTagJumpExactMatch && 0 < state.m_nMatchAll ) break; 

		{
			std::tstring curPath = state.m_szCurPath;
			if( seachDirs.exist( curPath ) ){
				// �����ς� =>�I��
				break;
			}
			seachDirs.push_back( curPath );
		}

		//�^�O�t�@�C�������쐬����B
		auto_sprintf( szTagFile, _T("%ts%ts"), state.m_szCurPath, TAG_FILENAME_T );
		DEBUG_TRACE( _T("tag: %ts\n"), szTagFile );
		
		//�^�O�t�@�C�����J���B
		FILE* fp = _tfopen( szTagFile, _T("rb") );
		if( fp )
		{
			DEBUG_TRACE( _T("open tags\n") );
			bool bSorted = true;
			bool bFoldcase = false;
			int  nTagFormat = 2; // 2��1���ǂ߂�̂Ńf�t�H���g��2
			int  nLines = 0;
			int  baseDirId = 0;
			if( state.m_bJumpPath ){
				baseDirId = cList.AddBaseDir( state.m_szCurPath );
			}
			state.m_nNextMode = nDefaultNextMode;

			// �o�b�t�@�̌�납��2�����ڂ�\0���ǂ����ŁA�s���܂œǂݍ��񂾂��m�F����
			const int nLINEDATA_LAST_CHAR = _countof( szLineData ) - 2;
			szLineData[nLINEDATA_LAST_CHAR] = '\0';
			while( fgets( szLineData, _countof( szLineData ), fp ) )
			{
				nLines++;
				int  nRet;
				// fgets���s���ׂĂ�ǂݍ��߂Ă��Ȃ��ꍇ�̍l��
				if( '\0' != szLineData[nLINEDATA_LAST_CHAR]
				    && '\n' != szLineData[nLINEDATA_LAST_CHAR] ){
					// ���s�R�[�h�܂ł��̂Ă�
					int ch = fgetc( fp );
					while( ch != '\n' && ch != EOF ){
						ch = fgetc( fp );
					}
				}
				if( 1 == nLines && szLineData[0] == '\x0c' ){
					// etags�Ȃ̂Ŏ��̃t�@�C��
					break;
				}
				if( '!' == szLineData[0] ){
					if( 0 == strncmp_literal( szLineData + 1, "_TAG_" ) ){
						s[0][0] = s[1][0] = s[2][0] = 0;
						nRet = sscanf(
							szLineData, 
							TAG_FILE_INFO_A,	//tags�t�@�C�����
							s[0], s[1], s[2]
						);
						if( nRet < 2 ) goto next_line;
						const ACHAR* pTag = s[0] + 6;
						if( 0 == strncmp_literal(pTag , "FILE_FORMAT" ) ){
							n2 = atoi( s[1] );
							if( 1 <=n2 && n2 <= 2 ){
								nTagFormat = n2;
							}
						}else if( 0 == strncmp_literal( pTag, "FILE_SORTED" ) ){
							n2 = atoi( s[1] );
							bSorted   = (1 == n2);
							bFoldcase = (2 == n2);
							if( bTagJumpICaseByTags ){
								bTagJumpICase = bFoldcase;
							}
						}else if( 0 == strncmp_literal( pTag, "S_SEARCH_NEXT" ) ){
							// �Ǝ��g��:���Ɍ�������tag�t�@�C���̎w��
							if( '0' <= s[1][0] && s[1][0] <= '3' ){
								n2 = atoi( s[1] );
								if( 0 <= n2 && n2 <= 3 ){
									state.m_nNextMode = n2;
								}
								if( 1 <= n2 && s[2][0] ){
									// s[2] == ��΃p�X(�f�B���N�g��)
									TCHAR baseWork[1024];
									CopyDirDir( baseWork, to_tchar(s[2]), state.m_szCurPath );
									szNextPath[0] = 0;
									if( !GetLongFileName( baseWork, szNextPath ) ){
										// �G���[�Ȃ�ϊ��O��K�p
										auto_strcpy( szNextPath, baseWork );
									}
								}
							}
						}else if( 0 == strncmp_literal( pTag, "S_FILE_BASEDIR" ) ){
							TCHAR baseWork[1024];
							// �Ǝ��g��:�t�@�C�����̊�f�B���N�g��
							if( state.m_bJumpPath ){
								// �p�X�e�ǂݑւ����́A���΃p�X�������ꍇ�ɘA�����K�v
								CopyDirDir( baseWork, to_tchar(s[1]), state.m_szCurPath );
								baseDirId = cList.AddBaseDir( baseWork );
							}else{
								auto_strcpy( baseWork, to_tchar(s[1]) );
								AddLastYenFromDirectoryPath( baseWork );
								baseDirId = cList.AddBaseDir( baseWork );
							}
						}
					}
					goto next_line;	//�R�����g�Ȃ�X�L�b�v
				}
				if( szLineData[0] < '!' ) goto next_line;
				//chop( szLineData );

				s[0][0] = s[1][0] = s[2][0] = s[3][0] = '\0';
				n2 = 0;
				//	@@ 2005.03.31 MIK TAG_FORMAT�萔��
				if( 2 == nTagFormat ){
					nRet = sscanf(
						szLineData, 
						TAG_FORMAT_2_A,	//�g��tags�t�H�[�}�b�g
						s[0], s[1], &n2, s[2], s[3]
						);
					// 2010.04.02 nRet < 4 ��3�ɕύX�B�W���t�H�[�}�b�g���ǂݍ���
					if( nRet < 3 ) goto next_line;
					if( n2 <= 0 ) goto next_line;	//�s�ԍ��s��(-excmd=n���w�肳��ĂȂ�����)
				}else{
					nRet = sscanf(
						szLineData, 
						TAG_FORMAT_1_A,	//tags�t�H�[�}�b�g
						s[0], s[1], &n2
						);
					if( nRet < 2 ) goto next_line;
					if( n2 <= 0 ) goto next_line;
				}

				int  cmp;
				if( bTagJumpAnyWhere ){
					if( bTagJumpICase ){
						cmp = stristr_j( s[0], paszKeyword ) != NULL ? 0 : -1;
					}else{
						cmp = strstr_j( s[0], paszKeyword ) != NULL ? 0 : -1;
					}
				}else{
					if( bTagJumpExactMatch ){
						// ���S��v
						if( bTagJumpICase ){
							cmp = auto_stricmp( s[0], paszKeyword );
						}else{
							cmp = auto_strcmp( s[0], paszKeyword );
						}
					}else{
						// �O����v
						if( bTagJumpICase ){
							cmp = my_strnicmp( s[0], paszKeyword, length );
						}else{
							cmp = strncmp( s[0], paszKeyword, length );
						}
					}
				}
				if( 0 == cmp ){
					state.m_nMatchAll++;
					if( nTop < state.m_nMatchAll ){
						if( cList.GetCount() < nCap ){
							cList.AddParamA( s[0], s[1], n2, s[2][0], s[3], state.m_nDepth, baseDirId );
						}else{
							// �T���ł��؂�(���y�[�W�ł�蒼��)
							m_bNextItem = true;
							break;
						}
					}
				}
				else if( 0 < cmp ){
					//	tags�̓\�[�g����Ă���̂ŁC�擪�����case sensitive��
					//	��r���ʂɂ���Č����̎��͏����̑ł��؂肪�\
					//	2005.04.05 MIK �o�O�C��
					if( (!bTagJumpICase) && bSorted && (!bTagJumpAnyWhere) ) break;
					// 2010.07.21 Foldcase�����ł��؂�B������tags�ƃT�N�����̃\�[�g���������łȂ���΂Ȃ�Ȃ�
					if( bTagJumpICase  && bFoldcase && (!bTagJumpAnyWhere) ) break;
				}
next_line:
				;
				szLineData[nLINEDATA_LAST_CHAR] = '\0';
			}

			//�t�@�C�������B
			fclose( fp );
			DEBUG_TRACE( _T("close m:%d\n "), state.m_nMatchAll );
		}
		
		if( szNextPath[0] ){
			state.m_bJumpPath = true;
			auto_strcpy( state.m_szCurPath, szNextPath );
			state.m_nLoop = CalcMaxUpDirectory( state.m_szCurPath );
			szNextPath[0] = 0;
		}else{
//			_tcscat( state.m_szCurPath, _T("..\\") );
			//�J�����g�p�X��1�K�w��ցB
			DirUp( state.m_szCurPath );
		}
		
		if( 0 != state.m_nMatchAll && false == m_bNextItem ){
			// 0 �y�[�W�߂���p: �ł��؂��Ă��Ȃ��̂Ŏ��̃y�[�W�ł́A����tags�̎����猟���ł���
			// (�Ō�ɒʉ߂������̂�ێ�)
			*m_psFindPrev = state;
			++(m_psFindPrev->m_nDepth);
			DEBUG_TRACE( _T("FindPrev udpate: d:%d m:%d n:%d L:%d j:%d\n") , m_psFindPrev->m_nDepth, m_psFindPrev->m_nMatchAll, m_psFindPrev->m_nNextMode, m_psFindPrev->m_nLoop, (int)m_psFindPrev->m_bJumpPath );
		}
		if( 0 == state.m_nMatchAll ){
			// �L�[���[�h�i���ݗp: ���̍i�荞�݌����ł́A����tags�̎����猟���ł���
			// (�Ō�ɒʉ߂������̂�ێ�)
			*m_psFind0Match = state;
			++(m_psFind0Match->m_nDepth);
			DEBUG_TRACE( _T("Find0Match udpate: d:%d m:%d n:%d L:%d j:%d\n") , m_psFind0Match->m_nDepth, m_psFind0Match->m_nMatchAll, m_psFind0Match->m_nNextMode, m_psFind0Match->m_nLoop, (int)m_psFind0Match->m_bJumpPath );
		}
		if( m_bNextItem ){
			break;
		}
	}
	return state.m_nMatchAll;
}

/*!
	�p�X����t�@�C���������݂̂����o���D(2�o�C�g�Ή�)
*/
const TCHAR* CDlgTagJumpList::GetFileName( void )
{
	return GetFileTitlePointer(GetFilePath());
}


void CDlgTagJumpList::SetTextDir()
{
	if( GetHwnd() ){
		DlgItem_SetText( GetHwnd(), IDC_STATIC_BASEDIR, _T("") );
		if( GetFileName() ){
			std::tstring strPath = GetFilePath();
			strPath[ GetFileName() - GetFilePath() ] = _T('\0');
			DlgItem_SetText( GetHwnd(), IDC_STATIC_BASEDIR, strPath.c_str() );
		}
	}
}

int CDlgTagJumpList::CalcMaxUpDirectory( const TCHAR* p )
{
	int loop = CalcDirectoryDepth( p );
	if( loop <  0 ) loop =  0;
	if( loop > (_MAX_PATH/2) ) loop = (_MAX_PATH/2);	//\A\B\C...�̂悤�ȂƂ�1�t�H���_��2���������̂�...
	return loop;
}

/*!
	
	@date 2010.04.02 Moca Command_TagJumpByTagsFileKeyword���番���E�ړ�
	@param basePath [in,out] \�t�f�B���N�g���p�X��΃p�X�����B���������̂ɒ���
	@param fileName [in] ���΁E��΃t�@�C�����p�X
	@param depth    [in] fineName����΃p�X�̎������B1==1��̃f�B���N�g��
	@retval pszOutput ���� �uC:\dir1\filename.txt�v�̌`��(..\�t���͔p�~)
	@retval NULL   ���s
*/
TCHAR* CDlgTagJumpList::GetFullPathFromDepth( TCHAR* pszOutput, int count,
	TCHAR* basePath, const TCHAR* fileName, int depth )
{
	DEBUG_TRACE( _T("base  %ts\n"), basePath );
	DEBUG_TRACE( _T("file  %ts\n"), fileName );
	DEBUG_TRACE( _T("depth %d\n"),  depth );
	//���S�p�X�����쐬����B
	const TCHAR	*p = fileName;
	if( p[0] == _T('\\') ){	//�h���C�u�Ȃ���΃p�X���H
		if( p[1] == _T('\\') ){	//�l�b�g���[�N�p�X���H
			_tcscpy( pszOutput, p );	//�������H���Ȃ��B
		}else{
			//�h���C�u���H�����ق����悢�H
			_tcscpy( pszOutput, p );	//�������H���Ȃ��B
		}
	}else if( _istalpha( p[0] ) && p[1] == _T(':') ){	//��΃p�X���H
		_tcscpy( pszOutput, p );	//�������H���Ȃ��B
	}else{
		for( int i = 0; i < depth; i++ ){
			//_tcscat( basePath, _T("..\\") );
			DirUp( basePath );
		}
		if( -1 == auto_snprintf_s( pszOutput, count, _T("%ts%ts"), basePath, p ) ){
			return NULL;
		}
	}
	return pszOutput;
}

/*!
	�f�B���N�g���ƃf�B���N�g����A������
*/
TCHAR* CDlgTagJumpList::CopyDirDir( TCHAR* dest, const TCHAR* target, const TCHAR* base )
{
	if( _IS_REL_PATH( target ) ){
		auto_strcpy( dest, base );
		AddLastYenFromDirectoryPath( dest );
		auto_strcat( dest, target );
	}else{
		auto_strcpy( dest, target );
	}
	AddLastYenFromDirectoryPath( dest );
	return dest;
}

/*
	@param dir [in,out] �t�H���_�̃p�X 
	in == C:\dir\subdir\
	out == C:\dir\
*/
TCHAR* CDlgTagJumpList::DirUp( TCHAR* dir )
{
	CutLastYenFromDirectoryPath( dir );
	const TCHAR *p = GetFileTitlePointer(dir); //�Ō��\�̎��̕������擾 last_index_of('\\') + 1;
	if( 0 < p - dir){
		dir[p - dir] = '\0';
	}
	return dir;
}
