#include "StdAfx.h"
#include "CWriteManager.h"
#include <list>
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"
#include "CEditApp.h" // CAppExitException
#include "window/CEditWnd.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"
#include "charset/CUnicode.h"
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

		//�e�s�o��
		int			nLineNumber = 0;
		CDocLine*	pcDocLine = pcDocLineMgr.GetDocLineTop();
		// 1�s��
		{
			++nLineNumber;
			CMemory cmemOutputBuffer;
			{
				CNativeW cstrSrc;
				CMemory cstrBomCheck;
				pcCodeBase->GetBom( &cstrBomCheck );
				if( sSaveInfo.bBomExist && 0 < cstrBomCheck.GetRawLength() ){
					// 1�s�ڂɂ�BOM��t������B�G���R�[�_��bom������ꍇ�̂ݕt������B
					CUnicode().GetBom( cstrSrc._GetMemory() );
				}
				if( pcDocLine ){
					cstrSrc.AppendNativeData( pcDocLine->_GetDocLineDataWithEOL() );
				}
				EConvertResult e = pcCodeBase->UnicodeToCode( cstrSrc, &cmemOutputBuffer );
				if(e==RESULT_LOSESOME){
					nRetVal=RESULT_LOSESOME;
				}
			}
			out.Write(cmemOutputBuffer.GetRawPtr(), cmemOutputBuffer.GetRawLength());
			if( pcDocLine ){
				pcDocLine = pcDocLine->GetNextLine();
			}
		}
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
			{
				// �������ݎ��̃R�[�h�ϊ� cstrSrc -> cmemOutputBuffer
				EConvertResult e = pcCodeBase->UnicodeToCode(
					pcDocLine->_GetDocLineDataWithEOL(),
					&cmemOutputBuffer
				);
				if(e==RESULT_LOSESOME){
					if(nRetVal==RESULT_COMPLETE)nRetVal=RESULT_LOSESOME;
				}
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
