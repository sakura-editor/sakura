/*!	@file
	@brief �^�O�W�����v���X�g�_�C�A���O�{�b�N�X

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 2003, MIK

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



#include <stdio.h>
#include "sakura_rc.h"
#include "global.h"
#include "funccode.h"
#include "mymessage.h"
#include "CDialog.h"
#include "CDlgTagJumpList.h"
#include "etc_uty.h"
#include "debug.h"
#include "my_icmp.h"

#include "sakura.hh"
const DWORD p_helpids[] = {
	IDC_LIST_TAGJUMP,		HIDC_LIST_TAGJUMPLIST,			//�t�@�C��
	IDOK,					HIDC_TAGJUMPLIST_IDOK,			//OK
	IDCANCEL,				HIDC_TAGJUMPLIST_IDCANCEL,		//�L�����Z��
	IDC_BUTTON_HELP,		HIDC_BUTTON_TAGJUMPLIST_HELP,	//�w���v
//	IDC_STATIC,				-1,
	0, 0
};

/*
	ctags.exe ���o�͂���A�g���q�ƑΉ�������
*/
static const char *p_extentions[] = {
#if 1
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
#else
	/*asm*/			"asm,s",								"d=��`,l=���x��,m=�}�N��,t=�^",
	/*asp*/			"asp,asa",								"f=�֐�,s=sub",
	/*awk*/			"awk,gawk,mawk",						"f=�֐�",
	/*beta*/		"bet",									"f=fragment,p=�p�^�[��,s=�X���b�g,v=���z",
	/*c*/			"c,h",									"c=�N���X,d=�}�N��,e=�񋓒萔,f=�֐�,g=�񋓌^,m=�����o,n=���O���,p=�v���g�^�C�v,s=�\����,t=�^��`,u=���p��,v=�ϐ�,x=�O���ϐ�",
	/*c++*/			"c++,cc,cp,cpp,cxx,h++,hh,hp,hpp,hxx",	"c=�N���X,d=�}�N��,e=�񋓒萔,f=�֐�,g=�񋓌^,m=�����o,n=���O���,p=�v���g�^�C�v,s=�\����,t=�^��`,u=���p��,v=�ϐ�,x=�O���ϐ�",
	/*java*/		"java",									"c=�N���X,d=�}�N��,e=�񋓒萔,f=�֐�,g=�񋓌^,m=�����o,n=���O���,p=�v���g�^�C�v,s=�\����,t=�^��`,u=���p��,v=�ϐ�,x=�O���ϐ�",
	/*vera*/		"vr,vri,vrh",							"c=�N���X,d=�}�N��,e=�񋓒萔,f=�֐�,g=�񋓌^,m=�����o,n=���O���,p=�v���g�^�C�v,s=�\����,t=�^��`,u=���p��,v=�ϐ�,x=�O���ϐ�",
	/*cobol*/		"cbl,cob",								"d=�f�[�^,f=�t�@�C��,g=�O���[�v,p=�p���O���t,P=�v���O����,s=�Z�N�V����",
	/*eiffel*/		"e",									"c=�N���X,f=feature,l=���[�J��",
	/*fortran*/		"f,for,ftn,f77,f90,f95",				"b=�u���b�N�f�[�^,c=����,e=�����,f=�֐�,i=�C���^�t�F�[�X,k=�R���|�[�l���g,l=���x��,L=���[�J��,m=���W���[��,n=���O�ꗗ,p=�v���O����,s=�T�u���[�`��,t=�^,v=�ϐ�",
	/*lisp*/		"cl,clisp,el,l,lisp,lsp,ml",			"f=�֐�",
	/*lua*/			"lua",									"f=�֐�",
	/*makefile*/	"mak",									"m=�}�N��",
	/*pascal*/		"p,pas",								"f=�֐�,p=�葱��",
	/*perl*/		"pl,pm,perl",							"s=�T�u���[�`��,p=�p�b�P�[�W",
	/*php*/			"php,php3,phtml",						"c=�N���X,f=�֐�",
	/*python*/		"py,python",							"c=�N���X,f=�֐�,m=�����o",
	/*rexx*/		"cmd,rexx,rx",							"s=�T�u���[�`��",
	/*ruby*/		"rb",									"c=�N���X,f=���\�b�h,F=singleton ���\�b�h,m=mixin",
	/*scheme*/		"sch,scheme,scm,sm",					"f=�֐�,s=�Z�b�g",
	/*sh*/			"sh,bsh,bash,ksh,zsh",					"f=�֐�",
	/*slang*/		"sl",									"f=�֐�,n=���O���",
	/*sql*/			"sql",									"c=�J�[�\��,d=�v���g�^�C�v,f=�֐�,F=field,l=���[�J��,P=�p�b�P�[�W,p=�葱��,r=���R�[�h,s=subtype,t=�e�[�u��,T=�g���K,v=�ϐ�",
	/*tcl*/			"tcl,tk,wish,itcl",						"p=�葱��,c=�N���X,f=���\�b�h",
	/*verilog*/		"v",									"f=�֐�,m=���W���[��,P=����,p=port,r=reg,t=�^�X�N,v=�ϐ�,w=wire",
	/*vim*/			"vim",									"f=�֐�,v=�ϐ�",
	/*yacc*/		"y",									"l=���x��",
//	/*vb*/			"bas,cls,ctl,dob,dsr,frm,pag",			"a=����,c=�N���X,f=�֐�,l=���x��,s=�葱��,v=�ϐ�",
#endif
					NULL,									NULL
};



CDlgTagJumpList::CDlgTagJumpList()
{
	m_nCount = 0;
	memset( m_uParam, 0, sizeof( m_uParam ) );
	m_nIndex = -1;
	m_bOverflow = false;
	m_pszFileName = NULL;
}

CDlgTagJumpList::~CDlgTagJumpList()
{
	int	i;

	for( i = 0; i < m_nCount; i++ )
	{
		free( m_uParam[i].s0 );
		free( m_uParam[i].s1 );
		free( m_uParam[i].s3 );
		free( m_uParam[i].s4 );
	}

	m_nCount = 0;
	m_nIndex = -1;
	m_bOverflow = false;

	if( m_pszFileName ) free( m_pszFileName );
	m_pszFileName = NULL;
}

/* ���[�_���_�C�A���O�̕\�� */
CDlgTagJumpList::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
)
{
	return CDialog::DoModal( hInstance, hwndParent, IDD_TAGJUMPLIST, lParam );
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgTagJumpList::SetData( void )
{
	HWND	hwndList;
	LV_ITEM	lvi;
	char	tmp[1024];
	int		nIndex;
	char	*p;

	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_TAGJUMP );
	ListView_DeleteAllItems( hwndList );

	for( nIndex = 0; nIndex < m_nCount; nIndex++ )
	{
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = nIndex;
		lvi.iSubItem = 0;
		lvi.pszText  = m_uParam[nIndex].s0;
		ListView_InsertItem( hwndList, &lvi );

		wsprintf( tmp, "%d", m_uParam[nIndex].n2 );
		ListView_SetItemText( hwndList, nIndex, 1, tmp );

		p = GetNameByType( m_uParam[nIndex].s3[0], m_uParam[nIndex].s1 );
		strcpy( tmp, p );
		free( p );
		ListView_SetItemText( hwndList, nIndex, 2, tmp );

		ListView_SetItemText( hwndList, nIndex, 3, m_uParam[nIndex].s1 );

		ListView_SetItemText( hwndList, nIndex, 4, m_uParam[nIndex].s4 );

		ListView_SetItemState( hwndList, nIndex, 0, LVIS_SELECTED | LVIS_FOCUSED );

//		if( nIndex == 0 )
//		{
//			ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
//		}
	}

	nIndex = SearchBestTag();
	if( nIndex != -1 )
	{
		ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
		ListView_EnsureVisible( hwndList, nIndex, FALSE );
	}

	if( m_bOverflow )
	{
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = nIndex;
		lvi.iSubItem = 0;
		lvi.pszText  = "";
		ListView_InsertItem( hwndList, &lvi );
		ListView_SetItemText( hwndList, nIndex, 1, "" );
		ListView_SetItemText( hwndList, nIndex, 2, "" );
		ListView_SetItemText( hwndList, nIndex, 3, "(����ȍ~�͐؂�̂Ă܂���)" );
		ListView_SetItemText( hwndList, nIndex, 4, "" );
	}

	return;
}

/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[ */
int CDlgTagJumpList::GetData( void )
{
	HWND	hwndList;

	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_TAGJUMP );
	m_nIndex = ListView_GetNextItem( hwndList, -1, LVIS_SELECTED );
	if( m_nIndex == -1 || m_nIndex >= MAX_TAGJUMPLIST ) return FALSE;

	return TRUE;
}

BOOL CDlgTagJumpList::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	HWND		hwndList;
	LV_COLUMN	col;
	RECT		rc;
	long		lngStyle;

	m_hWnd = hwndDlg;

	//���X�g�r���[�̕\���ʒu���擾����B
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_TAGJUMP );
	rc.left = rc.top = rc.right = rc.bottom = 0;
	::GetWindowRect( hwndList, &rc );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 16 / 100;
	col.pszText  = "�L�[���[�h";
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_RIGHT;
	col.cx       = (rc.right - rc.left) * 8 / 100;
	col.pszText  = "�s�ԍ�";
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 12 / 100;
	col.pszText  = "���";
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 41 / 100;
	col.pszText  = "�t�@�C����";
	col.iSubItem = 3;
	ListView_InsertColumn( hwndList, 3, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 20 / 100;
	col.pszText  = "���l";
	col.iSubItem = 4;
	ListView_InsertColumn( hwndList, 4, &col );

	/* �s�I�� */
	lngStyle = ::SendMessage( hwndList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
	lngStyle |= LVS_EX_FULLROWSELECT;
	::SendMessage( hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lngStyle );

	/* ���N���X�����o */
	return CDialog::OnInitDialog( m_hWnd, wParam, lParam );
}

BOOL CDlgTagJumpList::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* �w���v */
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID( F_TAGJUMP_LIST ) );
		return TRUE;

	case IDOK:			/* ���E�ɕ\�� */
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( m_hWnd, (BOOL)GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
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

	hwndList = GetDlgItem( m_hWnd, IDC_LIST_TAGJUMP );

	if( hwndList == pNMHDR->hwndFrom )
	{
		switch( pNMHDR->code )
		{
		case NM_DBLCLK:
			m_nIndex = ListView_GetNextItem( hwndList, -1, LVIS_SELECTED );
			if( m_nIndex == -1 || m_nIndex >= MAX_TAGJUMPLIST ) return TRUE;

			::EndDialog( m_hWnd, GetData() );
			return TRUE;

		}
	}

	/* ���N���X�����o */
	return CDialog::OnNotify( wParam, lParam );
}

LPVOID CDlgTagJumpList::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

bool CDlgTagJumpList::AddParam( char *s0, char *s1, int n2, char *s3, char *s4 )
{
	if( m_nCount >= MAX_TAGJUMPLIST ) 
	{
		m_bOverflow = true;
		return false;
	}

	//if( n2 <= 0 ) return false;

	if( -1 == m_nIndex ) m_nIndex = 0;	//�K��l

	m_uParam[m_nCount].s0 = strdup( s0 );
	m_uParam[m_nCount].s1 = strdup( s1 );
	m_uParam[m_nCount].n2 = n2;
	m_uParam[m_nCount].s3 = strdup( s3 );
	m_uParam[m_nCount].s4 = strdup( s4 );

	m_nCount++;

	return true;
}

bool CDlgTagJumpList::GetSelectedParam( char *s0, char *s1, int *n2, char *s3, char *s4 )
{
	if( -1 == m_nIndex ) return false;
	if( m_nIndex >= MAX_TAGJUMPLIST ) return false;

	strcpy( s0, m_uParam[m_nIndex].s0 );
	strcpy( s1, m_uParam[m_nIndex].s1 );
	*n2 = m_uParam[m_nIndex].n2;
	strcpy( s3, m_uParam[m_nIndex].s3 );
	strcpy( s4, m_uParam[m_nIndex].s4 );

	return true;
}

char *CDlgTagJumpList::GetNameByType( const char type, const char *name )
{
	const char	*p;
	char	*token;
	int		i;
	char	tmp[1024];

	p = strrchr( name, '.' );
	if( ! p ) p = ".c";	//������Ȃ��Ƃ��� ".c" �Ƒz�肷��B
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

void CDlgTagJumpList::SetFileName( const char *pszFileName )
{
	if( NULL == pszFileName ) return;

	if( m_pszFileName ) free( m_pszFileName );

	m_pszFileName = strdup( pszFileName );

	return;
}

int CDlgTagJumpList::SearchBestTag( void )
{
	if( m_nCount <= 0 ) return -1;	//�I�ׂ܂���B
	if( NULL == m_pszFileName ) return 0;

	char	szFileSrc[1024];
	char	szFileDst[1024];
	char	szExtSrc[1024];
	char	szExtDst[1024];
	int		nMatch = -1;
	int		i;

	strcpy( szFileSrc, "" );
	strcpy( szExtSrc,  "" );
	_splitpath( m_pszFileName, NULL, NULL, szFileSrc, szExtSrc );

	for( i = 0; i < m_nCount; i++ )
	{
		strcpy( szFileDst, "" );
		strcpy( szExtDst,  "" );
		_splitpath( m_uParam[i].s1, NULL, NULL, szFileDst, szExtDst );
		
		if( stricmp( szFileSrc, szFileDst ) == 0 )
		{
			if( stricmp( szExtSrc, szExtDst ) == 0 ) return i;
			if( nMatch == -1 ) nMatch = i;
		}
	}

	if( nMatch != -1 ) return nMatch;

	return 0;
}

/*[EOF]*/
