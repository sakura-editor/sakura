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
#ifndef SAKURA_STDAPI_85471C2C_6AEE_410D_BD09_A59056A5BA68_H_
#define SAKURA_STDAPI_85471C2C_6AEE_410D_BD09_A59056A5BA68_H_

//ランタイム情報ライブラリにアクセスするWindowsヘッダを参照する
//c++規格への準拠が厳しくなったため、WindowsSDKが無名enumをtypedefするコードが怒られる。
#ifdef _MSC_VER
	//一時的に警告を無効にしてインクルードする
	#pragma warning(push)
	#pragma warning(disable:4091)
	#include <ImageHlp.h> //MakeSureDirectoryPathExists
	#pragma warning(pop)
#else
	#include <ImageHlp.h> //MakeSureDirectoryPathExists
#endif
#include "mem/CNativeT.h"

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
	#define MakeSureDirectoryPathExistsT MakeSureDirectoryPathExistsW

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              W系描画API                                      //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	LPWSTR CharNextW_AnyBuild(
		LPCWSTR lpsz
	);

	LPWSTR CharPrevW_AnyBuild(
		LPCWSTR lpszStart,
		LPCWSTR lpszCurrent
	);

	#define GetTextExtentPoint32W_AnyBuild GetTextExtentPoint32

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             その他W系API                                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	inline int LoadStringW_AnyBuild(
		HINSTANCE	hInstance,
		UINT		uID,
		LPWSTR		lpBuffer,
		int			nBufferCount	//!< バッファのサイズ。文字単位。
	)
	{
		return ::LoadStringW(hInstance, uID, lpBuffer, nBufferCount);
	}

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

#endif /* SAKURA_STDAPI_85471C2C_6AEE_410D_BD09_A59056A5BA68_H_ */
/*[EOF]*/
