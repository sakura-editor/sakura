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
#ifndef SAKURA_OS_4EAF837F_94E1_4B90_BF99_5AC3DEC630E79_H_
#define SAKURA_OS_4EAF837F_94E1_4B90_BF99_5AC3DEC630E79_H_

#include <ObjIdl.h> // LPDATAOBJECT


//システム資源
BOOL GetSystemResources( int*, int*, int* );	/* システムリソースを調べる */
BOOL CheckSystemResources( const TCHAR* );	/* システムリソースのチェック */

//クリップボード
bool SetClipboardText( HWND hwnd, const ACHAR* pszText, int nLength );    //!< クリープボードにText形式でコピーする。ANSI版。nLengthは文字単位。
bool SetClipboardText( HWND hwnd, const WCHAR* pszText, int nLength ); //!< クリープボードにText形式でコピーする。UNICODE版。nLengthは文字単位。
BOOL IsDataAvailable( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat );
HGLOBAL GetGlobalData( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat );

//	Sep. 10, 2002 genta CWSH.cppからの移動に伴う追加
bool ReadRegistry(HKEY Hive, const TCHAR* Path, const TCHAR* Item, TCHAR* Buffer, unsigned BufferCount);

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


//カレントディレクトリユーティリティ。
//コンストラクタでカレントディレクトリを保存し、デストラクタでカレントディレクトリを復元するモノ。
//2008.03.01 kobake 作成
class CCurrentDirectoryBackupPoint{
public:
	CCurrentDirectoryBackupPoint();
	~CCurrentDirectoryBackupPoint();
private:
	TCHAR m_szCurDir[_MAX_PATH];
};



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      メッセージ定数                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- マウス -- -- //

#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif
// novice 2004/10/10 マウスサイドボタン対応
#ifndef WM_XBUTTONDOWN
	#define WM_XBUTTONDOWN   0x020B
	#define WM_XBUTTONUP     0x020C
	#define WM_XBUTTONDBLCLK 0x020D
#endif
#ifndef XBUTTON1
	#define XBUTTON1 0x0001
	#define XBUTTON2 0x0002
#endif


// -- -- テーマ -- -- //

// 2006.06.17 ryoji WM_THEMECHANGED
#ifndef	WM_THEMECHANGED
#define WM_THEMECHANGED		0x031A
#endif


// -- -- IME (imm.h) -- -- //

#ifndef IMR_RECONVERTSTRING
#define IMR_RECONVERTSTRING             0x0004
#endif // IMR_RECONVERTSTRING

/* 2002.04.09 minfu 再変換調整 */
#ifndef IMR_CONFIRMRECONVERTSTRING
#define IMR_CONFIRMRECONVERTSTRING             0x0005
#endif // IMR_CONFIRMRECONVERTSTRING

#endif /* SAKURA_OS_4EAF837F_94E1_4B90_BF99_5AC3DEC630E79_H_ */
/*[EOF]*/
