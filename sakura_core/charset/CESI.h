/*!	@file
	@brief 文字コードの判定調査する時に使う情報入れ

	@author Sakura-Editor collaborators
	@date 2006/12/10 新規作成
	@date 2007/10/26 クラスの説明変更 (旧：文字コード調査情報保持クラス)
	@date 2008/19/17 クラスの説明変更（旧：文字コードを調査する時に使うインターフェースクラス）
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
#ifndef SAKURA_CESI_B70CADC4_E43F_40D7_B87C_0C7C14ABDF41_H_
#define SAKURA_CESI_B70CADC4_E43F_40D7_B87C_0C7C14ABDF41_H_

struct SEncodingConfig;

#include "_main/global.h"


struct tagEncodingInfo {
	ECodeType eCodeID;  // 文字コード識別番号
	int nSpecific;	// 評価値1
	int nPoints;	// 評価値2
};
typedef struct tagEncodingInfo  MBCODE_INFO, WCCODE_INFO;

/*
	○　評価値の使い方　○

	SJIS, JIS, EUCJP, UTF-8, UTF-7 の場合：

	typedef名 MBCODE_INFO
	評価値１ → 固有バイト数
	評価値２ → ポイント数（特有バイト数 － 不正バイト数）

	UTF-16 UTF-16BE の場合：

	typedef名 WCCODE_INFO
	評価値１ → ワイド文字の改行の個数
	評価値２ → 不正バイト数
*/

static const DWORD ESI_NOINFORMATION		= 0;
static const DWORD ESI_MBC_DETECTED			= 1;
static const DWORD ESI_WC_DETECTED			= 2;
static const DWORD ESI_NODETECTED			= 4;


// ワイド文字の２種類あるものの格納位置
enum EStoreID4WCInfo {
	ESI_WCIDX_UTF16LE,
	ESI_WCIDX_UTF16BE,
	ESI_WCIDX_MAX,
};
// BOM タイプ
enum EBOMType {
	ESI_BOMTYPE_UNKNOWN = -1,
	ESI_BOMTYPE_LE =0,
	ESI_BOMTYPE_BE =1,
};



/*!
	文字コードを調査する時に生じる情報格納クラス
//*/

class CESI {
public:

	virtual ~CESI() { ; }
	explicit CESI( const SEncodingConfig& ref ) : m_pEncodingConfig(&ref) {
		m_dwStatus = ESI_NOINFORMATION;
		m_nTargetDataLen = -1;
		m_eMetaName = CODE_NONE;
	}

	//! 調査結果の情報を格納
	void SetInformation( const char*, const int );

protected:

	//! 添え字に使われる優先順位表を作成
	void InitPriorityTable( void );

	//	**** 全般
	// マルチバイト系とUNICODE系とでそれぞれ情報の格納先が違う。
	// 以下の関数で吸収する
	int GetIndexById( const ECodeType ) const; //!< 文字コードID から情報格納先インデックスを生成

	// データセッタ/ゲッター
	void SetEvaluation( const ECodeType, const int, const int );
	void GetEvaluation( const ECodeType, int *, int * ) const;

	//! 調査対象となったデータの長さ（8bit 単位）
	int m_nTargetDataLen;

	//! 判定結果を格納するもの
	unsigned int m_dwStatus;

public:

	// m_dwStatus のセッター／ゲッター
	void SetStatus( DWORD inf ){ m_dwStatus |= inf; }
	DWORD GetStatus( void ) const { return m_dwStatus; }

	// m_nTargetDataLen のセッター／ゲッター
protected:
	void SetDataLen( const int n ){ if( n < 1 ){ m_nTargetDataLen = 0; }else{ m_nTargetDataLen = n; } }
public:
	int GetDataLen( void ) const { return m_nTargetDataLen; }

protected:
	/*
		文字列の文字コード情報を収集する
	*/
	void ScanCode( const char *, const int );

	void GetEncodingInfo_sjis( const char *, const int );
	void GetEncodingInfo_jis( const char *, const int );
	void GetEncodingInfo_eucjp( const char *, const int );
	void GetEncodingInfo_utf8( const char *, const int );
	void GetEncodingInfo_utf7( const char *, const int );
	void GetEncodingInfo_cesu8( const char *, const int );
	void GetEncodingInfo_uni( const char *, const int );
	void GetEncodingInfo_latin1( const char *, const int );
	void GetEncodingInfo_meta( const char *, const int );


	bool _CheckUtf16Eol( const char* pS, const int nLen, const bool bbig_endian );
	inline bool _CheckUtf16EolLE( const char* p, const int n ){ return _CheckUtf16Eol( p, n, false ); }
	inline bool _CheckUtf16EolBE( const char* p, const int n ){ return _CheckUtf16Eol( p, n, true ); }

public:
	//
	//	**** マルチバイト判定関係の変数その他
	//
	static const int NUM_OF_MBCODE = (CODE_CODEMAX - 2);
	MBCODE_INFO m_aMbcInfo[NUM_OF_MBCODE];   //!< SJIS, JIS, EUCJP, UTF8, UTF7 情報（優先度に従って格納される）
	MBCODE_INFO* m_apMbcInfo[NUM_OF_MBCODE]; //!< 評価順にソートされた SJIS, JIS, EUCJP, UTF8, UTF7, CESU8 の情報
	int m_nMbcSjisHankata;                   //!< SJIS 半角カタカナのバイト数
	int m_nMbcEucZenHirakata;                //!< EUC 全角ひらがなカタカナのバイト数
	int m_nMbcEucZen;                        //!< EUC 全角のバイト数

	//! マルチバイト系の捜査結果を、ポイントが大きい順にソート。 ソートした結果は、m_apMbcInfo に格納
	void SortMBCInfo( void );

	//! EUC と SJIS が候補のトップ２に上がっているかどうか
	bool IsAmbiguousEucAndSjis( void ){
		// EUC と SJIS がトップ2に上がった時
		// かつ、EUC と SJIS のポイント数が同数のとき
		if( (m_apMbcInfo[0]->eCodeID == CODE_SJIS && m_apMbcInfo[1]->eCodeID == CODE_EUC
		     || m_apMbcInfo[1]->eCodeID == CODE_SJIS && m_apMbcInfo[0]->eCodeID == CODE_EUC)
		 && m_apMbcInfo[0]->nPoints == m_apMbcInfo[1]->nPoints
		){
			return true;
		}
		return false;
	}

	//! SJIS と UTF-8 が候補のトップ2に上がっているかどうか
	bool IsAmbiguousUtf8AndCesu8( void ){
		// UTF-8 と SJIS がトップ2に上がった時
		// かつ、UTF-8 と SJIS のポイント数が同数のとき
		if( (m_apMbcInfo[0]->eCodeID == CODE_UTF8 && m_apMbcInfo[1]->eCodeID == CODE_CESU8
		     || m_apMbcInfo[1]->eCodeID == CODE_UTF8 && m_apMbcInfo[0]->eCodeID == CODE_CESU8)
		 && m_apMbcInfo[0]->nPoints == m_apMbcInfo[1]->nPoints
		){
			return true;
		}
		return false;
	}

protected:
	void GuessEucOrSjis( void );	//!< EUC か SJIS かを判定
	void GuessUtf8OrCesu8( void );	//!< UTF-8 か CESU-8 かを判定
public:
	//
	// 	**** UTF-16 判定関係の変数その他
	//
	WCCODE_INFO m_aWcInfo[ESI_WCIDX_MAX];  //!< UTF-16 LE/BE 情報
	EBOMType m_eWcBomType;          //!< m_pWcInfo から推測される BOM の種類
	ECodeType m_eMetaName;          //!< エンコーディング名からの種類判別

	EBOMType GetBOMType(void) const { return m_eWcBomType; }
	ECodeType GetMetaName() const { return m_eMetaName; }

protected:
	//! BOMの種類を推測して m_eWcBomType を設定
	void GuessUtf16Bom( void );
	ECodeType AutoDetectByXML( const char*, int );
	ECodeType AutoDetectByHTML( const char*, int );
	ECodeType AutoDetectByCoding( const char*, int );


public:
	const SEncodingConfig* m_pEncodingConfig;

#ifdef _DEBUG
public:
	static void GetDebugInfo( const char*, const int, CNativeT* );
#endif
};

#endif /* SAKURA_CESI_B70CADC4_E43F_40D7_B87C_0C7C14ABDF41_H_ */
/*[EOF]*/
