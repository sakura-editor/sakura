#pragma once

//定数
enum EConvertResult{
	RESULT_COMPLETE, //!< データを失うことなく変換が完了した。
	RESULT_LOSESOME, //!< 変換が完了したが、一部のデータが失われた。
	RESULT_FAILURE,  //!< 何らかの原因により失敗した。
};

#include "CEol.h"
#include "mem/CNativeW.h"
class CMemory;
class CNativeW;

/*!
	文字コード基底クラス。
	
	ここで言う「特定コード」とは、
	CCodeBaseを継承した子クラスが定める、一意の文字コードのことです。
*/
class CCodeBase{
public:
	virtual ~CCodeBase(){}
//	virtual bool IsCode(const CMemory* pMem){return false;}  //!< 特定コードであればtrue

	//文字コード変換
	virtual EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst)=0;	//!< 特定コード → UNICODE    変換
	virtual EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst)=0;	//!< UNICODE    → 特定コード 変換

	//ファイル形式
	virtual void GetBom(CMemory* pcmemBom);											//!< BOMデータ取得
	virtual void GetEol(CMemory* pcmemEol, EEolType eEolType)=0;					//!< 改行データ取得

	// 文字コード表示用		2008/6/9 Uchi
	virtual EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst);			//!< UNICODE → Hex 変換

	// 変換エラー処理（１バイト <-> U+D800 から U+D8FF）
	static int BinToText( const unsigned char*, const int, unsigned short* );
	static int TextToBin( const unsigned short );

	// MIME Header デコーダ
	static bool MIMEHeaderDecode( const char*, const int, CMemory*, const ECodeType );
};

/*!
	バイナリ１バイトを U+DC00 から U+DCFF までに対応付ける
*/
inline int CCodeBase::BinToText( const unsigned char *pSrc, const int nLen, unsigned short *pDst )
{
	int i;

	for( i = 0; i < nLen; ++i ){
		pDst[i] = static_cast<unsigned short>(pSrc[i]) + 0xdc00;
	}

	return i;
}


/*!
	U+DC00 から U+DCFF からバイナリ1バイトを復元
*/
inline int CCodeBase::TextToBin( const unsigned short cSrc )
{
	return static_cast<int>((cSrc - 0xdc00) & 0x00ff);
}
