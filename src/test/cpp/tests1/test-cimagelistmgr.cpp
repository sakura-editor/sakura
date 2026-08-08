/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include <vector>

#include "env/CommonSetting.h"
#include "uiparts/CImageListMgr.h"

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
