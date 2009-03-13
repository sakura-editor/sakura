#pragma once

#include "CCodeBase.h"
#include "charset/codeutil.h"

class CShiftJis : public CCodeBase{

public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return SJISToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToSJIS(pDst); }	//!< UNICODE    → 特定コード 変換
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< 改行データ取得
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst);			//!< UNICODE → Hex 変換

public:
	//実装
	static EConvertResult SJISToUnicode(CMemory* pMem);		// SJIS      → Unicodeコード変換
	static EConvertResult UnicodeToSJIS(CMemory* pMem);		// Unicode   → SJISコード変換
	static void S_GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< 改行データ取得
	// 2005-09-02 D.S.Koba
	// 2007.08.14 kobake CMemoryからCShiftJisへ移動
	static int GetSizeOfChar( const char* pData, int nDataLen, int nIdx ); //!< 指定した位置の文字が何バイト文字かを返す

protected:
	// 実装
	// 2008.11.10 変換ロジックを書き直す
	inline static int _SjisToUni_char( const unsigned char*, unsigned short*, const ECharSet, bool* pbError );
	static int SjisToUni( const char*, const int, wchar_t *, bool* pbError );
	inline static int _UniToSjis_char( const unsigned short*, unsigned char*, const ECharSet, bool* pbError );
	static int UniToSjis( const wchar_t*, const int, char*, bool *pbError );
};



/*!
	SJIS の全角一文字または半角一文字のUnicodeへの変換

	eCharset は CHARSET_JIS_ZENKAKU または CHARSET_JIS_HANKATA 。

	高速化のため、インライン化
*/
inline int CShiftJis::_SjisToUni_char( const unsigned char *pSrc, unsigned short *pDst, const ECharSet eCharset, bool* pbError )
{
	int nret;
	bool berror = false;

	switch( eCharset ){
	case CHARSET_JIS_HANKATA:
		// 半角カタカナを処理
		// エラーは起こらない。
		nret = MyMultiByteToWideChar_JP( pSrc, 1, pDst );
		// 保護コード
		if( nret < 1 ){
			nret = 1;
		}
		break;
	case CHARSET_JIS_ZENKAKU:
		// 全角文字を処理
		nret = MyMultiByteToWideChar_JP( pSrc, 2, pDst );
		if( nret < 1 ){	// SJIS -> Unicode 変換に失敗
			nret = BinToText( pSrc, 2, pDst );
		}
		break;
	default:
		// 致命的エラー回避コード
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
	UNICODE -> SJIS 一文字変換

	eCharset は CHARSET_UNI_NORMAL または CHARSET_UNI_SURROG。

	高速化のため、インライン化
*/
inline int CShiftJis::_UniToSjis_char( const unsigned short* pSrc, unsigned char* pDst, const ECharSet eCharset, bool* pbError )
{
	int nret;
	bool berror = false;

	if( eCharset == CHARSET_UNI_NORMAL ){
		nret = MyWideCharToMultiByte_JP( pSrc, 1, pDst );
		if( nret < 1 ){
			// Uni -> SJIS 変換に失敗
			berror = true;
			pDst[0] = '?';
			nret = 1;
		}
	}else if( eCharset == CHARSET_UNI_SURROG ){
		// サロゲートペアは SJIS に変換できない。
		berror = true;
		pDst[0] = '?';
		nret = 1;
	}else{
		// 保護コード
		berror = true;
		pDst[0] = '?';
		nret = 1;
	}

	if( pbError ){
		*pbError = berror;
	}

	return nret;
}

