/*!	@file
	@brief 各国語メッセージリソース対応

	@author nasukoji
	@date 2011.04.10	新規作成
*/
/*
	Copyright (C) 2011, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CSelectLang.h"
#include "util/os.h"
#include "util/module.h"

#include <new>

LPTSTR CSelectLang::m_szDefaultLang = NULL;					// メッセージリソースDLL未読み込み時のデフォルト言語
CSelectLang::SELLANG_INFO CSelectLang::m_sLangInfo = {0};	// メッセージリソース用構造体

/*!
	@brief デストラクタ

	@note 読み込んだメッセージリソースDLLを解放する

	@date 2011.04.10 nasukoji	新規作成
*/
CSelectLang::~CSelectLang( void )
{
	if( m_sLangInfo.hInstance ){
		FreeLibrary( m_sLangInfo.hInstance );
		m_sLangInfo.hInstance = NULL;
	}

	m_sLangInfo.szDllName[0] = _T('\0');
	m_sLangInfo.szLangName[0] = _T('\0');
	m_sLangInfo.wLangId = 0;
	m_sLangInfo.bValid = FALSE;

	if( m_szDefaultLang ){
		delete[] m_szDefaultLang;
		m_szDefaultLang = NULL;
	}
}

/*!
	@brief メッセージリソースDLLのインスタンスハンドルを返す

	@retval メッセージリソースDLLのインスタンスハンドル

	@note メッセージリソースDLLをロードしていない場合exeのインスタンスハンドルが返る

	@date 2011.04.10 nasukoji	新規作成
*/
HINSTANCE CSelectLang::getLangRsrcInstance( void )
{
	return m_sLangInfo.hInstance;
}

/*!
	@brief メッセージリソースDLL未読み込み時のデフォルト言語の文字列を返す

	@retval デフォルト言語の文字列（"(Japanese)" または "(English(United States))"）

	@note アプリケーションリソースより読み込んだ "(Japanese)" または "(English(United States))"

	@date 2011.04.10 nasukoji	新規作成
*/
LPCTSTR CSelectLang::getDefaultLangString( void )
{
	return m_szDefaultLang;
}

/*!
	@brief 言語環境を初期化する
	
	@retval メッセージリソースDLLのインスタンスハンドル

	@note メッセージリソースDLLが未指定、または読み込みエラー発生の時はexeのインスタンスハンドルが返る
	@note （LoadString()の引数としてそのまま使用するため）
	@note デフォルト言語の文字列の読み込みも行う
	@note プロセス毎にCShareDataから1回だけ呼ばれる

	@date 2011.04.10 nasukoji	新規作成
*/
HINSTANCE CSelectLang::InitializeLanguageEnvironment( void )
{
	TCHAR szTemp[256];
	int nCount;

	// 言語情報ダイアログで "System default" に表示する文字列を作成する
	nCount = ::LoadString( GetModuleHandle(NULL), STR_SELLANG_NAME, szTemp, 255 );

	if( nCount ){
		if( m_szDefaultLang )
			delete[] m_szDefaultLang;

		m_szDefaultLang = new TCHAR[nCount + 1];		// 番兵分を加算

		if( m_szDefaultLang ){
			szTemp[nCount] = _T('\0');
			_tcscpy( m_szDefaultLang, szTemp );
			
		}
	}

	if( m_sLangInfo.hInstance && m_sLangInfo.hInstance != GetModuleHandle(NULL) ){
		// 読み込み済みのDLLを解放する
		::FreeLibrary( m_sLangInfo.hInstance );
		m_sLangInfo.hInstance = NULL;
	}

	//カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CCurrentDirectoryBackupPoint cCurDirBackup;
	ChangeCurrentDirectoryToExeDir();
// ★iniまたはexeフォルダとなるように改造が必要

	WIN32_FIND_DATA w32fd;
	TCHAR szPath[] = _T("sakura_lang_*.dll");			// サーチするメッセージリソースDLL
	HANDLE handle = FindFirstFile( szPath, &w32fd );
	BOOL result = (INVALID_HANDLE_VALUE != handle) ? TRUE : FALSE;

	while( result ){
		if( ! (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ){		//フォルダでない
			// バッファに登録する。
			_tcscpy( m_sLangInfo.szDllName, w32fd.cFileName );
			m_sLangInfo.hInstance = CSelectLang::LoadLangRsrcLibrary( m_sLangInfo );

			if( m_sLangInfo.hInstance && !m_sLangInfo.bValid ){
				// メッセージリソースDLLとしては無効
				::FreeLibrary( m_sLangInfo.hInstance );
				m_sLangInfo.hInstance = NULL;
			}else{
				break;
			}
		}

		result = FindNextFile( handle, &w32fd );
	}

	if( INVALID_HANDLE_VALUE != handle ){
		FindClose( handle );
		handle = INVALID_HANDLE_VALUE;
	}

	if( !m_sLangInfo.hInstance ){
		// DLLが読み込まれなかった場合、exeのインスタンスハンドルとする
		m_sLangInfo.hInstance = GetModuleHandle(NULL);
	}else if( m_sLangInfo.bValid ){
		// ロケールを設定
		SetThreadUILanguage( m_sLangInfo.wLangId );						// Vista / Win7
		SetThreadLocale(MAKELCID( m_sLangInfo.wLangId, SORT_DEFAULT ));	// Win2000/XP
	}

	return m_sLangInfo.hInstance;
}

/*!
	@brief メッセージリソースDLLをロードする
	
	@retval メッセージリソースDLLのインスタンスハンドル

	@note メッセージリソースDLLが未指定、または読み込みエラー発生の時はNULLが返る

	@date 2011.04.10 nasukoji	新規作成
*/
HINSTANCE CSelectLang::LoadLangRsrcLibrary( SELLANG_INFO& lang )
{
	if( lang.szDllName[0] == _T('\0') )
		return NULL;		// DLLが指定されていなければNULLを返す

	int nCount;

	lang.bValid  = FALSE;
	lang.szLangName[0] = _T('\0');
	lang.wLangId = 0;

	HINSTANCE hInstance = LoadLibraryExedir( lang.szDllName );

	if( hInstance ){
		// 言語名を取得
		nCount = ::LoadString( hInstance, STR_SELLANG_NAME, lang.szLangName, _countof(lang.szLangName) );

		if( nCount > 0 ){
			// 言語IDを取得
			TCHAR szBuf[7];		// "0x" + 4桁 + 番兵
			nCount = ::LoadString( hInstance, STR_SELLANG_LANGID, szBuf, _countof(szBuf) );
			szBuf[_countof(szBuf) - 1] = _T('\0');

			if( nCount > 0 ){
				lang.wLangId = (WORD)_tcstoul( szBuf, NULL, 16 );		// 言語IDを数値化

				if( lang.wLangId > 0 )
					lang.bValid = TRUE;		// メッセージリソースDLLとして有効
			}
		}
	}

	return hInstance;
}



// 文字列リソース読み込み用グローバル
CLoadString::CLoadStrBuffer CLoadString::m_acLoadStrBufferTemp[];	// 文字列読み込みバッファの配列（CLoadString::LoadStringSt() が使用する）
int CLoadString::m_nDataTempArrayIndex = 0;							// 最後に使用したバッファのインデックス（CLoadString::LoadStringSt() が使用する）


/*!
	@brief 静的バッファに文字列リソースを読み込む（各国語メッセージリソース対応）

	@param[in] uid リソースID

	@retval 読み込んだ文字列（文字列無しの時 "" が返る）

	@note 静的バッファ（m_acLoadStrBufferTemp[?]）に文字列リソースを読み込む。
	@note バッファは複数準備しているが、呼び出す毎に更新するのでバッファ個数を
	@note 超えて呼び出すと順次内容が失われていく。
	@note 呼び出し直後での使用や関数の引数などでの使用を想定しており、前回値を
	@note 取り出すことはできない。
	@note 使用例）::SetWindowText( m_hWnd, CLoadString::LoadStringSt(STR_ERR_DLGSMCLR1) );
	@note アプリケーション内の関数への引数とする場合、その関数が本関数を使用
	@note しているか意識する必要がある（上限を超えれば内容が更新されるため）
	@note 内容を保持したい場合は CLoadString::LoadString() を使用する。

	@date 2011.06.01 nasukoji	新規作成
*/
LPCTSTR CLoadString::LoadStringSt( UINT uid )
{
	// 使用するバッファの現在位置を進める
	m_nDataTempArrayIndex = ( m_nDataTempArrayIndex >= _countof(m_acLoadStrBufferTemp) - 1) ?
															0 : ( m_nDataTempArrayIndex + 1 );

	m_acLoadStrBufferTemp[m_nDataTempArrayIndex].LoadString( uid );

	return /* CLoadString:: */ m_acLoadStrBufferTemp[m_nDataTempArrayIndex].GetStringPtr();
}

/*!
	@brief 文字列リソースを読み込む（各国語メッセージリソース対応）

	@param[in] uid リソースID

	@retval 読み込んだ文字列（文字列無しの時 "" が返る）

	@note メンバ変数内に記憶されるため  CLoadString::LoadStringSt() の様に
	@note 不用意に破壊されることはない。
	@note ただし、変数を準備する必要があるのが不便。
	@note 使用例）
	@note   CLoadString cStr[2];
	@note   cDlgInput1.DoModal( m_hInstance, m_hWnd,
	@note       cStr[0].LoadString(STR_ERR_DLGPRNST1),
	@note       cStr[1].LoadString(STR_ERR_DLGPRNST2),
	@note       sizeof( m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName ) - 1, szWork ) )

	@date 2011.06.01 nasukoji	新規作成
*/
LPCTSTR CLoadString::LoadString( UINT uid )
{
	m_cLoadStrBuffer.LoadString( uid );

	return /* this-> */ m_cLoadStrBuffer.GetStringPtr();
}

/*!
	@brief 文字列リソースを読み込む（読み込み実行部）

	@param[in] uid  リソースID

	@retval 読み込んだ文字数（TCHAR単位）

	@note メッセージリソースより文字列を読み込む。メッセージリソースDLLに指定の
	@note リソースが存在しない、またはメッセージリソースDLL自体が読み込まれて
	@note いない場合、内部リソースより文字列を読み込む。
	@note 最初は静的バッファに読み込むがバッファ不足となったらバッファを拡張
	@note して読み直す。
	@note 取得したバッファはデストラクタで解放する。
	@note ANSI版は2バイト文字の都合により（バッファ - 2）バイトまでしか読まない
	@note 場合があるので1バイト少ない値でバッファ拡張を判定する。

	@date 2011.06.01 nasukoji	新規作成
*/
int CLoadString::CLoadStrBuffer::LoadString( UINT uid )
{
	if( !m_pszString ){
		// バッファポインタが設定されていない場合初期化する（普通はあり得ない）
		m_pszString = m_szString;					// 変数内に準備したバッファを接続
		m_nBufferSize = _countof(m_szString);		// 配列個数
		m_szString[m_nBufferSize - 1] = 0;
		m_nLength = _tcslen(m_szString);			// 文字数
	}

	HINSTANCE hRsrc = CSelectLang::getLangRsrcInstance();		// メッセージリソースDLLのインスタンスハンドル

	if( !hRsrc ){
		// メッセージリソースDLL読込処理前は内部リソースを使う
		hRsrc = ::GetModuleHandle(NULL);
	}

	int nRet = 0;

	while(1){
		nRet = ::LoadString( hRsrc, uid, m_pszString, m_nBufferSize );

		// リソースが無い
		if( nRet == 0 ){
			if( hRsrc != ::GetModuleHandle(NULL) ){
				hRsrc = ::GetModuleHandle(NULL);	// 内部リソースを使う
			}else{
				// 内部リソースからも読めなかったら諦める（普通はあり得ない）
				m_pszString[0] = 0;
				break;
			}
#ifdef UNICODE
		}else if( nRet >= m_nBufferSize - 1 ){
#else
		}else if( nRet >= m_nBufferSize - 2 ){		// ANSI版は1小さい長さで再読み込みを判定する
#endif
			// 読みきれなかった場合、バッファを拡張して読み直す
			int nTemp = m_nBufferSize + LOADSTR_ADD_SIZE;		// 拡張したサイズ
			LPTSTR pTemp;

			try{
				pTemp = new TCHAR[nTemp];
			}
			catch(std::bad_alloc){
				// メモリ割り当て例外（例外の発生する環境の場合でも旧来の処理にする）
				pTemp = NULL;
			}

			if( pTemp ){
				if( m_pszString != m_szString ){
					delete[] m_pszString;
				}

				m_pszString = pTemp;
				m_nBufferSize = nTemp;
			}else{
				// メモリ取得に失敗した場合は直前の内容で諦める
				nRet = _tcslen( m_pszString );
				break;
			}
		}else{
			break;		// 文字列リソースが正常に取得できた
		}
	}

	m_nLength = nRet;	// 読み込んだ文字数

	return nRet;
}


/*[EOF]*/
