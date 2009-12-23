#pragma once

class CEditDoc;
class CFuncInfoArr;
struct SOneRule;

/*! ルールファイルの1行を管理する構造体

	@date 2002.04.01 YAZAKI
	@date 2007.11.29 kobake 名前変更: oneRule→SOneRule
*/
struct SOneRule {
	wchar_t szMatch[256];
	int		nLength;
	wchar_t szGroupName[256];
};

class CDocOutline{
public:
	CDocOutline(CEditDoc* pcDoc) : m_pcDocRef(pcDoc) { }
	void	MakeFuncList_C( CFuncInfoArr*,bool bVisibleMemberFunc = true );					//!< C/C++関数リスト作成
	void	MakeFuncList_PLSQL( CFuncInfoArr* );											//!< PL/SQL関数リスト作成
	void	MakeTopicList_txt( CFuncInfoArr* );												//!< テキスト・トピックリスト作成
	void	MakeFuncList_Java( CFuncInfoArr* );												//!< Java関数リスト作成
	void	MakeTopicList_cobol( CFuncInfoArr* );											//!< COBOL アウトライン解析
	void	MakeTopicList_asm( CFuncInfoArr* );												//!< アセンブラ アウトライン解析
	void	MakeFuncList_Perl( CFuncInfoArr* );												//!< Perl関数リスト作成	//	Sep. 8, 2000 genta
	void	MakeFuncList_VisualBasic( CFuncInfoArr* );										//!< Visual Basic関数リスト作成 //June 23, 2001 N.Nakatani
	void	MakeFuncList_python( CFuncInfoArr* pcFuncInfoArr );								//!< Python アウトライン解析 // 2007.02.08 genta
	void	MakeFuncList_Erlang( CFuncInfoArr* pcFuncInfoArr );								//!< Erlang アウトライン解析 // 2009.08.10 genta
	void	MakeTopicList_wztxt(CFuncInfoArr*);												//!< 階層付きテキスト アウトライン解析 // 2003.05.20 zenryaku
	void	MakeTopicList_html(CFuncInfoArr*);												//!< HTML アウトライン解析 // 2003.05.20 zenryaku
	void	MakeTopicList_tex(CFuncInfoArr*);												//!< TeX アウトライン解析 // 2003.07.20 naoh
	void	MakeFuncList_RuleFile( CFuncInfoArr* );											//!< ルールファイルを使ってリスト作成 2002.04.01 YAZAKI
	int		ReadRuleFile( const TCHAR* pszFilename, SOneRule* pcOneRule, int nMaxCount );	//!< ルールファイル読込 2002.04.01 YAZAKI
	void	MakeFuncList_BookMark( CFuncInfoArr* );											//!< ブックマークリスト作成 //2001.12.03 hor
private:
	CEditDoc* m_pcDocRef;
};

