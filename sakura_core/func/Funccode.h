/*!	@file
	@brief 機能番号定義

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, MIK, Stonee, asa-o, Misaka, hor
	Copyright (C) 2002, aroka, YAZAKI, minfu, MIK, ai, genta
	Copyright (C) 2003, 鬼, genta, MIK, Moca
	Copyright (C) 2004, genta, zenryaku, kazika, Moca, isearch
	Copyright (C) 2005, genta, MIK, maru
	Copyright (C) 2006, aroka, かろと, fon, ryoji
	Copyright (C) 2007, ryoji, genta

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

#ifndef _FUNCCODE_H_
#define _FUNCCODE_H_

//Oct. 17, 2000 jepro  F_FILECLOSE:「ファイルを閉じる」というキャプションを変更
//Feb. 28, 2004 genta  F_FILESAVECLOSE
//Jan. 24, 2005 genta  F_FILESAVEALL
//Jan. 24, 2005 genta  F_FILESAVE_QUIET
//2007.02.13    ryoji  F_EXITALLEDITORS追加
//Sep. 14, 2000 jepro  F_PRINT_PAGESETUP:「印刷のページレイアウトの設定」から「印刷ページ設定」に変更 
//Feb.  9, 2001 jepro  F_OPEN_HHPP「.cまたは.cppと同名の.hを開く」から変更
//Feb.  9, 2001 jepro  F_OPEN_CCPP「.hと同名の.c(なければ.cpp)を開く」から変更
//Feb.  7, 2001 JEPRO  F_OPEN_HfromtoC 追加
//Sep. 17, 2000 jepro  F_PLSQL_COMPILE_ON_SQLPLUS 説明の「コンパイル」を「実行」に統一
//Dec. 27, 2000 JEPRO  F_EXITALL追加
//2007.09.02 kobake パラメータがwchar_tで来ることを示唆するよう、名前を F_CHAR から F_WCHAR に変更
//2001.12.03 hor    F_LTRIM
//2001.12.03 hor    F_RTRIM
//2001.12.06 hor    F_SORT_ASC
//2001.12.06 hor    F_SORT_DESC
//2001.12.06 hor    F_MERGE
//2002.04.09 minfu  F_RECONVERT
//Dec. 4, 2002 genta F_FILE_REOPEN～
//Oct. 10, 2000 JEPRO F_ROLLDOWN     下の半ページアップに名称変更
//Oct. 10, 2000 JEPRO F_ROLLUP       下の半ページダウンに名称変更
//Oct.  6, 2000 JEPRO F_HalfPageUp   名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
//Oct.  6, 2000 JEPRO F_HalfPageDown 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
//Oct. 10, 2000 JEPRO F_1PageUp      従来のページアップを半ページアップと名称変更し１ページアップを追加
//Oct. 10, 2000 JEPRO F_1PageDown    従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
//2001/06/20 asa-o F_WndScrollDown
//2001/06/20 asa-o F_WndScrollUp
//Oct. 10, 2000 JEPRO F_ROLLDOWN_SEL     下の半ページアップに名称変更
//Oct. 10, 2000 JEPRO F_ROLLUP_SEL       下の半ページダウンに名称変更
//Oct.  6, 2000 JEPRO F_HalfPageUp_Sel   名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
//Oct.  6, 2000 JEPRO F_HalfPageDown_Sel 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
//Oct. 10, 2000 JEPRO F_1PageUp_Sel      従来のページアップを半ページアップと名称変更し１ページアップを追加
//Oct. 10, 2000 JEPRO F_1PageDown_Sel    従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
//2002/2/3 aroka F_COPYFNAME
//Sept. 15, 2000 JEPRO F_CREATEKEYBINDLIST 上の行はIDM_TESTのままではうまくいかないのでFに変えて登録
//2002.06.02     MIK    F_CTRL_CODE_DIALOG			
//Aug. 29, 2002  ai     F_TOHANKATA				 	
//Sep. 17, 2000  jepro  F_TOZENKAKUKATA 説明を「半角→全角カタカナ」から変更
//Sep. 17, 2000  jepro  F_TOZENKAKUHIRA 説明を「半角→全角ひらがな」から変更
//Jly. 30, 2001  Misaka F_TOZENEI					
//2002.2.11      YAZAKI F_TOHANEI							
//2001/05/27     Stonee F_SPACETOTAB		
//Oct. 17, 2000  jepro  F_UUDECODE 説明を「選択部分をUUENCODEデコード」から変更
//02/06/26 ai F_JUMP_SRCHSTARTPOS		
//2002.06.15 MIK F_SEARCH_BOX			
//20060201 aroka F_OUTLINE_TOGGLE	 
//2003.04.13 MIK F_TAGJUMP_LIST			
//2003.04.13 MIK F_TAGS_MAKE			
//2003.04.15 MIK F_DIRECT_TAGJUMP		
//Apr. 03. 2003 genta F_TAGJUMP_CLOSE		 
//2005.03.31 MIK F_TAGJUMP_KEYWORD		
//2002.05.25 MIK F_DIFF_DIALOG			
//2002.05.25 MIK F_DIFF					
//2002.05.25 MIK F_DIFF_NEXT			
//2002.05.25 MIK F_DIFF_PREV			
//2002.05.25 MIK F_DIFF_RESET		
//2004.10.13 isearch F_ISEARCH_NEXT	    
//2004.10.13 isearch F_ISEARCH_PREV	 
//2004.10.13 isearch F_ISEARCH_REGEXP_NEXT   
//2004.10.13 isearch F_ISEARCH_REGEXP_PREV  
//2004.10.13 isearch F_ISEARCH_MIGEMO_NEXT   
//2004.10.13 isearch F_ISEARCH_MIGEMO_PREV    
//2003.06.08 Moca F_CHGMOD_EOL_CRLF
//2003.06.08 Moca F_CHGMOD_EOL_LF
//2003.06.08 Moca F_CHGMOD_EOL_CR
//2003.06.23 Moca F_CHGMOD_EOL           
//2005.01.29 genta F_SET_QUOTESTRING     	
//2006.04.26 maru F_TRACEOUT            	
//2003.06.10 MIK F_SHOWTAB			 	
//2003.04.08 MIK F_FAVORITE				
//Stonee, 2001/05/18 設定ダイアログ用の機能番号を用意 F_TYPE_SCREEN～F_OPTION_HELPER
//Jul. 03, 2001 JEPRO 追加 F_TYPE_HELPER		 	
//2001.11.17 add MIK F_TYPE_REGEX_KEYWORD	 	
//2006.10.06 ryoji 追加 F_TYPE_KEYHELP		 	
//Feb. 11, 2007 genta 追加 F_OPTION_TAB		 
//Oct. 7, 2000 JEPRO  F_WRAPWINDOWWIDTH  WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更 F_SPLIT_V				
//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更 F_SPLIT_H				
//Sept. 17, 2000 jepro 説明に「に」を追加 F_SPLIT_VH				
//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL) F_WIN_CLOSEALL			
//2004.07.14 kazika F_BIND_WINDOW			
//2004-09-21 Moca F_TOPMOST			
//2006.03.23 fon F_WINLIST			
//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一 F_MINIMIZE_ALL				
//2001.02.10 by MIK F_MAXIMIZE_H			
//2007.06.20 ryoji F_GROUPCLOSE	
//2007.06.20 ryoji F_NEXTGROUP			
//2007.06.20 ryoji F_PREVGROUP			
//2007.06.20 ryoji F_TAB_MOVERIGHT	
//2007.06.20 ryoji F_TAB_MOVELEFT	
//2007.06.20 ryoji F_TAB_SEPARATE		
//2007.06.20 ryoji F_TAB_JOINTNEXT			
//2007.06.20 ryoji F_TAB_JOINTPREV			
//2006.03.24 fon F_TOGGLE_KEY_SEARCH		
//Nov. 25, 2000 JEPRO 追加 F_HELP_CONTENTS			
//Nov. 25, 2000 JEPRO 追加 F_HELP_SEARCH			
//Dec. 24, 2000 JEPRO 追加 F_ABOUT					
//Oct. 19, 2002 genta F_GETSELECTED
//2003-02-21 鬼 F_EXPANDPARAMETER 
//2003.06.25 Moca F_GETLINESTR       
//2003.06.25 Moca F_GETLINECOUNT    
//2004.03.16 zenryaku F_CHGTABWIDTH     
//2005.7.30 maru F_ISTEXTSELECTED  
//2005.7.30 maru F_GETSELLINEFROM  
//2005.7.30 maru F_GETSELCOLUMNFROM   
//2005.7.30 maru F_GETSELLINETO    
//2005.7.30 maru F_GETSELCOLUMNTO    
//2005.7.30 maru F_ISINSMODE        
//2005.07.31 maru F_GETCHARCODE      
//2005.08.05 maru F_GETLINECODE      
//2005.08.05 maru F_ISPOSSIBLEUNDO   
//2005.08.05 maru F_ISPOSSIBLEREDO   

/*
引数の扱いは
  CEditView::HandleCommand
  CMacro::HandleFunction
  MacroFuncInfo CSMacroMgr::m_MacroFuncInfoArr[]
を参照
*/

//2007.09.30 kobake 機能番号定数を列挙型に変更。(デバッグをしやすくするため)
#include "Funccode_enum.h"

#ifndef UINT16
#define UINT16 WORD
#endif
#ifndef uint16_t
typedef UINT16 uint16_t;
#endif

// 機能一覧に関するデータ宣言
namespace nsFuncCode{
	extern const uint16_t		ppszFuncKind[];
	extern const int			nFuncKindNum;
	extern const int			pnFuncListNumArr[];
	extern const EFunctionCode*	ppnFuncListArr[];
	extern const int			nFincListNumArrNum;

	extern const EFunctionCode	pnFuncList_Special[];
	extern const int			nFuncList_Special_Num;
};
///////////////////////////////////////////////////////////////////////


/*機能番号に対応したヘルプトピックIDを返す*/
int FuncID_To_HelpContextID( EFunctionCode nFuncID );	//Stonee, 2001/02/23

class CEditDoc;
struct DLLSHAREDATA;

//2007.10.30 kobake 機能チェックをCEditWndからここへ移動
bool IsFuncEnable( const CEditDoc*, const DLLSHAREDATA*, EFunctionCode );	/* 機能が利用可能か調べる */
bool IsFuncChecked( const CEditDoc*, const DLLSHAREDATA*, EFunctionCode );	/* 機能がチェック状態か調べる */

#endif // _FUNCCODE_H_
