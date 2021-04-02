﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_CDOCOUTLINE_BDF55702_D938_432D_99F2_BF0F98A7C5FE_H_
#define SAKURA_CDOCOUTLINE_BDF55702_D938_432D_99F2_BF0F98A7C5FE_H_
#pragma once

class CEditDoc;
class CFuncInfoArr;
struct SOneRule;
enum EOutlineType;

class CDocOutline{
public:
	CDocOutline(CEditDoc* pcDoc) : m_pcDocRef(pcDoc) { }
	void	MakeFuncList_C( CFuncInfoArr* pcFuncInfoArr,
							EOutlineType& nOutlineType,
						    const WCHAR* pszFileName,
							bool bVisibleMemberFunc = true );					//!< C/C++関数リスト作成
	void	MakeFuncList_PLSQL( CFuncInfoArr* pcFuncInfoArr );					//!< PL/SQL関数リスト作成
	void	MakeTopicList_txt( CFuncInfoArr* pcFuncInfoArr );					//!< テキスト・トピックリスト作成
	void	MakeFuncList_Java( CFuncInfoArr* pcFuncInfoArr );					//!< Java関数リスト作成
	void	MakeTopicList_cobol( CFuncInfoArr* pcFuncInfoArr );					//!< COBOL アウトライン解析
	void	MakeTopicList_asm( CFuncInfoArr* pcFuncInfoArr );					//!< アセンブラ アウトライン解析
	void	MakeFuncList_Perl( CFuncInfoArr* pcFuncInfoArr );					//!< Perl関数リスト作成	//	Sep. 8, 2000 genta
	void	MakeFuncList_VisualBasic( CFuncInfoArr* pcFuncInfoArr );			//!< Visual Basic関数リスト作成 //June 23, 2001 N.Nakatani
	void	MakeFuncList_python( CFuncInfoArr* pcFuncInfoArr );					//!< Python アウトライン解析 // 2007.02.08 genta
	void	MakeFuncList_Erlang( CFuncInfoArr* pcFuncInfoArr );					//!< Erlang アウトライン解析 // 2009.08.10 genta
	void	MakeTopicList_wztxt( CFuncInfoArr* pcFuncInfoArr );					//!< 階層付きテキスト アウトライン解析 // 2003.05.20 zenryaku
	void	MakeTopicList_html(CFuncInfoArr* pcFuncInfoArr, bool bXml);			//!< HTML アウトライン解析 // 2003.05.20 zenryaku
	void	MakeTopicList_tex(CFuncInfoArr* pcFuncInfoArr);						//!< TeX アウトライン解析 // 2003.07.20 naoh
	void	MakeFuncList_RuleFile( CFuncInfoArr* pcFuncInfoArr,
								   std::wstring& sTitleOverride );				//!< ルールファイルを使ってリスト作成 2002.04.01 YAZAKI
	int		ReadRuleFile( const WCHAR* pszFilename, SOneRule* pcOneRule,
						  int nMaxCount, bool& bRegex, std::wstring& title );	//!< ルールファイル読込 2002.04.01 YAZAKI
	void	MakeFuncList_BookMark( CFuncInfoArr* );								//!< ブックマークリスト作成 //2001.12.03 hor
private:
	CEditDoc* m_pcDocRef;
};
#endif /* SAKURA_CDOCOUTLINE_BDF55702_D938_432D_99F2_BF0F98A7C5FE_H_ */
