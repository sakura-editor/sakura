/*! @file
	@brief ダークモード判定ヘルパー

	darkmodelib の判定APIをラップし、ダークモードの有効判定を
	一箇所に集約する。

	SPDX-License-Identifier: Zlib
*/
#pragma once

#include <DarkModeSubclass.h>

/*! ダークモードが有効かつ実際にダーク表示すべきかを判定する

	DarkMode::isEnabled()          … ライブラリが有効化されているか (classic以外)
	DarkMode::isExperimentalActive() … Windowsが実際にダークモードか

	両方が true のときのみダーク用の描画を行う。
*/
inline bool IsDarkModeActive()
{
	return DarkMode::isEnabled() && DarkMode::isExperimentalActive();
}

/*! 共有データのダークモード設定をライブラリに反映する

	@param bDarkMode  共有データの m_bDarkMode 値
*/
inline void ApplyDarkModeSetting(BOOL bDarkMode)
{
	const auto dmType = bDarkMode
		? DarkMode::DarkModeType::dark : DarkMode::DarkModeType::light;
	DarkMode::setDarkModeConfigEx(static_cast<UINT>(dmType));
	DarkMode::setDefaultColors(true);
}
