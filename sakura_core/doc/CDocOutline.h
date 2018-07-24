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
#ifndef SAKURA_CDOCOUTLINE_F40D43DF_FDBE_4758_B015_E9C369A17606_H_
#define SAKURA_CDOCOUTLINE_F40D43DF_FDBE_4758_B015_E9C369A17606_H_

class CEditDoc;
class CFuncInfoArr;
struct SOneRule;

class CDocOutline{
public:
	CDocOutline(CEditDoc* pcDoc) : m_pcDocRef(pcDoc) { }
	void	MakeFuncList_C( CFuncInfoArr*, EOutlineType& nOutlineType, const TCHAR* pszFileName,
		bool bVisibleMemberFunc = true );					//!< C/C++関数リスト作成
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
	void	MakeTopicList_html(CFuncInfoArr*, bool bXml);									//!< HTML アウトライン解析 // 2003.05.20 zenryaku
	void	MakeTopicList_tex(CFuncInfoArr*);												//!< TeX アウトライン解析 // 2003.07.20 naoh
	void	MakeFuncList_RuleFile( CFuncInfoArr*, std::tstring& );											//!< ルールファイルを使ってリスト作成 2002.04.01 YAZAKI
	int		ReadRuleFile( const TCHAR*, SOneRule*, int, bool&, std::wstring& );	//!< ルールファイル読込 2002.04.01 YAZAKI
	void	MakeFuncList_BookMark( CFuncInfoArr* );											//!< ブックマークリスト作成 //2001.12.03 hor
private:
	CEditDoc* m_pcDocRef;
};

#endif /* SAKURA_CDOCOUTLINE_F40D43DF_FDBE_4758_B015_E9C369A17606_H_ */
/*[EOF]*/
