/*! @file
	@brief Mac 版：文件相关的 Win32 替代函数和 io/CBinaryStream 的测试
*/
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "io/CBinaryStream.h"

#include <filesystem>
#include <fstream>

#include <unistd.h>

namespace {

//! 测试用的临时文件夹（文件名含日文，确认 UTF-16 → UTF-8 路径转换）
struct MacIoTest : public ::testing::Test {
	std::filesystem::path dir;
	sakura::wstring wdir;

	void SetUp() override {
		dir = std::filesystem::temp_directory_path() / std::filesystem::path(u8"sakura_mac_io_テスト_") += std::to_string(::getpid());
		std::filesystem::create_directories(dir);
		wdir = sakura::path_to_wstring(dir);
	}
	void TearDown() override {
		std::error_code ec;
		std::filesystem::permissions(dir / u8"読取専用.txt", std::filesystem::perms::owner_write, std::filesystem::perm_options::add, ec);
		std::filesystem::remove_all(dir, ec);
	}
	sakura::wstring wpath(const char8_t* name) const {
		return wdir + LTEXT("/") + sakura::path_to_wstring(std::filesystem::path(name));
	}
};

} // namespace

TEST_F(MacIoTest, BinaryStreamRoundTrip)
{
	const auto path = wpath(u8"日本語.bin");
	const char data[] = "abc\0\x01\xFF\r\n";
	{
		CBinaryOutputStream out(path.c_str(), true);
		ASSERT_TRUE(out.Good());
		EXPECT_EQ(out.Write(data, sizeof(data)), int(sizeof(data)));
	}
	CBinaryInputStream in(path.c_str());
	ASSERT_TRUE(in.Good());
	EXPECT_EQ(in.GetLength(), int(sizeof(data)));
	char buf[sizeof(data)] = {};
	EXPECT_EQ(in.Read(buf, sizeof(buf)), int(sizeof(data)));
	EXPECT_EQ(0, memcmp(buf, data, sizeof(data)));
}

TEST_F(MacIoTest, OpenMissingFileThrowsInExceptionMode)
{
	const auto path = wpath(u8"ない/ファイル.bin");
	EXPECT_THROW(CBinaryOutputStream(path.c_str(), true), CError_FileOpen);
	CBinaryInputStream in(path.c_str());
	EXPECT_FALSE(in.Good());
}

TEST_F(MacIoTest, FileAttributes)
{
	EXPECT_EQ(GetFileAttributesW(wpath(u8"存在しない").c_str()), INVALID_FILE_ATTRIBUTES);
	EXPECT_EQ(GetLastError(), DWORD(ERROR_FILE_NOT_FOUND));

	EXPECT_EQ(GetFileAttributesW(wdir.c_str()) & FILE_ATTRIBUTE_DIRECTORY, DWORD(FILE_ATTRIBUTE_DIRECTORY));

	std::ofstream(dir / u8"普通.txt") << "x";
	EXPECT_EQ(GetFileAttributesW(wpath(u8"普通.txt").c_str()), DWORD(FILE_ATTRIBUTE_NORMAL));

	std::ofstream(dir / u8".隠し") << "x";
	EXPECT_NE(GetFileAttributesW(wpath(u8".隠し").c_str()) & FILE_ATTRIBUTE_HIDDEN, 0u);

	std::ofstream(dir / u8"読取専用.txt") << "x";
	const auto ro = wpath(u8"読取専用.txt");
	EXPECT_TRUE(SetFileAttributesW(ro.c_str(), FILE_ATTRIBUTE_READONLY));
	EXPECT_NE(GetFileAttributesW(ro.c_str()) & FILE_ATTRIBUTE_READONLY, 0u);
	EXPECT_TRUE(SetFileAttributesW(ro.c_str(), FILE_ATTRIBUTE_NORMAL));
	EXPECT_EQ(GetFileAttributesW(ro.c_str()) & FILE_ATTRIBUTE_READONLY, 0u);
}

TEST(MacIo, WfopenIgnoresWindowsOnlyModes)
{
	const auto dir = std::filesystem::temp_directory_path();
	const auto p = dir / ("sakura_mac_wfopen_" + std::to_string(::getpid()) + ".txt");
	const auto wp = sakura::path_to_wstring(p);
	FILE* fp = _wfopen(wp.c_str(), LTEXT("wt, ccs=UTF-8"));
	ASSERT_NE(fp, nullptr);
	fputs("ok", fp);
	fclose(fp);
	fp = _wfopen(wp.c_str(), LTEXT("rb"));
	ASSERT_NE(fp, nullptr);
	char buf[4] = {};
	EXPECT_EQ(fread(buf, 1, 3, fp), 2u);
	fclose(fp);
	EXPECT_STREQ(buf, "ok");
	std::filesystem::remove(p);
}
