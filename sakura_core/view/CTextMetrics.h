/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CTEXTMETRICS_7972A864_FDFF_4852_9EA5_A91D39657A7F_H_
#define SAKURA_CTEXTMETRICS_7972A864_FDFF_4852_9EA5_A91D39657A7F_H_
#pragma once

//2007.08.25 kobake 追加

#include <array>
#include <vector>
#include <Windows.h>
#include "basis/SakuraBasis.h"
#include "charset/charcode.h"
#include "debug/Debug2.h"

class CTextMetrics{
public:
	void CopyTextMetricsStatus(CTextMetrics* pDst) const;
	void Update(HDC hdc, HFONT hFont, int nLineSpace, int nColmSpace);

	//! 半角文字の横幅を取得。単位はピクセル。
	[[nodiscard]] int GetHankakuWidth() const { return m_nCharWidth; }
	//! 半角文字の縦幅を取得。単位はピクセル。
	[[nodiscard]] int GetHankakuHeight() const { return m_nCharHeight; }
	//! 半角文字の文字間隔を取得。単位はピクセル。
	[[nodiscard]] int GetHankakuDx() const { return m_nDxBasis; }
	//! Y方向文字間隔。文字縦幅＋行間隔。単位はピクセル。
	[[nodiscard]] int GetHankakuDy() const { return m_nDyBasis; }

	[[nodiscard]] CPixelXInt GetCharSpacing() const {
		return GetHankakuDx() - GetHankakuWidth();
	}
	// レイアウト幅分のピクセル幅を取得する
	[[nodiscard]] int GetCharPxWidth(CLayoutXInt col) const {
		return static_cast<Int>(col);
	}
	[[nodiscard]] int GetCharPxWidth() const { return 1; }
	[[nodiscard]] int GetCharHeightMarginByFontNo(int) const { return 0; }

	// 固定文字x桁のレイアウト幅を取得する
	[[nodiscard]] CLayoutXInt GetLayoutXDefault(CKetaXInt chars) const {
		return CLayoutXInt(m_nDxBasis * static_cast<Int>(chars));
	}
	// 固定文字1桁あたりのレイアウト幅を取得する
	[[nodiscard]] CLayoutXInt GetLayoutXDefault() const{ return GetLayoutXDefault(CKetaXInt(1)); }
	
	//! 半角文字列の文字間隔配列を取得。要素数は64。
	[[nodiscard]] const int* GetDxArray_AllHankaku() const { return m_anHankakuDx.data(); }

	const int* GenerateDxArray2(
		std::vector<int>* vResultArray, //!< [out] 文字間隔配列の受け取りコンテナ
		const wchar_t* pText,           //!< [in]  文字列
		int nLength,                    //!< [in]  文字列長
		CCharWidthCache& cache = GetCharWidthCache()
	) const {
		return GenerateDxArray(vResultArray, pText, nLength,
			m_nDxBasis, 8, 0, m_nDxBasis - m_nCharWidth, cache);
	}

	//! 指定した文字列により文字間隔配列を生成する。
	static const int* GenerateDxArray(
		std::vector<int>* vResultArray, //!< [out] 文字間隔配列の受け取りコンテナ
		const wchar_t* pText,           //!< [in]  文字列
		int nLength,                    //!< [in]  文字列長
		int	nHankakuDx,					//!< [in]  半角文字の文字間隔
		int	nTabSpace = 8,				//   [in]  TAB幅
		int	nIndent = 0,				//   [in]  インデント
		int nCharSpacing = 0,			//   [in]  文字の間隔
		CCharWidthCache& cache = GetCharWidthCache()
	);

	//!文字列のピクセル幅を返す。
	[[nodiscard]] static int CalcTextWidth(
		const wchar_t* pText, //!< 文字列
		int nLength,          //!< 文字列長
		const int* pnDx       //!< 文字間隔の入った配列
	);

	//!文字列のピクセル幅を返す。
	static int CalcTextWidth2(
		const wchar_t* pText, //!< 文字列
		int nLength,          //!< 文字列長
		int nHankakuDx,       //!< 半角文字の文字間隔
		int nCharSpacing,     //!< 文字の隙間
		std::vector<int>& vDxArray, //!< [out] 文字間隔配列
		CCharWidthCache& cache = GetCharWidthCache()
	);

	int CalcTextWidth3(
		const wchar_t* pText, //!< 文字列
		int nLength,          //!< 文字列長
		CCharWidthCache& cache = GetCharWidthCache()
	) const;

private:
	int	m_nCharWidth;      //!< 半角文字の横幅
	int m_nCharHeight;     //!< 半角文字の縦幅
	int m_nDxBasis;        //!< 半角文字の文字間隔 (横幅+α)
	int m_nDyBasis;        //!< 半角文字の行間隔 (縦幅+α)
	std::array<int, 64> m_anHankakuDx; //!< 半角用文字間隔配列
};
#endif /* SAKURA_CTEXTMETRICS_7972A864_FDFF_4852_9EA5_A91D39657A7F_H_ */
