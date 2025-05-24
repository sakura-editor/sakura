/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CHARSETDETECTOR_23D1755A_0001_4BAE_96C1_C3BD492E6BD0_H_
#define SAKURA_CHARSETDETECTOR_23D1755A_0001_4BAE_96C1_C3BD492E6BD0_H_
#pragma once

#include <string_view>

#include "extmodule/CIcu4cI18n.h"
#include "extmodule/CUchardet.h"

/*!
 * @brief 文字コード検出クラス
 */
class CharsetDetector final
{
	CIcu4cI18n _icuin;
	UCharsetDetector* _csd;

	CUchardet _uchardet;
	uchardet_t _ud = nullptr;

public:
	CharsetDetector() noexcept;
	~CharsetDetector() noexcept;

	bool IsAvailable() const noexcept {
		return _icuin.IsAvailable() || _uchardet.IsAvailable();
	}

	ECodeType Detect(const std::string_view& bytes);
};
#endif /* SAKURA_CHARSETDETECTOR_23D1755A_0001_4BAE_96C1_C3BD492E6BD0_H_ */
