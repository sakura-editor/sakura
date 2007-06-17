/*!	@file
	@brief �G�f�B�^�v���Z�X�N���X

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CProcess��蕪��
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, genta, Moca, MIK
	Copyright (C) 2004, Moca, naoh

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "stdafx.h"
#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CEditApp.h"
#include "CShareData.h"
#include "Debug.h"
#include "etc_uty.h"
#include "CEditWnd.h" // 2002/2/3 aroka
#include "mymessage.h" // 2002/2/3 aroka
#include "CDocLine.h" // 2003/03/28 MIK
#include <tchar.h>
#include "CRunningTimer.h"


/*!
	@brief �G�f�B�^�v���Z�X������������
	
	CEditWnd���쐬����B
	
	@author aroka
	@date 2002/01/07

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 2004.05.13 Moca CEditWnd::Create()�Ɏ��s�����ꍇ��false��Ԃ��悤�ɁD
*/
bool CNormalProcess::Initialize()
{
	MY_RUNNINGTIMER( cRunningTimer, "NormalProcess::Init" );

	HANDLE			hMutex;
	HWND			hWnd;

	/* �v���Z�X�������̖ڈ� */
	hMutex = GetInitializeMutex();	// 2002/2/8 aroka ���ݓ����Ă����̂ŕ���
	if( NULL == hMutex ){
		return false;
	}

	if ( CProcess::Initialize() == false ){
		return false;
	}

	/* �R�}���h���C���I�v�V���� */
	bool			bReadOnly;
	bool			bDebugMode;
	bool			bGrepMode;
	bool			bGrepDlg;
	GrepInfo		gi;
	FileInfo		fi;
	
	/* �R�}���h���C���Ŏ󂯎�����t�@�C�����J����Ă���ꍇ�� */
	/* ���̕ҏW�E�B���h�E���A�N�e�B�u�ɂ��� */
	CCommandLine::Instance()->GetFileInfo(fi); // 2002/2/8 aroka �����Ɉړ�
	if( 0 < strlen( fi.m_szPath ) ){
		//	Oct. 27, 2000 genta
		//	MRU����J�[�\���ʒu�𕜌����鑀���CEditDoc::FileRead��
		//	�s����̂ł����ł͕K�v�Ȃ��D

		HWND hwndOwner;
		/* �w��t�@�C�����J����Ă��邩���ׂ� */
		// 2007.03.13 maru �����R�[�h���قȂ�Ƃ��̓��[�j���O���o���悤��
		if( FALSE != m_cShareData.IsPathOpened( fi.m_szPath, &hwndOwner, fi.m_nCharCode ) ){
			//	From Here Oct. 19, 2001 genta
			//	�J�[�\���ʒu�������Ɏw�肳��Ă�����w��ʒu�ɃW�����v
			if( fi.m_nY >= 0 ){	//	�s�̎w�肪���邩
				POINT& pt = *(POINT*)CProcess::m_pShareData->m_szWork;
				if( fi.m_nX < 0 ){
					//	���̎w�肪�����ꍇ
					::SendMessage( hwndOwner, MYWM_GETCARETPOS, 0, 0 );
				}
				else {
					pt.x = fi.m_nX;
				}
				pt.y = fi.m_nY;
				::SendMessage( hwndOwner, MYWM_SETCARETPOS, 0, 0 );
			}
			//	To Here Oct. 19, 2001 genta
			/* �A�N�e�B�u�ɂ��� */
			ActivateFrameWindow( hwndOwner );
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			return false;
		}else{

		}
	}

	MY_TRACETIME( cRunningTimer, "CheckFile" );
//�����v���Z�X��
	/* �G�f�B�^�E�B���h�E�I�u�W�F�N�g���쐬 */
	m_pcEditWnd = new CEditWnd;
	MY_TRACETIME( cRunningTimer, "CEditWnd Created" );

	/* �R�}���h���C���̉�� */	 // 2002/2/8 aroka �����Ɉړ�
	bDebugMode = CCommandLine::Instance()->IsDebugMode();
	bGrepMode = CCommandLine::Instance()->IsGrepMode();
	bGrepDlg = CCommandLine::Instance()->IsGrepDlg();
	
	if( bDebugMode ){
		hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );

//	#ifdef _DEBUG/////////////////////////////////////////////
		/* �f�o�b�O���j�^���[�h�ɐݒ� */
		m_pcEditWnd->SetDebugModeON();
		// 2004.09.20 naoh �A�E�g�v�b�g�p�^�C�v�ʐݒ�
		m_pcEditWnd->m_cEditDoc.SetDocumentType( m_cShareData.GetDocumentTypeExt("output"), true );
//	#endif////////////////////////////////////////////////////
	}else
	if( bGrepMode ){
		hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );
		// 2004.05.13 Moca CEditWnd::Create()�Ɏ��s�����ꍇ�̍l����ǉ�
		if( NULL == hWnd ){
			goto end_of_func;
		}
		/* GREP */
		CCommandLine::Instance()->GetGrepInfo(gi); // 2002/2/8 aroka �����Ɉړ�
		if( false == bGrepDlg ){
			TCHAR szWork[MAX_PATH];
			/* �����O�t�@�C�������擾���� */
			if( FALSE != ::GetLongFileName( gi.cmGrepFolder.GetPtr(), szWork ) ){
				gi.cmGrepFolder.SetData( szWork, strlen( szWork ) );
			}
			// 2003.06.23 Moca GREP���s�O��Mutex���J��
			//	�������Ȃ���Grep���I���܂ŐV�����E�B���h�E���J���Ȃ�
			m_hWnd = hWnd;
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].DoGrep(
				&gi.cmGrepKey,
				&gi.cmGrepFile,
				&gi.cmGrepFolder,
				gi.bGrepSubFolder,
				gi.bGrepNoIgnoreCase,
				gi.bGrepRegularExp,
				gi.nGrepCharSet,	//	2002/09/21 Moca
				gi.bGrepOutputLine,
				gi.bGrepWordOnly,	//	Jun. 26, 2001 genta
				gi.nGrepOutputStyle
			);
			return true; // 2003.06.23 Moca
		}else{
			//-GREPDLG�Ń_�C�A���O���o���B�@���������f�i2002/03/24 YAZAKI�j
			CShareData::getInstance()->AddToSearchKeyArr( gi.cmGrepKey.GetPtr() );
			CShareData::getInstance()->AddToGrepFileArr( gi.cmGrepFile.GetPtr() );
			CShareData::getInstance()->AddToGrepFolderArr( gi.cmGrepFolder.GetPtr() );
			m_pShareData->m_Common.m_bGrepSubFolder = gi.bGrepSubFolder;
			m_pShareData->m_Common.m_bLoHiCase = gi.bGrepNoIgnoreCase;
			m_pShareData->m_Common.m_bRegularExp = gi.bGrepRegularExp;
			m_pShareData->m_Common.m_nGrepCharSet = gi.nGrepCharSet;
			m_pShareData->m_Common.m_bGrepOutputLine = gi.bGrepOutputLine;
			m_pShareData->m_Common.m_bWordOnly = gi.bGrepWordOnly;
			m_pShareData->m_Common.m_nGrepOutputStyle = gi.nGrepOutputStyle;
			// 2003.06.23 Moca GREP�_�C�A���O�\���O��Mutex���J��
			//	�������Ȃ���Grep���I���܂ŐV�����E�B���h�E���J���Ȃ�
			m_hWnd = hWnd;
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			
			//	Oct. 9, 2003 genta �R�}���h���C������GERP�_�C�A���O��\���������ꍇ��
			//	�����̐ݒ肪BOX�ɔ��f����Ȃ�
			lstrcpy( m_pcEditWnd->m_cEditDoc.m_cDlgGrep.m_szText, gi.cmGrepKey.GetPtr() );		/* ���������� */
			lstrcpy( m_pcEditWnd->m_cEditDoc.m_cDlgGrep.m_szFile, gi.cmGrepFile.GetPtr() );		/* �����t�@�C�� */
			lstrcpy( m_pcEditWnd->m_cEditDoc.m_cDlgGrep.m_szFolder, gi.cmGrepFolder.GetPtr() );	/* �����t�H���_ */

			
			// Feb. 23, 2003 Moca Owner window���������w�肳��Ă��Ȃ�����
			int nRet = m_pcEditWnd->m_cEditDoc.m_cDlgGrep.DoModal( m_hInstance, hWnd,  NULL);
			if( FALSE != nRet ){
				m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].HandleCommand(F_GREP, TRUE, 0, 0, 0, 0);
			}
			return true; // 2003.06.23 Moca
		}
	}else{
		// 2004.05.13 Moca �����if���̒�����O�Ɉړ�
		// �t�@�C�������^�����Ȃ��Ă�ReadOnly�w���L���ɂ��邽�߁D
		bReadOnly = CCommandLine::Instance()->IsReadOnly(); // 2002/2/8 aroka �����Ɉړ�
		if( 0 < strlen( fi.m_szPath ) ){
			//	Mar. 9, 2002 genta �����^�C�v�w��
			hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray, 
							fi.m_szPath, fi.m_nCharCode, bReadOnly/* �ǂݎ���p�� */,
							fi.m_szDocType[0] == '\0' ? -1 :
								m_cShareData.GetDocumentTypeExt( fi.m_szDocType )
				 );
			// 2004.05.13 Moca CEditWnd::Create()�Ɏ��s�����ꍇ�̍l����ǉ�
			if( NULL == hWnd ){
				goto end_of_func;
			}
			//	Nov. 6, 2000 genta
			//	�L�����b�g�ʒu�̕����̂���
			//	�I�v�V�����w�肪�Ȃ��Ƃ��͉�ʈړ����s��Ȃ��悤�ɂ���
			//	Oct. 19, 2001 genta
			//	���ݒ聁-1�ɂȂ�悤�ɂ����̂ŁC���S�̂��ߗ��҂��w�肳�ꂽ�Ƃ�����
			//	�ړ�����悤�ɂ���D || �� &&
			if( ( 0 <= fi.m_nViewTopLine && 0 <= fi.m_nViewLeftCol )
				&& fi.m_nViewTopLine < m_pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
				m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nViewTopLine = fi.m_nViewTopLine;
				m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nViewLeftCol = fi.m_nViewLeftCol;
			}

			//	�I�v�V�����w�肪�Ȃ��Ƃ��̓J�[�\���ʒu�ݒ���s��Ȃ��悤�ɂ���
			//	Oct. 19, 2001 genta
			//	0���ʒu�Ƃ��Ă͗L���Ȓl�Ȃ̂Ŕ���Ɋ܂߂Ȃ��Ă͂Ȃ�Ȃ�
			if( 0 <= fi.m_nX || 0 <= fi.m_nY ){
				/*
				  �J�[�\���ʒu�ϊ�
				  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
				  ��
				  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
				*/
				int		nPosX;
				int		nPosY;
				m_pcEditWnd->m_cEditDoc.m_cLayoutMgr.CaretPos_Phys2Log(
					fi.m_nX,
					fi.m_nY,
					&nPosX,
					&nPosY
				);
				// 2004.04.03 Moca EOF�����̍s�ŏI����Ă���ƁAEOF�̈��̍s�Ɉړ����Ă��܂��o�O�C��
				// MoveCursor���␳����̂ł�����x�s��Ȃ��ėǂ��Ȃ���
//				if( nPosY < m_pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() )
				{
					// From Here Mar. 28, 2003 MIK
					//���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤�ɁB
					const CDocLine *pTmpDocLine = m_pcEditWnd->m_cEditDoc.m_cDocLineMgr.GetLineInfo( nPosY );
					if( pTmpDocLine ){
						if( pTmpDocLine->GetLengthWithoutEOL() < fi.m_nX ) nPosX--;
					}
					// To Here Mar. 28, 2003 MIK
					m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].MoveCursor( nPosX, nPosY, TRUE );
					m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev =
						m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX;
				// 2004.04.03 Moca �폜
				// }else{
				// 	int		i;
				// 	i = m_pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() - 1;
				// 	if( i < 0 ){
				// 		i = 0;
				// 	}
				// 	m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].MoveCursor( 0, i, TRUE );
				// 	m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = 0;
				}
			}
			m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].RedrawAll();
		}else{
			// 2004.05.13 Moca �t�@�C�������^�����Ȃ��Ă�ReadOnly�ƃ^�C�v�w���L���ɂ���
			hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray,
										NULL, fi.m_nCharCode, bReadOnly/* �ǂݎ���p�� */,
										fi.m_szDocType[0] == '\0' ? -1 :
										m_cShareData.GetDocumentTypeExt( fi.m_szDocType )
									);
		}
	}
	MY_TRACETIME( cRunningTimer, "EditDoc->Create() End" );

end_of_func:
	m_hWnd = hWnd;
	::ReleaseMutex( hMutex );
	::CloseHandle( hMutex );
	return hWnd ? true : false;
}

/*!
	@brief �G�f�B�^�v���Z�X�̃��b�Z�[�W���[�v
	
	@author aroka
	@date 2002/01/07
*/
bool CNormalProcess::MainLoop()
{
	if( NULL != m_pcEditWnd && NULL != m_hWnd ){
		m_pcEditWnd->MessageLoop();	/* ���b�Z�[�W���[�v */
		return true;
	}
	return false;
}

/*!
	@brief �G�f�B�^�v���Z�X���I������
	
	@author aroka
	@date 2002/01/07
	�����͂Ȃɂ����Ȃ��B��n����dtor�ŁB
*/
void CNormalProcess::Terminate()
{
}



/*!
	�f�X�g���N�^
	
	@date 2002/2/3 aroka �w�b�_����ړ�
*/
CNormalProcess::~CNormalProcess()
{
	if( m_pcEditWnd ){
		delete m_pcEditWnd;
	}
};


/*!
	@brief Mutex(�v���Z�X�������̖ڈ�)���擾����

	���������ɋN������ƃE�B���h�E���\�ɏo�Ă��Ȃ����Ƃ�����B
	
	@date 2002/2/8 aroka Initialize����ړ�
	@retval Mutex �̃n���h����Ԃ�
	@retval ���s�������̓����[�X���Ă��� NULL ��Ԃ�
*/
HANDLE CNormalProcess::GetInitializeMutex() const
{
	MY_RUNNINGTIMER( cRunningTimer, "NormalProcess::GetInitializeMutex" );
	HANDLE hMutex;
	hMutex = ::CreateMutex( NULL, TRUE, GSTR_MUTEX_SAKURA_INIT );
	if( NULL == hMutex ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST,
			GSTR_APPNAME, _T("CreateMutex()���s�B\n�I�����܂��B") );
		return NULL;
	}
	if( ::GetLastError() == ERROR_ALREADY_EXISTS ){
		DWORD dwRet = ::WaitForSingleObject( hMutex, 15000 );	// 2002/2/8 aroka ������������
		if( WAIT_TIMEOUT == dwRet ){// �ʂ̒N�����N����
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
				_T("�G�f�B�^�܂��̓V�X�e�����r�W�[��Ԃł��B\n���΂炭�҂��ĊJ���Ȃ����Ă��������B") );
			::CloseHandle( hMutex );
			return NULL;
		}
	}
	return hMutex;
}

/*[EOF]*/
