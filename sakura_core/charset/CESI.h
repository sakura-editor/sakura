#pragma once

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

class CESI;
class CMemory;

#include "global.h"

#include "mem/CNativeT.h"


struct tagEncodingInfo {
	ECodeType eCodeID;  // 文字コード識別番号
	//int nEval1;    // 評価値１
	//int nEval2;    // 評価値２
	int nSpecific;
	int nPoints;
};
typedef struct tagEncodingInfo  MBCODE_INFO, WCCODE_INFO;

/*
	○　評価値の使い方　○

	SJIS, JIS, EUCJP, UTF-8, UTF-7 の場合：

	typedef名 MBCODE_INFO
	評価値１ → 特有バイト数
	評価値２ → ポイント数（特有バイト数 － 不正バイト数）

	UTF-16 UTF-16BE の場合：

	typedef名 WCCODE_INFO
	評価値１ → ワイド文字の改行の個数
	評価値２ → 不正バイト数
*/

static const DWORD ESI_NOINFORMATION		= 0;
static const DWORD ESI_MBC_DETECTED			= 1;
static const DWORD ESI_MBC_HANKATACOUNTED	= 2;
static const DWORD ESI_WC_DETECTED			= 4;
static const DWORD ESI_NODETECTED			= 8;
static const DWORD ESI_MBC_CESU8DETECTED	= 16;

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
*/

class CESI {
public:

	virtual ~CESI() { ; }
	CESI() {
		m_dwStatus = ESI_NOINFORMATION;
		m_nTargetDataLen = -1;
	}
	// 調査結果の情報を格納
	bool SetInformation( const char*, const int, ECodeType eFavoriteCodeType );

protected:

	// 添え字に使われる優先順位表を作成
	void InitPriorityTable( const ECodeType );
	// ECodeType で指定された文字コードが優先されるようにする
	void MakePriorityTable( const ECodeType );

	//	**** 全般
	// マルチバイト系とUNICODE系とでそれぞれ情報の格納先が違う。
	// 以下の関数で吸収する
	int GetIndexById( const ECodeType ) const; //!< 文字コードID から情報格納先インデックスを生成

	// データセッタ/ゲッター
	void SetEvaluation( const ECodeType, const int, const int );
	void GetEvaluation( const ECodeType, int *, int * ) const;

	// 優先されている文字コードを取得
	ECodeType GetFavoriteCode( void ) const;

	// 調査対象となったデータの長さ（8bit 単位）
	int m_nTargetDataLen;

	// 判定結果を格納するもの
	unsigned int m_dwStatus;

public:

	// m_dwStatus のセッター／ゲッター
	void SetStatus( DWORD inf ){ m_dwStatus |= inf; }
	DWORD GetStatus(  ) const { return m_dwStatus; }

	// m_nTargetDataLen のセッター／ゲッター
	void SetDataLen( const int n ){ m_nTargetDataLen = n; }
	int GetDataLen( void ) const { return m_nTargetDataLen; }

protected:
	/*
		文字列の文字コード情報を収集する
	*/
	void ScanMBCode( const char *, const int );
	void ScanUnicode( const char *, const int );

	void GetEncodingInfo_sjis( const char *, const int );
	void GetEncodingInfo_jis( const char *, const int );
	void GetEncodingInfo_eucjp( const char *, const int );
	void GetEncodingInfo_utf8( const char *, const int );
	void GetEncodingInfo_utf7( const char *, const int );
	void GetEncodingInfo_uni( const char *, const int );

	bool _CheckUtf16Eol( const char* pS, const int nLen, const bool bbig_endian );
	inline bool _CheckUtf16EolLE( const char* p, const int n ){ return _CheckUtf16Eol( p, n, false ); }
	inline bool _CheckUtf16EolBE( const char* p, const int n ){ return _CheckUtf16Eol( p, n, true ); }

public:
	//
	//	**** マルチバイト判定関係の変数その他
	//
	static const int NUM_OF_MBCODE = (CODE_CODEMAX - 2);
	MBCODE_INFO m_aMbcInfo[NUM_OF_MBCODE];   //!< SJIS, JIS, EUCJP, UTF8, UTF7 情報（優先度に従って格納される）
	MBCODE_INFO* m_apMbcInfo[NUM_OF_MBCODE]; //!< 評価順にソートされた SJIS, JIS, EUCJP, UTF8, UTF7 の情報
	int m_aMbcPriority[CODE_CODEMAX];        //!< 実行時に使用される優先順位表
	int m_nMbcSjisHankata;                   //!< SJIS 半角カタカナのバイト数
	bool m_bMbcCesu8Enabled;                 //!< UTF-8 の検査において、CESU-8 が検出されたかどうか

	/*
		// デフォルトの優先順位
		const int gm_pMbDefaultPriority[] =
		{
			0,			//CODE_SJIS
			1,			//CODE_JIS
			2,			//CODE_EUC
			INT_MAX,	//CODE_UNICODE
			3,			//CODE_UTF8
			4,			//CODE_UTF7
			INT_MAX,	//CODE_UNICODEBE
		};
	*/

	// マルチバイト系の捜査結果を、ポイントが大きい順にソート。
	// ソートした結果は、m_apMbcInfo に格納
	void SortMBCInfo( void );

public:
	//
	// 	**** UTF-16 判定関係の変数その他
	//
	WCCODE_INFO m_aWcInfo[ESI_WCIDX_MAX];  //!< UTF-16 LE/BE 情報
	EBOMType m_eWcBomType;          //!< m_pWcInfo から推測される BOM の種類

	EBOMType GetBOMType(void) const { return m_eWcBomType; }

protected:
	// BOMの種類を推測
	EBOMType GuessUtf16Bom( void ) const;


#ifdef _DEBUG
public:
	static void GetDebugInfo( const char*, const int, CNativeT* );
#endif
};
