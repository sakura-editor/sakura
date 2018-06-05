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
#ifndef SAKURA_STDAPI_85471C2C_6AEE_410D_BD09_A59056A5BA68_H_
#define SAKURA_STDAPI_85471C2C_6AEE_410D_BD09_A59056A5BA68_H_


//ランタイム情報ライブラリにアクセスするWindowsヘッダを参照する
//c++規格への準拠が厳しくなったため、WindowsSDKが無名enumをtypedefするコードが怒られる。
#if defined(_MSC_VER) && _MSC_VER >= 1900
	//一時的に警告を無効にしてインクルードする
	#pragma warning(push)
	#pragma warning(disable:4091)
	#include <ImageHlp.h> //MakeSureDirectoryPathExists
	#pragma warning(pop)
#else
	#include <ImageHlp.h> //MakeSureDirectoryPathExists
#endif


//デバッグ用。
//VistaだとExtTextOutの結果が即反映されない。この関数を用いると即反映されるので、
//デバッグ時ステップ実行する際に便利になる。ただし、当然重くなる。
#ifdef _DEBUG
#define DEBUG_SETPIXEL(hdc) SetPixel(hdc,-1,-1,0); //SetPixelをすると、結果が即反映される。
#else
#define DEBUG_SETPIXEL(hdc)
#endif

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

	/*!
		ANSI版でも使えるExtTextOutW_AnyBuild。
		文字数制限1024半角文字。(文字間隔配列を1024半角文字分しか用意していないため)
	*/
#ifdef _UNICODE
	inline BOOL ExtTextOutW_AnyBuild(
		HDC				hdc,
		int				x,
		int				y,
		UINT			fuOptions,
		const RECT*		lprc,
		LPCWSTR			lpwString,
		UINT			cbCount,
		const int*		lpDx
	)
	{
		BOOL ret=::ExtTextOut(hdc,x,y,fuOptions,lprc,lpwString,cbCount,lpDx);
		DEBUG_SETPIXEL(hdc);
		return ret;
	}
#else
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
#endif

#ifdef _UNICODE
	inline BOOL TextOutW_AnyBuild(
		HDC		hdc,
		int		nXStart,
		int		nYStart,
		LPCWSTR	lpwString,
		int		cbString
	)
	{
		BOOL ret=::TextOut(hdc,nXStart,nYStart,lpwString,cbString);
		DEBUG_SETPIXEL(hdc);
		return ret;
	}
#else
	BOOL TextOutW_AnyBuild(
		HDC		hdc,
		int		nXStart,
		int		nYStart,
		LPCWSTR	lpwString,
		int		cbString
	);
#endif

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

#ifdef _UNICODE
	inline int LoadStringW_AnyBuild(
		HINSTANCE	hInstance,
		UINT		uID,
		LPWSTR		lpBuffer,
		int			nBufferCount	//!< バッファのサイズ。文字単位。
	)
	{
		return ::LoadStringW(hInstance, uID, lpBuffer, nBufferCount);
	}
#else
	int LoadStringW_AnyBuild(
		HINSTANCE	hInstance,
		UINT		uID,
		LPWSTR		lpBuffer,
		int			nBufferCount	//!< バッファのサイズ。文字単位。
	);
#endif


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                    描画API 不具合ラップ                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//VistaでSetPixelが動かないため、代替関数を用意。
	void SetPixelSurely(HDC hdc,int x,int y,COLORREF c);

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

#endif /* SAKURA_STDAPI_85471C2C_6AEE_410D_BD09_A59056A5BA68_H_ */
/*[EOF]*/
