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

#include "StdAfx.h"
#include <stdio.h>
#include <string.h>
#include "CKeyMacroMgr.h"
#include "CMacro.h"
#include "macro/CSMacroMgr.h"// 2002/2/10 aroka
#include "charset/charcode.h"
#include "mem/CMemory.h"
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
	const LPARAM*	lParams,
	CEditView*		pcEditView
)
{
	CMacro* macro = new CMacro( nFuncID );
	macro->AddLParam( lParams, pcEditView );
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
	out.WriteF(LSW(STR_ERR_DLGKEYMACMGR1));

	//マクロ内容
	CMacro* p = m_pTop;
	while (p){
		p->Save( hInstance, out );
		p = p->GetNext();
	}

	out.Close();
	return TRUE;
}



/** キーボードマクロの実行
	CMacroに委譲。
	
	@date 2007.07.20 genta flags追加．CMacro::Exec()に
		FA_FROMMACROを含めた値を渡す．
*/
bool CKeyMacroMgr::ExecKeyMacro( CEditView* pcEditView, int flags ) const
{
	CMacro* p = m_pTop;
	int macroflag = flags | FA_FROMMACRO;
	bool bRet = true;
	while (p){
		if( !p->Exec(pcEditView, macroflag) ){
			bRet = false;
			break;
		}
		p = p->GetNext();
	}
	return bRet;
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
	std::tstring MACRO_ERROR_TITLE_string = LS(STR_ERR_DLGKEYMACMGR2);
	const TCHAR* MACRO_ERROR_TITLE = MACRO_ERROR_TITLE_string.c_str();

	int line = 1;	//	エラー時に行番号を通知するため．1始まり．
	for( ; in.Good() ; ++line ){
		std::wstring strLine = in.ReadLineW();
		const WCHAR* szLine = strLine.c_str(); // '\0'終端文字列を取得
		using namespace WCODE;

		int nLineLen = strLine.length();
		// 先行する空白をスキップ
		for( i = 0; i < nLineLen; ++i ){
			if( szLine[i] != SPACE && szLine[i] != TAB ){
				break;
			}
		}
		nBgn = i;
		//	Jun. 16, 2002 genta 空行を無視する
		if( nBgn == nLineLen || szLine[nBgn] == LTEXT('\0') ){
			continue;
		}
		// コメント行の検出
		//# パフォーマンス：'/'のときだけ２文字目をテスト
		if( szLine[nBgn] == LTEXT('/') && nBgn + 1 < nLineLen && szLine[nBgn + 1] == LTEXT('/') ){
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
			int nArgSizeMax = _countof( mInfo->m_varArguments );
			if( mInfo->m_pData  ){
				nArgSizeMax = mInfo->m_pData->m_nArgMaxSize;
			}
			for(nArgs = 0; szLine[i] ; ++nArgs ) {
				// Jun. 16, 2002 genta プロトタイプチェック
				if( nArgs >= nArgSizeMax ){
					::MYMESSAGEBOX(
						NULL,
						MB_OK | MB_ICONSTOP | MB_TOPMOST,
						MACRO_ERROR_TITLE,
						LS(STR_ERR_DLGKEYMACMGR3),
						line,
						i + 1
					);
					m_nReady = false;
				}
				VARTYPE type = VT_EMPTY;
				if( nArgs < 4 ){
					type = mInfo->m_varArguments[nArgs];
				}else{
					if(  mInfo->m_pData && nArgs < mInfo->m_pData->m_nArgMinSize ){
						type = mInfo->m_pData->m_pVarArgEx[nArgs - 4];
					}
				}

				//	Skip Space
				while( szLine[i] == LTEXT(' ') || szLine[i] == LTEXT('\t') )
					i++;

				//@@@ 2002.2.2 YAZAKI PPA.DLLマクロにあわせて仕様変更。文字列は''で囲む。
				//	Jun. 16, 2002 genta double quotationも許容する
				if( LTEXT('\'') == szLine[i] || LTEXT('\"') == szLine[i]  ){	//	'で始まったら文字列だよきっと。
					// Jun. 16, 2002 genta プロトタイプチェック
					// Jun. 27, 2002 genta 余分な引数を無視するよう，VT_EMPTYを許容する．
					if( type != VT_BSTR && 
						type != VT_EMPTY ){
						::MYMESSAGEBOX(
							NULL,
							MB_OK | MB_ICONSTOP | MB_TOPMOST,
							MACRO_ERROR_TITLE,
							LS(STR_ERR_DLGKEYMACMGR4),
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
					nBgn = nEnd = i;	//	nBgnは引数の先頭の文字
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
						if( i == nLineLen ){	//	行末に来てしまった
							::MYMESSAGEBOX(
								NULL,
								MB_OK | MB_ICONSTOP | MB_TOPMOST,
								MACRO_ERROR_TITLE,
								LS(STR_ERR_DLGKEYMACMGR5),
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

					CNativeW cmemWork;
					cmemWork.SetString( strLine.c_str() + nBgn, nEnd - nBgn );
					// 2014.01.28 「"\\'"」のような場合の不具合を修正
					cmemWork.Replace( L"\\\\", L"\\\1" ); // 一時置換(最初に必要)
					cmemWork.Replace( LTEXT("\\\'"), LTEXT("\'") );

					//	Jun. 16, 2002 genta double quotationもエスケープ解除
					cmemWork.Replace( LTEXT("\\\""), LTEXT("\"") );
					cmemWork.Replace( L"\\r", L"\r" );
					cmemWork.Replace( L"\\n", L"\n" );
					cmemWork.Replace( L"\\t", L"\t" );
					{
						// \uXXXX 置換
						size_t nLen = cmemWork.GetStringLength();
						size_t nBegin = 0;
						const wchar_t* p = cmemWork.GetStringPtr();
						CNativeW cmemTemp;
						for( size_t n = 0; n < nLen; n++ ){
							if( n + 1 < nLen && p[n] == L'\\' && p[n+1] == L'u' ){
								size_t k;
								for( k = n + 2; k < nLen && k < n + 2 + 4
									&& (WCODE::Is09(p[k])
										|| (L'a' <= p[k] && p[k] <= L'f')
										|| (L'A' <= p[k] && p[k] <= L'F')); k++ ){
								}
								cmemTemp.AppendString( p + nBegin, n - nBegin );
								nBegin = k;
								if( 0 < k - n - 2 ){
									wchar_t hex[5];
									wcsncpy( hex, &p[n+2], k - n - 2 );
									hex[k - n - 2] = L'\0';
									wchar_t* pEnd = NULL;
									wchar_t c = static_cast<wchar_t>(wcstol(hex, &pEnd, 16));
									cmemTemp.AppendString( &c, 1 );
								}
								n = k - 1;
							}
						}
						if( nBegin != 0 ){
							if( 0 < nLen - nBegin ){
								cmemTemp.AppendString( p + nBegin, nLen - nBegin );
							}
							cmemWork.swap( cmemTemp );
						}
					}
					cmemWork.Replace( L"\\\1", L"\\" ); // 一時置換を\に戻す(最後でないといけない)
					macro->AddStringParam( cmemWork.GetStringPtr(), cmemWork.GetStringLength() );	//	引数を文字列として追加
				}
 				else if ( Is09(szLine[i]) || szLine[i] == L'-' ){	//	数字で始まったら数字列だ(-記号も含む)。
					// Jun. 16, 2002 genta プロトタイプチェック
					// Jun. 27, 2002 genta 余分な引数を無視するよう，VT_EMPTYを許容する．
					if( type != VT_I4 &&
						type != VT_EMPTY){
						::MYMESSAGEBOX(
							NULL,
							MB_OK | MB_ICONSTOP | MB_TOPMOST,
							MACRO_ERROR_TITLE,
							LS(STR_ERR_DLGKEYMACMGR6),
							line,
							i + 1,
							szFuncName,
							nArgs + 1
						);
						m_nReady = false;
						break;
					}
					nBgn = nEnd = i;	//	nBgnは引数の先頭の文字
					//	行末の検出のため，ループ回数を1増やした
					for( i = nBgn + 1; i <= nLineLen; ++i ){		//	最後の文字+1までスキャン
						if( Is09(szLine[i]) ){	// まだ数値
//							++i;
							continue;
						}
						else {
							nEnd = i;	//	数字の最後の文字
							i--;
							break;
						}
					}

					CNativeW cmemWork;
					cmemWork.SetString( strLine.c_str() + nBgn, nEnd - nBgn );
					// Jun. 16, 2002 genta
					//	数字の中にquotationは入っていないよ
					//cmemWork.Replace( L"\\\'", L"\'" );
					//cmemWork.Replace( L"\\\\", L"\\" );
					macro->AddIntParam( _wtoi(cmemWork.GetStringPtr()) );	//	引数を数字として追加
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
						LS(STR_ERR_DLGKEYMACMGR7), line, i + 1 );
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
				LS(STR_ERR_DLGKEYMACMGR8), line, szFuncName );
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

/*! キーボードマクロを文字列から読み込み */
BOOL CKeyMacroMgr::LoadKeyMacroStr( HINSTANCE hInstance, const TCHAR* pszCode )
{
	// 一時ファイル名を作成
	TCHAR szTempDir[_MAX_PATH];
	TCHAR szTempFile[_MAX_PATH];
	if( 0 == ::GetTempPath( _MAX_PATH, szTempDir ) )return FALSE;
	if( 0 == ::GetTempFileName( szTempDir, _T("mac"), 0, szTempFile ) )return FALSE;
	// 一時ファイルに書き込む
	CTextOutputStream out = CTextOutputStream( szTempFile );
	out.WriteString( to_wchar( pszCode ) );
	out.Close();

	// マクロ読み込み
	BOOL bRet = LoadKeyMacro( hInstance, szTempFile );

	::DeleteFile( szTempFile );			// 一時ファイル削除

	return bRet;
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
	CMacroFactory::getInstance()->RegisterCreator( Creator );
}
//	To Here Apr. 29, 2002 genta


