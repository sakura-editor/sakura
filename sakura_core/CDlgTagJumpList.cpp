/*!	@file
	@brief タグジャンプリストダイアログボックス

	@author MIK
	@date 2003.4.13
	@date 2005.03.31 MIK キーワード指定TagJump対応のため大幅に変更
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK
	Copyright (C) 2006, genta, ryoji

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



#include "stdafx.h"
#include <stdio.h>
#include "sakura_rc.h"
#include "global.h"
#include "funccode.h"
#include "mymessage.h"
#include "CDialog.h"
#include "CDlgTagJumpList.h"
#include "CSortedTagJumpList.h"
//#include <Shlwapi.h> // 2006.01.08 genta Not used
#include "CRecent.h"
#include "etc_uty.h"
#include "debug.h"
#include "my_icmp.h"
#include "charcode.h"  // 2006.06.28 rastiv

#include "sakura.hh"
const DWORD p_helpids[] = {
	IDC_LIST_TAGJUMP,		HIDC_LIST_TAGJUMPLIST,			//ファイル
	IDOK,					HIDC_TAGJUMPLIST_IDOK,			//OK
	IDCANCEL,				HIDC_TAGJUMPLIST_IDCANCEL,		//キャンセル
	IDC_BUTTON_HELP,		HIDC_BUTTON_TAGJUMPLIST_HELP,	//ヘルプ
	IDC_KEYWORD,			HDIC_TAGJUMPLIST_KEYWORD,		//キーワード
//	IDC_STATIC,				-1,
	0, 0
};

/*
	ctags.exe が出力する、拡張子と対応する種類
*/
static const char *p_extentions[] = {
	/*asm*/			"asm,s",								"d=define,l=label,m=macro,t=type",
	/*asp*/			"asp,asa",								"f=function,s=sub",
	/*awk*/			"awk,gawk,mawk",						"f=function",
	/*beta*/		"bet",									"f=fragment,p=pattern,s=slot,v=virtual",
	/*c*/			"c,h",									"c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar",
	/*c++*/			"c++,cc,cp,cpp,cxx,h++,hh,hp,hpp,hxx",	"c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar",
	/*java*/		"java",									"c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar",
	/*vera*/		"vr,vri,vrh",							"c=class,d=macro,e=enumerator,f=function,g=enum,m=member,n=namespace,p=prototype,s=struct,t=typedef,u=union,v=variable,x=externvar",
	/*cobol*/		"cbl,cob",								"d=data,f=file,g=group,p=paragraph,P=program,s=section",
	/*eiffel*/		"e",									"c=class,f=feature,l=local",
	/*fortran*/		"f,for,ftn,f77,f90,f95",				"b=block data,c=common,e=entry,f=function,i=interface,k=component,l=label,L=local,m=module,n=namelist,p=program,s=subroutine,t=type,v=variable",
	/*lisp*/		"cl,clisp,el,l,lisp,lsp,ml",			"f=function",
	/*lua*/			"lua",									"f=function",
	/*makefile*/	"mak",									"m=macro",
	/*pascal*/		"p,pas",								"f=function,p=procedure",
	/*perl*/		"pl,pm,perl",							"s=subroutine,p=package",
	/*php*/			"php,php3,phtml",						"c=class,f=function",
	/*python*/		"py,python",							"c=class,f=function,m=member",
	/*rexx*/		"cmd,rexx,rx",							"s=subroutine",
	/*ruby*/		"rb",									"c=class,f=method,F=singleton method,m=mixin",
	/*scheme*/		"sch,scheme,scm,sm",					"f=function,s=set",
	/*sh*/			"sh,bsh,bash,ksh,zsh",					"f=function",
	/*slang*/		"sl",									"f=function,n=namespace",
	/*sql*/			"sql",									"c=cursor,d=prototype,f=function,F=field,l=local,P=package,p=procedure,r=record,s=subtype,t=table,T=trigger,v=variable",
	/*tcl*/			"tcl,tk,wish,itcl",						"p=procedure,c=class,f=method",
	/*verilog*/		"v",									"f=function,m=module,P=parameter,p=port,r=reg,t=task,v=variable,w=wire",
	/*vim*/			"vim",									"f=function,v=variable",
	/*yacc*/		"y",									"l=label",
//	/*vb*/			"bas,cls,ctl,dob,dsr,frm,pag",			"a=attribute,c=class,f=function,l=label,s=procedure,v=variable",
					NULL,									NULL
};



CDlgTagJumpList::CDlgTagJumpList()
	: m_cList( 100 ),
	  m_nIndex( -1 ),
	  m_nLoop( -1 ),
	  m_pszFileName( NULL ),
	  m_pszKeyword( NULL ),
	  m_bTagJumpICase( FALSE ),
	  m_bTagJumpAnyWhere( FALSE ),
	  m_nTimerId( 0 )
{
}

CDlgTagJumpList::~CDlgTagJumpList()
{
	Empty();
	m_nLoop = 0;

	if( m_pszFileName ) free( m_pszFileName );
	m_pszFileName = NULL;
	if( m_pszKeyword ) free( m_pszKeyword );
	m_pszKeyword = NULL;

	StopTimer();
}

/*!
	タイマー停止

	@author MIK
	@date 2005.03.31 新規作成
*/
void CDlgTagJumpList::StopTimer( void )
{
	if( m_nTimerId != 0 ){
		::KillTimer( m_hWnd, m_nTimerId );
		m_nTimerId = 0;
	}
}

/*!
	タイマー開始
	
	キーワード指定時，一定期間文字入力がなければリストを更新するため
	「一定期間」を計るタイマーが必要

	@author MIK
	@date 2005.03.31 新規作成
*/
void CDlgTagJumpList::StartTimer( void )
{
	StopTimer();
	m_nTimerId = ::SetTimer( m_hWnd, 12345, TAGJUMP_TIMER_DELAY, NULL );
}

/*!
	リストのクリア

	@author MIK
	@date 2005.03.31 新規作成
*/
void CDlgTagJumpList::Empty( void )
{
	m_nIndex = -1;
	m_cList.Empty();
}

/*
	モーダルダイアログの表示

	@param[in] lParam 0=ダイレクトタグジャンプ, 1=キーワードを指定してタグジャンプ
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

/* ダイアログデータの設定 */
void CDlgTagJumpList::SetData( void )
{
	//	From Here 2005.04.03 MIK 設定値の読み込み
	if( 0 != m_lParam )
	{
		HWND hwndKey;
		hwndKey = ::GetDlgItem( m_hWnd, IDC_KEYWORD );

		m_bTagJumpICase = m_pShareData->m_bTagJumpICase;
		::CheckDlgButton( m_hWnd, IDC_CHECK_ICASE, m_bTagJumpICase ? BST_CHECKED : BST_UNCHECKED );
		m_bTagJumpAnyWhere = m_pShareData->m_bTagJumpAnyWhere;
		::CheckDlgButton( m_hWnd, IDC_CHECK_ANYWHERE, m_bTagJumpAnyWhere ? BST_CHECKED : BST_UNCHECKED );
		::SendMessage( hwndKey, CB_LIMITTEXT, (WPARAM)_MAX_PATH-1, 0 );
		CRecent cRecent;
		cRecent.EasyCreate( RECENT_FOR_TAGJUMP_KEYWORD );
		for( int i = 0; i < cRecent.GetItemCount(); i++ ){
			::SendMessage( hwndKey, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)cRecent.GetItem(i) );
		}
		if( m_pszKeyword != NULL ){
			::SetDlgItemText( m_hWnd, IDC_KEYWORD, m_pszKeyword );
		}
		else if( cRecent.GetItemCount() > 0 ){
			::SendMessage( hwndKey, CB_SETCURSEL, 0, 0 );
		}
		cRecent.Terminate();
		StartTimer();
	}
	//	To Here 2005.04.03 MIK 設定値の読み込み

	UpdateData();
}

/*! @brief Jump候補の更新

	@date 2005.03.31 MIK 
		ダイアログOpen時以外にも更新が必要なためSetData()より分離
*/
void CDlgTagJumpList::UpdateData( void )
{
	HWND	hwndList;
	LV_ITEM	lvi;
	char	tmp[32];
	int		nIndex;
	char	*p;
	int		count;

	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_TAGJUMP );
	ListView_DeleteAllItems( hwndList );

	count = m_cList.GetCount();

	for( nIndex = 0; nIndex < count; nIndex++ )
	{
		CSortedTagJumpList::TagJumpInfo* item;
		item = m_cList.GetPtr( nIndex );
		if( NULL == item ) break;

		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = nIndex;
		lvi.iSubItem = 0;
		lvi.pszText  = item->keyword;
		ListView_InsertItem( hwndList, &lvi );

		wsprintf( tmp, "%d", item->depth );
		ListView_SetItemText( hwndList, nIndex, 1, tmp );

		wsprintf( tmp, "%d", item->no );
		ListView_SetItemText( hwndList, nIndex, 2, tmp );

		p = GetNameByType( item->type, item->filename );
		ListView_SetItemText( hwndList, nIndex, 3, p );
		free( p );

		ListView_SetItemText( hwndList, nIndex, 4, item->filename );

		ListView_SetItemText( hwndList, nIndex, 5, item->note );

		ListView_SetItemState( hwndList, nIndex, 0, LVIS_SELECTED | LVIS_FOCUSED );
	}

	//	数が多すぎる場合は切り捨てた旨を末尾に挿入
	if( m_cList.IsOverflow() )
	{
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = nIndex;
		lvi.iSubItem = 0;
		lvi.pszText  = "(通知)";
		ListView_InsertItem( hwndList, &lvi );
		ListView_SetItemText( hwndList, nIndex, 1, "" );
		ListView_SetItemText( hwndList, nIndex, 2, "" );
		ListView_SetItemText( hwndList, nIndex, 3, "" );
		ListView_SetItemText( hwndList, nIndex, 4, "(これ以降は切り捨てました)" );
		ListView_SetItemText( hwndList, nIndex, 5, "" );
	}

	m_nIndex = SearchBestTag();
	if( m_nIndex != -1 )
	{
		ListView_SetItemState( hwndList, m_nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
		ListView_EnsureVisible( hwndList, m_nIndex, FALSE );
	}

	return;
}

/*!	ダイアログデータの取得

	@return TRUE: 正常, FALSE: 入力エラー

	@date 2005.04.03 MIK 設定値の保存処理追加
*/
int CDlgTagJumpList::GetData( void )
{
	HWND	hwndList;

	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_TAGJUMP );
	m_nIndex = ListView_GetNextItem( hwndList, -1, LVIS_SELECTED );
	if( m_nIndex == -1 || m_nIndex >= m_cList.GetCapacity() ) return FALSE;

	//	From Here 2005.04.03 MIK 設定値の保存
	if( 0 != m_lParam )
	{
		m_pShareData->m_bTagJumpICase = m_bTagJumpICase;
		m_pShareData->m_bTagJumpAnyWhere = m_bTagJumpAnyWhere;

		char	tmp[MAX_TAG_STRING_LENGTH];
		strcpy( tmp, "" );
		::GetDlgItemText( m_hWnd, IDC_KEYWORD, tmp, sizeof( tmp ) / sizeof( char ) );
		SetKeyword( tmp );

		//設定を保存
		CRecent cRecentTagJumpKeyword;
		cRecentTagJumpKeyword.EasyCreate( RECENT_FOR_TAGJUMP_KEYWORD );
		cRecentTagJumpKeyword.AppendItem( m_pszKeyword );
		cRecentTagJumpKeyword.Terminate();
	}
	//	To Here 2005.04.03 MIK

	return TRUE;
}

/*!
	@date 2005.03.31 MIK
		階層カラムの追加．キーワード指定欄の追加
*/
BOOL CDlgTagJumpList::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	HWND		hwndList;
	LV_COLUMN	col;
	RECT		rc;
	long		lngStyle;
	BOOL		bRet;

	m_hWnd = hwndDlg;

	//リストビューの表示位置を取得する。
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_TAGJUMP );
	//ListView_DeleteAllItems( hwndList );
	rc.left = rc.top = rc.right = rc.bottom = 0;
	::GetWindowRect( hwndList, &rc );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 19 / 100;
	col.pszText  = "キーワード";
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_CENTER;
	col.cx       = (rc.right - rc.left) * 6 / 100;
	col.pszText  = "階層";
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_RIGHT;
	col.cx       = (rc.right - rc.left) * 8 / 100;
	col.pszText  = "行番号";
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 9 / 100;
	col.pszText  = "種類";
	col.iSubItem = 3;
	ListView_InsertColumn( hwndList, 3, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 35 / 100;
	col.pszText  = "ファイル名";
	col.iSubItem = 4;
	ListView_InsertColumn( hwndList, 4, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 20 / 100;
	col.pszText  = "備考";
	col.iSubItem = 5;
	ListView_InsertColumn( hwndList, 5, &col );

	/* 行選択 */
	lngStyle = ::SendMessage( hwndList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
	lngStyle |= LVS_EX_FULLROWSELECT;
	::SendMessage( hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lngStyle );

	HWND hwndKey;
	hwndKey = ::GetDlgItem( m_hWnd, IDC_KEYWORD );
	if( m_lParam == 0 ){
		//標準
		::ShowWindow( ::GetDlgItem( m_hWnd, IDC_STATIC_KEYWORD ), FALSE );
		::ShowWindow( hwndKey, FALSE );
		::ShowWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_ICASE ), FALSE );
		::ShowWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_ANYWHERE ), FALSE );
		bRet = TRUE;
	}else{
		//キーワード指定
		::ShowWindow( ::GetDlgItem( m_hWnd, IDC_STATIC_KEYWORD ), TRUE );
		::ShowWindow( hwndKey, TRUE );
		::ShowWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_ICASE ), TRUE );
		::ShowWindow( ::GetDlgItem( m_hWnd, IDC_CHECK_ANYWHERE ), TRUE );
		::SetFocus( hwndKey );
		bRet = FALSE;	//for set focus
	}

	/* 基底クラスメンバ */
	CDialog::OnInitDialog( m_hWnd, wParam, lParam );
	
	return bRet;
}

BOOL CDlgTagJumpList::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* ヘルプ */
		MyWinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID( F_TAGJUMP_LIST ) );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	case IDOK:			/* 左右に表示 */
		StopTimer();
		/* ダイアログデータの取得 */
		::EndDialog( m_hWnd, (BOOL)GetData() );
		return TRUE;

	case IDCANCEL:
		StopTimer();
		::EndDialog( m_hWnd, FALSE );
		return TRUE;

	// From Here 2005.04.03 MIK 検索条件設定
	case IDC_CHECK_ICASE:
		StartTimer();
		m_bTagJumpICase = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_ICASE ) == BST_CHECKED ? TRUE : FALSE;
		return TRUE;

	case IDC_CHECK_ANYWHERE:
		StartTimer();
		m_bTagJumpAnyWhere = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_ANYWHERE ) == BST_CHECKED ? TRUE : FALSE;
		return TRUE;
	// To Here 2005.04.03 MIK 検索条件設定
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgTagJumpList::OnNotify( WPARAM wParam, LPARAM lParam )
{
	NMHDR*	pNMHDR;
	HWND	hwndList;

	pNMHDR = (NMHDR*)lParam;

	hwndList = GetDlgItem( m_hWnd, IDC_LIST_TAGJUMP );

	//	候補一覧リストボックス
	if( hwndList == pNMHDR->hwndFrom )
	{
		switch( pNMHDR->code )
		{
		case NM_DBLCLK:
			m_nIndex = ListView_GetNextItem( hwndList, -1, LVIS_SELECTED );
			if( m_nIndex == -1 || m_nIndex >= m_cList.GetCount() ) return TRUE;

			StopTimer();
			::EndDialog( m_hWnd, GetData() );
			return TRUE;

		}
	}

	/* 基底クラスメンバ */
	return CDialog::OnNotify( wParam, lParam );
}

/*!
	タイマー経過

	タイマーを停止し，候補リストを更新する
*/
BOOL CDlgTagJumpList::OnTimer( WPARAM wParam )
{
	StopTimer();

	char	szKey[ MAX_TAG_STRING_LENGTH ];
	strcpy( szKey, "" );
	::GetDlgItemText( m_hWnd, IDC_KEYWORD, szKey, sizeof( szKey ) );
	find_key( szKey );
	UpdateData();

	return TRUE;
}

/*!
	タイマー経過

	タイマーを停止し，候補リストを更新する
*/
BOOL CDlgTagJumpList::OnCbnEditChange( HWND hwndCtl, int wID )
{
	StartTimer();

	/* 基底クラスメンバ */
	return CDialog::OnCbnEditChange( hwndCtl, wID );
}

BOOL CDlgTagJumpList::OnCbnSelChange( HWND hwndCtl, int wID )
{
	StartTimer();

	/* 基底クラスメンバ */
	return CDialog::OnCbnSelChange( hwndCtl, wID );
}

#if 0
BOOL CDlgTagJumpList::OnEditChange( HWND hwndCtl, int wID )
{
	StartTimer();

	/* 基底クラスメンバ */
	return CDialog::OnEditChange( hwndCtl, wID );
}
#endif

LPVOID CDlgTagJumpList::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

bool CDlgTagJumpList::AddParam( char *s0, char *s1, int n2, char *s3, char *s4, int depth )
{
	if( -1 == m_nIndex ) m_nIndex = 0;	//規定値

	m_cList.AddParam( s0, s1, n2, s3[0], s4, depth );

	return true;
}

bool CDlgTagJumpList::GetSelectedParam( char *s0, char *s1, int *n2, char *s3, char *s4, int *depth )
{
	if( -1 == m_nIndex || m_nIndex >= m_cList.GetCount() ) return false;

	m_cList.GetParam( m_nIndex, s0, s1, n2, &s3[0], s4, depth );

	return true;
}

char *CDlgTagJumpList::GetNameByType( const char type, const char *name )
{
	const char	*p;
	char	*token;
	int		i;
	//	2005.03.31 MIK
	char	tmp[MAX_TAG_STRING_LENGTH];

	p = strrchr( name, '.' );
	if( ! p ) p = ".c";	//見つからないときは ".c" と想定する。
	p++;

	for( i = 0; p_extentions[i]; i += 2 )
	{
		strcpy( tmp, p_extentions[i] );
		token = strtok( tmp, "," );
		while( token )
		{
			if( stricmp( p, token ) == 0 )
			{
				strcpy( tmp, p_extentions[i+1] );
				token = strtok( tmp, "," );
				while( token )
				{
					if( token[0] == type )
					{
						return strdup( &token[2] );
					}

					token = strtok( NULL, "," );
				}

				return strdup( "" );
			}

			token = strtok( NULL, "," );
		}
	}

	return strdup( "" );
}

/*!
	与えられたファイル名と，そこに含まれるディレクトリの深さを設定する．
*/
void CDlgTagJumpList::SetFileName( const char *pszFileName )
{
	if( NULL == pszFileName ) return;

	if( m_pszFileName ) free( m_pszFileName );

	m_pszFileName = strdup( pszFileName );

	m_nLoop = CalcDirectoryDepth( m_pszFileName );
	if( m_nLoop <  0 ) m_nLoop =  0;
	if( m_nLoop > (_MAX_PATH/2) ) m_nLoop = (_MAX_PATH/2);	//\A\B\C...のようなとき1フォルダで2文字消費するので...

	return;
}

/*!
	検索キーワードの設定

*/
void CDlgTagJumpList::SetKeyword( const char *pszKeyword )
{
	if( NULL == pszKeyword ) return;

	if( m_pszKeyword ) free( m_pszKeyword );

	m_pszKeyword = strdup( pszKeyword );

	return;
}

/*!
	得られた候補から最も期待に近いと思われるものを
	選び出す．(初期選択位置決定のため)

	@return 選択されたアイテムのindex

*/
int CDlgTagJumpList::SearchBestTag( void )
{
	if( m_cList.GetCount() <= 0 ) return -1;	//選べません。
	if( NULL == m_pszFileName ) return 0;

	char	szFileSrc[1024];
	char	szFileDst[1024];
	char	szExtSrc[1024];
	char	szExtDst[1024];
	int		nMatch = -1;
	int		i;
	int		count;

	strcpy( szFileSrc, "" );
	strcpy( szExtSrc,  "" );
	_splitpath( m_pszFileName, NULL, NULL, szFileSrc, szExtSrc );

	count = m_cList.GetCount();

	for( i = 0; i < count; i++ )
	{
		CSortedTagJumpList::TagJumpInfo* item;
		item = m_cList.GetPtr( i );

		strcpy( szFileDst, "" );
		strcpy( szExtDst,  "" );
		_splitpath( item->filename, NULL, NULL, szFileDst, szExtDst );
		
		if( stricmp( szFileSrc, szFileDst ) == 0 )
		{
			if( stricmp( szExtSrc, szExtDst ) == 0 ) return i;
			if( nMatch == -1 ) nMatch = i;
		}
	}

	if( nMatch != -1 ) return nMatch;

	return 0;
}

/*!
	タグファイルからキーワードにマッチするデータを抽出し，
	m_cListに設定する
	
*/
void CDlgTagJumpList::find_key( const char* keyword )
{
	char	szCurrentPath[1024];	//カレントフォルダ
	char	szTagFile[1024];		//タグファイル
	char	szLineData[MAX_TAG_STRING_LENGTH*4];		//行バッファ
	char	s[5][MAX_TAG_STRING_LENGTH];
	int		n2;
	int	length = strlen( keyword );
	int	nMatch;
	int	i;
	FILE*	fp;
	int	nRet;
	int cmp;
	
	Empty();

	if( length == 0 ) return;

	strcpy( szCurrentPath, GetFilePath() );
	szCurrentPath[ strlen( szCurrentPath ) - strlen( GetFileName() ) ] = '\0';

	for( i = 0; i <= m_nLoop; i++ )
	{
		//タグファイル名を作成する。
		wsprintf( szTagFile, "%s%s", szCurrentPath, TAG_FILENAME );

		//タグファイルを開く。
		fp = fopen( szTagFile, "r" );
		if( fp )
		{
			nMatch = 0;
			while( fgets( szLineData, sizeof( szLineData ), fp ) )
			{
				if( szLineData[0] <= '!' ) goto next_line;	//コメントならスキップ
				//chop( szLineData );

				s[0][0] = s[1][0] = s[2][0] = s[3][0] = s[4][0] = '\0';
				n2 = 0;
				nRet = sscanf( szLineData, 
					TAG_FORMAT,	//tagsフォーマット
					s[0], s[1], &n2, s[3], s[4]
					);
				if( nRet < 4 ) goto next_line;
				if( n2 <= 0 ) goto next_line;	//行番号不正(-excmd=nが指定されてないかも)

				if( m_bTagJumpAnyWhere ){
					if( m_bTagJumpICase )
						cmp = strstri( s[0], keyword ) != NULL ? 0 : -1;
					else
						cmp = strstr( s[0], keyword ) != NULL ? 0 : -1;
				}else{
					if( m_bTagJumpICase )
						cmp = strnicmp( s[0], keyword, length );
					else
						cmp = strncmp( s[0], keyword, length );
				}

				if( 0 == cmp )
				{
					m_cList.AddParam( s[0], s[1], n2, s[3][0], s[4], i );
					nMatch++;
				}else if( 0 < cmp ){
					//	tagsはソートされているので，先頭からのcase sensitiveな
					//	比較結果によって検索の時は処理の打ち切りが可能
					//	2005.04.05 MIK バグ修正
					if( (!m_bTagJumpICase) && (!m_bTagJumpAnyWhere) ) break;
				}
next_line:
				//if( nMatch >= 10 ) break;
				;
			}

			//ファイルを閉じる。
			fclose( fp );

		}
		
		strcat( szCurrentPath, "..\\" );
	}
}

/*!
	パスからファイル名部分のみを取り出す．(2バイト対応)
*/
const char * CDlgTagJumpList::GetFileName( void )
{
	const char *p, *pszName;
	pszName = p = GetFilePath();
	while( *p != '\0'  ){
		if( _IS_SJIS_1( (unsigned char)*p ) && _IS_SJIS_2( (unsigned char)p[1] ) ){
			p+=2;
		}else if( *p == '\\' ){
			pszName = p + 1;
			p++;
		}else{
			p++;
		}
	}
	return pszName;
}

/*[EOF]*/
