/*!	@file
	@brief 文字コードを調査する時に使うインターフェースクラス

	@author Sakura-Editor collaborators
	@date 2006/12/10 新規作成
	@date 2007/10/26 クラスの説明変更 (旧：文字コード調査情報保持クラス)
*/
/*
	Copyright (C) 2006
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

#include <limits.h>
#include <stdio.h>
#include "charset/codechecker.h"
#include "charset/CESI.h"
#include "charset/CCodePage.h"
#include "charset/CCodeMediator.h"
#include "charset/CEuc.h"
#include "charset/codeutil.h"

// 非依存推奨
#include "window/CEditWnd.h"
#include "env/CShareData.h"


/*!
	マルチバイト文字コードの優先順位表（既定値）

	@note
	@li 数が少ないほど優先度は高い。
	@li 各項目の値は、0 以上 CODE_MAX 未満で有効とする。

	CODE_UNICODE と CODE_UNICODEBE を除く各項目の値は、
	対応する文字コード情報の格納先 (m_pMbInfo) インデックスとして使っているため、
	連続させている。

	CODE_SJIS と CODE_UTF8 が同ポイントの第一候補になる可能性があるため、
	CODE_UTF8, CODE_CESU8 を優先的にしている。つまり、SJIS と UTF-8系(UTF-8 と CESU-8) が
	同ポイントになった場合は、必ず UTF=8系 を優先させるようにする。
*/
static const int gm_aMbcPriority[] =
{
	2,			//CODE_SJIS
	4,			//CODE_JIS
	3,			//CODE_EUC
	INT_MAX,	//CODE_UNICODE
	0,			//CODE_UTF8
	5,			//CODE_UTF7
	INT_MAX,	//CODE_UNICODEBE
	1,			//CODE_CESU8
	6,			//CODE_LATIN1
};








/*!
	デフォルトコンストラクタ
*/
void CESI::SetInformation( const char *pS, const int nLen )
{
	// 文字情報を収集
	ScanCode( pS, nLen );
	return;
}



/*!
	文字コード ID から情報格納配列 m_pMbInfo または m_pWcInfo の添え字を取得

	@return
	　nCodeType が CODE_UNICODE, CODE_UNICODEBE の場合は m_pWcInfo 用の添え字が，
	それ以外の場合は m_pMbInfo 用の添え字が返却される。
*/
int CESI::GetIndexById( const ECodeType eCodeType ) const
{
	int nret;
	if( CODE_UNICODE == eCodeType ){
		nret = 0;
	}else if( CODE_UNICODEBE == eCodeType ){
		nret = 1;
	}else{
		nret = gm_aMbcPriority[eCodeType]; // 優先順位表の優先度数をそのまま m_aMbcInfo の添え字として使う。
	}
	return nret;
}

/*!
	収集した評価値を格納

	@return
	　nCodeType が CODE_UNICODE, CODE_UNICODEBE の場合は m_pWcInfo へ，
	それ以外の場合は m_pMbInfo へ値が格納されることに注意。
*/
void CESI::SetEvaluation( const ECodeType eCodeId, const int v1, const int v2 )
{
	int nidx;
	struct tagEncodingInfo *pcEI;

	nidx = GetIndexById( eCodeId );
	if( eCodeId == CODE_UNICODE || eCodeId == CODE_UNICODEBE ){
		pcEI = &m_aWcInfo[nidx];
	}else{
		pcEI = &m_aMbcInfo[nidx];
	}
	pcEI->eCodeID = eCodeId;
	pcEI->nSpecific = v1;
	pcEI->nPoints = v2;

	return;
}


/*!
	収集した評価値を取得

	@return
	　nCodeType が CODE_UNICODE, CODE_UNICODEBE の場合は m_pWcInfo から，
	それ以外の場合は m_pMbInfo から値が取得されることに注意。
*/
void CESI::GetEvaluation( const ECodeType eCodeId, int *pv1, int *pv2 ) const
{
	int nidx;
	const struct tagEncodingInfo *pcEI;

	nidx = GetIndexById( eCodeId );
	if( eCodeId == CODE_UNICODE || eCodeId == CODE_UNICODEBE ){
		pcEI = &m_aWcInfo[nidx];
	}else{
		pcEI = &m_aMbcInfo[nidx];
	}
	*pv1 = pcEI->nSpecific;
	*pv2 = pcEI->nPoints;

	return;
}






/*!
	配列 m_pMbInfo の要素へのポインタを評価順にソートして m_ppMbInfo にセット

	m_pMbInfo に格納される文字コード情報の元々の順番は、m_pMbPriority[] テーブルの添え字に従う。
	バブルソートは、元あった順序を比較的変更しない。
*/
void CESI::SortMBCInfo( void )
{
	MBCODE_INFO *pei_tmp;
	int i, j;

	/*
		「特有バイト数 － 不正バイト数＝ポイント数 (.nPoints)」の数の大きい順にソート（バブルソート）
	*/
	for( i = 0; i < NUM_OF_MBCODE; i++ ){
		m_apMbcInfo[i] = &m_aMbcInfo[i];
	}
	for( i = 1; i < NUM_OF_MBCODE; i++ ){
		for( j = 0; j < NUM_OF_MBCODE - i; j++ ){
			if( m_apMbcInfo[j]->nPoints < m_apMbcInfo[j+1]->nPoints ){
				pei_tmp = m_apMbcInfo[j+1];
				m_apMbcInfo[j+1] = m_apMbcInfo[j];
				m_apMbcInfo[j] = pei_tmp;
			}
		}
	}
}









/*!
	SJIS の文字コード判定情報を収集する
*/
void CESI::GetEncodingInfo_sjis( const char* pS, const int nLen )
{
	const char *pr, *pr_end;
	int nillbytes, num_of_sjis_encoded_bytes, num_of_sjis_hankata;
	int nret;
	ECharSet echarset;

	if( nLen < 1 || pS == NULL ){
		SetEvaluation( CODE_SJIS, 0, 0 );
		m_nMbcSjisHankata = 0;
		return;
	}

	nillbytes = 0;
	num_of_sjis_encoded_bytes = 0;
	num_of_sjis_hankata = 0;
	pr = pS;
	pr_end = pr + nLen;

	for( ; 0 != (nret = CheckSjisChar(pr, pr_end-pr, &echarset)); pr += nret ){
		if( echarset != CHARSET_BINARY ){
			if( echarset == CHARSET_JIS_ZENKAKU ){
				num_of_sjis_encoded_bytes += nret;
				unsigned short sDst[4];
				if( 0 == MyMultiByteToWideChar_JP(reinterpret_cast<const unsigned char*>(pr), nret, sDst) ){
					nillbytes += nret;
				}
			}
			if( echarset == CHARSET_JIS_HANKATA ){
				num_of_sjis_encoded_bytes += nret;
				num_of_sjis_hankata++;
			}
		}else{
			if( pr_end - pr < GuessSjisCharsz(pr[0]) ){ break; }
			nillbytes += nret;
		}
	}

	SetEvaluation( CODE_SJIS,
		num_of_sjis_encoded_bytes,
		num_of_sjis_encoded_bytes - nillbytes );

	m_nMbcSjisHankata = num_of_sjis_hankata;

	return;
}



/*!
	JIS の文字コード判定情報を収集する
*/
void CESI::GetEncodingInfo_jis( const char* pS, const int nLen )
{
	const char *pr, *pr_end;
	const char *pr_next;
	int nescbytes, nillbytes;
	int nlen, nerror;
	EMyJisEscseq emyjisesc;

	if( nLen < 1 || pS == NULL ){
		SetEvaluation( CODE_JIS, 0, 0 );
		return;
	}

	nescbytes = 0;
	nillbytes = 0;
	pr = pS;
	pr_end = pS + nLen;
	emyjisesc = MYJISESC_ASCII7;

	do{
		switch( emyjisesc ){
		case MYJISESC_ASCII7:
			nlen = CheckJisAscii7Part( pr, pr_end-pr, &pr_next, &emyjisesc, &nerror );
			break;
		case MYJISESC_HANKATA:
			nlen = CheckJisHankataPart( pr, pr_end-pr, &pr_next, &emyjisesc, &nerror );
			break;
		case MYJISESC_ZENKAKU:
			nlen = CheckJisZenkakuPart( pr, pr_end-pr, &pr_next, &emyjisesc, &nerror );
			break;
		//case MYJISESC_UNKNOWN:
		default:
			nlen = CheckJisUnknownPart( pr, pr_end-pr, &pr_next, &emyjisesc, &nerror );
		}
		nescbytes += pr_next-(pr+nlen);
		nillbytes += nerror;
		pr = pr_next;
	}while( pr_next < pr_end );

	if( nillbytes ){
		SetEvaluation( CODE_JIS, 0, INT_MIN );
	}else{
		SetEvaluation( CODE_JIS, nescbytes, nescbytes );
	}

	return;
}



/*!
	EUC-JP の文字コード判定情報を収集する
*/
void CESI::GetEncodingInfo_eucjp( const char* pS, const int nLen )
{
	const char *pr, *pr_end;
	int nillbytes, num_of_eucjp_encoded_bytes;
	int num_of_euc_zen_hirakata, num_of_euc_zen;
	int nret;
	ECharSet echarset;

	if( nLen < 1 || pS == NULL ){
		SetEvaluation( CODE_EUC, 0, 0 );
		m_nMbcEucZenHirakata = 0;
		m_nMbcEucZen = 0;
		return;
	}

	nillbytes = 0;
	num_of_eucjp_encoded_bytes = 0;
	pr = pS;
	pr_end = pr + nLen;

	num_of_euc_zen_hirakata = 0;
	num_of_euc_zen = 0;

	for( ; 0 != (nret = CheckEucjpChar(pr, pr_end-pr, &echarset)); pr += nret ){
		if( echarset != CHARSET_BINARY ){
			if( 1 < nret ){
				num_of_eucjp_encoded_bytes += nret;
				num_of_euc_zen += nret;
				if( IsEucZen_hirakata(pr) ){
					num_of_euc_zen_hirakata += 2;
				}else{
					bool bRet;
					unsigned short wc[4];
					CEuc::_EucjpToUni_char(reinterpret_cast<const unsigned char*>(pr), wc, echarset, NULL, &bRet);
					if( bRet ){
						nillbytes += nret;
					}
				}
			}
		}else{
			if( pr_end - pr < GuessEucjpCharsz(pr[0]) ){ break; }
			nillbytes += nret;
		}
	}

	SetEvaluation( CODE_EUC,
		num_of_eucjp_encoded_bytes,
		num_of_eucjp_encoded_bytes - nillbytes );

	m_nMbcEucZen = num_of_euc_zen;
	m_nMbcEucZenHirakata = num_of_euc_zen_hirakata;

	return;
}




/*!
	UTF-7 の文字コード判定情報を収集する

	@note
	　1 バイト以上のエラー（いわゆる ill-formed）が見つかれば UTF-7 と判定されない。
	@date 2015.03.05 Moca 「+G10)」のような、-がない場合ポイントに加算しないようにする
*/
void CESI::GetEncodingInfo_utf7( const char* pS, const int nLen )
{
	const char *pr, *pr_end;
	char *pr_next;
	int npoints, nlen_setb;
	bool berror;

	if( nLen < 1 || pS == NULL ){
		SetEvaluation( CODE_UTF7, 0, 0 );
		return;
	}

	npoints = 0;
	berror = false;
	pr = pS;
	pr_end = pS + nLen;


	do{ // 検査ループ --------------------------------------------------

		/* セットD/O 文字列の検査 */
		CheckUtf7DPart( pr, pr_end-pr, &pr_next, &berror );
		if( berror || pr_next == pr_end ){
			// エラーが出た、または、データの検査が終了した場合、検査終了。
			break;
		}

		pr = pr_next;

		/* セットB 文字列の検査 */
		bool bNoAddPoint = false;
		nlen_setb = CheckUtf7BPart( pr, pr_end-pr, &pr_next, &berror, 0, &bNoAddPoint );
		if( pr+nlen_setb == pr_next && pr_next == pr_end ){
			// セットＢ文字列の終端文字 '-' が無い、かつ、
			// 次の読み込み位置が調査データの終端文字上にある場合、エラーを取り消して検査終了
			berror = false;
			break;
		}
		if( berror ){
			// 一つ以上のエラーが見つかれば、検査終了。
			break;
		}

		pr = pr_next;
		if( !bNoAddPoint ){
			npoints += nlen_setb; // セットＢの文字列の長さをポイントとして加算する。
		}
	}while( pr_next < pr_end );  // 検査ループ終了  --------------------


	if( berror ){
		// エラーが発見された場合、ポイントをマイナス値にしておく。
		npoints = INT_MIN;
	}

	if( npoints < 0 ){
		SetEvaluation( CODE_UTF7, 0, npoints );
	}else{
		SetEvaluation( CODE_UTF7, npoints, npoints );
		// UTF-7 特有の文字列は、セットＢの文字列の長さとする。
	}

	return;
}



/*!
	UTF-8 の文字コード判定情報を収集する
*/
void CESI::GetEncodingInfo_utf8( const char* pS, const int nLen )
{
	const char *pr, *pr_end;
	int nillbytes, num_of_utf8_encoded_bytes;
	int nret;
	ECharSet echarset;

	if( nLen < 1 || pS == NULL ){
		SetEvaluation( CODE_UTF8, 0, 0 );
		return;
	}

	nillbytes = 0;
	num_of_utf8_encoded_bytes = 0;
	pr = pS;
	pr_end = pS + nLen;

	for( ; 0 != (nret = CheckUtf8Char(pr, pr_end-pr, &echarset, true, 0)); pr += nret ){
		if( echarset != CHARSET_BINARY ){
			if( 1 < nret ){
				num_of_utf8_encoded_bytes += nret;
			}
		}else{
			if( pr_end - pr < GuessUtf8Charsz(pr[0]) ){ break; }
			nillbytes += nret;
		}
	}

	SetEvaluation( CODE_UTF8,
		num_of_utf8_encoded_bytes,
		num_of_utf8_encoded_bytes - nillbytes );

	return;
}



/*!
	CESU-8 の文字コード判定情報を収集する
*/
void CESI::GetEncodingInfo_cesu8( const char* pS, const int nLen )
{
	const char *pr, *pr_end;
	int nillbytes, num_of_cesu8_encoded_bytes;
	int nret;
	ECharSet echarset;

	if( nLen < 1 || pS == NULL ){
		SetEvaluation( CODE_CESU8, 0, 0 );
		return;
	}

	nillbytes = 0;
	num_of_cesu8_encoded_bytes = 0;
	pr = pS;
	pr_end = pS + nLen;

	for( ; 0 != (nret = CheckCesu8Char(pr, pr_end-pr, &echarset, 0)); pr += nret ){
		if( echarset != CHARSET_BINARY ){
			if( 1 < nret ){
				num_of_cesu8_encoded_bytes += nret;
			}
		}else{
			if( pr_end - pr < GuessCesu8Charsz(pr[0]) ){ break; }
			nillbytes += nret;
		}
	}

	SetEvaluation( CODE_CESU8,
		num_of_cesu8_encoded_bytes,
		num_of_cesu8_encoded_bytes - nillbytes );

	return;
}



/*!
	Latin1(欧文, Windows-1252)の文字コード判定情報を収集する

	@note
	　必ずFalse。
*/
void CESI::GetEncodingInfo_latin1( const char* pS, const int nLen )
{
	SetEvaluation( CODE_LATIN1, 0, - nLen );
	return;
}




void CESI::GetEncodingInfo_meta( const char* pS, const int nLen )
{
	// XML宣言は先頭にあるので、最初にチェック
	ECodeType encoding = AutoDetectByXML( pS, nLen );
	if( encoding == CODE_NONE ){
		// スクリプト等Coding中にHTMLがあるのでCodingを優先
		encoding = AutoDetectByCoding( pS, nLen );
	}
	if( encoding == CODE_NONE ){
		encoding = AutoDetectByHTML( pS, nLen );
	}
	m_eMetaName = encoding;
}



/*!
	UTF-16 チェッカ内で使う改行コード確認関数
*/
bool CESI::_CheckUtf16Eol( const char* pS, const int nLen, const bool bbig_endian )
{
	wchar_t wc0;
	wchar_t wc1;

	if( nLen >= 4 ){
		if( bbig_endian ){
			wc0 = (pS[0] << 8) | pS[1];
			wc1 = (pS[2] << 8) | pS[3];
		}else{
			wc0 = pS[0] | (pS[1] << 8);
			wc1 = pS[2] | (pS[3] << 8);
		}
		if( (wc0 == 0x000d && wc1 == 0x000a) || wc0 == 0x000d || wc0 == 0x000a ){
			return true;
		}
	}else if( nLen >= 2 ){
		if( bbig_endian ){
			wc0 = (pS[0] << 8) | pS[1];
		}else{
			wc0 = pS[0] | (pS[1] << 8);
		}
		if( wc0 == 0x000d || wc0 == 0x000a ){
			return true;
		}
	}
	return false;

}



/*!
	UTF-16 LE/BE の文字コード判定情報を収集する

	@note 改行文字を数える。ついでに ASCII 版の改行コードも数える。
	　nLen は２で割り切れるものが入ってくることを仮定。
*/
void CESI::GetEncodingInfo_uni( const char* pS, const int nLen )
{
	const char *pr1, *pr2, *pr_end;
	int nillbytes1, nillbytes2;
	int nnewlinew1, nnewlinew2;
	register int nret1, nret2;
	ECharSet echarset1, echarset2;

	if( nLen < 1 || pS == NULL ){
		SetEvaluation( CODE_UNICODE, 0, 0 );
		SetEvaluation( CODE_UNICODEBE, 0, 0 );
		return;
	}

	if( nLen % 2 == 1 ){
		// 2 で割り切れない長さのデータだった場合は、候補から外す。
		SetEvaluation( CODE_UNICODE, 0, INT_MIN );
		SetEvaluation( CODE_UNICODEBE, 0, INT_MIN );
		return;
	}

	nillbytes1 = nillbytes2 = 0;
	nnewlinew1 = nnewlinew2 = 0;
	pr1 = pr2 = pS;
	pr_end = pS + nLen;

	for( ; ; ){

		nret1 = CheckUtf16leChar( reinterpret_cast<const wchar_t*>(pr1), (pr_end - pr1)/sizeof(wchar_t), &echarset1, 0 );
		nret2 = CheckUtf16beChar( reinterpret_cast<const wchar_t*>(pr2), (pr_end - pr2)/sizeof(wchar_t), &echarset2, 0 );
		if( nret1 == 0 && nret2 == 0 ){
			// LE BE 両方共のチェックが終了した。
			break;
		}

		//
		// (pr_end - pr1) と (pr_end - pr2) は常に sizeof(wchar_t) で割り切れること。
		//

		// UTF-16 LE の処理
		if( nret1 != 0 ){
			if( echarset1 != CHARSET_BINARY ){
				if( nret1 == 1 ){
					// UTF-16 LE 版の改行コードをカウント
					if( _CheckUtf16EolLE(pr1, pr_end-pr1) ){ nnewlinew1++; }
				}
			}else{
				unsigned int n = GuessUtf16Charsz( pr1[0] | static_cast<wchar_t>(pr1[1] << 8) );
				if( (pr_end-pr1)/sizeof(wchar_t) < n ){
					break;
				}
				nillbytes1 += nret1*sizeof(wchar_t);
			}
			pr1 += nret1*sizeof(wchar_t);
		}

		// UTF-16 BE の処理
		if( nret2 != 0 ){
			if( echarset2 != CHARSET_BINARY ){
				if( nret2 == 1 ){
					// UTF-16 BE 版の改行コードをカウント
					if( _CheckUtf16EolBE(pr2, pr_end-pr2) ){ nnewlinew2++; }
				}
			}else{
				unsigned int n = GuessUtf16Charsz((static_cast<wchar_t>(pr2[0] << 8)) | pr2[1]);
				if( (pr_end-pr2)/sizeof(wchar_t) < n ){
					break;
				}
				nillbytes2 += nret2*sizeof(wchar_t);
			}
			pr2 += nret2*sizeof(wchar_t);
		}
	}

	if( nillbytes1 < 1 ){
		SetEvaluation( CODE_UNICODE, nnewlinew1, nnewlinew1 );
	}else{
		SetEvaluation( CODE_UNICODE, 0, INT_MIN );
	}
	if( nillbytes2 < 1 ){
		SetEvaluation( CODE_UNICODEBE, nnewlinew2, nnewlinew2 );
	}else{
		SetEvaluation( CODE_UNICODEBE, 0, INT_MIN );
	}

	return;
}




/*!
	テキストの文字コードを収集して整理する．

	@return 入力データがない時に false
*/
void CESI::ScanCode( const char* pS, const int nLen )
{
	// 対象となったデータ長を記録。
	SetDataLen( nLen );

	// データを調査
	GetEncodingInfo_sjis( pS, nLen );
	GetEncodingInfo_jis( pS, nLen );
	GetEncodingInfo_eucjp( pS, nLen );
	GetEncodingInfo_utf8( pS, nLen );
	GetEncodingInfo_utf7( pS, nLen );
	GetEncodingInfo_cesu8( pS, nLen );
	GetEncodingInfo_latin1( pS, nLen );
	GetEncodingInfo_uni( pS, nLen );
	SortMBCInfo();

	GuessEucOrSjis();  // EUC か SJIS かを判定
	GuessUtf8OrCesu8(); // UTF-8 か CESU-8 かを判定

	GuessUtf16Bom();   // UTF-16 の BOM を判定

	GetEncodingInfo_meta( pS, nLen );
}




/*!
	UTF-16 の BOM の種類を推測

	@retval CESI_BOMTYPE_UTF16LE   Little-Endian(LE)
	@retval CESI_BOMTYPE_UTF16BE   Big-Endian(BE)
	@retval CESI_BOMTYPE_UNKNOWN   不明
*/
void CESI::GuessUtf16Bom( void )
{
	int i, j;
	EBOMType ebom_type;

	i = m_aWcInfo[ESI_WCIDX_UTF16LE].nSpecific;  // UTF-16 LE の改行の個数
	j = m_aWcInfo[ESI_WCIDX_UTF16BE].nSpecific;  // UTF-16 BE の改行の個数
	ebom_type = ESI_BOMTYPE_UNKNOWN;
	if( i > j && j < 1 ){
		ebom_type = ESI_BOMTYPE_LE;   // LE
	}else if( i < j && i < 1 ){
		ebom_type = ESI_BOMTYPE_BE;   // BE
	}
	if( ebom_type != ESI_BOMTYPE_UNKNOWN ){
		if( m_aWcInfo[ebom_type].nSpecific - m_aWcInfo[ebom_type].nPoints > 0 ){
			// 不正バイトが検出されている場合は、BOM タイプ不明とする。
			ebom_type = ESI_BOMTYPE_UNKNOWN;
		}
	}

	m_eWcBomType = ebom_type;
}




/*!
	SJIS と EUC の紛らわしさを解消する

	m_bEucFlag が TRUE のとき EUC, FALSE のとき SJIS
*/
void CESI::GuessEucOrSjis( void )
{
	if( IsAmbiguousEucAndSjis()
	 && static_cast<double>(m_nMbcEucZenHirakata) / m_nMbcEucZen >= 0.25 ){ // 0.25 という値は適当です。
		// EUC をトップに持ってくる
		int i;
		for( i = 0; i < 2; ++i ){
			if( m_apMbcInfo[i]->eCodeID == CODE_EUC ){
				break;
			}
		}
		if( i == 1 ){
			MBCODE_INFO* ptemp;
			ptemp = m_apMbcInfo[0];
			m_apMbcInfo[0] = m_apMbcInfo[1];
			m_apMbcInfo[1] = ptemp;
		}
	}
}


/*!
	UTF-8 と CESU-8 の紛らわしさを解消する

	m_bCesu8Flag が TRUE のとき CESU-8, FALSE のとき UTF-8
*/
void CESI::GuessUtf8OrCesu8( void )
{
	if( IsAmbiguousUtf8AndCesu8()
	 && m_pEncodingConfig->m_bPriorCesu8 ){
		int i;
		for( i = 0; i < 2; ++i ){
			if( m_apMbcInfo[i]->eCodeID == CODE_CESU8 ){
				break;
			}
		}
		if( i == 1 ){
			MBCODE_INFO* ptemp;
			ptemp = m_apMbcInfo[0];
			m_apMbcInfo[0] = m_apMbcInfo[1];
			m_apMbcInfo[1] = ptemp;
		}
	}
}



static const struct{
	const char* name;
	int nLen;
	int nCode;
} encodingNameToCode[] = {
	{ "windows-31j",  11, CODE_SJIS },
	{ "x-sjis",        6, CODE_SJIS },
	{ "shift_jis",     9, CODE_SJIS },
	{ "cp932",         9, CODE_SJIS },
	{ "iso-2022-jp",  11, CODE_JIS },
	{ "euc-jp",        6, CODE_EUC },
//	{ "utf-7",         5, CODE_UTF7 },
	{ "utf-8",         5, CODE_UTF8 },
	{ "cesu-8",        6, CODE_CESU8 },
	{ "iso-8859-1",   10, CODE_LATIN1 },
	{ "latin1",        7, CODE_LATIN1 },
	{ "latin-1",       8, CODE_LATIN1 },
	{ "windows-1252", 12, CODE_LATIN1 },
	{"ibm437",             6,   437},
	{"asmo-708",           8,   708},
	{"dos-720",            7,   720},
	{"ibm737",             6,   737},
	{"ibm775",             6,   775},
	{"ibm850",             6,   850},
	{"ibm852",             6,   852},
	{"ibm855",             6,   855},
	{"ibm857",             6,   857},
	{"ibm00858",           8,   858},
	{"ibm860",             6,   860},
	{"ibm861",             6,   861},
	{"dos-862",            7,   862},
	{"ibm863",             6,   863},
	{"ibm864",             6,   864},
	{"ibm865",             6,   865},
	{"cp866",              5,   866},
	{"ibm869",             6,   869},
	{"windows-874",       11,   874},
	{"gb2312",             6,   936},
	{"ks_c_5601-1987",    14,   949},
	{"big5",               4,   950},
	{"ibm1026",            7,  1026},
	{"windows-1250",      12,  1250},
	{"windows-1251",      12,  1251},
	{"windows-1252",      12,  1252},
	{"windows-1253",      12,  1253},
	{"windows-1254",      12,  1254},
	{"windows-1255",      12,  1255},
	{"windows-1256",      12,  1256},
	{"windows-1257",      12,  1257},
	{"windows-1258",      12,  1258},
	{"johab",              5,  1361},
	{"macintosh",          9, 10000},
	{"x-mac-japanese",    14, 10001},
	{"x-mac-chinesetrad", 17, 10002},
	{"x-mac-korean",      12, 10003},
	{"x-mac-arabic",      12, 10004},
	{"x-mac-hebrew",      12, 10005},
	{"x-mac-greek",       11, 10006},
	{"x-mac-cyrillic",    14, 10007},
	{"x-mac-chinesesimp", 17, 10008},
	{"x-mac-romanian",    14, 10010},
	{"x-mac-ukrainian",   15, 10017},
	{"x-mac-thai",        10, 10021},
	{"x-mac-ce",           8, 10029},
	{"x-mac-icelandic",   15, 10079},
	{"x-mac-turkish",     13, 10081},
	{"x-mac-croatian",    14, 10082},
	{"x-chinese-cns",     13, 20000},
	{"x-cp20001",          9, 20001},
	{"x-chinese-eten",    14, 20002},
	{"x-cp20003",          9, 20003},
	{"x-cp20004",          9, 20004},
	{"x-cp20005",          9, 20005},
	{"x-ia5",              5, 20105},
	{"x-ia5-german",      12, 20106},
	{"x-ia5-swedish",     13, 20107},
	{"x-ia5-norwegian",   15, 20108},
	{"x-cp20261",          9, 20261},
	{"x-cp20269",          9, 20269},
	{"koi8-r",             6, 20866},
	{"x-cp20936",          9, 20936},
	{"x-cp20949",          9, 20949},
	{"koi8-u",             6, 21866},
	{"iso-8859-2",        10, 28592},
	{"latin2",             6, 28592} ,
	{"iso-8859-3",        10, 28593},
	{"latin3",             6, 28593},
	{"iso-8859-4",        10, 28594},
	{"latin4",             6, 28594},
	{"iso-8859-5",        10, 28595},
	{"iso-8859-6",        10, 28596},
	{"iso-8859-7",        10, 28597},
	{"iso-8859-8",        10, 28598},
	{"iso-8859-9",        10, 28599},
	{"latin5",             6, 28599},
	{"iso-8859-13",       11, 28603},
	{"iso-8859-15",       11, 28605},
	{"latin-9",            7, 28605},
	{"x-europa",           8, 29001},
	{"iso-8859-8-i",      12, 38598},
	{"iso-2022-kr",       11, 50225},
	{"x-cp50227",          9, 50227},
	{"euc-cn",             6, 51936},
	{"euc-kr",             6, 51949},
	{"hz-gb-2312",        10, 52936},
	{"gb18030",            7, 54936},
	{"x-iscii-de",        10, 57002},
	{"x-iscii-be",        10, 57003},
	{"x-iscii-ta",        10, 57004},
	{"x-iscii-te",        10, 57005},
	{"x-iscii-as",        10, 57006},
	{"x-iscii-or",        10, 57007},
	{"x-iscii-ka",        10, 57008},
	{"x-iscii-ma",        10, 57009},
	{"x-iscii-gu",        10, 57010},
	{"x-iscii-pa",        10, 57011},
	{ NULL, 0, 0}
};



static bool IsXMLWhiteSpace( int c )
{
	if( c == ' '
	 || c == '\t'
	 || c == '\r'
	 || c == '\n'
	){
		return true;
	}
	return false;
}



/*!	ファイル中のエンコーディング指定を利用した文字コード自動選択
 *	@return	決定した文字コード。 未決定は-1を返す
*/
ECodeType CESI::AutoDetectByXML( const char* pBuf, int nSize )
{

	// ASCII comportible encoding XML
	if( 20 < nSize && 0 == memcmp( pBuf, "<?xml", 5 ) ){
		if( !IsXMLWhiteSpace( pBuf[5] ) ){
			return CODE_NONE;
		}
		char quoteXML = '\0';
		int i;
		// xml規格ではencodingはverionに続いて現れる以外は許されない。ここではいいにする
		for( i = 5; i < nSize - 12; i++ ){
			// [ \t\r\n]encoding[ \t\r\n]*=[ \t\r\n]*[\"\']を探す
			if( IsXMLWhiteSpace( pBuf[i] )
			 && 0 == memcmp( pBuf + i + 1, "encoding", 8 ) ){
				i += 9;
				while( i < nSize - 2 ){
					if( !IsXMLWhiteSpace( pBuf[i] ) ) break;
					i++;
				}
				if( pBuf[i] != '=' ){
					break;
				}
				i++;
				while( i < nSize - 1 ){
					if( !IsXMLWhiteSpace( pBuf[i] ) ) break;
					i++;
				}
				char quoteChar;
				if( pBuf[i] != '\'' && pBuf[i] != '\"' ){
					break;
				}
				quoteChar = pBuf[i];
				i++;
				int k;
				for( k = 0; encodingNameToCode[k].name != NULL; k++ ){
					const int nLen = encodingNameToCode[k].nLen;
					if( i + nLen < nSize - 1
					  && pBuf[i + nLen] == quoteChar
					  && 0 == memicmp( encodingNameToCode[k].name, pBuf + i, nLen ) ){
						return static_cast<ECodeType>(encodingNameToCode[k].nCode);
					}
				}
			}else{
				if( pBuf[i] == '<' || pBuf[i] == '>' ){
					break;
				}
				// encoding指定無しでxml宣言が終了した
				if( pBuf[i] == '?' && pBuf[i + 1] == '>' ){
					return CODE_UTF8;
				}
			}
		}
		for( ; i < nSize; i++ ){
			if( pBuf[i] == '<' || pBuf[i] == '>' ){
				break;
			}
			// encoding指定無しでxml宣言が終了した
			if( pBuf[i] == '?' && pBuf[i + 1] == '>' ){
				return CODE_UTF8;
			}
		}
	}else
	// 比較に必要なのは10バイトだが、xml宣言に必要なバイト数以上とする
	if( 20 < nSize && 0 == memcmp( pBuf, "<\0?\x00x\0m\0l\0", 10 ) ){
		if( IsXMLWhiteSpace( pBuf[10] ) && '\0' == pBuf[11] ){
			return CODE_UNICODE;
		}
	}else
	if( 20 < nSize && 0 == memcmp( pBuf, "\0<\0?\x00x\0m\0l", 10 ) ){
		if( '\0' == pBuf[10] && IsXMLWhiteSpace( pBuf[11] ) ){
			return CODE_UNICODEBE;
		}
	}

	return CODE_NONE;
}



ECodeType CESI::AutoDetectByHTML( const char* pBuf, int nSize )
{
	for( int i = 0; i + 14 < nSize; i++ ){
		// 「<meta http-equiv="Content-Type" content="text/html; Charset=Shift_JIS">」
		// 「<meta charset="utf-8">」
		if( pBuf[i] == '<' && 0 == memicmp(&pBuf[i+1], "meta", 4) && IsXMLWhiteSpace(pBuf[i+5]) ){
			i += 5;
			ECodeType encoding = CODE_NONE;
			bool bContentType = false;
			while( i < nSize ){
				if( IsXMLWhiteSpace(pBuf[i]) && i + 1 < nSize && !IsXMLWhiteSpace(pBuf[i+1]) ){
					int nAttType = 0;
					i++;
					if( i + 12 < nSize && 0 == memicmp(pBuf + i, "http-equiv", 10)
						&& (IsXMLWhiteSpace(pBuf[i+10]) || '=' == pBuf[i+10]) ){
						i += 10;
						nAttType = 1; // http-equiv
					}else if( i + 9 < nSize && 0 == memicmp(pBuf + i, "content", 7)
						&& (IsXMLWhiteSpace(pBuf[i+7]) || '=' == pBuf[i+7]) ){
						i += 7;
						nAttType = 2; // content
					}else if( i + 9 < nSize && 0 == memicmp(pBuf + i, "charset", 7)
						&& (IsXMLWhiteSpace(pBuf[i+7]) || '=' == pBuf[i+7]) ){
						i += 7;
						nAttType = 3; // charset
					}else{
						// その他の属性名読み飛ばし
						while( i < nSize && !IsXMLWhiteSpace(pBuf[i]) ){ i++; }
					}
					if( nSize <= i ){ return CODE_NONE; }
					while( IsXMLWhiteSpace(pBuf[i]) && i < nSize ){ i++; }
					if( nSize <= i ){ return CODE_NONE; }
					if( '=' == pBuf[i]){
						i += 1;
						if( nSize <= i ){ return CODE_NONE; }
					}else{
						// [<meta att ...]
						i--;
						continue;
					}
					while( IsXMLWhiteSpace(pBuf[i]) && i < nSize ){ i++; }
					if( nSize <= i ){ return CODE_NONE; }
					char quoteChar = '\0';
					int nBeginAttVal = i;
					int nEndAttVal;
					int nNextPos;
					if( '\'' == pBuf[i] || '"' == pBuf[i] ){
						quoteChar = pBuf[i];
						i++;
						nBeginAttVal = i;
						if( nSize <= i ){ return CODE_NONE; }
						while( i < nSize && quoteChar != pBuf[i] && '<' != pBuf[i] && '>' != pBuf[i] ){ i++; }
						nEndAttVal = i;
						i++;
						nNextPos = i;
					}else{
						while( i < nSize && !IsXMLWhiteSpace(pBuf[i]) && '<' != pBuf[i] && '>' != pBuf[i] ){ i++; }
						nEndAttVal = i;
						nNextPos = i;
					}
					if( 1 == nAttType ){
						// http-equiv
						if( 12 == nEndAttVal - nBeginAttVal && 0 == memicmp(pBuf + nBeginAttVal, "content-type", 12) ){
							bContentType = true;
							if( encoding != CODE_NONE ){
								return encoding;
							}
						}
					}else if( 2 == nAttType ){
						i = nBeginAttVal;
						while( i < nEndAttVal && ';' != pBuf[i] ){ i++; }
						if( nEndAttVal <= i ){ i = nNextPos; continue; }
						i++; // Skip ';'
						while( i < nEndAttVal && IsXMLWhiteSpace(pBuf[i]) ){ i++; }
						if( nEndAttVal <= i ){ i = nNextPos; continue; }
						if( i + 7 < nEndAttVal && 0 == memicmp(pBuf + i, "charset", 7) ){
							i += 7;
							while( i < nEndAttVal && IsXMLWhiteSpace(pBuf[i]) ){ i++; }
							if( nEndAttVal <= i ){ i = nNextPos; continue; }
							if( '=' != pBuf[i] ){ i = nNextPos; continue; }
							i++;
							while( i < nEndAttVal && IsXMLWhiteSpace(pBuf[i]) ){ i++; }
							if( nEndAttVal <= i ){ i = nNextPos; continue; }
							int nCharsetBegin = i;
							while( i < nEndAttVal && !IsXMLWhiteSpace(pBuf[i]) ){ i++; }
							int k;
							for( k = 0; encodingNameToCode[k].name != NULL; k++ ){
								const int nLen = encodingNameToCode[k].nLen;
								if( i - nCharsetBegin == nLen
								  && 0 == memicmp( encodingNameToCode[k].name, pBuf + nCharsetBegin, nLen ) ){
									if( bContentType ){
										return static_cast<ECodeType>(encodingNameToCode[k].nCode);
									}else{
										encoding = static_cast<ECodeType>(encodingNameToCode[k].nCode);
										break;
									}
								}
							}
						}
						i = nNextPos;
					}else if( 3 == nAttType ){
						int k;
						for( k = 0; encodingNameToCode[k].name != NULL; k++ ){
							const int nLen = encodingNameToCode[k].nLen;
							if( nEndAttVal - nBeginAttVal == nLen
							  && 0 == memicmp( encodingNameToCode[k].name, pBuf + nBeginAttVal, nLen ) ){
								return static_cast<ECodeType>(encodingNameToCode[k].nCode);
							}
						}
					}
				}else if( '<' == pBuf[i] ){
					i--;
					break;
				}else if( '>' == pBuf[i] ){
					break;
				}else{
					// 連続したスペース
					i++;
				}
			}
		}
	}
	return CODE_NONE;
}


static bool IsEncodingNameChar( int c )
{
	return ('A' <= c && c <= 'Z')
		|| ('a' <= c && c <= 'z')
		|| '_' == c
		|| '-' == c
	;
}



/* コーディング文字列の識別
「# coding: utf-8」等を取得する
*/
ECodeType CESI::AutoDetectByCoding( const char* pBuf, int nSize )
{
	bool bComment = false;
	int nLineNum = 1;
	for( int i = 0; i + 8 < nSize; i++ ){
		if( bComment && 0 == memcmp(pBuf + i, "coding", 6)
			&& ('=' == pBuf[i+6] ||':' == pBuf[i+6]) ){
			i += 7;
			for(; i < nSize && (' ' == pBuf[i] || '\t' == pBuf[i]); i++ ){}
			if( nSize <= i ){
				return CODE_NONE;
			}
			int nBegin = i;
			for(;i < nSize && IsEncodingNameChar(pBuf[i]); i++ ){}
			if( nBegin == i ){
				return CODE_NONE;
			}
			int k;
			for( k = 0; encodingNameToCode[k].name != NULL; k++ ){
				const int nLen = encodingNameToCode[k].nLen;
				if( i - nBegin == nLen
				  && 0 == memicmp( encodingNameToCode[k].name, pBuf + nBegin, nLen ) ){
					return static_cast<ECodeType>(encodingNameToCode[k].nCode);
				}
			}
		}else if( '\r' == pBuf[i] || '\n' == pBuf[i] ){
			if( '\r' == pBuf[i] && '\n' == pBuf[i+1] ){
				i++;
			}
			nLineNum++;
			bComment = false;
			if( 3 <= nLineNum ){
				break;
			}
		}else if( '#' == pBuf[i] ){
			bComment = true;
		}
	}
	return CODE_NONE;
}


#ifdef _DEBUG


/*!
	収集した情報をダンプする

	@param[out] pcmtxtOut 出力は、このポインタが指すオブジェクトに追加される。
*/
void CESI::GetDebugInfo( const char* pS, const int nLen, CNativeT* pcmtxtOut )
{
	TCHAR szWork[10240];
	int v1, v2, v3, v4;
	int i;

	CEditDoc& doc = *CEditWnd::getInstance()->GetDocument();
	ECodeType ecode_result;
	CESI cesi( doc.m_cDocType.GetDocumentAttribute().m_encoding );

	// テスト実行
	cesi.SetInformation( pS, nLen/*, CODE_SJIS*/ );
	ecode_result = CCodeMediator::CheckKanjiCode( &cesi );

	//
	//	判別結果を分析
	//

	pcmtxtOut->AppendString( LS(STR_ESI_CHARCODE_DETECT) );	// "--文字コード調査結果-----------\r\n"
	pcmtxtOut->AppendString( LS(STR_ESI_RESULT_STATE) );	// "判別結果の状態\r\n"


	if( cesi.m_nTargetDataLen < 1 || cesi.m_dwStatus == ESI_NOINFORMATION ){
		pcmtxtOut->AppendString( LS(STR_ESI_NO_INFO) );	// "\t判別結果を取得できません。\r\n"
		return;
	}
	if( cesi.m_dwStatus != ESI_NODETECTED ){
		// nstat == CESI_MBC_DETECTED or CESI_WC_DETECTED
		pcmtxtOut->AppendString( LS(STR_ESI_DETECTED) );	// "\tおそらく正常に判定されました。\r\n"
	}else{
		pcmtxtOut->AppendString( LS(STR_ESI_NO_DETECTED) );	// "\tコードを検出できませんでした。\r\n"
	}



	pcmtxtOut->AppendString( LS(STR_ESI_DOC_TYPE) );	// "文書種別\r\n"


	auto_sprintf( szWork, _T("\t%s\r\n"), doc.m_cDocType.GetDocumentAttribute().m_szTypeName );
	pcmtxtOut->AppendString( szWork );


	pcmtxtOut->AppendString( LS(STR_ESI_DEFAULT_CHARCODE) );	// "デフォルト文字コード\r\n"

	TCHAR szCpName[100];
	CCodePage::GetNameNormal(szCpName, doc.m_cDocType.GetDocumentAttribute().m_encoding.m_eDefaultCodetype);
	auto_sprintf( szWork, _T("\t%ts\r\n"), szCpName );
	pcmtxtOut->AppendString( szWork );


	pcmtxtOut->AppendString( LS(STR_ESI_SAMPLE_LEN) );	// "サンプルデータ長\r\n"


	auto_sprintf( szWork, LS(STR_ESI_SAMPLE_LEN_FORMAT), cesi.GetDataLen() );	// "\t%d バイト\r\n"
	pcmtxtOut->AppendString( szWork );


	pcmtxtOut->AppendString( LS(STR_ESI_BYTES_AND_POINTS) );	// "固有バイト数とポイント数\r\n"


	pcmtxtOut->AppendString( _T("\tUNICODE\r\n") );
	cesi.GetEvaluation( CODE_UNICODE, &v1, &v2 );
	cesi.GetEvaluation( CODE_UNICODEBE, &v3, &v4 );
	auto_sprintf( szWork, LS(STR_ESI_UTF16LE_B_AND_P), v1, v2 ); // "\t\tUTF16LE 固有バイト数 %d,\tポイント数 %d\r\n"
	pcmtxtOut->AppendString( szWork );
	auto_sprintf( szWork, LS(STR_ESI_UTF16BE_B_AND_P), v3, v4 ); // "\t\tUTF16BE 固有バイト数 %d,\tポイント数 %d\r\n"
	pcmtxtOut->AppendString( szWork );
	pcmtxtOut->AppendString( LS(STR_ESI_BOM) );	// "\t\tBOM の推測結果　"
	switch( cesi.m_eWcBomType ){
	case ESI_BOMTYPE_LE:
		auto_sprintf( szWork, _T("LE\r\n") );
		break;
	case ESI_BOMTYPE_BE:
		auto_sprintf( szWork, _T("BE\r\n") );
		break;
	default:
		auto_sprintf( szWork, LS(STR_ESI_BOM_UNKNOWN) );	// "不明\r\n"
	}
	pcmtxtOut->AppendString( szWork );
	pcmtxtOut->AppendString( LS(STR_ESI_MBC_OTHER_UNICODE) );
	for( i = 0; i < NUM_OF_MBCODE; ++i ){
		if( !IsValidCodeOrCPType(cesi.m_apMbcInfo[i]->eCodeID) ){
			cesi.m_apMbcInfo[i]->eCodeID = CODE_SJIS;
		}
		cesi.GetEvaluation( cesi.m_apMbcInfo[i]->eCodeID, &v1, &v2 );
		auto_sprintf( szWork, LS(STR_ESI_OTHER_B_AND_P),	// "\t\t%d.%ts\t固有バイト数 %d\tポイント数 %d\r\n"
			i+1, CCodeTypeName(cesi.m_apMbcInfo[i]->eCodeID).Normal(), v1, v2 );
		pcmtxtOut->AppendString( szWork );
	}
	auto_sprintf( szWork, LS(STR_ESI_EUC_ZENKAKU), static_cast<double>(cesi.m_nMbcEucZenHirakata)/cesi.m_nMbcEucZen );	// "\t\t・EUC全角カナかな/EUC全角\t%6.3f\r\n"
	pcmtxtOut->AppendString( szWork );


	pcmtxtOut->AppendString( LS(STR_ESI_RESULT) );	// "判定結果\r\n"


	auto_sprintf( szWork, _T("\t%ts\r\n"), CCodeTypeName(ecode_result).Normal() );
	pcmtxtOut->AppendString( szWork );


	return;
}

#endif
