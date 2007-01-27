/*!	@file
	@brief �v���Z�X�ԋ��L�f�[�^�̃t�@�C���ց^����̓��o��

	@author Norio Nakatani

	@date 2005.04.07 D.S.Koba �e�Z�N�V�����̓��o�͂��֐��Ƃ��ĕ���
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, jepro
	Copyright (C) 2001, genta, Stonee, jepro, MIK, asa-o, YAZAKI, hor
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, genta, Moca
	Copyright (C) 2003, MIK, genta, Moca
	Copyright (C) 2004, MIK, Moca, D.S.Koba, genta
	Copyright (C) 2005, MIK, genta, D.S.Koba, ryoji, aroka, Moca
	Copyright (C) 2006, aroka, ryoji, D.S.Koba, fon
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
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

/*!
	���L�f�[�^�̓ǂݍ���/�ۑ� 2

	@param[in] bRead true: �ǂݍ��� / false: ��������

	@date 2004-01-11 D.S.Koba CProfile�ύX�ɂ��R�[�h�ȗ���
	@date 2005-04-05 D.S.Koba �e�Z�N�V�����̓��o�͂��֐��Ƃ��ĕ���
*/
BOOL CShareData::ShareData_IO_2( bool bRead )
{
	MY_RUNNINGTIMER( cRunningTimer, "CShareData::ShareData_IO_2" );

	char		szIniFileName[_MAX_PATH + 1];
	CProfile	cProfile;
	char		szPath[_MAX_PATH + 1];
	char		szDrive[_MAX_DRIVE];
	char		szDir[_MAX_DIR];
	char		szFname[_MAX_FNAME];
	char		szExt[_MAX_EXT];

	// Feb. 12, 2006 D.S.Koba
	if( bRead ){
		cProfile.SetReadingMode();
	} else {
		cProfile.SetWritingMode();
	}

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
	
	// Feb. 12, 2006 D.S.Koba
	ShareData_IO_Mru( cProfile );
	ShareData_IO_Keys( cProfile );
	ShareData_IO_Grep( cProfile );
	ShareData_IO_Folders( cProfile );
	ShareData_IO_Cmd( cProfile );
	ShareData_IO_Nickname( cProfile );
	ShareData_IO_Common( cProfile );
	ShareData_IO_Toolbar( cProfile );
	ShareData_IO_CustMenu( cProfile );
	ShareData_IO_Font( cProfile );
	ShareData_IO_KeyBind( cProfile );
	ShareData_IO_Print( cProfile );
	ShareData_IO_Types( cProfile );
	ShareData_IO_KeyWords( cProfile );
	ShareData_IO_Macro( cProfile );
	ShareData_IO_Other( cProfile );
	
	if( !bRead ){
		cProfile.WriteProfile( szIniFileName, " sakura.ini �e�L�X�g�G�f�B�^�ݒ�t�@�C��" );
	}

//	MYTRACE( "Ini�t�@�C������ 8 ���v����(�~���b) = %d\n", cRunningTimer.Read() );
//	MYTRACE( "Ini�t�@�C������ ���v����(�~���b) = %d\n", cRunningTimerStart.Read() );

	return TRUE;
}

/*!
	@brief ���L�f�[�^��Mru�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B�ǂݍ��ݎ��̏��������C��
*/
void CShareData::ShareData_IO_Mru( CProfile& cProfile )
{
	const char* pszSecName = "MRU";
	int			i;
	int			nSize;
	FileInfo*	pfiWork;
	char		szKeyName[64];

	cProfile.IOProfileData( pszSecName, "_MRU_Counts", m_pShareData->m_nMRUArrNum );
	nSize = m_pShareData->m_nMRUArrNum;
	for( i = 0; i < nSize; ++i ){
		pfiWork = &m_pShareData->m_fiMRUArr[i];
		wsprintf( szKeyName, "MRU[%02d].nViewTopLine", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nViewTopLine );
		wsprintf( szKeyName, "MRU[%02d].nViewLeftCol", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nViewLeftCol );
		wsprintf( szKeyName, "MRU[%02d].nX", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nX );
		wsprintf( szKeyName, "MRU[%02d].nY", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nY );
		wsprintf( szKeyName, "MRU[%02d].nCharCode", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_nCharCode );
		wsprintf( szKeyName, "MRU[%02d].szPath", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_szPath, sizeof( pfiWork->m_szPath ));
		wsprintf( szKeyName, "MRU[%02d].szMark", i );
		cProfile.IOProfileData( pszSecName, szKeyName, pfiWork->m_szMarkLines, sizeof( pfiWork->m_szMarkLines ));
		//���C�ɓ���	//@@@ 2003.04.08 MIK
		wsprintf( szKeyName, "MRU[%02d].bFavorite", i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_bMRUArrFavorite[i] );
	}
	//@@@ 2001.12.26 YAZAKI �c���m_fiMRUArr���������B
	if ( cProfile.IsReadingMode() ){
		FileInfo	fiInit;
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

	cProfile.IOProfileData( pszSecName, "_MRUFOLDER_Counts", m_pShareData->m_nOPENFOLDERArrNum );
	nSize = m_pShareData->m_nOPENFOLDERArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "MRUFOLDER[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_szOPENFOLDERArr[i],
			sizeof( m_pShareData->m_szOPENFOLDERArr[0] ));
		//���C�ɓ���	//@@@ 2003.04.08 MIK
		strcat( szKeyName, ".bFavorite" );
		cProfile.IOProfileData( pszSecName, szKeyName, m_pShareData->m_bOPENFOLDERArrFavorite[i] );
	}
	//�ǂݍ��ݎ��͎c���������
	if ( cProfile.IsReadingMode() ){
		for (; i< MAX_OPENFOLDER; ++i){
			// 2005.04.05 D.S.Koba
			m_pShareData->m_szOPENFOLDERArr[i][0] = '\0';
			m_pShareData->m_bOPENFOLDERArrFavorite[i] = false;	//���C�ɓ���	//@@@ 2003.04.08 MIK
		}
	}
}

/*!
	@brief ���L�f�[�^��Keys�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B�ǂݍ��ݎ��̏��������C��
*/
void CShareData::ShareData_IO_Keys( CProfile& cProfile )
{
	const char* pszSecName = "Keys";
	int		i;
	int		nSize;
	char	szKeyName[64];

	cProfile.IOProfileData( pszSecName, "_SEARCHKEY_Counts", m_pShareData->m_nSEARCHKEYArrNum );
	nSize = m_pShareData->m_nSEARCHKEYArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "SEARCHKEY[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szSEARCHKEYArr[i], sizeof( m_pShareData->m_szSEARCHKEYArr[0] ));
	}
	//�ǂݍ��ݎ��͎c���������
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_SEARCHKEY; ++i){
			m_pShareData->m_szSEARCHKEYArr[i][0] = '\0';
		}
	}

	cProfile.IOProfileData( pszSecName, "_REPLACEKEY_Counts", m_pShareData->m_nREPLACEKEYArrNum );
	nSize = m_pShareData->m_nREPLACEKEYArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "REPLACEKEY[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szREPLACEKEYArr[i], sizeof( m_pShareData->m_szREPLACEKEYArr[0] ) );
	}
	//�ǂݍ��ݎ��͎c���������
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_REPLACEKEY; ++i){
			m_pShareData->m_szREPLACEKEYArr[i][0] = '\0';
		}
	}
}

/*!
	@brief ���L�f�[�^��Grep�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B�ǂݍ��ݎ��̏��������C��
*/
void CShareData::ShareData_IO_Grep( CProfile& cProfile )
{
	const char* pszSecName = "Grep";
	int		i;
	int		nSize;
	char	szKeyName[64];

	cProfile.IOProfileData( pszSecName, "_GREPFILE_Counts", m_pShareData->m_nGREPFILEArrNum );
	nSize = m_pShareData->m_nGREPFILEArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "GREPFILE[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szGREPFILEArr[i], sizeof( m_pShareData->m_szGREPFILEArr[0] ));
	}
	//�ǂݍ��ݎ��͎c���������
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_GREPFILE; ++i){
			m_pShareData->m_szGREPFILEArr[i][0] = '\0';
		}
	}

	cProfile.IOProfileData( pszSecName, "_GREPFOLDER_Counts", m_pShareData->m_nGREPFOLDERArrNum );
	nSize = m_pShareData->m_nGREPFOLDERArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "GREPFOLDER[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szGREPFOLDERArr[i], sizeof( m_pShareData->m_szGREPFOLDERArr[0] ));
	}
	//�ǂݍ��ݎ��͎c���������
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_GREPFOLDER; ++i){
			m_pShareData->m_szGREPFOLDERArr[i][0] = '\0';
		}
	}
}

/*!
	@brief ���L�f�[�^��Folders�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Folders( CProfile& cProfile )
{
	const char* pszSecName = "Folders";
	/* �}�N���p�t�H���_ */
	cProfile.IOProfileData( pszSecName, "szMACROFOLDER",
		m_pShareData->m_szMACROFOLDER, sizeof( m_pShareData->m_szMACROFOLDER ));
	/* �ݒ�C���|�[�g�p�t�H���_ */
	cProfile.IOProfileData( pszSecName, "szIMPORTFOLDER",
		m_pShareData->m_szIMPORTFOLDER, sizeof( m_pShareData->m_szIMPORTFOLDER ));
}

/*!
	@brief ���L�f�[�^��Cmd�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B�ǂݍ��ݎ��̏��������C��
*/
void CShareData::ShareData_IO_Cmd( CProfile& cProfile )
{
	const char* pszSecName = "Cmd";
	int		i;
	char	szKeyName[64];

	cProfile.IOProfileData( pszSecName, "nCmdArrNum", m_pShareData->m_nCmdArrNum );
	int nSize = m_pShareData->m_nCmdArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "szCmdArr[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szCmdArr[i], sizeof( m_pShareData->m_szCmdArr[0] ));
	}
	//�ǂݍ��ݎ��͎c���������
	if( cProfile.IsReadingMode() ){
		for(; i < MAX_CMDARR; ++i){
			m_pShareData->m_szCmdArr[i][0] = '\0';
		}
	}
}

/*!
	@brief ���L�f�[�^��Nickname�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B�ǂݍ��ݎ��̏��������C��
*/
void CShareData::ShareData_IO_Nickname( CProfile& cProfile )
{
	const char* pszSecName = "Nickname";
	int		i;
	char	szKeyName[64];

	cProfile.IOProfileData( pszSecName, "ArrNum", m_pShareData->m_nTransformFileNameArrNum );
	int nSize = m_pShareData->m_nTransformFileNameArrNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "From%02d", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szTransformFileNameFrom[i], sizeof( m_pShareData->m_szTransformFileNameFrom[0] ));
		wsprintf( szKeyName, "To%02d", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szTransformFileNameTo[i], sizeof( m_pShareData->m_szTransformFileNameTo[0] ));
	}
	// �ǂݍ��ݎ��C�c���NULL�ōď�����
	if( cProfile.IsReadingMode() ){
		for( ; i < MAX_TRANSFORM_FILENAME; i++ ){
			m_pShareData->m_szTransformFileNameFrom[i][0] = '\0';
			m_pShareData->m_szTransformFileNameTo[i][0]   = '\0';
		}
	}
}

/*!
	@brief ���L�f�[�^��Common�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Common( CProfile& cProfile )
{
	const char* pszSecName = "Common";
	// 2005.04.07 D.S.Koba
	Common& common = m_pShareData->m_Common;

	cProfile.IOProfileData( pszSecName, "nCaretType"				, common.m_nCaretType );
	//	Oct. 2, 2005 genta
	//	�����l��}�����[�h�ɌŒ肷�邽�߁C�ݒ�̓ǂݏ�������߂�
	//cProfile.IOProfileData( pszSecName, "bIsINSMode"				, common.m_bIsINSMode );
	cProfile.IOProfileData( pszSecName, "bIsFreeCursorMode"		, common.m_bIsFreeCursorMode );
	
	cProfile.IOProfileData( pszSecName, "bStopsBothEndsWhenSearchWord"	, common.m_bStopsBothEndsWhenSearchWord );
	cProfile.IOProfileData( pszSecName, "bStopsBothEndsWhenSearchParagraph"	, common.m_bStopsBothEndsWhenSearchParagraph );
	//	Oct. 27, 2000 genta
	cProfile.IOProfileData( pszSecName, "m_bRestoreCurPosition"	, common.m_bRestoreCurPosition );
	// 2002.01.16 hor
	cProfile.IOProfileData( pszSecName, "m_bRestoreBookmarks"	, common.m_bRestoreBookmarks );
	cProfile.IOProfileData( pszSecName, "bAddCRLFWhenCopy"		, common.m_bAddCRLFWhenCopy );
	cProfile.IOProfileData( pszSecName, "nRepeatedScrollLineNum"	, common.m_nRepeatedScrollLineNum );
	cProfile.IOProfileData( pszSecName, "nRepeatedScroll_Smooth"	, common.m_nRepeatedScroll_Smooth );
	cProfile.IOProfileData( pszSecName, "bCloseAllConfirm"		, common.m_bCloseAllConfirm );	/* [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F���� */	// 2006.12.25 ryoji
	cProfile.IOProfileData( pszSecName, "bExitConfirm"			, common.m_bExitConfirm );
	cProfile.IOProfileData( pszSecName, "bSearchRegularExp"		, common.m_bRegularExp );
	cProfile.IOProfileData( pszSecName, "bSearchLoHiCase"		, common.m_bLoHiCase );
	cProfile.IOProfileData( pszSecName, "bSearchWordOnly"		, common.m_bWordOnly );
	cProfile.IOProfileData( pszSecName, "bSearchConsecutiveAll"		, common.m_bConsecutiveAll );	// 2007.01.16 ryoji
	cProfile.IOProfileData( pszSecName, "bSearchNOTIFYNOTFOUND"	, common.m_bNOTIFYNOTFOUND );
	// 2002.01.26 hor
	cProfile.IOProfileData( pszSecName, "bSearchAll"				, common.m_bSearchAll );
	cProfile.IOProfileData( pszSecName, "bSearchSelectedArea"	, common.m_bSelectedArea );
	cProfile.IOProfileData( pszSecName, "bGrepSubFolder"			, common.m_bGrepSubFolder );
	cProfile.IOProfileData( pszSecName, "bGrepOutputLine"		, common.m_bGrepOutputLine );
	cProfile.IOProfileData( pszSecName, "nGrepOutputStyle"		, common.m_nGrepOutputStyle );
	cProfile.IOProfileData( pszSecName, "bGrepDefaultFolder"		, common.m_bGrepDefaultFolder );
	// 2002/09/21 Moca �ǉ�
	cProfile.IOProfileData( pszSecName, "nGrepCharSet"			, common.m_nGrepCharSet );
	cProfile.IOProfileData( pszSecName, "bGrepRealTime"			, common.m_bGrepRealTimeView ); // 2003.06.16 Moca
	
	cProfile.IOProfileData( pszSecName, "bGTJW_RETURN"			, common.m_bGTJW_RETURN );
	cProfile.IOProfileData( pszSecName, "bGTJW_LDBLCLK"			, common.m_bGTJW_LDBLCLK );
	cProfile.IOProfileData( pszSecName, "bBackUp"				, common.m_bBackUp );
	cProfile.IOProfileData( pszSecName, "bBackUpDialog"			, common.m_bBackUpDialog );
	cProfile.IOProfileData( pszSecName, "bBackUpFolder"			, common.m_bBackUpFolder );
	
	if( !cProfile.IsReadingMode() ){
		int	nDummy;
		int	nCharChars;
		nDummy = strlen( common.m_szBackUpFolder );
		/* �t�H���_�̍Ōオ�u���p����'\\'�v�łȂ��ꍇ�́A�t������ */
		nCharChars = &common.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( common.m_szBackUpFolder, nDummy, &common.m_szBackUpFolder[nDummy] );
		if( 1 == nCharChars && common.m_szBackUpFolder[nDummy - 1] == '\\' ){
		}else{
			strcat( common.m_szBackUpFolder, "\\" );
		}
	}
	cProfile.IOProfileData( pszSecName, "szBackUpFolder",
		common.m_szBackUpFolder, sizeof( common.m_szBackUpFolder ));
	if( cProfile.IsReadingMode() ){
		int	nDummy;
		int	nCharChars;
		nDummy = strlen( common.m_szBackUpFolder );
		/* �t�H���_�̍Ōオ�u���p����'\\'�v�łȂ��ꍇ�́A�t������ */
		nCharChars = &common.m_szBackUpFolder[nDummy] - CMemory::MemCharPrev( common.m_szBackUpFolder, nDummy, &common.m_szBackUpFolder[nDummy] );
		if( 1 == nCharChars && common.m_szBackUpFolder[nDummy - 1] == '\\' ){
		}else{
			strcat( common.m_szBackUpFolder, "\\" );
		}
	}
	
	
	cProfile.IOProfileData( pszSecName, "nBackUpType"			, common.m_nBackUpType );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt1"		, common.m_nBackUpType_Opt1 );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt2"		, common.m_nBackUpType_Opt2 );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt3"		, common.m_nBackUpType_Opt3 );
	cProfile.IOProfileData( pszSecName, "bBackUpType2_Opt4"		, common.m_nBackUpType_Opt4 );
	cProfile.IOProfileData( pszSecName, "bBackUpDustBox"			, common.m_bBackUpDustBox );	//@@@ 2001.12.11 add MIK
	cProfile.IOProfileData( pszSecName, "bBackUpPathAdvanced"	, common.m_bBackUpPathAdvanced );	/* 20051107 aroka */
	cProfile.IOProfileData( pszSecName, "szBackUpPathAdvanced"	,
		common.m_szBackUpPathAdvanced, sizeof( common.m_szBackUpPathAdvanced ));	/* 20051107 aroka */
	cProfile.IOProfileData( pszSecName, "nFileShareMode"			, common.m_nFileShareMode );
	cProfile.IOProfileData( pszSecName, "szExtHelp",
		common.m_szExtHelp, sizeof( common.m_szExtHelp ));
	cProfile.IOProfileData( pszSecName, "szExtHtmlHelp",
		common.m_szExtHtmlHelp, sizeof( common.m_szExtHtmlHelp ));
	
	cProfile.IOProfileData( pszSecName, "szMigemoDll",
		common.m_szMigemoDll, sizeof( common.m_szMigemoDll ));
	cProfile.IOProfileData( pszSecName, "szMigemoDict",
		common.m_szMigemoDict, sizeof( common.m_szMigemoDict ));
	
	// ai 02/05/23 Add S
	{// Keword Help Font
		const char*	pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
		char		szKeyData[1024];
		if( cProfile.IsReadingMode() ){
			if( true == cProfile.IOProfileData( pszSecName, "khlf", szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, pszForm,
					&common.m_lf_kh.lfHeight,
					&common.m_lf_kh.lfWidth,
					&common.m_lf_kh.lfEscapement,
					&common.m_lf_kh.lfOrientation,
					&common.m_lf_kh.lfWeight,
					&common.m_lf_kh.lfItalic,
					&common.m_lf_kh.lfUnderline,
					&common.m_lf_kh.lfStrikeOut,
					&common.m_lf_kh.lfCharSet,
					&common.m_lf_kh.lfOutPrecision,
					&common.m_lf_kh.lfClipPrecision,
					&common.m_lf_kh.lfQuality,
					&common.m_lf_kh.lfPitchAndFamily
				);
			}
		}else{
			wsprintf( szKeyData, pszForm,
				common.m_lf_kh.lfHeight,
				common.m_lf_kh.lfWidth,
				common.m_lf_kh.lfEscapement,
				common.m_lf_kh.lfOrientation,
				common.m_lf_kh.lfWeight,
				common.m_lf_kh.lfItalic,
				common.m_lf_kh.lfUnderline,
				common.m_lf_kh.lfStrikeOut,
				common.m_lf_kh.lfCharSet,
				common.m_lf_kh.lfOutPrecision,
				common.m_lf_kh.lfClipPrecision,
				common.m_lf_kh.lfQuality,
				common.m_lf_kh.lfPitchAndFamily
			);
			cProfile.IOProfileData( pszSecName, "khlf", szKeyData, 0 );
		}
	
		cProfile.IOProfileData( pszSecName, "khlfFaceName"		, common.m_lf_kh.lfFaceName, 0 );
	
	}// Keword Help Font
	// ai 02/05/23 Add S
//	cProfile.IOProfileData( pszSecName, "bClickKeySearch"		, common.m_bUseCaretKeyWord );	// 2006.03.24 fon sakura�N������FALSE�Ƃ��A���������Ȃ�
	
	
	cProfile.IOProfileData( pszSecName, "nMRUArrNum_MAX"			, common.m_nMRUArrNum_MAX );
	cProfile.IOProfileData( pszSecName, "nOPENFOLDERArrNum_MAX"	, common.m_nOPENFOLDERArrNum_MAX );
	cProfile.IOProfileData( pszSecName, "bDispTOOLBAR"			, common.m_bDispTOOLBAR );
	cProfile.IOProfileData( pszSecName, "bDispSTATUSBAR"			, common.m_bDispSTATUSBAR );
	cProfile.IOProfileData( pszSecName, "bDispFUNCKEYWND"		, common.m_bDispFUNCKEYWND );
	cProfile.IOProfileData( pszSecName, "nFUNCKEYWND_Place"		, common.m_nFUNCKEYWND_Place );
	cProfile.IOProfileData( pszSecName, "nFUNCKEYWND_GroupNum"	, common.m_nFUNCKEYWND_GroupNum );		// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����
	
	cProfile.IOProfileData( pszSecName, "bDispTabWnd"			, common.m_bDispTabWnd );	//�^�u�E�C���h�E	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( pszSecName, "bDispTabWndMultiWin"	, common.m_bDispTabWndMultiWin );	//�^�u�E�C���h�E	//@@@ 2003.05.31 MIK
	cProfile.IOProfileData( pszSecName, "szTabWndCaption"		, common.m_szTabWndCaption		, MAX_CAPTION_CONF_LEN );	//@@@ 2003.06.13 MIK
	cProfile.IOProfileData( pszSecName, "bSameTabWidth"			, common.m_bSameTabWidth );	// 2006.01.28 ryoji �^�u�𓙕��ɂ���
	cProfile.IOProfileData( pszSecName, "bDispTabIcon"			, common.m_bDispTabIcon );	// 2006.01.28 ryoji �^�u�ɃA�C�R����\������
	cProfile.IOProfileData( pszSecName, "bSortTabList"			, common.m_bSortTabList );	// 2006.05.10 ryoji �^�u�ꗗ���\�[�g����
	
	// 2001/06/20 asa-o �����E�B���h�E�̃X�N���[���̓������Ƃ�
	cProfile.IOProfileData( pszSecName, "bSplitterWndHScroll"	, common.m_bSplitterWndHScroll );
	cProfile.IOProfileData( pszSecName, "bSplitterWndVScroll"	, common.m_bSplitterWndVScroll );
	
	cProfile.IOProfileData( pszSecName, "szMidashiKigou"			, common.m_szMidashiKigou, 0 );
	cProfile.IOProfileData( pszSecName, "szInyouKigou"			, common.m_szInyouKigou, 0 );
	
	// 2001/06/14 asa-o �⊮�ƃL�[���[�h�w���v�̓^�C�v�ʂɈړ������̂ō폜�F�R�s
	cProfile.IOProfileData( pszSecName, "bUseHokan"				, common.m_bUseHokan );
	// 2002/09/21 Moca bGrepKanjiCode_AutoDetect �� bGrepCharSet�ɓ��������̂ō폜
	// 2001/06/19 asa-o �^�C�v�ʂɈړ������̂ō폜�F1�s
	cProfile.IOProfileData( pszSecName, "bSaveWindowSize"		, common.m_nSaveWindowSize );
	cProfile.IOProfileData( pszSecName, "nWinSizeType"			, common.m_nWinSizeType );
	cProfile.IOProfileData( pszSecName, "nWinSizeCX"				, common.m_nWinSizeCX );
	cProfile.IOProfileData( pszSecName, "nWinSizeCY"				, common.m_nWinSizeCY );
	// 2004.03.30 Moca *nWinPos*��ǉ�
	cProfile.IOProfileData( pszSecName, "nSaveWindowPos"			, common.m_nSaveWindowPos );
	cProfile.IOProfileData( pszSecName, "nWinPosX"				, common.m_nWinPosX );
	cProfile.IOProfileData( pszSecName, "nWinPosY"				, common.m_nWinPosY );
	cProfile.IOProfileData( pszSecName, "bTaskTrayUse"			, common.m_bUseTaskTray );
	cProfile.IOProfileData( pszSecName, "bTaskTrayStay"			, common.m_bStayTaskTray );
//@@@ 2002.01.08 YAZAKI �^�X�N�g���C���u�g��Ȃ��v�ɂ��Ă��풓���`�F�b�N���c���Ă���ƍċN���Łu�g���E�풓�v�ɂȂ�o�O�C��
#if 0
	if( bRead ){
		/* �^�X�N�g���C�ɏ풓����Ƃ��́A�K���^�X�N�g���C�A�C�R�����g�� */
		if( common.m_bStayTaskTray ){
			common.m_bUseTaskTray = TRUE;
		}
	}
#endif
	cProfile.IOProfileData( pszSecName, "wTrayMenuHotKeyCode"		, common.m_wTrayMenuHotKeyCode );
	cProfile.IOProfileData( pszSecName, "wTrayMenuHotKeyMods"		, common.m_wTrayMenuHotKeyMods );
	cProfile.IOProfileData( pszSecName, "bUseOLE_DragDrop"			, common.m_bUseOLE_DragDrop );
	cProfile.IOProfileData( pszSecName, "bUseOLE_DropSource"			, common.m_bUseOLE_DropSource );
	cProfile.IOProfileData( pszSecName, "bDispExitingDialog"			, common.m_bDispExitingDialog );
	cProfile.IOProfileData( pszSecName, "bEnableUnmodifiedOverwrite"	, common.m_bEnableUnmodifiedOverwrite );
	cProfile.IOProfileData( pszSecName, "bSelectClickedURL"			, common.m_bSelectClickedURL );
	cProfile.IOProfileData( pszSecName, "bGrepExitConfirm"			, common.m_bGrepExitConfirm );/* Grep���[�h�ŕۑ��m�F���邩 */
//	cProfile.IOProfileData( pszSecName, "bRulerDisp"					, common.m_bRulerDisp );/* ���[���[�\�� */
	cProfile.IOProfileData( pszSecName, "nRulerHeight"				, common.m_nRulerHeight );/* ���[���[���� */
	cProfile.IOProfileData( pszSecName, "nRulerBottomSpace"			, common.m_nRulerBottomSpace );/* ���[���[�ƃe�L�X�g�̌��� */
	cProfile.IOProfileData( pszSecName, "nRulerType"					, common.m_nRulerType );/* ���[���[�̃^�C�v */
	//	Sep. 18, 2002 genta �ǉ�
	cProfile.IOProfileData( pszSecName, "nLineNumberRightSpace"		, common.m_nLineNumRightSpace );/* �s�ԍ��̉E���̌��� */
	cProfile.IOProfileData( pszSecName, "nVertLineOffset"			, common.m_nVertLineOffset ); // 2005.11.10 Moca
	cProfile.IOProfileData( pszSecName, "bCopyAndDisablSelection"	, common.m_bCopyAndDisablSelection );/* �R�s�[������I������ */
	cProfile.IOProfileData( pszSecName, "bHtmlHelpIsSingle"			, common.m_bHtmlHelpIsSingle );/* HtmlHelp�r���[�A�͂ЂƂ� */
	cProfile.IOProfileData( pszSecName, "bCompareAndTileHorz"		, common.m_bCompareAndTileHorz );/* ������r��A���E�ɕ��ׂĕ\�� */	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕s�v�̂͂�
	cProfile.IOProfileData( pszSecName, "bDropFileAndClose"			, common.m_bDropFileAndClose );/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
	cProfile.IOProfileData( pszSecName, "nDropFileNumMax"			, common.m_nDropFileNumMax );/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
	cProfile.IOProfileData( pszSecName, "bCheckFileTimeStamp"		, common.m_bCheckFileTimeStamp );/* �X�V�̊Ď� */
	cProfile.IOProfileData( pszSecName, "bNotOverWriteCRLF"			, common.m_bNotOverWriteCRLF );/* ���s�͏㏑�����Ȃ� */
	cProfile.IOProfileData( pszSecName, "bAutoCloseDlgFind"			, common.m_bAutoCloseDlgFind );/* �����_�C�A���O�������I�ɕ��� */
	cProfile.IOProfileData( pszSecName, "bAutoCloseDlgFuncList"		, common.m_bAutoCloseDlgFuncList );/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */
	cProfile.IOProfileData( pszSecName, "bAutoCloseDlgReplace"		, common.m_bAutoCloseDlgReplace );/* �u�� �_�C�A���O�������I�ɕ��� */
	cProfile.IOProfileData( pszSecName, "bAutoColmnPaste"			, common.m_bAutoColmnPaste );/* ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t�� */
	cProfile.IOProfileData( pszSecName, "bScrollBarHorz"				, common.m_bScrollBarHorz );/* �����X�N���[���o�[���g�� */
	
	cProfile.IOProfileData( pszSecName, "bHokanKey_RETURN"			, common.m_bHokanKey_RETURN );/* VK_RETURN �⊮����L�[���L��/���� */
	cProfile.IOProfileData( pszSecName, "bHokanKey_TAB"				, common.m_bHokanKey_TAB );/* VK_TAB    �⊮����L�[���L��/���� */
	cProfile.IOProfileData( pszSecName, "bHokanKey_RIGHT"			, common.m_bHokanKey_RIGHT );/* VK_RIGHT  �⊮����L�[���L��/���� */
	cProfile.IOProfileData( pszSecName, "bHokanKey_SPACE"			, common.m_bHokanKey_SPACE );/* VK_SPACE  �⊮����L�[���L��/���� */
	
	cProfile.IOProfileData( pszSecName, "nDateFormatType"			, common.m_nDateFormatType );/* ���t�����̃^�C�v */
	cProfile.IOProfileData( pszSecName, "szDateFormat"				, common.m_szDateFormat, 0 );//���t����
	cProfile.IOProfileData( pszSecName, "nTimeFormatType"			, common.m_nTimeFormatType );/* ���������̃^�C�v */
	cProfile.IOProfileData( pszSecName, "szTimeFormat"				, common.m_szTimeFormat, 0 );//��������
	
	cProfile.IOProfileData( pszSecName, "bMenuIcon"					, common.m_bMenuIcon );//���j���[�ɃA�C�R����\������
	cProfile.IOProfileData( pszSecName, "bAutoMIMEdecode"			, common.m_bAutoMIMEdecode );//�t�@�C���ǂݍ��ݎ���MIME��decode���s����
	cProfile.IOProfileData( pszSecName, "bQueryIfCodeChange"			, common.m_bQueryIfCodeChange );//	Oct. 03, 2004 genta �O��ƈقȂ镶���R�[�h�̂Ƃ��ɖ₢���킹���s����
	cProfile.IOProfileData( pszSecName, "bAlertIfFileNotExist"		, common.m_bAlertIfFileNotExist );// Oct. 09, 2004 genta �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������
	
	cProfile.IOProfileData( pszSecName, "bNoFilterSaveNew"			, common.m_bNoFilterSaveNew );	// �V�K����ۑ����͑S�t�@�C���\��	// 2006.11.16 ryoji
	cProfile.IOProfileData( pszSecName, "bNoFilterSaveFile"			, common.m_bNoFilterSaveFile );	// �V�K�ȊO����ۑ����͑S�t�@�C���\��	// 2006.11.16 ryoji
	
	/* �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu */
	const char* pszKeyName = "rcOpenDialog";
	const char* pszForm = "%d,%d,%d,%d";
	char		szKeyData[1024];
	if( cProfile.IsReadingMode() ){
		if( true == cProfile.IOProfileData( pszSecName, pszKeyName, szKeyData, sizeof( szKeyData )) ){
			sscanf( szKeyData, pszForm,
				&common.m_rcOpenDialog.left,
				&common.m_rcOpenDialog.top,
				&common.m_rcOpenDialog.right,
				&common.m_rcOpenDialog.bottom
			);
		}
	}else{
		wsprintf( szKeyData, pszForm,
			common.m_rcOpenDialog.left,
			common.m_rcOpenDialog.top,
			common.m_rcOpenDialog.right,
			common.m_rcOpenDialog.bottom
		);
		cProfile.IOProfileData( pszSecName, pszKeyName, szKeyData, 0 );
	}
	
	//2002.02.08 aroka,hor
	cProfile.IOProfileData( pszSecName, "bMarkUpBlankLineEnable"	, common.m_bMarkUpBlankLineEnable );
	cProfile.IOProfileData( pszSecName, "bFunclistSetFocusOnJump"	, common.m_bFunclistSetFocusOnJump );
	
	//	Apr. 05, 2003 genta �E�B���h�E�L���v�V�����̃J�X�^�}�C�Y
	cProfile.IOProfileData( pszSecName, "szWinCaptionActive"
		, common.m_szWindowCaptionActive, MAX_CAPTION_CONF_LEN );
	cProfile.IOProfileData( pszSecName, "szWinCaptionInactive"
		, common.m_szWindowCaptionInactive, MAX_CAPTION_CONF_LEN );
	
	// �A�E�g���C��/�g�s�b�N���X�g �̈ʒu�ƃT�C�Y���L��  20060201 aroka
	cProfile.IOProfileData( pszSecName, "bRememberOutlineWindowPos", common.m_bRememberOutlineWindowPos);
	if( common.m_bRememberOutlineWindowPos ){
		cProfile.IOProfileData( pszSecName, "widthOutlineWindow"	, common.m_widthOutlineWindow);
		cProfile.IOProfileData( pszSecName, "heightOutlineWindow", common.m_heightOutlineWindow);
		cProfile.IOProfileData( pszSecName, "xOutlineWindowPos"	, common.m_xOutlineWindowPos);
		cProfile.IOProfileData( pszSecName, "yOutlineWindowPos"	, common.m_yOutlineWindowPos);
	}
	
}

/*!
	@brief ���L�f�[�^��Toolbar�Z�N�V�����̓��o��
	@param[in]		bRead		true: �ǂݍ��� / false: ��������
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B�ǂݍ��ݎ��̏��������C��
*/
void CShareData::ShareData_IO_Toolbar( CProfile& cProfile )
{
	const char* pszSecName = "Toolbar";
	int		i;
	char	szKeyName[64];
	Common& common = m_pShareData->m_Common;

	cProfile.IOProfileData( pszSecName, "nToolBarButtonNum", common.m_nToolBarButtonNum );
	cProfile.IOProfileData( pszSecName, "bToolBarIsFlat", common.m_bToolBarIsFlat );
	int	nSize = common.m_nToolBarButtonNum;
	for( i = 0; i < nSize; ++i ){
		wsprintf( szKeyName, "nTBB[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, common.m_nToolBarButtonIdxArr[i] );
	}
	//�ǂݍ��ݎ��͎c���������
	if( cProfile.IsReadingMode() ){
		for(; i< MAX_TOOLBARBUTTONS; ++i){
			common.m_nToolBarButtonIdxArr[i] = 0;
		}
	}
}

/*!
	@brief ���L�f�[�^��CustMenu�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_CustMenu( CProfile& cProfile )
{
	const char* pszSecName = "CustMenu";
	int		i, j;
	char	szKeyName[64];
	Common& common = m_pShareData->m_Common;

	for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
		wsprintf( szKeyName, "szCMN[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, common.m_szCustMenuNameArr[i], MAX_CUSTOM_MENU_NAME_LEN + 1 );	//	Oct. 15, 2001 genta �ő咷�w��
		wsprintf( szKeyName, "nCMIN[%02d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, common.m_nCustMenuItemNumArr[i] );
		int nSize = common.m_nCustMenuItemNumArr[i];
		for( j = 0; j < nSize; ++j ){
			wsprintf( szKeyName, "nCMIF[%02d][%02d]", i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, common.m_nCustMenuItemFuncArr[i][j] );
			wsprintf( szKeyName, "nCMIK[%02d][%02d]", i, j );
			cProfile.IOProfileData( pszSecName, szKeyName, common.m_nCustMenuItemKeyArr[i][j] );
		}
	}
}

/*!
	@brief ���L�f�[�^��Font�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Font( CProfile& cProfile )
{
	const char* pszSecName = "Font";
	const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
	char		szKeyData[1024];
	Common& common = m_pShareData->m_Common;
	if( cProfile.IsReadingMode() ){
		if( true == cProfile.IOProfileData( pszSecName, "lf", szKeyData, sizeof( szKeyData )) ){
			sscanf( szKeyData, pszForm,
				&common.m_lf.lfHeight,
				&common.m_lf.lfWidth,
				&common.m_lf.lfEscapement,
				&common.m_lf.lfOrientation,
				&common.m_lf.lfWeight,
				&common.m_lf.lfItalic,
				&common.m_lf.lfUnderline,
				&common.m_lf.lfStrikeOut,
				&common.m_lf.lfCharSet,
				&common.m_lf.lfOutPrecision,
				&common.m_lf.lfClipPrecision,
				&common.m_lf.lfQuality,
				&common.m_lf.lfPitchAndFamily
			);
		}
	}else{
		wsprintf( szKeyData, pszForm,
			common.m_lf.lfHeight,
			common.m_lf.lfWidth,
			common.m_lf.lfEscapement,
			common.m_lf.lfOrientation,
			common.m_lf.lfWeight,
			common.m_lf.lfItalic,
			common.m_lf.lfUnderline,
			common.m_lf.lfStrikeOut,
			common.m_lf.lfCharSet,
			common.m_lf.lfOutPrecision,
			common.m_lf.lfClipPrecision,
			common.m_lf.lfQuality,
			common.m_lf.lfPitchAndFamily
		);
		cProfile.IOProfileData( pszSecName, "lf", szKeyData, 0 );
	}
	
	cProfile.IOProfileData( pszSecName, "lfFaceName",
		common.m_lf.lfFaceName, sizeof( common.m_lf.lfFaceName ));
	
	cProfile.IOProfileData( pszSecName, "bFontIs_FIXED_PITCH", common.m_bFontIs_FIXED_PITCH );
}

/*!
	@brief ���L�f�[�^��KeyBind�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_KeyBind( CProfile& cProfile )
{
	const char* pszSecName = "KeyBind";
	int		i;
	char	szKeyName[64];
	char	szKeyData[1024];
	int		nSize = m_pShareData->m_nKeyNameArrNum;
	for( i = 0; i < nSize; ++i ){
		// 2005.04.07 D.S.Koba
		KEYDATA& keydata = m_pShareData->m_pKeyNameArr[i];
		strcpy( szKeyName, keydata.m_szKeyName );
		
		if( cProfile.IsReadingMode() ){
			if( true == cProfile.IOProfileData( pszSecName, szKeyName,
				szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, "%d,%d,%d,%d,%d,%d,%d,%d",
					&keydata.m_nFuncCodeArr[0],
					&keydata.m_nFuncCodeArr[1],
					&keydata.m_nFuncCodeArr[2],
					&keydata.m_nFuncCodeArr[3],
					&keydata.m_nFuncCodeArr[4],
					&keydata.m_nFuncCodeArr[5],
					&keydata.m_nFuncCodeArr[6],
					&keydata.m_nFuncCodeArr[7]
				 );
			}
		}else{
			wsprintf( szKeyData, "%d,%d,%d,%d,%d,%d,%d,%d",
				keydata.m_nFuncCodeArr[0],
				keydata.m_nFuncCodeArr[1],
				keydata.m_nFuncCodeArr[2],
				keydata.m_nFuncCodeArr[3],
				keydata.m_nFuncCodeArr[4],
				keydata.m_nFuncCodeArr[5],
				keydata.m_nFuncCodeArr[6],
				keydata.m_nFuncCodeArr[7]
			);
			cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
		}
	}
}

/*!
	@brief ���L�f�[�^��Print�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Print( CProfile& cProfile )
{
	const char* pszSecName = "Print";
	int		i, j;
	char	szKeyName[64];
	char	szKeyData[1024];
	for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
		// 2005.04.07 D.S.Koba
		PRINTSETTING& printsetting = m_pShareData->m_PrintSettingArr[i];
		wsprintf( szKeyName, "PS[%02d].nInts", i );
		static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";
		if( cProfile.IsReadingMode() ){
			if( true == cProfile.IOProfileData( pszSecName, szKeyName,
				szKeyData, sizeof( szKeyData ) ) ){
				sscanf( szKeyData, pszForm,
					&printsetting.m_nPrintFontWidth		,
					&printsetting.m_nPrintFontHeight		,
					&printsetting.m_nPrintDansuu			,
					&printsetting.m_nPrintDanSpace		,
					&printsetting.m_nPrintLineSpacing		,
					&printsetting.m_nPrintMarginTY		,
					&printsetting.m_nPrintMarginBY		,
					&printsetting.m_nPrintMarginLX		,
					&printsetting.m_nPrintMarginRX		,
					&printsetting.m_nPrintPaperOrientation,
					&printsetting.m_nPrintPaperSize		,
					&printsetting.m_bPrintWordWrap		,
					&printsetting.m_bPrintLineNumber		,
					&printsetting.m_bHeaderUse[0]			,
					&printsetting.m_bHeaderUse[1]			,
					&printsetting.m_bHeaderUse[2]			,
					&printsetting.m_bFooterUse[0]			,
					&printsetting.m_bFooterUse[1]			,
					&printsetting.m_bFooterUse[2]
				 );
			}
		}else{
			wsprintf( szKeyData, pszForm,
				printsetting.m_nPrintFontWidth		,
				printsetting.m_nPrintFontHeight		,
				printsetting.m_nPrintDansuu			,
				printsetting.m_nPrintDanSpace			,
				printsetting.m_nPrintLineSpacing		,
				printsetting.m_nPrintMarginTY			,
				printsetting.m_nPrintMarginBY			,
				printsetting.m_nPrintMarginLX			,
				printsetting.m_nPrintMarginRX			,
				printsetting.m_nPrintPaperOrientation	,
				printsetting.m_nPrintPaperSize		,
				printsetting.m_bPrintWordWrap			,
				printsetting.m_bPrintLineNumber		,
				printsetting.m_bHeaderUse[0]			,
				printsetting.m_bHeaderUse[1]			,
				printsetting.m_bHeaderUse[2]			,
				printsetting.m_bFooterUse[0]			,
				printsetting.m_bFooterUse[1]			,
				printsetting.m_bFooterUse[2]
			);
			cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
		}
		
		wsprintf( szKeyName, "PS[%02d].szSName"	, i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_szPrintSettingName	,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintSettingName ));
		wsprintf( szKeyName, "PS[%02d].szFF"	, i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_szPrintFontFaceHan	,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceHan ));
		wsprintf( szKeyName, "PS[%02d].szFFZ"	, i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_szPrintFontFaceZen	,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_szPrintFontFaceZen ));
		for( j = 0; j < 3; ++j ){
			wsprintf( szKeyName, "PS[%02d].szHF[%d]" , i, j );
			cProfile.IOProfileData( pszSecName, szKeyName,
				printsetting.m_szHeaderForm[j],
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szHeaderForm [0] ));
			wsprintf( szKeyName, "PS[%02d].szFTF[%d]", i, j );
			cProfile.IOProfileData( pszSecName, szKeyName,
				printsetting.m_szFooterForm[j],
				sizeof( m_pShareData->m_PrintSettingArr[0].m_szFooterForm[0] ));
		}
		wsprintf( szKeyName, "PS[%02d].szDriver", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_mdmDevMode.m_szPrinterDriverName,
			sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDriverName ));
		wsprintf( szKeyName, "PS[%02d].szDevice", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_mdmDevMode.m_szPrinterDeviceName, 
			sizeof( m_pShareData->m_PrintSettingArr[0].m_mdmDevMode.m_szPrinterDeviceName ));
		wsprintf( szKeyName, "PS[%02d].szOutput", i );
		cProfile.IOProfileData( pszSecName, szKeyName,
			printsetting.m_mdmDevMode.m_szPrinterOutputName,
			sizeof( printsetting.m_mdmDevMode.m_szPrinterOutputName ));
		
		// 2002.02.16 hor �Ƃ肠�������ݒ��ϊ����Ƃ�
		if(0==strcmp(printsetting.m_szHeaderForm[0],"&f") &&
		   0==strcmp(printsetting.m_szFooterForm[0],"&C- &P -")
		){
			strcpy( printsetting.m_szHeaderForm[0], "$f" );
			strcpy( printsetting.m_szFooterForm[0], "" );
			strcpy( printsetting.m_szFooterForm[1], "- $p -" );
		}
		
		//�֑�	//@@@ 2002.04.09 MIK
		wsprintf( szKeyName, "PS[%02d].bKinsokuHead", i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuHead );
		wsprintf( szKeyName, "PS[%02d].bKinsokuTail", i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuTail );
		wsprintf( szKeyName, "PS[%02d].bKinsokuRet",  i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuRet );	//@@@ 2002.04.13 MIK
		wsprintf( szKeyName, "PS[%02d].bKinsokuKuto", i ); cProfile.IOProfileData( pszSecName, szKeyName, printsetting.m_bPrintKinsokuKuto );	//@@@ 2002.04.17 MIK
	}
}

/*!
	@brief ���L�f�[�^��Types�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Types( CProfile& cProfile )
{
	const char* pszSecName;
	int		i, j;
	char	szKey[256];
	char	szKeyName[64];
	char	szKeyData[1024];

	for( i = 0; i < MAX_TYPES; ++i ){
		// 2005.04.07 D.S.Koba
		Types& types = m_pShareData->m_Types[i];
		wsprintf( szKey, "Types(%d)", i );
		pszSecName = szKey;
		static const char* pszForm = "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";	//MIK
		strcpy( szKeyName, "nInts" );
		if( cProfile.IsReadingMode() ){
			if( true == cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
				sscanf( szKeyData, pszForm,
					&types.m_nIdx,
					&types.m_nMaxLineSize,
					&types.m_nColmSpace,
					&types.m_nTabSpace,
					&types.m_nKeyWordSetIdx[0],
					&types.m_nKeyWordSetIdx[1],	//MIK
					&types.m_nStringType,
					&types.m_bLineNumIsCRLF,
					&types.m_nLineTermType,
					&types.m_bWordWrap,
					&types.m_nCurrentPrintSetting
				 );
			}
			// �܂�Ԃ����̍ŏ��l��10�B���Ȃ��Ƃ��S�Ȃ��ƃn���O�A�b�v����B // 20050818 aroka
			if( types.m_nMaxLineSize < MINLINESIZE ){
				types.m_nMaxLineSize = MINLINESIZE;
			}
		}else{
			wsprintf( szKeyData, pszForm,
				types.m_nIdx,
				types.m_nMaxLineSize,
				types.m_nColmSpace,
				types.m_nTabSpace,
				types.m_nKeyWordSetIdx[0],
				types.m_nKeyWordSetIdx[1],	//MIK
				types.m_nStringType,
				types.m_bLineNumIsCRLF,
				types.m_nLineTermType,
				types.m_bWordWrap,
				types.m_nCurrentPrintSetting
			);
			cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
		}
		// 2005.01.13 MIK Keywordset 3-10
		cProfile.IOProfileData( pszSecName, "nKeywordSelect3",  types.m_nKeyWordSetIdx[2] );
		cProfile.IOProfileData( pszSecName, "nKeywordSelect4",  types.m_nKeyWordSetIdx[3] );
		cProfile.IOProfileData( pszSecName, "nKeywordSelect5",  types.m_nKeyWordSetIdx[4] );
		cProfile.IOProfileData( pszSecName, "nKeywordSelect6",  types.m_nKeyWordSetIdx[5] );
		cProfile.IOProfileData( pszSecName, "nKeywordSelect7",  types.m_nKeyWordSetIdx[6] );
		cProfile.IOProfileData( pszSecName, "nKeywordSelect8",  types.m_nKeyWordSetIdx[7] );
		cProfile.IOProfileData( pszSecName, "nKeywordSelect9",  types.m_nKeyWordSetIdx[8] );
		cProfile.IOProfileData( pszSecName, "nKeywordSelect10", types.m_nKeyWordSetIdx[9] );

		/* �s�Ԃ̂����� */
		if( !cProfile.IsReadingMode() ){
			if( types.m_nLineSpace < /* 1 */ 0 ){
				types.m_nLineSpace = /* 1 */ 0;
			}
			if( types.m_nLineSpace > LINESPACE_MAX ){ // Feb. 18, 2003 genta �ő�l�̒萔��
				types.m_nLineSpace = LINESPACE_MAX;
			}
		}
		cProfile.IOProfileData( pszSecName, "nLineSpace", types.m_nLineSpace );
		if( cProfile.IsReadingMode() ){
			if( types.m_nLineSpace < /* 1 */ 0 ){
				types.m_nLineSpace = /* 1 */ 0;
			}
			if( types.m_nLineSpace > 16 ){
				types.m_nLineSpace = 16;
			}
		}


		cProfile.IOProfileData( pszSecName, "szTypeName",
			types.m_szTypeName,
			sizeof( m_pShareData->m_Types[0].m_szTypeName ));
		cProfile.IOProfileData( pszSecName, "szTypeExts",
			types.m_szTypeExts,
			sizeof( m_pShareData->m_Types[0].m_szTypeExts ));
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
		cProfile.IOProfileData( pszSecName, "szTabViewString",
		/*&*/types.m_szTabViewString,
		sizeof( types.m_szTabViewString ));
//#endif
		cProfile.IOProfileData( pszSecName, "bTabArrow"			, types.m_bTabArrow );	//@@@ 2003.03.26 MIK
		cProfile.IOProfileData( pszSecName, "bInsSpace"			, types.m_bInsSpace );	// 2001.12.03 hor

		// From Here Sep. 28, 2002 genta / YAZAKI
		if( cProfile.IsReadingMode() ){
			//	Block Comment
			char buffer[2][ BLOCKCOMMENT_BUFFERSIZE ];
			//	2004.10.02 Moca �΂ɂȂ�R�����g�ݒ肪�Ƃ��ɓǂݍ��܂ꂽ�Ƃ������L���Ȑݒ�ƌ��Ȃ��D
			//	�u���b�N�R�����g�̎n�܂�ƏI���D�s�R�����g�̋L���ƌ��ʒu
			bool bRet1, bRet2;
			buffer[0][0] = buffer[1][0] = '\0';
			bRet1 = cProfile.IOProfileData( pszSecName, "szBlockCommentFrom"	,
				buffer[0], BLOCKCOMMENT_BUFFERSIZE );			
			bRet2 = cProfile.IOProfileData( pszSecName, "szBlockCommentTo"	,
				buffer[1], BLOCKCOMMENT_BUFFERSIZE );
			if( bRet1 && bRet2 ) types.m_cBlockComment.CopyTo( 0, buffer[0], buffer[1] );

			//@@@ 2001.03.10 by MIK
			buffer[0][0] = buffer[1][0] = '\0';
			bRet1 = cProfile.IOProfileData( pszSecName, "szBlockCommentFrom2",
				buffer[0], BLOCKCOMMENT_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( pszSecName, "szBlockCommentTo2"	,
				buffer[1], BLOCKCOMMENT_BUFFERSIZE );
			if( bRet1 && bRet2 ) types.m_cBlockComment.CopyTo( 1, buffer[0], buffer[1] );
			
			//	Line Comment
			char lbuf[ COMMENT_DELIMITER_BUFFERSIZE ];
			int  pos;

			lbuf[0] = '\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( pszSecName, "szLineComment"		,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( pszSecName, "nLineCommentColumn"	, pos );
			if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 0, lbuf, pos );

			lbuf[0] = '\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( pszSecName, "szLineComment2"		,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );
			bRet2 = cProfile.IOProfileData( pszSecName, "nLineCommentColumn2", pos );
			if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 1, lbuf, pos );

			lbuf[0] = '\0'; pos = -1;
			bRet1 = cProfile.IOProfileData( pszSecName, "szLineComment3"		,
				lbuf, COMMENT_DELIMITER_BUFFERSIZE );	//Jun. 01, 2001 JEPRO �ǉ�
			bRet2 = cProfile.IOProfileData( pszSecName, "nLineCommentColumn3", pos );	//Jun. 01, 2001 JEPRO �ǉ�
			if( bRet1 && bRet2 ) types.m_cLineComment.CopyTo( 2, lbuf, pos );
		}
		else { // write
			//	Block Comment
			cProfile.IOProfileData( pszSecName, "szBlockCommentFrom"	,
				const_cast<char*>(types.m_cBlockComment.getBlockCommentFrom( 0 )), 0 );
			cProfile.IOProfileData( pszSecName, "szBlockCommentTo"	,
				const_cast<char*>(types.m_cBlockComment.getBlockCommentTo( 0 )), 0 );

			//@@@ 2001.03.10 by MIK
			cProfile.IOProfileData( pszSecName, "szBlockCommentFrom2",
				const_cast<char*>(types.m_cBlockComment.getBlockCommentFrom( 1 )), 0 );
			cProfile.IOProfileData( pszSecName, "szBlockCommentTo2"	,
				const_cast<char*>(types.m_cBlockComment.getBlockCommentTo(1)), 0 );

			//	Line Comment
		cProfile.IOProfileData( pszSecName, "szLineComment"		,
			const_cast<char*>(types.m_cLineComment.getLineComment( 0 )), 0 );
		cProfile.IOProfileData( pszSecName, "szLineComment2"		,
			const_cast<char*>(types.m_cLineComment.getLineComment( 1 )), 0 );
		cProfile.IOProfileData( pszSecName, "szLineComment3"		,
			const_cast<char*>(types.m_cLineComment.getLineComment( 2 )), 0 );	//Jun. 01, 2001 JEPRO �ǉ�

		//	From here May 12, 2001 genta
		int pos;
		pos = types.m_cLineComment.getLineCommentPos( 0 );
		cProfile.IOProfileData( pszSecName, "nLineCommentColumn"	, pos );
		pos = types.m_cLineComment.getLineCommentPos( 1 );
		cProfile.IOProfileData( pszSecName, "nLineCommentColumn2", pos );
		pos = types.m_cLineComment.getLineCommentPos( 2 );
		cProfile.IOProfileData( pszSecName, "nLineCommentColumn3", pos );	//Jun. 01, 2001 JEPRO �ǉ�
		//	To here May 12, 2001 genta

		}
		// To Here Sep. 28, 2002 genta / YAZAKI

		cProfile.IOProfileData( pszSecName, "szIndentChars"		,
			types.m_szIndentChars,
			sizeof( m_pShareData->m_Types[0].m_szIndentChars ));
		cProfile.IOProfileData( pszSecName, "cLineTermChar"		, types.m_cLineTermChar );

		cProfile.IOProfileData( pszSecName, "nDefaultOutline"	, types.m_nDefaultOutline );/* �A�E�g���C����͕��@ */
		cProfile.IOProfileData( pszSecName, "szOutlineRuleFilename"	,
			types.m_szOutlineRuleFilename,
			sizeof( m_pShareData->m_Types[0].m_szOutlineRuleFilename ));/* �A�E�g���C����̓��[���t�@�C�� */
		cProfile.IOProfileData( pszSecName, "nSmartIndent"		, types.m_nSmartIndent );/* �X�}�[�g�C���f���g��� */
		//	Nov. 20, 2000 genta
		cProfile.IOProfileData( pszSecName, "nImeState"			, types.m_nImeState );	//	IME����

		//	2001/06/14 Start By asa-o: �^�C�v�ʂ̕⊮�t�@�C��
		//	Oct. 5, 2002 genta sizeof()�Ō���ă|�C���^�̃T�C�Y���擾���Ă����̂��C��
		cProfile.IOProfileData( pszSecName, "szHokanFile"		,
			types.m_szHokanFile,
			sizeof( m_pShareData->m_Types[0].m_szHokanFile ));		//	�⊮�t�@�C��
		//	2001/06/14 End

		//	2001/06/19 asa-o
		cProfile.IOProfileData( pszSecName, "bHokanLoHiCase"		, types.m_bHokanLoHiCase );

		//	2003.06.23 Moca �t�@�C��������̓��͕⊮�@�\
		cProfile.IOProfileData( pszSecName, "bUseHokanByFile"		, types.m_bUseHokanByFile );

		//@@@ 2002.2.4 YAZAKI
		cProfile.IOProfileData( pszSecName, "szExtHelp"			,
			types.m_szExtHelp,
			sizeof( m_pShareData->m_Types[0].m_szExtHelp ));
			
		cProfile.IOProfileData( pszSecName, "szExtHtmlHelp"		,
			types.m_szExtHtmlHelp,
			sizeof( types.m_szExtHtmlHelp ));
		cProfile.IOProfileData( pszSecName, "bHtmlHelpIsSingle"	, types.m_bHokanLoHiCase );

		cProfile.IOProfileData( pszSecName, "bAutoIndent"			, types.m_bAutoIndent );
		cProfile.IOProfileData( pszSecName, "bAutoIndent_ZENSPACE"	, types.m_bAutoIndent_ZENSPACE );
		cProfile.IOProfileData( pszSecName, "bRTrimPrevLine"			, types.m_bRTrimPrevLine );			// 2005.10.08 ryoji
		cProfile.IOProfileData( pszSecName, "nIndentLayout"			, types.m_nIndentLayout );

		/* �F�ݒ� I/O */
		IO_ColorSet( &cProfile, pszSecName, types.m_ColorInfoArr  );

		// 2005.11.08 Moca �w�茅�c��
		for(j = 0; j < MAX_VERTLINES; j++ ){
			wsprintf( szKeyName, "nVertLineIdx%d", j + 1 );
			cProfile.IOProfileData( pszSecName, szKeyName, types.m_nVertLineIdx[j] );
			if( types.m_nVertLineIdx[j] == 0 ){
				break;
			}
		}

//@@@ 2001.11.17 add start MIK
		{	//���K�\���L�[���[�h
			char	*p;
			cProfile.IOProfileData( pszSecName, "bUseRegexKeyword", types.m_bUseRegexKeyword );/* ���K�\���L�[���[�h�g�p���邩�H */
			for(j = 0; j < 100; j++)
			{
				wsprintf( szKeyName, "RxKey[%03d]", j );
				if( cProfile.IsReadingMode() )
				{
					types.m_RegexKeywordArr[j].m_szKeyword[0] = '\0';
					types.m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
					if( true == cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) )
					{
						p = strchr(szKeyData, ',');
						if( p )
						{
							*p = '\0';
							types.m_RegexKeywordArr[j].m_nColorIndex = GetColorIndexByName(szKeyData);	//@@@ 2002.04.30
							if( types.m_RegexKeywordArr[j].m_nColorIndex == -1 )	//���O�łȂ�
								types.m_RegexKeywordArr[j].m_nColorIndex = atoi(szKeyData);
							p++;
							strcpy(types.m_RegexKeywordArr[j].m_szKeyword, p);
							if( types.m_RegexKeywordArr[j].m_nColorIndex < 0
							 || types.m_RegexKeywordArr[j].m_nColorIndex >= COLORIDX_LAST )
							{
								types.m_RegexKeywordArr[j].m_nColorIndex = COLORIDX_REGEX1;
							}
						}
					}
				}
				else
				// 2002.02.08 hor ����`�l�𖳎�
				if(lstrlen(types.m_RegexKeywordArr[j].m_szKeyword))
				{
					//wsprintf( szKeyData, "%d,%s",
					//	types.m_RegexKeywordArr[j].m_nColorIndex,
					//	types.m_RegexKeywordArr[j].m_szKeyword);
					wsprintf( szKeyData, "%s,%s",
						GetColorNameByIndex( types.m_RegexKeywordArr[j].m_nColorIndex ),
						types.m_RegexKeywordArr[j].m_szKeyword);
					cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
				}
			}
		}
//@@@ 2001.11.17 add end MIK

		/* �֑� */
		cProfile.IOProfileData( pszSecName, "bKinsokuHead"	, types.m_bKinsokuHead );
		cProfile.IOProfileData( pszSecName, "bKinsokuTail"	, types.m_bKinsokuTail );
		cProfile.IOProfileData( pszSecName, "bKinsokuRet"	, types.m_bKinsokuRet );	//@@@ 2002.04.13 MIK
		cProfile.IOProfileData( pszSecName, "bKinsokuKuto"	, types.m_bKinsokuKuto );	//@@@ 2002.04.17 MIK
		cProfile.IOProfileData( pszSecName, "szKinsokuHead"	,
			types.m_szKinsokuHead,
			sizeof( m_pShareData->m_Types[0].m_szKinsokuHead ));
		cProfile.IOProfileData( pszSecName, "szKinsokuTail"	,
			types.m_szKinsokuTail,
			sizeof( m_pShareData->m_Types[0].m_szKinsokuTail ));
		cProfile.IOProfileData( pszSecName, "bUseDocumentIcon"	, types.m_bUseDocumentIcon );	// Sep. 19 ,2002 genta �ϐ������C��

//@@@ 2006.04.10 fon ADD-start
		{	/* �L�[���[�h���� */
			static const char* pszForm = "%d,%s,%s";
			char	*pH, *pT;	/* <pH>keyword<pT> */
			cProfile.IOProfileData( pszSecName, "bUseKeyWordHelp", types.m_bUseKeyWordHelp );	/* �L�[���[�h�����I�����g�p���邩�H */
//			cProfile.IOProfileData( pszSecName, "nKeyHelpNum", types.m_nKeyHelpNum );				/* �o�^������ */
			cProfile.IOProfileData( pszSecName, "bUseKeyHelpAllSearch", types.m_bUseKeyHelpAllSearch );	/* �q�b�g�������̎���������(&A) */
			cProfile.IOProfileData( pszSecName, "bUseKeyHelpKeyDisp", types.m_bUseKeyHelpKeyDisp );		/* 1�s�ڂɃL�[���[�h���\������(&W) */
			cProfile.IOProfileData( pszSecName, "bUseKeyHelpPrefix", types.m_bUseKeyHelpPrefix );		/* �I��͈͂őO����v����(&P) */
			for(j = 0; j < MAX_KEYHELP_FILE; j++){
				wsprintf( szKeyName, "KDct[%02d]", j );
				/* �ǂݏo�� */
				if( cProfile.IsReadingMode() ){
					types.m_KeyHelpArr[j].m_nUse = 0;
					types.m_KeyHelpArr[j].m_szAbout[0] = '\0';
					types.m_KeyHelpArr[j].m_szPath[0] = '\0';
					if( true == cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
						pH = szKeyData;
						if( NULL != (pT=strchr(pH, ',')) ){
							*pT = '\0';
							types.m_KeyHelpArr[j].m_nUse = atoi( pH );
							pH = pT+1;
							if( NULL != (pT=strchr(pH, ',')) ){
								*pT = '\0';
								strcpy( types.m_KeyHelpArr[j].m_szAbout, pH );
								pH = pT+1;
								if( NULL != (*pH) ){
									strcpy( types.m_KeyHelpArr[j].m_szPath, pH );
									types.m_nKeyHelpNum = j+1;	// ini�ɕۑ������ɁA�ǂݏo�����t�@�C�������������Ƃ���
								}
							}
						}
					}
				}/* �������� */
				else{
					if(lstrlen(types.m_KeyHelpArr[j].m_szPath)){
						wsprintf( szKeyData, pszForm,
							types.m_KeyHelpArr[j].m_nUse,
							types.m_KeyHelpArr[j].m_szAbout,
							types.m_KeyHelpArr[j].m_szPath
						);
						cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
					}
				}
			}
			/* ���o�[�W����ini�t�@�C���̓ǂݏo���T�|�[�g */
			if( cProfile.IsReadingMode() ){
				cProfile.IOProfileData( pszSecName, "szKeyWordHelpFile",
				types.m_KeyHelpArr[0].m_szPath, sizeof( types.m_KeyHelpArr[0].m_szPath ) );
			}
		}
//@@@ 2006.04.10 fon ADD-end

	}/* for */
}

/*!
	@brief ���L�f�[�^��KeyWords�Z�N�V�����̓��o��
	@param[in]		bRead		true: �ǂݍ��� / false: ��������
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_KeyWords( CProfile& cProfile )
{
	const char*		pszSecName = "KeyWords";
	int				i, j;
	char			szKeyName[64];
	char			szKeyData[1024];
	CKeyWordSetMgr*	pCKeyWordSetMgr = &m_pShareData->m_CKeyWordSetMgr;
	int				nKeyWordSetNum = pCKeyWordSetMgr->m_nKeyWordSetNum;

	cProfile.IOProfileData( pszSecName, "nCurrentKeyWordSetIdx"	, pCKeyWordSetMgr->m_nCurrentKeyWordSetIdx );
	bool bIOSuccess = cProfile.IOProfileData( pszSecName, "nKeyWordSetNum", nKeyWordSetNum );
	if( cProfile.IsReadingMode() ){
		// nKeyWordSetNum ���ǂݍ��߂Ă���΁A���ׂĂ̏�񂪂�����Ă���Ɖ��肵�ď�����i�߂�
		if( bIOSuccess ){
			// 2004.11.25 Moca �L�[���[�h�Z�b�g�̏��́A���ڏ��������Ȃ��Ŋ֐��𗘗p����
			// �����ݒ肳��Ă��邽�߁A��ɍ폜���Ȃ��ƌŒ胁�����̊m�ۂɎ��s����\��������
			int  nMemLen = MAX_KEYWORDNUM * ( MAX_KEYWORDLEN + 1 ) + 1;
			char *pszMem = new char[nMemLen];
			pCKeyWordSetMgr->ResetAllKeyWordSet();
			for( i = 0; i < nKeyWordSetNum; ++i ){
				int nKEYWORDCASE = 0;
				int nKeyWordNum = 0;
				wsprintf( szKeyName, "szSN[%02d]", i );
				cProfile.IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData ));
				wsprintf( szKeyName, "nCASE[%02d]", i );
				cProfile.IOProfileData( pszSecName, szKeyName, nKEYWORDCASE );
				wsprintf( szKeyName, "nKWN[%02d]", i );
				cProfile.IOProfileData( pszSecName, szKeyName, nKeyWordNum );
				pCKeyWordSetMgr->AddKeyWordSet( szKeyData, nKEYWORDCASE, nKeyWordNum );
				// 2004.11.25 Moca szKW[%02d].Size �̏��͗��p����Ӗ����Ȃ��B
//				wsprintf( szKeyName, "szKW[%02d].Size", i );
//				cProfile.IOProfileData( pszSecName, szKeyName, nMemLen );
				wsprintf( szKeyName, "szKW[%02d]", i );
				if( true == cProfile.IOProfileData( pszSecName, szKeyName, pszMem, nMemLen ) ){
					pCKeyWordSetMgr->SetKeyWordArr( i, nKeyWordNum, pszMem );
				}
			}
			delete [] pszMem;
		}
	}else{
		int nSize = pCKeyWordSetMgr->m_nKeyWordSetNum;
		for( i = 0; i < nSize; ++i ){
			wsprintf( szKeyName, "szSN[%02d]", i );
			cProfile.IOProfileData( pszSecName, szKeyName,
				pCKeyWordSetMgr->m_szSetNameArr[i],
				sizeof( pCKeyWordSetMgr->m_szSetNameArr[0] ));
			wsprintf( szKeyName, "nCASE[%02d]", i );
			cProfile.IOProfileData( pszSecName, szKeyName, pCKeyWordSetMgr->m_nKEYWORDCASEArr[i] );
			wsprintf( szKeyName, "nKWN[%02d]", i );
			cProfile.IOProfileData( pszSecName, szKeyName, pCKeyWordSetMgr->m_nKeyWordNumArr[i] );
			
			int nMemLen = 0;
			for( j = 0; j < pCKeyWordSetMgr->m_nKeyWordNumArr[i]; ++j ){
				nMemLen += strlen( pCKeyWordSetMgr->GetKeyWord( i, j ) );
				nMemLen ++;
			}
			nMemLen ++;
			wsprintf( szKeyName, "szKW[%02d].Size", i );
			cProfile.IOProfileData( pszSecName, szKeyName, nMemLen );
			char* pszMem = new char[nMemLen + 1];	//	May 25, 2003 genta ��؂��TAB�ɕύX�����̂ŁC�Ō��\0�̕���ǉ�
			char* pMem = pszMem;
			for( j = 0; j < pCKeyWordSetMgr->m_nKeyWordNumArr[i]; ++j ){
				//	May 25, 2003 genta ��؂��TAB�ɕύX
				int kwlen = strlen( pCKeyWordSetMgr->GetKeyWord( i, j ) );
				memcpy( pMem, pCKeyWordSetMgr->GetKeyWord( i, j ), kwlen );
				pMem += kwlen;
				*pMem++ = '\t';
			}
			*pMem = '\0';
			wsprintf( szKeyName, "szKW[%02d]", i );
			cProfile.IOProfileData( pszSecName, szKeyName, pszMem, nMemLen );
			delete [] pszMem;
		}
	}
}

/*!
	@brief ���L�f�[�^��Macro�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Macro( CProfile& cProfile )
{
	const char* pszSecName = "Macro";
	int		i;	
	char	szKeyName[64];
	for( i = 0; i < MAX_CUSTMACRO; ++i ){
		MacroRec& macrorec = m_pShareData->m_MacroTable[i];
		//	Oct. 4, 2001 genta ���܂�Ӗ����Ȃ������Ȃ̂ō폜�F3�s
		// 2002.02.08 hor ����`�l�𖳎�
		if( !cProfile.IsReadingMode() && !lstrlen(macrorec.m_szName) && !lstrlen(macrorec.m_szFile) ) continue;
		wsprintf( szKeyName, "Name[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_szName, MACRONAME_MAX - 1 );
		wsprintf( szKeyName, "File[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_szFile, _MAX_PATH );
		wsprintf( szKeyName, "ReloadWhenExecute[%03d]", i );
		cProfile.IOProfileData( pszSecName, szKeyName, macrorec.m_bReloadWhenExecute );
	}
}

/*!
	@brief ���L�f�[�^��Other�Z�N�V�����̓��o��
	@param[in,out]	cProfile	INI�t�@�C�����o�̓N���X

	@date 2005-04-07 D.S.Koba ShareData_IO_2���番���B
*/
void CShareData::ShareData_IO_Other( CProfile& cProfile )
{
	const char* pszSecName = "Other";	//�Z�N�V������1�쐬�����B2003.05.12 MIK
	int		i;	
	char	szKeyName[64];

	/* **** ���̑��̃_�C�A���O **** */
	/* �O���R�}���h���s�́u�W���o�͂𓾂�v */
	cProfile.IOProfileData( pszSecName, "bGetStdout"		, m_pShareData->m_bGetStdout );
	/* �w��s�փW�����v�́u���s�P�ʂ̍s�ԍ��v���u�܂�Ԃ��P�ʂ̍s�ԍ��v�� */
	cProfile.IOProfileData( pszSecName, "bLineNumIsCRLF"	, m_pShareData->m_bLineNumIsCRLF );
	
	/* DIFF�����\�� */	//@@@ 2002.05.27 MIK
	cProfile.IOProfileData( pszSecName, "nDiffFlgOpt"	, m_pShareData->m_nDiffFlgOpt );
	
	/* CTAGS */	//@@@ 2003.05.12 MIK
	cProfile.IOProfileData( pszSecName, "nTagsOpt"		, m_pShareData->m_nTagsOpt );
	cProfile.IOProfileData( pszSecName, "szTagsCmdLine"	, m_pShareData->m_szTagsCmdLine, sizeof( m_pShareData->m_szTagsCmdLine ) );
	
	//From Here 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v
	cProfile.IOProfileData( pszSecName, "_TagJumpKeyword_Counts", m_pShareData->m_nTagJumpKeywordArrNum );
	for( i = 0; i < m_pShareData->m_nTagJumpKeywordArrNum; ++i ){
		wsprintf( szKeyName, "TagJumpKeyword[%02d]", i );
		if( i >= m_pShareData->m_nTagJumpKeywordArrNum ){
			strcpy( m_pShareData->m_szTagJumpKeywordArr[i], "" );
		}
		cProfile.IOProfileData( pszSecName, szKeyName,
			m_pShareData->m_szTagJumpKeywordArr[i], sizeof( m_pShareData->m_szTagJumpKeywordArr[0] ));
	}
	cProfile.IOProfileData( pszSecName, "m_bTagJumpICase"		, m_pShareData->m_bTagJumpICase );
	cProfile.IOProfileData( pszSecName, "m_bTagJumpAnyWhere"		, m_pShareData->m_bTagJumpAnyWhere );
	//From Here 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v��

	//	MIK �o�[�W�������i�������݂̂݁j
	if( ! cProfile.IsReadingMode() ){
		TCHAR	iniVer[256];
		wsprintf( iniVer, _T("%d.%d.%d.%d"), 
					HIWORD( m_pShareData->m_dwProductVersionMS ),
					LOWORD( m_pShareData->m_dwProductVersionMS ),
					HIWORD( m_pShareData->m_dwProductVersionLS ),
					LOWORD( m_pShareData->m_dwProductVersionLS ) );
		cProfile.IOProfileData( pszSecName, _T("szVersion")	, iniVer, sizeof( iniVer ) );
	}
}

/*!
	@brief �F�ݒ� I/O

	�w�肳�ꂽ�F�ݒ���w�肳�ꂽ�Z�N�V�����ɏ������ށB�܂���
	�w�肳�ꂽ�Z�N�V�������炢��ݒ��ǂݍ��ށB

	@param[in,out]	pcProfile		�����o���A�ǂݍ��ݐ�Profile object (���o�͕�����bRead�Ɉˑ�)
	@param[in]		pszSecName		�Z�N�V������
	@param[in,out]	pColorInfoArr	�����o���A�ǂݍ��ݑΏۂ̐F�ݒ�ւ̃|�C���^ (���o�͕�����bRead�Ɉˑ�)
*/
void CShareData::IO_ColorSet( CProfile* pcProfile, const char* pszSecName, ColorInfo* pColorInfoArr )
{
	char	szKeyName[256];
	char	szKeyData[1024];
	int		j;
	for( j = 0; j < COLORIDX_LAST; ++j ){
		static const char* pszForm = "%d,%d,%06x,%06x,%d";
#ifndef STR_COLORDATA_HEAD3
		wsprintf( szKeyName, "CI[%02d]", j );
#else
		wsprintf( szKeyName, "C[%s]", g_ColorAttributeArr[j].szName );	//Stonee, 2001/01/12, 2001/01/15
#endif
		if( pcProfile->IsReadingMode() ){
			if( true == pcProfile->IOProfileData( pszSecName, szKeyName, szKeyData, sizeof( szKeyData )) ){
				pColorInfoArr[j].m_bUnderLine = FALSE;
				sscanf( szKeyData, pszForm,
					&pColorInfoArr[j].m_bDisp,
					&pColorInfoArr[j].m_bFatFont,
					&pColorInfoArr[j].m_colTEXT,
					&pColorInfoArr[j].m_colBACK,
					&pColorInfoArr[j].m_bUnderLine
				 );
			}else{
				// 2006.12.07 ryoji
				// sakura Ver1.5.13.1 �ȑO��ini�t�@�C����ǂ񂾂Ƃ��ɃL�����b�g���e�L�X�g�w�i�F�Ɠ����ɂȂ��
				// ������ƍ���̂ŃL�����b�g�F���ǂ߂Ȃ��Ƃ��̓L�����b�g�F���e�L�X�g�F�Ɠ����ɂ���
				if( COLORIDX_CARET == j )
					pColorInfoArr[j].m_colTEXT = pColorInfoArr[COLORIDX_TEXT].m_colTEXT;
			}
			// 2006.12.18 ryoji
			// �����ݒ肪����ΏC������
			unsigned int fAttribute = g_ColorAttributeArr[j].fAttribute;
			if( 0 != (fAttribute & COLOR_ATTRIB_FORCE_DISP) )
				pColorInfoArr[j].m_bDisp = TRUE;
			if( 0 != (fAttribute & COLOR_ATTRIB_NO_BOLD) )
				pColorInfoArr[j].m_bFatFont = FALSE;
			if( 0 != (fAttribute & COLOR_ATTRIB_NO_UNDERLINE) )
				pColorInfoArr[j].m_bUnderLine = FALSE;
		}else{
			wsprintf( szKeyData, pszForm,
				pColorInfoArr[j].m_bDisp,
				pColorInfoArr[j].m_bFatFont,
				pColorInfoArr[j].m_colTEXT,
				pColorInfoArr[j].m_colBACK,
				pColorInfoArr[j].m_bUnderLine
			);
			pcProfile->IOProfileData( pszSecName, szKeyName, szKeyData, 0 );
		}
	}
	return;
}


/*!
	@brief �^�O�W�����v���̕ۑ�

	�^�O�W�����v����Ƃ��ɁA�^�O�W�����v��̏���ۑ�����B

	@param[in] pTagJump �ۑ�����^�O�W�����v���
	@retval true	�ۑ�����
	@retval false	�ۑ����s

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

	@param[out] pTagJump �Q�Ƃ���^�O�W�����v���
	@retval true	�Q�Ɛ���
	@retval false	�Q�Ǝ��s

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
