/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CSHIFTJIS_092DD5ED_C21B_4122_8A97_CF4EE64B7EFD_H_
#define SAKURA_CSHIFTJIS_092DD5ED_C21B_4122_8A97_CF4EE64B7EFD_H_

#include "CCodeBase.h"
#include "charset/codeutil.h"

class CShiftJis : public CCodeBase{

public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return SJISToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToSJIS(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
// GetEol��CCodeBase�Ɉړ�	2010/6/13 Uchi
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar);			//!< UNICODE �� Hex �ϊ�

public:
	//����
	static EConvertResult SJISToUnicode(CMemory* pMem);		// SJIS      �� Unicode�R�[�h�ϊ�
	static EConvertResult UnicodeToSJIS(CMemory* pMem);		// Unicode   �� SJIS�R�[�h�ϊ�
//	S_GetEol��CCodeBase�Ɉړ�	2010/6/13 Uchi
	// 2005-09-02 D.S.Koba
	// 2007.08.14 kobake CMemory����CShiftJis�ֈړ�
	static int GetSizeOfChar( const char* pData, int nDataLen, int nIdx ); //!< �w�肵���ʒu�̕��������o�C�g��������Ԃ�

protected:
	// ����
	// 2008.11.10 �ϊ����W�b�N����������
	inline static int _SjisToUni_char( const unsigned char*, unsigned short*, const ECharSet, bool* pbError );
	static int SjisToUni( const char*, const int, wchar_t *, bool* pbError );
	inline static int _UniToSjis_char( const unsigned short*, unsigned char*, const ECharSet, bool* pbError );
	static int UniToSjis( const wchar_t*, const int, char*, bool *pbError );
};



/*!
	SJIS �̑S�p�ꕶ���܂��͔��p�ꕶ����Unicode�ւ̕ϊ�

	eCharset �� CHARSET_JIS_ZENKAKU �܂��� CHARSET_JIS_HANKATA �B

	�������̂��߁A�C�����C����
*/
inline int CShiftJis::_SjisToUni_char( const unsigned char *pSrc, unsigned short *pDst, const ECharSet eCharset, bool* pbError )
{
	int nret;
	bool berror = false;

	switch( eCharset ){
	case CHARSET_JIS_HANKATA:
		// ���p�J�^�J�i������
		// �G���[�͋N����Ȃ��B
		nret = MyMultiByteToWideChar_JP( pSrc, 1, pDst );
		// �ی�R�[�h
		if( nret < 1 ){
			nret = 1;
		}
		break;
	case CHARSET_JIS_ZENKAKU:
		// �S�p����������
		nret = MyMultiByteToWideChar_JP( pSrc, 2, pDst );
		if( nret < 1 ){	// SJIS -> Unicode �ϊ��Ɏ��s
			nret = BinToText( pSrc, 2, pDst );
		}
		break;
	default:
		// �v���I�G���[����R�[�h
		berror = true;
		pDst[0] = L'?';
		nret = 1;
	}

	if( pbError ){
		*pbError = berror;
	}

	return nret;
}




/*!
	UNICODE -> SJIS �ꕶ���ϊ�

	eCharset �� CHARSET_UNI_NORMAL �܂��� CHARSET_UNI_SURROG�B

	�������̂��߁A�C�����C����
*/
inline int CShiftJis::_UniToSjis_char( const unsigned short* pSrc, unsigned char* pDst, const ECharSet eCharset, bool* pbError )
{
	int nret;
	bool berror = false;

	if( eCharset == CHARSET_UNI_NORMAL ){
		nret = MyWideCharToMultiByte_JP( pSrc, 1, pDst );
		if( nret < 1 ){
			// Uni -> SJIS �ϊ��Ɏ��s
			berror = true;
			pDst[0] = '?';
			nret = 1;
		}
	}else if( eCharset == CHARSET_UNI_SURROG ){
		// �T���Q�[�g�y�A�� SJIS �ɕϊ��ł��Ȃ��B
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

#endif /* SAKURA_CSHIFTJIS_092DD5ED_C21B_4122_8A97_CF4EE64B7EFD_H_ */
/*[EOF]*/
