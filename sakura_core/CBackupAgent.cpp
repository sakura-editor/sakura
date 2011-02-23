#include "StdAfx.h"
#include "CBackupAgent.h"
#include "window/CEditWnd.h"
#include "util/format.h" //GetDateTimeFormat

ECallbackResult CBackupAgent::OnPreBeforeSave(SSaveInfo* pSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//�V�����t�@�C�������ꍇ�͉������Ȃ�
	if(!fexist(pSaveInfo->cFilePath))return CALLBACK_CONTINUE;

	//���ʐݒ�F�ۑ����Ƀo�b�N�A�b�v���쐬����
	if( GetDllShareData().m_Common.m_sBackup.m_bBackUp ){
		//	Jun.  5, 2004 genta �t�@�C������^����悤�ɁD�߂�l�ɉ�����������ǉ��D
		// �t�@�C���ۑ��O�Ƀo�b�N�A�b�v����
		int nBackupResult = 0;
		{
			pcDoc->m_cDocFileOperation.DoFileUnlock();	//�o�b�N�A�b�v�쐬�O�Ƀ��b�N���������� #####�X�}�[�g����Ȃ���I
			nBackupResult = MakeBackUp( pSaveInfo->cFilePath );
			pcDoc->m_cDocFileOperation.DoFileLock();	//�o�b�N�A�b�v�쐬��Ƀ��b�N��߂� #####�X�}�[�g����Ȃ���I
		}
		switch( nBackupResult ){
		case 2:	//	���f�w��
			return CALLBACK_INTERRUPT;
		case 3: //	�t�@�C���G���[
			if( IDYES != ::MYMESSAGEBOX(
				CEditWnd::Instance()->GetHwnd(),
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				_T("�t�@�C���ۑ�"),
				_T("�o�b�N�A�b�v�̍쐬�Ɏ��s���܂����D���t�@�C���ւ̏㏑�����p�����čs���܂����D")
			)){
				return CALLBACK_INTERRUPT;
			}
			break;
		}
	}
	return CALLBACK_CONTINUE;
}





/*! �o�b�N�A�b�v�̍쐬
	@author genta
	@date 2001.06.12 asa-o
		�t�@�C���̎��������Ƀo�b�N�A�b�v�t�@�C�������쐬����@�\
	@date 2001.12.11 MIK �o�b�N�A�b�v�t�@�C�����S�~���ɓ����@�\
	@date 2004.06.05 genta �o�b�N�A�b�v�Ώۃt�@�C���������ŗ^����悤�ɁD
		���O��t���ĕۑ��̎��͎����̃o�b�N�A�b�v������Ă����Ӗ��Ȃ̂ŁD
		�܂��C�o�b�N�A�b�v���ۑ����s��Ȃ��I������ǉ��D
	@date 2005.11.26 aroka �t�@�C����������FormatBackUpPath�ɕ���

	@retval 0 �o�b�N�A�b�v�쐬���s�D
	@retval 1 �o�b�N�A�b�v�쐬�����D
	@retval 2 �o�b�N�A�b�v�쐬���s�D�ۑ����f�w���D
	@retval 3 �t�@�C������G���[�ɂ��o�b�N�A�b�v�쐬���s�D
	
	@todo Advanced mode�ł̐���Ǘ�
*/
int CBackupAgent::MakeBackUp(
	const TCHAR* target_file
)
{
	int		nRet;

	/* �o�b�N�A�b�v�\�[�X�̑��݃`�F�b�N */
	//	Aug. 21, 2005 genta �������݃A�N�Z�X�����Ȃ��ꍇ��
	//	�t�@�C�����Ȃ��ꍇ�Ɠ��l�ɉ������Ȃ�
	if( (_taccess( target_file, 2 )) == -1 ){
		return 0;
	}

	const CommonSetting_Backup& bup_setting = GetDllShareData().m_Common.m_sBackup;

	TCHAR	szPath[_MAX_PATH];
	if( false == FormatBackUpPath( szPath, _countof(szPath), target_file ) ){
		int nMsgResult = ::TopConfirmMessage(
			CEditWnd::Instance()->GetHwnd(),
			_T("�o�b�N�A�b�v��̃p�X�쐬���ɃG���[�ɂȂ�܂����B\n")
			_T("�p�X���������邩�s���ȏ����ł��B\n")
			_T("�o�b�N�A�b�v���쐬�����ɏ㏑���ۑ����Ă�낵���ł����H")
		);
		if( nMsgResult == IDYES ){
			return 0;//	�ۑ��p��
		}
		return 2;// �ۑ����f
	}

	//@@@ 2002.03.23 start �l�b�g���[�N�E�����[�o�u���h���C�u�̏ꍇ�͂��ݔ��ɕ��荞�܂Ȃ�
	bool dustflag = false;
	if( bup_setting.m_bBackUpDustBox ){
		dustflag = !IsLocalDrive( szPath );
	}
	//@@@ 2002.03.23 end

	if( bup_setting.m_bBackUpDialog ){	/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
		::MessageBeep( MB_ICONQUESTION );
		if( bup_setting.m_bBackUpDustBox && !dustflag ){	//���ʐݒ�F�o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	//@@@ 2001.12.11 add start MIK	//2002.03.23
			nRet = ::MYMESSAGEBOX(
				CEditWnd::Instance()->GetHwnd(),
				MB_YESNO/*CANCEL*/ | MB_ICONQUESTION | MB_TOPMOST,
				_T("�o�b�N�A�b�v�쐬�̊m�F"),
				_T("�ύX�����O�ɁA�o�b�N�A�b�v�t�@�C�����쐬���܂��B\n")
				_T("��낵���ł����H  [������(N)] ��I�Ԃƍ쐬�����ɏ㏑���i�܂��͖��O��t���āj�ۑ��ɂȂ�܂��B\n")
				_T("\n")
				_T("%ts\n")
				_T("    ��\n")
				_T("%ts\n")
				_T("\n")
				_T("�쐬�����o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞�݂܂��B\n"),
				target_file,
				szPath
			);
		}
		else{	//@@@ 2001.12.11 add end MIK
			nRet = ::MYMESSAGEBOX(
				CEditWnd::Instance()->GetHwnd(),
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				_T("�o�b�N�A�b�v�쐬�̊m�F"),
				_T("�ύX�����O�ɁA�o�b�N�A�b�v�t�@�C�����쐬���܂��B\n")
				_T("��낵���ł����H  [������(N)] ��I�Ԃƍ쐬�����ɏ㏑���i�܂��͖��O��t���āj�ۑ��ɂȂ�܂��B\n")
				_T("\n")
				_T("%ts\n")
				_T("    ��\n")
				_T("%ts\n")
				_T("\n"),
				target_file,
				szPath
			);	//Jul. 06, 2001 jepro [���O��t���ĕۑ�] �̏ꍇ������̂Ń��b�Z�[�W���C��
		}	//@@@ 2001.12.11 add MIK
		//	Jun.  5, 2005 genta �߂�l�ύX
		if( IDNO == nRet ){
			return 0;//	�ۑ��p��
		}else if( IDCANCEL == nRet ){
			return 2;// �ۑ����f
		}
//To Here Feb. 27, 2001
	}

	//	From Here Aug. 16, 2000 genta
	//	Jun.  5, 2005 genta 1�̊g���q���c���ł�ǉ�
	if( bup_setting.GetBackupType() == 3 ||
		bup_setting.GetBackupType() == 6 ){
		//	���ɑ��݂���Backup�����炷����
		int				i;

		//	�t�@�C�������p
		HANDLE			hFind;
		WIN32_FIND_DATA	fData;

		TCHAR*	pBase = szPath + _tcslen( szPath ) - 2;	//	2: �g���q�̍Ō��2���̈Ӗ�

		//------------------------------------------------------------------
		//	1. �Y���f�B���N�g������backup�t�@�C����1���T��
		for( i = 0; i <= 99; i++ ){	//	�ő�l�Ɋւ�炸�C99�i2���̍ő�l�j�܂ŒT��
			//	�t�@�C�������Z�b�g
			auto_sprintf( pBase, _T("%02d"), i );

			hFind = ::FindFirstFile( szPath, &fData );
			if( hFind == INVALID_HANDLE_VALUE ){
				//	�����Ɏ��s���� == �t�@�C���͑��݂��Ȃ�
				break;
			}
			::FindClose( hFind );
			//	���������t�@�C���̑������`�F�b�N
			//	�͖ʓ|���������炵�Ȃ��D
			//	�������O�̃f�B���N�g������������ǂ��Ȃ�̂��낤...
		}
		--i;

		//------------------------------------------------------------------
		//	2. �ő�l���琧����-1�Ԃ܂ł��폜
		int boundary = bup_setting.GetBackupCount();
		boundary = boundary > 0 ? boundary - 1 : 0;	//	�ŏ��l��0

		for( ; i >= boundary; --i ){
			//	�t�@�C�������Z�b�g
			auto_sprintf( pBase, _T("%02d"), i );
			if( ::DeleteFile( szPath ) == 0 ){
				::MessageBox( CEditWnd::Instance()->GetHwnd(), szPath, _T("�폜���s"), MB_OK );
				//	Jun.  5, 2005 genta �߂�l�ύX
				//	���s���Ă��ۑ��͌p��
				return 0;
				//	���s�����ꍇ
				//	��ōl����
			}
		}

		//	���̈ʒu��i�͑��݂���o�b�N�A�b�v�t�@�C���̍ő�ԍ���\���Ă���D

		//	3. ��������0�Ԃ܂ł̓R�s�[���Ȃ���ړ�
		TCHAR szNewPath[MAX_PATH];
		TCHAR *pNewNrBase;

		_tcscpy( szNewPath, szPath );
		pNewNrBase = szNewPath + _tcslen( szNewPath ) - 2;

		for( ; i >= 0; --i ){
			//	�t�@�C�������Z�b�g
			auto_sprintf( pBase, _T("%02d"), i );
			auto_sprintf( pNewNrBase, _T("%02d"), i + 1 );

			//	�t�@�C���̈ړ�
			if( ::MoveFile( szPath, szNewPath ) == 0 ){
				//	���s�����ꍇ
				//	��ōl����
				::MessageBox( CEditWnd::Instance()->GetHwnd(), szPath, _T("�ړ����s"), MB_OK );
				//	Jun.  5, 2005 genta �߂�l�ύX
				//	���s���Ă��ۑ��͌p��
				return 0;
			}
		}
	}
	//	To Here Aug. 16, 2000 genta

	/* �o�b�N�A�b�v�̍쐬 */
	//	Aug. 21, 2005 genta ���݂̃t�@�C���ł͂Ȃ��^�[�Q�b�g�t�@�C�����o�b�N�A�b�v����悤��
	TCHAR	szDrive[_MAX_DIR];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	_tsplitpath( szPath, szDrive, szDir, szFname, szExt );
	TCHAR	szPath2[MAX_PATH];
	auto_sprintf( szPath2, _T("%ts%ts"), szDrive, szDir );

	HANDLE			hFind;
	WIN32_FIND_DATA	fData;

	hFind = ::FindFirstFile( szPath2, &fData );
	if( hFind == INVALID_HANDLE_VALUE ){
		//	�����Ɏ��s���� == �t�@�C���͑��݂��Ȃ�
		::CreateDirectory( szPath2, NULL );
	}
	::FindClose( hFind );

	if( ::CopyFile( target_file, szPath, FALSE ) ){
		/* ����I�� */
		//@@@ 2001.12.11 start MIK
		if( bup_setting.m_bBackUpDustBox && !dustflag ){	//@@@ 2002.03.23 �l�b�g���[�N�E�����[�o�u���h���C�u�łȂ�
			TCHAR	szDustPath[_MAX_PATH+1];
			_tcscpy(szDustPath, szPath);
			szDustPath[_tcslen(szDustPath) + 1] = _T('\0');
			SHFILEOPSTRUCT	fos;
			fos.hwnd   = CEditWnd::Instance()->GetHwnd();
			fos.wFunc  = FO_DELETE;
			fos.pFrom  = szDustPath;
			fos.pTo    = NULL;
			fos.fFlags = FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMATION;	//�_�C�A���O�Ȃ�
			fos.fAnyOperationsAborted = true; //false;
			fos.hNameMappings = NULL;
			fos.lpszProgressTitle = NULL; //"�o�b�N�A�b�v�t�@�C�������ݔ��Ɉړ����Ă��܂�...";
			if( ::SHFileOperation(&fos) == 0 ){
				/* ����I�� */
			}else{
				/* �G���[�I�� */
			}
		}
		//@@@ 2001.12.11 end MIK
	}else{
		/* �G���[�I�� */
		//	Jun.  5, 2005 genta �߂�l�ύX
		return 3;
	}
	//	Jun.  5, 2005 genta �߂�l�ύX
	return 1;
}




// CInvalidParameterHandler: CRT ���ł̃G���[���o������肵�Ĉُ�I����h�����߂̃N���X
// �� �}���`�X���b�h�ɂ͔�Ή�
class CInvalidParameterHandler{
public:
	CInvalidParameterHandler()
	{
		bInvalid = false;	// �G���[�̗L��
		oldParamFunc = _set_invalid_parameter_handler( newParamFunc );	// Release �p�i�����p�����[�^�n���h���j
		oldRepoFunc = _CrtSetReportHook( newRepoFunc );					// Debug �p�i�f�o�b�O���|�[�g�t�b�N�j
	}
	~CInvalidParameterHandler()
	{
		_set_invalid_parameter_handler(oldParamFunc);
		_CrtSetReportHook(oldRepoFunc);
	}
	bool IsInvalid(){ return bInvalid; }
private:
	static void newParamFunc(
		const wchar_t* expression,
		const wchar_t* function,
		const wchar_t* file,
		unsigned int line,
		uintptr_t pReserved)
	{
		bInvalid = true;
	}
	static int newRepoFunc( int reportType, char *message, int *returnValue )
	{
		bInvalid = true;
		return TRUE;
	}
	static bool bInvalid;
	static _invalid_parameter_handler oldParamFunc;
	static _CRT_REPORT_HOOK oldRepoFunc;
};
bool CInvalidParameterHandler::bInvalid;
_invalid_parameter_handler CInvalidParameterHandler::oldParamFunc;
_CRT_REPORT_HOOK CInvalidParameterHandler::oldRepoFunc;


/*! �o�b�N�A�b�v�̍쐬

	@author aroka
	@date 2005.11.29 aroka
		MakeBackUp���番���D���������Ƀo�b�N�A�b�v�t�@�C�������쐬����@�\�ǉ�

	@retval true  ����
	@retval false �o�b�t�@�s��
	
	@todo Advanced mode�ł̐���Ǘ�
*/
bool CBackupAgent::FormatBackUpPath(
	TCHAR*			szNewPath,	//!< [out] szNewPath �o�b�N�A�b�v��p�X��
	size_t 			newPathCount,	//!< [in] szNewPath�̃T�C�Y
	const TCHAR*	target_file	//!< [in]  target_file �o�b�N�A�b�v���p�X��
)
{
	TCHAR	szDrive[_MAX_DIR];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	TCHAR*	psNext;

	const CommonSetting_Backup& bup_setting = GetDllShareData().m_Common.m_sBackup;

	/* �p�X�̕��� */
	_tsplitpath( target_file, szDrive, szDir, szFname, szExt );

	if( bup_setting.m_bBackUpFolder
	  && (!bup_setting.m_bBackUpFolderRM || !IsLocalDrive( target_file ))) {	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */	// m_bBackUpFolderRM �ǉ�	2010/5/27 Uchi
		if (GetFullPathName(bup_setting.m_szBackUpFolder, _MAX_PATH, szNewPath, & psNext) == 0) {
			// ���܂����Ȃ�����
			_tcscpy( szNewPath, bup_setting.m_szBackUpFolder );
		}
		/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
		AddLastYenFromDirectoryPath( szNewPath );
	}
	else{
		auto_sprintf( szNewPath, _T("%ts%ts"), szDrive, szDir );
	}

	/* ���΃t�H���_��}�� */
	if( !bup_setting.m_bBackUpPathAdvanced ){

		time_t	ltime;
		struct	tm *today, *gmt;
		wchar_t	szTime[64];
		wchar_t	szForm[64];

		TCHAR*	pBase;
		int     nBaseCount;
		pBase = szNewPath + _tcslen( szNewPath );
		nBaseCount = newPathCount - _tcslen( szNewPath );

		/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
		switch( bup_setting.GetBackupType() ){
		case 1:
			if( -1 == auto_snprintf_s( pBase, nBaseCount, _T("%ts.bak"), szFname ) ){
				return false;
			}
			break;
		case 5: //	Jun.  5, 2005 genta 1�̊g���q���c����
			if( -1 == auto_snprintf_s( pBase, nBaseCount, _T("%ts%ts.bak"), szFname, szExt ) ){
				return false;
			}
			break;
		case 2:	//	���t�C����
			_tzset();
			_wstrdate( szTime );
			time( &ltime );				/* �V�X�e�������𓾂܂� */
			gmt = gmtime( &ltime );		/* �����W�����ɕϊ����� */
			today = localtime( &ltime );/* ���n���Ԃɕϊ����� */

			wcscpy( szForm, L"" );
			if( bup_setting.GetBackupOpt(BKUP_YEAR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
				wcscat( szForm, L"%Y" );
			}
			if( bup_setting.GetBackupOpt(BKUP_MONTH) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
				wcscat( szForm, L"%m" );
			}
			if( bup_setting.GetBackupOpt(BKUP_DAY) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
				wcscat( szForm, L"%d" );
			}
			if( bup_setting.GetBackupOpt(BKUP_HOUR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
				wcscat( szForm, L"%H" );
			}
			if( bup_setting.GetBackupOpt(BKUP_MIN) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
				wcscat( szForm, L"%M" );
			}
			if( bup_setting.GetBackupOpt(BKUP_SEC) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
				wcscat( szForm, L"%S" );
			}
			/* YYYYMMDD�����b �`���ɕϊ� */
			wcsftime( szTime, _countof( szTime ) - 1, szForm, today );
			if( -1 == auto_snprintf_s( pBase, nBaseCount, _T("%ts_%ls%ts"), szFname, szTime, szExt ) ){
				return false;
			}
			break;
	//	2001/06/12 Start by asa-o: �t�@�C���ɕt������t��O��̕ۑ���(�X�V����)�ɂ���
		case 4:	//	���t�C����
			{
				CFileTime ctimeLastWrite;
				GetLastWriteTimestamp( target_file, &ctimeLastWrite );

				wcscpy( szTime, L"" );
				if( bup_setting.GetBackupOpt(BKUP_YEAR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
					auto_sprintf(szTime,L"%d",ctimeLastWrite->wYear);
				}
				if( bup_setting.GetBackupOpt(BKUP_MONTH) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
					auto_sprintf(szTime,L"%ls%02d",szTime,ctimeLastWrite->wMonth);
				}
				if( bup_setting.GetBackupOpt(BKUP_DAY) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
					auto_sprintf(szTime,L"%ls%02d",szTime,ctimeLastWrite->wDay);
				}
				if( bup_setting.GetBackupOpt(BKUP_HOUR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
					auto_sprintf(szTime,L"%ls%02d",szTime,ctimeLastWrite->wHour);
				}
				if( bup_setting.GetBackupOpt(BKUP_MIN) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
					auto_sprintf(szTime,L"%ls%02d",szTime,ctimeLastWrite->wMinute);
				}
				if( bup_setting.GetBackupOpt(BKUP_SEC) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
					auto_sprintf(szTime,L"%ls%02d",szTime,ctimeLastWrite->wSecond);
				}
				if( -1 == auto_sprintf_s( pBase, nBaseCount, _T("%ts_%ls%ts"), szFname, szTime, szExt ) ){
					return false;
				}
			}
			break;
	// 2001/06/12 End

		case 3: //	?xx : xx = 00~99, ?�͔C�ӂ̕���
		case 6: //	Jun.  5, 2005 genta 3�̊g���q���c����
			//	Aug. 15, 2000 genta
			//	�����ł͍쐬����o�b�N�A�b�v�t�@�C�����̂ݐ�������D
			//	�t�@�C������Rotation�͊m�F�_�C�A���O�̌�ōs���D
			{
				//	Jun.  5, 2005 genta �g���q���c����悤�ɏ����N�_�𑀍삷��
				TCHAR* ptr;
				if( bup_setting.GetBackupType() == 3 ){
					ptr = szExt;
				}
				else {
					ptr = szExt + _tcslen( szExt );
				}
				*ptr   = _T('.');
				*++ptr = bup_setting.GetBackupExtChar();
				*++ptr = _T('0');
				*++ptr = _T('0');
				*++ptr = _T('\0');
			}
			if( -1 == auto_snprintf_s( pBase, nBaseCount, _T("%ts%ts"), szFname, szExt ) ){
				return false;
			}
			break;
		}

	}else{ // �ڍאݒ�g�p����
		TCHAR szFormat[1024];

		switch( bup_setting.GetBackupTypeAdv() ){
		case 4:	//	�t�@�C���̓��t�C����
			{
				// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX
				CFileTime ctimeLastWrite;
				GetLastWriteTimestamp( target_file, &ctimeLastWrite );
				if( !GetDateTimeFormat( szFormat, _countof(szFormat), bup_setting.m_szBackUpPathAdvanced , ctimeLastWrite.GetSYSTEMTIME() ) ){
					return false;
				}
			}
			break;
		case 2:	//	���݂̓��t�C����
		default:
			{
				time_t	ltime;
				struct	tm *today;

				time( &ltime );				/* �V�X�e�������𓾂܂� */
				today = localtime( &ltime );/* ���n���Ԃɕϊ����� */

				/* YYYYMMDD�����b �`���ɕϊ� */
				CInvalidParameterHandler cInvalidParam;	// CRT ���G���[���o�N���X	// 2011.02.23 ryoji Wiki BugReport/67 �̏C��
				_tcsftime( szFormat, _countof( szFormat ) - 1, bup_setting.m_szBackUpPathAdvanced , today );
				if( cInvalidParam.IsInvalid() )
					return false;	// _tcsftime() �������Ȉ��������o����
			}
			break;
		}

		{
			// make keys
			// $0-$9�ɑΉ�����t�H���_����؂�o��
			TCHAR keybuff[1024];
			_tcscpy( keybuff, szDir );
			CutLastYenFromDirectoryPath( keybuff );

			TCHAR *folders[10];
			{
				//	Jan. 9, 2006 genta VC6�΍�
				int idx;
				for( idx=0; idx<10; ++idx ){
					folders[idx] = 0;
				}
				folders[0] = szFname;

				for( idx=1; idx<10; ++idx ){
					TCHAR *cp;
					cp = _tcsrchr(keybuff, _T('\\'));
					if( cp != NULL ){
						folders[idx] = cp+1;
						*cp = _T('\0');
					}
					else{
						break;
					}
				}
			}
			{
				// $0-$9��u��
				//wcscpy( szNewPath, L"" );
				TCHAR *q= szFormat;
				TCHAR *q2 = szFormat;
				while( *q ){
					if( *q==_T('$') ){
						++q;
						if( isdigit(*q) ){
							q[-1] = _T('\0');
							_tcscat( szNewPath, q2 );
//							if( newPathCount <  auto_strlcat( szNewPath, q2, newPathCount ) ){
//								return false;
//							}
							if( folders[*q-_T('0')] != 0 ){
								_tcscat( szNewPath, folders[*q-_T('0')] );
//								if( newPathCount < auto_strlcat( szNewPath, folders[*q-_T('0')], newPathCount ) ){
//									return false;
//								}
							}
							q2 = q+1;
						}
					}
					++q;
				}
				_tcscat( szNewPath, q2 );
//				if( newPathCount < auto_strlcat( szNewPath, q2, newPathCount ) ){
//					return false;
//				}
			}
		}
		{
			TCHAR temp[1024];
			TCHAR *cp;
			//	2006.03.25 Aroka szExt[0] == '\0'�̂Ƃ��̃I�[�o���������C��
			TCHAR *ep = (szExt[0]!=0) ? &szExt[1] : &szExt[0];
			assert( newPathCount <= _countof(temp) );

			// * ���g���q�ɂ���
			while( _tcschr( szNewPath, _T('*') ) ){
				_tcscpy( temp, szNewPath );
				cp = _tcschr( temp, _T('*') );
				*cp = 0;
				if( -1 == auto_snprintf_s( szNewPath, newPathCount, _T("%ts%ts%ts"), temp, ep, cp+1 ) ){
					return false;
				}
			}
			//	??�̓o�b�N�A�b�v�A�Ԃɂ������Ƃ���ł͂��邪�C
			//	�A�ԏ����͖�����2���ɂ����Ή����Ă��Ȃ��̂�
			//	�g�p�ł��Ȃ�����?��_�ɕϊ����Ă��������
			while(( cp = _tcschr( szNewPath, _T('?') ) ) != NULL){
				*cp = _T('_');
			}
		}
	}
	return true;
}



