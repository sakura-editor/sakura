/*!	@file
	@brief 文字コード認識ライブラリ

	@author Sakura-Editor collaborators
	@date 1998/03/06 新規作成 by 原作者 Norio Nakatani (C) 1998-2001
	@date 2006/03/06 名称・ライセンス改訂 [文字コード定数の定義] → [文字コード認識ライブラリ]
*/
/*
	Copyright (C) 2006, D. S. Koba, rastiv

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


#ifndef _CHARCODE_H_
#define _CHARCODE_H_


typedef unsigned char	uchar_t;		//  unsigned char の別名．
typedef unsigned short	uchar16_t;		//  UTF-16 用．
typedef unsigned long	uchar32_t;		//  UTF-32 用．
typedef long wchar32_t;




enum ECodeType;
#if 0
/*! 文字コードセット種別 */
enum ECodeType {
	CODE_SJIS,				// MS-CP932(Windows-31J), シフトJIS(Shift_JIS)
	CODE_JIS,				// MS-CP5022x(ISO-2022-JP-MS)
	CODE_EUC,				// MS-CP51932, eucJP-ms(eucJP-open)
	CODE_UNICODE,			// UTF-16 LittleEndian(UCS-2)
	CODE_UTF8,				// UTF-8(UCS-2)
	CODE_UTF7,				// UTF-7(UCS-2)
	CODE_UNICODEBE,			// UTF-16 BigEndian(UCS-2)
	// ...
	CODE_CODEMAX,
	CODE_AUTODETECT	= 99	/* 文字コード自動判別 */
	
	/*
		- MS-CP50220 
			Unicode から cp50220 への変換時に、
			JIS X 0201 片仮名は JIS X 0208 の片仮名に置換される
		- MS-CP50221
			Unicode から cp50221 への変換時に、
			JIS X 0201 片仮名は、G0 集合への指示のエスケープシーケンス ESC ( I を用いてエンコードされる
		- MS-CP50222
			Unicode から cp50222 への変換時に、
			JIS X 0201 片仮名は、SO/SI を用いてエンコードされる
		
		参考
		http://legacy-encoding.sourceforge.jp/wiki/
	*/
};
#endif

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
enum enumJisESCSeqType {
	JISESC_UNKNOWN,
	JISESC_ASCII,
	JISESC_JISX0201Latin,
	JISESC_JISX0201Latin_OLD,
	JISESC_JISX0201Katakana,
	JISESC_JISX0208_1978,
	JISESC_JISX0208_1983,
	JISESC_JISX0208_1990,
};

/*
	文字コード判定情報 構造体群
*/
typedef struct EncodingInfo_t {
	ECodeType eCodeID;		// 文字コード識別番号
	int nSpecBytes;			// 特有バイト数
	int nDiff;				// ポイント数 := 特有バイト数 － 不正バイト数
} MBCODE_INFO;
typedef struct WC_EncodingInfo_t {
	ECodeType eCodeID;		// 文字コード識別番号
	int nCRorLF;			// ワイド文字の改行の個数
	int nLostBytes;			// 不正バイト数
} WCCODE_INFO;
typedef struct UnicodeInfo_t {
	WCCODE_INFO Uni;		// 文字コード判定情報 for UNICODE
	WCCODE_INFO UniBe;		// 文字コード判定情報 for UNICODE BE
	int nCRorLF_ascii;		// マルチバイト文字の改行の個数
} UNICODE_INFO;


#define TAB 				(char)'\t'
#define SPACE				(char)' '
#define CRLF				"\015\012"
#define LFCR				"\012\015"
#define CR					(char)'\015'
#define LF					(char)'\012'
#define ESC					(char)'\x1b'

/*
	関数のエミュレーション
*/
#define _IS_SJIS_1		Charcode::IsSJisKan1
#define _IS_SJIS_2		Charcode::IsSJisKan2
#define UUDECODE_CHAR	Charcode::Uu_CharToVal




namespace Charcode
{
	// BASE64エンコード後に使用する文字
	extern const uchar_t BASE64CHAR[];
	// BASE64デコードする際に使うバイナリ値
	extern const uchar_t BASE64VAL[];
	// UTF7SetD を処理する際に使うブール値
	extern const bool UTF7SetD[];
	// JIS コードのエスケープシーケンス文字列データ
	extern const char JISESCDATA_ASCII[];
	extern const char JISESCDATA_JISX0201Latin[];
	extern const char JISESCDATA_JISX0201Latin_OLD[];
	extern const char JISESCDATA_JISX0201Katakana[];
	extern const char JISESCDATA_JISX0208_1978[];
	extern const char JISESCDATA_JISX0208_1983[];
	extern const char JISESCDATA_JISX0208_1990[];
	extern const int TABLE_JISESCLEN[];
	extern const char* TABLE_JISESCDATA[];
	
	uchar_t __fastcall Base64_CharToVal( const uchar_t );
	uchar_t __fastcall Base64_ValToChar( const uchar_t );
	uchar_t __fastcall Uu_CharToVal( const uchar_t );
	int __fastcall GetJisESCSeqLen( const enumJisESCSeqType );
	const char* __fastcall GetJisESCSeqData( const enumJisESCSeqType );

	/*
	|| 共有実装ヘルパ関数
	*/
	
	// --- 文字コード判別支援
	bool __fastcall IsSJisKan1( const uchar_t );  // SJIS 文字長2 の場合の 0バイト目チェック
	bool __fastcall IsSJisKan2( const uchar_t );  // SJIS 文字長2 の場合の 1バイト目チェック
	bool __fastcall IsSJisKan( const uchar_t* ); // IsSJisKan1 + IsSJisKan2
	bool __fastcall IsSJisHanKata( const uchar_t );  // SJIS 半角カタカナ判別
	bool __fastcall IsEucKan1( const uchar_t );  // EUCJP 文字長2 の場合の 0バイト目チェック
	bool __fastcall IsEucKan2( const uchar_t );  // EUCJP 文字長2 の場合の 1バイト目チェック
	bool __fastcall IsEucKan( const uchar_t* );  // IsEucKan1  + IsEucKan2
	bool __fastcall IsEucHanKata2( const uchar_t );  // EUCJP 半角カタカナ 2バイト目判別  add by genta
	bool __fastcall IsUtf16SurrogHi( const uchar16_t );  // UTF16 文字長4 の場合の 0-1バイト目チェック
	bool __fastcall IsUtf16SurrogLow( const uchar16_t ); // UTF16 文字長4 の場合の 2-3バイト目チェック
#if 0
	bool __fastcall IsUtf16Surrogates( const uchar16_t* ); // UTF16 のサロゲートペア判別
	bool __fastcall IsUtf16SurrogHiOrLow( const uchar16_t );  // UTF16 のサロゲート片判別
#endif
	bool __fastcall IsBase64Char( const uchar_t );  // UTF-7 で使われる Modified BASE64 を判別
	bool __fastcall IsUtf7SetDChar( const uchar_t ); // UTF-7 Set D の文字を判別
	// --- 文字長予測．
	int GuessCharLen_utf8( const uchar_t*, const int nStrLen = 4 );
	int GuessCharLen_sjis( const uchar_t*, const int nStrLen = 2 );
	int GuessCharLen_eucjp( const uchar_t*, const int nStrLen = 3 );
#if 0
	int GuessCharLenAsUtf16_imp( const uchar_t*, const int, bool bBigEndian );
	int GuessCharLenAsUtf16( const uchar_t*, const int nStrLen = 4 );
	int GuessCharLenAsUtf16Be( const uchar_t*, const int nStrLen = 4 );
#endif
	// --- 一文字チェック
	int CheckSJisChar( const uchar_t*, const int );
	int CheckSJisCharR( const uchar_t*, const int );  // CheckCharLenAsSJis の逆方向モード
	int CheckEucJpChar( const uchar_t*, const int );
	int CheckUtf8Char( const uchar_t*, const int );
	int imp_CheckUtf16Char( const uchar_t*, const int, bool );
	int CheckUtf16Char( const uchar_t*, const int );
	int CheckUtf16BeChar( const uchar_t*, const int );
#if 0
	int CheckJisChar_JISX0208( const uchar_t*, const int );
#endif
	// --- 文字長チェック
	int GetCharLen_sjis( const uchar_t*, const int );
	int GetCharLenR_sjis( const uchar_t*, const int );  // GetCharLenAsSJis の逆方向モード
	int GetCharLen_eucjp( const uchar_t*, const int );
	int GetCharLen_utf8( const uchar_t*, const int );
	// --- UTF-7 正当性チェック
	int CheckUtf7SetDPart( const uchar_t*, const int, uchar_t*& ref_pNextC );
	int CheckUtf7SetBPart( const uchar_t*, const int, uchar_t*& ref_pNextC );
	// --- JIS エスケープシーケンス検出
	int DetectJisESCSeq( const uchar_t* pS, const int nLen, int* pnEscType );
	// --- ユニコード BOM 検出器
	ECodeType DetectUnicodeBom( const char*, const int );
	
	/*
	|| 文字列の文字コード情報を得る．
	*/
	
	// --- 文字列チェック
	void GetEncdInf_SJis( const char*, const int, MBCODE_INFO* );
	void GetEncdInf_Jis( const char*, const int, MBCODE_INFO* );
	void GetEncdInf_EucJp( const char*, const int, MBCODE_INFO* );
	void GetEncdInf_Utf8( const char*, const int, MBCODE_INFO* );
	void GetEncdInf_Utf7( const char*, const int, MBCODE_INFO* );
	void GetEncdInf_Uni( const char*, const int, UNICODE_INFO* );
} // ends namespace Charcode.

#endif /* _CHARCODE_H_ */

/*[EOF]*/
