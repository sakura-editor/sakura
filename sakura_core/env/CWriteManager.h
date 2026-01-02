/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CWRITEMANAGER_1FC8A485_C76D_40D7_8505_5EECFCD98155_H_
#define SAKURA_CWRITEMANAGER_1FC8A485_C76D_40D7_8505_5EECFCD98155_H_
#pragma once

#include "doc/CDocListener.h"
#include "charset/CCodeBase.h"

class CDocLineMgr;
struct SSaveInfo;

class CWriteManager : public CProgressSubject{
public:
	//	Feb. 6, 2001 genta 引数追加(改行コード設定)
	//	Jul. 26, 2003 ryoji BOM引数追加
	EConvertResult WriteFile_From_CDocLineMgr(
		const CDocLineMgr&	pcDocLineMgr,	//!< [in]
		const SSaveInfo&	sSaveInfo		//!< [in]
	);
};
#endif /* SAKURA_CWRITEMANAGER_1FC8A485_C76D_40D7_8505_5EECFCD98155_H_ */
