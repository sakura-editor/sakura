#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_FAST_COMPILE
#include "catch.hpp"

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

TEST_CASE("Append", "[test]")
{
	CHECK(Add(1, 2) == 3);
	CHECK(Add(2, 3) == 5);
}

TEST_CASE("DivideByZero", "[test]")
{
	CHECK_THROWS_AS(Divide(2, 0), DivideByZeroException);
}

TEST_CASE("PointerSize", "[test]")
{
	int pointerSize = 0;
#if defined(_WIN64)
	pointerSize = sizeof(__int64);
#elif  defined(_WIN32)
	pointerSize = sizeof(int);
#else
#endif
	printf("pointerSize = %d\n", pointerSize);
	CHECK(sizeof(void*) == pointerSize);
}
