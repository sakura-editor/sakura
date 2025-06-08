/*!	@file
	@brief ズーム倍率算出
*/
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "zoom.h"
#include <algorithm>
#include <cmath>

/*!
	@brief 値テーブル上における指定値の位置を取得
	@param[in] vTable 値テーブル
	@param[in] nValue 指定値
	@return 位置
	@note 「位置」は基本的にはテーブルのインデックスに相当しますが、
	      テーブル範囲外およびテーブル上の隣り合う二値間については 0.5 で表現します。
	@note 例として vTable:{10, 20, 30} の場合の nValue と戻り値との関係を示します。
	nValue |    0 |    5 |   10 |   15 |   25 |   30 |   35 |   40 |
	-------|------|------|------|------|------|------|------|------|
	戻り値 | -0.5 | -0.5 |  0.0 |  0.5 |  1.5 |  2.0 |  2.5 |  2.5 |
*/
[[nodiscard]] static double GetPositionInTable( const std::vector<double>& vTable, double nValue )
{
	double nPosition = (double)vTable.size() - 0.5;
	for( size_t i = 0; i < vTable.size(); ++i ){
		if( nValue <= vTable[i] ){
			if( nValue == vTable[i] ){
				nPosition = (double)i;
			}else{
				nPosition = (double)i - 0.5;
			}
			break;
		}
	}
	return nPosition;
}

/*!
	@brief 最小単位で丸めた値を取得
	@param[in] nValue	対象値
	@param[in] nUnit	最小単位(0.0 の場合は丸めなし)
	@return 丸められた値
*/
[[nodiscard]] static double GetQuantizedValue( double nValue, double nUnit )
{
	return (0.0 < nUnit) ? std::floor( nValue / nUnit ) * nUnit : nValue;
}

/*!
	@brief 拡大方向の移動量を反映したズーム倍率テーブルのインデックスを取得
	@param[in] zoomSetting		ズーム設定
	@param[in] nBaseValue		基準値
	@param[in] nCurrentValue	現在値
	@param[in] nCurrentIndex	現在値に対応するインデックス
	@param[in] nUpSteps			インデックスの移動量(1以上であること)
	@return ズーム倍率テーブルのインデックス
*/
[[nodiscard]] static int GetZoomUpIndex( const ZoomSetting& zoomSetting, double nBaseValue, double nCurrentValue, int nCurrentIndex, int nUpSteps )
{
	int nIndex = nCurrentIndex + nUpSteps;
	const int nIndexMax = (int)zoomSetting.m_vZoomFactors.size() - 1;
	for( ; nIndex < nIndexMax; ++nIndex ){
		double nNextValue = nBaseValue * zoomSetting.m_vZoomFactors[nIndex];
		nNextValue = GetQuantizedValue( nNextValue, zoomSetting.m_nValueUnit );
		if( nNextValue != nCurrentValue ){
			break;
		}
	}

	return std::min( nIndex, nIndexMax );
}

/*!
	@brief 縮小方向の移動量を反映したズーム倍率テーブルのインデックスを取得
	@param[in] zoomSetting		ズーム設定
	@param[in] nBaseValue		基準値
	@param[in] nCurrentValue	現在値
	@param[in] nCurrentIndex	現在値に対応するインデックス
	@param[in] nDownSteps		インデックスの移動量(1以上であること)
	@return ズーム倍率テーブルのインデックス
*/
[[nodiscard]] static int GetZoomDownIndex( const ZoomSetting& zoomSetting, double nBaseValue, double nCurrentValue, int nCurrentIndex, int nDownSteps )
{
	int nIndex = nCurrentIndex - nDownSteps;
	double nLastValue = nCurrentValue;
	bool bFindingOneMoreChange = false;
	for( ; 0 <= nIndex; --nIndex ){
		double nValue = nBaseValue * zoomSetting.m_vZoomFactors[nIndex];
		nValue = GetQuantizedValue( nValue, zoomSetting.m_nValueUnit );
		if( bFindingOneMoreChange && nValue != nLastValue ){
			break;
		}else if( nValue != nCurrentValue ){
			// もう一度値が変化するまでインデックスを進める
			bFindingOneMoreChange = true;
			nLastValue = nValue;
		}else{
			// 値が変化しなかったので次へ
		}
	}

	if( bFindingOneMoreChange ){
		++nIndex;
	}

	return std::max( 0, nIndex );
}

/*!
	@brief 基準値に対してズーム倍率を適用した値を取得
	@param[in] zoomSetting	ズーム設定
	@param[in] nBaseValue	基準値
	@param[in] nCurrentZoom	変更前のズーム倍率
	@param[in] nSteps		ズーム段階の変更量
	@param[out] pnValueOut	変更後のズーム倍率を適用した値
	@param[out] pnZoomOut	変更後のズーム倍率
	@return ズームできたかどうか
	@note 戻り値が false の場合には pnValueOut, pnZoomOut は設定されません。
*/
bool GetZoomedValue( const ZoomSetting& zoomSetting, double nBaseValue, double nCurrentZoom, int nSteps, double* pnValueOut, double* pnZoomOut )
{
	if( nSteps == 0 ){
		return false;
	}

	const bool bZoomUp = (0 < nSteps);
	const int nIndexMin = 0;
	const int nIndexMax = (int)zoomSetting.m_vZoomFactors.size() - 1;

	const double nPosition = GetPositionInTable( zoomSetting.m_vZoomFactors, nCurrentZoom );
	auto nCurrentIndex = (int)(bZoomUp ? std::floor( nPosition ) : std::ceil( nPosition ));
	if( (!bZoomUp && nCurrentIndex <= nIndexMin) || (bZoomUp && nIndexMax <= nCurrentIndex) ){
		// 現在の倍率がすでに倍率テーブルの範囲外でかつ
		// さらに外側へ移動しようとした場合は今の位置を維持
		return false;
	}

	const double nCurrentValue = GetQuantizedValue( nBaseValue * nCurrentZoom, zoomSetting.m_nValueUnit );
	const double nValueMin = std::min( {zoomSetting.m_nValueMin, nBaseValue, nCurrentValue} );
	const double nValueMax = std::max( {zoomSetting.m_nValueMax, nBaseValue, nCurrentValue} );

	int nNextIndex;
	if( bZoomUp ){
		nNextIndex = GetZoomUpIndex( zoomSetting, nBaseValue, nCurrentValue, nCurrentIndex, nSteps );
	}else{
		nNextIndex = GetZoomDownIndex( zoomSetting, nBaseValue, nCurrentValue, nCurrentIndex, -nSteps );
	}
	double nNextZoom = zoomSetting.m_vZoomFactors[nNextIndex];
	double nNextValue = GetQuantizedValue( nBaseValue * nNextZoom, zoomSetting.m_nValueUnit );

	if( nNextValue < nValueMin || nValueMax < nNextValue ){
		// 値の上下限を超過していたら上下限で丸める
		// 倍率は丸めた後のサイズで再計算
		nNextValue = std::clamp( nNextValue, nValueMin, nValueMax );
		if( nBaseValue != 0.0 ){
			nNextZoom = nNextValue / nBaseValue;
		}
	}

	if( nCurrentValue == nNextValue ){
		return false;
	}

	if( pnValueOut != nullptr ){
		*pnValueOut = nNextValue;
	}
	if( pnZoomOut != nullptr ){
		*pnZoomOut = nNextZoom;
	}

	return true;
}
