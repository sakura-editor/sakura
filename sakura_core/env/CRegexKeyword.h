/*!	@file
	@brief CRegexKeyword Library

	正規表現キーワードを扱う。
	BREGEXP.DLLを利用する。

	@author MIK
	@date Nov. 17, 2001
*/
/*
	Copyright (C) 2001, MIK
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

#ifndef SAKURA_CREGEXKEYWORD_84205099_71D3_4F2B_A6E5_821DA1C8D201_H_
#define SAKURA_CREGEXKEYWORD_84205099_71D3_4F2B_A6E5_821DA1C8D201_H_
#pragma once

#include "_main/global.h"
#include "extmodule/CBregexp.h"
#include "config/maxdata.h" //MAX_REGEX_KEYWORD

struct STypeConfig;

//@@@ 2001.11.17 add start MIK
struct RegexKeywordInfo {
	int	m_nColorIndex;		//色指定番号
};
//@@@ 2001.11.17 add end MIK

enum class ERkStatus {
	RK_EMPTY   = 0,	//初期状態
	RK_CLOSE   = 1,	//BREGEXPクローズ
	RK_OPEN    = 2,	//BREGEXPオープン
	RK_ACTIVE  = 3,	//コンパイル済み
	RK_ERROR   = 9,	//コンパイルエラー
};

enum class ERkMStatus {
	RK_EMPTY   = 0,	//初期状態
	RK_MATCH   = 4,	//マッチする
	RK_NOMATCH = 5,	//この行ではマッチしない
};

//!	正規表現キーワード検索情報構造体
struct RegexInfo_t {
	using CPatternHolder = std::unique_ptr<CBregexp::CPattern>;

	CPatternHolder pPattern = nullptr;	//コンパイル済みパターン

	ERkStatus	nStatus;	//!< 状態(EMPTY,CLOSE,OPEN,ACTIVE,ERROR)
	ERkMStatus	nMatch;		//!< このキーワードのマッチ状態(EMPTY,MATCH,NOMATCH)
	int			nOffset;	//!< マッチした位置
	int			nLength;	//!< マッチした長さ
	int			nHead;		//!< 先頭のみチェックするか？
	ERkMStatus	nFlag;		//!< 色指定のチェックが入っているか？ YES=RK_EMPTY, NO=RK_NOMATCH
};
using REGEX_INFO = RegexInfo_t;
using REGEX_INFO_ARR = std::array<REGEX_INFO, MAX_REGEX_KEYWORD>;

class CStringRef;

//!	正規表現キーワードクラス
/*!
	正規表現キーワードを扱う。
*/
class CRegexKeyword : public CBregexp {
public:
	explicit CRegexKeyword(const std::filesystem::path& bregonigPath);
	~CRegexKeyword() override;

	//! 行検索開始
	BOOL RegexKeyLineStart( void );
	//! 行検索
	BOOL RegexIsKeyword(std::wstring_view text, int nPos, int *nMatchLen, int *nMatchColor);
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
	int				m_nTypeIndex = -1;			//!< 現在のタイプ設定番号
	bool			m_bUseRegexKeyword = false;	//!< 正規表現キーワードを使用する・しない

private:
	const STypeConfig*	m_pTypes = nullptr;		//!< タイプ設定へのポインタ(呼び出し側が持っているもの)
	int				m_nTypeId = -1;				//!< タイプ設定ID
	DWORD			m_nCompiledMagicNumber = 1;	//!< コンパイル済みか？
	int				m_nRegexKeyCount;			//!< 現在のキーワード数
	REGEX_INFO_ARR	m_sInfo{};					//!< キーワード一覧(BREGEXPコンパイル対象)
};

//@@@ 2001.11.17 add end MIK
#endif /* SAKURA_CREGEXKEYWORD_84205099_71D3_4F2B_A6E5_821DA1C8D201_H_ */
