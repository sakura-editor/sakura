/*!	@file
	@brief 文字コード認識・判別支援関数ライブラリ

	@author Sakura-Editor collaborators
	@date 1998/03/06 新規作成
	@date 2006/03/06 名称変更（旧名：文字コード定数の定義）
	@date 2007/03/19 名称改定（旧名：文字コード認識ライブラリ）
*/
/*
	Copyright (C) 2006, D. S. Koba, genta
	Copyright (C) 2007

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
#include "charset/codechecker.h"
#include "mem/CMemory.h"
#include "convert/convert_util2.h"
#include "charset/codeutil.h"
#include "charset/charcode.h"
#include <algorithm>



/* =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


                         データ表と変換補助


*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


*/


/*
	判別テーブル  UTF-7 のどのセットの文字か

	@author D. S. Koba
	@date 2007.04.29 UTF-7 セット O の情報を追加 by rastiv.
*/
// !"#$%&*;<=>@[]^_`{|}
const char TABLE_IsUtf7Direct[] = {
	0, 0, 0, 0, 0, 0, 0, 0,  //00-07:
	0, 1, 1, 0, 0, 1, 0, 0,  //08-0f:TAB, LF, CR
	0, 0, 0, 0, 0, 0, 0, 0,  //10-17:
	0, 0, 0, 0, 0, 0, 0, 0,  //18-1f:
	1, 2, 2, 2, 2, 2, 2, 1,  //20-27:SP, `'`
	1, 1, 2, 0, 1, 1, 1, 1,  //28-2f:(, ), `,`, -, ., /
	1, 1, 1, 1, 1, 1, 1, 1,  //30-37:0 - 7
	1, 1, 1, 2, 2, 2, 2, 1,  //38-3f:8, 9, :, ?
	2, 1, 1, 1, 1, 1, 1, 1,  //40-47:A - G
	1, 1, 1, 1, 1, 1, 1, 1,  //48-4f:H - O
	1, 1, 1, 1, 1, 1, 1, 1,  //50-57:P - W
	1, 1, 1, 2, 0, 2, 2, 2,  //58-5f:X, Y, Z
	2, 1, 1, 1, 1, 1, 1, 1,  //60-67:a - g
	1, 1, 1, 1, 1, 1, 1, 1,  //68-6f:h - o
	1, 1, 1, 1, 1, 1, 1, 1,  //70-77:p - w
	1, 1, 1, 2, 2, 2, 0, 0,  //78-7f:x, y, z
};


#if 0 // 未使用だけど、参考のために書き残し

/*
	JIS エスケープシーケンスデータ
	@author D. S. Koba
*/
const char JISESCDATA_ASCII[]				= "\x1b""(B";
const char JISESCDATA_JISX0201Latin[]		= "\x1b""(J";
const char JISESCDATA_JISX0201Latin_OLD[]	= "\x1b""(H";
const char JISESCDATA_JISX0201Katakana[]	= "\x1b""(I";
const char JISESCDATA_JISX0208_1978[]		= "\x1b""$@";
const char JISESCDATA_JISX0208_1983[]		= "\x1b""$B";
const char JISESCDATA_JISX0208_1990[]		= "\x1b""&@""\x1b""$B";

// 順序は enumJISEscSeqType に依存 (charcode.h にて定義されている)
const int TABLE_JISESCLEN[] = {
	0,		// JISESC_UNKNOWN
	3,		// JISESC_ASCII
	3,		// JISESC_JISX0201Latin
	3,		// JISESC_JISX0201Latin_OLD
	3,		// JISESC_JISX0201Katakana
	3,		// JISESC_JISX0208_1978
	3,		// JISESC_JISX0208_1983
	6,		// JISESC_JISX0208_1990
};
const char* TABLE_JISESCDATA[] = {
	"",
	JISESCDATA_ASCII,
	JISESCDATA_JISX0201Latin,
	JISESCDATA_JISX0201Latin_OLD,
	JISESCDATA_JISX0201Katakana,
	JISESCDATA_JISX0208_1978,
	JISESCDATA_JISX0208_1983,
	JISESCDATA_JISX0208_1990,
};

#endif
















/* =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


                         文字コード判別支援


*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


*/


/*
	SJIS のこと.

	第１バイト |  1000 0001(0x81)         |   1110 0000(0xE0)         |   1010 0001(0xA1)
	           | ～ 1001 1111(0x9F)       |  ～ 1110 1111(0xEF)       |  ～ 1101 1111(0xDF)
	           | → SJIS 全角漢字かなカナ |  → SJIS 全角漢字カナかな |  → 半角カナ
	-----------+--------------------------+---------------------------+-------------------------
	第２バイト |        0100 0000(0x40)  ～  1111 1100(0xFC)          |      ----
	           |         ただし 0111 1111(0x7F) は除く.               |

	参考：「■G-PROJECT■ -日本語文字コードの判別」http://www.gprj.net/dev/tips/other/kanji.shtml
	      「ミケネコの文字コードの部屋」http://mikeneko.creator.club.ne.jp/~lab/kcode/index.html
*/

/*!
	SJIS 文字をチェック

	@param[out] pnCharset 確認した文字コードの種別が格納される

	@return 確認した文字の長さ
*/
int CheckSjisChar( const char* pS, const int nLen, ECharSet *peCharset )
{
	unsigned char uc;

	if( 0 < nLen ){
		uc = pS[0];
		if( (uc & 0x80) == 0 ){
			// ASCII またはローマ字(JIS X 0201 Roman)
			if( peCharset ){
				*peCharset = CHARSET_ASCII7;
			}
			return 1;
		}
		if( IsSjisHankata(static_cast<char>(uc)) ){
			// 半角カナ(JIS X 0201 Kana)
			if( peCharset ){
				*peCharset = CHARSET_JIS_HANKATA;
			}
			return 1;
		}
		if( 1 < nLen && IsSjisZen(pS) ){
			// SJIS 漢字・全角カナかな  (JIS X 0208)
			if( peCharset ){
				*peCharset = CHARSET_JIS_ZENKAKU;
			}
			return 2;
		}
		if( peCharset ){
			*peCharset = CHARSET_BINARY;
		}
		return 1;
	}
	return 0;
}




/*
	EUC-JP のこと.

	第1バイト |   1000 1110(0x8E)   |  1000 1111(0x8F)    |  1010 0001(0xA1) ～ 1111 1110(0xFE)
	          |   → 半角カナ       |  → 補助漢字        |  → 漢字かなカナ
	----------+---------------------+---------------------+-------------------------------------
	第2バイト |  1010 0001(0xA1)    |   1010 0001(0xA1)   |      1010 0001(0xA1)
	          | ～ 1101 1111(0xDF)  |  ～ 1111 1110(0xFE) |     ～ 1111 1110(0xFE)
	----------+---------------------+---------------------+-------------------------------------
	第3バイト |        ----         |   1010 0001(0xA1)   |        ----
	          |                     |  ～ 1111 1110(0xFE) |

	参考：「■G-PROJECT■ -日本語文字コードの判別」http://www.gprj.net/dev/tips/other/kanji.shtml
	      「ミケネコの文字コードの部屋」http://mikeneko.creator.club.ne.jp/~lab/kcode/index.html
*/

/*!
	EUC-JP 文字をチェック

	@sa CheckSjisChar()

	@date 2006.09.23 EUCJP 半角カタカナ判別が間違っていたのを修正．genta
*/
int CheckEucjpChar( const char* pS, const int nLen, ECharSet *peCharset )
{
	unsigned char uc;

	if( 0 < nLen ){
		uc = pS[0];
		if( (uc & 0x80) == 0 ){
			// ASCII またはローマ字です.  (JIS X 0201 Roman.)
			if( peCharset ){
				*peCharset = CHARSET_ASCII7;
			}
			return 1;
		}
		if( 1 < nLen ){
			if( IsEucjpZen(pS) ){
				// EUC-JP 漢字・かなカナ です.  (JIS X 0208.)
				if( peCharset ){
					*peCharset = CHARSET_JIS_ZENKAKU;
				}
				return 2;
			}
			if( IsEucjpHankata(pS) ){
				// 半角カナです.  (JIS X 0201 Kana.)
				if( peCharset ){
					*peCharset = CHARSET_JIS_HANKATA;
				}
				return 2;
			}
			if( 2 < nLen ){
				if( IsEucjpSupplemtal(pS) ){
					// EUC-JP 補助漢字です.  (JIS X 0212.)
					if( peCharset ){
						*peCharset = CHARSET_JIS_SUPPLEMENTAL;
					}
					return 3;
				}
			}
		}
		if( peCharset ){
			*peCharset = CHARSET_BINARY;
		}
		return 1;
	}
	return 0;
}






/*!
	JIS の エスケープ文字列を検出する

	@param [in]  pS         調査データ
	@param [in]  nLen       調査データ長
	@param [out] peEscType  検出されたエスケープ文字列の種類

	@retval n == 0 調査データがない
	@retval n > 0 エスケープシーケンスの長さ
	@retval n < 0 エスケープシーケンスが検出されなかった

	@note
		戻り値がゼロより大きい場合に限り，*pnEscType が更新される．\n
		pnEscType は NULL でも良い．\n
*/
int DetectJisEscseq( const char* pS, const int nLen, EMyJisEscseq* peEscType )
{
	const char *pr, *pr_end;
	int expected_esc_len;
	EJisEscseq ejisesc;
	EMyJisEscseq emyjisesc;

	if( nLen < 1 ){
		*peEscType = MYJISESC_NONE;
		return 0;
	}

	ejisesc = JISESC_UNKNOWN;
	expected_esc_len = 0;
	pr = const_cast<char*>( pS );
	pr_end = pS + nLen;

	if( pr[0] == ACODE::ESC ){
		expected_esc_len++;
		pr++;
		if( pr + 1 < pr_end ){
			expected_esc_len += 2;
			if( pr[0] == '(' ){
				if( pr[1] == 'B' ){
					ejisesc = JISESC_ASCII;				// ESC ( B  -  ASCII
				}else if( pr[1] == 'J'){
					ejisesc = JISESC_JISX0201Latin;		// ESC ( J  -  JIS X 0201 ラテン
				}else if( pr[1] == 'H'){
					ejisesc = JISESC_JISX0201Latin_OLD;	// ESC ( H  -  JIS X 0201 ラテン
				}else if( pr[1] == 'I' ){
					ejisesc = JISESC_JISX0201Katakana;	// ESC ( I  -  JIS X 0201 片仮名
				}
			}else if( pr[0] == '$' ){
				if( pr[1] == 'B' ){
					ejisesc = JISESC_JISX0208_1983;		// ESC $ B  -  JIS X 0208-1983
				}else if( pr[1] == '@' ){
					ejisesc = JISESC_JISX0208_1978;		// ESC $ @  -  JIS X 0208-1978  (旧JIS)
				}
			}
		}else if( pr + 4 < pr_end ){
			expected_esc_len += 5;
			if( 0 == strncmp( pr, "&@\x1b$B", 5 ) ){
				ejisesc = JISESC_JISX0208_1990;			// ESC & @ ESC $ B  -  JIS X 0208-1990
			}
		}
	}

	// 検出されたJIS エスケープシーケンス識別ＩＤを
	// 内部の JIS エスケープシーケンス識別ＩＤに変換
	switch( ejisesc ){
	case JISESC_ASCII:
	case JISESC_JISX0201Latin_OLD:
	case JISESC_JISX0201Latin:
		emyjisesc = MYJISESC_ASCII7;
		break;
	case JISESC_JISX0201Katakana:
		emyjisesc = MYJISESC_HANKATA;
		break;
	case JISESC_JISX0208_1978:
	case JISESC_JISX0208_1990:
	case JISESC_JISX0208_1983:
		emyjisesc = MYJISESC_ZENKAKU;
		break;
	default:
		if( 0 < expected_esc_len ){
			emyjisesc = MYJISESC_UNKNOWN;
		}else{
			emyjisesc = MYJISESC_NONE;
		}
	}

	*peEscType = emyjisesc;
	return expected_esc_len;
}



/*!
	JIS 文字列をチェック

	今のエスケープシーケンスから次のエスケープシーケンスに変わる間をブロックと便宜的に呼んでいます。
*/
int _CheckJisAnyPart(
		const char *pS,			// [in]    チェック対象となるバッファポインタ
		const int nLen,			// [in]    チェック対象となるバッファの長さ
		const char **ppNextChar,		// [out]   次のエスケープシーケンス文字列の次の文字へのポインタ
								//       つまり、次に検査を開始する文字列（先頭のエスケープシーケンスを含めない）へのポインタ
		EMyJisEscseq *peNextEsc,// [out]   次のエスケープシーケンスの種類
		int *pnErrorCount,		// [out]   ブロック中の不正文字数
		const int nType			// [in]    今のエスケープシーケンスの種類
)
{
	EMyJisEscseq emyesc = MYJISESC_NONE;
	int nesclen;
	int nerror_cnt;
	const char *pr, *pr_end;

	if( nLen < 1 ){
		*peNextEsc = MYJISESC_NONE;
		*ppNextChar = const_cast<char*>( pS );
		*pnErrorCount = 0;
		return 0;
	}

	nerror_cnt = 0;
	nesclen = 0;
	pr = pS;
	pr_end = pS + nLen;

	for( ; pr < pr_end; pr++ ){
		nesclen = DetectJisEscseq( pr, pr_end-pr, &emyesc );  // 次のエスケープシーケンスを検索
		if( emyesc != MYJISESC_NONE || nesclen > 0 ){
			// 長さ nesclen の JIS エスケープシーケンス（種類 emyesc）が見つかった
			break;
		}
		if( pnErrorCount ){
			switch( nType ){
			case JISCHECK_ASCII7:
				if( !IsAscii7(*pr) ){
					nerror_cnt++;
				}
				break;
			case JISCHECK_HANKATA:
				if( !IsJisHankata(*pr) ){
					nerror_cnt++;
				}
				break;
			case JISCHECK_ZENKAKU:
				if( (pr-pS+1) % 2 == 0 ){
					if( !IsJisZen(pr-1) ){
						nerror_cnt += 2;
					}
				}
				break;
			default:
				if( !IsJis(*pr) ){
					nerror_cnt++;
				}
			}
		}
	}
	if( pnErrorCount ){
		*pnErrorCount = nerror_cnt;
	}

	*peNextEsc = emyesc;
	if( pr < pr_end ){
		*ppNextChar = const_cast<const char*>(pr) + nesclen;
	}else{
		*ppNextChar = const_cast<const char*>(pr_end);
		pr = pr_end;
	}

	return pr - pS;
}














/*
	UTF-16 サロゲート処理のこと.

	U+10000 から U+10FFFF の文字値 a0 に対しては,

		a0 = HHHHHHHHHHLLLLLLLLLL  U+10000 ～ U+10FFFF
		w1 = 110110HH HHHHHHHH     上位サロゲート：U+D800 ～ U+DBFF
		w2 = 110111LL LLLLLLLL     下位サロゲート：U+DC00 ～ U+DFFF

	1. 0x10000 を引き, 20ビットの文字値 a1 (0x00000 ～ 0xFFFFF) で表現した後,
	     a1 ← a0 - 0x10000
	2. 上位 10ビットを w1, 下位 10ビットを w2 に分け,
	     w1 ← (a1 & 0xFFC0) >> 6
	     w2 ←  a1 & 0x03FF
	3. w1, w2 の上位 6ビット分の空き領域を, それぞれ 110110 と 110111 で埋める.
	     w1 ← w1 | 0xD800
	     w2 ← w2 | 0xDC00


	U+FFFE, U+FFFF は, 未定義値.

	参考資料：「UCSとUTF」http://homepage1.nifty.com/nomenclator/unicode/ucs_utf.htm
*/

/*!
	UTF-16 LE/BE 文字をチェック　(組み合わせ文字列考慮なし)
*/
int _CheckUtf16Char( const wchar_t* pS, const int nLen, ECharSet *peCharset, const int nOption, const bool bBigEndian )
{
	wchar_t wc1, wc2 = 0;
	int ncwidth;
	ECharSet echarset;

	if( nLen < 1 ){
		return 0;
	}

	echarset = CHARSET_UNI_NORMAL;

	// 文字を読み込む

	wc1 = pS[0];
	if( bBigEndian == true ){
		wc1 = _SwapHLByte( wc1 );
	}
	if( 1 < nLen ){
		wc2 = pS[1];
		if( bBigEndian == true ){
			wc2 = _SwapHLByte( wc2 );
		}
	}

	if( 2 <= nLen ){

		// サロゲートペアの確認

		if( IsUtf16SurrogHi(wc1) && IsUtf16SurrogLow(wc2) ){
			echarset = CHARSET_UNI_SURROG;
			ncwidth = 2;
			goto EndFunc;
		}
	}

	// サロゲート断片の確認

	if( IsUtf16SurrogHi(wc1) || IsUtf16SurrogLow(wc1) ){
		echarset = CHARSET_BINARY;
		ncwidth = 1;
		goto EndFunc;
	}

	// サロゲートペアでない文字
	ncwidth = 1;

	// 非文字と予約コードポイントの確認
	if( nOption != 0 && echarset != CHARSET_BINARY ){
		if( ncwidth == 1 ){
			if( (nOption & UC_NONCHARACTER) && IsUnicodeNoncharacter(wc1) ){
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
		}else if( ncwidth == 2 ){
			wchar32_t wc32_checking = DecodeUtf16Surrog( wc1, wc2 );
			if( (nOption & UC_NONCHARACTER) && IsUnicodeNoncharacter(wc32_checking) ){
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
		}else{
			// 保護コード
			echarset = CHARSET_BINARY;
			ncwidth = 1;
		}
	}


EndFunc:;
	if( peCharset ){
		*peCharset = echarset;
	}

	return ncwidth;
}







/* -------------------------------------------------------------------------------------------------------------- *
UTF-8のコード
ビット列		内容
0xxx xxxx	1バイトコードの先頭
110x xxxx	2バイトコードの先頭
1110 xxxx	3バイトコードの先頭
1111 0xxx	4バイトコードの先頭
10xx xxxx	UTF-8 バイトコードの 2 バイト目以降

UTF-8のエンコーディング

ビット列                  MSB -         UCS ビット列         - LSB     第1バイト  第2バイト  第3バイト  第4バイト
\u0～\u7F         (UCS2)  0000 0000 0000 0000  0000 0000 0aaa bbbb  -> 0aaa bbbb     ---        ---        ---
\u80～\u7FF       (UCS2)  0000 0000 0000 0000  0000 0aaa bbbb cccc  -> 110a aabb  10bb cccc     ---        ---
\u800～\uFFFF     (UCS2)  0000 0000 0000 0000  aaaa bbbb cccc dddd  -> 1110 aaaa  10bb bbcc  10cc dddd     ---
\u10000～\u1FFFFF (UCS4)  0000 0000 000a bbbb  cccc dddd eeee ffff  -> 1111 0abb  10bb cccc  10dd ddee  10ee ffff

参考資料：「UCSとUTF」http://nomenclator.la.coocan.jp/unicode/ucs_utf.htm
* --------------------------------------------------------------------------------------------------------------- */

/*!
	UTF-8 文字をチェック　(組み合わせ文字列考慮なし)

	@sa CheckSjisChar()

	@date 2008/11/01 syat UTF8ファイルで欧米の特殊文字が読み込めない不具合を修正
*/
int CheckUtf8Char( const char *pS, const int nLen, ECharSet *peCharset, const bool bAllow4byteCode, const int nOption )
{
	unsigned char c0, c1, c2, c3;
	int ncwidth;
	ECharSet echarset;

	if( nLen < 1 ){
		return 0;
	}

	echarset = CHARSET_UNI_NORMAL;
	c0 = pS[0];

	if( c0 < 0x80 ){	// 第１バイトが 0aaabbbb の場合
		ncwidth = 1;	// １バイトコードである
		goto EndFunc;
	}else
	if( 1 < nLen && (c0 & 0xe0) == 0xc0 ){	// 第１バイトが110aaabbの場合
		c1 = pS[1];
		// 第２バイトが10bbccccの場合
		if( (c1 & 0xc0) == 0x80 ){
			ncwidth = 2;	// ２バイトコードである
			// 第１バイトがaaabb=0000xの場合（\u80未満に変換される）
			if( (c0 & 0x1e) == 0 ){
				// デコードできない.(往復変換不可領域)
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
			goto EndFunc;
		}
	}else
	if( 2 < nLen && (c0 & 0xf0) == 0xe0 ){	// 第１バイトが1110aaaaの場合
		c1 = pS[1];
		c2 = pS[2];
		// 第２バイトが10bbbbcc、第３バイトが10ccddddの場合
		if( (c1 & 0xc0) == 0x80 && (c2 & 0xc0) == 0x80 ){
			ncwidth = 3;	// ３バイトコードである
			// 第１バイトのaaaa=0000、第２バイトのbbbb=0xxxの場合(\u800未満に変換される)
			if( (c0 & 0x0f) == 0 && (c1 & 0x20) == 0 ){
				// デコードできない.(往復変換不可領域)
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
			//if( (c0 & 0x0f) == 0x0f && (c1 & 0x3f) == 0x3f && (c2 & 0x3e) == 0x3e ){
			//	// Unicode でない文字(U+FFFE, U+FFFF)
			//	charset = CHARSET_BINARY;
			//	ncwidth = 1;
			//}
			if( bAllow4byteCode == true && (c0 & 0x0f) == 0x0d && (c1 & 0x20) != 0 ){
				// サロゲート領域 (U+D800 から U+DFFF)
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
			goto EndFunc;
		}
	}else
	if( 3 < nLen && (c0 & 0xf8) == 0xf0 ){	// 第１バイトが11110abbの場合
		c1 = pS[1];
		c2 = pS[2];
		c3 = pS[3];
		// 第2バイトが10bbcccc、第3バイトが10ddddee、第4バイトが10ddddeeの場合
		if( (c1 & 0xc0) == 0x80 && (c2 & 0xc0) == 0x80 && (c3 & 0xc0) == 0x80 ){
			ncwidth = 4;  // ４バイトコードである
			echarset = CHARSET_UNI_SURROG;  // サロゲートペアの文字（初期化）
			// 第1バイトのabb=000、第2バイトのbb=00の場合（\u10000未満に変換される）
			if( (c0 & 0x07) == 0 && (c1 & 0x30) == 0 ){
				// デコードできない.(往復変換不可領域)
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
			// １バイト目が 11110xxx=11110100のとき、
			// かつ、1111 01xx : 10xx oooo の x のところに値があるとき
			if( (c0 & 0x04) != 0 && ((c0 & 0x03) != 0 || (c1 & 0x30) != 0) ){
				// 値が大きすぎ（0x10ffffより大きい）
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
			if( bAllow4byteCode == false ){
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
			goto EndFunc;
		}
	}

	// 規定外のフォーマット
	echarset = CHARSET_BINARY;
	ncwidth = 1;

EndFunc:

	// 非文字と予約コードポイントをチェック
	if( nOption != 0 && echarset != CHARSET_BINARY ){
		wchar32_t wc32;
		wc32 = DecodeUtf8( reinterpret_cast<const unsigned char*>(pS), ncwidth );
		if( (nOption & UC_NONCHARACTER) && IsUnicodeNoncharacter(wc32) ){
			echarset = CHARSET_BINARY;
			ncwidth = 1;
		}else{
			// 保護コード
			echarset = CHARSET_BINARY;
			ncwidth = 1;
		}
	}

	if( peCharset ){
		*peCharset = echarset;
	}
	return ncwidth;
}

/*!
	UTF-8 文字をチェック　(組み合わせ文字列考慮なし)

	@note 途中までUTF-8のエンコーディングが合っていれば、CHARSET_BINARY2を設定する

	@date 2015.12.30 novice  第１バイトが11110abbのとき、nLenより大きい値を返すのを修正
*/
int CheckUtf8Char2( const char *pS, const int nLen, ECharSet *peCharset, const bool bAllow4byteCode, const int nOption )
{
	unsigned char c0, c1, c2;
	int ncwidth;
	ECharSet echarset;

	if( nLen < 1 ){
		return 0;
	}

	ncwidth = CheckUtf8Char( pS, nLen, &echarset, true, 0 );
	c0 = pS[0];
	if( echarset == CHARSET_BINARY ){
		if( 1 == nLen && (c0 & 0xe0) == 0xc0 ){	// 第１バイトが110aaabbの場合
			echarset = CHARSET_BINARY2; // 文字列断片(継続用)
			ncwidth = 1;
			goto EndFunc;
		}else
		if( 2 == nLen && (c0 & 0xf0) == 0xe0 ){	// 第１バイトが1110aaaaの場合
			c1 = pS[1];
			// 第２バイトが10bbbbcc、第３バイトが10ccddddの場合
			if( (c1 & 0xc0) == 0x80 ){
				echarset = CHARSET_BINARY2; // 文字列断片(継続用)
				ncwidth = 2;	// ３バイトコードの先頭2バイトである
				if( (c0 & 0x0f) == 0 && (c1 & 0x20) == 0 ){
					// デコードできない.(往復変換不可領域)
					echarset = CHARSET_BINARY;
					ncwidth = 1;
				}
				//if( (c0 & 0x0f) == 0x0f && (c1 & 0x3f) == 0x3f && (c2 & 0x3e) == 0x3e ){
				//	// Unicode でない文字(U+FFFE, U+FFFF)
				//	charset = CHARSET_BINARY;
				//	ncwidth = 1;
				//}
				if( bAllow4byteCode == true && (c0 & 0x0f) == 0x0d && (c1 & 0x20) != 0 ){
					// サロゲート領域 (U+D800 から U+DFFF)
					echarset = CHARSET_BINARY;
					ncwidth = 1;
				}
				goto EndFunc;
			}
		}else
		if( 1 == nLen && (c0 & 0xf0) == 0xe0 ){	// 第１バイトが1110aaaaの場合
			echarset = CHARSET_BINARY2; // 文字列断片(継続用)
			ncwidth = 1;
			goto EndFunc;
		}else
		if( 0 < nLen && nLen <= 3 && (c0 & 0xf8) == 0xf0 ){	// 第１バイトが11110abbの場合
			if( 1 < nLen ){
				c1 = pS[1];
			}else{
				c1 = 0xbf;
			}
			if( 2 < nLen ){
				c2 = pS[2];
			}else{
				c2 = 0xbf;
			}
			// 第2バイトが10bbcccc、第3バイトが10ddddee
			if( (c1 & 0xc0) == 0x80 && (c2 & 0xc0) == 0x80 ){
				ncwidth = nLen;  // ４バイトコードである
				echarset = CHARSET_BINARY2; // 文字列断片(継続用)
				// 第1バイトのabb=000、第2バイトのbb=00の場合（\u10000未満に変換される）
				if( (c0 & 0x07) == 0 && (c1 & 0x30) == 0 ){
					// デコードできない.(往復変換不可領域)
					echarset = CHARSET_BINARY;
					ncwidth = 1;
				}
				// １バイト目が 11110xxx=11110100のとき、
				// かつ、1111 01xx : 10xx oooo の x のところに値があるとき
				if( (c0 & 0x04) != 0 && (c0 & 0x03) != 0 ){
					// 値が大きすぎ（0x10ffffより大きい）
					echarset = CHARSET_BINARY;
					ncwidth = 1;
				}
				if( bAllow4byteCode == false ){
					echarset = CHARSET_BINARY;
					ncwidth = 1;
				}
				goto EndFunc;
			}
		}
	}else{
		goto EndFunc;
	}

	// 規定外のフォーマット
	echarset = CHARSET_BINARY;
	ncwidth = 1;

EndFunc:

	if( peCharset ){
		*peCharset = echarset;
	}
	return ncwidth;
}

/*
	CESU-8 文字のチェック　(組み合わせ文字列考慮なし)
*/
int CheckCesu8Char( const char* pS, const int nLen, ECharSet* peCharset, const int nOption )
{
	ECharSet echarset1, echarset2, eret_charset;
	int nclen1, nclen2, nret_clen;

	if( nLen < 1 ){
		return 0;
	}

	// １文字目のスキャン
	nclen1 = CheckUtf8Char( &pS[0], nLen, &echarset1, false, 0 );

	// 文字長が３未満の場合
	if( nclen1 < 3 ){
		// echarset == BAINARY の場合は、からなず nclen1 < 3
		eret_charset = echarset1;
		nret_clen = nclen1;
	}else
	// 文字長が３の場合
	if( nclen1 == 3 ){
		// 正常な３バイト文字があった。

		// ２文字目のスキャン
		nclen2 = CheckUtf8Char( &pS[3], nLen-3, &echarset2, false, 0 );

		// &pS[3]からの文字長が３でないか echarset2 が CHARSET_BINARY だった場合。
		if( nclen2 != 3 || echarset2 == CHARSET_BINARY ){
			// nclen1 と echarset1 を結果とする。
			eret_charset = echarset1;
			nret_clen = nclen1;
			// &pS[0] から３バイトがサロゲート片だった場合。
			if( IsUtf8SurrogHi(&pS[0]) || IsUtf8SurrogLow(&pS[0]) ){
				eret_charset = CHARSET_BINARY;
				nret_clen = 1;
			}
			goto EndFunc;
		}

		//    nclen1 == 3 && echarset1 != CHARSET_BINARY
		// && nclen2 == 3 && echarset2 != CHARSET_BINARY の場合。

		// UTF-8版サロゲートペアを確認。
		if( IsUtf8SurrogHi(&pS[0]) && IsUtf8SurrogLow(&pS[3]) ){
			// CESU-8 であるかどうかをチェック
			eret_charset = CHARSET_UNI_SURROG;
			nret_clen = 6;  // CESU-8 のサロゲートである
		}else
		// &pS[0] から３バイトがサロゲート片だった場合。
		if( IsUtf8SurrogHi(&pS[0]) || IsUtf8SurrogLow(&pS[0]) ){
			eret_charset = CHARSET_BINARY;
			nret_clen = 1;
		}else
		// 通常の３バイト文字
		{
			eret_charset = echarset1;
			nret_clen = 3;
		}
	}else
	// 文字長が３より大きい場合
	{  // nclen1 == 4
		// UTF-16 サロゲートに変換される領域
		// 4バイトコードは禁止
		eret_charset = CHARSET_BINARY;
		nret_clen = 1;
	}

EndFunc:;


	// 非文字と予約コードポイントを確認
	if( nOption != 0 && eret_charset != CHARSET_BINARY ){
		wchar32_t wc32;
		if( nret_clen < 4 ){
			wc32 = DecodeUtf8( reinterpret_cast<const unsigned char*>(pS), nret_clen );
			if( (nOption & UC_NONCHARACTER) && IsUnicodeNoncharacter(wc32) ){
				eret_charset = CHARSET_BINARY;
				nret_clen = 1;
			}
		}else if( nret_clen == 6 ){
			wc32 = DecodeUtf16Surrog(
				static_cast<unsigned short>(DecodeUtf8(reinterpret_cast<const unsigned char*>(&pS[0]), 3) & 0x0000ffff),
				static_cast<unsigned short>(DecodeUtf8(reinterpret_cast<const unsigned char*>(&pS[3]), 3) & 0x0000ffff) );
			if( (nOption & UC_NONCHARACTER) && IsUnicodeNoncharacter(wc32) ){
				eret_charset = CHARSET_BINARY;
				nret_clen = 1;
			}
		}else{
			// 保護コード
			eret_charset = CHARSET_BINARY;
			nret_clen = 1;
		}
	}

	if( peCharset ){
		*peCharset = eret_charset;
	}
	return nret_clen;
}




/*
	UTF-7 のこと.

	UTF-7 セットD：　半角英数字、'(),-./:?、および、TAB SP CR LF
	UTF-7 セットO：　!"#$%&*;<=>@[]^_`{|}
	UTF-7 セットB：　パッド文字を除く BASE64 文字 (Modified Base 64)

	1. セットD または セットO にあるユニコード文字は, 等価な ASCII 文字で表現される.
	2. セットD または セットO にないユニコード文字は, Modified Base 64 符号化され, セットB 文字列によって表現される.
	3. セットB の開始は, ASCII 文字 '+' によって, セットB 文字列の終端は, セットB にない文字の出現によって認識される.
	   セットB 文字列の終端部分には, セットB にない ASCII 文字 '-' を入れてもよいことになっていて,
	   その終端文字は, デコーダーにより, 見つかり次第削除される.
	4. セットB 開始文字である ASCII 文字 '+' 自体は, "+-" という文字列で表される.

	参考資料：「UCSとUTF」http://homepage1.nifty.com/nomenclator/unicode/ucs_utf.htm
	          「RFC 2152」http://www.ietf.org/rfc/rfc2152.txt
*/

/*!
	UTF-7 セットＤの文字列か

	@return セットＤの文字列の長さ

	@param[out] ppNextChar 次のブロック（UTF-7セットB部分）の先頭文字のポインタが格納される。（'+'を飛ばす）

	pbError を NULL 以外に設定していて、pbError に true が格納された場合、
	戻り値と ppNextChar に格納されるポインタは使えない。
	1つ以上のエラーが見つかれば候補から外れるのでそういう適当な仕様に。
*/
int CheckUtf7DPart( const char *pS, const int nLen, char **ppNextChar, bool *pbError )
{
	const char *pr, *pr_end;
	bool berror = false;

	if( nLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		*ppNextChar = const_cast<char*>( pS );
		return 0;
	}

	pr = pS;
	pr_end = pS + nLen;
	for( ; pr < pr_end; ++pr ){
		if( *pr == '+' ){
			break;
		}
		if( !IsUtf7Direct(*pr) ){
			// UTF-7セットDの文字集合でないものが１文字でも入っている場合、
			// エラーを返す。*pbError == true の場合は、
			// *ppNextChar は不定となる。
			berror = true;
//			break;    // 無限ループになるのでここで break しない。
		}
	}
	if( pbError ){
		*pbError = berror;
	}

	if( pr < pr_end ){
		// '+' をスキップ
		*ppNextChar = const_cast<char*>(pr) + 1;
	}else{
		*ppNextChar = const_cast<char*>(pr);
	}
	return pr - pS;
}




/*!
	UTF-7 セットＢの文字列か

	@return セットＢ文字列の長さ

	@param[out] ppNextChar 次のブロック（UTF-7セットD部分）の先頭文字のポインタが格納される（文字'-'を飛ばす）

	@note この関数の前に CheckUtf7DPart() が実行される必要がある。
*/
int CheckUtf7BPart( const char *pS, const int nLen, char **ppNextChar, bool *pbError, const int nOption, bool* pbNoAddPoint )
{
	const char *pr, *pr_end;
	bool berror_found, bminus_found;
	int nchecklen;

	wchar_t* pdata;
	int ndatalen, nret;
	ECharSet echarset;
	CMemory cmbuffer;

	if( pbNoAddPoint ){
		*pbNoAddPoint = false;
	}

	if( nLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		*ppNextChar = const_cast<char*>( pS );
		return 0;
	}

	berror_found = false;
	bminus_found = false;
	pr = pS;
	pr_end = pS + nLen;

	for( ; pr < pr_end; ++pr ){
		// セットＢの文字でなくなるまでループ
		if( !IsBase64(*pr) ){
			if( *pr == '-' ){
				bminus_found= true;
			}else{
				bminus_found = false;
			}
			break;
		}
	}

	nchecklen = pr - pS;

	// 保護コード
	if( nchecklen < 1 ){
		nchecklen = 0;
	}


	/*
	◆ デコード後のデータ長の確認

	調査してきたデータ長 nchecklen(= pr - pS) を８で割ってみる.
	その余りの値から考えられるビット列は…

	             |----------------------------- Base64 表現 --------------------------------------------|
	             第1バイト  第2バイト  第3バイト  第4バイト  第5バイト  第6バイト  第7バイト  第8バイト
	残り１文字   00xx xxxx  00xx xxxx  00xx xx00     ---        ---        ---        ---        ---
	残り２文字   00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx 0000     ---        ---
	残り３文字   00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx

	上記３通りのいづれにも当てはまらない場合は全データを落とす（不正バイトとする）.
	*/
	const char *pr_ = pr - 1;
	switch( nchecklen % 8 ){
	case 0:
		break;
	case 3:
		if( Base64ToVal(pr_[0]) & 0x03 ){
			berror_found = true;
		}
		break;
	case 6:
		if( Base64ToVal(pr_[0]) & 0x0f ){
			berror_found = true;
		}
		break;
	case 8:
		// nchecklen == 0 の場合
		break;
	default:
		berror_found = true;
	}

	if( UC_LOOSE == (nOption & UC_LOOSE) ){
		goto EndFunc;
	}

	// UTF-7文字列 "+-" のチェック

	if( pr < pr_end && (nchecklen < 1 && bminus_found != true) ){
		// 読み取りポインタがデータの終端を指していなくて
		// 確認できた Set B 文字列の長さがゼロの場合は、
		// 必ず終端文字 '-' が存在していることを確認する。
		berror_found = true;
	}

	// 実際にデコードして内容を確認する。

	if( berror_found == true || nchecklen < 1 ){
		goto EndFunc;
	}

	cmbuffer.AllocBuffer( nchecklen );
	pdata = reinterpret_cast<wchar_t*>( cmbuffer.GetRawPtr() );
	if( pdata == NULL ){
		goto EndFunc;
	}
	ndatalen = _DecodeBase64(pS, nchecklen, reinterpret_cast<char*>(pdata)) / sizeof(wchar_t);
	CMemory::SwapHLByte( reinterpret_cast<char*>(pdata), ndatalen*sizeof(wchar_t) );
	for( int i = 0; i < ndatalen; i += nret ){
		nret = CheckUtf16leChar( &pdata[i], ndatalen - i, &echarset, nOption & UC_NONCHARACTER );
		if( echarset == CHARSET_BINARY ){
			berror_found = true;
			goto EndFunc;
		}
		if( nret == 1 && IsUtf7SetD(pdata[i]) ){
			berror_found = true;
			goto EndFunc;
		}
	}

EndFunc:;

	if( pbError ){
		*pbError = berror_found;
	}

	if( (berror_found == false || UC_LOOSE == (nOption & UC_LOOSE)) && (pr < pr_end && bminus_found == true) ){
		// '-' をスキップ。
		*ppNextChar = const_cast<char*>(pr) + 1;
	}else{
		*ppNextChar = const_cast<char*>(pr);

		if( (UC_LOOSE != (nOption & UC_LOOSE)) && bminus_found == false ){
			// 2015.03.05 Moca エンコードチェック時に終端の'-'がない場合はポイントを加算しない
			if( pr < pr_end ){
				// バッファの終端の場合を除く
				if( pbNoAddPoint ){
					*pbNoAddPoint = true;
				}
			}
		}
	}

	return nchecklen;
}
