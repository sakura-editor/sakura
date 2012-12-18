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
	Copyright (C) 2007, ryoji, maru, genta
	Copyright (C) 2008, nasukoji, ryoji, syat
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "Funccode.h"
#include "CMacro.h"
#include "CEditApp.h"
#include "CEditView.h" //2002/2/10 aroka
#include "CSMacroMgr.h" //2002/2/10 aroka
#include "etc_uty.h" //2002/2/10 aroka
#include "CEditDoc.h"	//	2002/5/13 YAZAKI ヘッダ整理
#include "Debug.h"
#include "OleTypes.h" //2003-02-21 鬼

CMacro::CMacro( int nFuncID )
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
		delete del_p->m_pData;
		delete del_p;
	}
	return;
}

/*	引数の型振り分け
	機能IDによって、期待する型は異なります。
	そこで、引数の型を機能IDによって振り分けて、AddParamしましょう。
	たとえば、F_INSTEXTの1つめ、2つめの引数は文字列、3つめの引数はintだったりするのも、ここでうまく振り分けられることを期待しています。

	lParamは、HandleCommandのparamに値を渡しているコマンドの場合にのみ使います。
*/
void CMacro::AddLParam( LPARAM lParam, const CEditView* pcEditView )
{
	switch( m_nFuncID ){
	/*	文字列パラメータを追加 */
	case F_INSTEXT:
	case F_FILEOPEN:
	case F_EXECMD:
	case F_EXECEXTMACRO:
		{
			AddStringParam( (const char *)lParam );	//	lParamを追加。
			LPARAM lFlag = 0x00;
			lFlag = pcEditView->m_pShareData->m_nExecFlgOpt;
			AddIntParam( lFlag );
		}
		break;

	case F_JUMP:	//	指定行へジャンプ（ただしPL/SQLコンパイルエラー行へのジャンプは未対応）
		{
			AddIntParam( pcEditView->m_pcEditDoc->m_cDlgJump.m_nLineNum );
			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_pShareData->m_bLineNumIsCRLF		? 0x01 : 0x00;
			lFlag |= pcEditView->m_pcEditDoc->m_cDlgJump.m_bPLSQL	? 0x02 : 0x00;
			AddIntParam( lFlag );
		}
		break;

	case F_BOOKMARK_PATTERN:	//2002.02.08 hor
	case F_SEARCH_NEXT:
	case F_SEARCH_PREV:
		{
			AddStringParam( pcEditView->m_pShareData->m_szSEARCHKEYArr[0] );	//	lParamを追加。

			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bWordOnly		? 0x01 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bLoHiCase		? 0x02 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bRegularExp		? 0x04 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND				? 0x08 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind				? 0x10 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_bSearchAll					? 0x20 : 0x00;
			AddIntParam( lFlag );
		}
		break;
	case F_REPLACE:
	case F_REPLACE_ALL:
		{
			AddStringParam( pcEditView->m_pShareData->m_szSEARCHKEYArr[0] );	//	lParamを追加。
			AddStringParam( pcEditView->m_pShareData->m_szREPLACEKEYArr[0] );	//	lParamを追加。

			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bWordOnly		? 0x01 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bLoHiCase		? 0x02 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bRegularExp		? 0x04 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND				? 0x08 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind				? 0x10 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_bSearchAll					? 0x20 : 0x00;
			lFlag |= pcEditView->m_pcEditDoc->m_cDlgReplace.m_nPaste					? 0x40 : 0x00;	//	CShareDataに入れなくていいの？
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_bSelectedArea					? 0x80 : 0x00;	//	置換する時は選べない
			lFlag |= pcEditView->m_pcEditDoc->m_cDlgReplace.m_nReplaceTarget << 8;	//	8bitシフト（0x100で掛け算）
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_bConsecutiveAll	? 0x0400: 0x00;	// 2007.01.16 ryoji
			AddIntParam( lFlag );
		}
		break;
	case F_GREP:
		{
			AddStringParam( pcEditView->m_pShareData->m_szSEARCHKEYArr[0] );	//	lParamを追加。
			AddStringParam( pcEditView->m_pShareData->m_szGREPFILEArr[0] );	//	lParamを追加。
			AddStringParam( pcEditView->m_pShareData->m_szGREPFOLDERArr[0] );	//	lParamを追加。

			LPARAM lFlag = 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_bGrepSubFolder				? 0x01 : 0x00;
			//			この編集中のテキストから検索する(0x02.未実装)
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bLoHiCase		? 0x04 : 0x00;
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bRegularExp		? 0x08 : 0x00;
			lFlag |= (pcEditView->m_pShareData->m_Common.m_sSearch.m_nGrepCharSet == CODE_AUTODETECT) ? 0x10 : 0x00;	//	2002/09/21 Moca 下位互換性のための処理
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_bGrepOutputLine				? 0x20 : 0x00;
			lFlag |= (pcEditView->m_pShareData->m_Common.m_sSearch.m_nGrepOutputStyle == 2)		? 0x40 : 0x00;	//	CShareDataに入れなくていいの？
			lFlag |= pcEditView->m_pShareData->m_Common.m_sSearch.m_nGrepCharSet << 8;
			AddIntParam( lFlag );
		}
		break;
	/*	数値パラメータを追加 */
	case F_CHAR:
		AddIntParam( lParam );
		break;

	/*	標準もパラメータを追加 */
	default:
		AddIntParam( lParam );
		break;
	}
}

/*	引数に文字列を追加。
*/
void CMacro::AddStringParam( const char* szParam )
{
	CMacroParam* param = new CMacroParam;
	param->m_pNext = NULL;

	//	必要な領域を確保してコピー。
	int nLen = _tcslen( szParam );
	param->m_pData = new char[nLen + 1];
	memcpy((char*)param->m_pData, szParam, nLen );
	param->m_pData[nLen] = _T('\0');

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
	param->m_pData = new char[16];	//	数値格納（最大16桁）用
	itoa(nParam, param->m_pData, 10);

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

/**	コマンドを実行する（pcEditView->HandleCommandを発行する）
	m_nFuncIDによって、引数の型を正確に渡してあげましょう。
	
	@note
	paramArrは何かのポインタ（アドレス）をLONGであらわした値になります。
	引数がchar*のときは、paramArr[i]をそのままHandleCommandに渡してかまいません。
	引数がintのときは、*((int*)paramArr[i])として渡しましょう。
	
	たとえば、F_INSTEXTの1つめ、2つめの引数は文字列、3つめの引数はint、4つめの引数が無し。だったりする場合は、次のようにしましょう。
	pcEditView->HandleCommand( m_nFuncID, TRUE, paramArr[0], paramArr[1], *((int*)paramArr[2]), 0);
	
	@date 2007.07.20 genta : flags追加．FA_FROMMACROはflagsに含めて渡すものとする．
		(1コマンド発行毎に毎回演算する必要はないので)
*/
void CMacro::Exec( CEditView* pcEditView, int flags ) const
{
	const char* paramArr[4] = {NULL, NULL, NULL, NULL};	//	4つに限定。
	
	CMacroParam* p = m_pParamTop;
	int i = 0;
	for (i = 0; i < 4; i++) {
		if (!p) break;	//	pが無ければbreak;
		paramArr[i] = p->m_pData;
		p = p->m_pNext;
	}
	CMacro::HandleCommand(pcEditView, m_nFuncID | flags, paramArr, i);
}

/*	CMacroを再現するための情報をhFileに書き出します。

	InsText("なんとか");
	のように。
*/
void CMacro::Save( HINSTANCE hInstance, HFILE hFile ) const
{
	TCHAR			szFuncName[1024];
	TCHAR			szFuncNameJapanese[500];
	int				nTextLen;
	const char*		pText;
	char			szLine[1024];
	CMemory			cmemWork;

	/* 2002.2.2 YAZAKI CSMacroMgrに頼む */
	if (CSMacroMgr::GetFuncInfoByID( hInstance, m_nFuncID, szFuncName, szFuncNameJapanese)){
		switch ( m_nFuncID ){
		case F_INSTEXT:
		case F_FILEOPEN:
		case F_EXECEXTMACRO:
			//	引数ひとつ分だけ保存
			pText = m_pParamTop->m_pData;
			nTextLen = _tcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace_j( _T("\\"), _T("\\\\") );
			cmemWork.Replace_j( _T("\'"), _T("\\\'") );
			wsprintf(
				szLine,
				_T("S_%s(\'%s\');\t// %s\r\n"),
				szFuncName,
				cmemWork.GetStringPtr(),
				szFuncNameJapanese
			);
			_lwrite( hFile, szLine, _tcslen( szLine ) );
			break;
		case F_JUMP:		//	指定行へジャンプ（ただしPL/SQLコンパイルエラー行へのジャンプは未対応）
			wsprintf(
				szLine,
				_T("S_%s(%d, %d);\t// %s\r\n"),
				szFuncName,
				(m_pParamTop->m_pData ? atoi(m_pParamTop->m_pData) : 1),
				m_pParamTop->m_pNext->m_pData ? atoi(m_pParamTop->m_pNext->m_pData) : 0,
				szFuncNameJapanese
			);
			_lwrite( hFile, szLine, _tcslen( szLine ) );
			break;
		case F_BOOKMARK_PATTERN:	//2002.02.08 hor
		case F_SEARCH_NEXT:
		case F_SEARCH_PREV:
			pText = m_pParamTop->m_pData;
			nTextLen = _tcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace_j( _T("\\"), _T("\\\\") );
			cmemWork.Replace_j( _T("\'"), _T("\\\'") );
			wsprintf( szLine, "S_%s(\'%s\', %d);\t// %s\r\n", szFuncName, cmemWork.GetStringPtr(), m_pParamTop->m_pNext->m_pData ? atoi(m_pParamTop->m_pNext->m_pData) : 0, szFuncNameJapanese );
			_lwrite( hFile, szLine, _tcslen( szLine ) );
			break;
		case F_EXECMD:
			//	引数ひとつ分だけ保存
			pText = m_pParamTop->m_pData;
			nTextLen = _tcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace_j( _T("\\"), _T("\\\\") );
			cmemWork.Replace_j( _T("\'"), _T("\\\'") );
			wsprintf( szLine, "S_%s(\'%s\', %d);\t// %s\r\n", szFuncName, cmemWork.GetStringPtr(), m_pParamTop->m_pNext->m_pData ? atoi(m_pParamTop->m_pNext->m_pData) : 0, szFuncNameJapanese );
			_lwrite( hFile, szLine, _tcslen( szLine ) );
			break;
		case F_REPLACE:
		case F_REPLACE_ALL:
			pText = m_pParamTop->m_pData;
			nTextLen = _tcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace_j( _T("\\"), _T("\\\\") );
			cmemWork.Replace_j( _T("\'"), _T("\\\'") );
			{
				CMemory cmemWork2(m_pParamTop->m_pNext->m_pData, _tcslen(m_pParamTop->m_pNext->m_pData));
				cmemWork2.Replace_j( "\\", "\\\\" );
				cmemWork2.Replace_j( "\'", "\\\'" );
				wsprintf(
					szLine,
					_T("S_%s(\'%s\', \'%s\', %d);\t// %s\r\n"),
					szFuncName,
					cmemWork.GetStringPtr(),
					cmemWork2.GetStringPtr(),
					m_pParamTop->m_pNext->m_pNext->m_pData ? atoi(m_pParamTop->m_pNext->m_pNext->m_pData) : 0,
					szFuncNameJapanese
				);
				_lwrite( hFile, szLine, _tcslen( szLine ) );
			}
			break;
		case F_GREP:
			pText = m_pParamTop->m_pData;
			nTextLen = _tcslen(pText);
			cmemWork.SetString( pText, nTextLen );
			cmemWork.Replace_j( _T("\\"), _T("\\\\") );
			cmemWork.Replace_j( _T("\'"), _T("\\\'") );
			{
				CMemory cmemWork2(m_pParamTop->m_pNext->m_pData, _tcslen(m_pParamTop->m_pNext->m_pData));
				cmemWork2.Replace_j( _T("\\"), _T("\\\\") );
				cmemWork2.Replace_j( _T("\'"), _T("\\\'") );

				CMemory cmemWork3(m_pParamTop->m_pNext->m_pNext->m_pData, _tcslen(m_pParamTop->m_pNext->m_pNext->m_pData));
				cmemWork3.Replace_j( _T("\\"), _T("\\\\") );
				cmemWork3.Replace_j( _T("\'"), _T("\\\'") );
				wsprintf(
					szLine,
					_T("S_%s(\'%s\', \'%s\', \'%s\', %d);\t// %s\r\n"),
					szFuncName,
					cmemWork.GetStringPtr(),
					cmemWork2.GetStringPtr(),
					cmemWork3.GetStringPtr(),
					m_pParamTop->m_pNext->m_pNext->m_pNext->m_pData ? atoi(m_pParamTop->m_pNext->m_pNext->m_pNext->m_pData) : 0,
					szFuncNameJapanese
				);
				_lwrite( hFile, szLine, _tcslen( szLine ) );
			}
			break;
		default:
			if( 0 == m_pParamTop ){
				wsprintf( szLine, "S_%s();\t// %s\r\n", szFuncName, szFuncNameJapanese );
			}else{
				wsprintf( szLine, "S_%s(%d);\t// %s\r\n", szFuncName, m_pParamTop->m_pData ? atoi(m_pParamTop->m_pData) : 0, szFuncNameJapanese );
			}
			_lwrite( hFile, szLine, _tcslen( szLine ) );
			break;
		}
		return;
	}
	wsprintf( szLine, _T("CMacro::GetFuncInfoByID()に、バグがあるのでエラーが出ましたぁぁぁぁぁぁあああ\r\n") );
	_lwrite( hFile, szLine, _tcslen( szLine ) );
}

/**	マクロ引数変換

	MacroコマンドをCEditView::HandleCommandに引き渡す．
	引数がないマクロを除き，マクロとHandleCommandでの対応をここで定義する必要がある．

	@param pcEditView	[in]	操作対象EditView
	@param Index	[in] 下位16bit: 機能ID, 上位ワードはそのままCMacro::HandleCommand()に渡す．
	@param Argument [in] 引数
	@param ArgSize	[in] 引数の数
	
	@date 2007.07.08 genta Indexのコマンド番号を下位ワードに制限
*/
void CMacro::HandleCommand(
	CEditView*			pcEditView,
	const int			Index,
	const char*			Argument[],
	const int			ArgSize
)
{
	const TCHAR EXEC_ERROR_TITLE[] = _T("Macro実行エラー");

	switch ( LOWORD(Index) ) 
	{
	case F_CHAR:		//	文字入力。数値は文字コード
	case F_IME_CHAR:	//	日本語入力
	case F_CHGMOD_EOL:	//	入力改行コード指定。EEolTypeの数値を指定。2003.06.23 Moca
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
		/* NO BREAK */
	case F_PASTE:	// 2011.06.26 Moca
	case F_PASTEBOX:	// 2011.06.26 Moca
	case F_TEXTWRAPMETHOD:	//	テキストの折り返し方法の指定。数値は、0x0（折り返さない）、0x1（指定桁で折り返す）、0x2（右端で折り返す）	// 2008.05.30 nasukoji
	case F_GOLINETOP:	//	行頭に移動。数値は、0x0（デフォルト）、0x1（空白を無視して先頭に移動）、0x2（未定義）、0x4（選択して移動）、0x8（改行単位で先頭に移動：未実装）
		//	一つ目の引数が数値。
		pcEditView->HandleCommand( Index, FALSE, (Argument[0] != NULL ? atoi(Argument[0]) : 0 ), 0, 0, 0 );
		break;
	case F_INSTEXT:		//	テキスト挿入
	case F_ADDTAIL:		//	この操作はキーボード操作では存在しないので保存することができない？
	case F_SET_QUOTESTRING:	// Jan. 29, 2005 genta 追加 テキスト引数1つを取るマクロはここに統合していこう．
		//	一つ目の引数が文字列。
		//	ただし2つ目の引数は文字数。
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
			int len = _tcslen(Argument[0]);
			pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], len, 0, 0 );	//	標準
		}
		break;
	/* 一つ目、二つ目とも引数は数値 */
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
			pcEditView->m_pcEditDoc->m_cDlgJump.m_nLineNum = atoi(Argument[0]);	//ジャンプ先
			LPARAM lFlag = Argument[1] != NULL ? atoi(Argument[1]) : 1; // デフォルト1
			pcEditView->m_pShareData->m_bLineNumIsCRLF = lFlag & 0x01 ? 1 : 0;
			pcEditView->m_pcEditDoc->m_cDlgJump.m_bPLSQL = lFlag & 0x02 ? 1 : 0;
			pcEditView->HandleCommand( Index, FALSE, 0, 0, 0, 0 );	//	標準
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
		{
			LPARAM lFlag = Argument[1] != NULL ? atoi(Argument[1]) : 0;
			if( 0 < _tcslen( Argument[0] ) ){
				/* 正規表現 */
				if( lFlag & 0x04
					&& !CheckRegexpSyntax( Argument[0], NULL, true )
				)
				{
					break;
				}

				/* 検索文字列 */
				CShareData::getInstance()->AddToSearchKeyArr( (const char*)Argument[0] );
			}
			//	設定値バックアップ
			//	マクロパラメータ→設定値変換
			pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bWordOnly	= lFlag & 0x01 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bLoHiCase	= lFlag & 0x02 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bRegularExp	= lFlag & 0x04 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND	= lFlag & 0x08 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind	= lFlag & 0x10 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_sSearch.m_bSearchAll			= lFlag & 0x20 ? 1 : 0;

			//	コマンド発行
		//	pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, 0, 0);
			pcEditView->HandleCommand( Index, FALSE, 0, 0, 0, 0);
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
			pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], (LPARAM)(Argument[1] != NULL ? atoi(Argument[1]) : 0 ), 0, 0);
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
			pcEditView->HandleCommand(
				Index,
				FALSE,
				(LPARAM)Argument[0], 
				(LPARAM)(Argument[1] != NULL ? atoi(Argument[1]) : 0 ),
				(LPARAM)(Argument[2] != NULL ? atoi(Argument[2]) : 0 ),
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
		if( Argument[0] == NULL || 0 == _tcslen( Argument[0] ) ){
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
			LPARAM lFlag = Argument[2] != NULL ? atoi(Argument[2]) : 0;
			/* 正規表現 */
			if( lFlag & 0x04
				&& !CheckRegexpSyntax( Argument[0], NULL, true )
			)
			{
				break;
			}

			/* 検索文字列 */
			CShareData::getInstance()->AddToSearchKeyArr( (const char*)Argument[0] );

			/* 検索文字列 */
			CShareData::getInstance()->AddToReplaceKeyArr( (const char*)Argument[1] );

			pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bWordOnly		= lFlag & 0x01 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bLoHiCase		= lFlag & 0x02 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_sSearch.m_sSearchOption.bRegularExp		= lFlag & 0x04 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_sSearch.m_bNOTIFYNOTFOUND	= lFlag & 0x08 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_sSearch.m_bAutoCloseDlgFind	= lFlag & 0x10 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_sSearch.m_bSearchAll			= lFlag & 0x20 ? 1 : 0;
			pcEditView->m_pcEditDoc->m_cDlgReplace.m_nPaste			= lFlag & 0x40 ? 1 : 0;	//	CShareDataに入れなくていいの？
//			pcEditView->m_pShareData->m_Common.m_sSearch.m_bSelectedArea		= 0;	//	lFlag & 0x80 ? 1 : 0;
			pcEditView->m_pShareData->m_Common.m_sSearch.m_bConsecutiveAll	= lFlag & 0x0400 ? 1 : 0;	// 2007.01.16 ryoji
			if (LOWORD(Index) == F_REPLACE) {	// 2007.07.08 genta コマンドは下位ワード
				//	置換する時は選べない
				pcEditView->m_pShareData->m_Common.m_sSearch.m_bSelectedArea	= 0;
			}
			else if (LOWORD(Index) == F_REPLACE_ALL) {	// 2007.07.08 genta コマンドは下位ワード
				//	全置換の時は選べる？
				pcEditView->m_pShareData->m_Common.m_sSearch.m_bSelectedArea	= lFlag & 0x80 ? 1 : 0;
			}
			pcEditView->m_pcEditDoc->m_cDlgReplace.m_nReplaceTarget	= (lFlag >> 8) & 0x03;	//	8bitシフト（0x100で割り算）	// 2007.01.16 ryoji 下位 2bitだけ取り出す
			//	コマンド発行
			pcEditView->HandleCommand( Index, FALSE, 0, 0, 0, 0);
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
			CMemory cmWork1;	cmWork1.SetString( Argument[0] );	cmWork1.Replace_j( _T("\""), _T("\"\"") );	//	検索文字列
			CMemory cmWork2;	cmWork2.SetString( Argument[1] );	cmWork2.Replace_j( _T("\""), _T("\"\"") );	//	ファイル名
			CMemory cmWork3;	cmWork3.SetString( Argument[2] );	cmWork3.Replace_j( _T("\""), _T("\"\"") );	//	フォルダ名

			LPARAM lFlag = Argument[3] != NULL ? atoi(Argument[3]) : 5;

			// 2002/09/21 Moca 文字コードセット
			int		nCharSet;
			{
				nCharSet = CODE_SJIS;
				if( lFlag & 0x10 ){	// 文字コード自動判別(下位互換用)
					nCharSet = CODE_AUTODETECT;
				}
				int nCode = (lFlag >> 8) & 0xff; // 下から 7-15 ビット目(0開始)を使う
				if( IsValidCodeTypeExceptSJIS(nCode) || CODE_AUTODETECT == nCode ){
					nCharSet = nCode;
				}
			}

			// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
			char	pCmdLine[1024];
			TCHAR	pOpt[64];
			wsprintf(
				pCmdLine,
				_T("-GREPMODE -GKEY=\"%s\" -GFILE=\"%s\" -GFOLDER=\"%s\" -GCODE=%d"),
				cmWork1.GetStringPtr(),
				cmWork2.GetStringPtr(),
				cmWork3.GetStringPtr(),
				nCharSet
			);

			pOpt[0] = '\0';
			if( lFlag & 0x01 ) _tcscat( pOpt, _T("S") );	/* サブフォルダからも検索する */
			if( lFlag & 0x04 ) _tcscat( pOpt, _T("L") );	/* 英大文字と英小文字を区別する */
			if( lFlag & 0x08 ) _tcscat( pOpt, _T("R") );	/* 正規表現 */
			if( lFlag & 0x20 ) _tcscat( pOpt, _T("P") );	/* 行を出力するか該当部分だけ出力するか */
			if( lFlag & 0x40 ) _tcscat( pOpt, _T("2") );	/* Grep: 出力形式 */
			else _tcscat( pOpt, _T("1") );
			if( 0 < _tcslen( pOpt ) ){
				_tcscat( pCmdLine, _T(" -GOPT=") );
				_tcscat( pCmdLine, pOpt );
			}

			/* 新規編集ウィンドウの追加 ver 0 */
			CEditApp::OpenNewEditor( pcEditView->m_hInstance, pcEditView->m_hWnd, pCmdLine, CODE_NONE, FALSE );
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
			pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, 0, 0);
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
			/* デフォルト値 */
			//	Sep. 11, 2004 genta 初期値を「変更しない」に
			//	0だとSJIS指定となってしまうため
			ECodeType nCharCode = CODE_AUTODETECT;
			int nSaveLineCode = 0;
			
			if (Argument[1] != NULL){
				nCharCode = (ECodeType)atoi( Argument[1] );
			}
			if (Argument[2] != NULL){
				nSaveLineCode = atoi( Argument[2] );
			}
			// 文字コードセット
			//	Sep. 11, 2004 genta 文字コード設定の範囲チェック
			if(	IsValidCodeType(nCharCode) && nCharCode != pcEditView->m_pcEditDoc->m_nCharCode ){
				pcEditView->m_pcEditDoc->m_nCharCode = nCharCode;
				//	From Here Jul. 26, 2003 ryoji BOM状態を初期化
				switch( pcEditView->m_pcEditDoc->m_nCharCode ){
				case CODE_UNICODE:
				case CODE_UNICODEBE:
					pcEditView->m_pcEditDoc->m_bBomExist = TRUE;
					break;
				case CODE_UTF8:
				default:
					pcEditView->m_pcEditDoc->m_bBomExist = FALSE;
					break;
				}
				//	To Here Jul. 26, 2003 ryoji BOM状態を初期化
			}

			// 改行コード
			switch (nSaveLineCode){
			case 0:		pcEditView->m_pcEditDoc->m_cSaveLineCode = EOL_NONE;	break;
			case 1:		pcEditView->m_pcEditDoc->m_cSaveLineCode = EOL_CRLF;	break;
			case 2:		pcEditView->m_pcEditDoc->m_cSaveLineCode = EOL_LF;		break;
			case 3:		pcEditView->m_pcEditDoc->m_cSaveLineCode = EOL_CR;		break;
			default:	pcEditView->m_pcEditDoc->m_cSaveLineCode = EOL_NONE;	break;
			}
			
			pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, 0, 0);
			pcEditView->m_pcEditDoc->m_cSaveLineCode = EOL_NONE;	// 2008.03.20 ryoji 改行コードは引き継がない（上書き保存では常に「変換なし」になるように）
		}
		break;
	// Jul. 5, 2002 genta
	case F_EXTHTMLHELP:
	case F_EXECEXTMACRO:				// 2008.10.22 syat
		pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], (LPARAM)Argument[1], 0, 0);
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
				noconfirm = ( atoi( Argument[0] ) != 0 );
			}
			pcEditView->HandleCommand( Index, FALSE, noconfirm, 0, 0, 0 );
		}
		break;
	//	To Here Dec. 4, 2002 genta
	case F_TOPMOST:
		{
			int lparam1;
			if( Argument[0] != NULL ){
				lparam1 = atoi( Argument[0] );
				pcEditView->HandleCommand( Index, FALSE, lparam1, 0, 0, 0 );
			}
		}
		break;	//	Jan. 29, 2005 genta 抜けていた
	case F_TAGJUMP_KEYWORD:	// @@ 2005.03.31 MIK
		{
			//引数はNULLでもOK
			pcEditView->HandleCommand( Index, FALSE, (LPARAM)Argument[0], 0, 0, 0);
		}
		break;
	case F_NEXTWINDOW:
	case F_PREVWINDOW:
		pcEditView->m_pcEditDoc->HandleCommand( Index );	// 2009.04.11 ryoji F_NEXTWINDOW/F_PREVWINDOWが動作しなかったのを修正
		break;
	case F_CHGTABWIDTH:		//  タブサイズを取得、設定する（キーマクロでは取得は無意味）
	case F_CHGWRAPCOLM:		//  折り返し桁を取得、設定する（キーマクロでは取得は無意味）
		{
			VARIANT vArg[1];			// HandleFunctionに渡す引数
			VARIANT vResult;			// HandleFunctionから返る値
			//	一つ目の引数が数値。
			vArg[0].vt = VT_I4;
			vArg[0].intVal = (Argument[0] != NULL ? atoi(Argument[0]) : 0 );
			HandleFunction( pcEditView, Index, vArg, 1, vResult );
		}
		break;
	default:
		//	引数なし。
		pcEditView->HandleCommand( Index, FALSE, 0, 0, 0, 0 );	//	標準
		break;
	}
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
bool CMacro::HandleFunction(CEditView *View, int ID, VARIANT *Arguments, int ArgSize, VARIANT &Result)
{
	Variant varCopy;	// VT_BYREFだと困るのでコピー用

	//2003-02-21 鬼
	switch(LOWORD(ID))
	{
	case F_GETFILENAME:
		{
			const TCHAR* FileName = View->m_pcEditDoc->GetFilePath();
			SysString S(FileName, _tcslen(FileName));
			Wrap(&Result)->Receive(S);
		}
		return true;
	case F_GETSAVEFILENAME:
		//	2006.09.04 ryoji 保存時のファイルのパス
		{
			const TCHAR* FileName = View->m_pcEditDoc->GetSaveFilePath();
			SysString S(FileName, _tcslen(FileName));
			Wrap(&Result)->Receive(S);
		}
		return true;
	case F_GETSELECTED:
		{
			if(View->IsTextSelected())
			{
				CMemory cMem;
				if(!View->GetSelectedData(&cMem, FALSE, NULL, FALSE, FALSE)) return false;
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
			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
			//void ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen);
			//pszSourceを展開して、pszBufferにコピー
			char *Source;
			int SourceLength;
			Wrap(&varCopy.Data.bstrVal)->Get(&Source, &SourceLength);
			char Buffer[2048];
			View->m_pcEditDoc->ExpandParameter(Source, Buffer, 2047);
			delete[] Source;
			SysString S(Buffer, _tcslen(Buffer));
			Wrap(&Result)->Receive(S);
		}
		return true;
	case F_GETLINESTR:
		//	2003.06.01 Moca マクロ追加
		{
			if( ArgSize != 1 ) return false;
			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
			if( -1 < varCopy.Data.lVal ){
				const char *Buffer;
				int nLength, nLine;
				if( 0 == varCopy.Data.lVal ){
					nLine = View->m_nCaretPosY_PHY;
				}else{
					nLine = varCopy.Data.lVal - 1;
				}
				Buffer = View->m_pcEditDoc->m_cDocLineMgr.GetLineStr( nLine, &nLength );
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
			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
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
			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
			Wrap( &Result )->Receive( View->m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
			View->m_pcEditDoc->ChangeLayoutParam(
				false, 
				varCopy.Data.iVal,
				View->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas()
			);

			// 2009.08.28 nasukoji	「折り返さない」選択時にTAB幅が変更されたらテキスト最大幅の再算出が必要
			if( View->m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP && varCopy.Data.iVal ){
				// 最大幅の再算出時に各行のレイアウト長の計算も行う
				View->m_pcEditDoc->m_cLayoutMgr.CalculateTextWidth();
				View->m_pcEditDoc->RedrawAllViews( NULL );		// スクロールバーの更新が必要なので再表示を実行する
			}
		}
		return true;
	case F_ISTEXTSELECTED:
		//	2005.07.30 maru マクロ追加
		{
			if(View->IsTextSelected()) {
				if(View->m_bBeginBoxSelect) {
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
			Wrap( &Result )->Receive( View->m_nSelectLineFrom + 1 );
		}
		return true;
	case F_GETSELCOLMFROM:
		//	2005.07.30 maru マクロ追加
		{
			Wrap( &Result )->Receive( View->m_nSelectColmFrom + 1 );
		}
		return true;
	case F_GETSELLINETO:
		//	2005.07.30 maru マクロ追加
		{
			Wrap( &Result )->Receive( View->m_nSelectLineTo + 1 );
		}
		return true;
	case F_GETSELCOLMTO:
		//	2005.07.30 maru マクロ追加
		{
			Wrap( &Result )->Receive( View->m_nSelectColmTo + 1);
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
			Wrap( &Result )->Receive(View->m_pcEditDoc->m_nCharCode);
		}
		return true;
	case F_GETLINECODE:
		//	2005.08.04 maru マクロ追加
		{
			switch( View->m_pcEditDoc->GetNewLineCode() ){
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
			Wrap( &Result )->Receive( View->m_pcEditDoc->IsEnableUndo() );
		}
		return true;
	case F_ISPOSSIBLEREDO:
		//	2005.08.04 maru マクロ追加
		{
			Wrap( &Result )->Receive( View->m_pcEditDoc->IsEnableRedo() );
		}
		return true;
	case F_CHGWRAPCOLM:
		//	2008.06.19 ryoji マクロ追加
		{
			if( ArgSize != 1 ) return false;
			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_I4) != S_OK) return false;	// VT_I4として解釈
			Wrap( &Result )->Receive( View->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() );
			if( varCopy.Data.iVal < MINLINEKETAS || varCopy.Data.iVal > MAXLINEKETAS )
				return true;
			View->m_pcEditDoc->m_nTextWrapMethodCur = WRAP_SETTING_WIDTH;
			View->m_pcEditDoc->m_bTextWrapMethodCurTemp = !( View->m_pcEditDoc->m_nTextWrapMethodCur == View->m_pcEditDoc->GetDocumentAttribute().m_nTextWrapMethod );
			View->m_pcEditDoc->ChangeLayoutParam(
				false, 
				View->m_pcEditDoc->m_cLayoutMgr.GetTabSpace(),
				varCopy.Data.iVal
			);
		}
		return true;
	case F_ISCURTYPEEXT:
		//	2006.09.04 ryoji 指定した拡張子が現在のタイプ別設定に含まれているかどうかを調べる
		{
			if( ArgSize != 1 ) return false;

			TCHAR *Source;
			int SourceLength;

			int nType1 = View->m_pcEditDoc->GetDocumentType();	// 現在のタイプ

			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
			Wrap(&varCopy.Data.bstrVal)->Get(&Source, &SourceLength);
			int nType2 = CShareData::getInstance()->GetDocumentTypeOfExt(Source);	// 指定拡張子のタイプ
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

			if(VariantChangeType(&varCopy.Data, &(Arguments[0]), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
			Wrap(&varCopy.Data.bstrVal)->Get(&Source, &SourceLength);
			int nType1 = CShareData::getInstance()->GetDocumentTypeOfExt(Source);	// 拡張子１のタイプ
			delete[] Source;

			if(VariantChangeType(&varCopy.Data, &(Arguments[1]), 0, VT_BSTR) != S_OK) return false;	// VT_BSTRとして解釈
			Wrap(&varCopy.Data.bstrVal)->Get(&Source, &SourceLength);
			int nType2 = CShareData::getInstance()->GetDocumentTypeOfExt(Source);	// 拡張子２のタイプ
			delete[] Source;

			Wrap( &Result )->Receive( (nType1 == nType2)? 1: 0 );	// タイプ別設定の一致／不一致
		}
		return true;
	default:
		return false;
	}
}

/*[EOF]*/
