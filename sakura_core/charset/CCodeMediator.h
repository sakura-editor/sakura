/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCODEMEDIATOR_26846888_8435_4071_81A6_9DB01C8FB718_H_
#define SAKURA_CCODEMEDIATOR_26846888_8435_4071_81A6_9DB01C8FB718_H_
#pragma once

#include "types/CType.h" //SEncodingConfig

/*!
 * @brief CCodeMediator クラス
 * 
 * 日本語コードセット判別の詳細を隠ぺいするための仲介クラスです。
 */
class CCodeMediator final {
public:
	explicit CCodeMediator(const SEncodingConfig &encodingConfig) noexcept
		: m_sEncodingConfig(encodingConfig)
	{
	}

	/* 日本語コードセット判別 */
	ECodeType CheckKanjiCode(const char* buff, size_t size) noexcept;
	/* ファイルの日本語コードセット判別 */
	ECodeType CheckKanjiCodeOfFile(const WCHAR* pszFile);

private:
	const SEncodingConfig& m_sEncodingConfig;
};
#endif /* SAKURA_CCODEMEDIATOR_26846888_8435_4071_81A6_9DB01C8FB718_H_ */
