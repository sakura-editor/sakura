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

//ランタイム情報ライブラリにアクセスするWindowsヘッダを参照する
#include <ImageHlp.h>

#include "mem/CNativeW.h"

//デバッグ用。
//VistaだとExtTextOutの結果が即反映されない。この関数を用いると即反映されるので、
//デバッグ時ステップ実行する際に便利になる。ただし、当然重くなる。
#ifdef _DEBUG
#define DEBUG_SETPIXEL(hdc) SetPixel(hdc, -1, -1, 0); //SetPixelをすると、結果が即反映される。
#else
#define DEBUG_SETPIXEL(hdc)
#endif

namespace ApiWrap
{
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//          W系が存在しないAPIのための、新しい関数定義         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//W版が無いので、自作
BOOL MakeSureDirectoryPathExistsW(LPCWSTR pszDirPath);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    描画API 不具合ラップ                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//VistaでSetPixelが動かないため、代替関数を用意。
void SetPixelSurely(HDC hdc, int x, int y, COLORREF c);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      よく使う引数値                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! よく使うExtTextOutのオプション
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
    return (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
}

//! CTRLを押しているかどうか
inline bool GetKeyState_Control()
{
    return (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
}

//! ALTを押しているかどうか
inline bool GetKeyState_Alt()
{
    return (::GetKeyState(VK_MENU) & 0x8000) != 0;
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
} // namespace ApiWrap
using namespace ApiWrap;
