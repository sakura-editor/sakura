#pragma once

#include <imagehlp.h> //MakeSureDirectoryPathExists

namespace ApiWrap
{
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          W系が存在しないAPIのための、新しい関数定義         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//W版が無いので、自作
	BOOL MakeSureDirectoryPathExistsW(LPCWSTR wszDirPath);
#ifdef _UNICODE
	#define MakeSureDirectoryPathExistsT MakeSureDirectoryPathExistsW
#else
	#define MakeSureDirectoryPathExistsT MakeSureDirectoryPathExists
#endif


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              W系描画API (ANSI版でも利用可能)                //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	BOOL ExtTextOutW_AnyBuild(
		HDC				hdc,
		int				x,
		int				y,
		UINT			fuOptions,
		const RECT*		lprc,
		LPCWSTR			lpwString,
		UINT			cbCount,
		const int*		lpDx
	);

	BOOL TextOutW_AnyBuild(
		HDC		hdc,
		int		nXStart,
		int		nYStart,
		LPCWSTR	lpString,
		int		cbString
	);

	LPWSTR CharNextW_AnyBuild(
		LPCWSTR lpsz
	);

	LPWSTR CharPrevW_AnyBuild(
		LPCWSTR lpszStart,
		LPCWSTR lpszCurrent
	);

#ifdef _UNICODE
	#define GetTextExtentPoint32W_AnyBuild GetTextExtentPoint32
#else
	BOOL GetTextExtentPoint32W_AnyBuild(
		HDC		hdc, 
		LPCWSTR	lpString, 
		int		cbString, 
		LPSIZE	lpSize
	);
#endif



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             その他W系API (ANSI版でも利用可能)               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	int LoadStringW_AnyBuild(
		HINSTANCE	hInstance,
		UINT		uID,
		LPWSTR		lpBuffer,
		int			nBufferCount	//!< バッファのサイズ。文字単位。
	);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      よく使う引数値                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//! よく使うExtTextOutW_AnyBuildのオプション
	inline UINT ExtTextOutOption()
	{
		return ETO_CLIPPED | ETO_OPAQUE;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       よく使う用法                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//! SHIFTを押しているかどうか
	inline bool GetKeyState_Shift()
	{
		return (::GetKeyState(VK_SHIFT)&0x8000)!=0;
	}

	//! CTRLを押しているかどうか
	inline bool GetKeyState_Control()
	{
		return (::GetKeyState(VK_CONTROL)&0x8000)!=0;
	}

	//! ALTを押しているかどうか
	inline bool GetKeyState_Alt()
	{
		return (::GetKeyState(VK_MENU)&0x8000)!=0;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           定数                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//	Jun. 29, 2002 こおり
	//	Windows 95対策．Property SheetのサイズをWindows95が認識できる物に固定する．
	#if defined(_WIN64) || defined(_UNICODE)
		static const size_t sizeof_old_PROPSHEETHEADER = sizeof(PROPSHEETHEADER);
	#else
		static const size_t sizeof_old_PROPSHEETHEADER = 40;
	#endif

	//	Jan. 29, 2002 genta
	//	Win95/NTが納得するsizeof( MENUITEMINFO )
	//	これ以外の値を与えると古いOSでちゃんと動いてくれない．
	#if defined(_WIN64) || defined(_UNICODE)
		static const int SIZEOF_MENUITEMINFO = sizeof(MENUITEMINFO);
	#else
		static const int SIZEOF_MENUITEMINFO = 44;
	#endif


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             SendMessage,PostMessage意味付け                 //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 過去のUNICODE化の名残です。
	// 現在となっては、特に意味はありません。

	//文字コードに関係のなさそうな SendMessage は SendMessageAny に差し替えておく。
	#define SendMessageAny SendMessage

	//WM_COMMAND系の SendMessage は SendMessageCmd に差し替えておく。
	#define SendMessageCmd SendMessage

	//文字コードに関係のなさそうな PostMessage は PostMessageAny に差し替えておく。
	#define PostMessageAny PostMessage

	//WM_COMMAND系の PostMessage は PostMessageCmd に差し替えておく。
	#define PostMessageCmd PostMessage

}
using namespace ApiWrap;




//	Sep. 22, 2003 MIK
//	古いSDK対策．新しいSDKでは不要
#ifndef _WIN64
#ifndef DWORD_PTR
#define DWORD_PTR DWORD
#endif
#ifndef ULONG_PTR
#define ULONG_PTR ULONG
#endif
#ifndef LONG_PTR
#define LONG_PTR LONG
#endif
#ifndef UINT_PTR
#define UINT_PTR UINT
#endif
#ifndef INT_PTR
#define INT_PTR INT
#endif
#ifndef SetWindowLongPtr
#define SetWindowLongPtr SetWindowLong
#endif
#ifndef GetWindowLongPtr
#define GetWindowLongPtr GetWindowLong
#endif
#ifndef DWLP_USER
#define DWLP_USER DWL_USER
#endif
#ifndef GWLP_WNDPROC
#define GWLP_WNDPROC GWL_WNDPROC
#endif
#ifndef GWLP_USERDATA
#define GWLP_USERDATA GWL_USERDATA
#endif
#ifndef GWLP_HINSTANCE
#define GWLP_HINSTANCE GWL_HINSTANCE
#endif
#ifndef DWLP_MSGRESULT
#define DWLP_MSGRESULT DWL_MSGRESULT
#endif
#endif  //_WIN64

#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT 29
#endif
#ifndef COLOR_MENUBAR
#define COLOR_MENUBAR 30
#endif

