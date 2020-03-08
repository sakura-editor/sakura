/*! @file */
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
#pragma once

#include <ObjIdl.h> // LPDATAOBJECT

//システム資源
BOOL GetSystemResources( int* pnSystemResources, int* pnUserResources, int* pnGDIResources );	/* システムリソースを調べる */
BOOL CheckSystemResources( const WCHAR* pszAppName );	/* システムリソースのチェック */

//クリップボード
bool SetClipboardText( HWND hwnd, const WCHAR* pszText, int nLength ); //!< クリープボードにText形式でコピーする。UNICODE版。nLengthは文字単位。
BOOL IsDataAvailable( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat );
HGLOBAL GetGlobalData( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat );

//	Sep. 10, 2002 genta CWSH.cppからの移動に伴う追加
bool ReadRegistry(HKEY Hive, const WCHAR* Path, const WCHAR* Item, WCHAR* Buffer, unsigned BufferCount);

//	May 01, 2004 genta マルチモニタ対応のデスクトップ領域取得
bool GetMonitorWorkRect(HWND     hWnd, LPRECT prcWork, LPRECT prcMonitor = NULL);	// 2006.04.21 ryoji パラメータ prcMonitor を追加
bool GetMonitorWorkRect(LPCRECT  prc,  LPRECT prcWork, LPRECT prcMonitor = NULL);	// 2006.04.21 ryoji
bool GetMonitorWorkRect(POINT    pt,   LPRECT prcWork, LPRECT prcMonitor = NULL);	// 2006.04.21 ryoji
bool GetMonitorWorkRect(HMONITOR hMon, LPRECT prcWork, LPRECT prcMonitor = NULL);	// 2006.04.21 ryoji

// 2006.06.17 ryoji
#define PACKVERSION( major, minor ) MAKELONG( minor, major )
DWORD GetComctl32Version();					// Comctl32.dll のバージョン番号を取得						// 2006.06.17 ryoji
BOOL IsVisualStyle();						// 自分が現在ビジュアルスタイル表示状態かどうかを示す		// 2006.06.17 ryoji
void PreventVisualStyle( HWND hWnd );		// 指定ウィンドウでビジュアルスタイルを使わないようにする	// 2006.06.23 ryoji
void MyInitCommonControls();				// コモンコントロールを初期化する							// 2006.06.21 ryoji

/* Wow64 のエミュレーション上で実行しているか判定する */
BOOL IsWow64();

/*!
	@brief Wow64 の ファイルシステムリダイレクションを一時的に無効にして、クラス破棄時に元に戻すクラス
	@note  このクラスを継承しないように final をつける
*/
class CDisableWow64FsRedirect final {
public:
	/*!
		@brief 	コンストラクタで ファイルシステムリダイレクションを無効にする
		@param isOn この引数が TRUE のときに無効化処理を行う
	*/
	CDisableWow64FsRedirect(BOOL isOn);

	/*!
		@brief 	ファイルシステムリダイレクションを元に戻す
	*/
	~CDisableWow64FsRedirect();
	
	// コピー不可 (C++11 で利用可能)
	CDisableWow64FsRedirect(const CDisableWow64FsRedirect&) = delete;
	CDisableWow64FsRedirect& operator = (const CDisableWow64FsRedirect&) = delete;
	
	// ムーブ不可 (C++11 で利用可能)
	CDisableWow64FsRedirect(CDisableWow64FsRedirect&&) = delete;
	CDisableWow64FsRedirect& operator = (CDisableWow64FsRedirect&&) = delete;

private:
	BOOL	m_isSuccess;
	PVOID	m_OldValue;
};

//カレントディレクトリユーティリティ。
//コンストラクタでカレントディレクトリを保存し、デストラクタでカレントディレクトリを復元するモノ。
//2008.03.01 kobake 作成
class CCurrentDirectoryBackupPoint{
public:
	CCurrentDirectoryBackupPoint();
	~CCurrentDirectoryBackupPoint();
private:
	WCHAR m_szCurDir[_MAX_PATH];
};

/*!
	@brief PowerShell が利用可能か判定する
*/
BOOL IsPowerShellAvailable(void);

/*!
	@brief IMEのオープン状態を設定する
	@param hWnd 設定対象のウィンドウハンドル
	@param bOpen 設定するオープン状態
	@param pBackup `nullptr` でなければ設定前のオープン状態を取得
	@return	手続きが成功したら true 失敗したら false
*/
BOOL ImeSetOpen(HWND hWnd, BOOL bOpen, BOOL* pBackup);
