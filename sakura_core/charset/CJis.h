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
#ifndef SAKURA_CJIS_3DD8E095_FA6C_46BA_BDD8_00F658818D72_H_
#define SAKURA_CJIS_3DD8E095_FA6C_46BA_BDD8_00F658818D72_H_

#include "CCodeBase.h"

class CJis : public CCodeBase{
public:
	CJis(bool base64decode = true) : m_base64decode(base64decode) { }
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ return JISToUnicode(cSrc, pDst, m_base64decode); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ return UnicodeToJIS(cSrc, pDst); }	//!< UNICODE    → 特定コード 変換
// GetEolはCCodeBaseに移動	2010/6/13 Uchi
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar);			//!< UNICODE → Hex 変換

public:
	//実装
	static EConvertResult JISToUnicode(const CMemory& cSrc, CNativeW* pDstMem, bool base64decode = true);	// E-Mail(JIS→Unicode)コード変換	//2007.08.13 kobake 追加
	static EConvertResult UnicodeToJIS(const CNativeW& cSrc, CMemory* pDstMem);		// Unicode   → JISコード変換

protected:
	// 2008.11.10  変換ロジックを書き直す
	static int _JisToUni_block( const unsigned char*, const int, unsigned short*, const EMyJisEscseq, bool* pbError );
	static int JisToUni( const char*, const int, wchar_t*, bool* pbError );
	static int _SjisToJis_char( const unsigned char*, unsigned char*, const ECharSet, bool* pbError );
	static int UniToJis( const wchar_t*, const int, char*, bool* pbError );

private:
	//変換方針
	bool m_base64decode;

public:
	//各種判定定数
	// JIS コードのエスケープシーケンス文字列データ
	static const char JISESCDATA_ASCII7[];
	static const char JISESCDATA_JISX0201Latin[];
	static const char JISESCDATA_JISX0201Latin_OLD[];
	static const char JISESCDATA_JISX0201Katakana[];
	static const char JISESCDATA_JISX0208_1978[];
	static const char JISESCDATA_JISX0208_1983[];
	static const char JISESCDATA_JISX0208_1990[];
//	static const int TABLE_JISESCLEN[];
//	static const char* TABLE_JISESCDATA[];
};


#if 0 // codechecker.h に定義されている
/*! JIS コードのエスケープシーケンスたち */
/*
	符号化文字集合       16進表現            文字列表現[*1]
	------------------------------------------------------------
	JIS C 6226-1978      1b 24 40            ESC $ @
	JIS X 0208-1983      1b 24 42            ESC $ B
	JIS X 0208-1990      1b 26 40 1b 24 42   ESC & @ ESC $ B
	JIS X 0212-1990      1b 24 28 44         ESC $ ( D
	JIS X 0213:2000 1面  1b 24 28 4f         ESC $ ( O
	JIS X 0213:2004 1面  1b 24 28 51         ESC $ ( Q
	JIS X 0213:2000 2面  1b 24 28 50         ESC $ ( P
	JIS X 0201 ラテン    1b 28 4a            ESC ( J
	JIS X 0201 ラテン    1b 28 48            ESC ( H         (歴史的[*2])
	JIS X 0201 片仮名    1b 28 49            ESC ( I
	ISO/IEC 646 IRV      1b 28 42            ESC ( B
	
	
	  [*1] 各バイトを便宜的にISO/IEC 646 IRVの文字で表したもの。
	       ただしESCはバイト値1bを表す。
	
	  [*2] JIS X 0201の指示としては使用すべきでないが、古いデータでは
	       使われている可能性がある。
	
	出展：http://www.asahi-net.or.jp/~wq6k-yn/code/
	参考：http://homepage2.nifty.com/zaco/code/
*/
enum EJisESCSeqType {
	JISESC_UNKNOWN,
	JISESC_ASCII,
	JISESC_JISX0201Latin,
	JISESC_JISX0201Latin_OLD,
	JISESC_JISX0201Katakana,
	JISESC_JISX0208_1978,
	JISESC_JISX0208_1983,
	JISESC_JISX0208_1990,
};

#endif

#endif /* SAKURA_CJIS_3DD8E095_FA6C_46BA_BDD8_00F658818D72_H_ */
/*[EOF]*/
