/*!	@file
	@brief Latin1 (Latin1, ����, Windows-1252, Windows Codepage 1252 West European) �Ή��N���X

	@author Uchi
	@date 20010/03/20 �V�K�쐬
*/
/*
	Copyright (C) 20010, Uchi

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
#ifndef SAKURA_CLATIN1_H_
#define SAKURA_CLATIN1_H_

#include "CCodeBase.h"

class CLatin1 : public CCodeBase{

public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return Latin1ToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToLatin1(pDst); }				//!< UNICODE    �� ����R�[�h �ϊ�
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar);			//!< UNICODE �� Hex �ϊ�

public:
	//����
	static EConvertResult Latin1ToUnicode(CMemory* pMem);		// Latin1   �� Unicode�R�[�h�ϊ�
	static EConvertResult UnicodeToLatin1(CMemory* pMem);		// Unicode  �� Latin1�R�[�h�ϊ�
	static int GetSizeOfChar( const char* pData, int nDataLen, int nIdx ); //!< �w�肵���ʒu�̕��������o�C�g��������Ԃ�

protected:
	// ����
	static int Latin1ToUni( const char*, const int, wchar_t *, bool* pbError );
	inline static int _UniToLatin1_char( const unsigned short*, unsigned char*, const ECharSet, bool* pbError );
	static int UniToLatin1( const wchar_t*, const int, char*, bool *pbError );
};

/*!
	UNICODE -> Latin1 �ꕶ���ϊ�

	eCharset �� CHARSET_UNI_NORMAL �܂��� CHARSET_UNI_SURROG�B

	�������̂��߁A�C�����C����
*/
inline int CLatin1::_UniToLatin1_char( const unsigned short* pSrc, unsigned char* pDst, const ECharSet eCharset, bool* pbError )
{
	int nret;
	bool berror = false;
	BOOL blost;

	if( eCharset == CHARSET_UNI_NORMAL ){
		if ((pSrc[0] >= 0 && pSrc[0] <= 0x7f) || (pSrc[0] >= 0xa0 && pSrc[0] <= 0xff)) {
			// ISO 58859-1�͈̔�
			pDst[0] = (unsigned char)pSrc[0];
			nret = 1;
		} else {
			// ISO 8859-1�ȊO
			nret = ::WideCharToMultiByte( 1252, 0, reinterpret_cast<const wchar_t*>(pSrc), 1, reinterpret_cast<char*>(pDst), 4, NULL, &blost );
			if( blost == TRUE ){
				// Uni -> CLatin1 �ϊ��Ɏ��s
				berror = true;
				pDst[0] = '?';
				nret = 1;
			}
		}
	}else if( eCharset == CHARSET_UNI_SURROG ){
		// �T���Q�[�g�y�A�� CLatin1 �ɕϊ��ł��Ȃ��B
		berror = true;
		pDst[0] = '?';
		nret = 1;
	}else{
		// �ی�R�[�h
		berror = true;
		pDst[0] = '?';
		nret = 1;
	}

	if( pbError ){
		*pbError = berror;
	}

	return nret;
}

#endif /* SAKURA_CLATIN1_H_ */
