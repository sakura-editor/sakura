#include "stdafx.h"
#include "CDocReader.h"

/* �S�s�f�[�^��Ԃ�
	���s�R�[�h�́ACFLF���ꂳ���B
	@retval �S�s�f�[�^�Bfree�ŊJ�����Ȃ���΂Ȃ�Ȃ��B
	@note   Debug�ł̃e�X�g�ɂ̂ݎg�p���Ă���B
*/
wchar_t* CDocReader::GetAllData(int* pnDataLen)
{
	int			nDataLen;
	int			nLineLen;
	CDocLine* 	pDocLine;

	pDocLine = m_pcDocLineMgr->GetDocLineTop();
	nDataLen = 0;
	while( NULL != pDocLine ){
		//	Oct. 7, 2002 YAZAKI
		nDataLen += pDocLine->GetLengthWithoutEOL() + 2;	//	\r\n��ǉ����ĕԂ�����+2����B
		pDocLine = pDocLine->GetNextLine();
	}

	wchar_t* pData;
	pData = (wchar_t*)malloc( (nDataLen + 1) * sizeof(wchar_t) );
	if( NULL == pData ){
		TopErrorMessage( NULL, _T("CDocLineMgr::GetAllData()\n�������m�ۂɎ��s���܂����B\n%d�o�C�g"), nDataLen + 1 );
		return NULL;
	}
	pDocLine = m_pcDocLineMgr->GetDocLineTop();

	nDataLen = 0;
	while( NULL != pDocLine ){
		//	Oct. 7, 2002 YAZAKI
		nLineLen = pDocLine->GetLengthWithoutEOL();
		if( 0 < nLineLen ){
			wmemcpy( &pData[nDataLen], pDocLine->GetPtr(), nLineLen );
			nDataLen += nLineLen;
		}
		pData[nDataLen++] = L'\r';
		pData[nDataLen++] = L'\n';
		pDocLine = pDocLine->GetNextLine();
	}
	pData[nDataLen] = L'\0';
	*pnDataLen = nDataLen;
	return pData;
}


const wchar_t* CDocReader::GetLineStr( CLogicInt nLine, CLogicInt* pnLineLen )
{
	CDocLine* pDocLine;
	pDocLine = m_pcDocLineMgr->GetLine( nLine );
	if( NULL == pDocLine ){
		*pnLineLen = CLogicInt(0);
		return NULL;
	}
	// 2002/2/10 aroka CMemory �̃����o�ϐ��ɒ��ڃA�N�Z�X���Ȃ�(inline������Ă���̂ő��x�I�Ȗ��͂Ȃ�)
	return pDocLine->GetDocLineStrWithEOL( pnLineLen );
}


/*!
	�w�肳�ꂽ�s�ԍ��̕�����Ɖ��s�R�[�h�������������擾
	
	@author Moca
	@date 2003.06.22
*/
const wchar_t* CDocReader::GetLineStrWithoutEOL( CLogicInt nLine, int* pnLineLen )
{
	const CDocLine* pDocLine = m_pcDocLineMgr->GetLine( nLine );
	if( NULL == pDocLine ){
		*pnLineLen = 0;
		return NULL;
	}
	*pnLineLen = pDocLine->GetLengthWithoutEOL();
	return pDocLine->GetPtr();
}




/*! ���A�N�Z�X���[�h�F�擪�s�𓾂�

	@param pnLineLen [out] �s�̒������Ԃ�B
	@return 1�s�ڂ̐擪�ւ̃|�C���^�B
	�f�[�^��1�s���Ȃ��Ƃ��́A����0�A�|�C���^NULL���Ԃ�B

*/
const wchar_t* CDocReader::GetFirstLinrStr( int* pnLineLen )
{
	const wchar_t* pszLine;
	if( CLogicInt(0) == m_pcDocLineMgr->GetLineCount() ){
		pszLine = NULL;
		*pnLineLen = 0;
	}else{
		pszLine = m_pcDocLineMgr->GetDocLineTop()->GetDocLineStrWithEOL( pnLineLen );

		m_pcDocLineMgr->m_pDocLineCurrent = m_pcDocLineMgr->GetDocLineTop()->GetNextLine();
	}
	return pszLine;
}





/*!
	���A�N�Z�X���[�h�F���̍s�𓾂�

	@param pnLineLen [out] �s�̒������Ԃ�B
	@return ���s�̐擪�ւ̃|�C���^�B
	GetFirstLinrStr()���Ăяo����Ă��Ȃ���NULL���Ԃ�

*/
const wchar_t* CDocReader::GetNextLinrStr( int* pnLineLen )
{
	const wchar_t* pszLine;
	if( NULL == m_pcDocLineMgr->m_pDocLineCurrent ){
		pszLine = NULL;
		*pnLineLen = 0;
	}
	else{
		pszLine = m_pcDocLineMgr->m_pDocLineCurrent->GetDocLineStrWithEOL( pnLineLen );

		m_pcDocLineMgr->m_pDocLineCurrent = m_pcDocLineMgr->m_pDocLineCurrent->GetNextLine();
	}
	return pszLine;
}

