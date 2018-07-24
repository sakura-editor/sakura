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
#ifndef SAKURA_CTYPE_BF915633_AE38_4C73_8E5B_0411063A1AD89_H_
#define SAKURA_CTYPE_BF915633_AE38_4C73_8E5B_0411063A1AD89_H_

#include "CEol.h"
#include "env/CommonSetting.h"
#include "doc/CDocTypeSetting.h"
#include "doc/CLineComment.h"
#include "doc/CBlockComment.h"
#include "charset/charset.h"  // ECodeType
#include "CRegexKeyword.h"	// RegexKeywordInfo


//! タブ表示方法
enum ETabArrow {
	TABARROW_STRING = 0,	//!< 文字指定
	TABARROW_SHORT,			//!< 短い矢印
	TABARROW_LONG,			//!< 長い矢印
};

//! アウトライン解析の種類
enum EOutlineType{
	OUTLINE_C,
	OUTLINE_C_CPP,		// C/C++自動認識
	OUTLINE_PLSQL,
	OUTLINE_TEXT,
	OUTLINE_JAVA,
	OUTLINE_COBOL,
	OUTLINE_ASM,
	OUTLINE_PERL,		//	Sep. 8, 2000 genta
	OUTLINE_VB,			//	June 23, 2001 N.Nakatani
	OUTLINE_WZTXT,		// 2003.05.20 zenryaku 階層付テキストアウトライン解析
	OUTLINE_HTML,		// 2003.05.20 zenryaku HTMLアウトライン解析
	OUTLINE_TEX,		// 2003.07.20 naoh TeXアウトライン解析
	OUTLINE_FILE,		//	2002.04.01 YAZAKI ルールファイル用
	OUTLINE_PYTHON,		//	2007.02.08 genta Pythonアウトライン解析
	OUTLINE_ERLANG,		//	2009.08.10 genta Erlangアウトライン解析
	OUTLINE_XML,		//  2014.12.25 Moca
	OUTLINE_CPP,		//  2015.11.13 Moca
	//	新しいアウトライン解析は必ずこの直前へ挿入
	OUTLINE_CODEMAX,
	OUTLINE_BOOKMARK,	//	2001.12.03 hor
	OUTLINE_PLUGIN,		//	2009.10.29 syat プラグインによるアウトライン解析
	OUTLINE_FILETREE,	//	2012.06.20 Moca ファイルツリー
	OUTLINE_DEFAULT =-1,//	2001.12.03 hor
	OUTLINE_UNKNOWN	= 99,
	OUTLINE_TREE = 100,		// 汎用ツリー 2010.03.28 syat
	OUTLINE_TREE_TAGJUMP = 101,	// 汎用ツリー(タグジャンプ付き) 2013.05.01 Moca
	OUTLINE_CLSTREE = 200,	// 汎用ツリー(クラス) 2010.03.28 syat
	OUTLINE_LIST = 300,		// 汎用リスト 2010.03.28 syat
};

//! スマートインデント種別
enum ESmartIndentType {
	SMARTINDENT_NONE,		//!< なし
	SMARTINDENT_CPP			//!< C/C++
};

//! ヒアドキュメント種別
enum EHereDocType{
	HEREDOC_PHP,			//!< PHP
	HEREDOC_RUBY,			//!< Ruby
	HEREDOC_PERL			//!< Perl
};

//! 背景画像表示位置
enum EBackgroundImagePos {
	BGIMAGE_TOP_LEFT,		//!< 左上
	BGIMAGE_TOP_RIGHT,		//!< 右上
	BGIMAGE_BOTTOM_LEFT,	//!< 左下
	BGIMAGE_BOTTOM_RIGHT,	//!< 右下
	BGIMAGE_CENTER,			//!< 中央
	BGIMAGE_TOP_CENTER,		//!< 中央上
	BGIMAGE_BOTTOM_CENTER,	//!< 中央下
	BGIMAGE_CENTER_LEFT,	//!< 中央左
	BGIMAGE_CENTER_RIGHT	//!< 中央右
};

//! エンコードオプション
struct SEncodingConfig{
	bool				m_bPriorCesu8;					//!< 自動判別時に CESU-8 を優先するかどうか
	ECodeType			m_eDefaultCodetype;				//!< デフォルト文字コード
	EEolType			m_eDefaultEoltype;				//!< デフォルト改行コード	// 2011.01.24 ryoji
	bool				m_bDefaultBom;					//!< デフォルトBOM			// 2011.01.24 ryoji
};

//! 文字列区切り記号エスケープ方法
enum EStringLiteralType{
	STRING_LITERAL_CPP,		//!< C/C++言語風
	STRING_LITERAL_PLSQL,	//!< PL/SQL風
	STRING_LITERAL_HTML,	//!< HTML/XML風
	STRING_LITERAL_CSHARP,	//!< C#風
	STRING_LITERAL_PYTHON,	//!< Python風
};

//! 右クリックメニュー表示
enum EKeyHelpRMenuType{
	KEYHELP_RMENU_NONE,		//!< 非表示
	KEYHELP_RMENU_TOP,		//!< メニュー先頭
	KEYHELP_RMENU_BOTTOM,	//!< メニュー末尾
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       タイプ別設定                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! タイプ別設定
struct STypeConfig{
	//2007.09.07 変数名変更: m_nMaxLineSize→m_nMaxLineKetas
	int					m_nIdx;
	int					m_id;
	TCHAR				m_szTypeName[MAX_TYPES_NAME];	//!< タイプ属性：名称
	TCHAR				m_szTypeExts[MAX_TYPES_EXTS];	//!< タイプ属性：拡張子リスト
	int					m_nTextWrapMethod;				//!< テキストの折り返し方法		// 2008.05.30 nasukoji
	CKetaXInt			m_nMaxLineKetas;				//!< 折り返し桁数
	int					m_nColumnSpace;					//!< 文字と文字の隙間
	int					m_nLineSpace;					//!< 行間のすきま
	CKetaXInt			m_nTabSpace;					//!< TABの文字数
	ETabArrow			m_bTabArrow;					//!< タブ矢印表示		//@@@ 2003.03.26 MIK
	EDIT_CHAR			m_szTabViewString[8+1];			//!< TAB表示文字列	// 2003.1.26 aroka サイズ拡張	// 2009.02.11 ryoji サイズ戻し(17->8+1)
	bool				m_bInsSpace;					//!< スペースの挿入	// 2001.12.03 hor
	int					m_nTsvMode;						//!< TSVモード	// 2015.05.02 syat
	// 2005.01.13 MIK 配列化
	int					m_nKeyWordSetIdx[MAX_KEYWORDSET_PER_TYPE];	//!< キーワードセット

	CLineComment		m_cLineComment;					//!< 行コメントデリミタ			//@@@ 2002.09.22 YAZAKI
	CBlockComment		m_cBlockComments[2];			//!< ブロックコメントデリミタ	//@@@ 2002.09.22 YAZAKI

	int					m_nStringType;					//!< 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""]['']
	bool				m_bStringLineOnly;				//!< 文字列は行内のみ
	bool				m_bStringEndLine;				//!< (終了文字列がない場合)行末まで色分け
	int					m_nHeredocType;
	wchar_t				m_szIndentChars[64];			//!< その他のインデント対象文字

	int					m_nColorInfoArrNum;				//!< 色設定配列の有効数
	ColorInfo			m_ColorInfoArr[64];				//!< 色設定配列

	SFilePath			m_szBackImgPath;				//!< 背景画像
	EBackgroundImagePos m_backImgPos;					//!< 背景画像表示位置
	bool				m_backImgRepeatX;				//!< 背景画像表示横方向繰り返し
	bool				m_backImgRepeatY;				//!< 背景画像表示縦方向繰り返し
	bool				m_backImgScrollX;				//!< 背景画像表示横方向スクロール
	bool				m_backImgScrollY;				//!< 背景画像表示縦方向スクロール
	POINT				m_backImgPosOffset;				//!< 背景画像表示オフセット

	bool				m_bLineNumIsCRLF;				//!< 行番号の表示 false=折り返し単位／true=改行単位
	int					m_nLineTermType;				//!< 行番号区切り  0=なし 1=縦線 2=任意
	wchar_t				m_cLineTermChar;				//!< 行番号区切り文字
	CKetaXInt			m_nVertLineIdx[MAX_VERTLINES];	//!< 指定桁縦線
	int 				m_nNoteLineOffset;				//!< ノート線のオフセット

	bool				m_bWordWrap;					//!< 英文ワードラップをする
	bool				m_bKinsokuHead;					//!< 行頭禁則をする		//@@@ 2002.04.08 MIK
	bool				m_bKinsokuTail;					//!< 行末禁則をする		//@@@ 2002.04.08 MIK
	bool				m_bKinsokuRet;					//!< 改行文字のぶら下げ	//@@@ 2002.04.13 MIK
	bool				m_bKinsokuKuto;					//!< 句読点のぶらさげ	//@@@ 2002.04.17 MIK
	bool				m_bKinsokuHide;					//!< ぶら下げを隠す		// 2011/11/30 Uchi
	wchar_t				m_szKinsokuHead[200];			//!< 行頭禁則文字	//@@@ 2002.04.08 MIK
	wchar_t				m_szKinsokuTail[200];			//!< 行頭禁則文字	//@@@ 2002.04.08 MIK
	wchar_t				m_szKinsokuKuto[200];			//!< 句読点ぶらさげ文字	// 2009.08.07 ryoji

	int					m_nCurrentPrintSetting;			//!< 現在選択している印刷設定

	BOOL				m_bOutlineDockDisp;				//!< アウトライン解析表示の有無
	EDockSide			m_eOutlineDockSide;				//!< アウトライン解析ドッキング配置
	int					m_cxOutlineDockLeft;			//!< アウトラインの左ドッキング幅
	int					m_cyOutlineDockTop;				//!< アウトラインの上ドッキング高
	int					m_cxOutlineDockRight;			//!< アウトラインの右ドッキング幅
	int					m_cyOutlineDockBottom;			//!< アウトラインの下ドッキング高
	int					m_nDockOutline;					//!< ドッキング時のアウトライン/ブックマーク
	EOutlineType		m_eDefaultOutline;				//!< アウトライン解析方法
	SFilePath			m_szOutlineRuleFilename;		//!< アウトライン解析ルールファイル
	int					m_nOutlineSortCol;				//!< アウトライン解析ソート列番号
	bool				m_bOutlineSortDesc;				//!< アウトライン解析ソート降順
	int					m_nOutlineSortType;				//!< アウトライン解析ソート基準
	SFileTree			m_sFileTree;					/*!< ファイルツリー設定 */

	ESmartIndentType	m_eSmartIndent;					//!< スマートインデント種別
	bool				m_bIndentCppStringIgnore;		//!< C/C++インデント：文字列を無視する
	bool				m_bIndentCppCommentIgnore;		//!< C/C++インデント：コメントを無視する
	bool				m_bIndentCppUndoSep;			//!< C/C++インデント：Undoバッファを分ける
	int					m_nImeState;					//!< 初期IME状態	Nov. 20, 2000 genta

	//	2001/06/14 asa-o 補完のタイプ別設定
	SFilePath			m_szHokanFile;					//!< 入力補完 単語ファイル
	int					m_nHokanType;					//!< 入力補完 種別(プラグイン)
	//	2003.06.23 Moca ファイル内からの入力補完機能
	bool				m_bUseHokanByFile;				//!< 入力補完 開いているファイル内から候補を探す
	bool				m_bUseHokanByKeyword;			//!< 強調キーワードから入力補完
	
	//	2001/06/19 asa-o
	bool				m_bHokanLoHiCase;				//!< 入力補完機能：英大文字小文字を同一視する

	SFilePath			m_szExtHelp;					//!< 外部ヘルプ１
	SFilePath			m_szExtHtmlHelp;				//!< 外部HTMLヘルプ
	bool				m_bHtmlHelpIsSingle;			//!< HtmlHelpビューアはひとつ

	bool				m_bChkEnterAtEnd;				//!< 保存時に改行コードの混在を警告する	2013/4/14 Uchi

	SEncodingConfig		m_encoding;						//!< エンコードオプション


//@@@ 2001.11.17 add start MIK
	bool				m_bUseRegexKeyword;								//!< 正規表現キーワードを使うか
	DWORD				m_nRegexKeyMagicNumber;							//!< 正規表現キーワード更新マジックナンバー
	RegexKeywordInfo	m_RegexKeywordArr[MAX_REGEX_KEYWORD];			//!< 正規表現キーワード
	wchar_t				m_RegexKeywordList[MAX_REGEX_KEYWORDLISTLEN];	//!< 正規表現キーワード
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	bool				m_bUseKeyWordHelp;				//!< キーワード辞書セレクト機能を使うか
	int					m_nKeyHelpNum;					//!< キーワード辞書の冊数
	KeyHelpInfo			m_KeyHelpArr[MAX_KEYHELP_FILE];	//!< キーワード辞書ファイル
	bool				m_bUseKeyHelpAllSearch;			//!< ヒットした次の辞書も検索(&A)
	bool				m_bUseKeyHelpKeyDisp;			//!< 1行目にキーワードも表示する(&W)
	bool				m_bUseKeyHelpPrefix;			//!< 選択範囲で前方一致検索(&P)
	EKeyHelpRMenuType	m_eKeyHelpRMenuShowType;		//!< 右クリックメニュー表示
//@@@ 2006.04.10 fon ADD-end

	//	2002/04/30 YAZAKI Commonから移動。
	bool				m_bAutoIndent;					//!< オートインデント
	bool				m_bAutoIndent_ZENSPACE;			//!< 日本語空白もインデント
	bool				m_bRTrimPrevLine;				//!< 2005.10.11 ryoji 改行時に末尾の空白を削除
	int					m_nIndentLayout;				//!< 折り返しは2行目以降を字下げ表示

	//	Sep. 10, 2002 genta
	bool				m_bUseDocumentIcon;				//!< ファイルに関連づけられたアイコンを使う

	bool				m_bUseTypeFont;					//!< タイプ別フォントの使用
	LOGFONT				m_lf;							//!< フォント // 2013.03.18 aroka
	INT					m_nPointSize;					//!< フォントサイズ（1/10ポイント単位）

	STypeConfig()
	: m_nMaxLineKetas(10) //	画面折り返し幅がTAB幅以下にならないことを初期値でも保証する	//	2004.04.03 Moca
	{
	}

	int					m_nLineNumWidth;				//!< 行番号の最小桁数 2014.08.02 katze
}; /* STypeConfig */

//! タイプ別設定(mini)
struct STypeConfigMini
{
	int					m_id;
	TCHAR				m_szTypeName[MAX_TYPES_NAME];	//!< タイプ属性：名称
	TCHAR				m_szTypeExts[MAX_TYPES_EXTS];	//!< タイプ属性：拡張子リスト
	SEncodingConfig		m_encoding;						//!< エンコードオプション
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   タイプ別設定アクセサ                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//!ドキュメント種類。共有データ内 STypeConfig へのアクセサも兼ねる。
//2007.12.13 kobake 作成
class CTypeConfig{
public:
	CTypeConfig()
	{
#ifdef _DEBUG
		//元がintだったので、未初期化で使うと問題が発生するように、あえて、変な値を入れておく。
		m_nType = 1234;
#else
		//リリース時は、未初期化でも問題が起こりにくいように、ゼロクリアしておく
		m_nType = 0;
#endif
	}
	explicit CTypeConfig(int n)
	{
		m_nType = n;
	}
	bool IsValidType() const{ return m_nType>=0 && m_nType<MAX_TYPES; }
	int GetIndex() const{ /*assert(IsValid());*/ return m_nType; }

	//共有データへの簡易アクセサ
//	STypeConfig* operator->(){ return GetTypeConfig(); }
//	STypeConfig* GetTypeConfig();
private:
	int m_nType;
};



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        タイプ設定                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CType{
public:
	virtual ~CType(){ }
	void InitTypeConfig(int nIdx, STypeConfig&);
protected:
	virtual void InitTypeConfigImp(STypeConfig* pType) = 0;
};

#define GEN_CTYPE(CLASS_NAME) \
class CLASS_NAME : public CType{ \
protected: \
	void InitTypeConfigImp(STypeConfig* pType); \
};

GEN_CTYPE(CType_Asm)
GEN_CTYPE(CType_Awk)
GEN_CTYPE(CType_Basis)
GEN_CTYPE(CType_Cobol)
GEN_CTYPE(CType_Cpp)
GEN_CTYPE(CType_Dos)
GEN_CTYPE(CType_Html)
GEN_CTYPE(CType_Ini)
GEN_CTYPE(CType_Java)
GEN_CTYPE(CType_Pascal)
GEN_CTYPE(CType_Perl)
GEN_CTYPE(CType_Rich)
GEN_CTYPE(CType_Sql)
GEN_CTYPE(CType_Tex)
GEN_CTYPE(CType_Text)
GEN_CTYPE(CType_Vb)
GEN_CTYPE(CType_Other)



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	スペースの判定
*/
inline bool C_IsSpace( wchar_t c, bool bExtEol )
{
	return (
		L'\t' == c ||
		L' ' == c ||
		WCODE::IsLineDelimiter(c, bExtEol)
	);
}

#endif /* SAKURA_CTYPE_BF915633_AE38_4C73_8E5B_0411063A1AD89_H_ */
/*[EOF]*/
