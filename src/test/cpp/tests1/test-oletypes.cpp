/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "_os/OleTypes.h"
#include <string>

/*!
	@brief SysString constructor/destructor.
 */
TEST(OleTypes_SysString, constructor)
{
	// constructor
	{
		SysString target1;
		std::wstring str_wchar = L"wchar";
		SysString target2(str_wchar.c_str(), str_wchar.size());
		std::string str_char = "char";
		SysString target3(str_char.c_str(), str_char.size());
		SysString target4(target2);
		SysString target5(target3.Data);

		EXPECT_EQ(nullptr, target1.Data);
		EXPECT_NE(nullptr, target2.Data);
		EXPECT_NE(nullptr, target3.Data);
		EXPECT_NE(nullptr, target4.Data);
		EXPECT_NE(nullptr, target5.Data);

		std::string s1, s2, s3, s4, s5;
		target1.Get(&s1);
		target2.Get(&s2);
		target3.Get(&s3);
		target4.Get(&s4);
		target5.Get(&s5);
		EXPECT_STREQ("", s1.c_str());
		EXPECT_STREQ("wchar", s2.c_str());
		EXPECT_STREQ("char", s3.c_str());
		EXPECT_STREQ("wchar", s4.c_str());
		EXPECT_STREQ("char", s5.c_str());
	}

	// destructor (pointer)
	{
		SysString* p1 = new SysString;
		std::wstring str_wchar = L"wchar";
		SysString* p2 = new SysString(str_wchar.c_str(), str_wchar.size());
		std::string str_char = "char";
		SysString* p3 = new SysString(str_char.c_str(), str_char.size());
		SysString* p4 = new SysString(*p2);
		SysString* p5 = new SysString(p3->Data);

		std::string t1, t2, t3, t4, t5;
		p1->Get(&t1);
		p2->Get(&t2);
		p3->Get(&t3);
		p4->Get(&t4);
		p5->Get(&t5);
		EXPECT_STREQ("", t1.c_str());
		EXPECT_STREQ("wchar", t2.c_str());
		EXPECT_STREQ("char", t3.c_str());
		EXPECT_STREQ("wchar", t4.c_str());
		EXPECT_STREQ("char", t5.c_str());

		delete p1;
		delete p2;
		delete p3;
		delete p4;
		delete p5;
	}
}

/*!
	@brief SysString operator=.
 */
TEST(OleTypes_SysString, operator_eq)
{
	// GitHub #2465 メモリリークする.
	SysString src1;
	std::wstring str_wchar2 = L"wchar";
	SysString src2(str_wchar2.c_str(), str_wchar2.size());
	std::string str_char3 = "char";
	SysString src3(str_char3.c_str(), str_char3.size());
	std::string str_char4 = "source";
	SysString src4(str_char4.c_str(), str_char4.size());

	// Data=nullptr <== Data=variation
	SysString dst1, dst2, dst3;
	dst1 = src1;
	dst2 = src2;
	dst3 = src3;
	// Data=not_nullptr <== Data=variation
	SysString dst4(src4), dst5(src4);
	dst4 = src2;
	dst5 = src1;

	std::string s1, s2, s3, s4, s5;
	dst1.Get(&s1);
	dst2.Get(&s2);
	dst3.Get(&s3);
	dst4.Get(&s4);
	dst5.Get(&s5);
	EXPECT_STREQ("", s1.c_str());
	EXPECT_STREQ("wchar", s2.c_str());
	EXPECT_STREQ("char", s3.c_str());
	EXPECT_STREQ("wchar", s4.c_str());
	EXPECT_STREQ("", s5.c_str());

	// GitHub #2465 自分自身をコピーすると正しくない.
	// copy myself
	SysString dst6 = src2;
	dst6 = dst6;
	std::string s6;
	dst6.Get(&s6);
	EXPECT_STREQ("wchar", s6.c_str());
}

/*!
	@brief SysString Length.
 */
TEST(OleTypes_SysString, Length)
{
	SysString target1;
	std::wstring str_wchar = L"wchar";
	SysString target2(str_wchar.c_str(), str_wchar.size());
	std::string str_char = "char";
	SysString target3(str_char.c_str(), str_char.size());
	SysString target4(target2);
	SysString target5(target3.Data);

	EXPECT_EQ(0, target1.Length());
	EXPECT_EQ(5, target2.Length());
	EXPECT_EQ(4, target3.Length());
	EXPECT_EQ(5, target4.Length());
	EXPECT_EQ(4, target5.Length());
}

/*!
	@brief SysString Get/GetW.
 */
TEST(OleTypes_SysString, Get)
{
	SysString target1;
	std::wstring str_wchar = L"wchar";
	SysString target2(str_wchar.c_str(), str_wchar.size());
	std::string str_char = "char";
	SysString target3(str_char.c_str(), str_char.size());
	SysString target4(target2);
	SysString target5(target3.Data);

	// Get(char**, int*)
	{
		char *s1 = nullptr;
		char *s2 = nullptr;
		char *s3 = nullptr;
		char *s4 = nullptr;
		char *s5 = nullptr;
		int n1 = 9;
		int n2 = 9;
		int n3 = 9;
		int n4 = 9;
		int n5 = 9;
		target1.Get(&s1, &n1);
		target2.Get(&s2, &n2);
		target3.Get(&s3, &n3);
		target4.Get(&s4, &n4);
		target5.Get(&s5, &n5);
		EXPECT_STREQ("", s1);
		EXPECT_STREQ("wchar", s2);
		EXPECT_STREQ("char", s3);
		EXPECT_STREQ("wchar", s4);
		EXPECT_STREQ("char", s5);
		EXPECT_EQ(0, n1);
		EXPECT_EQ(5, n2);
		EXPECT_EQ(4, n3);
		EXPECT_EQ(5, n4);
		EXPECT_EQ(4, n5);
	}

	// Get(wchar_t**, int*)
	{
		wchar_t *s1 = nullptr;
		wchar_t *s2 = nullptr;
		wchar_t *s3 = nullptr;
		wchar_t *s4 = nullptr;
		wchar_t *s5 = nullptr;
		int n1 = 9;
		int n2 = 9;
		int n3 = 9;
		int n4 = 9;
		int n5 = 9;
		target1.GetW(&s1, &n1);
		target2.GetW(&s2, &n2);
		target3.GetW(&s3, &n3);
		target4.GetW(&s4, &n4);
		target5.GetW(&s5, &n5);
		EXPECT_STREQ(L"", s1);
		EXPECT_STREQ(L"wchar", s2);
		EXPECT_STREQ(L"char", s3);
		EXPECT_STREQ(L"wchar", s4);
		EXPECT_STREQ(L"char", s5);
		EXPECT_EQ(0, n1);
		EXPECT_EQ(5, n2);
		EXPECT_EQ(4, n3);
		EXPECT_EQ(5, n4);
		EXPECT_EQ(4, n5);
	}

	// Get(std::string)
	{
		std::string s1, s2, s3, s4, s5;
		target1.Get(&s1);
		target2.Get(&s2);
		target3.Get(&s3);
		target4.Get(&s4);
		target5.Get(&s5);
		EXPECT_STREQ("", s1.c_str());
		EXPECT_STREQ("wchar", s2.c_str());
		EXPECT_STREQ("char", s3.c_str());
		EXPECT_STREQ("wchar", s4.c_str());
		EXPECT_STREQ("char", s5.c_str());
	}

	// GetW(std::wstring)
	{
		std::wstring s1, s2, s3, s4, s5;
		target1.GetW(&s1);
		target2.GetW(&s2);
		target3.GetW(&s3);
		target4.GetW(&s4);
		target5.GetW(&s5);
		EXPECT_STREQ(L"", s1.c_str());
		EXPECT_STREQ(L"wchar", s2.c_str());
		EXPECT_STREQ(L"char", s3.c_str());
		EXPECT_STREQ(L"wchar", s4.c_str());
		EXPECT_STREQ(L"char", s5.c_str());
	}
}

/*!
	@brief Variant constructor/destructor.
 */
TEST(OleTypes_Variant, constructor)
{
	// GitHub #2466 メンバ変数の初期化漏れ.
	// constructor
	{
		Variant target1;
		EXPECT_EQ(VT_EMPTY, target1.Data.vt);

		Variant src2;
		src2.Receive(123);
		Variant target2(src2);
		ASSERT_EQ(VT_I4, target2.Data.vt);
		EXPECT_EQ(123, target2.Data.lVal);

		VARIANT src3;
		::VariantInit(&src3);
		src3.vt = VT_I4;
		src3.lVal = 456;
		Variant target3(src3);
		ASSERT_EQ(VT_I4, target3.Data.vt);
		EXPECT_EQ(456, target3.Data.lVal);
	}

	// destructor (pointer)
	{
		Variant *p1 = new Variant;
		ASSERT_EQ(VT_EMPTY, p1->Data.vt);
		delete p1;

		Variant src2;
		src2.Receive(123);
		Variant *p2 = new Variant(src2);
		ASSERT_EQ(VT_I4, p2->Data.vt);
		EXPECT_EQ(123, p2->Data.lVal);
		delete p2;

		VARIANT src3;
		::VariantInit(&src3);
		src3.vt = VT_I4;
		src3.lVal = 456;
		Variant *p3 = new Variant(src3);
		ASSERT_EQ(VT_I4, p3->Data.vt);
		EXPECT_EQ(456, p3->Data.lVal);
		delete p3;
	}
}

/*!
	@brief Variant operator=.
 */
TEST(OleTypes_Variant, operator_eq)
{
	// GitHub #2466 メモリリークする.
	// string
	std::string str_char = "char";
	SysString str1(str_char.c_str(), str_char.size());
	Variant src1;
	src1.Receive(str1);
	Variant dst1 = src1;
	ASSERT_EQ(VT_BSTR, dst1.Data.vt);
	EXPECT_STREQ(L"char", dst1.Data.bstrVal);

	// int
	Variant src2;
	src2.Receive(123);
	Variant dst2 = src2;
	ASSERT_EQ(VT_I4, dst2.Data.vt);
	EXPECT_EQ(123, dst2.Data.lVal);

	// GitHub #2466 自分自身をコピーすると正しくない.
	// copy myself
	Variant src3;
	src3.Receive(456);
	src3 = src3;
	ASSERT_EQ(VT_I4, src3.Data.vt);
	EXPECT_EQ(456, src3.Data.lVal);
}

/*!
	@brief Variant Receive.
 */
TEST(OleTypes_Variant, Receive)
{
	// Receive(SysString)
	{
		SysString src1;
		Variant target1;
		target1.Receive(src1);
		ASSERT_EQ(VT_BSTR, target1.Data.vt);
		EXPECT_EQ(nullptr, target1.Data.bstrVal);
		EXPECT_EQ(nullptr, src1.Data);

		std::string str2 = "char";
		SysString src2(str2.c_str(), str2.size());
		Variant target2;
		target2.Receive(src2);
		ASSERT_EQ(VT_BSTR, target2.Data.vt);
		EXPECT_STREQ(L"char", target2.Data.bstrVal);
		EXPECT_EQ(nullptr, src2.Data);
	}

	// Receive(int)
	{
		Variant target3;
		target3.Receive(123);
		ASSERT_EQ(VT_I4, target3.Data.vt);
		EXPECT_EQ(123, target3.Data.lVal);
	}
}

/*!
	@brief Variant Wrap.
 */
TEST(OleTypes_Variant, Wrap)
{
	VARIANT src;
	::VariantInit(&src);
	src.vt = VT_I4;
	src.lVal = 123;
	Variant* p = Wrap(&src);
	ASSERT_EQ(VT_I4, p->Data.vt);
	EXPECT_EQ(123, p->Data.lVal);

	p->Receive(456);
	ASSERT_EQ(VT_I4, p->Data.vt);
	EXPECT_EQ(456, p->Data.lVal);
	ASSERT_EQ(VT_I4, src.vt);
	EXPECT_EQ(456, src.lVal);
}

/*!
	@brief SysString Wrap.
 */
TEST(OleTypes_SysString, Wrap)
{
	BSTR src;
	src = ::SysAllocString(L"hello");
	SysString* p = Wrap(&src);
	std::string str;
	p->Get(&str);
	EXPECT_STREQ("hello", str.c_str());
	p = nullptr;
	::SysFreeString(src);
}
