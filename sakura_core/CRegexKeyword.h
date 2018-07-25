/*!	@file
	@brief CRegexKeyword Library

	正規表現キーワードを扱う。
	BREGEXP.DLLを利用する。

	@author MIK
	@date Nov. 17, 2001
*/
/*
	Copyright (C) 2001, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

#ifndef	_REGEX_KEYWORD_H_
#define	_REGEX_KEYWORD_H_

#include "_main/global.h"
#include "extmodule/CBregexp.h"
#include "config/maxdata.h" //MAX_REGEX_KEYWORD

struct STypeConfig;

#define USE_PARENT	//親を使ってキーワード格納領域を削減する。




//@@@ 2001.11.17 add start MIK
struct RegexKeywordInfo {
	int	m_nColorIndex;		//色指定番号
};
//@@@ 2001.11.17 add end MIK


//!	正規表現キーワード検索情報構造体
typedef struct RegexInfo_t {
	BREGEXP_W	*pBregexp;	//BREGEXP_W構造体
#ifdef USE_PARENT
#else
	struct RegexKeywordInfo	sRegexKey;	//コンパイルパターンを保持
#endif
	int    nStatus;		//状態(EMPTY,CLOSE,OPEN,ACTIVE,ERROR)
	int    nMatch;		//このキーワードのマッチ状態(EMPTY,MATCH,NOMATCH)
	int    nOffset;		//マッチした位置
	int    nLength;		//マッチした長さ
	int    nHead;		//先頭のみチェックするか？
	int    nFlag;           //色指定のチェックが入っているか？ YES=RK_EMPTY, NO=RK_NOMATCH
} REGEX_INFO;



//!	正規表現キーワードクラス
/*!
	正規表現キーワードを扱う。
*/
class CRegexKeyword : public CBregexp {
public:
	CRegexKeyword(LPCTSTR);
	~CRegexKeyword();

	//! 行検索開始
	BOOL RegexKeyLineStart( void );
	//! 行検索
	BOOL RegexIsKeyword( const CStringRef& cStr, int nPos, int *nMatchLen, int *nMatchColor );
	//! タイプ設定
	BOOL RegexKeySetTypes( const STypeConfig *pTypesPtr );

	//! 書式(囲み)チェック
	static BOOL RegexKeyCheckSyntax( const wchar_t *s );

	static DWORD GetNewMagicNumber();


protected:
	//! コンパイル
	BOOL RegexKeyCompile(void);
	//! 変数初期化
	BOOL RegexKeyInit( void );


public:
	int				m_nTypeIndex;				//!< 現在のタイプ設定番号
	bool			m_bUseRegexKeyword;			//!< 正規表現キーワードを使用する・しない

private:
	const STypeConfig*	m_pTypes;				//!< タイプ設定へのポインタ(呼び出し側が持っているもの)
	int				m_nTypeId;					//!< タイプ設定ID
	DWORD			m_nCompiledMagicNumber;		//!< コンパイル済みか？
	int				m_nRegexKeyCount;			//!< 現在のキーワード数
	REGEX_INFO		m_sInfo[MAX_REGEX_KEYWORD];	//!< キーワード一覧(BREGEXPコンパイル対象)
#ifdef USE_PARENT
#else
	wchar_t			m_keywordList[MAX_REGEX_KEYWORDLISTLEN];
#endif
	wchar_t			m_szMsg[256];				//!< BREGEXP_Wからのメッセージを保持する
};

#endif	//_REGEX_KEYWORD_H_

//@@@ 2001.11.17 add end MIK


