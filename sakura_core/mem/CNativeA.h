/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CNATIVEA_03C02187_A42C_4403_9D24_8B4CA20EEA81_H_
#define SAKURA_CNATIVEA_03C02187_A42C_4403_9D24_8B4CA20EEA81_H_
#pragma once

#include "CNative.h"

/*!
 * マルチバイト文字列管理クラス
 *
 * @author kobake
 * @date 2007/11/06 kobake 新規作成
 * @deprecated use std::string instead
 */
using CNativeA = CNative<CHAR>;

// CMemory派生クラスにはメンバー追加禁止
static_assert(sizeof(CNativeA) == sizeof(CMemory), "size check");

#endif /* SAKURA_CNATIVEA_03C02187_A42C_4403_9D24_8B4CA20EEA81_H_ */
