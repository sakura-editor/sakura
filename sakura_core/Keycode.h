//	$Id$
/*!	@file
	キーコード定義

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
#define	KEY_LBUTTONCLK	 VK_LBUTTON
#define	KEY_LBUTTONDBLCLK 0x00000200
#define	KEY_RBUTTONCLK	 VK_RBUTTON
#define	KEY_RBUTTONDBLCLK 0x00000400



///////////////////////////////////////////////////////////////////////
#endif /* _KEYCODE_H_ */

/*[EOF]*/
