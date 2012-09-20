#include "StdAfx.h"
#include "CWriteManager.h"
#include <list>
#include "doc/CDocLineMgr.h"
#include "doc/CDocLine.h"
#include "CEditApp.h" // CAppExitException
#include "window/CEditWnd.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"
#include "io/CIoBridge.h"
#include "io/CBinaryStream.h"
#include "util/window.h"


/*! �o�b�t�@���e���t�@�C���ɏ����o�� (�e�X�g�p)

	@note Windows�p�ɃR�[�f�B���O���Ă���
	@date 2003.07.26 ryoji BOM�����ǉ�
*/
EConvertResult CWriteManager::WriteFile_From_CDocLineMgr(
	const CDocLineMgr&	pcDocLineMgr,	//!< [in]
	const SSaveInfo&	sSaveInfo		//!< [in]
)
{
	EConvertResult		nRetVal = RESULT_COMPLETE;
	std::auto_ptr<CCodeBase> pcCodeBase( CCodeFactory::CreateCodeBase(sSaveInfo.eCharCode,0) );

	try
	{
		//�t�@�C���I�[�v��
		CBinaryOutputStream out(sSaveInfo.cFilePath,true);

		//BOM�o��
		if(sSaveInfo.bBomExist){
			CMemory cBom;
			pcCodeBase->GetBom(&cBom);
			out.Write(cBom.GetRawPtr(),cBom.GetRawLength());
		}

		//�e�s�o��
		int			nLineNumber = 0;
		CDocLine*	pcDocLine = pcDocLineMgr.GetDocLineTop();
		while( pcDocLine ){
			++nLineNumber;

			//�o�ߒʒm
			if(pcDocLineMgr.GetLineCount()>0 && nLineNumber%1024==0){
				NotifyProgress(nLineNumber * 100 / pcDocLineMgr.GetLineCount());
				// �������̃��[�U�[������\�ɂ���
				if( !::BlockingHook( NULL ) ){
					throw CAppExitException(); //���f���o
				}
			}

			//1�s�o�� -> cmemOutputBuffer
			CMemory cmemOutputBuffer;
			if( 0 < pcDocLine->GetLengthWithoutEOL() ){
				CNativeW cstrSrc( pcDocLine->GetPtr(), pcDocLine->GetLengthWithoutEOL() );

				// �������ݎ��̃R�[�h�ϊ� cstrSrc -> cmemOutputBuffer
				EConvertResult e = CIoBridge::ImplToFile(
					cstrSrc,
					&cmemOutputBuffer,
					sSaveInfo.eCharCode
				);
				if(e==RESULT_LOSESOME){
					if(nRetVal==RESULT_COMPLETE)nRetVal=RESULT_LOSESOME;
				}
			}

			//���s�o�� -> cmemOutputBuffer
			if( pcDocLine->GetEol() != EOL_NONE ){
				CMemory cEolMem;
				pcCodeBase->GetEol(&cEolMem,sSaveInfo.cEol!=EOL_NONE?sSaveInfo.cEol:pcDocLine->GetEol());
				cmemOutputBuffer.AppendRawData(cEolMem.GetRawPtr(),cEolMem.GetRawLength());
			}

			//�t�@�C���ɏo�� cmemOutputBuffer -> fp
			out.Write(cmemOutputBuffer.GetRawPtr(), cmemOutputBuffer.GetRawLength());

			//���̍s��
			pcDocLine = pcDocLine->GetNextLine();
		}

		//�t�@�C���N���[�Y
		out.Close();
	}
	catch(CError_FileOpen){ //########### �����_�ł́A���̗�O�����������ꍇ�͐���ɓ���ł��Ȃ�
		ErrorMessage(
			CEditWnd::getInstance()->GetHwnd(),
			_T("\'%ts\'\n")
			_T("�t�@�C����ۑ��ł��܂���B\n")
			_T("�p�X�����݂��Ȃ����A���̃A�v���P�[�V�����Ŏg�p����Ă���\��������܂��B"),
			sSaveInfo.cFilePath.c_str()
		);
		nRetVal = RESULT_FAILURE;
	}
	catch(CError_FileWrite){
		nRetVal = RESULT_FAILURE;
	}
	catch(CAppExitException){
		//���f���o
		return RESULT_FAILURE;
	}

	return nRetVal;
}
