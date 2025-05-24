/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
