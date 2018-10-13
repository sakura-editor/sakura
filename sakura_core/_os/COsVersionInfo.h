/*!	@file
	@brief COsVersionInfo

	OSVERSIONINFOをラッピング

	@author YAZAKI
	@date 2002年3月3日
*/
/*
	Copyright (C) 2001, YAZAKI, shoji masami
	Copyright (C) 2002, YAZAKI, minfu
	Copyright (C) 2003, genta
	Copyright (C) 2005, ryoji
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

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

#ifndef _COSVERSIONINFO_H_
#define _COSVERSIONINFO_H_


// 稼働中のOSはxp以降か？＝常に真。
inline bool IsWinXP_or_later()			{ return true; }
// 稼働中のOSはw2k以降か？＝常に真。
inline bool IsWin2000_or_later()		{ return true; }
// 稼働中のOSはwindows Meか？＝常に偽。
inline bool IsWinMe()					{ return false; }
// 稼働中のOSはWineか？＝常に真。
inline bool IsWine()					{ return true; }


#endif


