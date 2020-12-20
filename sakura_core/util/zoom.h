/*!	@file
	@brief ズーム倍率算出
*/
/*
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

#ifndef SAKURA_ZOOM_0CD4E589_F708_48BF_8601_8BF851827765_H_
#define SAKURA_ZOOM_0CD4E589_F708_48BF_8601_8BF851827765_H_
#pragma once

#include <initializer_list>
#include <vector>

/*!
	@brief ズーム設定を保持
*/
class ZoomSetting
{
public:
	ZoomSetting( std::initializer_list<double> iZoomFactors, double nValueMin, double nValueMax, double nValueUnit );

	bool IsValid() const { return m_bValid; }
	const std::vector<double>& GetZoomFactors() const { return m_vZoomFactors; }
	double GetValueMin() const { return m_nValueMin; }
	double GetValueMax() const { return m_nValueMax; }
	double GetValueUnit() const { return m_nValueUnit; }
private:
	bool m_bValid;
	std::vector<double> m_vZoomFactors;
	double m_nValueMin;
	double m_nValueMax;
	double m_nValueUnit;
};

bool GetZoomedValue( const ZoomSetting& zoomSetting, double nBaseValue, double nCurrentZoom, int nSteps, double* pnValueOut, double* pnZoomOut );
#endif /* SAKURA_ZOOM_0CD4E589_F708_48BF_8601_8BF851827765_H_ */
