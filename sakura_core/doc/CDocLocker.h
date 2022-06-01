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
#ifndef SAKURA_CDOCLOCKER_6B8C1CF4_B0FD_4631_ADB4_E53776A2FAAE_H_
#define SAKURA_CDOCLOCKER_6B8C1CF4_B0FD_4631_ADB4_E53776A2FAAE_H_
#pragma once

#include "doc/CDocListener.h"

class CDocLocker : public CDocListenerEx{
public:
	CDocLocker();

	//クリア
	void Clear(void) { m_bIsDocWritable = true; }

	//ロード前後
	void OnAfterLoad(const SLoadInfo& sLoadInfo) override;
	
	//セーブ前後
	void OnBeforeSave(const SSaveInfo& sSaveInfo) override;
	void OnAfterSave(const SSaveInfo& sSaveInfo) override;

	//状態
	bool IsDocWritable() const{ return m_bIsDocWritable; }

	//チェック
	void CheckWritable(bool bMsg);

private:
	bool m_bIsDocWritable;
};
#endif /* SAKURA_CDOCLOCKER_6B8C1CF4_B0FD_4631_ADB4_E53776A2FAAE_H_ */
