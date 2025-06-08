/*! @file */
/*
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
#include "pch.h"
#include <cstdio>
#include <exception>

class DivideByZeroException : public std::exception
{
public:
	explicit DivideByZeroException(const char* message)
#ifdef _MSC_VER
        : std::exception(message)
#else
        : std::exception()
#endif
    {
    }
};

int Add(int a, int b)
{
	return a + b;
}

int Divide(int a, int b)
{
	if (b == 0)
	{
		throw DivideByZeroException("Divide By Zero");	
	}
	return a / b;
}

TEST(test, Append)
{
	EXPECT_EQ(Add(1, 2), 3);
	ASSERT_EQ(Add(2, 3), 5);
}

TEST(test, DivideByZero)
{
	ASSERT_THROW(Divide(2, 0), DivideByZeroException);
}

TEST(test, PointerSize)
{
	int pointerSize = 0;
#if defined(_WIN64)
	pointerSize = sizeof(__int64);
#elif  defined(_WIN32)
	pointerSize = sizeof(int);
#else
#endif
	printf("pointerSize = %d\n", pointerSize);
	ASSERT_EQ(sizeof(void*), pointerSize);
}
