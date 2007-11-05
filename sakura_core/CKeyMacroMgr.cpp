/*!	@file
	@brief キーボードマクロ

	@author Norio Nakatani

	@date 20011229 aroka バグ修正、コメント追加
	YAZAKI 組替え
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka
	Copyright (C) 2002, YAZAKI, aroka, genta
	Copyright (C) 2004, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "CKeyMacroMgr.h"
#include "CMacro.h"
#include "CSMacroMgr.h"// 2002/2/10 aroka
#include "debug.h"
#include "charcode.h"
#include "CMemory.h"
#include "CMacroFactory.h"
#include "io/CTextStream.h"

CKeyMacroMgr::CKeyMacroMgr()
{
	m_pTop = NULL;
	m_pBot = NULL;
//	m_nKeyMacroDataArrNum = 0;	2002.2.2 YAZAKI
	//	Apr. 29, 2002 genta
	//	m_nReadyはCMacroManagerBaseへ
	return;
}

CKeyMacroMgr::~CKeyMacroMgr()
{
	/* キーマクロのバッファをクリアする */
	ClearAll();
	return;
}


/*! キーマクロのバッファをクリアする */
void CKeyMacroMgr::ClearAll( void )
{
	CMacro* p = m_pTop;
	CMacro* del_p;
	while (p){
		del_p = p;
		p = p->GetNext();
		delete del_p;
	}
//	m_nKeyMacroDataArrNum = 0;	2002.2.2 YAZAKI
	m_pTop = NULL;
	m_pBot = NULL;
	return;

}

/*! キーマクロのバッファにデータ追加
	機能番号と、引数ひとつを追加版。
	@date 2002.2.2 YAZAKI pcEditViewも渡すようにした。
*/
void CKeyMacroMgr::Append(
	EFunctionCode	nFuncID,
	LPARAM			lParam1,
	CEditView*		pcEditView
)
{
	CMacro* macro = new CMacro( nFuncID );
	macro->AddLParam( lParam1, pcEditView );
	Append(macro);
}

/*! キーマクロのバッファにデータ追加
	CMacroを指定して追加する版
*/
void CKeyMacroMgr::Append( CMacro* macro )
{
	if (m_pTop){
		m_pBot->SetNext(macro);
		m_pBot = macro;
	}
	else {
		m_pTop = macro;
		m_pBot = m_pTop;
	}
//	m_nKeyMacroDataArrNum++;	2002.2.2 YAZAKI
	return;
}



/*! キーボードマクロの保存
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
BOOL CKeyMacroMgr::SaveKeyMacro( HINSTANCE hInstance, const TCHAR* pszPath ) const
{
	CTextOutputStream out(pszPath);
	if(!out){
		return FALSE;
	}

	//最初のコメント
	out.WriteF(L"//キーボードマクロのファイル\n");

	//マクロ内容
	CMacro* p = m_pTop;
	while (p){
		p->Save( hInstance, out );
		p = p->GetNext();
	}

	out.Close();
	return TRUE;
}



/*! キーボードマクロの実行
	CMacroに委譲。
*/
void CKeyMacroMgr::ExecKeyMacro( CEditView* pcEditView ) const
{
	CMacro* p = m_pTop;
	while (p){
		p->Exec(pcEditView);
		p = p->GetNext();
	}
}

/*! キーボードマクロの読み込み
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
BOOL CKeyMacroMgr::LoadKeyMacro( HINSTANCE hInstance, const TCHAR* pszPath )
{
	/* キーマクロのバッファをクリアする */
	ClearAll();

	CTextInputStream in( pszPath );
	if(!in){
		m_nReady = false;
		return FALSE;
	}

	WCHAR	szFuncName[100];
	WCHAR	szFuncNameJapanese[256];
	EFunctionCode	nFuncID;
	int		i;
	int		nBgn, nEnd;
	CMacro* macro = NULL;

	//	Jun. 16, 2002 genta
	m_nReady = true;	//	エラーがあればfalseになる
	static const TCHAR MACRO_ERROR_TITLE[] = _T("Macro読み込みエラー");

	int line = 1;	//	エラー時に行番号を通知するため．1始まり．
	for( ; in.Good() ; ++line ){
		std::wstring szLine = in.ReadLineW();
		using namespace WCODE;

		int nLineLen = auto_strlen( szLine.c_str() );
		// 先行する空白をスキップ
		for( i = 0; i < nLineLen; ++i ){
			if( szLine[i] != SPACE && szLine[i] != TAB ){
				break;
			}
		}
		nBgn = i;
		// コメント行の検出
		//# パフォーマンス：'/'のときだけ２文字目をテスト
		if( szLine[nBgn] == LTEXT('/') && nBgn + 1 < nLineLen && szLine[nBgn + 1] == LTEXT('/') ){
			continue;
		}
		//	Jun. 16, 2002 genta 空行を無視する
		if( szLine[nBgn] == LTEXT('\0') ){
			continue;
		}

		// 関数名の取得
		szFuncName[0]='\0';// 初期化
		for( ; i < nLineLen; ++i ){
			//# バッファオーバーランチェック
			if( szLine[i] == LTEXT('(') && (i - nBgn)< _countof(szFuncName) ){
				auto_memcpy( szFuncName, &szLine[nBgn], i - nBgn );
				szFuncName[i - nBgn] = L'\0';
				++i;
				nBgn = i;
				break;
			}
		}
		// 関数名にS_が付いていたら

		/* 関数名→機能ID，機能名日本語 */
		//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
		nFuncID = CSMacroMgr::GetFuncInfoByName( hInstance, szFuncName, szFuncNameJapanese );
		if( -1 != nFuncID ){
			macro = new CMacro( nFuncID );
			// Jun. 16, 2002 genta プロトタイプチェック用に追加
			int nArgs;
			const MacroFuncInfo* mInfo= CSMacroMgr::GetFuncInfoByID( nFuncID );
			//	Skip Space
			for(nArgs = 0; szLine[i] ; ++nArgs ) {
				// Jun. 16, 2002 genta プロトタイプチェック
				if( nArgs >= _countof( mInfo->m_varArguments ) ){
					::MYMESSAGEBOX(
						NULL,
						MB_OK | MB_ICONSTOP | MB_TOPMOST,
						MACRO_ERROR_TITLE,
						_T("Line %d: Column %d: 引数が多すぎます\n"),
						line,
						i + 1
					);
					m_nReady = false;
				}

				while( szLine[i] == LTEXT(' ') || szLine[i] == LTEXT('\t') )
					i++;

				//@@@ 2002.2.2 YAZAKI PPA.DLLマクロにあわせて仕様変更。文字列は''で囲む。
				//	Jun. 16, 2002 genta double quotationも許容する
				if( LTEXT('\'') == szLine[i] || LTEXT('\"') == szLine[i]  ){	//	'で始まったら文字列だよきっと。
					// Jun. 16, 2002 genta プロトタイプチェック
					// Jun. 27, 2002 genta 余分な引数を無視するよう，VT_EMPTYを許容する．
					if( mInfo->m_varArguments[nArgs] != VT_BSTR && 
						mInfo->m_varArguments[nArgs] != VT_EMPTY ){
						::MYMESSAGEBOX(
							NULL,
							MB_OK | MB_ICONSTOP | MB_TOPMOST,
							MACRO_ERROR_TITLE,
							_T("Line %d: Column %d\r\n")
							_T("関数%lsの%d番目の引数に文字列は置けません．"),
							line,
							i + 1,
							szFuncName,
							nArgs + 1
						);
						m_nReady = false;
						break;
					}
					WCHAR cQuote = szLine[i];
					++i;
					nBgn = i;	//	nBgnは引数の先頭の文字
					//	Jun. 16, 2002 genta
					//	行末の検出のため，ループ回数を1増やした
					for( ; i <= nLineLen; ++i ){		//	最後の文字+1までスキャン
						if( szLine[i] == LTEXT('\\') ){	// エスケープのスキップ
							++i;
							continue;
						}
						if( szLine[i] == cQuote ){	//	始まりと同じquotationで終了。
							nEnd = i;	//	nEndは終わりの次の文字（'）
							break;
						}
						if( szLine[i] == LTEXT('\0') ){	//	行末に来てしまった
							::MYMESSAGEBOX(
								NULL,
								MB_OK | MB_ICONSTOP | MB_TOPMOST,
								MACRO_ERROR_TITLE,
								_T("Line %d:\r\n関数%lsの%d番目の引数の終わりに%mcがありません．"),
								line,
								szFuncName,
								nArgs + 1,
								cQuote
							);
							m_nReady = false;
							nEnd = i - 1;	//	nEndは終わりの次の文字（'）
							break;
						}
					}
					//	Jun. 16, 2002 genta
					if( !m_nReady ){
						break;
					}

					CNativeW2 cmemWork;
					cmemWork.SetString( szLine.c_str() + nBgn, nEnd - nBgn );
					cmemWork.Replace( LTEXT("\\\'"), LTEXT("\'") );

					//	Jun. 16, 2002 genta double quotationもエスケープ解除
					cmemWork.Replace( LTEXT("\\\""), LTEXT("\"") );
					cmemWork.Replace( LTEXT("\\\\"), LTEXT("\\") );
					macro->AddStringParam( cmemWork.GetStringPtr() );	//	引数を文字列として追加
				}
				else if ( is09(szLine[i]) ){	//	数字で始まったら数字列だ。
					// Jun. 16, 2002 genta プロトタイプチェック
					// Jun. 27, 2002 genta 余分な引数を無視するよう，VT_EMPTYを許容する．
					if( mInfo->m_varArguments[nArgs] != VT_I4 && 
						mInfo->m_varArguments[nArgs] != VT_EMPTY){
						::MYMESSAGEBOX(
							NULL,
							MB_OK | MB_ICONSTOP | MB_TOPMOST,
							MACRO_ERROR_TITLE,
							_T("Line %d: Column %d\r\n")
							_T("関数%lsの%d番目の引数に数値は置けません．"),
							line,
							i + 1,
							szFuncName,
							nArgs + 1
						);
						m_nReady = false;
						break;
					}
					nBgn = i;	//	nBgnは引数の先頭の文字
					for( ; i < nLineLen; ++i ){		//	最後の文字までスキャン
						if( is09(szLine[i]) ){	// まだ数値
//							++i;
							continue;
						}
						else {
							nEnd = i;	//	数字の最後の文字
							i--;
							break;
						}
					}

					CNativeW2 cmemWork;
					cmemWork.SetString( szLine.c_str() + nBgn, nEnd - nBgn );
					// Jun. 16, 2002 genta
					//	数字の中にquotationは入っていないよ
					//cmemWork.Replace( L"\\\'", L"\'" );
					//cmemWork.Replace( L"\\\\", L"\\" );
					macro->AddStringParam( cmemWork.GetStringPtr() );	//	引数を文字列として追加
				}
				//	Jun. 16, 2002 genta
				else if( szLine[i] == LTEXT(')') ){
					//	引数無し
					break;
				}
				else {
					//	Parse Error:文法エラーっぽい。
					//	Jun. 16, 2002 genta
					nBgn = nEnd = i;
					::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
						_T("Line %d: Column %d: Syntax Error\n"), line, i );
					m_nReady = false;
					break;
				}

				for( ; i < nLineLen; ++i ){		//	最後の文字までスキャン
					if( szLine[i] == LTEXT(')') || szLine[i] == LTEXT(',') ){	//	,もしくは)を読み飛ばす
						i++;
						break;
					}
				}
				if (szLine[i-1] == LTEXT(')')){
					break;
				}
			}
			//	Jun. 16, 2002 genta
			if( !m_nReady ){
				//	どこかでエラーがあったらしい
				delete macro;
				break;
			}
			/* キーマクロのバッファにデータ追加 */
			Append( macro );
		}
		else {
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
				_T("Line %d: %lsは存在しない関数です．\n"), line, szFuncName );
			//	Jun. 16, 2002 genta
			m_nReady = false;
			break;
		}
	}
	in.Close();

	//	Jun. 16, 2002 genta
	//	マクロ中にエラーがあったら異常終了できるようにする．
	return m_nReady ? TRUE : FALSE;
}

//	From Here Apr. 29, 2002 genta
/*!
	Factory

	@param ext [in] オブジェクト生成の判定に使う拡張子(小文字)

	@date 2004-01-31 genta RegisterExtの廃止のためRegisterCreatorに置き換え
		そのため，過ったオブジェクト生成を行わないために拡張子チェックは必須．
*/
CMacroManagerBase* CKeyMacroMgr::Creator(const TCHAR* ext)
{
	if( _tcscmp( ext, _T("mac") ) == 0 ){
		return new CKeyMacroMgr;
	}
	return NULL;
}

/*!	CKeyMacroManagerの登録

	@date 2004.01.31 genta RegisterExtの廃止のためRegisterCreatorに置き換え
*/
void CKeyMacroMgr::declare (void)
{
	//	常に実行
	CMacroFactory::Instance()->RegisterCreator( Creator );
}
//	To Here Apr. 29, 2002 genta

/*[EOF]*/
