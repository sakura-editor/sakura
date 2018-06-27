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
#ifndef SAKURA_CCODEBASE_79FA6B92_246A_4427_89C9_92E1F1335EB9_H_
#define SAKURA_CCODEBASE_79FA6B92_246A_4427_89C9_92E1F1335EB9_H_

//定数
enum EConvertResult{
	RESULT_COMPLETE, //!< データを失うことなく変換が完了した。
	RESULT_LOSESOME, //!< 変換が完了したが、一部のデータが失われた。
	RESULT_FAILURE,  //!< 何らかの原因により失敗した。
};

class CMemory;
class CNativeW;
struct CommonSetting_Statusbar;
enum EEolType;

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
	//! UNICODE    → 特定コード 変換
	virtual EConvertResult UnicodeToCode(const CStringRef& cSrc, CMemory* pDst){
		CNativeW mem(cSrc.GetPtr(), cSrc.GetLength());
		return UnicodeToCode(mem, pDst);
	}

	//ファイル形式
	virtual void GetBom(CMemory* pcmemBom);											//!< BOMデータ取得
	virtual void GetEol(CMemory* pcmemEol, EEolType eEolType){ S_GetEol(pcmemEol,eEolType); }	//!< 改行データ取得

	// 文字コード表示用		2008/6/9 Uchi
	virtual EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar);			//!< UNICODE → Hex 変換

	// 変換エラー処理（１バイト <-> U+D800 から U+D8FF）
	static int BinToText( const unsigned char*, const int, unsigned short* );
	static int TextToBin( const unsigned short );

	// MIME Header デコーダ
	static bool MIMEHeaderDecode( const char*, const int, CMemory*, const ECodeType );

	// CShiftJisより移動 2010/6/13 Uchi
	static void S_GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< 改行データ取得
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

#endif /* SAKURA_CCODEBASE_79FA6B92_246A_4427_89C9_92E1F1335EB9_H_ */
/*[EOF]*/
