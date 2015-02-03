/*!	@file
	@brief �t�@�C���ǂݍ��݃N���X

	@author Moca
	@date 2002/08/30 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca, genta
	Copyright (C) 2003, Moca, ryoji
	Copyright (C) 2006, rastiv

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include <stdlib.h>
#include <string.h>
#include "_main/global.h"
#include "mem/CMemory.h"
#include "CEol.h"
#include "io/CFileLoad.h"
#include "charset/charcode.h"
#include "io/CIoBridge.h"
#include "charset/CCodeFactory.h" ////
#include "charset/CCodePage.h"
#include "charset/CCodeMediator.h"
#include "util/string_ex2.h"
#include "charset/CESI.h"
#include "window/CEditWnd.h"

/*
	@note Win32API�Ŏ���
		2GB�ȏ�̃t�@�C���͊J���Ȃ�
*/

/*! ���[�h�p�o�b�t�@�T�C�Y�̏����l */
const int CFileLoad::gm_nBufSizeDef = 32768;
//(�œK�l���}�V���ɂ���ĈႤ�̂łƂ肠����32KB�m�ۂ���)

// /*! ���[�h�p�o�b�t�@�T�C�Y�̐ݒ�\�ȍŒ�l */
// const int gm_nBufSizeMin = 1024;

/*! �R���X�g���N�^ */
CFileLoad::CFileLoad( const SEncodingConfig& encode )
{
	m_pEencoding = &encode;

	m_hFile			= NULL;
	m_nFileSize		= 0;
	m_nFileDataLen	= 0;
	m_CharCode		= CODE_DEFAULT;
	m_pCodeBase		= NULL;////
	m_encodingTrait = ENCODING_TRAIT_ASCII;
	m_bBomExist		= false;	// Jun. 08, 2003 Moca
	m_nFlag 		= 0;
	m_nReadLength	= 0;
	m_eMode			= FLMODE_CLOSE;	// Jun. 08, 2003 Moca

	m_nLineIndex	= -1;

	m_pReadBuf = NULL;
	m_nReadDataLen    = 0;
	m_nReadBufSize    = 0;
	m_nReadBufOffSet  = 0;
}

/*! �f�X�g���N�^ */
CFileLoad::~CFileLoad( void )
{
	if( NULL != m_hFile ){
		FileClose();
	}
	if( NULL != m_pReadBuf ){
		free( m_pReadBuf );
	}
	if( NULL != m_pCodeBase ){
		delete m_pCodeBase;
	}
}

/*!
	�t�@�C�����J��
	@param pFileName [in] �t�@�C����
	@param bBigFile  [in] 2GB�ȏ�̃t�@�C�����J�����BGrep=true, 32bit�ł͂��̑�=false�ŉ^�p
	@param CharCode  [in] �t�@�C���̕����R�[�h�D
	@param nFlag [in] �����R�[�h�̃I�v�V����
	@param pbBomExist [out] BOM�̗L��
	@date 2003.06.08 Moca CODE_AUTODETECT���w��ł���悤�ɕύX
	@date 2003.07.26 ryoji BOM�����ǉ�
*/
ECodeType CFileLoad::FileOpen( LPCTSTR pFileName, bool bBigFile, ECodeType CharCode, int nFlag, bool* pbBomExist )
{
	HANDLE	hFile;
	ULARGE_INTEGER	fileSize;
	ECodeType	nBomCode;

	// FileClose���Ă�ł���ɂ��Ă�������
	if( NULL != m_hFile ){
#ifdef _DEBUG
		::MessageBox( NULL, _T("CFileLoad::FileOpen\nFileClose���Ă�ł���ɂ��Ă�������") , NULL, MB_OK );
#endif
		throw CError_FileOpen();
	}
	hFile = ::CreateFile(
		pFileName,
		GENERIC_READ,
		//	Oct. 18, 2002 genta FILE_SHARE_WRITE �ǉ�
		//	���v���Z�X���������ݒ��̃t�@�C�����J����悤��
		FILE_SHARE_READ | FILE_SHARE_WRITE,	// ���L
		NULL,						// �Z�L�����e�B�L�q�q
		OPEN_EXISTING,				// �쐬���@
		FILE_FLAG_SEQUENTIAL_SCAN,	// �t�@�C������
		NULL						// �e���v���[�g�t�@�C���̃n���h��
	);
	if( hFile == INVALID_HANDLE_VALUE ){
		throw CError_FileOpen();
	}
	m_hFile = hFile;

	// GetFileSizeEx �� Win2K�ȏ�
	fileSize.LowPart = ::GetFileSize( hFile, &fileSize.HighPart );
	if( 0xFFFFFFFF == fileSize.LowPart ){
		DWORD lastError = ::GetLastError();
		if( NO_ERROR != lastError ){
			FileClose();
			throw CError_FileOpen();
		}
	}
	if( !bBigFile && 0x80000000 <= fileSize.QuadPart ){
		// �t�@�C�����傫������(2GB��)
		FileClose();
		throw CError_FileOpen();
	}
	m_nFileSize = fileSize.QuadPart;
//	m_eMode = FLMODE_OPEN;

	// From Here Jun. 08, 2003 Moca �����R�[�h����
	// �f�[�^�ǂݍ���
	Buffering();

	nBomCode = CCodeMediator::DetectUnicodeBom( m_pReadBuf, m_nReadDataLen );
	if( CharCode == CODE_AUTODETECT ){
		if( nBomCode != CODE_NONE ){
			CharCode = nBomCode;
		}else{
			CCodeMediator mediator(*m_pEencoding);
			CharCode = mediator.CheckKanjiCode( m_pReadBuf, m_nReadDataLen );
		}
	}
	// To Here Jun. 08, 2003
	// �s���ȕ����R�[�h�̂Ƃ��̓f�t�H���g(SJIS:���ϊ�)��ݒ�
	if( !IsValidCodeOrCPType(CharCode) ){
		CharCode = CODE_DEFAULT;
	}
	m_CharCode = CharCode;
	m_pCodeBase=CCodeFactory::CreateCodeBase(m_CharCode, m_nFlag);
	m_encodingTrait = CCodePage::GetEncodingTrait(m_CharCode);
	m_nFlag = nFlag;

	m_nFileDataLen = m_nFileSize;
	bool bBom = false;
	if( 0 < m_nReadDataLen ){
		CMemory headData(m_pReadBuf, t_min(m_nReadDataLen, 10));
		CNativeW headUni;
		CIoBridge::FileToImpl(headData, &headUni, m_pCodeBase, m_nFlag);
		if( 1 <= headUni.GetStringLength() && headUni.GetStringPtr()[0] == 0xfeff ){
			bBom = true;
		}
	}
	if( bBom ){
		//	Jul. 26, 2003 ryoji BOM�̗L�����p�����[�^�ŕԂ�
		m_bBomExist = true;
		if( pbBomExist != NULL ){
			*pbBomExist = true;
		}
	}else{
		//	Jul. 26, 2003 ryoji BOM�̗L�����p�����[�^�ŕԂ�
		if( pbBomExist != NULL ){
			*pbBomExist = false;
		}
	}
	
	// To Here Jun. 13, 2003 Moca BOM�̏���
	m_eMode = FLMODE_READY;
//	m_cmemLine.AllocBuffer( 256 );
	m_pCodeBase->GetEol( &m_memEols[0], EOL_NEL );
	m_pCodeBase->GetEol( &m_memEols[1], EOL_LS );
	m_pCodeBase->GetEol( &m_memEols[2], EOL_PS );
	bool bEolEx = false;
	int  nMaxEolLen = 0;
	for( int k = 0; k < (int)_countof(m_memEols); k++ ){
		if( 0 != m_memEols[k].GetRawLength() ){
			bEolEx = true;
			nMaxEolLen = t_max(nMaxEolLen, m_memEols[k].GetRawLength());
		}
	}
	m_bEolEx = bEolEx;
	m_nMaxEolLen = nMaxEolLen;
	if(	false == GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol ){
		m_bEolEx = false;
	}

	m_nReadOffset2 = 0;
	m_nTempResult = RESULT_FAILURE;
	m_cLineTemp.SetString(L"");
	return m_CharCode;
}

/*!
	�t�@�C�������
	�ǂݍ��ݗp�o�b�t�@��m_memLine���N���A�����
*/
void CFileLoad::FileClose( void )
{
	ReadBufEmpty();
	if( NULL != m_hFile ){
		::CloseHandle( m_hFile );
		m_hFile = NULL;
	}
	if( NULL != m_pCodeBase ){
		delete m_pCodeBase;
		m_pCodeBase = NULL;
	}
	m_nFileSize		=  0;
	m_nFileDataLen	=  0;
	m_CharCode		= CODE_DEFAULT;
	m_bBomExist		= false; // From Here Jun. 08, 2003
	m_nFlag 		=  0;
	m_nReadLength	=  0;
	m_eMode			= FLMODE_CLOSE;
	m_nLineIndex	= -1;
}

/*! 1�s�ǂݍ���
	UTF-7�ꍇ�A�f�[�^����NEL,PS,LS���̉��s�܂ł�1�s�Ƃ��Ď��o��
*/
EConvertResult CFileLoad::ReadLine( CNativeW* pUnicodeBuffer, CEol* pcEol )
{
	if( m_CharCode != CODE_UTF7 && m_CharCode != CP_UTF7 ){
		return ReadLine_core( pUnicodeBuffer, pcEol );
	}
	if( m_nReadOffset2 == m_cLineTemp.GetStringLength() ){
		CEol cEol;
		EConvertResult e = ReadLine_core( &m_cLineTemp, &cEol );
		if( e == RESULT_FAILURE ){
			pUnicodeBuffer->_GetMemory()->SetRawDataHoldBuffer( L"", 0 );
			*pcEol = cEol;
			return RESULT_FAILURE;
		}
		m_nReadOffset2 = 0;
		m_nTempResult = e;
	}
	int  nOffsetTemp = m_nReadOffset2;
	int  nRetLineLen;
	CEol cEolTemp;
	const wchar_t* pRet = GetNextLineW( m_cLineTemp.GetStringPtr(), m_cLineTemp.GetStringLength(),
				&nRetLineLen, &m_nReadOffset2, &cEolTemp, GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol );
	if( m_cLineTemp.GetStringLength() == m_nReadOffset2 && nOffsetTemp == 0 ){
		// �r���ɉ��s���Ȃ�����́Aswap���g���Ē��g�̃R�s�[���ȗ�����
		pUnicodeBuffer->swap(m_cLineTemp);
		if( 0 < m_cLineTemp.GetStringLength() ){
			m_cLineTemp._GetMemory()->SetRawDataHoldBuffer( L"", 0 );
		}
		m_nReadOffset2 = 0;
	}else{
		// ���s���r���ɂ������B�K�v�����R�s�[
		pUnicodeBuffer->_GetMemory()->SetRawDataHoldBuffer( L"", 0 );
		pUnicodeBuffer->AppendString( pRet, nRetLineLen + cEolTemp.GetLen() );
	}
	*pcEol = cEolTemp;
	return m_nTempResult;
}


/*!
	���̘_���s�𕶎��R�[�h�ϊ����ă��[�h����
	�����A�N�Z�X��p
	GetNextLine�̂悤�ȓ��������
	@return	NULL�ȊO	1�s��ێ����Ă���f�[�^�̐擪�A�h���X��Ԃ��B�i���I�ł͂Ȃ��ꎞ�I�ȗ̈�B
			NULL		�f�[�^���Ȃ�����
*/
EConvertResult CFileLoad::ReadLine_core(
	CNativeW*	pUnicodeBuffer,	//!< [out] UNICODE�f�[�^�󂯎��o�b�t�@�B���s���܂߂ēǂݎ��B
	CEol*		pcEol			//!< [i/o]
)
{
	EConvertResult eRet = RESULT_COMPLETE;

#ifdef _DEBUG
	if( m_eMode < FLMODE_READY ){
		MYTRACE( _T("CFileLoad::ReadLine(): m_eMode = %d\n"), m_eMode );
		return RESULT_FAILURE;
	}
#endif
	//�s�f�[�^�o�b�t�@ (�����R�[�h�ϊ������̐��̃f�[�^)
	m_cLineBuffer.SetRawDataHoldBuffer("",0);

	// 1�s���o�� ReadBuf -> m_memLine
	//	Oct. 19, 2002 genta while�����𐮗�
	int			nBufLineLen;
	int			nEolLen;
	int			nBufferNext;
	for (;;) {
		const char* pLine = GetNextLineCharCode(
			m_pReadBuf,
			m_nReadDataLen,    //[in] �o�b�t�@�̗L���f�[�^�T�C�Y
			&nBufLineLen,      //[out]���s���܂܂Ȃ�����
			&m_nReadBufOffSet, //[i/o]�I�t�Z�b�g
			pcEol,
			&nEolLen,
			&nBufferNext
		);
		if(pLine==NULL)break;

		// ReadBuf����1�s���擾����Ƃ��A���s�R�[�h��������\�������邽��
		if( m_nReadDataLen <= m_nReadBufOffSet && FLMODE_READY == m_eMode ){// From Here Jun. 13, 2003 Moca
			int n = 128;
			int nMinAllocSize = m_cLineBuffer.GetRawLength() + nEolLen - nBufferNext + 100;
			while( n < nMinAllocSize ){
				n *= 2;
			}
			m_cLineBuffer.AllocBuffer( n );
			m_cLineBuffer.AppendRawData( pLine, nBufLineLen + nEolLen - nBufferNext );
			m_nReadBufOffSet -= nBufferNext;
			// �o�b�t�@���[�h   File -> ReadBuf
			Buffering();
			if( 0 == nBufferNext && 0 < nEolLen ){
				// �҂�����s�o��
				break;
			}
		}else{
			m_cLineBuffer.AppendRawData( pLine, nBufLineLen + nEolLen );
			break;
		}
	}
	m_nReadLength += m_cLineBuffer.GetRawLength();

	// �����R�[�h�ϊ� cLineBuffer -> pUnicodeBuffer
	EConvertResult eConvertResult = CIoBridge::FileToImpl(m_cLineBuffer,pUnicodeBuffer,m_pCodeBase,m_nFlag);
	if(eConvertResult==RESULT_LOSESOME){
		eRet = RESULT_LOSESOME;
	}

	m_nLineIndex++;

	// 2012.10.21 Moca BOM�̏���(UTF-7�Ή�)
	if( m_nLineIndex == 0 ){
		if( m_bBomExist && 1 <= pUnicodeBuffer->GetStringLength() ){
			if( pUnicodeBuffer->GetStringPtr()[0] == 0xfeff ){
				CNativeW tmp(pUnicodeBuffer->GetStringPtr() + 1, pUnicodeBuffer->GetStringLength() - 1);
				*pUnicodeBuffer = tmp;
			}
		}
	}
	if( 0 == pUnicodeBuffer->GetStringLength() ){
		eRet = RESULT_FAILURE;
	}

	return eRet;
}



/*!
	�o�b�t�@�Ƀf�[�^��ǂݍ���
	@note �G���[���� throw ����
*/
void CFileLoad::Buffering( void )
{
	DWORD	ReadSize;

	// �������[�m��
	if( NULL == m_pReadBuf ){
		int nBufSize;
		nBufSize = ( m_nFileSize < gm_nBufSizeDef )?( static_cast<int>(m_nFileSize) ):( gm_nBufSizeDef );
		//	Borland C++�ł�0�o�C�g��malloc���l�����s�ƌ��Ȃ�����
		//	�Œ�1�o�C�g�͎擾���邱�Ƃ�0�o�C�g�̃t�@�C�����J����悤�ɂ���
		if( 0 >= nBufSize ){
			nBufSize = 1; // Jun. 08, 2003  BCC��malloc(0)��NULL��Ԃ��d�l�ɑΏ�
		}

		m_pReadBuf = (char *)malloc( nBufSize );
		if( NULL == m_pReadBuf ){
			throw CError_FileRead(); // �������[�m�ۂɎ��s
		}
		m_nReadDataLen = 0;
		m_nReadBufSize = nBufSize;
		m_nReadBufOffSet = 0;
	}
	// ReadBuf���Ƀf�[�^���c���Ă���
	else if( m_nReadBufOffSet < m_nReadDataLen ){
		m_nReadDataLen -= m_nReadBufOffSet;
		memmove( m_pReadBuf, &m_pReadBuf[m_nReadBufOffSet], m_nReadDataLen );
		m_nReadBufOffSet = 0;
	}
	else{
		m_nReadBufOffSet = 0;
		m_nReadDataLen = 0;
	}
	// �t�@�C���̓ǂݍ���
	ReadSize = Read( &m_pReadBuf[m_nReadDataLen], m_nReadBufSize - m_nReadDataLen );
	if( 0 == ReadSize ){
		m_eMode = FLMODE_READBUFEND;	// �t�@�C���Ȃǂ̏I���ɒB�����炵��
	}
	m_nReadDataLen += ReadSize;
}

/*!
	�o�b�t�@�N���A
*/
void CFileLoad::ReadBufEmpty( void )
{
	if ( NULL != m_pReadBuf ){
		free( m_pReadBuf );
		m_pReadBuf = NULL;
	}
	m_nReadDataLen    = 0;
	m_nReadBufSize    = 0;
	m_nReadBufOffSet  = 0;
}


/*!
	 ���݂̐i�s�����擾����
	 @return 0% - 100%  �኱�덷���o��
*/
int CFileLoad::GetPercent( void ){
	int nRet;
	if( 0 == m_nFileDataLen || m_nReadLength > m_nFileDataLen ){
		nRet = 100;
	}else{
		nRet = static_cast<int>(m_nReadLength * 100 / m_nFileDataLen);
	}
	return nRet;
}

/*!
	GetNextLine�̔ėp�����R�[�h��
*/
const char* CFileLoad::GetNextLineCharCode(
	const char*	pData,		//!< [in]	����������
	int			nDataLen,	//!< [in]	����������̃o�C�g��
	int*		pnLineLen,	//!< [out]	1�s�̃o�C�g����Ԃ�������EOL�͊܂܂Ȃ�
	int*		pnBgn,		//!< [i/o]	����������̃o�C�g�P�ʂ̃I�t�Z�b�g�ʒu
	CEol*		pcEol,		//!< [i/o]	EOL
	int*		pnEolLen,	//!< [out]	EOL�̃o�C�g�� (Unicode�ō���Ȃ��悤��)
	int*		pnBufferNext	//!< [out]	���񎝉z���o�b�t�@��(EOL�̒f��)
){
	int nbgn = *pnBgn;
	int i;

	pcEol->SetType( EOL_NONE );
	*pnBufferNext = 0;

	if( nDataLen <= nbgn ){
		*pnLineLen = 0;
		*pnEolLen = 0;
		return NULL;
	}
	const unsigned char* pUData = (const unsigned char*)pData; // signed���ƕ����g����NEL�����������Ȃ�̂�
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
	int nLen = nDataLen;
	int neollen = 0;
	switch( m_encodingTrait ){
	case ENCODING_TRAIT_ERROR://
	case ENCODING_TRAIT_ASCII:
		{
			static const EEolType eEolEx[] = {
				EOL_NEL,
				EOL_LS,
				EOL_PS,
			};
			nLen = nDataLen;
			for( i = nbgn; i < nDataLen; ++i ){
				if( pData[i] == '\r' || pData[i] == '\n' ){
					pcEol->SetTypeByStringForFile( &pData[i], nDataLen - i );
					neollen = pcEol->GetLen();
					break;
				}
				if( m_bEolEx ){
					int k;
					for( k = 0; k < (int)_countof(eEolEx); k++ ){
						if( 0 != m_memEols[k].GetRawLength() && i + m_memEols[k].GetRawLength() - 1 < nDataLen
								&& 0 == memcmp( m_memEols[k].GetRawPtr(), pData + i, m_memEols[k].GetRawLength()) ){
							pcEol->SetType(eEolEx[k]);
							neollen = m_memEols[k].GetRawLength();
							break;
						}
					}
					if( k != (int)_countof(eEolEx) ){
						break;
					}
				}
			}
			// UTF-8��NEL,PS,LS�f�Ђ̌��o
			if( i == nDataLen && m_bEolEx ){
				for( i = t_max(0, nDataLen - m_nMaxEolLen - 1); i < nDataLen; i++ ){
					int k;
					bool bSet = false;
					for( k = 0; k < (int)_countof(eEolEx); k++ ){
						int nCompLen = t_min(nDataLen - i, m_memEols[k].GetRawLength());
						if( 0 != nCompLen && 0 == memcmp(m_memEols[k].GetRawPtr(), pData + i, nCompLen) ){
							*pnBufferNext = t_max(*pnBufferNext, nCompLen);
							bSet = true;
						}
					}
					if( bSet ){
						break;
					}
				}
				i = nDataLen;
			}
		}
		break;
	case ENCODING_TRAIT_UTF16LE:
		nLen = nDataLen - 1;
		for( i = nbgn; i < nLen; i += 2 ){
			wchar_t c = static_cast<wchar_t>((pUData[i + 1] << 8) | pUData[i]);
			if( WCODE::IsLineDelimiter(c, bExtEol) ){
				pcEol->SetTypeByStringForFile_uni( &pData[i], nDataLen - i );
				neollen = (Int)pcEol->GetLen() * sizeof(wchar_t);
				break;
			}
		}
		break;
	case ENCODING_TRAIT_UTF16BE:
		nLen = nDataLen - 1;
		for( i = nbgn; i < nLen; i += 2 ){
			wchar_t c = static_cast<wchar_t>((pUData[i] << 8) | pUData[i + 1]);
			if( WCODE::IsLineDelimiter(c, bExtEol) ){
				pcEol->SetTypeByStringForFile_unibe( &pData[i], nDataLen - i );
				neollen = (Int)pcEol->GetLen() * sizeof(wchar_t);
				break;
			}
		}
		break;
	case ENCODING_TRAIT_UTF32LE:
		nLen = nDataLen - 3;
		for( i = nbgn; i < nLen; i += 4 ){
			wchar_t c = static_cast<wchar_t>((pUData[i+1] << 8) | pUData[i]);
			if( pUData[i+3] == 0x00 && pUData[i+2] == 0x00 && WCODE::IsLineDelimiter(c, bExtEol) ){
				wchar_t c2;
				int eolTempLen;
				if( i + 4 < nLen && pUData[i+7] == 0x00 && pUData[i+6] == 0x00 ){
					c2 = static_cast<wchar_t>((pUData[i+5] << 8) | pUData[i+4]);
					eolTempLen = 2 * sizeof(wchar_t);
				}else{
					c2 = 0x0000;
					eolTempLen = 1 * sizeof(wchar_t);
				}
				wchar_t pDataTmp[2] = {c, c2};
				pcEol->SetTypeByStringForFile_uni( reinterpret_cast<char *>(pDataTmp), eolTempLen );
				neollen = (Int)pcEol->GetLen() * 4;
				break;
			}
		}
		break;
	case ENCODING_TRAIT_UTF32BE:
		nLen = nDataLen - 3;
		for( i = nbgn; i < nLen; i += 4 ){
			wchar_t c = static_cast<wchar_t>((pUData[i+2] << 8) | pUData[i+3]);
			if( pUData[i] == 0x00 && pUData[i+1] == 0x00 && WCODE::IsLineDelimiter(c, bExtEol) ){
				wchar_t c2;
				int eolTempLen;
				if( i + 4 < nLen && pUData[i+4] == 0x00 && pUData[i+5] == 0x00 ){
					c2 = static_cast<wchar_t>((pUData[i+6] << 8) | pUData[i+7]);
					eolTempLen = 2 * sizeof(wchar_t);
				}else{
					c2 = 0x0000;
					eolTempLen = 1 * sizeof(wchar_t);
				}
				wchar_t pDataTmp[2] = {c, c2};
				pcEol->SetTypeByStringForFile_uni( reinterpret_cast<char *>(pDataTmp), eolTempLen );
				neollen = (Int)pcEol->GetLen() * 4;
				break;
			}
		}
		break;
	case ENCODING_TRAIT_EBCDEC_CRLF:
	case ENCODING_TRAIT_EBCDEC:
		// EOL�R�[�h�ϊ����ݒ�
		for( i = nbgn; i < nDataLen; ++i ){
			if( m_encodingTrait == ENCODING_TRAIT_EBCDEC && bExtEol ){
				if( pData[i] == '\x15' ){
					pcEol->SetType(EOL_NEL);
					neollen = 1;
					break;
				}
			}
			if( pData[i] == '\x0d' || pData[i] == '\x25' ){
				char szEof[3] = {
					(pData[i]  == '\x25' ? '\x0a' : '\x0d'),
					(pData[i+1]== '\x25' ? '\x0a' : 
						(pData[i+1] == '\x0a' ? 0 : // EBCDEC ��"\x0a��LF�ɂȂ�Ȃ��悤�ɍ׍H����
							(i + 1 < nDataLen ? pData[i+1] : 0))),
					0
				};
				pcEol->SetTypeByStringForFile( szEof, t_min(nDataLen - i,2) );
				neollen = (Int)pcEol->GetLen();
				break;
			}
		}
		break;
	}

	if( neollen < 1 ){
		// EOL���Ȃ������ꍇ
		if( i != nDataLen ){
			i = nDataLen;		// �Ō�̔��[�ȃo�C�g�𗎂Ƃ��Ȃ��悤��
		}
	}else{
		// CR�̏ꍇ�́ACRLF��������Ȃ��̂Ŏ��̃o�b�t�@�֑���
		if( *pcEol == EOL_CR ){
			*pnBufferNext = neollen;
		}
	}

	*pnBgn = i + neollen;
	*pnLineLen = i - nbgn;
	*pnEolLen = neollen;

	return &pData[nbgn];
}
