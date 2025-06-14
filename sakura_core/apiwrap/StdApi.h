/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_STDAPI_29C8A971_234C_46ED_96DB_A2D479992ABE_H_
#define SAKURA_STDAPI_29C8A971_234C_46ED_96DB_A2D479992ABE_H_
#pragma once

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
	void SetPixelSurely(HDC hdc,int x,int y,COLORREF c);

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
#endif /* SAKURA_STDAPI_29C8A971_234C_46ED_96DB_A2D479992ABE_H_ */
