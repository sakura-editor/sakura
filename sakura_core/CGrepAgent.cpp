#include "StdAfx.h"
#include "CGrepAgent.h"
#include "CGrepEnumKeys.h"
#include "CGrepEnumFilterFiles.h"
#include "CGrepEnumFilterFolders.h"
#include "CSearchAgent.h"
#include "dlg/CDlgCancel.h"
#include "_main/CAppMode.h"
#include "COpeBlk.h"
#include "window/CEditWnd.h"
#include "charset/CCodeMediator.h"
#include "view/colors/CColorStrategy.h"
#include "charset/CCodeBase.h"
#include "io/CFileLoad.h"
#include "util/window.h"
#include "util/module.h"
#include "util/other_util.h"
#include "debug/CRunningTimer.h"
#include "sakura_rc.h"

CGrepAgent::CGrepAgent()
: m_bGrepMode( false )			/* Grep���[�h�� */
, m_bGrepRunning( false )		/* Grep������ */
{
}

ECallbackResult CGrepAgent::OnBeforeClose()
{
	//GREP�������͏I���ł��Ȃ�
	if( m_bGrepRunning ){
		// �A�N�e�B�u�ɂ���
		ActivateFrameWindow( CEditWnd::getInstance()->GetHwnd() );	//@@@ 2003.06.25 MIK
		TopInfoMessage(
			CEditWnd::getInstance()->GetHwnd(),
			LS(STR_GREP_RUNNINNG)
		);
		return CALLBACK_INTERRUPT;
	}
	return CALLBACK_CONTINUE;
}

void CGrepAgent::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	// ���O��t���ĕۑ�����ă��[�h���������ꂽ���̕s��������ǉ��iANSI�łƂ̍��فj	// 2009.08.12 ryoji
	m_bGrepMode = false;	// grep�E�B���h�E�͒ʏ�E�B���h�E��
	CAppMode::getInstance()->m_szGrepKey[0] = L'\0';
}

/*!
	@date 2014.03.09 novice �Ō��\\����菜���̂���߂�(d:\\ -> d:�ɂȂ�)
*/
void CGrepAgent::CreateFolders( const TCHAR* pszPath, std::vector<std::tstring>& vPaths )
{
	const int nPathLen = auto_strlen( pszPath );
	auto_array_ptr<TCHAR> szPath(new TCHAR[nPathLen + 1]);
	auto_array_ptr<TCHAR> szTmp(new TCHAR[nPathLen + 1]);
	auto_strcpy( &szPath[0], pszPath );
	TCHAR* token;
	int nPathPos = 0;
	while( NULL != (token = my_strtok<TCHAR>( &szPath[0], nPathLen, &nPathPos, _T(";"))) ){
		auto_strcpy( &szTmp[0], token );
		TCHAR* p;
		TCHAR* q;
		p = q = &szTmp[0];
		while( *p ){
			if( *p != _T('"') ){
				if( p != q ){
					*q = *p;
				}
				q++;
			}
			p++;
		}
		*q = _T('\0');
#if 0
		// 2011.12.25 �d�l�ύX�B�Ō��\\�͎�菜��
		int	nFolderLen = q - &szTmp[0];
		if( 0 < nFolderLen ){
			int nCharChars = &szTmp[nFolderLen] - CNativeT::GetCharPrev( &szTmp[0], nFolderLen, &szTmp[nFolderLen] );
			if( 1 == nCharChars && (_T('\\') == szTmp[nFolderLen - 1] || _T('/') == szTmp[nFolderLen - 1]) ){
				szTmp[nFolderLen - 1] = _T('\0');
			}
		}
#endif
		/* �����O�t�@�C�������擾���� */
		TCHAR szTmp2[_MAX_PATH];
		if( ::GetLongFileName( &szTmp[0], szTmp2 ) ){
			vPaths.push_back( szTmp2 );
		}else{
			vPaths.push_back( &szTmp[0] );
		}
	}
}

/*! �Ō��\\����菜��
	@date 2014.03.09 novice �V�K�쐬
*/
std::tstring CGrepAgent::ChopYen( const std::tstring& str )
{
	std::tstring dst = str;
	size_t nPathLen = dst.length();

	// �Ō�̃t�H���_��؂�L�����폜����
	// [A:\]�Ȃǂ̃��[�g�ł����Ă��폜
	for(size_t i = 0; i < nPathLen; i++ ){
#ifdef _MBCS
		if( _IS_SJIS_1( (unsigned char)dst[i] ) && (i + 1 < nPathLen) && _IS_SJIS_2( (unsigned char)dst[i + 1] ) ){
			// SJIS�ǂݔ�΂�
			i++;
		} else
#endif
		if( _T('\\') == dst[i] && i == nPathLen - 1 ){
			dst.resize( nPathLen - 1 );
			break;
		}
	}

	return dst;
}

/*! Grep���s

  @param[in] pcmGrepKey �����p�^�[��
  @param[in] pcmGrepFile �����Ώۃt�@�C���p�^�[��(!�ŏ��O�w��))
  @param[in] pcmGrepFolder �����Ώۃt�H���_

  @date 2008.12.07 nasukoji	�t�@�C�����p�^�[���̃o�b�t�@�I�[�o�����΍�
  @date 2008.12.13 genta �����p�^�[���̃o�b�t�@�I�[�o�����΍�
  @date 2012.10.13 novice �����I�v�V�������N���X���Ƒ��
*/
DWORD CGrepAgent::DoGrep(
	CEditView*				pcViewDst,
	const CNativeW*			pcmGrepKey,
	const CNativeT*			pcmGrepFile,
	const CNativeT*			pcmGrepFolder,
	bool					bGrepCurFolder,
	BOOL					bGrepSubFolder,
	const SSearchOption&	sSearchOption,
	ECodeType				nGrepCharSet,	// 2002/09/21 Moca �����R�[�h�Z�b�g�I��
	BOOL					bGrepOutputLine,
	int						nGrepOutputStyle,
	bool					bGrepOutputFileOnly,
	bool					bGrepOutputBaseFolder,
	bool					bGrepSeparateFolder
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CEditView::DoGrep" );
#endif

	// �ē��s��
	if( this->m_bGrepRunning ){
		assert_warning( false == this->m_bGrepRunning );
		return 0xffffffff;
	}

	this->m_bGrepRunning = true;

	int			nHitCount = 0;
	CDlgCancel	cDlgCancel;
	HWND		hwndCancel;
	//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
	CBregexp	cRegexp;
	CNativeW	cmemMessage;
	int			nWork;
	SGrepOption	sGrepOption;

	/*
	|| �o�b�t�@�T�C�Y�̒���
	*/
	cmemMessage.AllocStringBuffer( 4000 );

	pcViewDst->m_bDoing_UndoRedo		= true;


	/* �A���h�D�o�b�t�@�̏��� */
	if( NULL != pcViewDst->GetDocument()->m_cDocEditor.m_pcOpeBlk ){	/* ����u���b�N */
//@@@2002.2.2 YAZAKI NULL����Ȃ��Ɛi�܂Ȃ��̂ŁA�Ƃ肠�����R�����g�B��NULL�̂Ƃ��́Anew COpeBlk����B
//		while( NULL != m_pcOpeBlk ){}
//		delete m_pcOpeBlk;
//		m_pcOpeBlk = NULL;
	}
	else {
		pcViewDst->GetDocument()->m_cDocEditor.m_pcOpeBlk = new COpeBlk;
	}
	pcViewDst->GetDocument()->m_cDocEditor.m_pcOpeBlk->AddRef();

	pcViewDst->m_bCurSrchKeyMark = true;								/* ����������̃}�[�N */
	pcViewDst->m_strCurSearchKey = pcmGrepKey->GetStringPtr();				/* ���������� */
	pcViewDst->m_sCurSearchOption = sSearchOption;						// �����I�v�V����
	pcViewDst->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;

	/* ���K�\�� */

	//	From Here Jun. 27 genta
	/*
		Grep���s���ɓ������Č����E��ʐF�����p���K�\���o�b�t�@��
		����������D�����Grep�������ʂ̐F�������s�����߁D

		Note: �����ŋ�������͍̂Ō�̌���������ł�����
		Grep�Ώۃp�^�[���ł͂Ȃ����Ƃɒ���
	*/
	if( !pcViewDst->m_sSearchPattern.SetPattern(pcViewDst->GetHwnd(), pcViewDst->m_strCurSearchKey.c_str(), pcViewDst->m_strCurSearchKey.size(),
			pcViewDst->m_sCurSearchOption, &pcViewDst->m_CurRegexp) ){
		this->m_bGrepRunning = false;
		pcViewDst->m_bDoing_UndoRedo = false;
		pcViewDst->SetUndoBuffer();
		return 0;
	}

	hwndCancel = cDlgCancel.DoModeless( G_AppInstance(), pcViewDst->m_hwndParent, IDD_GREPRUNNING );

	::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, 0, FALSE );
	::DlgItem_SetText( hwndCancel, IDC_STATIC_CURFILE, _T(" ") );	// 2002/09/09 Moca add
	::CheckDlgButton( hwndCancel, IDC_CHECK_REALTIMEVIEW, GetDllShareData().m_Common.m_sSearch.m_bGrepRealTimeView );	// 2003.06.23 Moca

	//	2008.12.13 genta �p�^�[������������ꍇ�͓o�^���Ȃ�
	//	(���K�\�����r���œr�؂��ƍ���̂�)
	//	2011.12.10 Moca �\���̍ۂ�...�ɐ؂�̂Ă���̂œo�^����悤��
	wcsncpy_s( CAppMode::getInstance()->m_szGrepKey, _countof(CAppMode::getInstance()->m_szGrepKey), pcmGrepKey->GetStringPtr(), _TRUNCATE );
	this->m_bGrepMode = true;

	//	2007.07.22 genta
	//	�o�[�W�����ԍ��擾�̂��߁C������O�̕��ֈړ�����
	CSearchStringPattern pattern;
	if( !pattern.SetPattern(pcViewDst->GetHwnd(), pcmGrepKey->GetStringPtr(), pcmGrepKey->GetStringLength(),
			sSearchOption, &cRegexp) ){
		this->m_bGrepRunning = false;
		pcViewDst->m_bDoing_UndoRedo = false;
		pcViewDst->SetUndoBuffer();
		return 0;
	}
	
	// Grep�I�v�V�����܂Ƃ�
	sGrepOption.bGrepSubFolder = FALSE != bGrepSubFolder;
	sGrepOption.nGrepCharSet = nGrepCharSet;
	sGrepOption.bGrepOutputLine = FALSE != bGrepOutputLine;
	sGrepOption.nGrepOutputStyle = nGrepOutputStyle;
	sGrepOption.bGrepOutputFileOnly = bGrepOutputFileOnly;
	sGrepOption.bGrepOutputBaseFolder = bGrepOutputBaseFolder;
	sGrepOption.bGrepSeparateFolder = bGrepSeparateFolder;

//2002.02.08 Grep�A�C�R�����傫���A�C�R���Ə������A�C�R����ʁX�ɂ���B
	HICON	hIconBig, hIconSmall;
	//	Dec, 2, 2002 genta �A�C�R���ǂݍ��ݕ��@�ύX
	hIconBig   = GetAppIcon( G_AppInstance(), ICON_DEFAULT_GREP, FN_GREP_ICON, false );
	hIconSmall = GetAppIcon( G_AppInstance(), ICON_DEFAULT_GREP, FN_GREP_ICON, true );

	//	Sep. 10, 2002 genta
	//	CEditWnd�ɐV�݂����֐����g���悤��
	CEditWnd*	pCEditWnd = CEditWnd::getInstance();	//	Sep. 10, 2002 genta
	pCEditWnd->SetWindowIcon( hIconSmall, ICON_SMALL );
	pCEditWnd->SetWindowIcon( hIconBig, ICON_BIG );

	CGrepEnumKeys cGrepEnumKeys;
	{
		int nErrorNo = cGrepEnumKeys.SetFileKeys( pcmGrepFile->GetStringPtr() );
		if( nErrorNo != 0 ){
			this->m_bGrepRunning = false;
			pcViewDst->m_bDoing_UndoRedo = false;
			pcViewDst->SetUndoBuffer();

			const TCHAR* pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS0);
			if( nErrorNo == 1 ){
				pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS1);
			}else if( nErrorNo == 2 ){
				pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS2);
			}
			ErrorMessage( pcViewDst->m_hwndParent, _T("%ts"), pszErrorMessage );
			return 0;
		}
	}


	std::vector<std::tstring> vPaths;
	CreateFolders( pcmGrepFolder->GetStringPtr(), vPaths );

	nWork = pcmGrepKey->GetStringLength(); // 2003.06.10 Moca ���炩���ߒ������v�Z���Ă���

	/* �Ō�Ƀe�L�X�g��ǉ� */
	CNativeW	cmemWork;
	cmemMessage.AppendString( LSW( STR_GREP_SEARCH_CONDITION ) );	//L"\r\n����������  "
	if( 0 < nWork ){
		CNativeW cmemWork2;
		cmemWork2.SetNativeData( *pcmGrepKey );
		const STypeConfig& type = pcViewDst->m_pcEditDoc->m_cDocType.GetDocumentAttribute();
		if( FALSE == type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp ){
			// 2011.11.28 �F�w�肪�����Ȃ�G�X�P�[�v���Ȃ�
		}else
		if( type.m_nStringType == STRING_LITERAL_CPP || type.m_nStringType == STRING_LITERAL_CSHARP
			|| type.m_nStringType == STRING_LITERAL_PYTHON ){	/* �������؂�L���G�X�P�[�v���@ */
			cmemWork2.Replace( L"\\", L"\\\\" );
			cmemWork2.Replace( L"\'", L"\\\'" );
			cmemWork2.Replace( L"\"", L"\\\"" );
		}else if( type.m_nStringType == STRING_LITERAL_PLSQL ){
			cmemWork2.Replace( L"\'", L"\'\'" );
			cmemWork2.Replace( L"\"", L"\"\"" );
		}
		cmemWork.AppendString( L"\"" );
		cmemWork.AppendNativeData( cmemWork2 );
		cmemWork.AppendString( L"\"\r\n" );
	}else{
		cmemWork.AppendString( LSW( STR_GREP_SEARCH_FILE ) );	//L"�u�t�@�C�������v\r\n"
	}
	cmemMessage += cmemWork;



	cmemMessage.AppendString( LSW( STR_GREP_SEARCH_TARGET ) );	//L"�����Ώ�   "
	if( pcViewDst->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemWork.SetStringT( pcmGrepFile->GetStringPtr() );
	cmemMessage += cmemWork;




	cmemMessage.AppendString( L"\r\n" );
	cmemMessage.AppendString( LSW( STR_GREP_SEARCH_FOLDER ) );	//L"�t�H���_   "
	{
		std::tstring grepFolder;
		for( int i = 0; i < (int)vPaths.size(); i++ ){
			if( i ){
				grepFolder += _T(';');
			}
			std::tstring sPath = ChopYen( vPaths[i] );
			if( auto_strchr( sPath.c_str(), _T(';') ) ){
				grepFolder += _T('"');
				grepFolder += sPath;
				grepFolder += _T('"');
			}else{
				grepFolder += sPath;
			}
		}
		cmemWork.SetStringT( grepFolder.c_str() );
	}
	if( pcViewDst->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemMessage += cmemWork;
	cmemMessage.AppendString( L"\r\n" );

	const wchar_t*	pszWork;
	if( sGrepOption.bGrepSubFolder ){
		pszWork = LSW( STR_GREP_SUBFOLDER_YES );	//L"    (�T�u�t�H���_������)\r\n"
	}else{
		pszWork = LSW( STR_GREP_SUBFOLDER_NO );	//L"    (�T�u�t�H���_���������Ȃ�)\r\n"
	}
	cmemMessage.AppendString( pszWork );

	if( 0 < nWork ){ // 2003.06.10 Moca �t�@�C�������̏ꍇ�͕\�����Ȃ� // 2004.09.26 �������C��
		if( sSearchOption.bWordOnly ){
		/* �P��P�ʂŒT�� */
			cmemMessage.AppendString( LSW( STR_GREP_COMPLETE_WORD ) );	//L"    (�P��P�ʂŒT��)\r\n"
		}

		if( sSearchOption.bLoHiCase ){
			pszWork = LSW( STR_GREP_CASE_SENSITIVE );	//L"    (�p�啶������������ʂ���)\r\n"
		}else{
			pszWork = LSW( STR_GREP_IGNORE_CASE );	//L"    (�p�啶������������ʂ��Ȃ�)\r\n"
		}
		cmemMessage.AppendString( pszWork );

		if( sSearchOption.bRegularExp ){
			//	2007.07.22 genta : ���K�\�����C�u�����̃o�[�W�������o�͂���
			cmemMessage.AppendString( LSW( STR_GREP_REGEX_DLL ) );	//L"    (���K�\��:"
			cmemMessage.AppendStringT( cRegexp.GetVersionT() );
			cmemMessage.AppendString( L")\r\n" );
		}
	}

	if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
		cmemMessage.AppendString( LSW( STR_GREP_CHARSET_AUTODETECT ) );	//L"    (�����R�[�h�Z�b�g�̎�������)\r\n"
	}else if(IsValidCodeType(sGrepOption.nGrepCharSet)){
		cmemMessage.AppendString( LSW( STR_GREP_CHARSET ) );	//L"    (�����R�[�h�Z�b�g�F"
		cmemMessage.AppendStringT( CCodeTypeName(sGrepOption.nGrepCharSet).Normal() );
		cmemMessage.AppendString( L")\r\n" );
	}

	if( 0 < nWork ){ // 2003.06.10 Moca �t�@�C�������̏ꍇ�͕\�����Ȃ� // 2004.09.26 �������C��
		if( sGrepOption.bGrepOutputLine ){
		/* �Y���s */
			pszWork = LSW( STR_GREP_SHOW_MATCH_LINE );	//L"    (��v�����s���o��)\r\n"
		}else{
			pszWork = LSW( STR_GREP_SHOW_MATCH_AREA );	//L"    (��v�����ӏ��̂ݏo��)\r\n"
		}
		cmemMessage.AppendString( pszWork );

		if( sGrepOption.bGrepOutputFileOnly ){
			pszWork = LSW( STR_GREP_SHOW_FIRST_MATCH );	//L"    (�t�@�C�����ŏ��̂݌���)\r\n"
			cmemMessage.AppendString( pszWork );
		}
	}


	cmemMessage.AppendString( L"\r\n\r\n" );
	pszWork = cmemMessage.GetStringPtr( &nWork );
//@@@ 2002.01.03 YAZAKI Grep����̓J�[�\����Grep���O�̈ʒu�ɓ�����
	CLayoutInt tmp_PosY_Layout = pcViewDst->m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( 0 < nWork ){
		pcViewDst->GetCommander().Command_ADDTAIL( pszWork, nWork );
	}
	cmemMessage.Clear(); // ��������Ȃ�
	pszWork = NULL;
	
	//	2007.07.22 genta �o�[�W�������擾���邽�߂ɁC
	//	���K�\���̏���������ֈړ�


	/* �\������ON/OFF */
	// 2003.06.23 Moca ���ʐݒ�ŕύX�ł���悤��
	// 2008.06.08 ryoji �S�r���[�̕\��ON/OFF�𓯊�������
//	SetDrawSwitch(false);
	if( !CEditWnd::getInstance()->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V
		CEditWnd::getInstance()->RedrawAllViews( pcViewDst );	//	���̃y�C���̕\�����X�V
	const bool bDrawSwitchOld = pcViewDst->SetDrawSwitch(0 != GetDllShareData().m_Common.m_sSearch.m_bGrepRealTimeView);

	CGrepEnumFiles cGrepExceptAbsFiles;
	cGrepExceptAbsFiles.Enumerates(_T(""), cGrepEnumKeys.m_vecExceptAbsFileKeys);
	CGrepEnumFolders cGrepExceptAbsFolders;
	cGrepExceptAbsFolders.Enumerates(_T(""), cGrepEnumKeys.m_vecExceptAbsFolderKeys);

	int nGrepTreeResult = 0;

	for( int nPath = 0; nPath < (int)vPaths.size(); nPath++ ){
		bool bOutputBaseFolder = false;
		std::tstring sPath = ChopYen( vPaths[nPath] );
		int nTreeRet = DoGrepTree(
			pcViewDst,
			&cDlgCancel,
			pcmGrepKey->GetStringPtr(),
			cGrepEnumKeys,
			cGrepExceptAbsFiles,
			cGrepExceptAbsFolders,
			sPath.c_str(),
			sPath.c_str(),
			sSearchOption,
			sGrepOption,
			pattern,
			&cRegexp,
			0,
			bOutputBaseFolder,
			&nHitCount
		);
		if( nTreeRet == -1 ){
			nGrepTreeResult = -1;
			break;
		}
		nGrepTreeResult += nTreeRet;
	}
	if( -1 == nGrepTreeResult ){
		const wchar_t* p = LSW( STR_GREP_SUSPENDED );	//L"���f���܂����B\r\n"
		pcViewDst->GetCommander().Command_ADDTAIL( p, -1 );
	}
	{
		WCHAR szBuffer[128];
		auto_sprintf( szBuffer, LSW( STR_GREP_MATCH_COUNT ), nHitCount );	//L"%d ����������܂����B\r\n"
		pcViewDst->GetCommander().Command_ADDTAIL( szBuffer, -1 );
#ifdef _DEBUG
		auto_sprintf( szBuffer, LSW(STR_GREP_TIMER), cRunningTimer.Read() );
		pcViewDst->GetCommander().Command_ADDTAIL( szBuffer, -1 );
#endif
	}
	pcViewDst->GetCaret().MoveCursor( CLayoutPoint(CLayoutInt(0), tmp_PosY_Layout), true );	//	�J�[�\����Grep���O�̈ʒu�ɖ߂��B

	cDlgCancel.CloseDialog( 0 );

	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( CEditWnd::getInstance()->GetHwnd() );


	/* �A���h�D�o�b�t�@�̏��� */
	pcViewDst->SetUndoBuffer();

	//	Apr. 13, 2001 genta
	//	Grep���s��̓t�@�C����ύX�����̏�Ԃɂ���D
	pcViewDst->m_pcEditDoc->m_cDocEditor.SetModified(false,false);

	this->m_bGrepRunning = false;
	pcViewDst->m_bDoing_UndoRedo = false;

	/* �\������ON/OFF */
	pCEditWnd->SetDrawSwitchOfAllViews( bDrawSwitchOld );

	/* �ĕ`�� */
	if( !pCEditWnd->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
		pCEditWnd->RedrawAllViews( NULL );

	if( !bGrepCurFolder ){
		// ���s�t�H���_�����������t�H���_�ɕύX
		if( 0 < vPaths.size() ){
			::SetCurrentDirectory( vPaths[0].c_str() );
		}
	}

	return nHitCount;
}




/*! @brief Grep���s

	@date 2001.06.27 genta	���K�\�����C�u�����̍����ւ�
	@date 2003.06.23 Moca   �T�u�t�H���_���t�@�C���������̂��t�@�C�����T�u�t�H���_�̏��ɕύX
	@date 2003.06.23 Moca   �t�@�C��������""����菜���悤��
	@date 2003.03.27 �݂�   ���O�t�@�C���w��̓����Əd�������h�~�̒ǉ��D
		�啔�����ύX���ꂽ���߁C�ʂ̕ύX�_�L���͖����D
*/
int CGrepAgent::DoGrepTree(
	CEditView*				pcViewDst,
	CDlgCancel*				pcDlgCancel,		//!< [in] Cancel�_�C�A���O�ւ̃|�C���^
	const wchar_t*			pszKey,				//!< [in] �����L�[
	CGrepEnumKeys&			cGrepEnumKeys,		//!< [in] �����Ώۃt�@�C���p�^�[��
	CGrepEnumFiles&			cGrepExceptAbsFiles,	//!< [in] ���O�t�@�C����΃p�X
	CGrepEnumFolders&		cGrepExceptAbsFolders,	//!< [in] ���O�t�H���_��΃p�X
	const TCHAR*			pszPath,			//!< [in] �����Ώۃp�X
	const TCHAR*			pszBasePath,		//!< [in] �����Ώۃp�X(�x�[�X�t�H���_)
	const SSearchOption&	sSearchOption,		//!< [in] �����I�v�V����
	const SGrepOption&		sGrepOption,		//!< [in] Grep�I�v�V����
	const CSearchStringPattern& pattern,		//!< [in] �����p�^�[��
	CBregexp*				pRegexp,			//!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������
	int						nNest,				//!< [in] �l�X�g���x��
	bool&					bOutputBaseFolder,	//!< [i/o] �x�[�X�t�H���_���o��
	int*					pnHitCount			//!< [i/o] �q�b�g���̍��v
)
{
	::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURPATH, pszPath );

	int			i;
	int			count;
	CNativeW	cmemMessage;
	LPCTSTR		lpFileName;
	int			nWork = 0;
	int			nHitCountOld = -100;
	bool		bOutputFolderName = false;
	int			nBasePathLen = auto_strlen(pszBasePath);
	CGrepEnumFilterFiles cGrepEnumFilterFiles;
	cGrepEnumFilterFiles.Enumerates( pszPath, cGrepEnumKeys, cGrepExceptAbsFiles );

	/*
	 * �J�����g�t�H���_�̃t�@�C����T������B
	 */
	count = cGrepEnumFilterFiles.GetCount();
	for( i = 0; i < count; i++ ){
		lpFileName = cGrepEnumFilterFiles.GetFileName( i );

		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
			goto cancel_return;
		}
		/* ���f�{�^�������`�F�b�N */
		if( pcDlgCancel->IsCanceled() ){
			goto cancel_return;
		}

		/* �\���ݒ���`�F�b�N */
		CEditWnd::getInstance()->SetDrawSwitchOfAllViews(
			0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW )
		);

		//GREP���s�I
		::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, lpFileName );

		std::tstring currentFile = pszPath;
		currentFile += _T("\\");
		currentFile += lpFileName;
		int nBasePathLen2 = nBasePathLen + 1;
		if( (int)auto_strlen(pszPath) < nBasePathLen2 ){
			nBasePathLen2 = nBasePathLen;
		}

		/* �t�@�C�����̌��� */
		int nRet = DoGrepFile(
			pcViewDst,
			pcDlgCancel,
			pszKey,
			lpFileName,
			sSearchOption,
			sGrepOption,
			pattern,
			pRegexp,
			pnHitCount,
			currentFile.c_str(),
			pszBasePath,
			(sGrepOption.bGrepSeparateFolder && sGrepOption.bGrepOutputBaseFolder ? pszPath + nBasePathLen2 : pszPath),
			(sGrepOption.bGrepSeparateFolder ? lpFileName : currentFile.c_str() + nBasePathLen + 1),
			bOutputBaseFolder,
			bOutputFolderName,
			cmemMessage
		);

		// 2003.06.23 Moca ���A���^�C���\���̂Ƃ��͑��߂ɕ\��
		if( pcViewDst->GetDrawSwitch() ){
			if( LTEXT('\0') != pszKey[0] ){
				// �f�[�^�����̂Ƃ��t�@�C���̍��v���ő�10MB�𒴂�����\��
				nWork += ( cGrepEnumFilterFiles.GetFileSizeLow( i ) + 1023 ) / 1024;
			}
			if( *pnHitCount - nHitCountOld && 
				( *pnHitCount < 20 || 10000 < nWork ) ){
				nHitCountOld = -100; // ���\��
			}
		}
		if( *pnHitCount - nHitCountOld  >= 10 ){
			/* ���ʏo�� */
			if( 0 < cmemMessage.GetStringLength() ){
				pcViewDst->GetCommander().Command_ADDTAIL( cmemMessage.GetStringPtr(), cmemMessage.GetStringLength() );
				pcViewDst->GetCommander().Command_GOFILEEND( FALSE );
				if( !CEditWnd::getInstance()->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
					CEditWnd::getInstance()->RedrawAllViews( pcViewDst );	//	���̃y�C���̕\�����X�V
				cmemMessage.Clear();
			}
			nWork = 0;
			nHitCountOld = *pnHitCount;
		}
		if( -1 == nRet ){
			goto cancel_return;
		}
	}

	// 2010.08.25 �t�H���_�ړ��O�Ɏc����ɏo��
	if( 0 < cmemMessage.GetStringLength() ){
		pcViewDst->GetCommander().Command_ADDTAIL( cmemMessage.GetStringPtr(), cmemMessage.GetStringLength() );
		pcViewDst->GetCommander().Command_GOFILEEND( false );
		if( !CEditWnd::getInstance()->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V
			CEditWnd::getInstance()->RedrawAllViews( pcViewDst );	//	���̃y�C���̕\�����X�V
		cmemMessage.Clear();
	}

	/*
	 * �T�u�t�H���_����������B
	 */
	if( sGrepOption.bGrepSubFolder ){
		CGrepEnumFilterFolders cGrepEnumFilterFolders;
		cGrepEnumFilterFolders.Enumerates( pszPath, cGrepEnumKeys, cGrepExceptAbsFolders );

		count = cGrepEnumFilterFolders.GetCount();
		for( i = 0; i < count; i++ ){
			lpFileName = cGrepEnumFilterFolders.GetFileName( i );

			//�T�u�t�H���_�̒T�����ċA�Ăяo���B
			/* �������̃��[�U�[������\�ɂ��� */
			if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
				goto cancel_return;
			}
			/* ���f�{�^�������`�F�b�N */
			if( pcDlgCancel->IsCanceled() ){
				goto cancel_return;
			}
			/* �\���ݒ���`�F�b�N */
			CEditWnd::getInstance()->SetDrawSwitchOfAllViews(
				0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW )
			);

			//�t�H���_�����쐬����B
			// 2010.08.01 �L�����Z���Ń������[���[�N���Ă܂���
			std::tstring currentPath  = pszPath;
			currentPath += _T("\\");
			currentPath += lpFileName;

			int nGrepTreeResult = DoGrepTree(
				pcViewDst,
				pcDlgCancel,
				pszKey,
				cGrepEnumKeys,
				cGrepExceptAbsFiles,
				cGrepExceptAbsFolders,
				currentPath.c_str(),
				pszBasePath,
				sSearchOption,
				sGrepOption,
				pattern,
				pRegexp,
				nNest + 1,
				bOutputBaseFolder,
				pnHitCount
			);
			if( -1 == nGrepTreeResult ){
				goto cancel_return;
			}
			::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURPATH, pszPath );	//@@@ 2002.01.10 add �T�u�t�H���_����߂��Ă�����...
		}
	}

	::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, LTEXT(" ") );	// 2002/09/09 Moca add

	return 0;


cancel_return:;
	/* ���ʏo�� */
	if( 0 < cmemMessage.GetStringLength() ){
		pcViewDst->GetCommander().Command_ADDTAIL( cmemMessage.GetStringPtr(), cmemMessage.GetStringLength() );
		pcViewDst->GetCommander().Command_GOFILEEND( false );
		if( !CEditWnd::getInstance()->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V
			CEditWnd::getInstance()->RedrawAllViews( pcViewDst );	//	���̃y�C���̕\�����X�V
		cmemMessage.Clear();
	}

	return -1;
}




/*!	@brief Grep���ʂ��\�z����


	pWork�͏[���ȃ������̈�������Ă���R�g
	@date 2002/08/29 Moca �o�C�i���[�f�[�^�ɑΉ� pnWorkLen �ǉ�
	@date 2013.11.05 Moca cmemMessage�ɒ��ڒǉ�����悤��
*/
void CGrepAgent::SetGrepResult(
	/* �f�[�^�i�[�� */
	CNativeW& cmemMessage,
	/* �}�b�`�����t�@�C���̏�� */
	const TCHAR*		pszFilePath,	/*!< [in] �t���p�X or ���΃p�X*/
	const TCHAR*		pszCodeName,	/*!< [in] �����R�[�h���D" [SJIS]"�Ƃ� */
	/* �}�b�`�����s�̏�� */
	int			nLine,				/*!< [in] �}�b�`�����s�ԍ�(1�`) */
	int			nColumn,			/*!< [in] �}�b�`�������ԍ�(1�`) */
	const wchar_t*	pCompareData,	/*!< [in] �s�̕����� */
	int			nLineLen,			/*!< [in] �s�̕�����̒��� */
	int			nEolCodeLen,		/*!< [in] EOL�̒��� */
	/* �}�b�`����������̏�� */
	const wchar_t*	pMatchData,		/*!< [in] �}�b�`���������� */
	int			nMatchLen,			/*!< [in] �}�b�`����������̒��� */
	/* �I�v�V���� */
	const SGrepOption&	sGrepOption
)
{

	CNativeW cmemBuf(L"");
	wchar_t strWork[64];
	const wchar_t * pDispData;
	int k;
	bool bEOL = true;
	int nMaxOutStr = 0;

	/* �m�[�}�� */
	if( 1 == sGrepOption.nGrepOutputStyle ){
		if( sGrepOption.bGrepOutputBaseFolder || sGrepOption.bGrepSeparateFolder ){
			cmemBuf.AppendString( L"�E" );
		}
		cmemBuf.AppendStringT( pszFilePath );
		::auto_sprintf( strWork, L"(%d,%d)", nLine, nColumn );
		cmemBuf.AppendString( strWork );
		cmemBuf.AppendStringT( pszCodeName );
		cmemBuf.AppendString( L": " );
		nMaxOutStr = 2000; // 2003.06.10 Moca �ő咷�ύX
	}
	/* WZ�� */
	else if( 2 == sGrepOption.nGrepOutputStyle ){
		::auto_sprintf( strWork, L"�E(%6d,%-5d): ", nLine, nColumn );
		cmemBuf.AppendString( strWork );
		nMaxOutStr = 2500; // 2003.06.10 Moca �ő咷�ύX
	}
	// ���ʂ̂�
	else if( 3 == sGrepOption.nGrepOutputStyle ){
		nMaxOutStr = 2500;
	}

	/* �Y���s */
	if( sGrepOption.bGrepOutputLine ){
		pDispData = pCompareData;
		k = nLineLen - nEolCodeLen;
		if( nMaxOutStr < k ){
			k = nMaxOutStr; // 2003.06.10 Moca �ő咷�ύX
		}
	}
	/* �Y������ */
	else{
		pDispData = pMatchData;
		k = nMatchLen;
		if( nMaxOutStr < k ){
			k = nMaxOutStr; // 2003.06.10 Moca �ő咷�ύX
		}
		// �Y�������ɉ��s���܂ޏꍇ�͂��̉��s�R�[�h�����̂܂ܗ��p����(���̍s�ɋ�s�����Ȃ�)
		// 2003.06.10 Moca k==0�̂Ƃ��Ƀo�b�t�@�A���_�[�������Ȃ��悤��
		if( 0 < k && WCODE::IsLineDelimiter(pMatchData[ k - 1 ]) ){
			bEOL = false;
		}
	}

	cmemMessage.AllocStringBuffer( cmemMessage.GetStringLength() + cmemBuf.GetStringLength() + 2 );
	cmemMessage.AppendNativeData( cmemBuf );
	cmemMessage.AppendString( pDispData, k );
	if( bEOL ){
		cmemMessage.AppendString( L"\r\n", 2 );
	}
}

static void OutputPathInfo(
	CNativeW&		cmemMessage,
	SGrepOption		sGrepOption,
	const TCHAR*	pszFullPath,
	const TCHAR*	pszBaseFolder,
	const TCHAR*	pszFolder,
	const TCHAR*	pszRelPath,
	const TCHAR*	pszCodeName,
	bool&			bOutputBaseFolder,
	bool&			bOutputFolderName,
	BOOL&			bOutFileName
)
{
	{
		// �o�b�t�@��2^n ���m�ۂ���
		int n = 1024;
		int size = cmemMessage.GetStringLength() + 300;
		while( n < size ){
			n *= 2;
		}
		cmemMessage.AllocStringBuffer( n );
	}
	if( 3 == sGrepOption.nGrepOutputStyle ){
		return;
	}

	if( !bOutputBaseFolder && sGrepOption.bGrepOutputBaseFolder ){
		if( !sGrepOption.bGrepSeparateFolder && 1 == sGrepOption.nGrepOutputStyle ){
			cmemMessage.AppendString( L"��\"" );
		}else{
			cmemMessage.AppendString( L"��\"" );
		}
		cmemMessage.AppendStringT( pszBaseFolder );
		cmemMessage.AppendString( L"\"\r\n" );
		bOutputBaseFolder = true;
	}
	if( !bOutputFolderName && sGrepOption.bGrepSeparateFolder ){
		if( pszFolder[0] ){
			cmemMessage.AppendString( L"��\"" );
			cmemMessage.AppendStringT( pszFolder );
			cmemMessage.AppendString( L"\"\r\n" );
		}else{
			cmemMessage.AppendString( L"��\r\n" );
		}
		bOutputFolderName = true;
	}
	if( 2 == sGrepOption.nGrepOutputStyle ){
		if( !bOutFileName ){
			const TCHAR* pszDispFilePath = ( sGrepOption.bGrepSeparateFolder || sGrepOption.bGrepOutputBaseFolder ) ? pszRelPath : pszFullPath;
			if( sGrepOption.bGrepSeparateFolder ){
				cmemMessage.AppendString( L"��\"" );
			}else{
				cmemMessage.AppendString( L"��\"" );
			}
			cmemMessage.AppendStringT( pszDispFilePath );
			cmemMessage.AppendString( L"\"" );
			cmemMessage.AppendStringT( pszCodeName );
			cmemMessage.AppendString( L"\r\n" );
			bOutFileName = TRUE;
		}
	}
}

/*!
	Grep���s (CFileLoad���g�����e�X�g��)

	@retval -1 GREP�̃L�����Z��
	@retval ����ȊO �q�b�g��(�t�@�C���������̓t�@�C����)

	@date 2001/06/27 genta	���K�\�����C�u�����̍����ւ�
	@date 2002/08/30 Moca CFileLoad���g�����e�X�g��
	@date 2004/03/28 genta �s�v�Ȉ���nNest, bGrepSubFolder, pszPath���폜
*/
int CGrepAgent::DoGrepFile(
	CEditView*				pcViewDst,			//!< 
	CDlgCancel*				pcDlgCancel,		//!< [in] Cancel�_�C�A���O�ւ̃|�C���^
	const wchar_t*			pszKey,				//!< [in] �����p�^�[��
	const TCHAR*			pszFile,			//!< [in] �����Ώۃt�@�C����(�\���p)
	const SSearchOption&	sSearchOption,		//!< [in] �����I�v�V����
	const SGrepOption&		sGrepOption,		//!< [in] Grep�I�v�V����
	const CSearchStringPattern& pattern,		//!< [in] �����p�^�[��
	CBregexp*				pRegexp,			//!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������
	int*					pnHitCount,			//!< [i/o] �q�b�g���̍��v�D���X�̒l�Ɍ��������������Z���ĕԂ��D
	const TCHAR*			pszFullPath,		//!< [in] �����Ώۃt�@�C���p�X C:\Folder\SubFolder\File.ext
	const TCHAR*			pszBaseFolder,		//!< [in] �����t�H���_ C:\Folder
	const TCHAR*			pszFolder,			//!< [in] �T�u�t�H���_ SubFolder (!bGrepSeparateFolder) �܂��� C:\Folder\SubFolder (!bGrepSeparateFolder)
	const TCHAR*			pszRelPath,			//!< [in] ���΃p�X File.ext(bGrepSeparateFolder) �܂���  SubFolder\File.ext(!bGrepSeparateFolder)
	bool&					bOutputBaseFolder,	//!< 
	bool&					bOutputFolderName,	//!< 
	CNativeW&				cmemMessage			//!< 
)
{
	int		nHitCount;
//	char	szLine[16000];
	int		nLine;
	const wchar_t*	pszRes; // 2002/08/29 const�t��
	ECodeType	nCharCode;
	const wchar_t*	pCompareData; // 2002/08/29 const�t��
	int		nColumn;
	BOOL	bOutFileName;
	bOutFileName = FALSE;
	CEol	cEol;
	int		nEolCodeLen;
	const STypeConfigMini* type;
	CDocTypeManager().GetTypeConfigMini( CDocTypeManager().GetDocumentTypeOfPath( pszFile ), &type );
	CFileLoad	cfl( type->m_encoding );	// 2012/12/18 Uchi ��������t�@�C���̃f�t�H���g�̕����R�[�h���擾����l��
//	CFileLoad	cfl( pcViewDst->GetDocument()->m_cDocType.GetDocumentAttribute().m_encoding );
	int		nOldPercent = 0;

	int	nKeyLen = wcslen( pszKey );
	// �t�@�C�����\��
	const TCHAR* pszDispFilePath = ( sGrepOption.bGrepSeparateFolder || sGrepOption.bGrepOutputBaseFolder ) ? pszRelPath : pszFullPath;

	//	�����ł͐��K�\���R���p�C���f�[�^�̏������͕s�v

	const TCHAR*	pszCodeName; // 2002/08/29 const�t��
	pszCodeName = _T("");
	nHitCount = 0;
	nLine = 0;

	/* ���������������[���̏ꍇ�̓t�@�C���������Ԃ� */
	// 2002/08/29 �s���[�v�̑O���炱���Ɉړ�
	if( 0 == nKeyLen ){
		if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
			// 2003.06.10 Moca �R�[�h���ʏ����������Ɉړ��D
			// ���ʃG���[�ł��t�@�C�����ɃJ�E���g���邽��
			// �t�@�C���̓��{��R�[�h�Z�b�g����
			// 2014.06.19 Moca �t�@�C�����̃^�C�v�ʂ�m_encoding�ɕύX
			CCodeMediator cmediator( type->m_encoding );
			nCharCode = cmediator.CheckKanjiCodeOfFile( pszFullPath );
			if( !IsValidCodeType(nCharCode) ){
				pszCodeName = _T("  [(DetectError)]");
			}else{
				pszCodeName = CCodeTypeName(nCharCode).Bracket();
			}
		}
		{
			const wchar_t* pszFormatFullPath = L"";
			const wchar_t* pszFormatBasePath2 = L"";
			const wchar_t* pszFormatFilePath = L"";
			const wchar_t* pszFormatFilePath2 = L"";
			if( 1 == sGrepOption.nGrepOutputStyle ){
				// �m�[�}��
				pszFormatFullPath   = L"%ts%ts\r\n";
				pszFormatBasePath2  = L"��\"%ts\"\r\n";
				pszFormatFilePath   = L"�E\"%ts\"%ts\r\n";
				pszFormatFilePath2  = L"�E\"%ts\"%ts\r\n";
			}else if( 2 == sGrepOption.nGrepOutputStyle ){
				/* WZ�� */
				pszFormatFullPath   = L"��\"%ts\"%ts\r\n";
				pszFormatBasePath2  = L"��\"%ts\"\r\n";
				pszFormatFilePath   = L"��\"%ts\"%ts\r\n";
				pszFormatFilePath2  = L"��\"%ts\"%ts\r\n";
			}else if( 3 == sGrepOption.nGrepOutputStyle ){
				// ���ʂ̂�
				pszFormatFullPath   = L"%ts%ts\r\n";
				pszFormatBasePath2  = L"��\"%ts\"\r\n";
				pszFormatFilePath   = L"%ts\r\n";
				pszFormatFilePath2  = L"%ts\r\n";
			}
/*
			Base/Sep
			O / O  : (A)BaseFolder -> (C)Folder(Rel) -> (E)RelPath(File)
			O / X  : (B)BaseFolder ->                   (F)RelPath(RelFolder/File)
			X / O  :                  (D)Folder(Abs) -> (G)RelPath(File)
			X / X  : (H)FullPath
*/
			auto_array_ptr<wchar_t> pszWork(new wchar_t[auto_strlen(pszFullPath) + auto_strlen(pszCodeName) + 10]);
			wchar_t* szWork0 = &pszWork[0];
			if( sGrepOption.bGrepOutputBaseFolder || sGrepOption.bGrepSeparateFolder ){
				if( !bOutputBaseFolder && sGrepOption.bGrepOutputBaseFolder ){
					const wchar_t* pszFormatBasePath = L"";
					if( sGrepOption.bGrepSeparateFolder ){
						pszFormatBasePath = L"��\"%ts\"\r\n";	// (A)
					}else{
						pszFormatBasePath = pszFormatBasePath2;	// (B)
					}
					auto_sprintf( szWork0, pszFormatBasePath, pszBaseFolder );
					cmemMessage.AppendString( szWork0 );
					bOutputBaseFolder = true;
				}
				if( !bOutputFolderName && sGrepOption.bGrepSeparateFolder ){
					if( pszFolder[0] ){
						auto_sprintf( szWork0, L"��\"%ts\"\r\n", pszFolder );	// (C), (D)
					}else{
						auto_strcpy( szWork0, L"��\r\n" );
					}
					cmemMessage.AppendString( szWork0 );
					bOutputFolderName = true;
				}
				auto_sprintf( szWork0,
					(sGrepOption.bGrepSeparateFolder ? pszFormatFilePath // (E)
						: pszFormatFilePath2),	// (F), (G)
					pszDispFilePath, pszCodeName );
				cmemMessage.AppendString( szWork0 );
			}else{
				auto_sprintf( szWork0, pszFormatFullPath, pszFullPath, pszCodeName );	// (H)
				cmemMessage.AppendString( szWork0 );
			}
		}
		++(*pnHitCount);
		::SetDlgItemInt( pcDlgCancel->GetHwnd(), IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
		return 1;
	}


	try{
	// �t�@�C�����J��
	// FileClose�Ŗ����I�ɕ��邪�A���Ă��Ȃ��Ƃ��̓f�X�g���N�^�ŕ���
	// 2003.06.10 Moca �����R�[�h���菈����FileOpen�ōs��
	nCharCode = cfl.FileOpen( pszFullPath, sGrepOption.nGrepCharSet, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode() );
	if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
		pszCodeName = CCodeTypeName(nCharCode).Bracket();
	}

//	/* �������̃��[�U�[������\�ɂ��� */
	if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
		return -1;
	}
	/* ���f�{�^�������`�F�b�N */
	if( pcDlgCancel->IsCanceled() ){
		return -1;
	}

	/* ���������������[���̏ꍇ�̓t�@�C���������Ԃ� */
	// 2002/08/29 �t�@�C���I�[�v���̎�O�ֈړ�
	
	std::vector<std::pair<const wchar_t*, CLogicInt> > searchWords;
	if( sSearchOption.bWordOnly ){
		CSearchAgent::CreateWordList( searchWords, pszKey, nKeyLen );
	}

	// ���� : cfl.ReadLine �� throw ����\��������
	CNativeW cUnicodeBuffer;
	while( RESULT_FAILURE != cfl.ReadLine( &cUnicodeBuffer, &cEol ) )
	{
		const wchar_t*	pLine = cUnicodeBuffer.GetStringPtr();
		int		nLineLen = cUnicodeBuffer.GetStringLength();

		nEolCodeLen = cEol.GetLen();
		++nLine;
		pCompareData = pLine;

		/* �������̃��[�U�[������\�ɂ��� */
		// 2010.08.31 �Ԋu��1/32�ɂ���
		if( ((0 == nLine % 32)|| 10000 < nLineLen ) && !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
			return -1;
		}
		if( 0 == nLine % 64 ){
			/* ���f�{�^�������`�F�b�N */
			if( pcDlgCancel->IsCanceled() ){
				return -1;
			}
			//	2003.06.23 Moca �\���ݒ���`�F�b�N
			CEditWnd::getInstance()->SetDrawSwitchOfAllViews(
				0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW )
			);
			// 2002/08/30 Moca �i�s��Ԃ�\������(5MB�ȏ�)
			if( 5000000 < cfl.GetFileSize() ){
				int nPercent = cfl.GetPercent();
				if( 5 <= nPercent - nOldPercent ){
					nOldPercent = nPercent;
					TCHAR szWork[10];
					::auto_sprintf( szWork, _T(" (%3d%%)"), nPercent );
					std::tstring str;
					str = str + pszFile + szWork;
					::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, str.c_str() );
				}
			}
		}

		/* ���K�\������ */
		if( sSearchOption.bRegularExp ){
			int nIndex = 0;
#ifdef _DEBUG
			int nIndexPrev = -1;
#endif

			//	Jun. 21, 2003 genta ���[�v����������
			//	�}�b�`�ӏ���1�s���畡�����o����P�[�X��W���ɁC
			//	�}�b�`�ӏ���1�s����1�������o����ꍇ���O�P�[�X�ƂƂ炦�C
			//	���[�v�p���E�ł��؂����(bGrepOutputLine)���t�ɂ����D
			//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
			// From Here 2005.03.19 ����� ���͂�BREGEXP�\���̂ɒ��ڃA�N�Z�X���Ȃ�
			// 2010.08.25 �s���ȊO��^�Ƀ}�b�`����s��̏C��
			while( nIndex <= nLineLen && pRegexp->Match( pLine, nLineLen, nIndex ) ){

					//	�p�^�[������
					nIndex = pRegexp->GetIndex();
					int matchlen = pRegexp->GetMatchLen();
#ifdef _DEBUG
					if( nIndex <= nIndexPrev ){
						MYTRACE( _T("ERROR: CEditView::DoGrepFile() nIndex <= nIndexPrev break \n") );
						break;
					}
					nIndexPrev = nIndex;
#endif

					OutputPathInfo(
						cmemMessage, sGrepOption,
						pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
						bOutputBaseFolder, bOutputFolderName, bOutFileName
					);
					/* Grep���ʂ��AcmemMessage�Ɋi�[���� */
					SetGrepResult(
						cmemMessage,
						pszDispFilePath,
						pszCodeName,
						nLine,
						nIndex + 1,
						pLine,
						nLineLen,
						nEolCodeLen,
						pLine + nIndex,
						matchlen,
						sGrepOption
					);
					// To Here 2005.03.19 ����� ���͂�BREGEXP�\���̂ɒ��ڃA�N�Z�X���Ȃ�
					++nHitCount;
					++(*pnHitCount);
					if( 0 == ( (*pnHitCount) % 16 ) || *pnHitCount < 16 ){
						::SetDlgItemInt( pcDlgCancel->GetHwnd(), IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					//	Jun. 21, 2003 genta �s�P�ʂŏo�͂���ꍇ��1������Ώ\��
					if ( sGrepOption.bGrepOutputLine || sGrepOption.bGrepOutputFileOnly ) {
						break;
					}
					//	�T���n�߂�ʒu��␳
					//	2003.06.10 Moca �}�b�`����������̌�납�玟�̌������J�n����
					if( matchlen <= 0 ){
						matchlen = CNativeW::GetSizeOfChar( pLine, nLineLen, nIndex );
						if( matchlen <= 0 ){
							matchlen = 1;
						}
					}
					nIndex += matchlen;
			}
		}
		/* �P��̂݌��� */
		else if( sSearchOption.bWordOnly ){
			/*
				2002/02/23 Norio Nakatani
				�P��P�ʂ�Grep�������I�Ɏ����B�P���WhereCurrentWord()�Ŕ��ʂ��Ă܂��̂ŁA
				�p�P���C/C++���ʎq�Ȃǂ̌��������Ȃ�q�b�g���܂��B

				2002/03/06 YAZAKI
				Grep�ɂ����������B
				WhereCurrentWord�ŒP��𒊏o���āA���̒P�ꂪ������Ƃ����Ă��邩��r����B
			*/
			int nMatchLen;
			int nIdx = 0;
			// Jun. 26, 2003 genta ���ʂ�while�͍폜
			while( ( pszRes = CSearchAgent::SearchStringWord(pLine, nLineLen, nIdx, searchWords, sSearchOption.bLoHiCase, &nMatchLen) ) != NULL ){
				nIdx = pszRes - pLine + nMatchLen;
				OutputPathInfo(
					cmemMessage, sGrepOption,
					pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
					bOutputBaseFolder, bOutputFolderName, bOutFileName
				);
				/* Grep���ʂ��AcmemMessage�Ɋi�[���� */
				SetGrepResult(
					cmemMessage,
					pszDispFilePath, pszCodeName,
					//	Jun. 25, 2002 genta
					//	���ʒu��1�n�܂�Ȃ̂�1�𑫂��K�v������
					nLine, pszRes - pLine + 1, pLine, nLineLen, nEolCodeLen,
					pszRes, nMatchLen,
					sGrepOption
				);
				++nHitCount;
				++(*pnHitCount);
				//	May 22, 2000 genta
				if( 0 == ( (*pnHitCount) % 16 ) || *pnHitCount < 16 ){
					::SetDlgItemInt( pcDlgCancel->GetHwnd(), IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
				}

				// 2010.10.31 ryoji �s�P�ʂŏo�͂���ꍇ��1������Ώ\��
				if ( sGrepOption.bGrepOutputLine || sGrepOption.bGrepOutputFileOnly ) {
					break;
				}
			}
		}
		else {
			/* �����񌟍� */
			int nColumnPrev = 0;
			//	Jun. 21, 2003 genta ���[�v����������
			//	�}�b�`�ӏ���1�s���畡�����o����P�[�X��W���ɁC
			//	�}�b�`�ӏ���1�s����1�������o����ꍇ���O�P�[�X�ƂƂ炦�C
			//	���[�v�p���E�ł��؂����(bGrepOutputLine)���t�ɂ����D
			for (;;) {
				pszRes = CSearchAgent::SearchString(
					pCompareData,
					nLineLen,
					0,
					pattern
				);
				if(!pszRes)break;

				nColumn = pszRes - pCompareData + 1;

				OutputPathInfo(
					cmemMessage, sGrepOption,
					pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
					bOutputBaseFolder, bOutputFolderName, bOutFileName
				);
				/* Grep���ʂ��AcmemMessage�Ɋi�[���� */
				SetGrepResult(
					cmemMessage,
					pszDispFilePath, pszCodeName,
					nLine, nColumn + nColumnPrev, pCompareData, nLineLen, nEolCodeLen,
					pszRes, nKeyLen,
					sGrepOption
				);
				++nHitCount;
				++(*pnHitCount);
				//	May 22, 2000 genta
				if( 0 == ( (*pnHitCount) % 16 ) || *pnHitCount < 16 ){
					::SetDlgItemInt( pcDlgCancel->GetHwnd(), IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
				}
				
				//	Jun. 21, 2003 genta �s�P�ʂŏo�͂���ꍇ��1������Ώ\��
				if ( sGrepOption.bGrepOutputLine || sGrepOption.bGrepOutputFileOnly ) {
					break;
				}
				//	�T���n�߂�ʒu��␳
				//	2003.06.10 Moca �}�b�`����������̌�납�玟�̌������J�n����
				//	nClom : �}�b�`�ʒu
				//	matchlen : �}�b�`����������̒���
				int nPosDiff = nColumn += nKeyLen - 1;
				pCompareData += nPosDiff;
				nLineLen -= nPosDiff;
				nColumnPrev += nPosDiff;
			}
		}
		// �t�@�C�������̏ꍇ�́A1����������I��
		if( sGrepOption.bGrepOutputFileOnly && 1 <= nHitCount ){
			break;
		}
	}

	// �t�@�C���𖾎��I�ɕ��邪�A�����ŕ��Ȃ��Ƃ��̓f�X�g���N�^�ŕ��Ă���
	cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		CNativeW str(LSW(STR_GREP_ERR_FILEOPEN));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		cmemMessage.AppendNativeData( str );
		return 0;
	}
	catch( CError_FileRead ){
		CNativeW str(LSW(STR_GREP_ERR_FILEREAD));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		cmemMessage.AppendNativeData( str );
	} // ��O�����I���

	return nHitCount;
}


