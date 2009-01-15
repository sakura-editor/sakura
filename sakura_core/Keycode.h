/*!	@file
	@brief キーコード定義

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _KEYCODE_H_
#define _KEYCODE_H_


#include <windows.h>
#include "funccode.h"

#define _SHIFT	0x00000001
#define _CTRL	0x00000002
#define _ALT	0x00000004

/* キーコードの定義 */
/* マウス操作 */
// 2002/2/10 aroka 未使用
//#define	KEY_LBUTTONCLK	 VK_LBUTTON
//#define	KEY_LBUTTONDBLCLK 0x00000200
//#define	KEY_RBUTTONCLK	 VK_RBUTTON
//#define	KEY_RBUTTONDBLCLK 0x00000400

#define	VK_XBUTTON1		0x05	// マウスサイドボタン1（Windows2000以降で使用可能）	// 2009.01.12 nasukoji
#define	VK_XBUTTON2		0x06    // マウスサイドボタン2（Windows2000以降で使用可能）	// 2009.01.12 nasukoji


///////////////////////////////////////////////////////////////////////
#endif /* _KEYCODE_H_ */


/*[EOF]*/
