//	$Id$
/*!	@file
	@brief �v���Z�X�ԋ��L�f�[�^�̃t�@�C���ց^����̓��o��

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, Stonee, jepro, mik, asa-o, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK
	Copyright (C) 2003, MIK
	Copyright (C) 2004, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include <stdio.h>
#include <io.h>
#include "CShareData.h"
#include "mymessage.h"
#include "debug.h"
#include "global.h"
#include "CRunningTimer.h"
#include "CProfile.h"
#include "etc_uty.h"

#define STR_COLORDATA_HEAD3		" �e�L�X�g�G�f�B�^�F�ݒ� Ver3"	//Jan. 15, 2001 Stonee  �F�ݒ�Ver3�h���t�g(�ݒ�t�@�C���̃L�[��A�ԁ��������)	//Feb. 11, 2001 JEPRO �L���ɂ���



//	CShareData_new2.cpp�Ɠ���
CShareData::CShareData()
{
//	m_pszAppName = GSTR_CSHAREDATA;
	m_hFileMap   = NULL;
	m_pShareData = NULL;
//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ�
	m_nTransformFileNameCount = -1;
	return;
}

// ���W�X�g���͎g��Ȃ��B
// ���g�p�̂Q�֐����폜 2002/2/3 aroka
// 2004-01-11 D.S.Koba CProfile�ύX�ɂ��R�[�h�ȗ���

/*!
	���L�f�[�^�̓ǂݍ���/�ۑ� 2

	@param bRead [in] true: �ǂݍ��� / false: ��������
*/
BOOL CShareData::ShareData_IO_2( bool bRead )
{
	MY_RUNNINGTIMER( cRunningTimer, "CShareData::ShareData_IO_2" );

	char			szKey[256];
	char			szKeyData[1024];
	int				i, j;
	char			szKeyName[64];
	FileInfo		fiInit;
	FileInfo*		pfiWork;
	int				nZero = 0;
	char			szIniFileName[_MAX_PATH + 1];
	CProfile		cProfile;
	const char*		pszSecName;

	char			szPath[_MAX_PATH + 1];
	char			szDrive[_MAX_DRIVE];
	char			szDir[_MAX_DIR];
	char			szFname[_MAX_FNAME];
	char			szExt[_MAX_EXT];
	const char*		pszKeyName;
	const char*		pszForm;

	/* exe�̂���t�H���_ */
	::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szPath, sizeof(szPath)
	);
	_splitpath( szPath, szDrive, szDir, szFname, szExt );
	_makepath( szIniFileName, szDrive, szDir, szFname, "ini" );


//	MYTRACE( "Ini�t�@�C������-1 ���v����(�~���b) = %d\n", cRunningTimer.Read() );


	if( bRead ){
		if( false == cProfile.ReadProfile( szIniFileName ) ){
			/* �ݒ�t�@�C�������݂��Ȃ� */
			return FALSE;
		}
#ifdef _DEBUG
//@@@ 2001.12.26 YAZAKI �f�o�b�O�ł�����ɋN�����Ȃ����߁B
//		cProfile.DUMP();
#endif

	}
//	MYTRACE( "Ini�t�@�C������ 0 ���v����(�~���b) = %d\n", cRunningTimer.Read() );

	{//	MRU
		pszSecName = "MRU";
		cProfile.IOProfileData( bRead, pszSecName, "_MRU_Counts", m_pShareData->m_nMRUArrNum );

		for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
			pfiWork = &m_pShareData->m_fiMRUArr[i];

			wsprintf( szKeyName, "MRU[%02d].nViewTopLine", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_nViewTopLine );
			wsprintf( szKeyName, "MRU[%02d].nViewLeftCol", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_nViewLeftCol );
			wsprintf( szKeyName, "MRU[%02d].nX", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_nX );
			wsprintf( szKeyName, "MRU[%02d].nY", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_nY );
			wsprintf( szKeyName, "MRU[%02d].nCharCode", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_nCharCode );
			wsprintf( szKeyName, "MRU[%02d].szPath", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_szPath, sizeof( pfiWork->m_szPath ));
			wsprintf( szKeyName, "MRU[%02d].szMark", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, pfiWork->m_szMarkLines, sizeof( pfiWork->m_szMarkLines ));
			
			//���C�ɓ���	//@@@ 2003.04.08 MIK
			wsprintf( szKeyName, "MRU[%02d].bFavorite", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_bMRUArrFavorite[i] );
		}
		//@@@ 2001.12.26 YAZAKI �c���m_fiMRUArr���������B
		if ( bRead ){
			//	�c���fiInit�ŏ��������Ă����B
			fiInit.m_nCharCode = 0;
			fiInit.m_nViewLeftCol = 0;
			fiInit.m_nViewTopLine = 0;
			fiInit.m_nX = 0;
			fiInit.m_nY = 0;
			strcpy( fiInit.m_szPath, "" );
			strcpy( fiInit.m_szMarkLines, "" );	// 2002.01.16 hor
			for( ; i < MAX_MRU; ++i){
				m_pShareData->m_fiMRUArr[i] = fiInit;
				m_pShareData->m_bMRUArrFavorite[i] = false;	//���C�ɓ���	//@@@ 2003.04.08 MIK
			}
		}

		cProfile.IOProfileData( bRead, pszSecName, "_MRUFOLDER_Counts", m_pShareData->m_nOPENFOLDERArrNum );

		for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
			wsprintf( szKeyName, "MRUFOLDER[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_szOPENFOLDERArr[i],
				sizeof( m_pShareData->m_szOPENFOLDERArr[0] ));

			//���C�ɓ���	//@@@ 2003.04.08 MIK
			strcat( szKeyName, ".bFavorite" );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_bOPENFOLDERArrFavorite[i] );
		}
		if ( bRead ){
			for (; i< MAX_OPENFOLDER; ++i){
				strcpy( m_pShareData->m_szOPENFOLDERArr[i], "" );
				m_pShareData->m_bOPENFOLDERArrFavorite[i] = false;	//���C�ɓ���	//@@@ 2003.04.08 MIK
			}
		}
	}//	MRU

	{//	Keys
		pszSecName = "Keys";
		cProfile.IOProfileData( bRead, pszSecName, "_SEARCHKEY_Counts", m_pShareData->m_nSEARCHKEYArrNum );
		for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
			wsprintf( szKeyName, "SEARCHKEY[%02d]", i );
			if( i >= m_pShareData->m_nSEARCHKEYArrNum ){
				strcpy( m_pShareData->m_szSEARCHKEYArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szSEARCHKEYArr[i], sizeof( m_pShareData->m_szSEARCHKEYArr[0] ));
		}
		cProfile.IOProfileData( bRead, pszSecName, "_REPLACEKEY_Counts", m_pShareData->m_nREPLACEKEYArrNum );
		for( i = 0; i < m_pShareData->m_nREPLACEKEYArrNum; ++i ){
			wsprintf( szKeyName, "REPLACEKEY[%02d]", i );
			if( i >= m_pShareData->m_nREPLACEKEYArrNum ){
				strcpy( m_pShareData->m_szREPLACEKEYArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szREPLACEKEYArr[i], sizeof( m_pShareData->m_szREPLACEKEYArr[0] ) );
		}
	}// Keys

	{// Grep
		pszSecName = "Grep";
		cProfile.IOProfileData( bRead, pszSecName, "_GREPFILE_Counts", m_pShareData->m_nGREPFILEArrNum );
		for( i = 0; i < m_pShareData->m_nGREPFILEArrNum; ++i ){
			wsprintf( szKeyName, "GREPFILE[%02d]", i );
			if( i >= m_pShareData->m_nGREPFILEArrNum ){
				strcpy( m_pShareData->m_szGREPFILEArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szGREPFILEArr[i], sizeof( m_pShareData->m_szGREPFILEArr[0] ));
		}
		cProfile.IOProfileData( bRead, pszSecName, "_GREPFOLDER_Counts", m_pShareData->m_nGREPFOLDERArrNum );
		for( i = 0; i < m_pShareData->m_nGREPFOLDERArrNum; ++i ){
			wsprintf( szKeyName, "GREPFOLDER[%02d]", i );
			if( i >= m_pShareData->m_nGREPFOLDERArrNum ){
				strcpy( m_pShareData->m_szGREPFOLDERArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szGREPFOLDERArr[i], sizeof( m_pShareData->m_szGREPFOLDERArr[0] ));
		}
	}// Grep

	{//	Folders
		pszSecName = "Folders";
		/* �}�N���p�t�H���_ */
		cProfile.IOProfileData( bRead, pszSecName, "szMACROFOLDER",
			m_pShareData->m_szMACROFOLDER, sizeof( m_pShareData->m_szMACROFOLDER ));
		/* �ݒ�C���|�[�g�p�t�H���_ */
		cProfile.IOProfileData( bRead, pszSecName, "szIMPORTFOLDER",
			m_pShareData->m_szIMPORTFOLDER, sizeof( m_pShareData->m_szIMPORTFOLDER ));
	}//	Folders

	{//	Cmd
		pszSecName = "Cmd";
		cProfile.IOProfileData( bRead, pszSecName, "nCmdArrNum", m_pShareData->m_nCmdArrNum );
		for( i = 0; i < m_pShareData->m_nCmdArrNum; ++i ){
			wsprintf( szKeyName, "szCmdArr[%02d]", i );
			if( i >= m_pShareData->m_nCmdArrNum ){
				strcpy( m_pShareData->m_szCmdArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szCmdArr[i], sizeof( m_pShareData->m_szCmdArr[0] ));
		}
	}//	Cmd

	{//	Nickname
		pszSecName = "Nickname";
		cProfile.IOProfileData( bRead, pszSecName, "ArrNum", m_pShareData->m_nTransformFileNameArrNum );
		for( i = 0; i < m_pShareData->m_nTransformFileNameArrNum; ++i ){
			wsprintf( szKeyName, "From%02d", i );
			if( i >= m_pShareData->m_nTransformFileNameArrNum ){
				strcpy( m_pShareData->m_szTransformFileNameFrom[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szTransformFileNameFrom[i], sizeof( m_pShareData->m_szTransformFileNameFrom[0] ));

			wsprintf( szKeyName, "To%02d", i );
			if( i >= m_pShareData->m_nTransformFileNameArrNum ){
				strcpy( m_pShareData->m_szTransformFileNameFrom[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szTransformFileNameTo[i], sizeof( m_pShareData->m_szTransformFileNameTo[0] ));
		}
		// �ǂݍ��ݎ��C�c���NULL�ōď�����
		if( bRead ){
			for( ; i < MAX_TRANSFORM_FILENAME; i++ ){
				m_pShareData->m_szTransformFileNameFrom[i][0] = '\0';
				m_pShareData->m_szTransformFileNameTo[i][0]   = '\0';
			}
		}
	}//	Nickname

	/* ���ʐݒ� */
	{
		pszSecName = "Common";

		cProfile.IOProfileData( bRead, pszSecName, "nCaretType"				, m_pShareData->m_Common.m_nCaretType );
		cProfile.IOProfileData( bRead, pszSecName, "bIsINSMode"				, m_pShareData->m_Common.m_bIsINSMode );
		cProfile.IOProfileData( bRead, pszSecName, "bIsFreeCursorMode"		, m_pShareData->m_Common.m_bIsFreeCursorMode );

		cProfile.IOProfileData( bRead, pszSecName, "bStopsBothEndsWhenSearchWord"	, m_pShareData->m_Common.m_bStopsBothEndsWhenSearchWord );
		cProfile.IOProfileData( bRead, pszSecName, "bStopsBothEndsWhenSearchParagraph"	, m_pShareData->m_Common.m_bStopsBothEndsWhenSearchParagraph );
		//	Oct. 27, 2000 genta
		cProfile.IOProfileData( bRead, pszSecName, "m_bRestoreCurPosition"	, m_pShareData->m_Common.m_bRestoreCurPosition );
		// 2002.01.16 hor
		cProfile.IOProfileData( bRead, pszSecName, "m_bRestoreBookmarks"	, m_pShareData->m_Common.m_bRestoreBookmarks );
		cProfile.IOProfileData( bRead, pszSecName, "bAddCRLFWhenCopy"		, m_pShareData->m_Common.m_bAddCRLFWhenCopy );
		cProfile.IOProfileData( bRead, pszSecName, "nRepeatedScrollLineNum"	, m_pShareData->m_Common.m_nRepeatedScrollLineNum );
		cProfile.IOProfileData( bRead, pszSecName, "nRepeatedScroll_Smooth"	, m_pShareData->m_Common.m_nRepeatedScroll_Smooth );
		cProfile.IOProfileData( bRead, pszSecName, "bExitConfirm"			, m_pShareData->m_Common.m_bExitConfirm );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchRegularExp"		, m_pShareData->m_Common.m_bRegularExp );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchLoHiCase"		, m_pShareData->m_Common.m_bLoHiCase );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchWordOnly"		, m_pShareData->m_Common.m_bWordOnly );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchNOTIFYNOTFOUND"	, m_pShareData->m_Common.m_bNOTIFYNOTFOUND );
		// 2002.01.26 hor
		cProfile.IOProfileData( bRead, pszSecName, "bSearchAll"				, m_pShareData->m_Common.m_bSearchAll );
		cProfile.IOProfileData( bRead, pszSecName, "bSearchSelectedArea"	, m_pShareData->m_Common.m_bSelectedArea );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepSubFolder"			, m_pShareData->m_Common.m_bGrepSubFolder );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepOutputLine"		, m_pShareData->m_Common.m_bGrepOutputLine );
		cProfile.IOProfileData( bRead, pszSecName, "nGrepOutputStyle"		, m_pShareData->m_Common.m_nGrepOutputStyle );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepDefaultFolder"		, m_pShareData->m_Common.m_bGrepDefaultFolder );
		// 2002/09/21 Moca �ǉ�
		cProfile.IOProfileData( bRead, pszSecName, "nGrepCharSet"			, m_pShareData->m_Common.m_nGrepCharSet );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepRealTime"			, m_pShareData->m_Common.m_bGrepRealTimeView ); // 2003.06.16 Moca

		cProfile.IOProfileData( bRead, pszSecName, "bGTJW_RETURN"			, m_pShareData->m_Common.m_bGTJW_RETURN );
		cProfile.IOProfileData( bRead, pszSecName, "bGTJW_LDBLCLK"			, m_pShareData->m_Common.m_bGTJW_LDBLCLK );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUp"				, m_pShareData->m_Common.m_bBackUp );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpDialog"			, m_pShareData->m_Common.m_bBackUpDialog );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpFolder"			, m_pShareData->m_Common.m_bBackUpFolder );

		if( !bRead ){
			int	nDummy;
			int	nCharChars;
			nDummy = strlen( m_pShareData->m_Common.m_szBackUpFolder );
			/* �t�H���_�̍Ōオ�u���p����'\\'�v�łȂ��ꍇ�́A�t������ */
			nCharChars = &m_pShareData->m_Common.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( m_pShareData->m_Common.m_szBackUpFolder, nDummy, &m_pShareData->m_Common.m_szBackUpFolder[nDummy] );
			if( 1 == nCharChars && m_pShareData->m_Common.m_szBackUpFolder[nDummy - 1] == '\\' ){
			}else{
				strcat( m_pShareData->m_Common.m_szBackUpFolder, "\\" );
			}
		}
		cProfile.IOProfileData( bRead, pszSecName, "szBackUpFolder",
			m_pShareData->m_Common.m_szBackUpFolder, sizeof( m_pShareData->m_Common.m_szBackUpFolder ));
		if( bRead ){
			int	nDummy;
			int	nCharChars;
			nDummy = strlen( m_pShareData->m_Common.m_szBackUpFolder );
			/* �t�H���_�̍Ōオ�u���p����'\\'�v�łȂ��ꍇ�́A�t������ */
			nCharChars = &m_pShareData->m_Common.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( m_pShareData->m_Common.m_szBackUpFolder, nDummy, &m_pShareData->m_Common.m_szBackUpFolder[nDummy] );
			if( 1 == nCharChars && m_pShareData->m_Common.m_szBackUpFolder[nDummy - 1] == '\\' ){
			}else{
				strcat( m_pShareData->m_Common.m_szBackUpFolder, "\\" );
			}
		}


		cProfile.IOProfileData( bRead, pszSecName, "nBackUpType"				, m_pShareData->m_Common.m_nBackUpType );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt1"			, m_pShareData->m_Common.m_nBackUpType_Opt1 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt2"			, m_pShareData->m_Common.m_nBackUpType_Opt2 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpType2_Opt3"			, m_pShareData->m_Common.m_nBackUpType_Opt3 );
		cProfile.IOProfileData( bRead, pszSecName, "bBackUpDustBox"				, m_pShareData->m_Common.m_bBackUpDustBox );	//@@@ 2001.12.11 add MIK
		cProfile.IOProfileData( bRead, pszSecName, "nFileShareMode"				, m_pShareData->m_Common.m_nFileShareMode );
		cProfile.IOProfileData( bRead, pszSecName, "szExtHelp",
			m_pShareData->m_Common.m_szExtHelp, sizeof( m_pShareData->m_Common.m_szExtHelp ));
		cProfile.IOProfileData( bRead, pszSecName, "szExtHtmlHelp",
			m_pShareData->m_Common.m_szExtHtmlHelp, sizeof( m_pShareData->m_Common.m_szExtHtmlHelp ));
	
		cProfile.IOProfileData( bRead, pszSecName, "szMigemoDll",
			m_pShareData->m_Common.m_szMigemoDll, sizeof( m_pShareData->m_Common.m_szMigemoDll ));
		cProfile.IOProfileData( bRead, pszSecName, "szMigemoDict",
			m_pShareData->m_Common.m_szMigemoDict, sizeof( m_pShareData->m_Common.m_szMigemoDict ));

		// ai 02/05/23 Add S
		{// Keword Help Font
			pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
			if( bRead ){
				if( true == cProfile.IOProfileData( bRead, pszSecName, "khlf", szKeyData, sizeof( szKeyData )) ){
					sscanf( szKeyData, pszForm,
						&m_pShareData->m_Common.m_lf_kh.lfHeight,
						&m_pShareData->m_Common.m_lf_kh.lfWidth,
						&m_pShareData->m_Common.m_lf_kh.lfEscapement,
						&m_pShareData->m_Common.m_lf_kh.lfOrientation,
						&m_pShareData->m_Common.m_lf_kh.lfWeight,
						&m_pShareData->m_Common.m_lf_kh.lfItalic,
						&m_pShareData->m_Common.m_lf_kh.lfUnderline,
						&m_pShareData->m_Common.m_lf_kh.lfStrikeOut,
						&m_pShareData->m_Common.m_lf_kh.lfCharSet,
						&m_pShareData->m_Common.m_lf_kh.lfOutPrecision,
						&m_pShareData->m_Common.m_lf_kh.lfClipPrecision,
						&m_pShareData->m_Common.m_lf_kh.lfQuality,
						&m_pShareData->m_Common.m_lf_kh.lfPitchAndFamily
					);
				}
			}else{
				wsprintf( szKeyData, pszForm,
					m_pShareData->m_Common.m_lf_kh.lfHeight,
					m_pShareData->m_Common.m_lf_kh.lfWidth,
					m_pShareData->m_Common.m_lf_kh.lfEscapement,
					m_pShareData->m_Common.m_lf_kh.lfOrientation,
					m_pShareData->m_Common.m_lf_kh.lfWeight,
					m_pShareData->m_Common.m_lf_kh.lfItalic,
					m_pShareData->m_Common.m_lf_kh.lfUnderline,
					m_pShareData->m_Common.m_lf_kh.lfStrikeOut,
					m_pShareData->m_Common.m_lf_kh.lfCharSet,
					m_pShareData->m_Common.m_lf_kh.lfOutPrecision,
					m_pShareData->m_Common.m_lf_kh.lfClipPrecision,
					m_pShareData->m_Common.m_lf_kh.lfQuality,
					m_pShareData->m_Common.m_lf_kh.lfPitchAndFamily
				);
				cProfile.IOProfileData( bRead, pszSecName, "khlf", szKeyData, 0 );
			}

			cProfile.IOProfileData( bRead, pszSecName, "khlfFaceName"			, m_pShareData->m_Common.m_lf_kh.lfFaceName, 0 );

		}// Keword Help Font
		// ai 02/05/23 Add S


		cProfile.IOProfileData( bRead, pszSecName, "nMRUArrNum_MAX"				, m_pShareData->m_Common.m_nMRUArrNum_MAX );
		cProfile.IOProfileData( bRead, pszSecName, "nOPENFOLDERArrNum_MAX"		, m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX );
		cProfile.IOProfileData( bRead, pszSecName, "bDispTOOLBAR"				, m_pShareData->m_Common.m_bDispTOOLBAR );
		cProfile.IOProfileData( bRead, pszSecName, "bDispSTATUSBAR"				, m_pShareData->m_Common.m_bDispSTATUSBAR );
		cProfile.IOProfileData( bRead, pszSecName, "bDispFUNCKEYWND"			, m_pShareData->m_Common.m_bDispFUNCKEYWND );
		cProfile.IOProfileData( bRead, pszSecName, "nFUNCKEYWND_Place"			, m_pShareData->m_Common.m_nFUNCKEYWND_Place );
		cProfile.IOProfileData( bRead, pszSecName, "nFUNCKEYWND_GroupNum"		, m_pShareData->m_Common.m_nFUNCKEYWND_GroupNum );		// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����

		cProfile.IOProfileData( bRead, pszSecName, "bDispTabWnd"				, m_pShareData->m_Common.m_bDispTabWnd );	//�^�u�E�C���h�E	//@@@ 2003.05.31 MIK
		cProfile.IOProfileData( bRead, pszSecName, "bDispTabWndMultiWin"		, m_pShareData->m_Common.m_bDispTabWndMultiWin );	//�^�u�E�C���h�E	//@@@ 2003.05.31 MIK
		cProfile.IOProfileData( bRead, pszSecName, "szTabWndCaption"			, m_pShareData->m_Common.m_szTabWndCaption		, MAX_CAPTION_CONF_LEN );	//@@@ 2003.06.13 MIK

		// 2001/06/20 asa-o �����E�B���h�E�̃X�N���[���̓������Ƃ�
		cProfile.IOProfileData( bRead, pszSecName, "bSplitterWndHScroll"		, m_pShareData->m_Common.m_bSplitterWndHScroll );
		cProfile.IOProfileData( bRead, pszSecName, "bSplitterWndVScroll"		, m_pShareData->m_Common.m_bSplitterWndVScroll );

		cProfile.IOProfileData( bRead, pszSecName, "szMidashiKigou"				, m_pShareData->m_Common.m_szMidashiKigou, 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szInyouKigou"				, m_pShareData->m_Common.m_szInyouKigou, 0 );

		// 2001/06/14 asa-o �⊮�ƃL�[���[�h�w���v�̓^�C�v�ʂɈړ������̂ō폜�F�R�s
		cProfile.IOProfileData( bRead, pszSecName, "bUseHokan"					, m_pShareData->m_Common.m_bUseHokan );
		// 2002/09/21 Moca bGrepKanjiCode_AutoDetect �� bGrepCharSet�ɓ��������̂ō폜
		// 2001/06/19 asa-o �^�C�v�ʂɈړ������̂ō폜�F1�s
		cProfile.IOProfileData( bRead, pszSecName, "bSaveWindowSize"			, m_pShareData->m_Common.m_nSaveWindowSize );
		cProfile.IOProfileData( bRead, pszSecName, "nWinSizeType"				, m_pShareData->m_Common.m_nWinSizeType );
		cProfile.IOProfileData( bRead, pszSecName, "nWinSizeCX"					, m_pShareData->m_Common.m_nWinSizeCX );
		cProfile.IOProfileData( bRead, pszSecName, "nWinSizeCY"					, m_pShareData->m_Common.m_nWinSizeCY );
		// 2004.03.30 Moca *nWinPos*��ǉ�
		cProfile.IOProfileData( bRead, pszSecName, "nSaveWindowPos"			, m_pShareData->m_Common.m_nSaveWindowPos );
		cProfile.IOProfileData( bRead, pszSecName, "nWinPosX"				, m_pShareData->m_Common.m_nWinPosX );
		cProfile.IOProfileData( bRead, pszSecName, "nWinPosY"				, m_pShareData->m_Common.m_nWinPosY );
		cProfile.IOProfileData( bRead, pszSecName, "bTaskTrayUse"				, m_pShareData->m_Common.m_bUseTaskTray );
		cProfile.IOProfileData( bRead, pszSecName, "bTaskTrayStay"				, m_pShareData->m_Common.m_bStayTaskTray );
//@@@ 2002.01.08 YAZAKI �^�X�N�g���C���u�g��Ȃ��v�ɂ��Ă��풓���`�F�b�N���c���Ă���ƍċN���Łu�g���E�풓�v�ɂȂ�o�O�C��
#if 0
		if( bRead ){
			/* �^�X�N�g���C�ɏ풓����Ƃ��́A�K���^�X�N�g���C�A�C�R�����g�� */
			if( m_pShareData->m_Common.m_bStayTaskTray ){
				m_pShareData->m_Common.m_bUseTaskTray = TRUE;
			}
		}
#endif
		cProfile.IOProfileData( bRead, pszSecName, "wTrayMenuHotKeyCode"		, m_pShareData->m_Common.m_wTrayMenuHotKeyCode );
		cProfile.IOProfileData( bRead, pszSecName, "wTrayMenuHotKeyMods"		, m_pShareData->m_Common.m_wTrayMenuHotKeyMods );
		cProfile.IOProfileData( bRead, pszSecName, "bUseOLE_DragDrop"			, m_pShareData->m_Common.m_bUseOLE_DragDrop );
		cProfile.IOProfileData( bRead, pszSecName, "bUseOLE_DropSource"			, m_pShareData->m_Common.m_bUseOLE_DropSource );
		cProfile.IOProfileData( bRead, pszSecName, "bDispExitingDialog"			, m_pShareData->m_Common.m_bDispExitingDialog );
		cProfile.IOProfileData( bRead, pszSecName, "bEnableUnmodifiedOverwrite"	, m_pShareData->m_Common.m_bEnableUnmodifiedOverwrite );
		cProfile.IOProfileData( bRead, pszSecName, "bSelectClickedURL"			, m_pShareData->m_Common.m_bSelectClickedURL );
		cProfile.IOProfileData( bRead, pszSecName, "bGrepExitConfirm"			, m_pShareData->m_Common.m_bGrepExitConfirm );/* Grep���[�h�ŕۑ��m�F���邩 */
//		cProfile.IOProfileData( bRead, pszSecName, "bRulerDisp"					, m_pShareData->m_Common.m_bRulerDisp );/* ���[���[�\�� */
		cProfile.IOProfileData( bRead, pszSecName, "nRulerHeight"				, m_pShareData->m_Common.m_nRulerHeight );/* ���[���[���� */
		cProfile.IOProfileData( bRead, pszSecName, "nRulerBottomSpace"			, m_pShareData->m_Common.m_nRulerBottomSpace );/* ���[���[�ƃe�L�X�g�̌��� */
		cProfile.IOProfileData( bRead, pszSecName, "nRulerType"					, m_pShareData->m_Common.m_nRulerType );/* ���[���[�̃^�C�v */
		//	Sep. 18, 2002 genta �ǉ�
		cProfile.IOProfileData( bRead, pszSecName, "nLineNumberRightSpace"		, m_pShareData->m_Common.m_nLineNumRightSpace );/* �s�ԍ��̉E���̌��� */
		cProfile.IOProfileData( bRead, pszSecName, "bCopyAndDisablSelection"	, m_pShareData->m_Common.m_bCopyAndDisablSelection );/* �R�s�[������I������ */
		cProfile.IOProfileData( bRead, pszSecName, "bHtmlHelpIsSingle"			, m_pShareData->m_Common.m_bHtmlHelpIsSingle );/* HtmlHelp�r���[�A�͂ЂƂ� */
		cProfile.IOProfileData( bRead, pszSecName, "bCompareAndTileHorz"		, m_pShareData->m_Common.m_bCompareAndTileHorz );/* ������r��A���E�ɕ��ׂĕ\�� */	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕s�v�̂͂�
		cProfile.IOProfileData( bRead, pszSecName, "bDropFileAndClose"			, m_pShareData->m_Common.m_bDropFileAndClose );/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
		cProfile.IOProfileData( bRead, pszSecName, "nDropFileNumMax"			, m_pShareData->m_Common.m_nDropFileNumMax );/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
		cProfile.IOProfileData( bRead, pszSecName, "bCheckFileTimeStamp"		, m_pShareData->m_Common.m_bCheckFileTimeStamp );/* �X�V�̊Ď� */
		cProfile.IOProfileData( bRead, pszSecName, "bNotOverWriteCRLF"			, m_pShareData->m_Common.m_bNotOverWriteCRLF );/* ���s�͏㏑�����Ȃ� */
		cProfile.IOProfileData( bRead, pszSecName, "bAutoCloseDlgFind"			, m_pShareData->m_Common.m_bAutoCloseDlgFind );/* �����_�C�A���O�������I�ɕ��� */
		cProfile.IOProfileData( bRead, pszSecName, "bAutoCloseDlgFuncList"		, m_pShareData->m_Common.m_bAutoCloseDlgFuncList );/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
		cProfile.IOProfileData( bRead, pszSecName, "bAutoCloseDlgReplace"		, m_pShareData->m_Common.m_bAutoCloseDlgReplace );/* �u�� �_�C�A���O�������I�ɕ��� */
		cProfile.IOProfileData( bRead, pszSecName, "bAutoColmnPaste"			, m_pShareData->m_Common.m_bAutoColmnPaste );/* ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t�� */
		cProfile.IOProfileData( bRead, pszSecName, "bScrollBarHorz"				, m_pShareData->m_Common.m_bScrollBarHorz );/* �����X�N���[���o�[���g�� */

		cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_RETURN"			, m_pShareData->m_Common.m_bHokanKey_RETURN );/* VK_RETURN �⊮����L�[���L��/���� */
		cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_TAB"				, m_pShareData->m_Common.m_bHokanKey_TAB );/* VK_TAB    �⊮����L�[���L��/���� */
		cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_RIGHT"			, m_pShareData->m_Common.m_bHokanKey_RIGHT );/* VK_RIGHT  �⊮����L�[���L��/���� */
		cProfile.IOProfileData( bRead, pszSecName, "bHokanKey_SPACE"			, m_pShareData->m_Common.m_bHokanKey_SPACE );/* VK_SPACE  �⊮����L�[���L��/���� */

		cProfile.IOProfileData( bRead, pszSecName, "nDateFormatType"			, m_pShareData->m_Common.m_nDateFormatType );/* ���t�����̃^�C�v */
		cProfile.IOProfileData( bRead, pszSecName, "szDateFormat"				, m_pShareData->m_Common.m_szDateFormat, 0 );//���t����
		cProfile.IOProfileData( bRead, pszSecName, "nTimeFormatType"			, m_pShareData->m_Common.m_nTimeFormatType );/* ���������̃^�C�v */
		cProfile.IOProfileData( bRead, pszSecName, "szTimeFormat"				, m_pShareData->m_Common.m_szTimeFormat, 0 );//��������

		cProfile.IOProfileData( bRead, pszSecName, "bMenuIcon"					, m_pShareData->m_Common.m_bMenuIcon );//���j���[�ɃA�C�R����\������
		cProfile.IOProfileData( bRead, pszSecName, "bAutoMIMEdecode"			, m_pShareData->m_Common.m_bAutoMIMEdecode );//�t�@�C���ǂݍ��ݎ���MIME��decode���s����
		cProfile.IOProfileData( bRead, pszSecName, "bQueryIfCodeChange"			, m_pShareData->m_Common.m_bQueryIfCodeChange );//	Oct. 03, 2004 genta �O��ƈقȂ镶���R�[�h�̂Ƃ��ɖ₢���킹���s����
		cProfile.IOProfileData( bRead, pszSecName, "bAlertIfFileNotExist"		, m_pShareData->m_Common.m_bQueryIfCodeChange );// Oct. 09, 2004 genta �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������



		/* �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu */
		pszKeyName = "rcOpenDialog";
		pszForm = "%d,%d,%d,%d";
		if( bRead ){
			if( true == cProfile.IOProfileData( bRead, pszSecName, pszKeyName, szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, pszForm,
					&m_pShareData->m_Common.m_rcOpenDialog.left,
					&m_pShareData->m_Common.m_rcOpenDialog.top,
					&m_pShareData->m_Common.m_rcOpenDialog.right,
					&m_pShareData->m_Common.m_rcOpenDialog.bottom
				);
			}
		}else{
			wsprintf( szKeyData, pszForm,
				m_pShareData->m_Common.m_rcOpenDialog.left,
				m_pShareData->m_Common.m_rcOpenDialog.top,
				m_pShareData->m_Common.m_rcOpenDialog.right,
				m_pShareData->m_Common.m_rcOpenDialog.bottom
			);
			cProfile.IOProfileData( bRead, pszSecName, pszKeyName, szKeyData, 0 );
		}

		//2002.02.08 aroka,hor
		cProfile.IOProfileData( bRead, pszSecName, "bMarkUpBlankLineEnable"		, m_pShareData->m_Common.m_bMarkUpBlankLineEnable );
		cProfile.IOProfileData( bRead, pszSecName, "bFunclistSetFocusOnJump"	, m_pShareData->m_Common.m_bFunclistSetFocusOnJump );

		//	Apr. 05, 2003 genta �E�B���h�E�L���v�V�����̃J�X�^�}�C�Y
		cProfile.IOProfileData( bRead, pszSecName, "szWinCaptionActive"
			, m_pShareData->m_Common.m_szWindowCaptionActive, MAX_CAPTION_CONF_LEN );
		cProfile.IOProfileData( bRead, pszSecName, "szWinCaptionInactive"
			, m_pShareData->m_Common.m_szWindowCaptionInactive, MAX_CAPTION_CONF_LEN );

	}// Common
	
	/* �c�[���o�[ */
	{
		pszSecName = "Toolbar";
		cProfile.IOProfileData( bRead, pszSecName, "nToolBarButtonNum", m_pShareData->m_Common.m_nToolBarButtonNum );
		cProfile.IOProfileData( bRead, pszSecName, "bToolBarIsFlat", m_pShareData->m_Common.m_bToolBarIsFlat );
		for( i = 0; i < m_pShareData->m_Common.m_nToolBarButtonNum; ++i ){
			wsprintf( szKeyName, "nTBB[%03d]", i );
			if( i < m_pShareData->m_Common.m_nToolBarButtonNum ){
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_Common.m_nToolBarButtonIdxArr[i] );
			}else{
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, nZero );
			}
		}
	}// Tool bar

	/* �J�X�^�����j���[ */
	{
		pszSecName = "CustMenu";
		for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
			wsprintf( szKeyName, "szCMN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_Common.m_szCustMenuNameArr[i], MAX_CUSTOM_MENU_NAME_LEN + 1 );	//	Oct. 15, 2001 genta �ő咷�w��
			wsprintf( szKeyName, "nCMIN[%02d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_Common.m_nCustMenuItemNumArr[i] );
			for( j = 0; j < m_pShareData->m_Common.m_nCustMenuItemNumArr[i]; ++j ){
				wsprintf( szKeyName, "nCMIF[%02d][%02d]", i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_Common.m_nCustMenuItemFuncArr[i][j] );
				wsprintf( szKeyName, "nCMIK[%02d][%02d]", i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_Common.m_nCustMenuItemKeyArr[i][j] );
			}
		}
	}// Custom menu

	{// Font
		pszSecName = "Font";
		pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
		if( bRead ){
			if( true == cProfile.IOProfileData( bRead, pszSecName, "lf", szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, pszForm,
					&m_pShareData->m_Common.m_lf.lfHeight,
					&m_pShareData->m_Common.m_lf.lfWidth,
					&m_pShareData->m_Common.m_lf.lfEscapement,
					&m_pShareData->m_Common.m_lf.lfOrientation,
					&m_pShareData->m_Common.m_lf.lfWeight,
					&m_pShareData->m_Common.m_lf.lfItalic,
					&m_pShareData->m_Common.m_lf.lfUnderline,
					&m_pShareData->m_Common.m_lf.lfStrikeOut,
					&m_pShareData->m_Common.m_lf.lfCharSet,
					&m_pShareData->m_Common.m_lf.lfOutPrecision,
					&m_pShareData->m_Common.m_lf.lfClipPrecision,
					&m_pShareData->m_Common.m_lf.lfQuality,
					&m_pShareData->m_Common.m_lf.lfPitchAndFamily
				);
			}
		}else{
			wsprintf( szKeyData, pszForm,
				m_pShareData->m_Common.m_lf.lfHeight,
				m_pShareData->m_Common.m_lf.lfWidth,
				m_pShareData->m_Common.m_lf.lfEscapement,
				m_pShareData->m_Common.m_lf.lfOrientation,
				m_pShareData->m_Common.m_lf.lfWeight,
				m_pShareData->m_Common.m_lf.lfItalic,
				m_pShareData->m_Common.m_lf.lfUnderline,
				m_pShareData->m_Common.m_lf.lfStrikeOut,
				m_pShareData->m_Common.m_lf.lfCharSet,
				m_pShareData->m_Common.m_lf.lfOutPrecision,
				m_pShareData->m_Common.m_lf.lfClipPrecision,
				m_pShareData->m_Common.m_lf.lfQuality,
				m_pShareData->m_Common.m_lf.lfPitchAndFamily
			);
			cProfile.IOProfileData( bRead, pszSecName, "lf", szKeyData, 0 );
		}


		cProfile.IOProfileData( bRead, pszSecName, "lfFaceName",
			m_pShareData->m_Common.m_lf.lfFaceName, sizeof( m_pShareData->m_Common.m_lf.lfFaceName ));

		cProfile.IOProfileData( bRead, pszSecName, "bFontIs_FIXED_PITCH", m_pShareData->m_Common.m_bFontIs_FIXED_PITCH );
	}//	Font

	/* �L�[���蓖�� */
	{
		pszSecName = "KeyBind";

		for( i = 0; i < m_pShareData->m_nKeyNameArrNum; ++i ){

			strcpy( szKeyName, m_pShareData->m_pKeyNameArr[i].m_szKeyName );

			if( bRead ){
				if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName,
					szKeyData, sizeof( szKeyData )) ){
					sscanf( szKeyData, "%d,%d,%d,%d,%d,%d,%d,%d",
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[0],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[1],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[2],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[3],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[4],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[5],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[6],
						&m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[7]
					 );
				}
			}else{
				wsprintf( szKeyData, "%d,%d,%d,%d,%d,%d,%d,%d",
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[0],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[1],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[2],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[3],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[4],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[5],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[6],
					m_pShareData->m_pKeyNameArr[i].m_nFuncCodeArr[7]
				);
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
			}
		}
	}// KeyBind

	/* ����ݒ� */
	{
		pszSecName = "Print";
		for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
			wsprintf( szKeyName, "PS[%02d].nInts", i );
			static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
			if( bRead ){
				if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName,
					szKeyData, sizeof( szKeyData ) ) ){
					sscanf( szKeyData, pszForm,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintFontHeight		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintDansuu			,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintDanSpace		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintLineSpacing		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginTY		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginBY		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginLX		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintMarginRX		,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintPaperOrientation,
						&m_pShareData->m_PrintSettingArr[i].m_nPrintPaperSize		,
						&m_pShareData->m_PrintSettingArr[i].m_bPrintWordWrap		,
						&m_pShareData->m_PrintSettingArr[i].m_bPrintLineNumber		,
						&m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[0]			,
						&m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[1]			,
						&m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[2]			,
						&m_pShareData->m_PrintSettingArr[i].m_bFooterUse[0]			,
						&m_pShareData->m_PrintSettingArr[i].m_bFooterUse[1]			,
						&m_pShareData->m_PrintSettingArr[i].m_bFooterUse[2]

					 );
				}
			}else{
				wsprintf( szKeyData, pszForm,
					m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth		,
					m_pShareData->m_PrintSettingArr[i].m_nPrintFontHeight		,
					m_pShareData->m_PrintSettingArr[i].m_nPrintDansuu			,
					m_pShareData->m_PrintSettingArr[i].m_nPrintDanSpace			,
					m_pShareData->m_PrintSettingArr[i].m_nPrintLineSpacing		,
					m_pShareData->m_PrintSettingArr[i].m_nPrintMarginTY			,
					m_pShareData->m_PrintSettingArr[i].m_nPrintMarginBY			,
					m_pShareData->m_PrintSettingArr[i].m_nPrintMarginLX			,
					m_pShareData->m_PrintSettingArr[i].m_nPrintMarginRX			,
					m_pShareData->m_PrintSettingArr[i].m_nPrintPaperOrientation	,
					m_pShareData->m_PrintSettingArr[i].m_nPrintPaperSize		,
					m_pShareData->m_PrintSettingArr[i].m_bPrintWordWrap			,
					m_pShareData->m_PrintSettingArr[i].m_bPrintLineNumber		,
					m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[0]			,
					m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[1]			,
					m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[2]			,
					m_pShareData->m_PrintSettingArr[i].m_bFooterUse[0]			,
					m_pShareData->m_PrintSettingArr[i].m_bFooterUse[1]			,
					m_pShareData->m_PrintSettingArr[i].m_bFooterUse[2]
				);
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
			}


			wsprintf( szKeyName, "PS[%02d].szSName"	, i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName	,
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintSettingName ));
			wsprintf( szKeyName, "PS[%02d].szFF"	, i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceHan	,
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceHan ));
			wsprintf( szKeyName, "PS[%02d].szFFZ"	, i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceZen	,
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceZen ));
			for( j = 0; j < 3; ++j ){
				wsprintf( szKeyName, "PS[%02d].szHF[%d]" , i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName,
					m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[j],
					sizeof( m_pShareData->m_PrintSettingArr[0].m_szHeaderForm [0] ));
				wsprintf( szKeyName, "PS[%02d].szFTF[%d]", i, j );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName,
					m_pShareData->m_PrintSettingArr[i].m_szFooterForm[j],
					sizeof( m_pShareData->m_PrintSettingArr[0].m_szFooterForm[0] ));
			}
			wsprintf( szKeyName, "PS[%02d].szDriver", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterDriverName,
				sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDriverName ));
			wsprintf( szKeyName, "PS[%02d].szDevice", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterDeviceName, 
				sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDeviceName ));
			wsprintf( szKeyName, "PS[%02d].szOutput", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterOutputName,
				sizeof( m_pShareData->m_PrintSettingArr[i].m_mdmDevMode.m_szPrinterOutputName ));

			// 2002.02.16 hor �Ƃ肠�������ݒ��ϊ����Ƃ�
			if(0==strcmp(m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[0],"&f") &&
			   0==strcmp(m_pShareData->m_PrintSettingArr[i].m_szFooterForm[0],"&C- &P -")
			){
				strcpy( m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[0], "$f" );
				strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[0], "" );
				strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[1], "- $p -" );
			}

			//�֑�	//@@@ 2002.04.09 MIK
			wsprintf( szKeyName, "PS[%02d].bKinsokuHead", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuHead );
			wsprintf( szKeyName, "PS[%02d].bKinsokuTail", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuTail );
			wsprintf( szKeyName, "PS[%02d].bKinsokuRet",  i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuRet );	//@@@ 2002.04.13 MIK
			wsprintf( szKeyName, "PS[%02d].bKinsokuKuto", i ); cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_PrintSettingArr[i].m_bPrintKinsokuKuto );	//@@@ 2002.04.17 MIK

		}
	}// Print

	/* �^�C�v�ʐݒ� */
	for( i = 0; i < MAX_TYPES; ++i ){
		wsprintf( szKey, "Types(%d)", i );
		pszSecName = szKey;
		static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";	//MIK
		strcpy( szKeyName, "nInts" );
		if( bRead ){
			if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, pszForm,
					&m_pShareData->m_Types[i].m_nIdx,
					&m_pShareData->m_Types[i].m_nMaxLineSize,
					&m_pShareData->m_Types[i].m_nColmSpace,
					&m_pShareData->m_Types[i].m_nTabSpace,
					&m_pShareData->m_Types[i].m_nKeyWordSetIdx[0],
					&m_pShareData->m_Types[i].m_nKeyWordSetIdx[1],	//MIK
					&m_pShareData->m_Types[i].m_nStringType,
					&m_pShareData->m_Types[i].m_bLineNumIsCRLF,
					&m_pShareData->m_Types[i].m_nLineTermType,
					&m_pShareData->m_Types[i].m_bWordWrap,
					&m_pShareData->m_Types[i].m_nCurrentPrintSetting
				 );
			}
		}else{
			wsprintf( szKeyData, pszForm,
				m_pShareData->m_Types[i].m_nIdx,
				m_pShareData->m_Types[i].m_nMaxLineSize,
				m_pShareData->m_Types[i].m_nColmSpace,
				m_pShareData->m_Types[i].m_nTabSpace,
				m_pShareData->m_Types[i].m_nKeyWordSetIdx[0],
				m_pShareData->m_Types[i].m_nKeyWordSetIdx[1],	//MIK
				m_pShareData->m_Types[i].m_nStringType,
				m_pShareData->m_Types[i].m_bLineNumIsCRLF,
				m_pShareData->m_Types[i].m_nLineTermType,
				m_pShareData->m_Types[i].m_bWordWrap,
				m_pShareData->m_Types[i].m_nCurrentPrintSetting
			);
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
		}
		// 2005.01.13 MIK Keywordset 3-10
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect3",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[2] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect4",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[3] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect5",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[4] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect6",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[5] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect7",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[6] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect8",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[7] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect9",  m_pShareData->m_Types[i].m_nKeyWordSetIdx[8] );
		cProfile.IOProfileData( bRead, pszSecName, "nKeywordSelect10", m_pShareData->m_Types[i].m_nKeyWordSetIdx[9] );

		/* �s�Ԃ̂����� */
		if( !bRead ){
			if( m_pShareData->m_Types[i].m_nLineSpace < /* 1 */ 0 ){
				m_pShareData->m_Types[i].m_nLineSpace = /* 1 */ 0;
			}
			if( m_pShareData->m_Types[i].m_nLineSpace > LINESPACE_MAX ){ // Feb. 18, 2003 genta �ő�l�̒萔��
				m_pShareData->m_Types[i].m_nLineSpace = LINESPACE_MAX;
			}
		}
		cProfile.IOProfileData( bRead, pszSecName, "nLineSpace", m_pShareData->m_Types[i].m_nLineSpace );
		if( bRead ){
			if( m_pShareData->m_Types[i].m_nLineSpace < /* 1 */ 0 ){
				m_pShareData->m_Types[i].m_nLineSpace = /* 1 */ 0;
			}
			if( m_pShareData->m_Types[i].m_nLineSpace > 16 ){
				m_pShareData->m_Types[i].m_nLineSpace = 16;
			}
		}


		cProfile.IOProfileData( bRead, pszSecName, "szTypeName",
			m_pShareData->m_Types[i].m_szTypeName,
			sizeof( m_pShareData->m_Types[0].m_szTypeName ));
		cProfile.IOProfileData( bRead, pszSecName, "szTypeExts",
			m_pShareData->m_Types[i].m_szTypeExts,
			sizeof( m_pShareData->m_Types[0].m_szTypeExts ));
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
		cProfile.IOProfileData( bRead, pszSecName, "szTabViewString",
		/*&*/m_pShareData->m_Types[i].m_szTabViewString,
		sizeof( m_pShareData->m_Types[i].m_szTabViewString ));
//#endif
		cProfile.IOProfileData( bRead, pszSecName, "bTabArrow"			, m_pShareData->m_Types[i].m_bTabArrow );	//@@@ 2003.03.26 MIK
		cProfile.IOProfileData( bRead, pszSecName, "bInsSpace"			, m_pShareData->m_Types[i].m_bInsSpace );	// 2001.12.03 hor

		// From Here Sep. 28, 2002 genta / YAZAKI
		if( bRead ){
			//	Block Comment
			char buffer[2][ BLOCKCOMMENT_BUFFERSIZE ];
			//	2004.10.02 Moca �΂ɂȂ�R�����g�ݒ肪�Ƃ��ɓǂݍ��܂ꂽ�Ƃ������L���Ȑݒ�ƌ��Ȃ��D
			//	�u���b�N�R�����g�̎n�܂�ƏI���D�s�R�����g�̋L���ƌ��ʒu
			bool bRet1, bRet2;
			buffer[0][0] = buffer[1][0] = '\0';
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom"	,
				buffer[0], BLOCKCOMMENT_BUFFERSIZE );			
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo"	,
				buffer[1], BLOCKCOMMENT_BUFFERSIZE );
			if( bRet1 && bRet2 ) m_pShareData->m_Types[i].m_cBlockComment.CopyTo( 0, buffer[0], buffer[1] );

			//@@@ 2001.03.10 by MIK
			buffer[0][0] = buffer[1][0] = '\0';
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom2",
				buffer[0], BLOCKCOMMENT_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo2"	,
				buffer[1], BLOCKCOMMENT_BUFFERSIZE );
			if( bRet1 && bRet2 ) m_pShareData->m_Types[i].m_cBlockComment.CopyTo( 1, buffer[0], buffer[1] );
			
			//	Line Comment
			char lbuf[ COMMENT_DELIMITER_BUFFERSIZE ];
			int  pos;

			lbuf[0] = '\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szLineComment"		,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn"	, pos );
			if( bRet1 && bRet2 ) m_pShareData->m_Types[i].m_cLineComment.CopyTo( 0, lbuf, pos );

			lbuf[0] = '\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szLineComment2"		,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn2", pos );
			if( bRet1 && bRet2 ) m_pShareData->m_Types[i].m_cLineComment.CopyTo( 1, lbuf, pos );

			lbuf[0] = '\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( bRead, pszSecName, "szLineComment3"		,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );	//Jun. 01, 2001 JEPRO �ǉ�
			bRet2 = cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn3", pos );	//Jun. 01, 2001 JEPRO �ǉ�
			if( bRet1 && bRet2 ) m_pShareData->m_Types[i].m_cLineComment.CopyTo( 2, lbuf, pos );
		}
		else { // write
			//	Block Comment
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom"	,
				const_cast<char*>(m_pShareData->m_Types[i].m_cBlockComment.getBlockCommentFrom( 0 )), 0 );
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo"	,
				const_cast<char*>(m_pShareData->m_Types[i].m_cBlockComment.getBlockCommentTo( 0 )), 0 );

			//@@@ 2001.03.10 by MIK
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentFrom2",
				const_cast<char*>(m_pShareData->m_Types[i].m_cBlockComment.getBlockCommentFrom( 1 )), 0 );
			cProfile.IOProfileData( bRead, pszSecName, "szBlockCommentTo2"	,
				const_cast<char*>(m_pShareData->m_Types[i].m_cBlockComment.getBlockCommentTo(1)), 0 );

			//	Line Comment
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment"		,
			const_cast<char*>(m_pShareData->m_Types[i].m_cLineComment.getLineComment( 0 )), 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment2"		,
			const_cast<char*>(m_pShareData->m_Types[i].m_cLineComment.getLineComment( 1 )), 0 );
		cProfile.IOProfileData( bRead, pszSecName, "szLineComment3"		,
			const_cast<char*>(m_pShareData->m_Types[i].m_cLineComment.getLineComment( 2 )), 0 );	//Jun. 01, 2001 JEPRO �ǉ�

		//	From here May 12, 2001 genta
		int pos;
		pos = m_pShareData->m_Types[i].m_cLineComment.getLineCommentPos( 0 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn"	, pos );
		pos = m_pShareData->m_Types[i].m_cLineComment.getLineCommentPos( 1 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn2", pos );
		pos = m_pShareData->m_Types[i].m_cLineComment.getLineCommentPos( 2 );
		cProfile.IOProfileData( bRead, pszSecName, "nLineCommentColumn3", pos );	//Jun. 01, 2001 JEPRO �ǉ�
		//	To here May 12, 2001 genta

		}
		// To Here Sep. 28, 2002 genta / YAZAKI

		cProfile.IOProfileData( bRead, pszSecName, "szIndentChars"		,
			m_pShareData->m_Types[i].m_szIndentChars,
			sizeof( m_pShareData->m_Types[0].m_szIndentChars ));
		cProfile.IOProfileData( bRead, pszSecName, "cLineTermChar"		, m_pShareData->m_Types[i].m_cLineTermChar );

		cProfile.IOProfileData( bRead, pszSecName, "nDefaultOutline"	, m_pShareData->m_Types[i].m_nDefaultOutline );/* �A�E�g���C����͕��@ */
		cProfile.IOProfileData( bRead, pszSecName, "szOutlineRuleFilename"	,
			m_pShareData->m_Types[i].m_szOutlineRuleFilename,
			sizeof( m_pShareData->m_Types[0].m_szOutlineRuleFilename ));/* �A�E�g���C����̓��[���t�@�C�� */
		cProfile.IOProfileData( bRead, pszSecName, "nSmartIndent"		, m_pShareData->m_Types[i].m_nSmartIndent );/* �X�}�[�g�C���f���g��� */
		//	Nov. 20, 2000 genta
		cProfile.IOProfileData( bRead, pszSecName, "nImeState"			, m_pShareData->m_Types[i].m_nImeState );	//	IME����

		//	2001/06/14 Start By asa-o: �^�C�v�ʂ̕⊮�t�@�C���ƃL�[���[�h�w���v
		//	Oct. 5, 2002 genta sizeof()�Ō���ă|�C���^�̃T�C�Y���擾���Ă����̂��C��
		cProfile.IOProfileData( bRead, pszSecName, "szHokanFile"		,
			m_pShareData->m_Types[i].m_szHokanFile,
			sizeof( m_pShareData->m_Types[0].m_szHokanFile ));		//	�⊮�t�@�C��
		cProfile.IOProfileData( bRead, pszSecName, "bUseKeyWordHelp"	, m_pShareData->m_Types[i].m_bUseKeyWordHelp );	//	�L�[���[�h�w���v���g�p����
		cProfile.IOProfileData( bRead, pszSecName, "szKeyWordHelpFile"	, 
			m_pShareData->m_Types[i].m_szKeyWordHelpFile,
			sizeof( m_pShareData->m_Types[0].m_szKeyWordHelpFile ));	//	�L�[���[�h�w���v �����t�@�C��
		//	2001/06/14 End

		//	2001/06/19 asa-o
		cProfile.IOProfileData( bRead, pszSecName, "bHokanLoHiCase"		, m_pShareData->m_Types[i].m_bHokanLoHiCase );

		//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
		cProfile.IOProfileData( bRead, pszSecName, "bUseHokanByFile"		, m_pShareData->m_Types[i].m_bUseHokanByFile );

		//@@@ 2002.2.4 YAZAKI
		cProfile.IOProfileData( bRead, pszSecName, "szExtHelp"			,
			m_pShareData->m_Types[i].m_szExtHelp,
			sizeof( m_pShareData->m_Types[0].m_szExtHelp ));
			
		cProfile.IOProfileData( bRead, pszSecName, "szExtHtmlHelp"		,
			m_pShareData->m_Types[i].m_szExtHtmlHelp,
			sizeof( m_pShareData->m_Types[i].m_szExtHtmlHelp ));
		cProfile.IOProfileData( bRead, pszSecName, "bHtmlHelpIsSingle"	, m_pShareData->m_Types[i].m_bHokanLoHiCase );

		cProfile.IOProfileData( bRead, pszSecName, "bAutoIndent"			, m_pShareData->m_Types[i].m_bAutoIndent );
		cProfile.IOProfileData( bRead, pszSecName, "bAutoIndent_ZENSPACE"	, m_pShareData->m_Types[i].m_bAutoIndent_ZENSPACE );
		cProfile.IOProfileData( bRead, pszSecName, "nIndentLayout"			, m_pShareData->m_Types[i].m_nIndentLayout );

		/* �F�ݒ� I/O */
		IO_ColorSet( &cProfile, bRead, pszSecName, m_pShareData->m_Types[i].m_ColorInfoArr  );


//@@@ 2001.11.17 add start MIK
		{	//���K�\���L�[���[�h
			char	*p;
			cProfile.IOProfileData( bRead, pszSecName, "bUseRegexKeyword", m_pShareData->m_Types[i].m_bUseRegexKeyword );/* ���K�\���L�[���[�h�g�p���邩�H */
			for(j = 0; j < 100; j++)
			{
				wsprintf( szKeyName, "RxKey[%03d]", j );
				if( bRead )
				{
					m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_szKeyword[0] = '\0';
					m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
					if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) )
					{
						p = strchr(szKeyData, ',');
						if( p )
						{
							*p = '\0';
							m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex = GetColorIndexByName(szKeyData);	//@@@ 2002.04.30
							if( m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex == -1 )	//���O�łȂ�
								m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex = atoi(szKeyData);
							p++;
							strcpy(m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_szKeyword, p);
							if( m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex < 0
							 || m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex >= COLORIDX_LAST )
							{
								m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
							}
						}
					}
				}
				else
				// 2002.02.08 hor ����`�l�𖳎�
				if(lstrlen(m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_szKeyword))
				{
					//wsprintf( szKeyData, "%d,%s",
					//	m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex,
					//	m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_szKeyword);
					wsprintf( szKeyData, "%s,%s",
						GetColorNameByIndex( m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_nColorIndex ),
						m_pShareData->m_Types[i].m_RegexKeywordArr[j].m_szKeyword);
					cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
				}
			}
		}
//@@@ 2001.11.17 add end MIK

		/* �֑� */
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuHead"	, m_pShareData->m_Types[i].m_bKinsokuHead );
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuTail"	, m_pShareData->m_Types[i].m_bKinsokuTail );
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuRet"	, m_pShareData->m_Types[i].m_bKinsokuRet );	//@@@ 2002.04.13 MIK
		cProfile.IOProfileData( bRead, pszSecName, "bKinsokuKuto"	, m_pShareData->m_Types[i].m_bKinsokuKuto );	//@@@ 2002.04.17 MIK
		cProfile.IOProfileData( bRead, pszSecName, "szKinsokuHead"	,
			m_pShareData->m_Types[i].m_szKinsokuHead,
			sizeof( m_pShareData->m_Types[0].m_szKinsokuHead ));
		cProfile.IOProfileData( bRead, pszSecName, "szKinsokuTail"	,
			m_pShareData->m_Types[i].m_szKinsokuTail,
			sizeof( m_pShareData->m_Types[0].m_szKinsokuTail ));
		cProfile.IOProfileData( bRead, pszSecName, "bUseDocumentIcon"	, m_pShareData->m_Types[i].m_bUseDocumentIcon );	// Sep. 19 ,2002 genta �ϐ������C��

	}// Types ( for loop)

	/* �����L�[���[�h */
	{
		pszSecName = "KeyWords";
		int nKeyWordSetNum = m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum;
		bool bIOSuccess;
		cProfile.IOProfileData( bRead, pszSecName, "nCurrentKeyWordSetIdx"	, m_pShareData->m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
		bIOSuccess = cProfile.IOProfileData( bRead, pszSecName, "nKeyWordSetNum"			, nKeyWordSetNum );
		if( bRead ){
			// nKeyWordSetNum ���ǂݍ��߂Ă���΁A���ׂĂ̏�񂪂�����Ă���Ɖ��肵�ď�����i�߂�
			if( bIOSuccess ){
				// 2004.11.25 Moca �L�[���[�h�Z�b�g�̏��́A���ڏ��������Ȃ��Ŋ֐��𗘗p����
				// �����ݒ肳��Ă��邽�߁A��ɍ폜���Ȃ��ƌŒ胁�����̊m�ۂɎ��s����\��������
				int  nMemLen = MAX_KEYWORDNUM * ( MAX_KEYWORDLEN + 1 ) + 1;
				char *pszMem = new char[nMemLen];
				m_pShareData->m_CKeyWordSetMgr.ResetAllKeyWordSet();
				for( i = 0; i < nKeyWordSetNum; ++i ){
					int nKEYWORDCASE = 0;
					int nKeyWordNum = 0;
					wsprintf( szKeyName, "szSN[%02d]", i );
					cProfile.IOProfileData( bRead, pszSecName, szKeyName, szKeyData, sizeof( szKeyData ));
					wsprintf( szKeyName, "nCASE[%02d]", i );
					cProfile.IOProfileData( bRead, pszSecName, szKeyName, nKEYWORDCASE );
					wsprintf( szKeyName, "nKWN[%02d]", i );
					cProfile.IOProfileData( bRead, pszSecName, szKeyName, nKeyWordNum );
					m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( szKeyData, nKEYWORDCASE, nKeyWordNum );
					// 2004.11.25 Moca szKW[%02d].Size �̏��͗��p����Ӗ����Ȃ��B
//					wsprintf( szKeyName, "szKW[%02d].Size", i );
//					cProfile.IOProfileData( bRead, pszSecName, szKeyName, nMemLen );
					wsprintf( szKeyName, "szKW[%02d]", i );
					if( true == cProfile.IOProfileData( bRead, pszSecName, szKeyName, pszMem, nMemLen ) ){
						m_pShareData->m_CKeyWordSetMgr.SetKeyWordArr( i, nKeyWordNum, pszMem );
					}
				}
				delete [] pszMem;
			}
		}else{
			for( i = 0; i < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
				wsprintf( szKeyName, "szSN[%02d]", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName,
					m_pShareData->m_CKeyWordSetMgr.m_szSetNameArr[i],
					sizeof( m_pShareData->m_CKeyWordSetMgr.m_szSetNameArr[0] ));
				wsprintf( szKeyName, "nCASE[%02d]", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_CKeyWordSetMgr.m_nKEYWORDCASEArr[i] );
				wsprintf( szKeyName, "nKWN[%02d]", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i] );

				int nMemLen = 0;
				for( j = 0; j < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i]; ++j ){
					nMemLen += strlen( m_pShareData->m_CKeyWordSetMgr.GetKeyWord( i, j ) );
					nMemLen ++;
				}
				nMemLen ++;
				wsprintf( szKeyName, "szKW[%02d].Size", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, nMemLen );
				char* pszMem = new char[nMemLen + 1];	//	May 25, 2003 genta ��؂��TAB�ɕύX�����̂ŁC�Ō��\0�̕���ǉ�
				char* pMem = pszMem;
				for( j = 0; j < m_pShareData->m_CKeyWordSetMgr.m_nKeyWordNumArr[i]; ++j ){
					//	May 25, 2003 genta ��؂��TAB�ɕύX
					int kwlen = strlen( m_pShareData->m_CKeyWordSetMgr.GetKeyWord( i, j ) );
					memcpy( pMem, m_pShareData->m_CKeyWordSetMgr.GetKeyWord( i, j ), kwlen );
					pMem += kwlen;
					*pMem++ = '\t';
				}
				*pMem = '\0';
				wsprintf( szKeyName, "szKW[%02d]", i );
				cProfile.IOProfileData( bRead, pszSecName, szKeyName, pszMem, nMemLen );
				delete [] pszMem;
			}
		}
	}// Keywords
	
	//	From Here Sep. 14, 2001 genta
	//	Macro
	{
		pszSecName = "Macro";
		
		for( i = 0; i < MAX_CUSTMACRO; ++i ){
			//	Oct. 4, 2001 genta ���܂�Ӗ����Ȃ������Ȃ̂ō폜�F3�s
			// 2002.02.08 hor ����`�l�𖳎�
			if( !bRead && !lstrlen(m_pShareData->m_MacroTable[i].m_szName) && !lstrlen(m_pShareData->m_MacroTable[i].m_szFile) ) continue;
			wsprintf( szKeyName, "Name[%03d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_MacroTable[i].m_szName, MACRONAME_MAX - 1 );
			wsprintf( szKeyName, "File[%03d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_MacroTable[i].m_szFile, _MAX_PATH );
			wsprintf( szKeyName, "ReloadWhenExecute[%03d]", i );
			cProfile.IOProfileData( bRead, pszSecName, szKeyName, m_pShareData->m_MacroTable[i].m_bReloadWhenExecute );
		}
	}// Macro
	//	To Here  Sep. 14, 2001 genta

//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
	{
		pszSecName = "Other";	//�Z�N�V������1�쐬�����B2003.05.12 MIK
		/* **** ���̑��̃_�C�A���O **** */
		/* �O���R�}���h���s�́u�W���o�͂𓾂�v */
		cProfile.IOProfileData( bRead, pszSecName, "bGetStdout"		, m_pShareData->m_bGetStdout );
		/* �w��s�փW�����v�́u���s�P�ʂ̍s�ԍ��v���u�܂�Ԃ��P�ʂ̍s�ԍ��v�� */
		cProfile.IOProfileData( bRead, pszSecName, "bLineNumIsCRLF"	, m_pShareData->m_bLineNumIsCRLF );

		/* DIFF�����\�� */	//@@@ 2002.05.27 MIK
		cProfile.IOProfileData( bRead, pszSecName, "nDiffFlgOpt"	, m_pShareData->m_nDiffFlgOpt );
	
		/* CTAGS */	//@@@ 2003.05.12 MIK
		cProfile.IOProfileData( bRead, pszSecName, "nTagsOpt"		, m_pShareData->m_nTagsOpt );
		cProfile.IOProfileData( bRead, pszSecName, "szTagsCmdLine"	, m_pShareData->m_szTagsCmdLine, sizeof( m_pShareData->m_szTagsCmdLine ) );

		//From Here 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v
		cProfile.IOProfileData( bRead, pszSecName, "_TagJumpKeyword_Counts", m_pShareData->m_nTagJumpKeywordArrNum );
		for( i = 0; i < m_pShareData->m_nTagJumpKeywordArrNum; ++i ){
			wsprintf( szKeyName, "TagJumpKeyword[%02d]", i );
			if( i >= m_pShareData->m_nTagJumpKeywordArrNum ){
				strcpy( m_pShareData->m_szTagJumpKeywordArr[i], "" );
			}
			cProfile.IOProfileData( bRead, pszSecName, szKeyName,
				m_pShareData->m_szTagJumpKeywordArr[i], sizeof( m_pShareData->m_szTagJumpKeywordArr[0] ));
		}
		cProfile.IOProfileData( bRead, pszSecName, "m_bTagJumpICase"		, m_pShareData->m_bTagJumpICase );
		cProfile.IOProfileData( bRead, pszSecName, "m_bTagJumpAnyWhere"		, m_pShareData->m_bTagJumpAnyWhere );
		//From Here 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v��

		//	MIK �o�[�W�������i�������݂̂݁j
		if( ! bRead ){
			TCHAR	iniVer[256];
			wsprintf( iniVer, _T("%d.%d.%d.%d"), 
						HIWORD( m_pShareData->m_dwProductVersionMS ),
						LOWORD( m_pShareData->m_dwProductVersionMS ),
						HIWORD( m_pShareData->m_dwProductVersionLS ),
						LOWORD( m_pShareData->m_dwProductVersionLS ) );
			cProfile.IOProfileData( bRead, pszSecName, _T("szVersion")	, iniVer, sizeof( iniVer ) );
		}
	}

	if( !bRead ){
		cProfile.WriteProfile( szIniFileName, " sakura.ini �e�L�X�g�G�f�B�^�ݒ�t�@�C��" );
	}

//	MYTRACE( "Ini�t�@�C������ 8 ���v����(�~���b) = %d\n", cRunningTimer.Read() );
//	MYTRACE( "Ini�t�@�C������ ���v����(�~���b) = %d\n", cRunningTimerStart.Read() );

	return TRUE;
}


/*!
	@brief �F�ݒ� I/O

	�w�肳�ꂽ�F�ݒ���w�肳�ꂽ�Z�N�V�����ɏ������ށB�܂���
	�w�肳�ꂽ�Z�N�V�������炢��ݒ��ǂݍ��ށB

	@param pcProfile [i/o] �����o���A�ǂݍ��ݐ�Profile object (���o�͕�����bRead�Ɉˑ�)
	@param bRead [in] true: �ǂݏo�� / false: ��������
	@param pszSecName [in] �Z�N�V������
	@param pColorInfoArr [i/o] �����o���A�ǂݍ��ݑΏۂ̐F�ݒ�ւ̃|�C���^ (���o�͕�����bRead�Ɉˑ�)
*/
void CShareData::IO_ColorSet( CProfile* pcProfile, bool bRead, const char* pszSecName, ColorInfo* pColorInfoArr )
{
	char	szKeyName[256];
	char	szKeyData[1024];
	int		j;
	for( j = 0; j < COLORIDX_LAST; ++j ){
		static const char* pszForm = "%d,%d,%06x,%06x,%d";
#ifndef STR_COLORDATA_HEAD3
		wsprintf( szKeyName, "CI[%02d]", j );
#else
		wsprintf( szKeyName, "C[%s]", colorIDXKeyName[j] );	//Stonee, 2001/01/12, 2001/01/15
#endif
		if( bRead ){
			if( true == pcProfile->IOProfileData( bRead, pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
				pColorInfoArr[j].m_bUnderLine = FALSE;
				sscanf( szKeyData, pszForm,
					&pColorInfoArr[j].m_bDisp,
					&pColorInfoArr[j].m_bFatFont,
					&pColorInfoArr[j].m_colTEXT,
					&pColorInfoArr[j].m_colBACK,
					&pColorInfoArr[j].m_bUnderLine
				 );
			}
		}else{
			wsprintf( szKeyData, pszForm,
				pColorInfoArr[j].m_bDisp,
				pColorInfoArr[j].m_bFatFont,
				pColorInfoArr[j].m_colTEXT,
				pColorInfoArr[j].m_colBACK,
				pColorInfoArr[j].m_bUnderLine
			);
			pcProfile->IOProfileData( bRead, pszSecName, szKeyName, szKeyData, 0 );
		}
	}
	return;
}


/*!
	@brief �^�O�W�����v���̕ۑ�

	�^�O�W�����v����Ƃ��ɁA�^�O�W�����v��̏���ۑ�����B

	@param  pTagJump [in] �ۑ�����^�O�W�����v���
	@return true : �ۑ����� false : �ۑ����s

	@date 2004/06/21 �V�K�쐬
	@date 2004/06/22 Moca ��t�ɂȂ������ԌÂ������폜�������ɐV������������
*/
void CShareData::PushTagJump(const TagJump *pTagJump)
{
	int i = m_pShareData->m_TagJumpTop + 1;
	if( MAX_TAGJUMPNUM <= i ){
		i = 0;
	}
	if( m_pShareData->m_TagJumpNum < MAX_TAGJUMPNUM ){
		m_pShareData->m_TagJumpNum++;
	}
	m_pShareData->m_TagJump[i] = *pTagJump;
	m_pShareData->m_TagJumpTop = i;
}


/*!
	@brief �^�O�W�����v���̎Q��

	�^�O�W�����v�o�b�N����Ƃ��ɁA�^�O�W�����v���̏����Q�Ƃ���B

	@param  pTagJump [out] �Q�Ƃ���^�O�W�����v���
	@return true : �Q�Ɛ��� false : �Q�Ǝ��s

	@date 2004/06/21 �V�K�쐬
	@date 2004/06/22 Moca SetTagJump�ύX�ɂ��C��
*/
bool CShareData::PopTagJump(TagJump *pTagJump)
{
	if( 0 < m_pShareData->m_TagJumpNum ){
		*pTagJump = m_pShareData->m_TagJump[m_pShareData->m_TagJumpTop--];
		if( m_pShareData->m_TagJumpTop < 0 ){
			m_pShareData->m_TagJumpTop = MAX_TAGJUMPNUM - 1;
		}
		m_pShareData->m_TagJumpNum--;
		return true;
	}
	return false;
}


/*[EOF]*/
