//	$Id$
/*!	@file
	@brief �G�f�B�^�v���Z�X�N���X

	@author aroka
	@date 2002/01/07 Create
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CProcess��蕪��

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CEditApp.h"
#include "CMemory.h"
#include "CShareData.h"
#include "Debug.h"
#include "etc_uty.h"
#include <tchar.h>


/*!
	@brief �G�f�B�^�v���Z�X������������
	
	CEditWnd���쐬����B
	
	@author aroka
	@date 2002/01/07
*/
bool CNormalProcess::Initialize()
{
	HANDLE			hMutex;
	HWND			hWnd;
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;

	/* �v���Z�X�������̖ڈ� */
	hMutex = ::CreateMutex( NULL, TRUE, GSTR_MUTEX_SAKURA_INIT );
	if( NULL == hMutex ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST,
			GSTR_APPNAME, _T("CreateMutex()���s�B\n�I�����܂��B") );
		return false;
	}
	if( ::GetLastError() == ERROR_ALREADY_EXISTS ){
		DWORD dwRet = ::WaitForSingleObject( hMutex, 10000 );
		if( WAIT_TIMEOUT == dwRet ){// �ʂ̒N�����N����
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
				_T("�G�f�B�^�܂��̓V�X�e�����r�W�[��Ԃł��B\n���΂炭�҂��ĊJ���Ȃ����Ă��������B") );
			::CloseHandle( hMutex );
			return false;
		}
	}
	

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	if( !m_cShareData.Init() ){
		//	�K�؂ȃf�[�^�𓾂��Ȃ�����
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONERROR, GSTR_APPNAME,
			_T("�قȂ�o�[�W�����̃G�f�B�^�𓯎��ɋN�����邱�Ƃ͂ł��܂���B") );
		::ReleaseMutex( hMutex );
		::CloseHandle( hMutex );
		return false;
	}
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );

	/* �R�}���h���C���I�v�V���� */
	bool			bReadOnly;
	bool			bDebugMode;
	bool			bGrepMode;
	GrepInfo		gi;
	FileInfo		fi;
	
	/* �R�}���h���C���̉�� */
	bReadOnly = CCommandLine::Instance()->IsReadOnly();
	bDebugMode = CCommandLine::Instance()->IsDebugMode();
	bGrepMode = CCommandLine::Instance()->IsGrepMode();
	CCommandLine::Instance()->GetFileInfo(fi);
	CCommandLine::Instance()->GetGrepInfo(gi);
	
	/* �R�}���h���C���Ŏ󂯎�����t�@�C�����J����Ă���ꍇ�� */
	/* ���̕ҏW�E�B���h�E���A�N�e�B�u�ɂ��� */
	if( 0 < strlen( fi.m_szPath ) ){
		//	Oct. 27, 2000 genta
		//	MRU����J�[�\���ʒu�𕜌����鑀���CEditDoc::FileRead��
		//	�s����̂ł����ł͕K�v�Ȃ��D

		HWND hwndOwner;
		/* �w��t�@�C�����J����Ă��邩���ׂ� */
		if( FALSE != m_cShareData.IsPathOpened( fi.m_szPath, &hwndOwner ) ){
			//	From Here Oct. 19, 2001 genta
			//	�J�[�\���ʒu�������Ɏw�肳��Ă�����w��ʒu�ɃW�����v
			if( fi.m_nY >= 0 ){	//	�s�̎w�肪���邩
				POINT& pt = *(POINT*)m_pShareData->m_szWork;
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

//�����v���Z�X��
	/* �G�f�B�^�E�B���h�E�I�u�W�F�N�g���쐬 */
	m_pcEditWnd = new CEditWnd;
	if( bDebugMode ){
		hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );

//	#ifdef _DEBUG/////////////////////////////////////////////
		/* �f�o�b�O���j�^���[�h�ɐݒ� */
		m_pcEditWnd->SetDebugModeON();
//	#endif////////////////////////////////////////////////////
	}else
	if( bGrepMode ){
		hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );
		/* GREP */
		/*nHitCount = */

		TCHAR szWork[MAX_PATH];
		/* �����O�t�@�C�������擾���� */
		if( FALSE != ::GetLongFileName( gi.cmGrepFolder.GetPtr( NULL ), szWork ) ){
			gi.cmGrepFolder.SetData( szWork, strlen( szWork ) );
		}
		m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].DoGrep(
			&gi.cmGrepKey,
			&gi.cmGrepFile,
			&gi.cmGrepFolder,
			gi.bGrepSubFolder,
			gi.bGrepNoIgnoreCase,
			gi.bGrepRegularExp,
			gi.bGrepKanjiCode_AutoDetect,
			gi.bGrepOutputLine,
			gi.bGrepWordOnly,	//	Jun. 26, 2001 genta
			gi.nGrepOutputStyle
		);
	}else{
		if( 0 < (int)strlen( fi.m_szPath ) ){
			hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray, 
										fi.m_szPath, fi.m_nCharCode, bReadOnly/* �ǂݎ���p�� */ );
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

			//	Nov. 6, 2000 genta
			//	�L�����b�g�ʒu�̕����̂���
			//	m_nCaretPosX_Prev�̗p�r�͕s�������C�����ݒ肵�Ȃ��̂͂܂����̂łƂ肠���� 0
			m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = 0;
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
				if( nPosY < m_pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
					m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].MoveCursor( nPosX, nPosY, TRUE );
					m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = nPosX;
				}else{
					int		i;
					i = m_pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() - 1;
					if( i < 0 ){
						i = 0;
					}
					m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].MoveCursor( 0, i, TRUE );
					m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = 0;
				}
			}
			m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].RedrawAll();
		}else{
			hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );
		}
	}
	m_hWnd = hWnd;
	::ReleaseMutex( hMutex );
	::CloseHandle( hMutex );
	return true;
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


/*[EOF]*/
