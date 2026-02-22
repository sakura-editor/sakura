/*! @file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"

// テストではGMockを使う前提にする
#include <gmock/gmock.h>

#include "Funccode_enum.h"

// マッチャーのusing（追加するときは昇順で。）
using ::testing::_;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::Invoke;
using ::testing::IsEmpty;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::Le;
using ::testing::Lt;
using ::testing::Ne;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::StrCaseEq;
using ::testing::StrCaseNe;
using ::testing::StrEq;
using ::testing::StrNe;

//! googletestに機能IDを出力させる
void PrintTo(EFunctionCode eFuncCode, std::ostream* os);
