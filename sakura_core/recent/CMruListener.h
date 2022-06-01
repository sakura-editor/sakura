﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CMRULISTENER_8E94E46A_3AEE_4030_B866_12F24E862C49_H_
#define SAKURA_CMRULISTENER_8E94E46A_3AEE_4030_B866_12F24E862C49_H_
#pragma once

#include "doc/CDocListener.h"

class CMruListener : public CDocListenerEx{
public:
	//ロード前後
//	ECallbackResult OnCheckLoad(SLoadInfo* pLoadInfo) override;
	void OnBeforeLoad(SLoadInfo* sLoadInfo) override;
	void OnAfterLoad(const SLoadInfo& sLoadInfo) override;

	//セーブ前後
	void OnAfterSave(const SSaveInfo& sSaveInfo) override;

	//クローズ前後
	ECallbackResult OnBeforeClose() override;

protected:
	//ヘルパ
	void _HoldBookmarks_And_AddToMRU(); // Mar. 30, 2003 genta
};
#endif /* SAKURA_CMRULISTENER_8E94E46A_3AEE_4030_B866_12F24E862C49_H_ */
