#include "StdAfx.h"
#include "CReadManager.h"
#include <io.h>	// access
#include "CEditApp.h"	// CAppExitException
#include "window/CEditWnd.h"
#include "charset/CCodeMediator.h"
#include "io/CFileLoad.h"
#include "util/window.h"

/*!
	�t�@�C����ǂݍ���Ŋi�[����i�����ǂݍ��݃e�X�g�Łj
	@version	2.0
	@note	Windows�p�ɃR�[�f�B���O���Ă���
	@retval	TRUE	����ǂݍ���
	@retval	FALSE	�G���[(�܂��̓��[�U�ɂ��L�����Z��?)
	@date	2002/08/30 Moca ��ReadFile�����ɍ쐬 �t�@�C���A�N�Z�X�Ɋւ��镔����CFileLoad�ōs��
	@date	2003/07/26 ryoji BOM�̏�Ԃ̎擾��ǉ�
*/
EConvertResult CReadManager::ReadFile_To_CDocLineMgr(
	CDocLineMgr*		pcDocLineMgr,	//!< [out]
	const SLoadInfo&	sLoadInfo,		//!< [in]
	SFileInfo*			pFileInfo		//!< [out]
)
{
	LPCTSTR pszPath = sLoadInfo.cFilePath.c_str();

	// �����R�[�h���
	STypeConfig& type = CDocTypeManager().GetTypeSetting( sLoadInfo.nType );
	pFileInfo->eCharCode = sLoadInfo.eCharCode;
	if( CODE_AUTODETECT==pFileInfo->eCharCode) {
		CCodeMediator cmediator( type.m_encoding );
		pFileInfo->eCharCode = cmediator.CheckKanjiCodeOfFile( pszPath );
	}
	if( !IsValidCodeType(pFileInfo->eCharCode) ){
		pFileInfo->eCharCode = type.m_encoding.m_eDefaultCodetype;	// 2011.01.24 ryoji �f�t�H���g�����R�[�h
	}
	if ( pFileInfo->eCharCode == type.m_encoding.m_eDefaultCodetype ){
		pFileInfo->bBomExist = type.m_encoding.m_bDefaultBom;	// 2011.01.24 ryoji �f�t�H���gBOM
	}
	else{
		pFileInfo->bBomExist = CCodeTypeName( pFileInfo->eCharCode ).IsBomDefOn() ;
	}

	/* �����f�[�^�̃N���A */
	pcDocLineMgr->DeleteAllLine();

	/* �������̃��[�U�[������\�ɂ��� */
	if( !::BlockingHook( NULL ) ){
		return RESULT_FAILURE; //######INTERRUPT
	}

	EConvertResult eRet = RESULT_COMPLETE;

	try{
		CFileLoad cfl(type.m_encoding);

		// �t�@�C�����J��
		// �t�@�C�������ɂ�FileClose�����o���̓f�X�g���N�^�̂ǂ��炩�ŏ����ł��܂�
		//	Jul. 28, 2003 ryoji BOM�p�����[�^�ǉ�
		cfl.FileOpen( pszPath, pFileInfo->eCharCode, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode(), &pFileInfo->bBomExist );

		/* �t�@�C�������̎擾 */
		FILETIME	FileTime;
		if( cfl.GetFileTime( NULL, NULL, &FileTime ) ){
			pFileInfo->cFileTime.SetFILETIME(FileTime);
		}

		// ReadLine�̓t�@�C������ �����R�[�h�ϊ����ꂽ1�s��ǂݏo���܂�
		// �G���[����throw CError_FileRead �𓊂��܂�
		int				nLineNum = 0;
		CEol			cEol;
		CNativeW		cUnicodeBuffer;
		EConvertResult	eRead;
		while( RESULT_FAILURE != (eRead = cfl.ReadLine( &cUnicodeBuffer, &cEol )) ){
			if(eRead==RESULT_LOSESOME){
				eRet = RESULT_LOSESOME;
			}
			const wchar_t*	pLine = cUnicodeBuffer.GetStringPtr();
			int		nLineLen = cUnicodeBuffer.GetStringLength();
			++nLineNum;
			CDocEditAgent(pcDocLineMgr).AddLineStrX( pLine, nLineLen );
			//�o�ߒʒm
			if(nLineNum%512==0){
				NotifyProgress(cfl.GetPercent());
				// �������̃��[�U�[������\�ɂ���
				if( !::BlockingHook( NULL ) ){
					throw CAppExitException(); //���f���o
				}
			}
		}

		// �t�@�C�����N���[�Y����
		cfl.FileClose();
	}
	catch(CAppExitException){
		//WM_QUIT����������
		return RESULT_FAILURE;
	}
	catch( CError_FileOpen ){
		eRet = RESULT_FAILURE;
		if( !fexist( pszPath )){
			// �t�@�C�����Ȃ�
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				_T("%ls\n�Ƃ����t�@�C�����J���܂���B\n�t�@�C�������݂��܂���B"),	//Mar. 24, 2001 jepro �኱�C��
				pszPath
			);
		}
		else if( -1 == _taccess( pszPath, 4 )){
			// �ǂݍ��݃A�N�Z�X�����Ȃ�
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				_T("\'%ts\'\n�Ƃ����t�@�C�����J���܂���B\n�ǂݍ��݃A�N�Z�X��������܂���B"),
				pszPath
			 );
		}
		else{
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				_T("\'%ts\'\n�Ƃ����t�@�C�����J���܂���B\n���̃A�v���P�[�V�����Ŏg�p����Ă���\��������܂��B"),
				pszPath
			 );
		}
	}
	catch( CError_FileRead ){
		eRet = RESULT_FAILURE;
		ErrorMessage(
			CEditWnd::getInstance()->GetHwnd(),
			_T("\'%ts\'�Ƃ����t�@�C���̓ǂݍ��ݒ��ɃG���[���������܂����B\n�t�@�C���̓ǂݍ��݂𒆎~���܂��B"),
			pszPath
		 );
		/* �����f�[�^�̃N���A */
		pcDocLineMgr->DeleteAllLine();
	} // ��O�����I���

	NotifyProgress(0);
	/* �������̃��[�U�[������\�ɂ��� */
	if( !::BlockingHook( NULL ) ){
		return RESULT_FAILURE; //####INTERRUPT
	}

	/* �s�ύX��Ԃ����ׂă��Z�b�g */
	CModifyVisitor().ResetAllModifyFlag(pcDocLineMgr);
	return eRet;
}


