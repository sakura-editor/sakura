/*!	@file
	@brief assert関数

*/
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
