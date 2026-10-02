/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "env/ShareDataTestSuite.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

#include "env/CommonSetting.h"
#include "uiparts/CImageListMgr.h"
#include "uiparts/CMenuDrawer.h"

/*!
 * @brief アイコンビットマップを差し替えたCImageListMgr
 *
 * DrawToolIconはm_hIconBitmapがnullptrなら即座に失敗するため、
 * 素のCImageListMgrではアイコン番号の範囲チェックまで到達できない。
 * メンバがprotectedなので、派生クラスから偽のアイコンシートを流し込む。
 *
 * ピクセルバッファ版のDrawToolIconはm_pBitsを直接読むだけでGDIを使わないので、
 * ビットマップリソースもデバイスコンテキストも用意しなくてよい。
 */
class TestImageListMgr : public CImageListMgr {
public:
	//! アイコンiconCount個ぶんの画素バッファを用意する
	void SetUpFakeIcons(int iconCount)
	{
		m_nIconCount = iconCount;

		// アイコンはMAX_TOOLBAR_ICON_X桁の格子に並ぶ
		const auto rows = (iconCount + MAX_TOOLBAR_ICON_X - 1) / MAX_TOOLBAR_ICON_X;
		m_bmpWidth  = MAX_TOOLBAR_ICON_X * m_cx;
		m_bmpHeight = rows * m_cy;
		m_pixels.assign(size_t(m_bmpWidth) * m_bmpHeight, 0);

		// どのアイコンを読んだか判別できるように、各アイコンを番号で塗り分ける
		for (int imageNo = 0; imageNo < iconCount; ++imageNo) {
			const auto sx = (imageNo % MAX_TOOLBAR_ICON_X) * m_cx;
			const auto sy = (imageNo / MAX_TOOLBAR_ICON_X) * m_cy;
			for (int y = 0; y < m_cy; ++y) {
				for (int x = 0; x < m_cx; ++x) {
					m_pixels[size_t(m_bmpWidth) * (sy + y) + sx + x] = PixelOf(imageNo);
				}
			}
		}

		m_pBits = std::data(m_pixels);
		m_hIconBitmap = reinterpret_cast<HBITMAP>(1);	// nullptr判定を通すためのダミー
	}

	//! アイコン番号に対応する画素値
	static constexpr uint32_t PixelOf(int imageNo) noexcept
	{
		return 0xFF000000u | uint32_t(imageNo);
	}

	~TestImageListMgr()
	{
		// ダミーのハンドルなのでDeleteObjectに渡してはならない
		m_hIconBitmap = nullptr;
		m_pBits = nullptr;
	}

private:
	std::vector<uint32_t> m_pixels;
};

/*!
 * 有効なアイコン番号は0からアイコン数-1まで。
 *
 * 修正前は上限の比較がm_nIconCount < imageNoとなっており、
 * アイコン数そのものを有効な番号として通していた。
 */
TEST(CImageListMgr, DrawToolIconRejectsIconNumberOutOfRange)
{
	constexpr int iconCount = 8;
	TestImageListMgr icons;
	icons.SetUpFakeIcons(iconCount);
	ASSERT_EQ(icons.Count(), iconCount);

	std::vector<uint32_t> pixels(size_t(icons.cx()) * icons.cy(), 0);

	// アイコン数そのものは範囲外。修正前はこれを通していた
	EXPECT_FALSE(icons.DrawToolIcon(std::data(pixels), iconCount, true, icons.cx(), icons.cy()));

	EXPECT_FALSE(icons.DrawToolIcon(std::data(pixels), iconCount + 1, true, icons.cx(), icons.cy()));
	EXPECT_FALSE(icons.DrawToolIcon(std::data(pixels), -1, true, icons.cx(), icons.cy()));
}

/*!
 * 範囲内のアイコン番号は描画され、指定したアイコンの内容が書き出される。
 */
TEST(CImageListMgr, DrawToolIconDrawsIconInRange)
{
	constexpr int iconCount = 8;
	TestImageListMgr icons;
	icons.SetUpFakeIcons(iconCount);

	std::vector<uint32_t> pixels(size_t(icons.cx()) * icons.cy(), 0);

	for (int imageNo = 0; imageNo < iconCount; ++imageNo) {
		ASSERT_TRUE(icons.DrawToolIcon(std::data(pixels), imageNo, true, icons.cx(), icons.cy()))
			<< "imageNo = " << imageNo;
		EXPECT_THAT(pixels, testing::Each(TestImageListMgr::PixelOf(imageNo))) << "imageNo = " << imageNo;
	}
}

/*!
 * 初期状態のアイコン数はビットマップの格子をちょうど埋める。
 *
 * このためアイコン数そのものを通してしまうと、ビットマップの外側を読むことになる。
 * 修正前のピクセルバッファ版は確保領域外を参照していた。
 */
TEST(CImageListMgr, IconCountFillsTheWholeBitmapGrid)
{
	const CImageListMgr icons;
	EXPECT_EQ(icons.Count(), MAX_TOOLBAR_ICON_COUNT);
	EXPECT_EQ(MAX_TOOLBAR_ICON_COUNT, MAX_TOOLBAR_ICON_X * MAX_TOOLBAR_ICON_Y);

	// アイコン数と同じ番号は、格子の外（MAX_TOOLBAR_ICON_Y段目の次）を指す
	EXPECT_EQ(MAX_TOOLBAR_ICON_COUNT / MAX_TOOLBAR_ICON_X, MAX_TOOLBAR_ICON_Y);
}

/*!
 * アイコンビットマップが無い場合は何も描画しない。
 */
TEST(CImageListMgr, DrawToolIconFailsWithoutBitmap)
{
	const CImageListMgr icons;
	std::vector<uint32_t> pixels(size_t(icons.cx()) * icons.cy(), 0);

	EXPECT_FALSE(icons.DrawToolIcon(std::data(pixels), 0, true, icons.cx(), icons.cy()));
	EXPECT_FALSE(icons.DrawToolIcon(HDC(nullptr), 0, 0, 0, true, icons.cx(), icons.cy()));
}

/*!
 * HDC版も同じ範囲チェックを行う。
 *
 * 範囲外なら描画に進まないので、デバイスコンテキストが無くても検証できる。
 * 範囲内の描画はGDIのAlphaBlendを呼ぶため、ここでは対象にしない。
 */
TEST(CImageListMgr, DrawToolIconToDcRejectsIconNumberOutOfRange)
{
	constexpr int iconCount = 8;
	TestImageListMgr icons;
	icons.SetUpFakeIcons(iconCount);

	EXPECT_FALSE(icons.DrawToolIcon(HDC(nullptr), 0, 0, iconCount, true, icons.cx(), icons.cy()));
	EXPECT_FALSE(icons.DrawToolIcon(HDC(nullptr), 0, 0, iconCount + 1, true, icons.cx(), icons.cy()));
	EXPECT_FALSE(icons.DrawToolIcon(HDC(nullptr), 0, 0, -1, true, icons.cx(), icons.cy()));
}

/*!
 * @brief プラグインのアイコン(Add)のテスト
 *
 * 16x16の赤いBMPを用意する。左上の1画素だけ白にして、透過色として扱わせる。
 * CMenuDrawerが共有メモリを使うのでShareDataTestSuiteを継承する。
 */
struct PluginIconTest : public ::testing::Test, public env::ShareDataTestSuite {
	static constexpr uint32_t RED = 0xFFFF0000;

	static void SetUpTestSuite()
	{
		SetUpShareData();
	}

	static void TearDownTestSuite()
	{
		TearDownShareData();
	}

	void SetUp() override
	{
		BITMAPINFOHEADER bih = { sizeof(BITMAPINFOHEADER), 16, 16, 1, 24, BI_RGB };
		BITMAPFILEHEADER bfh = { 0x4D42 };	// "BM"
		bfh.bfOffBits = sizeof(bfh) + sizeof(bih);
		bfh.bfSize = bfh.bfOffBits + 16 * 16 * 3;

		std::ofstream out(bmpPath, std::ios::binary);
		out.write(reinterpret_cast<const char*>(&bfh), sizeof(bfh));
		out.write(reinterpret_cast<const char*>(&bih), sizeof(bih));
		for (int i = 0; i < 16 * 16; ++i) {
			const bool topLeft = (i == 16 * 15);	// ボトムアップなので最終行の先頭が左上
			const char bgr[3] = { char(topLeft ? 0xFF : 0), char(topLeft ? 0xFF : 0), char(0xFF) };
			out.write(bgr, sizeof(bgr));
		}
		out.close();

		ASSERT_TRUE(icons.Create(G_AppInstance()));
	}

	void TearDown() override
	{
		std::filesystem::remove(bmpPath);
	}

	std::vector<uint32_t> Draw(int imageNo) const
	{
		std::vector<uint32_t> pixels(size_t(icons.cx()) * icons.cy(), 0);
		EXPECT_TRUE(icons.DrawToolIcon(std::data(pixels), imageNo, true, icons.cx(), icons.cy()));
		return pixels;
	}

	std::filesystem::path bmpPath = std::filesystem::temp_directory_path() / L"sakura-test-icon.bmp";
	CImageListMgr icons;
};

/*!
 * 既定アイコンで埋まった状態でAddすると、ビットマップが1行拡張される。
 * 追加したアイコンを描画できること。透過色の画素は透明になる。
 *
 * #2627 修正前は拡張後もm_pBitsが古いビットマップを指していて、範囲外を読んでいた。
 */
TEST_F(PluginIconTest, DrawAddedIcon)
{
	const int imageNo = icons.Add(bmpPath.c_str());
	ASSERT_EQ(imageNo, MAX_TOOLBAR_ICON_COUNT);

	const auto pixels = Draw(imageNo);
	EXPECT_EQ(pixels[0], 0u);
	EXPECT_EQ(pixels[std::size(pixels) / 2], RED);
}

/*!
 * ResetExtendの後にAddすると、同じ番号で追加し直される。
 */
TEST_F(PluginIconTest, AddAfterResetExtend)
{
	ASSERT_EQ(icons.Add(bmpPath.c_str()), MAX_TOOLBAR_ICON_COUNT);

	icons.ResetExtend();
	ASSERT_EQ(icons.Count(), MAX_TOOLBAR_ICON_COUNT);

	const int imageNo = icons.Add(bmpPath.c_str());
	ASSERT_EQ(imageNo, MAX_TOOLBAR_ICON_COUNT);

	const auto pixels = Draw(imageNo);
	EXPECT_EQ(pixels[std::size(pixels) / 2], RED);
}

/*!
 * CMenuDrawerのCreateより後に追加したアイコンも、メニューに表示できる。
 *
 * #2627 修正前はCreate時点のアイコン数で配列を確保していたので、範囲外を参照していた。
 */
TEST_F(PluginIconTest, MenuBitmapOfAddedIcon)
{
	CMenuDrawer drawer;
	drawer.Create(G_AppInstance(), nullptr, &icons);

	constexpr int funcCode = F_PLUGCOMMAND_FIRST + 1;
	drawer.AddToolButton(icons.Add(bmpPath.c_str()), funcCode);

	GetDllShareData().m_Common.m_sWindow.m_bMenuIcon = TRUE;
	const auto hMenu = ::CreatePopupMenu();
	drawer.MyAppendMenu(hMenu, MF_STRING, funcCode, L"", L"", FALSE);
	drawer.MyAppendMenu(hMenu, MF_STRING, funcCode, L"", L"", FALSE);

	MENUITEMINFO first = { sizeof(first), MIIM_BITMAP };
	MENUITEMINFO second = { sizeof(second), MIIM_BITMAP };
	::GetMenuItemInfo(hMenu, 0, TRUE, &first);
	::GetMenuItemInfo(hMenu, 1, TRUE, &second);
	::DestroyMenu(hMenu);

	EXPECT_NE(first.hbmpItem, nullptr);
	EXPECT_EQ(second.hbmpItem, first.hbmpItem);	// 2回目は作成済みのものを使う
}
