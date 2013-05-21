/*!	@file
	キーボードマクロ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, hor
	Copyright (C) 2002, YAZAKI, aroka, genta, Moca, hor
	Copyright (C) 2003, 鬼, ryoji, Moca
	Copyright (C) 2004, genta, zenryaku
	Copyright (C) 2005, MIK, genta, maru, zenryaku, FILE
	Copyright (C) 2006, かろと, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, nasukoji, ryoji
	Copyright (C) 2009, ryoji, nasukoji
	Copyright (C) 2011, syat

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
#include "func/Funccode.h"
#include "CMacro.h"
#include "_main/CControlTray.h"
#include "view/CEditView.h" //2002/2/10 aroka
#include "macro/CSMacroMgr.h" //2002/2/10 aroka
#include "doc/CEditDoc.h"	//	2002/5/13 YAZAKI ヘッダ整理
#include "_os/OleTypes.h" //2003-02-21 鬼
#include "io/CTextStream.h"
#include "window/CEditWnd.h"
#include "env/CSakuraEnvironment.h"
#include "dlg/CDlgInput1.h"
#include "util/format.h"
#include "util/shell.h"
#include "uiparts/CWaitCursor.h"

CMacro::CMacro( EFunctionCode nFuncID )
{
	m_nFuncID = nFuncID;
	m_pNext = NULL;
	m_pParamTop = m_pParamBot = NULL;
}

CMacro::~CMacro( void )
{
	CMacroParam* p = m_pParamTop;
	CMacroParam* del_p;
	while (p){
		del_p = p;
		p = p->m_pNext;
		delete[] del_p->m_pData;
		delete del_p;
	}
	return;
}

/*	引数の型振り分け
	機能IDによって、期待する型は異なります。
	そこで、引数の型を機能IDによって振り分けて、AddParamしましょう。
	たとえば、F_INSTEXT_Wの1つめ、2つめの引数は文字列、3つめの引数はintだったりするのも、ここでうまく振り分けられることを期待しています。

	lParamは、HandleCommandのparamに値を渡しているコマンドの場合にのみ使います。
*/
void CMacro::AddLParam( LPARAM lParam, const CEditView* pcEditView )
{
	switch( m_nFuncID ){
	/*	文字列パラメータを追加 */
	case F_INSTEXT_W:
	case F_FILEOPEN:
	case F_EXECMD:
	case F_EXECEXTMACRO:
		{
			AddStringParam( (const wchar_t*)lParam );	//	lParamを追加。
			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().m_nExecFlgOpt;
			AddIntParam( lFlag );
		}
		break;

	case F_JUMP:	//	指定行へジャンプ（ただしPL/SQLコンパイルエラー行へのジャンプは未対応）
		{
			AddIntParam( pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgJump.m_nLineNum );
			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().m_bLineNumIsCRLF_ForJump		? 0x01 : 0x00;
			lFlag |= pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgJump.m_bPLSQL	? 0x02 : 0x00;
			AddIntParam( lFlag );
		}
		break;

	case F_BOOKMARK_PATTERN:	//2002.02.08 hor
	case F_SEARCH_NEXT:
	case F_SEARCH_PREV:
		{
			AddStringParam( pcEditView->m_strCurSearchKey.c_str() );	//	lParamを追加。

			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_sCurSearchOption.bWordOnly		? 0x01 : 0x00;
			lFlag |= pcEditView->m_sCurSearchOption.bLoHiCase		? 0x02 : 0x00;
			lFlag |= pcEditView->m_sCurSearchOption.bRegularExp		? 0x04 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND				? 0x08 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind				? 0x10 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bSearchAll					? 0x20 : 0x00;
			AddIntParam( lFlag );
		}
		break;
	case F_REPLACE:
	case F_REPLACE_ALL:
		{
			AddStringParam( pcEditView->m_strCurSearchKey.c_str() );	//	lParamを追加。
			AddStringParam( pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgReplace.m_strText2.c_str() );	//	lParamを追加。

			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_sCurSearchOption.bWordOnly		? 0x01 : 0x00;
			lFlag |= pcEditView->m_sCurSearchOption.bLoHiCase		? 0x02 : 0x00;
			lFlag |= pcEditView->m_sCurSearchOption.bRegularExp	? 0x04 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND				? 0x08 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind				? 0x10 : 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bSearchAll					? 0x20 : 0x00;
			lFlag |= pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgReplace.m_nPaste					? 0x40 : 0x00;	//	CShareDataに入れなくていいの？
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bSelectedArea					? 0x80 : 0x00;	//	置換する時は選べない
			lFlag |= pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgReplace.m_nReplaceTarget << 8;	//	8bitシフト（0x100で掛け算）
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bConsecutiveAll				? 0x0400: 0x00;	// 2007.01.16 ryoji
			AddIntParam( lFlag );
		}
		break;
	case F_GREP:
		{
			AddStringParam( pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgGrep.m_strText.c_str() );	//	lParamを追加。
			AddStringParam( GetDllShareData().m_sSearchKeywords.m_aGrepFiles[0] );	//	lParamを追加。
			AddStringParam( GetDllShareData().m_sSearchKeywords.m_aGrepFolders[0] );	//	lParamを追加。

			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bGrepSubFolder				? 0x01 : 0x00;
			//			この編集中のテキストから検索する(0x02.未実装)
			lFlag |= pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgGrep.m_sSearchOption.bLoHiCase		? 0x04 : 0x00;
			lFlag |= pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgGrep.m_sSearchOption.bRegularExp	? 0x08 : 0x00;
			lFlag |= (GetDllShareData().m_Common.m_sSearch.m_nGrepCharSet == CODE_AUTODETECT) ? 0x10 : 0x00;	//	2002/09/21 Moca 下位互換性のための処理
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_bGrepOutputLine				? 0x20 : 0x00;
			lFlag |= (GetDllShareData().m_Common.m_sSearch.m_nGrepOutputStyle == 2)		? 0x40 : 0x00;	//	CShareDataに入れなくていいの？
			lFlag |= GetDllShareData().m_Common.m_sSearch.m_nGrepCharSet << 8;
			AddIntParam( lFlag );
		}
		break;
	/*	数値パラメータを追加 */
	case F_WCHAR:
		AddIntParam( lParam ); //※文字コードが渡される
		break;
	case F_CHGMOD_EOL:
		{
			// EOLタイプ値をマクロ引数値に変換する	// 2009.08.18 ryoji
			int nFlag;
			switch( (int)lParam ){
			case EOL_CRLF:	nFlag = 1; break;
//			case EOL_LFCR:	nFlag = 2; break;
			case EOL_LF:	nFlag = 3; break;
			case EOL_CR:	nFlag = 4; break;
			default:		nFlag = 0; break;
			}
			AddIntParam( nFlag );
		}
		break;

	/*	標準もパラメータを追加 */
	default:
		AddIntParam( lParam );
		break;
	}
}

/*	引数に文字列を追加。
*/
void CMacro::AddStringParam( const WCHAR* szParam )
{
	CMacroParam* param = new CMacroParam;
	param->m_pNext = NULL;

	//	必要な領域を確保してコピー。
	int nLen = auto_strlen( szParam );
	param->m_pData = new WCHAR[nLen + 1];
	auto_memcpy(param->m_pData, szParam, nLen );
	param->m_pData[nLen] = LTEXT('\0');

	//	リストの整合性を保つ
	if (m_pParamTop){
		m_pParamBot->m_pNext = param; 
		m_pParamBot = param;
	}
	else {
		m_pParamTop = param;
		m_pParamBot = m_pParamTop;
	}
}

/*	引数に数値を追加。
*/
void CMacro::AddIntParam( const int nParam )
{
	CMacroParam* param = new CMacroParam;
	param->m_pNext = NULL;

	//	必要な領域を確保してコピー。
	param->m_pData = new WCHAR[16];	//	数値格納（最大16桁）用
	_itow(nParam, param->m_pData, 10);

	//	リストの整合性を保つ
	if (m_pParamTop){
		m_pParamBot->m_pNext = param; 
		m_pParamBot = param;
	}
	else {
		m_pParamTop = param;
		m_pParamBot = m_pParamTop;
	}
}

/**	コマンドを実行する（pcEditView->GetCommander().HandleCommandを発行する）
	m_nFuncIDによって、引数の型を正確に渡してあげましょう。
	
	@note
	paramArrは何かのポインタ（アドレス）をLONGであらわした値になります。
	引数がchar*のときは、paramArr[i]をそのままHandleCommandに渡してかまいません。
	引数がintのときは、*((int*)paramArr[i])として渡しましょう。
	
	たとえば、F_INSTEXT_Wの1つめ、2つめの引数は文字列、3つめの引数はint、4つめの引数が無し。だったりする場合は、次のようにしましょう。
	pcEditView->GetCommander().HandleCommand( m_nFuncID, true, paramArr[0], paramArr[1], *((int*)paramArr[2]), 0);
	
	@date 2007.07.20 genta : flags追加．FA_FROMMACROはflagsに含めて渡すものとする．
		(1コマンド発行毎に毎回演算する必要はないので)
*/
void CMacro::Exec( CEditView* pcEditView, int flags ) const
{
	const WCHAR* paramArr[4] = {NULL, NULL, NULL, NULL};	//	4つに限定。
	
	CMacroParam* p = m_pParamTop;
	int i = 0;
	for (i = 0; i < 4; i++) {
		if (!p) break;	//	pが無ければbreak;
		paramArr[i] = p->m_pData;
		p = p->m_pNext;
	}
	CMacro::HandleCommand(pcEditView, (EFunctionCode)(m_nFuncID | flags), paramArr, i);
}

/*	CMacroを再現するための情報をhFileに書き出します。

	InsText("なんとか");
	のように。
*/
void CMacro::Save( HINSTANCE hInstance, CTextOutputStream& out ) const
{
	WCHAR			szFuncName[1024];
	WCHAR			szFuncNameJapanese[500];
	int				nTextLen;
	const WCHAR*	pText;
	CNativeW		cmemWork;

	/* 2002.2.2 YAZAKI CSMacroMgrに頼む */
	if (CSMacroMgr::GetFuncInfoByID( hInstance, m_nFuncID, szFuncName, szFuncNameJapanese)){
		switch ( m_nFuncID ){
		case F_INSTEXT_W:
		case F_FILEOPEN:
		case F_EXECEXTMACRO:
			//	引数ひとつ分だけ保存
			pText = m_pParamTop->m_pData;
			nTextLen = wcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace( LTEXT("\\"), LTEXT("\\\\") );
			cmemWork.Replace( LTEXT("\'"), LTEXT("\\\'") );
			out.WriteF(
				LTEXT("S_%ls(\'%ls\');\t// %ls\r\n"),
				szFuncName,
				cmemWork.GetStringPtr(),
				szFuncNameJapanese
			);
			break;
		case F_JUMP:		//	指定行へジャンプ（ただしPL/SQLコンパイルエラー行へのジャンプは未対応）
			out.WriteF(
				LTEXT("S_%ls(%d, %d);\t// %ls\r\n"),
				szFuncName,
				(m_pParamTop->m_pData ? _wtoi(m_pParamTop->m_pData) : 1),
				m_pParamTop->m_pNext->m_pData ? _wtoi(m_pParamTop->m_pNext->m_pData) : 0,
				szFuncNameJapanese
			);
			break;
		case F_BOOKMARK_PATTERN:	//2002.02.08 hor
		case F_SEARCH_NEXT:
		case F_SEARCH_PREV:
			pText = m_pParamTop->m_pData;
			nTextLen = wcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace( LTEXT("\\"), LTEXT("\\\\") );
			cmemWork.Replace( LTEXT("\'"), LTEXT("\\\'") );
			out.WriteF( L"S_%ls(\'", szFuncName );
			out.WriteString( cmemWork.GetStringPtr(), cmemWork.GetStringLength() );
			out.WriteF( L"', %d);\t// %ls\r\n",
				m_pParamTop->m_pNext->m_pData ? _wtoi(m_pParamTop->m_pNext->m_pData) : 0,
				szFuncNameJapanese
			);
			break;
		case F_EXECMD:
			//	引数ひとつ分だけ保存
			pText = m_pParamTop->m_pData;
			nTextLen = wcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace( LTEXT("\\"), LTEXT("\\\\") );
			cmemWork.Replace( LTEXT("\'"), LTEXT("\\\'") );
			out.WriteF(
				LTEXT("S_%ls(\'%ls\', %d);\t// %ls\r\n"),
				szFuncName,
				cmemWork.GetStringPtr(),
				m_pParamTop->m_pNext->m_pData ? _wtoi(m_pParamTop->m_pNext->m_pData) : 0,
				szFuncNameJapanese
			);
			break;
		case F_REPLACE:
		case F_REPLACE_ALL:
			pText = m_pParamTop->m_pData;
			nTextLen = wcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace( LTEXT("\\"), LTEXT("\\\\") );
			cmemWork.Replace( LTEXT("\'"), LTEXT("\\\'") );
			{
				CNativeW cmemWork2(m_pParamTop->m_pNext->m_pData);
				cmemWork2.Replace( LTEXT("\\"), LTEXT("\\\\") );
				cmemWork2.Replace( LTEXT("\'"), LTEXT("\\\'") );
				out.WriteF( L"S_%ls(\'", szFuncName );
				out.WriteString( cmemWork.GetStringPtr(), cmemWork.GetStringLength() );
				out.WriteF( L"\', \'" );
				out.WriteString( cmemWork2.GetStringPtr(), cmemWork2.GetStringLength() );
				out.WriteF( L"\', %d);\t// %ls\r\n",
					m_pParamTop->m_pNext->m_pNext->m_pData ? _wtoi(m_pParamTop->m_pNext->m_pNext->m_pData) : 0,
					szFuncNameJapanese
				);
			}
			break;
		case F_GREP:
			pText = m_pParamTop->m_pData;
			nTextLen = wcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace( LTEXT("\\"), LTEXT("\\\\") );
			cmemWork.Replace( LTEXT("\'"), LTEXT("\\\'") );
			{
				CNativeW cmemWork2(m_pParamTop->m_pNext->m_pData);
				cmemWork2.Replace( LTEXT("\\"), LTEXT("\\\\") );
				cmemWork2.Replace( LTEXT("\'"), LTEXT("\\\'") );

				CNativeW cmemWork3(m_pParamTop->m_pNext->m_pNext->m_pData);
				cmemWork3.Replace( LTEXT("\\"), LTEXT("\\\\") );
				cmemWork3.Replace( LTEXT("\'"), LTEXT("\\\'") );
				out.WriteF( L"S_%ls(\'", szFuncName );
				out.WriteString( cmemWork.GetStringPtr(), cmemWork.GetStringLength() );
				out.WriteF(
					L"\', \'%ls\', \'%ls\', %d);\t// %ls\r\n",
					cmemWork2.GetStringPtr(),
					cmemWork3.GetStringPtr(),
					m_pParamTop->m_pNext->m_pNext->m_pNext->m_pData ? _wtoi(m_pParamTop->m_pNext->m_pNext->m_pNext->m_pData) : 0,
					szFuncNameJapanese
				);
			}
			break;
		default:
			if( 0 == m_pParamTop ){
				out.WriteF( LTEXT("S_%ls();\t// %ls\r\n"), szFuncName, szFuncNameJapanese );
			}else{
				out.WriteF( LTEXT("S_%ls(%d);\t// %ls\r\n"), szFuncName, m_pParamTop->m_pData ? _wtoi(m_pParamTop->m_pData) : 0, szFuncNameJapanese );
			}
			break;
		}
		return;
	}
	out.WriteF( LTEXT("CMacro::GetFuncInfoByID()に、バグがあるのでエラーが出ましたぁぁぁぁぁぁあああ\r\n") );
}

/**	マクロ引数変換

	MacroコマンドをpcEditView->GetCommander().HandleCommandに引き渡す．
	引数がないマクロを除き，マクロとHandleCommandでの対応をここで定義する必要がある．

	@param pcEditView	[in]	操作対象EditView
	@param Index	[in] 下位16bit: 機能ID, 上位ワードはそのままCMacro::HandleCommand()に渡す．
	@param Argument [in] 引数
	@param ArgSize	[in] 引数の数
	
	@date 2007.07.08 genta Indexのコマンド番号を下位ワードに制限
*/
void CMacro::HandleCommand(
	CEditView*			pcEditView,
	const EFunctionCode	Index,
	const WCHAR*		Argument[],
	const int			ArgSize
)
{
	const TCHAR EXEC_ERROR_TITLE[] = _T("Macro実行エラー");

	switch ( LOWORD(Index) ) 
	{
	case F_WCHAR:		//	文字入力。数値は文字コード
	case F_IME_CHAR:	//	日本語入力
		//	Jun. 16, 2002 genta
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				_T("挿入すべき文字コードが指定されていません．")
			);
			break;
		}
	case F_PASTE:	// 2011.06.26 Moca
	case F_PASTEBOX:	// 2011.06.26 Moca
	case F_TEXTWRAPMETHOD:	//	テキストの折り返し方法の指定。数値は、0x0（折り返さない）、0x1（指定桁で折り返す）、0x2（右端で折り返す）	// 2008.05.30 nasukoji
	case F_GOLINETOP:	//	行頭に移動。数値は、0x0（デフォルト）、0x1（空白を無視して先頭に移動）、0x2（未定義）、0x4（選択して移動）、0x8（改行単位で先頭に移動）
	case F_GOLINETOP_SEL:
	case F_GOLOGICALLINETOP_BOX:
	case F_GOLINETOP_BOX:
	case F_GOLINEEND:	//	行末に移動
	case F_GOLINEEND_SEL:
	case F_GOLINEEND_BOX:
	case F_SELECT_COUNT_MODE:	//	文字カウントの方法を指定。数値は、0x0（変更せず取得のみ）、0x1（文字数）、0x2（バイト数）、0x3（文字数⇔バイト数トグル）	// 2009.07.06 syat
	case F_OUTLINE:	//	アウトライン解析のアクションを指定。数値は、0x0（画面表示）、0x1（画面表示＆再解析）、0x2（画面表示トグル）
		//	一つ目の引数が数値。
		pcEditView->GetCommander().HandleCommand( Index, false, (Argument[0] != NULL ? _wtoi(Argument[0]) : 0 ), 0, 0, 0 );
		break;
	case F_CHGMOD_EOL:	//	入力改行コード指定。EEolTypeの数値を指定。2003.06.23 Moca
		//	Jun. 16, 2002 genta
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				_T("入力改行コードが指定されていません．")
			);
			break;
		}
		{
			// マクロ引数値をEOLタイプ値に変換する	// 2009.08.18 ryoji
			int nEol;
			switch( Argument[0] != NULL ? _wtoi(Argument[0]) : 0 ){
			case 1:		nEol = EOL_CRLF; break;
//			case 2:		nEol = EOL_LFCR; break;
			case 3:		nEol = EOL_LF; break;
			case 4:		nEol = EOL_CR; break;
			default:	nEol = EOL_NONE; break;
			}
			pcEditView->GetCommander().HandleCommand( Index, false, nEol, 0, 0, 0 );
		}
		break;
	case F_INSTEXT_W:		//	テキスト挿入
	case F_SET_QUOTESTRING:	// Jan. 29, 2005 genta 追加 テキスト引数1つを取るマクロはここに統合していこう．
		{
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				_T("引数(文字列)が指定されていません．")
			);
			break;
		}
		{
			pcEditView->GetCommander().HandleCommand( Index, false, (LPARAM)Argument[0], 0, 0, 0 );	//	標準
		}
		}
		break;
	case F_ADDTAIL_W:		//	この操作はキーボード操作では存在しないので保存することができない？
	case F_INSBOXTEXT:
		//	一つ目の引数が文字列。
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				_T("引数(文字列)が指定されていません．")
			);
			break;
		}
		{
			int len = wcslen(Argument[0]);
			pcEditView->GetCommander().HandleCommand( Index, false, (LPARAM)Argument[0], len, 0, 0 );	//	標準
		}
		break;
	/* 一つ目、二つ目とも引数は数値 */
	case F_CHG_CHARSET:
		{
			int		nCharSet = ( Argument[0] == NULL || Argument[0][0] == '\0' ) ? CODE_NONE : _wtoi(Argument[0]);
			BOOL	bBOM = ( Argument[1] == NULL ) ? FALSE : (_wtoi(Argument[1]) != 0);
			pcEditView->GetCommander().HandleCommand( Index, false, (LPARAM)nCharSet, (LPARAM)bBOM, 0, 0 );
		}
		break;
	case F_JUMP:		//	指定行へジャンプ（ただしPL/SQLコンパイルエラー行へのジャンプは未対応）
		//	Argument[0]へジャンプ。オプションはArgument[1]に。
		//		******** 以下「行番号の単位」 ********
		//		0x00	折り返し単位の行番号
		//		0x01	改行単位の行番号
		//		**************************************
		//		0x02	PL/SQLコンパイルエラー行を処理する
		//		未定義	テキストの□行目をブロックの1行目とする
		//		未定義	検出されたPL/SQLパッケージのブロックから選択
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				_T("ジャンプ先行番号が指定されていません．")
			);
			break;
		}
		{
			pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgJump.m_nLineNum = _wtoi(Argument[0]);	//ジャンプ先
			LPARAM lFlag = Argument[1] != NULL ? _wtoi(Argument[1]) : 1; // デフォルト1
			GetDllShareData().m_bLineNumIsCRLF_ForJump = ((lFlag & 0x01)!=0);
			pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgJump.m_bPLSQL = lFlag & 0x02 ? 1 : 0;
			pcEditView->GetCommander().HandleCommand( Index, false, 0, 0, 0, 0 );	//	標準
		}
		break;
	/*	一つ目の引数は文字列、二つ目の引数は数値	*/
	case F_BOOKMARK_PATTERN:	//2002.02.08 hor
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				_T("マーク行のパターンが指定されていません．")
			);
			break;
		}
		/* NO BREAK */
	case F_SEARCH_NEXT:
	case F_SEARCH_PREV:
		//	Argument[0]を検索。オプションはArgument[1]に。
		//	Argument[1]:
		//		0x01	単語単位で探す
		//		0x02	英大文字と小文字を区別する
		//		0x04	正規表現
		//		0x08	見つからないときにメッセージを表示
		//		0x10	検索ダイアログを自動的に閉じる
		//		0x20	先頭（末尾）から再検索する
		//		0x800	(マクロ専用)検索キーを履歴に登録しない
		{
			LPARAM lFlag = Argument[1] != NULL ? _wtoi(Argument[1]) : 0;
			SSearchOption sSearchOption;
			sSearchOption.bWordOnly			= (0 != (lFlag & 0x01));
			sSearchOption.bLoHiCase			= (0 != (lFlag & 0x02));
			sSearchOption.bRegularExp		= (0 != (lFlag & 0x04));
			bool bAddHistory = (0 == (lFlag & 0x800));
			int nLen = wcslen( Argument[0] );
			if( 0 < nLen ){
				/* 正規表現 */
				if( lFlag & 0x04
					&& !CheckRegexpSyntax( Argument[0], NULL, true )
				)
				{
					break;
				}

				/* 検索文字列 */
				if( nLen < _MAX_PATH && bAddHistory ){
					CSearchKeywordManager().AddToSearchKeyArr( Argument[0] );
					GetDllShareData().m_Common.m_sSearch.m_sSearchOption = sSearchOption;
				}
				pcEditView->m_strCurSearchKey = Argument[0];
				pcEditView->m_sCurSearchOption = sSearchOption;
				pcEditView->m_bCurSearchUpdate = true;
				pcEditView->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;
			}
			//	設定値バックアップ
			//	マクロパラメータ→設定値変換
			GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND	= lFlag & 0x08 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind	= lFlag & 0x10 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bSearchAll			= lFlag & 0x20 ? 1 : 0;

			//	コマンド発行
			pcEditView->GetCommander().HandleCommand( Index, false, 0, 0, 0, 0);
		}
		break;
	case F_DIFF:
		//	Argument[0]とDiff差分表示。オプションはArgument[1]に。
		//	Argument[1]:
		//		次の数値の和。
		//		0x0001 -i ignore-case         大文字小文字同一視
		//		0x0002 -w ignore-all-space    空白無視
		//		0x0004 -b ignore-space-change 空白変更無視
		//		0x0008 -B ignore-blank-lines  空行無視
		//		0x0010 -t expand-tabs         TAB-SPACE変換
		//		0x0020    (編集中のファイルが旧ファイル)
		//		0x0040    (DIFF差分がないときにメッセージ表示)
		/* NO BREAK */

	case F_EXECMD:
		//	Argument[0]を実行。オプションはArgument[1]に。
		//	Argument[1]:
		//		次の数値の和。
		//		0x01	標準出力を得る
		//		0x02	標準出力をキャレット位置に	//	2007.01.02 maru 引数の拡張
		//		0x04	編集中ファイルを標準入力へ	//	2007.01.02 maru 引数の拡張
		/* NO BREAK */

	case F_TRACEOUT:		// 2006.05.01 マクロ用アウトプットウインドウに出力
		//	Argument[0]を出力。オプションはArgument[1]に。
		//	Argument[1]:
		//		次の数値の和。
		//		0x01	ExpandParameterによる文字列展開を行う
		//		0x02	テキスト末尾に改行コードを付加しない
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				_T(	"引数(文字列)が指定されていません．" )
			);
			break;
		}
		{
			pcEditView->GetCommander().HandleCommand( Index, false, (LPARAM)Argument[0], (LPARAM)(Argument[1] != NULL ? _wtoi(Argument[1]) : 0 ), 0, 0);
		}
		break;

	/* はじめの引数は文字列。２つ目と３つ目は数値 */
	case F_PUTFILE:		// 2006.12.10 作業中ファイルの一時出力
		//	Argument[0]に出力。Argument[1]に文字コード。オプションはArgument[2]に。
		//	Argument[2]:
		//		次の値の和
		//		0x01	選択範囲を出力（非選択状態なら空ファイルを生成）
		// no break

	case F_INSFILE:		// 2006.12.10 キャレット位置にファイル挿入
		//	Argument[0]に出力。Argument[1]に文字コード。オプションはArgument[2]に。
		//	Argument[2]:
		//		現在は特になし
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T(	"ファイル名が指定されていません．" ));
			break;
		}
		{
			pcEditView->GetCommander().HandleCommand(
				Index,
				false,
				(LPARAM)Argument[0], 
				(LPARAM)(Argument[1] != NULL ? _wtoi(Argument[1]) : 0 ),
				(LPARAM)(Argument[2] != NULL ? _wtoi(Argument[2]) : 0 ),
				0
			);
		}
		break;

	/* はじめの2つの引数は文字列。3つ目は数値 */
	case F_REPLACE:
	case F_REPLACE_ALL:
		//	Argument[0]を、Argument[1]に置換。オプションはArgument[2]に（入れる予定）
		//	Argument[2]:
		//		次の数値の和。
		//		0x001	単語単位で探す
		//		0x002	英大文字と小文字を区別する
		//		0x004	正規表現
		//		0x008	見つからないときにメッセージを表示
		//		0x010	検索ダイアログを自動的に閉じる
		//		0x020	先頭（末尾）から再検索する
		//		0x040	クリップボードから貼り付ける
		//		******** 以下「置換範囲」 ********
		//		0x000	ファイル全体
		//		0x080	選択範囲
		//		**********************************
		//		******** 以下「置換対象」 ********
		//		0x000	見つかった文字列と置換
		//		0x100	見つかった文字列の前に挿入
		//		0x200	見つかった文字列の後に追加
		//		**********************************
		//		0x400	「すべて置換」は置換の繰返し（ON:連続置換, OFF:一括置換）
		//		0x800	(マクロ専用)検索キーを履歴に登録しない
		if( Argument[0] == NULL || Argument[0] == L'\0' ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("置換元パターンが指定されていません．"));
			break;
		}
		if( Argument[1] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("置換先パターンが指定されていません．"));
			break;
		}
		{
			CDlgReplace& cDlgReplace = pcEditView->m_pcEditDoc->m_pcEditWnd->m_cDlgReplace;
			LPARAM lFlag = Argument[2] != NULL ? _wtoi(Argument[2]) : 0;
			SSearchOption sSearchOption;
			sSearchOption.bWordOnly			= (0 != (lFlag & 0x01));
			sSearchOption.bLoHiCase			= (0 != (lFlag & 0x02));
			sSearchOption.bRegularExp		= (0 != (lFlag & 0x04));
			bool bAddHistory = (0 == (lFlag & 0x800));
			/* 正規表現 */
			if( lFlag & 0x04
				&& !CheckRegexpSyntax( Argument[0], NULL, true )
			)
			{
				break;
			}

			/* 検索文字列 */
			if( wcslen(Argument[0]) < _MAX_PATH && bAddHistory ){
				CSearchKeywordManager().AddToSearchKeyArr( Argument[0] );
				GetDllShareData().m_Common.m_sSearch.m_sSearchOption = sSearchOption;
			}
			pcEditView->m_strCurSearchKey = Argument[0];
			pcEditView->m_sCurSearchOption = sSearchOption;
			pcEditView->m_bCurSearchUpdate = true;
			pcEditView->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;

			/* 置換後文字列 */
			if( wcslen(Argument[1]) < _MAX_PATH && bAddHistory ){
				CSearchKeywordManager().AddToReplaceKeyArr( Argument[1] );
			}
			cDlgReplace.m_strText2 = Argument[1];

			GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND	= lFlag & 0x08 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bAutoCloseDlgFind	= lFlag & 0x10 ? 1 : 0;
			GetDllShareData().m_Common.m_sSearch.m_bSearchAll			= lFlag & 0x20 ? 1 : 0;
			cDlgReplace.m_nPaste			= lFlag & 0x40 ? 1 : 0;	//	CShareDataに入れなくていいの？
			cDlgReplace.m_bConsecutiveAll = lFlag & 0x0400 ? 1 : 0;	// 2007.01.16 ryoji
			if (LOWORD(Index) == F_REPLACE) {	// 2007.07.08 genta コマンドは下位ワード
				//	置換する時は選べない
				cDlgReplace.m_bSelectedArea = 0;
			}
			else if (LOWORD(Index) == F_REPLACE_ALL) {	// 2007.07.08 genta コマンドは下位ワード
				//	全置換の時は選べる？
				cDlgReplace.m_bSelectedArea	= lFlag & 0x80 ? 1 : 0;
			}
			cDlgReplace.m_nReplaceTarget	= (lFlag >> 8) & 0x03;	//	8bitシフト（0x100で割り算）	// 2007.01.16 ryoji 下位 2bitだけ取り出す
			if( bAddHistory ){
				GetDllShareData().m_Common.m_sSearch.m_bConsecutiveAll = cDlgReplace.m_bConsecutiveAll;
				GetDllShareData().m_Common.m_sSearch.m_bSelectedArea = cDlgReplace.m_bSelectedArea;
			}
			//	コマンド発行
			pcEditView->GetCommander().HandleCommand( Index, false, 0, 0, 0, 0);
		}
		break;
	case F_GREP:
		//	Argument[0]	検索文字列
		//	Argument[1]	検索対象にするファイル名
		//	Argument[2]	検索対象にするフォルダ名
		//	Argument[3]:
		//		次の数値の和。
		//		0x01	サブフォルダからも検索する
		//		0x02	この編集中のテキストから検索する（未実装）
		//		0x04	英大文字と英小文字を区別する
		//		0x08	正規表現
		//		0x10	文字コード自動判別
		//		******** 以下「結果出力」 ********
		//		0x00	該当行
		//		0x20	該当部分
		//		**********************************
		//		******** 以下「出力形式」 ********
		//		0x00	ノーマル
		//		0x40	ファイル毎
		//		**********************************
		//		0x0100 ～ 0xff00	文字コードセット番号 * 0x100
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("GREPパターンが指定されていません．"));
			break;
		}
		if( Argument[1] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("ファイル種別が指定されていません．"));
			break;
		}
		if( Argument[2] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("検索先フォルダが指定されていません．"));
			break;
		}
		{
			//	常に外部ウィンドウに。
			/*======= Grepの実行 =============*/
			/* Grep結果ウィンドウの表示 */
			CNativeW cmWork1;	cmWork1.SetString( Argument[0] );	cmWork1.Replace( L"\"", L"\"\"" );	//	検索文字列
			CNativeT cmWork2;	cmWork2.SetStringW( Argument[1] );	cmWork2.Replace( _T("\""), _T("\"\"") );	//	ファイル名
			CNativeT cmWork3;	cmWork3.SetStringW( Argument[2] );	cmWork3.Replace( _T("\""), _T("\"\"") );	//	フォルダ名

			LPARAM lFlag = Argument[3] != NULL ? _wtoi(Argument[3]) : 5;

			// 2002/09/21 Moca 文字コードセット
			ECodeType	nCharSet;
			{
				nCharSet = CODE_SJIS;
				if( lFlag & 0x10 ){	// 文字コード自動判別(下位互換用)
					nCharSet = CODE_AUTODETECT;
				}
				int nCode = (lFlag >> 8) & 0xff; // 下から 7-15 ビット目(0開始)を使う
				if( IsValidCodeTypeExceptSJIS(nCode) || CODE_AUTODETECT == nCode ){
					nCharSet = (ECodeType)nCode;
				}
			}

			// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
			CNativeT cCmdLine;
			TCHAR	szTemp[20];
			TCHAR	pOpt[64];
			cCmdLine.AppendString(_T("-GREPMODE -GKEY=\""));
			cCmdLine.AppendStringW(cmWork1.GetStringPtr());
			cCmdLine.AppendString(_T("\" -GFILE=\""));
			cCmdLine.AppendString(cmWork2.GetStringPtr());
			cCmdLine.AppendString(_T("\" -GFOLDER=\""));
			cCmdLine.AppendString(cmWork3.GetStringPtr());
			cCmdLine.AppendString(_T("\" -GCODE="));
			auto_sprintf( szTemp, _T("%d"), nCharSet );
			cCmdLine.AppendString(szTemp);

			//GOPTオプション
			pOpt[0] = '\0';
			if( lFlag & 0x01 )_tcscat( pOpt, _T("S") );	/* サブフォルダからも検索する */
			if( lFlag & 0x04 )_tcscat( pOpt, _T("L") );	/* 英大文字と英小文字を区別する */
			if( lFlag & 0x08 )_tcscat( pOpt, _T("R") );	/* 正規表現 */
			if( lFlag & 0x20 )_tcscat( pOpt, _T("P") );	/* 行を出力するか該当部分だけ出力するか */
			if( lFlag & 0x40 )_tcscat( pOpt, _T("2") );	/* Grep: 出力形式 */
			else _tcscat( pOpt, _T("1") );
			if( pOpt[0] != _T('\0') ){
				auto_sprintf( szTemp, _T(" -GOPT=%ts"), pOpt );
				cCmdLine.AppendString(szTemp);
			}

			/* 新規編集ウィンドウの追加 ver 0 */
			SLoadInfo sLoadInfo;
			sLoadInfo.cFilePath = _T("");
			sLoadInfo.eCharCode = CODE_NONE;
			sLoadInfo.bViewMode = false;
			CControlTray::OpenNewEditor(
				G_AppInstance(),
				pcEditView->GetHwnd(),
				sLoadInfo,
				cCmdLine.GetStringPtr()
			);
			/*======= Grepの実行 =============*/
			/* Grep結果ウィンドウの表示 */
		}
		break;
	case F_FILEOPEN:
		//	Argument[0]を開く。
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("読み込みファイル名が指定されていません．"));
			break;
		}
		{
			pcEditView->GetCommander().HandleCommand( Index, false, (LPARAM)Argument[0], 0, 0, 0);
		}
		break;
	case F_FILESAVEAS:
		//	Argument[0]を別名で保存。
		if( Argument[0] == NULL ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("保存ファイル名が指定されていません．"));
			break;
		}
		{
			// 文字コードセット
			//	Sep. 11, 2004 genta 文字コード設定の範囲チェック
			ECodeType nCharCode = CODE_AUTODETECT;	//デフォルト値
			if (Argument[1] != NULL){
				nCharCode = (ECodeType)_wtoi( Argument[1] );
			}
			if (IsValidCodeType(nCharCode) && nCharCode != pcEditView->m_pcEditDoc->GetDocumentEncoding()) {
				//	From Here Jul. 26, 2003 ryoji BOM状態を初期化
				pcEditView->m_pcEditDoc->SetDocumentEncoding(nCharCode, CCodeTypeName(pcEditView->m_pcEditDoc->GetDocumentEncoding()).IsBomDefOn() );
				//	To Here Jul. 26, 2003 ryoji BOM状態を初期化
			}

			// 改行コード
			int nSaveLineCode = 0;	//デフォルト値	//Sep. 11, 2004 genta 初期値を「変更しない」に
			if (Argument[2] != NULL){
				nSaveLineCode = _wtoi( Argument[2] );
			}
			EEolType eEol;
			switch (nSaveLineCode){
			case 0:		eEol = EOL_NONE;	break;
			case 1:		eEol = EOL_CRLF;	break;
			case 2:		eEol = EOL_LF;		break;
			case 3:		eEol = EOL_CR;		break;
			default:	eEol = EOL_NONE;	break;
			}
			
			pcEditView->GetCommander().HandleCommand( Index, false, (LPARAM)Argument[0], 0, (LPARAM)eEol, 0);
		}
		break;
	/* 2つの引数が文字列 */
	// Jul. 5, 2002 genta
	case F_EXTHTMLHELP:
	case F_EXECEXTMACRO:				// 2009.06.14 syat
		pcEditView->GetCommander().HandleCommand( Index, false, (LPARAM)Argument[0], (LPARAM)Argument[1], 0, 0);
		break;
	//	From Here Dec. 4, 2002 genta
	case F_FILE_REOPEN				://開き直す
	case F_FILE_REOPEN_SJIS			://SJISで開き直す
	case F_FILE_REOPEN_JIS			://JISで開き直す
	case F_FILE_REOPEN_EUC			://EUCで開き直す
	case F_FILE_REOPEN_UNICODE		://Unicodeで開き直す
	case F_FILE_REOPEN_UNICODEBE	://UnicodeBEで開き直す
	case F_FILE_REOPEN_UTF8			://UTF-8で開き直す
	case F_FILE_REOPEN_UTF7			://UTF-7で開き直す
		{
			int noconfirm = 0;
			if (Argument[0] != NULL){
				noconfirm = ( _wtoi( Argument[0] ) != 0 );
			}
			pcEditView->GetCommander().HandleCommand( Index, false, noconfirm, 0, 0, 0 );
		}
		break;
	//	To Here Dec. 4, 2002 genta
	case F_TOPMOST:
		{
			int lparam1;
			if( Argument[0] != NULL ){
				lparam1 = _wtoi( Argument[0] );
				pcEditView->GetCommander().HandleCommand( Index, false, lparam1, 0, 0, 0 );
			}
		}
		break;	//	Jan. 29, 2005 genta 抜けていた
	case F_TAGJUMP_KEYWORD:	// @@ 2005.03.31 MIK
		{
			//引数はNULLでもOK
			pcEditView->GetCommander().HandleCommand( Index, false, (LPARAM)Argument[0], 0, 0, 0);
		}
		break;
	case F_NEXTWINDOW:
	case F_PREVWINDOW:
		pcEditView->GetDocument()->HandleCommand( Index );	// 2009.04.11 ryoji F_NEXTWINDOW/F_PREVWINDOWが動作しなかったのを修正
		break;
	case F_MOVECURSORLAYOUT:
	case F_MOVECURSOR:
		{
			if( Argument[0] != NULL && Argument[1] != NULL && Argument[2] != NULL ){
				int lparam1 = _wtoi( Argument[0] ) - 1;
				int lparam2 = _wtoi( Argument[1] ) - 1;
				int lparam3 = _wtoi( Argument[2] );
				pcEditView->GetCommander().HandleCommand( Index, false, lparam1, lparam2, lparam3, 0);
			}else{
				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				_T("数値を指定してください．"));
			}
		}
		break;
	case F_CHGTABWIDTH:		//  タブサイズを取得、設定する（キーマクロでは取得は無意味）
	case F_CHGWRAPCOLM:		//  折り返し桁を取得、設定する（キーマクロでは取得は無意味）
		{
			VARIANT vArg[1];			// HandleFunctionに渡す引数
			VARIANT vResult;			// HandleFunctionから返る値
			//	一つ目の引数が数値。
			vArg[0].vt = VT_I4;
			vArg[0].intVal = (Argument[0] != NULL ? _wtoi(Argument[0]) : 0 );
			HandleFunction( pcEditView, Index, vArg, 1, vResult );
		}
		break;
	case F_SETFONTSIZE:
		{
			int val0 = Argument[0] != NULL ? _wtoi(Argument[0]) : 0;
			int val1 = Argument[1] != NULL ? _wtoi(Argument[1]) : 0;
			pcEditView->GetCommander().HandleCommand( Index, false, (LPARAM)val0, (LPARAM)val1, 0, 0 );
		}
		break;
	default:
		//	引数なし。
		pcEditView->GetCommander().HandleCommand( Index, false, 0, 0, 0, 0 );	//	標準
		break;
	}
}


inline bool VariantToBStr(Variant& varCopy, const VARIANT& arg)
{
	return VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(arg) ), 0, VT_BSTR) == S_OK;
}

inline bool VariantToI4(Variant& varCopy, const VARIANT& arg)
{
	return VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(arg) ), 0, VT_I4) == S_OK;
}

/**	値を返す関数を処理する

	@param View      [in] 対象となるView
	@param ID        [in] 下位16bit: 関数番号
	@param Arguments [in] 引数の配列
	@param ArgSize   [in] 引数の数(Argument)
	@param Result  [out] 結果の値を返す場所。戻り値がfalseのときは不定。
	
	@return true: 成功, false: 失敗

	@author 鬼
	@date 2003.02.21 鬼
	@date 2003.06.01 Moca 関数追加
	@date 2005.08.05 maru,zenryaku 関数追加
	@date 2005.11.29 FILE VariantChangeType対応
*/
bool CMacro::HandleFunction(CEditView *View, EFunctionCode ID, const VARIANT *Arguments, int ArgSize, VARIANT &Result)
{
	Variant varCopy;	// VT_BYREFだと困るのでコピー用

	//2003-02-21 鬼
	switch(LOWORD(ID))
	{
	case F_GETFILENAME:
		{
			const TCHAR* FileName = View->m_pcEditDoc->m_cDocFile.GetFilePath();
			SysString S(FileName, _tcslen(FileName));
			Wrap(&Result)->Receive(S);
		}
		return true;
	case F_GETSAVEFILENAME:
		//	2006.09.04 ryoji 保存時のファイルのパス
		{
			const TCHAR* FileName = View->m_pcEditDoc->m_cDocFile.GetSaveFilePath();
			SysString S(FileName, lstrlen(FileName));
			Wrap(&Result)->Receive(S);
		}
		return true;
	case F_GETSELECTED:
		{
			if(View->GetSelectionInfo().IsTextSelected())
			{
				CNativeW cMem;
				if(!View->GetSelectedData(&cMem, FALSE, NULL, FALSE, false)) return false;
				SysString S(cMem.GetStringPtr(), cMem.GetStringLength());
				Wrap(&Result)->Receive(S);
			}
			else
			{
				Result.vt = VT_BSTR;
				Result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_EXPANDPARAMETER:
		// 2003.02.24 Moca
		{
			if(ArgSize != 1) return false;
			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
			//void ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen);
			//pszSourceを展開して、pszBufferにコピー
			wchar_t *Source;
			int SourceLength;
			Wrap(&varCopy.Data.bstrVal)->GetW(&Source, &SourceLength);
			wchar_t Buffer[2048];
			CSakuraEnvironment::ExpandParameter(Source, Buffer, 2047);
			delete[] Source;
			SysString S(Buffer, wcslen(Buffer));
			Wrap(&Result)->Receive(S);
		}
		return true;
	case F_GETLINESTR:
		//	2003.06.01 Moca マクロ追加
		{
			if( ArgSize != 1 ) return false;
			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
			if( -1 < varCopy.Data.lVal ){
				const wchar_t *Buffer;
				CLogicInt nLength;
				CLogicInt nLine;
				if( 0 == varCopy.Data.lVal ){
					nLine = View->GetCaret().GetCaretLogicPos().GetY2();
				}else{
					nLine = CLogicInt(varCopy.Data.lVal - 1);
				}
				Buffer = View->m_pcEditDoc->m_cDocLineMgr.GetLine(nLine)->GetDocLineStrWithEOL(&nLength);
				if( Buffer != NULL ){
					SysString S( Buffer, nLength );
					Wrap( &Result )->Receive( S );
				}else{
					Result.vt = VT_BSTR;
					Result.bstrVal = SysAllocString(L"");
				}
			}else{
				return false;
			}
		}
		return true;
	case F_GETLINECOUNT:
		//	2003.06.01 Moca マクロ追加
		{
			if( ArgSize != 1 ) return false;
			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
			if( 0 == varCopy.Data.lVal ){
				int nLineCount;
				nLineCount = View->m_pcEditDoc->m_cDocLineMgr.GetLineCount();
				Wrap( &Result )->Receive( nLineCount );
			}else{
				return false;
			}
		}
		return true;
	case F_CHGTABWIDTH:
		//	2004.03.16 zenryaku マクロ追加
		{
			if( ArgSize != 1 ) return false;
			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
			int nTab = (Int)View->m_pcEditDoc->m_cLayoutMgr.GetTabSpace();
			Wrap( &Result )->Receive( nTab );
			// 2013.04.30 Moca 条件追加。不要な場合はChangeLayoutParamを呼ばない
			if( 0 < varCopy.Data.iVal && nTab != varCopy.Data.iVal ){
				View->m_pcEditDoc->m_pcEditWnd->ChangeLayoutParam(
					false, 
					CLayoutInt(varCopy.Data.iVal),
					View->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas()
				);

				// 2009.08.28 nasukoji	「折り返さない」選択時にTAB幅が変更されたらテキスト最大幅の再算出が必要
				if( View->m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
					// 最大幅の再算出時に各行のレイアウト長の計算も行う
					View->m_pcEditDoc->m_cLayoutMgr.CalculateTextWidth();
					View->m_pcEditDoc->m_pcEditWnd->RedrawAllViews( NULL );		// スクロールバーの更新が必要なので再表示を実行する
				}
			}
		}
		return true;
	case F_ISTEXTSELECTED:
		//	2005.07.30 maru マクロ追加
		{
			if(View->GetSelectionInfo().IsTextSelected()) {
				if(View->GetSelectionInfo().IsBoxSelecting()) {
					Wrap( &Result )->Receive( 2 );	//矩形選択中
				} else {
					Wrap( &Result )->Receive( 1 );	//選択中
				}
			} else {
				Wrap( &Result )->Receive( 0 );		//非選択中
			}
		}
		return true;
	case F_GETSELLINEFROM:
		//	2005.07.30 maru マクロ追加
		{
			Wrap( &Result )->Receive( (Int)View->GetSelectionInfo().m_sSelect.GetFrom().y + 1 );
		}
		return true;
	case F_GETSELCOLMFROM:
		//	2005.07.30 maru マクロ追加
		{
			Wrap( &Result )->Receive( (Int)View->GetSelectionInfo().m_sSelect.GetFrom().x + 1 );
		}
		return true;
	case F_GETSELLINETO:
		//	2005.07.30 maru マクロ追加
		{
			Wrap( &Result )->Receive( (Int)View->GetSelectionInfo().m_sSelect.GetTo().y + 1 );
		}
		return true;
	case F_GETSELCOLMTO:
		//	2005.07.30 maru マクロ追加
		{
			Wrap( &Result )->Receive( (Int)View->GetSelectionInfo().m_sSelect.GetTo().x + 1);
		}
		return true;
	case F_ISINSMODE:
		//	2005.07.30 maru マクロ追加
		{
			Wrap( &Result )->Receive(View->IsInsMode() /* Oct. 2, 2005 genta */);
		}
		return true;
	case F_GETCHARCODE:
		//	2005.07.31 maru マクロ追加
		{
			Wrap( &Result )->Receive(View->m_pcEditDoc->GetDocumentEncoding());
		}
		return true;
	case F_GETLINECODE:
		//	2005.08.04 maru マクロ追加
		{
			switch( View->m_pcEditDoc->m_cDocEditor.GetNewLineCode() ){
			case EOL_CRLF:
				Wrap( &Result )->Receive( 0 );
				break;
			case EOL_CR:
				Wrap( &Result )->Receive( 1 );
				break;
			case EOL_LF:
				Wrap( &Result )->Receive( 2 );
				break;
			}
		}
		return true;
	case F_ISPOSSIBLEUNDO:
		//	2005.08.04 maru マクロ追加
		{
			Wrap( &Result )->Receive( View->m_pcEditDoc->m_cDocEditor.IsEnableUndo() );
		}
		return true;
	case F_ISPOSSIBLEREDO:
		//	2005.08.04 maru マクロ追加
		{
			Wrap( &Result )->Receive( View->m_pcEditDoc->m_cDocEditor.IsEnableRedo() );
		}
		return true;
	case F_CHGWRAPCOLM:
		//	2008.06.19 ryoji マクロ追加
		{
			if( ArgSize != 1 ) return false;
			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
			Wrap( &Result )->Receive( (Int)View->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() );
			if( varCopy.Data.iVal < MINLINEKETAS || varCopy.Data.iVal > MAXLINEKETAS )
				return true;
			View->m_pcEditDoc->m_nTextWrapMethodCur = WRAP_SETTING_WIDTH;
			View->m_pcEditDoc->m_bTextWrapMethodCurTemp = !( View->m_pcEditDoc->m_nTextWrapMethodCur == View->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod );
			View->m_pcEditDoc->m_pcEditWnd->ChangeLayoutParam(
				false, 
				View->m_pcEditDoc->m_cLayoutMgr.GetTabSpace(),
				CLayoutInt(varCopy.Data.iVal)
			);
		}
		return true;
	case F_ISCURTYPEEXT:
		//	2006.09.04 ryoji 指定した拡張子が現在のタイプ別設定に含まれているかどうかを調べる
		{
			if( ArgSize != 1 ) return false;

			TCHAR *Source;
			int SourceLength;

			int nType1 = View->m_pcEditDoc->m_cDocType.GetDocumentType().GetIndex();	// 現在のタイプ

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			int nType2 = CDocTypeManager().GetDocumentTypeOfExt(Source).GetIndex();	// 指定拡張子のタイプ
			delete[] Source;

			Wrap( &Result )->Receive( (nType1 == nType2)? 1: 0 );	// タイプ別設定の一致／不一致
		}
		return true;
	case F_ISSAMETYPEEXT:
		//	2006.09.04 ryoji ２つの拡張子が同じタイプ別設定に含まれているかどうかを調べる
		{
			if( ArgSize != 2 ) return false;

			TCHAR *Source;
			int SourceLength;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			int nType1 = CDocTypeManager().GetDocumentTypeOfExt(Source).GetIndex();	// 拡張子１のタイプ
			delete[] Source;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			int nType2 = CDocTypeManager().GetDocumentTypeOfExt(Source).GetIndex();	// 拡張子２のタイプ
			delete[] Source;

			Wrap( &Result )->Receive( (nType1 == nType2)? 1: 0 );	// タイプ別設定の一致／不一致
		}
		return true;
	case F_INPUTBOX:
		//	2011.03.18 syat テキスト入力ダイアログの表示
		{
			if( ArgSize < 1 ) return false;
			TCHAR *Source;
			int SourceLength;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			std::tstring sMessage = Source;	// 表示メッセージ
			delete[] Source;

			std::tstring sDefaultValue = _T("");
			if( ArgSize >= 2 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
				Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
				sDefaultValue = Source;	// デフォルト値
				delete[] Source;
			}

			int nMaxLen = _MAX_PATH;
			if( ArgSize >= 3 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[2]) ), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
				nMaxLen = varCopy.Data.intVal;	// 最大入力長
				if( nMaxLen <= 0 ){
					nMaxLen = _MAX_PATH;
				}
			}

			TCHAR *Buffer = new TCHAR[ nMaxLen+1 ];
			_tcscpy( Buffer, sDefaultValue.c_str() );
			CDlgInput1 cDlgInput1;
			if( cDlgInput1.DoModal( G_AppInstance(), View->GetHwnd(), _T("sakura macro"), sMessage.c_str(), nMaxLen, Buffer ) ) {
				SysString S( Buffer, _tcslen(Buffer) );
				Wrap( &Result )->Receive( S );
			}else{
				Result.vt = VT_BSTR;
				Result.bstrVal = SysAllocString(L"");
			}
			delete[] Buffer;
		}
		return true;
	case F_MESSAGEBOX:	// メッセージボックスの表示
	case F_ERRORMSG:	// メッセージボックス（エラー）の表示
	case F_WARNMSG:		// メッセージボックス（警告）の表示
	case F_INFOMSG:		// メッセージボックス（情報）の表示
	case F_OKCANCELBOX:	// メッセージボックス（確認：OK／キャンセル）の表示
	case F_YESNOBOX:	// メッセージボックス（確認：はい／いいえ）の表示
		//	2011.03.18 syat メッセージボックスの表示
		{
			if( ArgSize < 1 ) return false;
			TCHAR *Source;
			int SourceLength;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			std::tstring sMessage = Source;	// 表示文字列
			delete[] Source;

			UINT uType = 0;		// メッセージボックス種別
			switch( LOWORD(ID) ) {
			case F_MESSAGEBOX:
				if( ArgSize >= 2 ){
					if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
					uType = varCopy.Data.uintVal;
				}else{
					uType = MB_OK;
				}
				break;
			case F_ERRORMSG:
				uType |= MB_OK | MB_ICONSTOP;
				break;
			case F_WARNMSG:
				uType |= MB_OK | MB_ICONEXCLAMATION;
				break;
			case F_INFOMSG:
				uType |= MB_OK | MB_ICONINFORMATION;
				break;
			case F_OKCANCELBOX:
				uType |= MB_OKCANCEL | MB_ICONQUESTION;
				break;
			case F_YESNOBOX:
				uType |= MB_YESNO | MB_ICONQUESTION;
				break;
			}
			int ret = ::MessageBox( View->GetHwnd(), sMessage.c_str(), _T("sakura macro"), uType );
			Wrap( &Result )->Receive( ret );
		}
		return true;
	case F_COMPAREVERSION:
		//	2011.03.18 syat バージョン番号の比較
		{
			if( ArgSize != 2 ) return false;
			TCHAR *Source;
			int SourceLength;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			std::tstring sVerA = Source;	// バージョンA
			delete[] Source;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
			Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
			std::tstring sVerB = Source;	// バージョンB
			delete[] Source;

			Wrap( &Result )->Receive( CompareVersion( sVerA.c_str(), sVerB.c_str() ) );
		}
		return true;
	case F_MACROSLEEP:
		//	2011.03.18 syat 指定した時間（ミリ秒）停止する
		{
			if( ArgSize != 1 ) return false;

			if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_UI4) != S_OK) return false;	// VT_UI4として解釈
			CWaitCursor cWaitCursor( View->GetHwnd() );	// カーソルを砂時計にする
			::Sleep( varCopy.Data.uintVal );
			Wrap( &Result )->Receive( 0 );	//戻り値は今のところ0固定
		}
		return true;
	case F_FILEOPENDIALOG:
	case F_FILESAVEDIALOG:
		//	2011.03.18 syat ファイルダイアログの表示
		{
			TCHAR *Source;
			int SourceLength;
			std::tstring sDefault;
			std::tstring sFilter;

			if( ArgSize >= 1 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
				Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
				sDefault = Source;	// 既定のファイル名
				delete[] Source;
			}

			if( ArgSize >= 2 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
				Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
				sFilter = Source;	// フィルタ文字列
				delete[] Source;
			}

			CDlgOpenFile cDlgOpenFile;
			cDlgOpenFile.Create(
				G_AppInstance(), View->GetHwnd(),
				sFilter.c_str(),
				sDefault.c_str()
			);
			bool bRet;
			TCHAR szPath[ _MAX_PATH ];
			_tcscpy( szPath, sDefault.c_str() );
			if( LOWORD(ID) == F_FILEOPENDIALOG ){
				bRet = cDlgOpenFile.DoModal_GetOpenFileName( szPath );
			}else{
				bRet = cDlgOpenFile.DoModal_GetSaveFileName( szPath );
			}
			if( bRet ){
				SysString S( szPath, _tcslen(szPath) );
				Wrap( &Result )->Receive( S );
			}else{
				Result.vt = VT_BSTR;
				Result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_FOLDERDIALOG:
		//	2011.03.18 syat フォルダダイアログの表示
		{
			TCHAR *Source;
			int SourceLength;
			std::tstring sMessage;
			std::tstring sDefault;

			if( ArgSize >= 1 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
				Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
				sMessage = Source;	// 表示メッセージ
				delete[] Source;
			}

			if( ArgSize >= 2 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
				Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
				sDefault = Source;	// 既定のファイル名
				delete[] Source;
			}

			TCHAR szPath[ _MAX_PATH ];
			int nRet = SelectDir( View->GetHwnd(), sMessage.c_str(), sDefault.c_str(), szPath );
			if( nRet == IDOK ){
				SysString S( szPath, _tcslen(szPath) );
				Wrap( &Result )->Receive( S );
			}else{
				Result.vt = VT_BSTR;
				Result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_GETCLIPBOARD:
		//	2011.03.18 syat クリップボードの文字列を取得
		{
			int nOpt = 0;

			if( ArgSize >= 1 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
				nOpt = varCopy.Data.intVal;	// オプション
			}

			CNativeW memBuff;
			bool bColumnSelect = false;
			bool bLineSelect = false;
			bool bRet = View->MyGetClipboardData( memBuff, &bColumnSelect, &bLineSelect );
			if( bRet ){
				SysString S( memBuff.GetStringPtr(), memBuff.GetStringLength() );
				Wrap( &Result )->Receive( S );
			}else{
				Result.vt = VT_BSTR;
				Result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_SETCLIPBOARD:
		//	2011.03.18 syat クリップボードに文字列を設定
		{
			TCHAR *Source;
			int SourceLength;
			std::tstring sValue;
			int nOpt = 0;

			if( ArgSize >= 1 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
				nOpt = varCopy.Data.intVal;	// オプション
			}

			if( ArgSize >= 2 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
				Wrap(&varCopy.Data.bstrVal)->GetT(&Source, &SourceLength);
				sValue = Source;	// 設定する文字列
				delete[] Source;
			}

			bool bColumnSelect = false;
			bool bLineSelect = false;
			bool bRet = View->MySetClipboardData( sValue.c_str(), sValue.size(), bColumnSelect, bLineSelect );
			Wrap( &Result )->Receive( bRet );
		}
		return true;

	case F_LAYOUTTOLOGICLINENUM:
		// レイアウト→ロジック行
		{
			if( ArgSize < 1 ){
				return false;
			}
			if( !VariantToI4(varCopy, Arguments[0]) ){
				return false;
			}
			CLayoutInt nLineNum = CLayoutInt(varCopy.Data.iVal - 1);
			int ret = 0;
			if( View->m_pcEditDoc->m_cLayoutMgr.GetLineCount() == nLineNum ){
				ret = (Int)View->m_pcEditDoc->m_cDocLineMgr.GetLineCount() + 1;
			}else{
				const CLayout* pcLayout = View->m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY(nLineNum);
				if( pcLayout != NULL ){
					ret = pcLayout->GetLogicLineNo() + 1;
				}
			}
			Wrap(&Result)->Receive(ret);
		}
		return true;

	case F_LINECOLUMNTOINDEX:
		// レイアウト→ロジック桁
		{
			if( ArgSize < 2 ){
				return false;
			}
			if( !VariantToI4(varCopy, Arguments[0]) ){
				return false;
			}
			CLayoutInt nLineNum = CLayoutInt(varCopy.Data.iVal - 1);
			if( !VariantToI4(varCopy, Arguments[1]) ){
				return false;
			}
			CLayoutInt nLineCol = CLayoutInt(varCopy.Data.iVal - 1);

			CLayoutPoint nLayoutPos(nLineCol, nLineNum);
			CLogicPoint nLogicPos( CLogicInt(0), CLogicInt(0) );
			View->m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(nLayoutPos, &nLogicPos);
			int ret = nLogicPos.GetX() + 1;
			Wrap(&Result)->Receive(ret);
		}
		return true;

	case F_LOGICTOLAYOUTLINENUM:
	case F_LINEINDEXTOCOLUMN:
		// ロジック→レイアウト行/桁
		{
			if( ArgSize < 2 ){
				return false;
			}
			if( !VariantToI4(varCopy, Arguments[0]) ){
				return false;
			}
			CLogicInt nLineNum = CLogicInt(varCopy.Data.iVal - 1);
			if( !VariantToI4(varCopy, Arguments[1]) ){
				return false;
			}
			CLogicInt nLineIdx = CLogicInt(varCopy.Data.iVal - 1);

			CLogicPoint nLogicPos(nLineIdx, nLineNum);
			CLayoutPoint nLayoutPos(CLayoutInt(0),CLayoutInt(0));
			View->m_pcEditDoc->m_cLayoutMgr.LogicToLayout(nLogicPos, &nLayoutPos);
			int ret = ((LOWORD(ID) == F_LOGICTOLAYOUTLINENUM) ? (Int)nLayoutPos.GetY2() : (Int)nLayoutPos.GetX2()) + 1;
			Wrap(&Result)->Receive(ret);
		}
		return true;

	case F_GETCOOKIE:
		{
			Variant varCopy2;
			if( ArgSize >= 2 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;
				if(VariantChangeType(&varCopy2.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;
				SysString ret = View->GetDocument()->m_cCookie.GetCookie(varCopy.Data.bstrVal, varCopy2.Data.bstrVal);
				Wrap( &Result )->Receive( ret );
				return true;
			}
			return false;
		}
	case F_GETCOOKIEDEFAULT:
		{
			Variant varCopy2, varCopy3;
			if( ArgSize >= 3 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;
				if(VariantChangeType(&varCopy2.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;
				if(VariantChangeType(&varCopy3.Data, const_cast<VARIANTARG*>( &(Arguments[2]) ), 0, VT_BSTR) != S_OK) return false;
				SysString ret = View->GetDocument()->m_cCookie.GetCookieDefault(varCopy.Data.bstrVal, varCopy2.Data.bstrVal,
					varCopy3.Data.bstrVal, SysStringLen(varCopy3.Data.bstrVal) );
				Wrap( &Result )->Receive( ret );
				return true;
			}
			return false;
		}
	case F_SETCOOKIE:
		{
			Variant varCopy2, varCopy3;
			if( ArgSize >= 3 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;
				if(VariantChangeType(&varCopy2.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;
				if(VariantChangeType(&varCopy3.Data, const_cast<VARIANTARG*>( &(Arguments[2]) ), 0, VT_BSTR) != S_OK) return false;
				int ret = View->GetDocument()->m_cCookie.SetCookie(varCopy.Data.bstrVal, varCopy2.Data.bstrVal,
					varCopy3.Data.bstrVal, SysStringLen(varCopy3.Data.bstrVal) );
				Wrap( &Result )->Receive( ret );
				return true;
			}
			return false;
		}
	case F_DELETECOOKIE:
		{
			Variant varCopy2;
			if( ArgSize >= 2 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;
				if(VariantChangeType(&varCopy2.Data, const_cast<VARIANTARG*>( &(Arguments[1]) ), 0, VT_BSTR) != S_OK) return false;
				int ret = View->GetDocument()->m_cCookie.DeleteCookie(varCopy.Data.bstrVal, varCopy2.Data.bstrVal);
				Wrap( &Result )->Receive( ret );
				return true;
			}
			return false;
		}
	case F_GETCOOKIENAMES:
		{
			Variant varCopy2, varCopy3;
			if( ArgSize >= 1 ){
				if(VariantChangeType(&varCopy.Data, const_cast<VARIANTARG*>( &(Arguments[0]) ), 0, VT_BSTR) != S_OK) return false;
				SysString ret = View->GetDocument()->m_cCookie.GetCookieNames(varCopy.Data.bstrVal);
				Wrap( &Result )->Receive( ret );
				return true;
			}
			return false;
		}
	default:
		return false;
	}
}


