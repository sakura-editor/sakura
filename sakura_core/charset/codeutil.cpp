#include "StdAfx.h"
#include <mbstring.h>
#include "charset/codeutil.h"
#include "charset/charcode.h"
#include "charset/codechecker.h"

#if 0
/*!
	@brief 拡張版 SJIS→JIS変換

	SJISコードをJISに変換する．その際，JISに対応領域のないIBM拡張文字を
	NEC選定IBM拡張文字に変換する．

	Shift_JIS fa40～fc4b の範囲の文字は 8754～879a または ed40～eefc に
	散在する文字に変換された後に，JISに変換されます．
	
	@param pszSrc [in] 変換する文字列へのポインタ (Shift JIS)
	
	@author すい
	@date 2002.10.03 1文字のみ扱い，変換まで行うように変更 genta
*/
unsigned int _mbcjmstojis_ex( unsigned int nSrc, bool* pbNonroundtrip )
{
	unsigned int	tmpw;	/* ← int が 16 bit 以上である事を期待しています。 */
	bool bnonrt = false;
	
	unsigned char c0 = static_cast<unsigned char>((nSrc & 0x0000ff00) >> 8);
	unsigned char c1 = static_cast<unsigned char>(nSrc & 0x000000ff);

	if(	IsSjisZen1( static_cast<char>(c0) )	/* Shift_JIS 全角文字の 1バイト目 */
	 && IsSjisZen2( static_cast<char>(c1) )	/* Shift_JIS 全角文字の 2バイト目 */
	){	/* Shift_JIS全角文字である */
		tmpw = static_cast<unsigned int>(c0 << 8) | static_cast<unsigned int>(c1);
		//tmpw = ( ((unsigned int)*pszSrc) << 8 ) | ( (unsigned int)*(pszSrc + 1) );
		if(
			( c0 == 0x0fa ) ||
			( c0 == 0x0fb ) ||
			( ( c0 == 0x0fc ) && ( c1 <= 0x04b ) )
		) {		/* fa40～fc4b の文字である。 */
			/* 文字コード変換処理 */
			if		  ( tmpw <= 0xfa49 ) {	tmpw -= 0x0b51;	}	/* fa40～fa49 → eeef～eef8 (ⅰ～ⅹ) */
			else	if( tmpw <= 0xfa53 ) {	tmpw -= 0x72f6;	}	/* fa4a～fa53 → 8754～875d (Ⅰ～Ⅹ) */
			else	if( tmpw <= 0xfa57 ) {	tmpw -= 0x0b5b;	}	/* fa54～fa57 → eef9～eefc (￢～＂) */
			else	if( tmpw == 0xfa58 ) {	tmpw  = 0x878a;	}	/* ㈱ */
			else	if( tmpw == 0xfa59 ) {	tmpw  = 0x8782;	}	/* № */
			else	if( tmpw == 0xfa5a ) {	tmpw  = 0x8784;	}	/* ℡ */
			else	if( tmpw == 0xfa5b ) {	tmpw  = 0x879a;	}	/* ∵ */
			else	if( tmpw <= 0xfa7e ) {	tmpw -= 0x0d1c;	}	/* fa5c～fa7e → ed40～ed62 (纊～兊) */
			else	if( tmpw <= 0xfa9b ) {	tmpw -= 0x0d1d;	}	/* fa80～fa9b → ed63～ed7e (兤～﨏) */
			else	if( tmpw <= 0xfafc ) {	tmpw -= 0x0d1c;	}	/* fa9c～fafc → ed80～ede0 (塚～浯) */
			else	if( tmpw <= 0xfb5b ) {	tmpw -= 0x0d5f;	}	/* fb40～fb5b → ede1～edfc (涖～犱) */
			else	if( tmpw <= 0xfb7e ) {	tmpw -= 0x0d1c;	}	/* fb5c～fb7e → ee40～ee62 (犾～神) */
			else	if( tmpw <= 0xfb9b ) {	tmpw -= 0x0d1d;	}	/* fb80～fb9b → ee63～ee7e (祥～蕙) */
			else	if( tmpw <= 0xfbfc ) {	tmpw -= 0x0d1c;	}	/* fb9c～fbfc → ee80～eee0 (蕫～髙) */
			else{							tmpw -= 0x0d5f;	}	/* fc40～fc4b → eee1～eeec (髜～黑) */
		}
		return _mbcjmstojis( tmpw );
	}
	return 0;
}
#endif





/*
	判別テーブル   WinAPI 関数 WideCharToMultiByte の特殊な変換（相互変換できない変換）か
	添え字の定義域：0x00 ～ 0x5f(=0x00ff - 0x00a0)
*/
static const bool bNA = false;
const bool TABLE_WctombSpec[] = {
	bNA,  true, true, true, bNA,  true, true, bNA,   // 00a0 - 00a7
	bNA,  true, true, true, true, true, true, true,  // 00a8 - 00af
	bNA,  bNA,  true, true, bNA,  true, bNA,  true,  // 00b0 - 00b7
	true, true, true, true, bNA,  bNA,  bNA,  bNA,   // 00b8 - 00bf
	true, true, true, true, true, true, true, true,  // 00c0 - 00c7
	true, true, true, true, true, true, true, true,  // 00c8 - 00cf
	true, true, true, true, true, true, true, bNA,   // 00d0 - 00d7
	true, true, true, true, true, true, true, true,  // 00d8 - 00df
	true, true, true, true, true, true, true, true,  // 00e0 - 00e7
	true, true, true, true, true, true, true, true,  // 00e8 - 00ef
	true, true, true, true, true, true, true, bNA,   // 00f0 - 00f7
	true, true, true, true, true, true, true, true,  // 00f8 - 00ff
};



//
//	判別・変換テーブル   SJIS の重複符号化されている文字を解決する補助的な表
//
//	参考資料一覧：
//		Windows-31J の文字セット，森山 将之
//		    http://www2d.biglobe.ne.jp/~msyk/charcode/cp932/Windows-31J-charset.html
//		JIS 基本漢字，Cyber Librarian
//		    http://www.asahi-net.or.jp/~ax2s-kmtn/ref/jisx0208.html
//		拡張文字コード変換マクロ，すい
//		    http://sakura.qp.land.to/?Macro%2F%C5%EA%B9%C6%2F126
//		[PRB] SHIFT - JIS と Unicode 間の変換問題，Microsoft
//		    http://support.microsoft.com/default.aspx?scid=kb;ja;Q170559
//

//
//	2区と13区にある重複文字の既定コードポイントを与える表
//
static const unsigned short TABLE_SjisPoorcodeDef[] = {
	0x81be, // ∪
	0x81bf, // ∩
	0x81ca, // ￢  (0xeef9 から変更)
	0x81da, // ∠
	0x81db, // ⊥
	0x81df, // ≡
	0x81e0, // ≒
	0x81e3, // √
	0x81e6, // ∵  (0x879a から変更)
	0x81e7, // ∫
	0x8754, // Ⅰ
	0x8755, // Ⅱ
	0x8756, // Ⅲ
	0x8757, // Ⅳ
	0x8758, // Ⅴ
	0x8759, // Ⅵ
	0x875a, // Ⅶ
	0x875b, // Ⅷ
	0x875c, // Ⅸ
	0x875d, // Ⅹ
	0x8782, // №
	0x8784, // ℡
	0x878a, // ㈱
};

//
//	2区と13区の重複文字を既定コードポイントへ変換する表
//
static const unsigned  short TABLE_SjisPoorcodeIndex[][2] = {
//  { 重複文字値, 対応する TABLE_SjisPoorcodeDef 内の位置 }
	{ 0x8790,  6 }, // ≒
	{ 0x8791,  5 }, // ≡
	{ 0x8792,  9 }, // ∫
	{ 0x8795,  7 }, // √
	{ 0x8796,  4 }, // ⊥
	{ 0x8797,  3 }, // ∠
	{ 0x879a,  8 }, // ∵
	{ 0x879b,  1 }, // ∩
	{ 0x879c,  0 }, // ∪
	{ 0xeef9,  2 }, // ￢
	{ 0xfa4a, 10 }, // Ⅰ
	{ 0xfa4b, 11 }, // Ⅱ
	{ 0xfa4c, 12 }, // Ⅲ
	{ 0xfa4d, 13 }, // Ⅳ
	{ 0xfa4e, 14 }, // Ⅴ
	{ 0xfa4f, 15 }, // Ⅵ
	{ 0xfa51, 16 }, // Ⅶ
	{ 0xfa52, 17 }, // Ⅷ
	{ 0xfa53, 18 }, // Ⅸ
	{ 0xfa54,  2 }, // ￢
	{ 0xfa54, 19 }, // Ⅹ
	{ 0xfa58, 22 }, // ㈱
	{ 0xfa59, 20 }, // №
	{ 0xfa5a, 21 }, // ℡
	{ 0xfa5b,  8 }, // ∵
};
static const int TABLESIZE_SjisPoorcodeIndex = 23;

//  確認のためのテーブル群と説明

//	typedef struct tag_SjisPoorcodeResolvTable {
//		uint16_t codeDefined;  // 既定コードポイント
//		uint16_t pcodeAlias[2];  // エイリアスコードポイント
//	} SJIS_POORCODE_RESOLV_TABLE;
//
//
//	JIS X 0208 により第2区にて定義された記号について
//
//	2区で定義されている文字に対しては、2区のコードポイントを使う。
//
//	const SJIS_POORCODE_RESOLV_TABLE TABLE_cSPCR_block2[] = {
//		{ 0x81be, { 0x879c, 0      } }, // 00. ∪
//		{ 0x81bf, { 0x879b, 0      } }, // 01. ∩
//		{ 0x81ca, { 0xeef9, 0xfa54 } }, // 02. ￢
//		{ 0x81da, { 0x8797, 0      } }, // 03. ∠
//		{ 0x81db, { 0x8796, 0      } }, // 04. ⊥
//		{ 0x81df, { 0x8791, 0      } }, // 05. ≡
//		{ 0x81e0, { 0x8790, 0      } }, // 06. ≒
//		{ 0x81e3, { 0x8795, 0      } }, // 07. √
//		{ 0x81e6, { 0x879a, 0xfa5b } }, // 08. ∵
//		{ 0x81e7, { 0x8792, 0      } }, // 09. ∫
//	};
//
//
//	NEC の都合により第13区に定義された文字について
//
//	2区で定義済みの文字を除いて、
//	残った文字に対してこの13区（NEC特殊文字）のコードポイントを使う。
//
//	const SJIS_POORCODE_RESOLV_TABLE TABLE_cSPCR_block13 = {
//		{ 0x8754, { 0xfa4a, 0 } }, // 10. Ⅰ
//		{ 0x8755, { 0xfa4b, 0 } }, // 11. Ⅱ
//		{ 0x8756, { 0xfa4c, 0 } }, // 12. Ⅲ
//		{ 0x8757, { 0xfa4d, 0 } }, // 13. Ⅳ
//		{ 0x8758, { 0xfa4e, 0 } }, // 14. Ⅴ
//		{ 0x8759, { 0xfa4f, 0 } }, // 15. Ⅵ
//		{ 0x875a, { 0xfa51, 0 } }, // 16. Ⅶ
//		{ 0x875b, { 0xfa52, 0 } }, // 17. Ⅷ
//		{ 0x875c, { 0xfa53, 0 } }, // 18. Ⅸ
//		{ 0x875d, { 0xfa54, 0 } }, // 19. Ⅹ
//		{ 0x8782, { 0xfa59, 0 } }, // 20. №
//		{ 0x8784, { 0xfa5a, 0 } }, // 21. ℡
//		{ 0x878a, { 0xfa58, 0 } }, // 22. ㈱
//	}
//
//
//
//	IBM の都合により第115区～119区に定義された文字について
//
//	2区と 13区で定義済みの文字を除いて残った文字に対しては、
//	<del>89区から92区にある、それと等価な NEC選定IBM拡張文字コードポイントを使う。</del>
//  115区から 119区にある IBM拡張文字のコードポイントを使う。
//	0xfa5c から 0xfc4b、
//	0xfa40 から 0xfa49、
//	0xfa55 から 0xfa57 の SJIS コードポイントを規定とする。
//	それらのコードポイントのエイリアスは、それぞれ、
//	0xed40 から 0xeeec、
//	0xeeef から 0xeef8、
//	0xeefa から 0xeefc の SJIS コードポイントである。
//




/*!
	SJIS フィルタ  2区と13区にある文字の正常化

	2区で定義されている文字を2区へ移動し、13区で定義されていて2区にはない文字を13区へ移動させる。
*/
unsigned int __fastcall SjisFilter_basis( const unsigned int uCode )
{
	int ni = 0, nleft, nright;
	unsigned int code_tmp;

	/* 二分検索開始 */
	nleft = 0;
	nright = TABLESIZE_SjisPoorcodeIndex -1;
	while( nleft <= nright ){
		ni = (nright + nleft) / 2;
		code_tmp = TABLE_SjisPoorcodeIndex[ni][0];
		if( uCode < code_tmp ){
			nright = ni - 1;
		}else if( code_tmp < uCode ){
			nleft = ni + 1;
		}else{
			break; // 発見
		}
	}

	if( nleft <= nright ){
		return TABLE_SjisPoorcodeDef[ TABLE_SjisPoorcodeIndex[ni][1] ];
	}
	return uCode;
}


/*!
	SJIS フィルタ  IBM拡張文字 → NEC 選定IBM拡張文字

	IBM 拡張文字をその文字と等価な NEC 選定 IBM 拡張領域のコードポイントへ
	可能な限り移動させる。
*/
unsigned int __fastcall SjisFilter_ibm2nec( const unsigned int uCode )
{
	uchar_t c1, c2;
	unsigned int code = (unsigned int) uCode;

	/*
		想定される入力値：

		fa40 ～ fa7e, fa80 ～ fafc
		fb40 ～ fb7e, fb80 ～ fbfc
		fc40 ～ fc4b
	*/
	c1 = static_cast<unsigned char>( (code >> 8) & 0x000000ff );
	c2 = static_cast<unsigned char>( code & 0x000000ff );
	if( c1 == 0xfa || c1 == 0xfb || (c1 == 0xfc && c2 <= 0x4b) ){
		if     ( code <= 0xfa49 ) { code -= 0x0b51; }	/* fa40～fa49 → eeef～eef8 (ⅰ～ⅹ) */
		else if( code <= 0xfa54 ) { ; }
		else if( code <= 0xfa57 ) { code -= 0x0b5b; }	/* fa55～fa57 → eefa～eefc (￤～＂) */
		else if( code <= 0xfa5b ) { ; }
		else if( code <= 0xfa7e ) { code -= 0x0d1c; }	/* fa5c～fa7e → ed40～ed62 (纊～兊) */
		else if( code <= 0xfa9b ) { code -= 0x0d1d; }	/* fa80～fa9b → ed63～ed7e (兤～﨏) */
		else if( code <= 0xfafc ) { code -= 0x0d1c; }	/* fa9c～fafc → ed80～ede0 (塚～浯) */
		else if( code <= 0xfb5b ) { code -= 0x0d5f; }	/* fb40～fb5b → ede1～edfc (涖～犱) */
		else if( code <= 0xfb7e ) { code -= 0x0d1c; }	/* fb5c～fb7e → ee40～ee62 (犾～神) */
		else if( code <= 0xfb9b ) { code -= 0x0d1d; }	/* fb80～fb9b → ee63～ee7e (祥～蕙) */
		else if( code <= 0xfbfc ) { code -= 0x0d1c; }	/* fb9c～fbfc → ee80～eee0 (蕫～髙) */
		else                      { code -= 0x0d5f; }	/* fc40～fc4b → eee1～eeec (髜～黑) */
	}

	return code;
}


/*!
	SJIS フィルタ  NEC 選定IBM拡張文字 → IBM拡張文字

	NEC 選定 IBM 拡張文字をその文字と等価な IBM 拡張文字領域のコードポイントへ
	可能な限り移動させる。
*/
unsigned int __fastcall SjisFilter_nec2ibm( const unsigned int uCode )
{
	uchar_t c1;
	unsigned int code = (unsigned int) uCode;

	/*
		想定された入力値：

		ed40 ～ ed7e, ed80 ～ edfc
		ee40 ～ ee7e, ee80 ～ eefc
	*/
	c1 = static_cast<unsigned char>( (code >> 8) & 0x000000ff );
	if( c1 == 0xed || c1 == 0xee ){
		if     ( code <= 0xed62 ) { code += 0x0d1c; }	/* ed40～ed62 → fa5c～fa7e (纊～兊) */
		else if( code <= 0xed7e ) { code += 0x0d1d; }	/* ed63～ed7e → fa80～fa9b (兤～﨏) */
		else if( code <= 0xede0 ) { code += 0x0d1c; }	/* ed80～ede0 → fa9c～fafc (塚～浯) */
		else if( code <= 0xedfc ) { code += 0x0d5f; }	/* ede1～edfc → fb40～fb5b (涖～犱) */
		else if( code <= 0xee62 ) { code += 0x0d1c; }	/* ee40～ee62 → fb5c～fb7e (犾～神) */
		else if( code <= 0xee7e ) { code += 0x0d1d; }	/* ee63～ee7e → fb80～fb9b (祥～蕙) */
		else if( code <= 0xeee0 ) { code += 0x0d1c; }	/* ee80～eee0 → fb9c～fbf0 (蕫～髙) */
		else if( code <= 0xeeec ) { code += 0x0d5f; }	/* eee1～eeec → fc40～fc4b (髜～黑) */
		else if( code <= 0xeeee ) { ; }
		else if( code <= 0xeef8 ) { code += 0x0b51; }	/* eeef～eef8 → fa40～fa49 (ⅰ～ⅹ) */
		else if( code == 0xeef9 ) { ; }
		else                      { code += 0x0b5b; }	/* eefa～eefc → fa55～fa57 (￤～＂) */
	}

	return code;
}

