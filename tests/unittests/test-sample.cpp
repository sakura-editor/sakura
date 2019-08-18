#include "doctest.h"
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

TEST_CASE("Append")
{
	CHECK(Add(1, 2) == 3);
	REQUIRE(Add(2, 3) == 5);
}

TEST_CASE("DivideByZero")
{
	REQUIRE_THROWS_AS(Divide(2, 0), DivideByZeroException);
}

TEST_CASE("PointerSize")
{
	int pointerSize = 0;
#if defined(_WIN64)
	pointerSize = sizeof(__int64);
#elif  defined(_WIN32)
	pointerSize = sizeof(int);
#else
#endif
	printf("pointerSize = %d\n", pointerSize);
	REQUIRE(sizeof(void*) == pointerSize);
}
