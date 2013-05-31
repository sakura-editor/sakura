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
CFileLoad::CFileLoad( SEncodingConfig& encode )
{
	m_pEencoding = &encode;

	m_hFile			= NULL;
	m_nFileSize		= 0;
	m_nFileDataLen	= 0;
	m_CharCode		= CODE_DEFAULT;
	m_pCodeBase		= NULL;////
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
	@param CharCode  [in] �t�@�C���̕����R�[�h�D
	@param nFlag [in] �����R�[�h�̃I�v�V����
	@param pbBomExist [out] BOM�̗L��
	@date 2003.06.08 Moca CODE_AUTODETECT���w��ł���悤�ɕύX
	@date 2003.07.26 ryoji BOM�����ǉ�
*/
ECodeType CFileLoad::FileOpen( LPCTSTR pFileName, ECodeType CharCode, int nFlag, bool* pbBomExist )
{
	HANDLE	hFile;
	DWORD	FileSize;
	DWORD	FileSizeHigh;
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

	FileSize = ::GetFileSize( hFile, &FileSizeHigh );
	// �t�@�C���T�C�Y���A��2GB�𒴂���ꍇ�͂Ƃ肠�����G���[
	if( 0x80000000 <= FileSize || 0 < FileSizeHigh ){
		FileClose();
		throw CError_FileOpen();
	}
	m_nFileSize = FileSize;
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
	if( !IsValidCodeType(CharCode) ){
		CharCode = CODE_DEFAULT;
	}
	m_CharCode = CharCode;
	m_pCodeBase=CCodeFactory::CreateCodeBase(m_CharCode, m_nFlag);
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

/*!
	���̘_���s�𕶎��R�[�h�ϊ����ă��[�h����
	�����A�N�Z�X��p
	GetNextLine�̂悤�ȓ��������
	@return	NULL�ȊO	1�s��ێ����Ă���f�[�^�̐擪�A�h���X��Ԃ��B�i���I�ł͂Ȃ��ꎞ�I�ȗ̈�B
			NULL		�f�[�^���Ȃ�����
*/
EConvertResult CFileLoad::ReadLine(
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
	/*static */CMemory cLineBuffer;
	cLineBuffer.SetRawData("",0);

	// 1�s���o�� ReadBuf -> m_memLine
	//	Oct. 19, 2002 genta while�����𐮗�
	int			nBufLineLen;
	int			nEolLen;
	for (;;) {
		const char* pLine = GetNextLineCharCode(
			m_pReadBuf,
			m_nReadDataLen,    //[in] �o�b�t�@�̗L���f�[�^�T�C�Y
			&nBufLineLen,      //[out]���s���܂܂Ȃ�����
			&m_nReadBufOffSet, //[i/o]�I�t�Z�b�g
			pcEol,
			&nEolLen
		);
		if(pLine==NULL)break;

		// ReadBuf����1�s���擾����Ƃ��A���s�R�[�h��������\�������邽��
		if( m_nReadDataLen <= m_nReadBufOffSet && FLMODE_READY == m_eMode ){// From Here Jun. 13, 2003 Moca
			cLineBuffer.AppendRawData( pLine, nBufLineLen );
			m_nReadBufOffSet -= nEolLen;
			// �o�b�t�@���[�h   File -> ReadBuf
			Buffering();
		}else{
			cLineBuffer.AppendRawData( pLine, nBufLineLen );
			break;
		}
	}
	m_nReadLength += ( nBufLineLen = cLineBuffer.GetRawLength() );

	// �����R�[�h�ϊ� cLineBuffer -> pUnicodeBuffer
	EConvertResult eConvertResult = CIoBridge::FileToImpl(cLineBuffer,pUnicodeBuffer,m_pCodeBase,m_nFlag);
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
	// �f�[�^����
	if( 0 != pUnicodeBuffer->GetStringLength() + nEolLen ){
		// ���s�R�[�h��ǉ�
		pUnicodeBuffer->AppendString( pcEol->GetValue2(), pcEol->GetLen() );
	}
	else{
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
		nBufSize = ( m_nFileSize < gm_nBufSizeDef )?( m_nFileSize ):( gm_nBufSizeDef );
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
	}else if(  0x10000 > m_nFileDataLen ){
		nRet = m_nReadLength * 100 / m_nFileDataLen ;
	}else{
		nRet = m_nReadLength / 128 * 100 / ( m_nFileDataLen / 128 );
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
	int*		pnEolLen	//!< [out]	EOL�̃o�C�g�� (Unicode�ō���Ȃ��悤��)
){
	int nbgn = *pnBgn;
	int i;
	int neollen;

	pcEol->SetType( EOL_NONE );

	if( nDataLen <= nbgn ){
		*pnLineLen = 0;
		*pnEolLen = 0;
		return NULL;
	}

	switch( m_CharCode ){
	case CODE_UNICODE:
		for( i = nbgn; i < nDataLen; i += 2 ){
			wchar_t c = static_cast<wchar_t>((pData[i + 1] << 8) | pData[i]);
			if( WCODE::IsLineDelimiter(c) ){
				pcEol->SetTypeByStringForFile_uni( &pData[i], nDataLen - i );
				break;
			}
		}
		break;
	case CODE_UNICODEBE:
		for( i = nbgn; i < nDataLen; i += 2 ){
			wchar_t c = static_cast<wchar_t>((pData[i] << 8) | pData[i + 1]);
			if( WCODE::IsLineDelimiter(c) ){
				pcEol->SetTypeByStringForFile_unibe( &pData[i], nDataLen - i );
				break;
			}
		}
		break;
	default:
		for( i = nbgn; i < nDataLen; ++i ){
			if( pData[i] == '\r' || pData[i] == '\n' ){
				pcEol->SetTypeByStringForFile( &pData[i], nDataLen - i );
				break;
			}
		}
	}

	neollen = pcEol->GetLen();
	if( m_CharCode == CODE_UNICODE || m_CharCode == CODE_UNICODEBE ){
		neollen *= sizeof(wchar_t);   // EOL �̃o�C�g�����v�Z
		if( neollen < 1 ){
			if( i != nDataLen ){
				i = nDataLen;		// �Ō�̔��[��1�o�C�g�𗎂Ƃ��Ȃ��悤��
			}
		}
	}

	*pnBgn = i + neollen;
	*pnLineLen = i - nbgn;
	*pnEolLen = neollen;

	return &pData[nbgn];
}
