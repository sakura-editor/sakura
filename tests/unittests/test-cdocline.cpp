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
#include "doc/logic/CDocLine.h"

#include <string_view>

TEST(CDocLine, IsEmptyLine)
{
	{
		CDocLine line;
		EXPECT_TRUE(line.IsEmptyLine());
	}
	{
		CDocLine line;
		line.SetDocLineString(L"空行ではない\n", 6, false);
		EXPECT_FALSE(line.IsEmptyLine());
	}
	{
		CDocLine line;
		line.SetDocLineString(L" \t\r\n", 4, false);
		EXPECT_TRUE(line.IsEmptyLine());
	}
}

TEST(CDocLine, GetLengthWithoutEOL)
{
	{
		CDocLine line;
		EXPECT_EQ(line.GetLengthWithoutEOL(), CLogicInt(0));
	}
	{
		CDocLine line;
		line.SetDocLineString(L"改行がありません", 8, false);
		EXPECT_EQ(line.GetLengthWithoutEOL(), CLogicInt(8));
	}
	{
		CDocLine line;
		line.SetDocLineString(L"LFがあります\n", 8, false);
		EXPECT_EQ(line.GetLengthWithoutEOL(), CLogicInt(7));
	}
	{
		CDocLine line;
		line.SetDocLineString(L"CRLFがあります\r\n", 11, false);
		EXPECT_EQ(line.GetLengthWithoutEOL(), CLogicInt(9));
	}
}

TEST(CDocLine, GetPtr)
{
	{
		CDocLine line;
		EXPECT_EQ(nullptr, line.GetPtr());
	}
	{
		std::wstring_view s = L"てきとうなもじれつ";
		CDocLine line;
		line.SetDocLineString(s.data(), s.length(), false);
		EXPECT_STREQ(line.GetPtr(), s.data());
	}
}

TEST(CDocLine, GetLengthWithEOL)
{
	{
		CDocLine line;
		EXPECT_EQ(line.GetLengthWithEOL(), CLogicInt(0));
	}
	{
		CDocLine line;
		line.SetDocLineString(L"改行がありません", 8, false);
		EXPECT_EQ(line.GetLengthWithEOL(), CLogicInt(8));
	}
	{
		CDocLine line;
		line.SetDocLineString(L"LFがあります\n", 8, false);
		EXPECT_EQ(line.GetLengthWithEOL(), CLogicInt(8));
	}
	{
		CDocLine line;
		line.SetDocLineString(L"CRLFがあります\r\n", 11, false);
		EXPECT_EQ(line.GetLengthWithEOL(), CLogicInt(11));
	}
}

TEST(CDocLine, GetDocLineStrWithEOL)
{
	{
		CDocLine line;
		line.SetDocLineString(L"もじれつ", 4, false);
		CLogicInt n(123);
		EXPECT_EQ(line.GetDocLineStrWithEOL(&n), line.GetPtr());
		EXPECT_EQ(n, line.GetLengthWithEOL());

		n = 123;
		EXPECT_EQ(CDocLine::GetDocLineStrWithEOL_Safe(&line, &n), line.GetPtr());
		EXPECT_EQ(n, line.GetLengthWithEOL());

		// USE_STRICT_INT 定義時は別の関数が呼ばれる
		int n2 = 123;
		EXPECT_EQ(line.GetDocLineStrWithEOL(&n), line.GetPtr());
		EXPECT_EQ(n, line.GetLengthWithEOL());
	}
	{
		CLogicInt n(123);
		EXPECT_EQ(CDocLine::GetDocLineStrWithEOL_Safe(nullptr, &n), nullptr);
		EXPECT_EQ(n, CLogicInt(0));

#ifdef _MSC_VER
		n = 123;
		CDocLine* p = reinterpret_cast<CDocLine*>(0);
		EXPECT_EQ(p->GetDocLineStrWithEOL(&n), nullptr);
		EXPECT_EQ(n, CLogicInt(0));

		// USE_STRICT_INT 定義時は別の関数が呼ばれる
		int n2 = 123;
		EXPECT_EQ(p->GetDocLineStrWithEOL(&n), nullptr);
		EXPECT_EQ(n, 0);
#endif
	}
}

TEST(CDocLine, GetStringRefWithEOL)
{
	{
		CDocLine line;
		line.SetDocLineString(L"もじれつ", 4, false);
		CStringRef ref = line.GetStringRefWithEOL();
		EXPECT_EQ(ref.GetPtr(), line.GetPtr());
		EXPECT_EQ(ref.GetLength(), line.GetLengthWithEOL());

		ref = CDocLine::GetStringRefWithEOL_Safe(&line);
		EXPECT_EQ(ref.GetPtr(), line.GetPtr());
		EXPECT_EQ(ref.GetLength(), line.GetLengthWithEOL());
	}
	{
		CLogicInt n(123);
		CStringRef ref = CDocLine::GetStringRefWithEOL_Safe(nullptr);
		EXPECT_EQ(ref.GetPtr(), nullptr);
		EXPECT_EQ(ref.GetLength(), 0);

#ifdef _MSC_VER
		CDocLine* p = reinterpret_cast<CDocLine*>(0);
		ref = p->GetStringRefWithEOL();
		EXPECT_EQ(ref.GetPtr(), nullptr);
		EXPECT_EQ(ref.GetLength(), 0);
#endif
	}
}


TEST(CDocLine, GetEol)
{
	{
		CDocLine line;
		CEol eol;
		EXPECT_EQ(line.GetEol(), eol);
	}
	{
		CDocLine line;
		line.SetDocLineString(L"\n", 1, false);
		CEol eol(EEolType::line_feed);
		EXPECT_EQ(line.GetEol(), eol);
	}
}

TEST(CDocLine, SetEol)
{
	// 改行コードを再設定するテスト
	CDocLine line;
	line.SetDocLineString(L"てすと", 3, false);
	EXPECT_STREQ(line.GetPtr(), L"てすと");

	line.SetEol(CEol(EEolType::cr_and_lf), nullptr);
	EXPECT_STREQ(line.GetPtr(), L"てすと\r\n");

	line.SetEol(CEol(EEolType::line_feed), nullptr);
	EXPECT_STREQ(line.GetPtr(), L"てすと\n");

	line.SetEol(CEol(EEolType::none), nullptr);
	EXPECT_STREQ(line.GetPtr(), L"てすと");
}

TEST(CDocLine, GetDocLineData)
{
	const CNativeW data(L"データ\n");
	CDocLine line;
	line.SetDocLineString(data, false);
	EXPECT_EQ(line._GetDocLineDataWithEOL(), data);
	EXPECT_EQ(line._GetDocLineData(), data);
}

TEST(CDocLine, SetDocLineStringMove)
{
	CNativeW data(L"データ\r\n");
	const wchar_t* originalPtr = data.GetStringPtr();

	CDocLine line;
	line.SetDocLineStringMove(&data, false);
	EXPECT_EQ(line.GetPtr(), originalPtr);
	EXPECT_EQ(line.GetEol(), CEol(EEolType::cr_and_lf));
	EXPECT_EQ(data.GetStringPtr(), nullptr);
}

TEST(CDocLine, GetPrevAndNextLine)
{
	CDocLine line;
	EXPECT_EQ(line.GetPrevLine(), nullptr);
	EXPECT_EQ(line.GetNextLine(), nullptr);

	CDocLine prev;
	CDocLine next;
	line._SetPrevLine(&prev);
	EXPECT_EQ(line.GetPrevLine(), &prev);
	line._SetNextLine(&next);
	EXPECT_EQ(line.GetNextLine(), &next);
}
