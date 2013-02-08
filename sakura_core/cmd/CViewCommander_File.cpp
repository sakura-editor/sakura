/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�t�@�C������n)�֐��Q

	2012/12/20	CViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2003, MIK, genta, �����, Moca
	Copyright (C) 2004, genta
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji, maru
	Copyright (C) 2007, ryoji, maru, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"
#include "_main/CControlTray.h"
#include "CWaitCursor.h"
#include "dlg/CDlgProperty.h"
#include "dlg/CDlgCancel.h"// 2002/2/8 hor
#include "doc/CDocReader.h"	//  Command_PROPERTY_FILE for _DEBUG
#include "window/CEditWnd.h"/// 2002/2/3 aroka �ǉ�
#include "CPrintPreview.h"
#include "io/CBinaryStream.h"
#include "io/CFileLoad.h"
#include "CWriteManager.h"
#include "CEditApp.h"
#include "recent/CMRUFile.h"
#include "util/window.h"
#include "charset/CCodeFactory.h"
#include "debug/CRunningTimer.h"
#include "sakura_rc.h"


/* �V�K�쐬 */
void CViewCommander::Command_FILENEW( void )
{
	/* �V���ȕҏW�E�B���h�E���N�� */
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = _T("");
	sLoadInfo.eCharCode = CODE_NONE;
	sLoadInfo.bViewMode = false;
	CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo, NULL, false, NULL, false );
	return;
}



/* �V�K�쐬�i�V�����E�C���h�E�ŊJ���j */
void CViewCommander::Command_FILENEW_NEWWINDOW( void )
{
	/* �V���ȕҏW�E�B���h�E���N�� */
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = _T("");
	sLoadInfo.eCharCode = CODE_DEFAULT;
	sLoadInfo.bViewMode = false;
	CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo,
		NULL,
		false,
		NULL,
		true
		);
	return;
}



/*! @brief �t�@�C�����J��

	@date 2003.03.30 genta �u���ĊJ���v���痘�p���邽�߂Ɉ����ǉ�
	@date 2004.10.09 genta ������CEditDoc�ֈړ�
*/
void CViewCommander::Command_FILEOPEN( const WCHAR* filename, ECodeType nCharCode, bool bViewMode )
{
	//���[�h���
	SLoadInfo sLoadInfo(filename?to_tchar(filename):_T(""), nCharCode, bViewMode);
	std::vector<std::tstring> files;

	//�K�v�ł���΁u�t�@�C�����J���v�_�C�A���O
	if(!sLoadInfo.cFilePath.IsValidPath()){
		bool bDlgResult = GetDocument()->m_cDocFileOperation.OpenFileDialog(
			CEditWnd::getInstance()->GetHwnd(),	//[in]  �I�[�i�[�E�B���h�E
			NULL,								//[in]  �t�H���_
			&sLoadInfo,							//[out] ���[�h���󂯎��
			files								//[out] �t�@�C����
		);
		if(!bDlgResult)return;

		sLoadInfo.cFilePath = files[0].c_str();
		// ���̃t�@�C���͐V�K�E�B���h�E
		for( size_t i = 1; i < files.size(); i++ ){
			SLoadInfo sFilesLoadInfo = sLoadInfo;
			sFilesLoadInfo.cFilePath = files[i].c_str();
			CControlTray::OpenNewEditor(
				G_AppInstance(),
				CEditWnd::getInstance()->GetHwnd(),
				sFilesLoadInfo,
				NULL,
				true
			);
		}
	}

	//�J��
	GetDocument()->m_cDocFileOperation.FileLoad( &sLoadInfo );
}



/*! �㏑���ۑ�

	F_FILESAVEALL�Ƃ̑g�ݍ��킹�݂̂Ŏg����R�}���h�D
	@param warnbeep [in] true: �ۑ��s�v or �ۑ��֎~�̂Ƃ��Ɍx�����o��
	@param askname	[in] true: �t�@�C�������ݒ�̎��ɓ��͂𑣂�

	@date 2004.02.28 genta ����warnbeep�ǉ�
	@date 2005.01.24 genta ����askname�ǉ�

*/
bool CViewCommander::Command_FILESAVE( bool warnbeep, bool askname )
{
	CEditDoc* pcDoc = GetDocument();

	//�t�@�C�������w�肳��Ă��Ȃ��ꍇ�́u���O��t���ĕۑ��v�̃t���[�֑J��
	if( !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		if( !askname )
			return false;	// �ۑ����Ȃ�
		return pcDoc->m_cDocFileOperation.FileSaveAs();
	}

	//�Z�[�u���
	SSaveInfo sSaveInfo;
	pcDoc->GetSaveInfo(&sSaveInfo);
	sSaveInfo.cEol = EOL_NONE; //���s�R�[�h���ϊ�
	sSaveInfo.bOverwriteMode = true; //�㏑���v��

	//�㏑������
	if(!warnbeep)CEditApp::getInstance()->m_cSoundSet.MuteOn();
	bool bRet = pcDoc->m_cDocFileOperation.DoSaveFlow(&sSaveInfo);
	if(!warnbeep)CEditApp::getInstance()->m_cSoundSet.MuteOff();

	return bRet;
}



/* ���O��t���ĕۑ��_�C�A���O */
bool CViewCommander::Command_FILESAVEAS_DIALOG()
{
	return 	GetDocument()->m_cDocFileOperation.FileSaveAs();
}



/* ���O��t���ĕۑ�
	filename�ŕۑ��BNULL�͌��ցB
*/
BOOL CViewCommander::Command_FILESAVEAS( const WCHAR* filename, EEolType eEolType )
{
	return 	GetDocument()->m_cDocFileOperation.FileSaveAs(filename, eEolType);
}



/*!	�S�ď㏑���ۑ�

	�ҏW���̑S�ẴE�B���h�E�ŏ㏑���ۑ����s���D
	�������C�㏑���ۑ��̎w�����o���݂̂Ŏ��s���ʂ̊m�F�͍s��Ȃ��D

	�㏑���֎~�y�уt�@�C�������ݒ�̃E�B���h�E�ł͉����s��Ȃ��D

	@date 2005.01.24 genta �V�K�쐬
*/
BOOL CViewCommander::Command_FILESAVEALL( void )
{
	CAppNodeGroupHandle(0).SendMessageToAllEditors(
		WM_COMMAND,
		MAKELONG( F_FILESAVE_QUIET, 0 ),
		0,
		NULL
	);
	return TRUE;
}



/* ����(����) */	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
void CViewCommander::Command_FILECLOSE( void )
{
	GetDocument()->m_cDocFileOperation.FileClose();
}



/*! @brief ���ĊJ��

	@date 2003.03.30 genta �J���_�C�A���O�ŃL�����Z�������Ƃ����̃t�@�C�����c��悤�ɁB
				���ł�FILEOPEN�Ɠ����悤�Ɉ�����ǉ����Ă���
*/
void CViewCommander::Command_FILECLOSE_OPEN( LPCWSTR filename, ECodeType nCharCode, bool bViewMode )
{
	GetDocument()->m_cDocFileOperation.FileCloseOpen( SLoadInfo(to_tchar(filename), nCharCode, bViewMode) );
}



//! �t�@�C���̍ăI�[�v��
void CViewCommander::Command_FILE_REOPEN(
	ECodeType	nCharCode,	//!< [in] �J�������ۂ̕����R�[�h
	bool		bNoConfirm	//!< [in] �t�@�C�����X�V���ꂽ�ꍇ�Ɋm�F���s��u�Ȃ��v���ǂ����Btrue:�m�F���Ȃ� false:�m�F����
)
{
	CEditDoc* pcDoc = GetDocument();
	if( !bNoConfirm && fexist(pcDoc->m_cDocFile.GetFilePath()) && pcDoc->m_cDocEditor.IsModified() ){
		int nDlgResult = MYMESSAGEBOX(
			m_pCommanderView->GetHwnd(),
			MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("%ts\n\n���̃t�@�C���͕ύX����Ă��܂��B\n�ă��[�h���s���ƕύX�������܂����A��낵���ł���?"),
			pcDoc->m_cDocFile.GetFilePath()
		);
		if( IDOK == nDlgResult ){
			//�p���B���֐i��
		}else{
			return; //���f
		}
	}

	// ����t�@�C���̍ăI�[�v��
	pcDoc->m_cDocFileOperation.ReloadCurrentFile( nCharCode );
}



/* ��� */
void CViewCommander::Command_PRINT( void )
{
	// �g���Ă��Ȃ��������폜 2003.05.04 �����
	Command_PRINT_PREVIEW();
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* ������s */
	pCEditWnd->m_pPrintPreview->OnPrint();
}



/* ����v���r���[ */
void CViewCommander::Command_PRINT_PREVIEW( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* ����v���r���[���[�h�̃I��/�I�t */
	pCEditWnd->PrintPreviewModeONOFF();
	return;
}



/* ����̃y�[�W���C�A�E�g�̐ݒ� */
void CViewCommander::Command_PRINT_PAGESETUP( void )
{
	BOOL		bRes;
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* ����y�[�W�ݒ� */
	bRes = pCEditWnd->OnPrintPageSetting();
	return;
}



//From Here Feb. 10, 2001 JEPRO �ǉ�
/* C/C++�w�b�_�t�@�C���܂��̓\�[�X�t�@�C�� �I�[�v���@�\ */
BOOL CViewCommander::Command_OPEN_HfromtoC( BOOL bCheckOnly )
{
	if ( Command_OPEN_HHPP( bCheckOnly, FALSE ) )	return TRUE;
	if ( Command_OPEN_CCPP( bCheckOnly, FALSE ) )	return TRUE;
	ErrorBeep();
	return FALSE;
// 2002/03/24 YAZAKI �R�[�h�̏d�����팸
// 2003.06.28 Moca �R�����g�Ƃ��Ďc���Ă����R�[�h���폜
}



/* C/C++�w�b�_�t�@�C�� �I�[�v���@�\ */		//Feb. 10, 2001 jepro	�������u�C���N���[�h�t�@�C���v����ύX
//BOOL CViewCommander::Command_OPENINCLUDEFILE( BOOL bCheckOnly )
BOOL CViewCommander::Command_OPEN_HHPP( BOOL bCheckOnly, BOOL bBeepWhenMiss )
{
	// 2003.06.28 Moca �w�b�_�E�\�[�X�̃R�[�h�𓝍����폜
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return m_pCommanderView->OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, source_ext, header_ext,
		_countof(source_ext), _countof(header_ext),
		_T("C/C++�w�b�_�t�@�C���̃I�[�v���Ɏ��s���܂����B") );
}



/* C/C++�\�[�X�t�@�C�� �I�[�v���@�\ */
//BOOL CViewCommander::Command_OPENCCPP( BOOL bCheckOnly )	//Feb. 10, 2001 JEPRO	�R�}���h�����኱�ύX
BOOL CViewCommander::Command_OPEN_CCPP( BOOL bCheckOnly, BOOL bBeepWhenMiss )
{
	// 2003.06.28 Moca �w�b�_�E�\�[�X�̃R�[�h�𓝍����폜
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return m_pCommanderView->OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, header_ext, source_ext,
		_countof(header_ext), _countof(source_ext),
		_T("C/C++�\�[�X�t�@�C���̃I�[�v���Ɏ��s���܂����B"));
}



/* Oracle SQL*Plus���A�N�e�B�u�\�� */
void CViewCommander::Command_ACTIVATE_SQLPLUS( void )
{
	HWND		hwndSQLPLUS;
	hwndSQLPLUS = ::FindWindow( _T("SqlplusWClass"), _T("Oracle SQL*Plus") );
	if( NULL == hwndSQLPLUS ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("Oracle SQL*Plus���A�N�e�B�u�\�����܂��B\n\n\nOracle SQL*Plus���N������Ă��܂���B\n") );
		return;
	}
	/* Oracle SQL*Plus���A�N�e�B�u�ɂ��� */
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( hwndSQLPLUS );
	return;
}



/* Oracle SQL*Plus�Ŏ��s */
void CViewCommander::Command_PLSQL_COMPILE_ON_SQLPLUS( void )
{
//	HGLOBAL		hgClip;
//	char*		pszClip;
	HWND		hwndSQLPLUS;
	int			nRet;
	BOOL		nBool;
	TCHAR		szPath[MAX_PATH + 2];
	BOOL		bResult;

	hwndSQLPLUS = ::FindWindow( _T("SqlplusWClass"), _T("Oracle SQL*Plus") );
	if( NULL == hwndSQLPLUS ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("Oracle SQL*Plus�Ŏ��s���܂��B\n\n\nOracle SQL*Plus���N������Ă��܂���B\n") );
		return;
	}
	/* �e�L�X�g���ύX����Ă���ꍇ */
	if( GetDocument()->m_cDocEditor.IsModified() ){
		nRet = ::MYMESSAGEBOX(
			m_pCommanderView->GetHwnd(),
			MB_YESNOCANCEL | MB_ICONEXCLAMATION,
			GSTR_APPNAME,
			_T("%ts\n�͕ύX����Ă��܂��B Oracle SQL*Plus�Ŏ��s����O�ɕۑ����܂����H"),
			GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ? GetDocument()->m_cDocFile.GetFilePath() : _T("(����)")
		);
		switch( nRet ){
		case IDYES:
			if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
				//nBool = HandleCommand( F_FILESAVE, TRUE, 0, 0, 0, 0 );
				nBool = Command_FILESAVE();
			}else{
				//nBool = HandleCommand( F_FILESAVEAS_DIALOG, TRUE, 0, 0, 0, 0 );
				nBool = Command_FILESAVEAS_DIALOG();
			}
			if( !nBool ){
				return;
			}
			break;
		case IDNO:
			return;
		case IDCANCEL:
		default:
			return;
		}
	}
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		/* �t�@�C���p�X�ɋ󔒂��܂܂�Ă���ꍇ�̓_�u���N�H�[�e�[�V�����ň͂� */
		//	2003.10.20 MIK �R�[�h�ȗ���
		if( _tcschr( GetDocument()->m_cDocFile.GetFilePath(), TCODE::SPACE ) ? TRUE : FALSE ){
			auto_sprintf( szPath, _T("@\"%ts\"\r\n"), GetDocument()->m_cDocFile.GetFilePath() );
		}else{
			auto_sprintf( szPath, _T("@%ts\r\n"), GetDocument()->m_cDocFile.GetFilePath() );
		}
		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		m_pCommanderView->MySetClipboardData( szPath, _tcslen( szPath ), false );

		/* Oracle SQL*Plus���A�N�e�B�u�ɂ��� */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( hwndSQLPLUS );

		/* Oracle SQL*Plus�Ƀy�[�X�g�̃R�}���h�𑗂� */
		DWORD_PTR	dwResult;
		bResult = ::SendMessageTimeout(
			hwndSQLPLUS,
			WM_COMMAND,
			MAKELONG( 201, 0 ),
			0,
			SMTO_ABORTIFHUNG | SMTO_NORMAL,
			3000,
			&dwResult
		);
		if( !bResult ){
			TopErrorMessage( m_pCommanderView->GetHwnd(), _T("Oracle SQL*Plus����̔���������܂���B\n���΂炭�҂��Ă���Ăю��s���Ă��������B") );
		}
	}else{
		ErrorBeep();
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("SQL���t�@�C���ɕۑ����Ȃ���Oracle SQL*Plus�Ŏ��s�ł��܂���B\n") );
		return;
	}
	return;
}



/* �u���E�Y */
void CViewCommander::Command_BROWSE( void )
{
	if( !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		ErrorBeep();
		return;
	}
//	char	szURL[MAX_PATH + 64];
//	auto_sprintf( szURL, L"%ls", GetDocument()->m_cDocFile.GetFilePath() );
	/* URL���J�� */
//	::ShellExecuteEx( NULL, L"open", szURL, NULL, NULL, SW_SHOW );

    SHELLEXECUTEINFO info; 
    info.cbSize =sizeof(info);
    info.fMask = 0;
    info.hwnd = NULL;
    info.lpVerb = NULL;
    info.lpFile = GetDocument()->m_cDocFile.GetFilePath();
    info.lpParameters = NULL;
    info.lpDirectory = NULL;
    info.nShow = SW_SHOWNORMAL;
    info.hInstApp = 0;
    info.lpIDList = NULL;
    info.lpClass = NULL;
    info.hkeyClass = 0; 
    info.dwHotKey = 0;
    info.hIcon =0;

	::ShellExecuteEx(&info);

	return;
}



/* �r���[���[�h */
void CViewCommander::Command_VIEWMODE( void )
{
	//�r���[���[�h�𔽓]
	CAppMode::getInstance()->SetViewMode(!CAppMode::getInstance()->IsViewMode());

	// �r������̐؂�ւ�
	// ���r���[���[�h ON ���͔r������ OFF�A�r���[���[�h OFF ���͔r������ ON �̎d�l�i>>data:5262�j�𑦎����f����
	GetDocument()->m_cDocFileOperation.DoFileUnlock();	// �t�@�C���̔r�����b�N����
	GetDocument()->m_cDocLocker.CheckWritable(!CAppMode::getInstance()->IsViewMode());	// �t�@�C�������\�̃`�F�b�N
	if( GetDocument()->m_cDocLocker.IsDocWritable() ){
		GetDocument()->m_cDocFileOperation.DoFileLock();	// �t�@�C���̔r�����b�N
	}

	// �e�E�B���h�E�̃^�C�g�����X�V
	this->GetEditWindow()->UpdateCaption();
}



/* �t�@�C���̃v���p�e�B */
void CViewCommander::Command_PROPERTY_FILE( void )
{
#ifdef _DEBUG
	{
		/* �S�s�f�[�^��Ԃ��e�X�g */
		wchar_t*	pDataAll;
		int		nDataAllLen;
		CRunningTimer cRunningTimer( "CViewCommander::Command_PROPERTY_FILE �S�s�f�[�^��Ԃ��e�X�g" );
		cRunningTimer.Reset();
		pDataAll = CDocReader(GetDocument()->m_cDocLineMgr).GetAllData( &nDataAllLen );
//		MYTRACE_A( "�S�f�[�^�擾             (%d�o�C�g) ���v����(�~���b) = %d\n", nDataAllLen, cRunningTimer.Read() );
		free( pDataAll );
		pDataAll = NULL;
//		MYTRACE_A( "�S�f�[�^�擾�̃������J�� (%d�o�C�g) ���v����(�~���b) = %d\n", nDataAllLen, cRunningTimer.Read() );
	}
#endif


	CDlgProperty	cDlgProperty;
//	cDlgProperty.Create( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument() );
	cDlgProperty.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() );
	return;
}



/* �ҏW�̑S�I�� */	// 2007.02.13 ryoji �ǉ�
void CViewCommander::Command_EXITALLEDITORS( void )
{
	CControlTray::CloseAllEditor( TRUE, GetMainWindow(), TRUE, 0 );
	return;
}



/* �T�N���G�f�B�^�̑S�I�� */	//Dec. 27, 2000 JEPRO �ǉ�
void CViewCommander::Command_EXITALL( void )
{
	CControlTray::TerminateApplication( GetMainWindow() );	// 2006.12.25 ryoji �����ǉ�
	return;
}



/*!	@brief �ҏW���̓��e��ʖ��ۑ�

	��ɕҏW���̈ꎞ�t�@�C���o�͂Ȃǂ̖ړI�Ɏg�p����D
	���݊J���Ă���t�@�C��(m_szFilePath)�ɂ͉e�����Ȃ��D

	@retval	TRUE ����I��
	@retval	FALSE �t�@�C���쐬�Ɏ��s

	@author	maru
	@date	2006.12.10 maru �V�K�쐬
*/
BOOL CViewCommander::Command_PUTFILE(
	LPCWSTR		filename,	//!< [in] filename �o�̓t�@�C����
	ECodeType	nCharCode,	//!< [in] nCharCode �����R�[�h�w��
							//!<  @li CODE_xxxxxxxxxx:�e�핶���R�[�h
							//!<  @li CODE_AUTODETECT:���݂̕����R�[�h���ێ�
	int			nFlgOpt		//!< [in] nFlgOpt ����I�v�V����
							//!<  @li 0x01:�I��͈͂��o�� (��I����Ԃł���t�@�C�����o�͂���)
)
{
	BOOL		bResult = TRUE;
	ECodeType	nSaveCharCode = nCharCode;
	if(filename[0] == L'\0') {
		return FALSE;
	}

	if(nSaveCharCode == CODE_AUTODETECT) nSaveCharCode = GetDocument()->GetDocumentEncoding();

	//	2007.09.08 genta CEditDoc::FileWrite()�ɂȂ���č����v�J�[�\��
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	std::auto_ptr<CCodeBase> pcSaveCode( CCodeFactory::CreateCodeBase(nSaveCharCode,0) );

	bool bBom = false;
	if (CCodeTypeName(nSaveCharCode).UseBom()) {
		bBom = GetDocument()->m_cDocFile.IsBomExist();
	}

	if(nFlgOpt & 0x01)
	{	/* �I��͈͂��o�� */
		try
		{
			CBinaryOutputStream out(to_tchar(filename),true);

			// �I��͈͂̎擾 -> cMem
			CNativeW cMem;
			m_pCommanderView->GetSelectedData(&cMem, FALSE, NULL, FALSE, FALSE);

			// BOM�ǉ�
			CNativeW cMem2;
			const CNativeW* pConvBuffer;
			if( bBom ){
				CNativeW cmemBom;
				std::auto_ptr<CCodeBase> pcUtf16( CCodeFactory::CreateCodeBase(CODE_UNICODE,0) );
				pcUtf16->GetBom(cmemBom._GetMemory());
				cMem2.AppendNativeData(cmemBom);
				cMem2.AppendNativeData(cMem);
				cMem.Clear();
				pConvBuffer = &cMem2;
			}else{
				pConvBuffer = &cMem;
			}

			// �������ݎ��̃R�[�h�ϊ� -> cDst
			CMemory cDst;
			pcSaveCode->UnicodeToCode(*pConvBuffer, &cDst);

			//����
			if( 0 < cDst.GetRawLength() )
				out.Write(cDst.GetRawPtr(),cDst.GetRawLength());
		}
		catch(CError_FileOpen)
		{
			WarningMessage(
				NULL,
				_T("\'%ls\'\n")
				_T("�t�@�C����ۑ��ł��܂���B\n")
				_T("�p�X�����݂��Ȃ����A���̃A�v���P�[�V�����Ŏg�p����Ă���\��������܂��B"),
				filename
			);
			bResult = FALSE;
		}
		catch(CError_FileWrite)
		{
			WarningMessage(
				NULL,
				_T("�t�@�C���̏������ݒ��ɃG���[���������܂����B")
			);
			bResult = FALSE;
		}
	}
	else {	/* �t�@�C���S�̂��o�� */
		HWND		hwndProgress;
		CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;

		if( NULL != pCEditWnd ){
			hwndProgress = pCEditWnd->m_cStatusBar.GetProgressHwnd();
		}else{
			hwndProgress = NULL;
		}
		if( NULL != hwndProgress ){
			::ShowWindow( hwndProgress, SW_SHOW );
		}

		// �ꎞ�t�@�C���o��
		EConvertResult eRet = CWriteManager().WriteFile_From_CDocLineMgr(
			GetDocument()->m_cDocLineMgr,
			SSaveInfo(
				to_tchar(filename),
				nSaveCharCode,
				EOL_NONE,
				bBom
			)
		);
		bResult = (eRet != RESULT_FAILURE);

		if(hwndProgress) ::ShowWindow( hwndProgress, SW_HIDE );
	}
	return bResult;
}



/*!	@brief �J�[�\���ʒu�Ƀt�@�C����}��

	���݂̃J�[�\���ʒu�Ɏw��̃t�@�C����ǂݍ��ށD

	@param[in] filename ���̓t�@�C����
	@param[in] nCharCode �����R�[�h�w��
		@li	CODE_xxxxxxxxxx:�e�핶���R�[�h
		@li	CODE_AUTODETECT:�O�񕶎��R�[�h�������͎������ʂ̌��ʂɂ��
	@param[in] nFlgOpt ����I�v�V�����i���݂͖���`�D0���w��̂��Ɓj

	@retval	TRUE ����I��
	@retval	FALSE �t�@�C���I�[�v���Ɏ��s

	@author	maru
	@date	2006.12.10 maru �V�K�쐬
*/
BOOL CViewCommander::Command_INSFILE( LPCWSTR filename, ECodeType nCharCode, int nFlgOpt )
{
	CFileLoad	cfl(GetDocument()->m_cDocType.GetDocumentAttribute().m_encoding);
	CEol cEol;
	int			nLineNum = 0;

	CDlgCancel*	pcDlgCancel = NULL;
	HWND		hwndCancel = NULL;
	HWND		hwndProgress = NULL;
	BOOL		bResult = TRUE;

	if(filename[0] == L'\0') {
		return FALSE;
	}

	//	2007.09.08 genta CEditDoc::FileLoad()�ɂȂ���č����v�J�[�\��
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	// �͈͑I�𒆂Ȃ�}������I����Ԃɂ��邽��	/* 2007.04.29 maru */
	BOOL	bBeforeTextSelected = m_pCommanderView->GetSelectionInfo().IsTextSelected();
	CLayoutPoint ptFrom;
	/*
	int	nLineFrom, nColmFrom;
	*/
	if (bBeforeTextSelected){
		ptFrom = m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom();
		/*
		nLineFrom = m_nSelectLineFrom;
		nColmFrom = m_nSelectColmFrom;
		*/
	}


	ECodeType	nSaveCharCode = nCharCode;
	if(nSaveCharCode == CODE_AUTODETECT) {
		EditInfo    fi;
		const CMRUFile  cMRU;
		if ( cMRU.GetEditInfo( to_tchar(filename), &fi ) ){
				nSaveCharCode = fi.m_nCharCode;
		} else {
			nSaveCharCode = GetDocument()->GetDocumentEncoding();
		}
	}

	/* �����܂ł��ĕ����R�[�h�����肵�Ȃ��Ȃ�ǂ����������� */
	if( !IsValidCodeType(nSaveCharCode) ) nSaveCharCode = CODE_SJIS;

	try{
		// �t�@�C�����J��
		cfl.FileOpen( to_tchar(filename), nSaveCharCode, 0 );

		/* �t�@�C���T�C�Y��65KB���z������i���_�C�A���O�\�� */
		if ( 0x10000 < cfl.GetFileSize() ) {
			pcDlgCancel = new CDlgCancel;
			if( NULL != ( hwndCancel = pcDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
				hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
				Progress_SetRange( hwndProgress, 0, 100 );
				Progress_SetPos( hwndProgress, 0);
			}
		}

		// ReadLine�̓t�@�C������ �����R�[�h�ϊ����ꂽ1�s��ǂݏo���܂�
		// �G���[����throw CError_FileRead �𓊂��܂�
		CNativeW cBuf;
		while( RESULT_FAILURE != cfl.ReadLine( &cBuf, &cEol ) ){

			const wchar_t*	pLine = cBuf.GetStringPtr();
			int			nLineLen = cBuf.GetStringLength();

			++nLineNum;
			Command_INSTEXT(false, pLine, CLogicInt(nLineLen), true);

			/* �i���_�C�A���O�L�� */
			if( NULL == pcDlgCancel ){
				continue;
			}
			/* �������̃��[�U�[������\�ɂ��� */
			if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
				break;
			}
			/* ���f�{�^�������`�F�b�N */
			if( pcDlgCancel->IsCanceled() ){
				break;
			}
			if( 0 == ( nLineNum & 0xFF ) ){
				Progress_SetPos( hwndProgress, cfl.GetPercent() );
				m_pCommanderView->Redraw();
			}
		}
		// �t�@�C���𖾎��I�ɕ��邪�A�����ŕ��Ȃ��Ƃ��̓f�X�g���N�^�ŕ��Ă���
		cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		WarningMessage( NULL, _T("file open error [%ls]"), filename );
		bResult = FALSE;
	}
	catch( CError_FileRead ){
		WarningMessage( NULL, _T("�t�@�C���̓ǂݍ��ݒ��ɃG���[���������܂����B") );
		bResult = FALSE;
	} // ��O�����I���

	delete pcDlgCancel;

	if (bBeforeTextSelected){	// �}�����ꂽ������I����Ԃ�
		m_pCommanderView->GetSelectionInfo().SetSelectArea(
			CLayoutRange(
				ptFrom,
				/*
				nLineFrom, nColmFrom,
				*/
				GetCaret().GetCaretLayoutPos()
				/*
				m_nCaretPosY, m_nCaretPosX
				*/
			)
		);
		m_pCommanderView->GetSelectionInfo().DrawSelectArea();
	}
	m_pCommanderView->Redraw();
	return bResult;
}
