#include "StdAfx.h"
#include "os.h"
#include "util/module.h"
#include "extmodule/CUxTheme.h"

/*!	Comctl32.dll のバージョン番号を取得

	@return Comctl32.dll のバージョン番号（失敗時は 0）

	@author ryoji
	@date 2006.06.17 ryoji 新規
*/
static DWORD s_dwComctl32Version = PACKVERSION(0, 0);
DWORD GetComctl32Version()
{
	if( PACKVERSION(0, 0) == s_dwComctl32Version )
		s_dwComctl32Version = GetDllVersion(_T("Comctl32.dll"));
	return s_dwComctl32Version;
}


/*!	自分が現在ビジュアルスタイル表示状態かどうかを示す
	Win32 API の IsAppThemed() はこれとは一致しない（IsAppThemed() と IsThemeActive() との差異は不明）

	@return ビジュアルスタイル表示状態(TRUE)／クラッシック表示状態(FALSE)

	@author ryoji
	@date 2006.06.17 ryoji 新規
*/
BOOL IsVisualStyle()
{
	// ロードした Comctl32.dll が Ver 6 以上で画面設定がビジュアルスタイル指定になっている場合だけ
	// ビジュアルスタイル表示になる（マニフェストで指定しないと Comctl32.dll は 6 未満になる）
	return ( (GetComctl32Version() >= PACKVERSION(6, 0)) && CUxTheme::getInstance()->IsThemeActive() );
}



/*!	指定ウィンドウでビジュアルスタイルを使わないようにする

	@param[in] hWnd ウィンドウ

	@author ryoji
	@date 2006.06.23 ryoji 新規
*/
void PreventVisualStyle( HWND hWnd )
{
	CUxTheme::getInstance()->SetWindowTheme( hWnd, L"", L"" );
	return;
}




/*!	コモンコントロールを初期化する

	@author ryoji
	@date 2006.06.21 ryoji 新規
*/
void MyInitCommonControls()
{
	BOOL (WINAPI *pfnInitCommonControlsEx)(LPINITCOMMONCONTROLSEX);

	BOOL bInit = FALSE;
	HINSTANCE hDll = ::GetModuleHandle(_T("COMCTL32"));
	if( NULL != hDll ){
		*(FARPROC*)&pfnInitCommonControlsEx = ::GetProcAddress( hDll, "InitCommonControlsEx" );
		if( NULL != pfnInitCommonControlsEx ){
			INITCOMMONCONTROLSEX icex;
			icex.dwSize = sizeof(icex);
			icex.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES;
			bInit = pfnInitCommonControlsEx( &icex );
		}
	}

	if( !bInit ){
		::InitCommonControls();
	}
}





/*!
	指定したウィンドウ／長方形領域／点／モニタに対応するモニタ作業領域を取得する

	モニタ作業領域：画面全体からシステムのタスクバーやアプリケーションのツールバーが占有する領域を除いた領域

	@param hWnd/prc/pt/hMon [in] 目的のウィンドウ／長方形領域／点／モニタ
	@param prcWork [out] モニタ作業領域
	@param prcMonitor [out] モニタ画面全体

	@retval true 対応するモニタはプライマリモニタ
	@retval false 対応するモニタは非プライマリモニタ

	@note 出力パラメータの prcWork や prcMonior に NULL を指定した場合、
	該当する領域情報は出力しない。呼び出し元は欲しいものだけを指定すればよい。
*/
//	From Here May 01, 2004 genta MutiMonitor
bool GetMonitorWorkRect(HWND hWnd, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	// 2006.04.21 ryoji Windows API 形式の関数呼び出しに変更（スタブに PSDK の MultiMon.h を利用）
	HMONITOR hMon = ::MonitorFromWindow( hWnd, MONITOR_DEFAULTTONEAREST );
	return GetMonitorWorkRect( hMon, prcWork, prcMonitor );
}
//	To Here May 01, 2004 genta

//	From Here 2006.04.21 ryoji MutiMonitor
bool GetMonitorWorkRect(LPCRECT prc, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	HMONITOR hMon = ::MonitorFromRect( prc, MONITOR_DEFAULTTONEAREST );
	return GetMonitorWorkRect( hMon, prcWork, prcMonitor );
}

bool GetMonitorWorkRect(POINT pt, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	HMONITOR hMon = ::MonitorFromPoint( pt, MONITOR_DEFAULTTONEAREST );
	return GetMonitorWorkRect( hMon, prcWork, prcMonitor );
}

bool GetMonitorWorkRect(HMONITOR hMon, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	MONITORINFO mi;
	::ZeroMemory( &mi, sizeof( mi ));
	mi.cbSize = sizeof( mi );
	::GetMonitorInfo( hMon, &mi );
	if( NULL != prcWork )
		*prcWork = mi.rcWork;		// work area rectangle of the display monitor
	if( NULL != prcMonitor )
		*prcMonitor = mi.rcMonitor;	// display monitor rectangle
	return ( mi.dwFlags == MONITORINFOF_PRIMARY ) ? true : false;
}
//	To Here 2006.04.21 ryoji MutiMonitor




/*!
	@brief レジストリから文字列を読み出す．
	
	@param Hive        [in]  HIVE
	@param Path        [in]  レジストリキーへのパス
	@param Item        [in]  レジストリアイテム名．NULLで標準のアイテム．
	@param Buffer      [out] 取得文字列を格納する場所
	@param BufferCount [in]  Bufferの指す領域のサイズ。文字単位。
	
	@retval true 値の取得に成功
	@retval false 値の取得に失敗
	
	@author 鬼
	@date 2002.09.10 genta CWSH.cppから移動
*/
bool ReadRegistry(HKEY Hive, const TCHAR* Path, const TCHAR* Item, TCHAR* Buffer, unsigned BufferCount)
{
	bool Result = false;
	
	HKEY Key;
	if(RegOpenKeyEx(Hive, Path, 0, KEY_READ, &Key) == ERROR_SUCCESS)
	{
		auto_memset(Buffer, 0, BufferCount);

		DWORD dwType = REG_SZ;
		DWORD dwDataLen = (BufferCount - 1) * sizeof(TCHAR); //※バイト単位！
		
		Result = (RegQueryValueEx(Key, Item, NULL, &dwType, reinterpret_cast<LPBYTE>(Buffer), &dwDataLen) == ERROR_SUCCESS);
		
		RegCloseKey(Key);
	}
	return Result;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      クリップボード                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//SetClipboardTextA,SetClipboardTextT 実装用テンプレート
//2007.08.14 kobake UNICODE用に改造
//
/*! クリープボードにText形式でコピーする
	@param hwnd    [in] クリップボードのオーナー
	@param pszText [in] 設定するテキスト
	@param nLength [in] 有効なテキストの長さ。文字単位。
	
	@retval true コピー成功
	@retval false コピー失敗。場合によってはクリップボードに元の内容が残る
	@date 2004.02.17 Moca 各所のソースを統合
*/
template <class T>
bool SetClipboardTextImp( HWND hwnd, const T* pszText, int nLength )
{
	HGLOBAL	hgClip;
	T*		pszClip;

	hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, (nLength + 1) * sizeof(T) );
	if( NULL == hgClip ){
		return false;
	}
	pszClip = (T*)::GlobalLock( hgClip );
	if( NULL == pszClip ){
		::GlobalFree( hgClip );
		return false;
	}
	auto_memcpy( pszClip, pszText, nLength );
	pszClip[nLength] = 0;
	::GlobalUnlock( hgClip );
	if( !::OpenClipboard( hwnd ) ){
		::GlobalFree( hgClip );
		return false;
	}
	::EmptyClipboard();
	if(sizeof(T)==sizeof(char)){
		::SetClipboardData( CF_OEMTEXT, hgClip );
	}
	else if(sizeof(T)==sizeof(wchar_t)){
		::SetClipboardData( CF_UNICODETEXT, hgClip );
	}
	else{
		assert(0); //※ここには来ない
	}
	::CloseClipboard();

	return true;
}

bool SetClipboardText( HWND hwnd, const ACHAR* pszText, int nLength )
{
	return SetClipboardTextImp<ACHAR>(hwnd,pszText,nLength);
}

bool SetClipboardText( HWND hwnd, const WCHAR* pszText, int nLength )
{
	return SetClipboardTextImp<WCHAR>(hwnd,pszText,nLength);
}

/*
	@date 2006.01.16 Moca 他のTYMEDが利用可能でも、取得できるように変更。
	@note IDataObject::GetData() で tymed = TYMED_HGLOBAL を指定すること。
*/
BOOL IsDataAvailable( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC	fe;

	// 2006.01.16 Moca 他のTYMEDが利用可能でも、IDataObject::GetData()で
	//  tymed = TYMED_HGLOBALを指定すれば問題ない
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = TYMED_HGLOBAL;
	// 2006.03.16 Moca S_FALSEでも受け入れてしまうバグを修正(ファイルのドロップ等)
	return S_OK == pDataObject->QueryGetData( &fe );
}

HGLOBAL GetGlobalData( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC fe;
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	// 2006.01.16 Moca fe.tymed = -1からTYMED_HGLOBALに変更。
	fe.tymed = TYMED_HGLOBAL;

	HGLOBAL hDest = NULL;
	STGMEDIUM stgMedium;
	// 2006.03.16 Moca SUCCEEDEDマクロではS_FALSEのとき困るので、S_OKに変更
	if( S_OK == pDataObject->GetData( &fe, &stgMedium ) ){
		if( stgMedium.pUnkForRelease == NULL ){
			if( stgMedium.tymed == TYMED_HGLOBAL )
				hDest = stgMedium.hGlobal;
		}else{
			if( stgMedium.tymed == TYMED_HGLOBAL ){
				DWORD nSize = ::GlobalSize( stgMedium.hGlobal );
				hDest = ::GlobalAlloc( GMEM_SHARE|GMEM_MOVEABLE, nSize );
				if( hDest != NULL ){
					// copy the bits
					LPVOID lpSource = ::GlobalLock( stgMedium.hGlobal );
					LPVOID lpDest = ::GlobalLock( hDest );
					memcpy_raw( lpDest, lpSource, nSize );
					::GlobalUnlock( hDest );
					::GlobalUnlock( stgMedium.hGlobal );
				}
			}
			::ReleaseStgMedium( &stgMedium );
		}
	}
	return hDest;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       システム資源                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* システムリソースを調べる
	Win16 の時は、GetFreeSystemResources という関数がありました。しかし、Win32 ではありません。
	サンクを作るだの DLL を作るだのは難しすぎます。簡単な方法を説明します。
	お使いの Windows95 の [アクセサリ]-[システムツール] にリソースメータがあるのなら、
	c:\windows\system\rsrc32.dll があるはずです。これは、リソースメータという Win32 アプリが、
	Win16 の GetFreeSystemResources 関数を呼ぶ為の DLL です。これを使いましょう。
*/
BOOL GetSystemResources(
	int*	pnSystemResources,
	int*	pnUserResources,
	int*	pnGDIResources
)
{
	#define GFSR_SYSTEMRESOURCES	0x0000
	#define GFSR_GDIRESOURCES		0x0001
	#define GFSR_USERRESOURCES		0x0002
	HINSTANCE	hlib;
	int (CALLBACK *GetFreeSystemResources)( int );

	hlib = ::LoadLibraryExedir( _T("RSRC32.dll") );
	if( (INT_PTR)hlib > 32 ){
		GetFreeSystemResources = (int (CALLBACK *)( int ))GetProcAddress(
			hlib,
			"_MyGetFreeSystemResources32@4"
		);
		if( GetFreeSystemResources != NULL ){
			*pnSystemResources = GetFreeSystemResources( GFSR_SYSTEMRESOURCES );
			*pnUserResources = GetFreeSystemResources( GFSR_USERRESOURCES );
			*pnGDIResources = GetFreeSystemResources( GFSR_GDIRESOURCES );
			::FreeLibrary( hlib );
			return TRUE;
		}else{
			::FreeLibrary( hlib );
			return FALSE;
		}
	}else{
		return FALSE;
	}
}


#if (WINVER < _WIN32_WINNT_WIN2K)
// NTではリソースチェックを行わない
/* システムリソースのチェック */
BOOL CheckSystemResources( const TCHAR* pszAppName )
{
	int		nSystemResources;
	int		nUserResources;
	int		nGDIResources;
	const TCHAR*	pszResourceName;
	/* システムリソースの取得 */
	if( GetSystemResources( &nSystemResources, &nUserResources,	&nGDIResources ) ){
//		MYTRACE( _T("nSystemResources=%d\n"), nSystemResources );
//		MYTRACE( _T("nUserResources=%d\n"), nUserResources );
//		MYTRACE( _T("nGDIResources=%d\n"), nGDIResources );
		pszResourceName = NULL;
		if( nSystemResources <= 5 ){
			pszResourceName = _T("システム ");
		}else
		if( nUserResources <= 5 ){
			pszResourceName = _T("ユーザー ");
		}else
		if( nGDIResources <= 5 ){
			pszResourceName = _T("GDI ");
		}
		if( NULL != pszResourceName ){
			ErrorBeep();
			ErrorBeep();
			::MYMESSAGEBOX( NULL, MB_OK | /*MB_YESNO | */ MB_ICONSTOP | MB_APPLMODAL | MB_TOPMOST, pszAppName,
				_T("%tsリソースが極端に不足しています。\n")
				_T("このまま%tsを起動すると、正常に動作しない可能性があります。\n")
				_T("新しい%tsの起動を中断します。\n")
				_T("\n")
				_T("システム リソース\t残り  %d%%\n")
				_T("User リソース\t残り  %d%%\n")
				_T("GDI リソース\t残り  %d%%\n\n"),
				pszResourceName,
				pszAppName,
				pszAppName,
				nSystemResources,
				nUserResources,
				nGDIResources
			);
//			) ){
				return FALSE;
//			}
		}
	}
	return TRUE;
}
#endif	// (WINVER < _WIN32_WINNT_WIN2K)


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        便利クラス                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//コンストラクタでカレントディレクトリを保存し、デストラクタでカレントディレクトリを復元するモノ。

CCurrentDirectoryBackupPoint::CCurrentDirectoryBackupPoint()
{
	int n = ::GetCurrentDirectory(_countof(m_szCurDir),m_szCurDir);
	if(n>0 && n<_countof(m_szCurDir)){
		//ok
	}
	else{
		//ng
		m_szCurDir[0] = _T('\0');
	}
}

CCurrentDirectoryBackupPoint::~CCurrentDirectoryBackupPoint()
{
	if(m_szCurDir[0]){
		::SetCurrentDirectory(m_szCurDir);
	}
}



