﻿/*!	@file
	@brief assert関数

*/
/*
	Copyright (C) 2007, kobake
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
#ifndef SAKURA_DEBUG2_46C93AD6_37D9_4646_B78C_E09168383A42_H_
#define SAKURA_DEBUG2_46C93AD6_37D9_4646_B78C_E09168383A42_H_
#pragma once

#include <cassert>

#include "debug/Debug1.h"
#include "util/MessageBoxF.h"

// C Runtime の定義をundefして独自定義に差し替える
#undef assert

#ifdef _DEBUG

	void debug_exit();
	void warning_point();

	#define assert(exp) \
		if(!(exp)){ \
			TRACE( "!assert: " #exp, NULL ); \
			ErrorMessage( NULL, L"!assert\n%hs(%d):\n%hs", __FILE__, __LINE__, #exp ); \
			debug_exit(); \
		} \
		((void)0)

	#define assert_warning(exp) \
		if(!(exp)){ \
			TRACE( "!warning: " #exp, NULL ); \
			warning_point(); \
		} \
		((void)0)

#else
	#define assert(exp)			((void)0)
	#define assert_warning(exp)	((void)0)
#endif
#endif /* SAKURA_DEBUG2_46C93AD6_37D9_4646_B78C_E09168383A42_H_ */
