/*
	Copyright (C) 2021-2022, Sakura Editor Organization

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
#include <windows.h>
#include <string>
#include "mem/CNativeW.h"
#include "env/CDocTypeManager.h"

TEST(CDocTypeManager, ConvertTypesExtToDlgExtNullptr1)
{
	const std::wstring expected = { L"" };
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(nullptr, nullptr);
	EXPECT_EQ(expected, actual);
}

TEST(CDocTypeManager, ConvertTypesExtToDlgExtNullptr2)
{
	const std::wstring expected = { L"" };
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(nullptr, L".txt");
	EXPECT_EQ(expected, actual);
}

TEST(CDocTypeManager, ConvertTypesExtToDlgExtOnce)
{
	const std::wstring expected = { L"*.txt;*.cpp" };
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(L"cpp", L".txt");
	EXPECT_EQ(expected, actual);
}

TEST(CDocTypeManager, ConvertTypesExtToDlgExtTwo)
{
	const std::wstring expected = { L"*.txt;*.cpp;*.h" };
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(L"cpp;h", L".txt");
	EXPECT_EQ(expected, actual);
}

TEST(CDocTypeManager, ConvertTypesExtToDlgExtThree)
{
	const std::wstring expected = { L"*.txt;*.cpp;*.h;*.hpp" };
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(L"cpp;h;hpp", L".txt");
	EXPECT_EQ(expected, actual);
}

TEST(CDocTypeManager, ConvertTypesExtToDlgExtAppendPeriod)
{
	const std::wstring expected = { L"*.txt;*.cpp;*.h" };
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(L".cpp;.h", L".txt");
	EXPECT_EQ(expected, actual);
}

TEST(CDocTypeManager, ConvertTypesExtToDlgExtSeparatorSpace)
{
	const std::wstring expected = { L"*.txt;*.cpp;*.h" };
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(L"cpp h", L".txt");
	EXPECT_EQ(expected, actual);
}

TEST(CDocTypeManager, ConvertTypesExtToDlgExtSeparatorComma)
{
	const std::wstring expected = { L"*.txt;*.cpp;*.h" } ;
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(L"cpp,h", L".txt");
	EXPECT_EQ(expected, actual);
}

TEST(CDocTypeManager, ConvertTypesExtToDlgExtTopNullptr)
{
	const std::wstring expected = { L"*.cpp;*.h" };
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(L"cpp,h", nullptr);
	EXPECT_EQ(expected, actual);
}

TEST(CDocTypeManager, ConvertTypesExtToDlgExtMerge)
{
	const std::wstring expected = { L"*.txt;*.cpp;*.h" };
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(L"txt,cpp,h", L".txt");
	EXPECT_EQ(expected, actual);
}

TEST(CDocTypeManager, ConvertTypesExtToDlgExtMerge2)
{
	const std::wstring expected = { L"*.txt;*.cpp;*.h" };
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(L"cpp,h,txt", L".txt");
	EXPECT_EQ(expected, actual);
}

TEST(CDocTypeManager, ConvertTypesExtToDlgExtExts64)
{
	const std::wstring expected = { L"*.txt;*.a;*.b;*.c;*.d;*.e;*.f;*.g;*.h;*.i;*.j;*.k;*.l;*.m;*.n;*.o;*.p;*.q;*.r;*.s;*.t;*.u;*.v;*.w;*.x;*.y;*.z;*.1;*.2;*.3;*.4;*.5;*.6" };
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(L"a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,1,2,3,4,5,6", L".txt");
	EXPECT_EQ(expected, actual);
}

TEST(CDocTypeManager, ConvertTypesExtToDlgExtExts64LongFileExt)
{
	const std::wstring expected = { L"*.extension_260_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long;*.a;*.b;*.c;*.d;*.e;*.f;*.g;*.h;*.i;*.j;*.k;*.l;*.m;*.n;*.o;*.p;*.q;*.r;*.s;*.t;*.u;*.v;*.w;*.x;*.y;*.z;*.1;*.2;*.3;*.4;*.5;*.6" };
	std::wstring actual = CDocTypeManager::ConvertTypesExtToDlgExt(L"a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,1,2,3,4,5,6", L".extension_260_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long");
	EXPECT_EQ(expected, actual);
}
