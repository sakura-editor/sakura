/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_CMYSIZE_E407E800_A14E_408D_966A_E626DE7ECA01_H_
#define SAKURA_CMYSIZE_E407E800_A14E_408D_966A_E626DE7ECA01_H_
#pragma once

#include <Windows.h> //SIZE

class CMySize : public SIZE{
public:
	//コンストラクタ・デストラクタ
	CMySize(){} //※初期化なし
	CMySize(int _cx,int _cy){ cx=_cx; cy=_cy; }
	CMySize(const SIZE& rhs){ cx=rhs.cx; cy=rhs.cy; }

	//関数
	void Set(int _cx,int _cy){ cx=_cx; cy=_cy; }

	//演算子
	bool operator == (const SIZE& rhs) const{ return cx==rhs.cx && cy==rhs.cy; }
	bool operator != (const SIZE& rhs) const{ return !operator==(rhs); }
};
#endif /* SAKURA_CMYSIZE_E407E800_A14E_408D_966A_E626DE7ECA01_H_ */
