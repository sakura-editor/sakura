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
