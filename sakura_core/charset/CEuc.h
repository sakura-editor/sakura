#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"
#include <mbstring.h>
#include "charset/codeutil.h"

class CEuc : public CCodeBase{
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return EUCToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToEUC(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< ���s�f�[�^�擾
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst);			//!< UNICODE �� Hex �ϊ�

public:
	//����
	static EConvertResult EUCToUnicode(CMemory* pMem);		// EUC       �� Unicode�R�[�h�ϊ�  //2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToEUC(CMemory* pMem);		// Unicode   �� EUC�R�[�h�ϊ�

protected:
	// ����
	// 2008.11.10 �ϊ����W�b�N����������
	inline static int _EucjpToUni_char( const unsigned char*, unsigned short*, const ECharSet, bool* pbError );
	static int EucjpToUni( const char*, const int, wchar_t*, bool* pbError );
	inline static int _UniToEucjp_char( const unsigned short*, unsigned char*, const ECharSet, bool* pbError );
	static int UniToEucjp( const wchar_t*, const int, char*, bool* pbError );
};



/*!
	EUCJP(CP51932) �̑S�p�ꕶ���܂��͔��p�Љ����ꕶ���̕ϊ�

	eCharset �� CHARSET_JIS_HANKATA �� CHARSET_JIS_ZENKAKU .

	�������̂��߁A�C�����C����
*/
inline int CEuc::_EucjpToUni_char( const unsigned char* pSrc, unsigned short* pDst, const ECharSet eCharset, bool* pbError )
{
	int nret;
	unsigned char czenkaku[2];
	unsigned int ctemp;
	bool berror=false;

	switch( eCharset ){
	case CHARSET_JIS_HANKATA:
		// ���p�J�^�J�i�������B�G���[�͋N����Ȃ�
		nret = MyMultiByteToWideChar_JP( &pSrc[1], 1, pDst );
		// �ی�R�[�h
		if( nret < 1 ){
			nret = 1;
		}
		break;
	case CHARSET_JIS_ZENKAKU:
		// EUCJP(CP51932) �� JIS
		czenkaku[0] = (pSrc[0] & 0x7f);
		czenkaku[1] = (pSrc[1] & 0x7f);
		// JIS �� SJIS
		ctemp = _mbcjistojms( (static_cast<unsigned int>(czenkaku[0]) << 8) | czenkaku[1] );
		if( ctemp != 0 ){
			// NEC�I��IBM�g���R�[�h�|�C���g����IBM�g���R�[�h�|�C���g�ɂɕϊ�
			unsigned int ctemp_ = SjisFilter_nec2ibm( ctemp );
			ctemp = ctemp_;
			// SJIS �� Unicode
			czenkaku[0] = static_cast<unsigned char>( (ctemp & 0x0000ff00) >> 8 );
			czenkaku[1] = static_cast<unsigned char>( ctemp & 0x000000ff );
			nret = MyMultiByteToWideChar_JP( &czenkaku[0], 2, pDst );
			if( nret < 1 ){
				nret = BinToText( pSrc, 2, pDst );
			}
		}else{
			// JIS -> SJIS �̕ϊ��G���[
			// �G���[�����֐����g��
			nret = BinToText( pSrc, 2, pDst );
		}
		break;
	default:
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


/*
	Unicode -> EUCJP �ꕶ���ϊ�

	�������̂��߁A�C�����C����
*/
inline int CEuc::_UniToEucjp_char( const unsigned short* pSrc, unsigned char* pDst, const ECharSet eCharset, bool* pbError )
{
	int nret=0, nclen;
	unsigned char cbuf[4];
	unsigned int ctemp;
	bool berror=false;

	if( eCharset == CHARSET_UNI_SURROG ){
		// �T���Q�[�g�� SJIS �ɕϊ��ł��Ȃ��B
		berror = true;
		pDst[0] = '?';
		nret = 1;
	}else if( eCharset == CHARSET_UNI_NORMAL ){
		nclen = MyWideCharToMultiByte_JP( pSrc, 1, cbuf );
		if( nclen < 1 ){
			// Uni -> SJIS �ŕϊ��G���[
			berror = true;
			pDst[0] = '?';
			nret = 1;
		}else if( nclen == 1 && IsAscii7(cbuf[0]) ){
			// 7bit ASCII �̏���
			pDst[0] = cbuf[0];
			nret = 1;
		}else if( nclen == 1 && IsSjisHankata(cbuf[0]) ){
			// ���p�����̏����F
			pDst[0] = 0x8e;
			pDst[1] = cbuf[0];
			nret = 2;
		}else if( nclen == 2/* && IsSjisZen(reinterpret_cast<char*>(cbuf)) */){
			// �S�p�����̏����F
			// SJIS -> JIS
			unsigned int ctemp_ = SjisFilter_ibm2nec( (static_cast<unsigned int>(cbuf[0]) << 8) | cbuf[1] );
				// < IBM�g��������NEC�I��IBM�g�������ɕϊ�
			ctemp = _mbcjmstojis( ctemp_ );
			if( ctemp == 0 ){
				berror = true;
				pDst[0] = '?';
				nret = 1;
			}else{
				// JIS -> EUCJP
				pDst[0] = static_cast<unsigned char>((ctemp & 0x0000ff00) >> 8) | 0x80;
				pDst[1] = static_cast<unsigned char>(ctemp & 0x000000ff) | 0x80;
				nret = 2;
			}
		}else{
			// �ی�R�[�h
			berror = true;
			pDst[0] = '?';
			nret = 1;
		}
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

