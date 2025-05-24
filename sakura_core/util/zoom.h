/*!	@file
	@brief ズーム倍率算出
*/
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_ZOOM_0CD4E589_F708_48BF_8601_8BF851827765_H_
#define SAKURA_ZOOM_0CD4E589_F708_48BF_8601_8BF851827765_H_
#pragma once

#include <vector>
#include <initializer_list>

/*!
	@brief ズーム設定を保持
*/
struct ZoomSetting
{
	/*!
		@brief コンストラクタ
		@param[in] iterZoomFactorsFirst	ズーム倍率テーブルの開始イテレータ
		@param[in] iterZoomFactorsLast	ズーム倍率テーブルの終了イテレータ
		@param[in] nValueMin			下限値(上限値以下であること)
		@param[in] nValueMax			上限値(下限値以上であること)
		@param[in] nValueUnit			値の最小単位(0以上であること)
		@note ズーム倍率テーブルは昇順であること
	*/
	template<class InputIter>
	ZoomSetting( InputIter iterZoomFactorsFirst, InputIter iterZoomFactorsLast, double nValueMin, double nValueMax, double nValueUnit ) :
		m_vZoomFactors( iterZoomFactorsFirst, iterZoomFactorsLast ),
		m_nValueMin( nValueMin ),
		m_nValueMax( nValueMax ),
		m_nValueUnit( nValueUnit )
	{}
	ZoomSetting( const std::initializer_list<double> iZoomFactors, double nValueMin, double nValueMax, double nValueUnit ) :
		ZoomSetting( iZoomFactors.begin(), iZoomFactors.end(), nValueMin, nValueMax, nValueUnit )
	{}

	const std::vector<double> m_vZoomFactors;
	const double m_nValueMin;
	const double m_nValueMax;
	const double m_nValueUnit;
};

bool GetZoomedValue( const ZoomSetting& zoomSetting, double nBaseValue, double nCurrentZoom, int nSteps, double* pnValueOut, double* pnZoomOut );
#endif /* SAKURA_ZOOM_0CD4E589_F708_48BF_8601_8BF851827765_H_ */
