/*! @file */
/*
	Copyright (C) 2008, kobake

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
#pragma once

#include "types/CType.h" //SEncodingConfig

/*!
 * @brief CCodeMediator クラス
 * 
 * 日本語コードセット判別の詳細を隠ぺいするための仲介クラスです。
 */
class CCodeMediator final
{
  public:
    explicit CCodeMediator(const SEncodingConfig &encodingConfig) noexcept
        : m_sEncodingConfig(encodingConfig)
    {
    }

    /* 日本語コードセット判別 */
    ECodeType CheckKanjiCode(const char *buff, size_t size) noexcept;
    /* ファイルの日本語コードセット判別 */
    ECodeType CheckKanjiCodeOfFile(const WCHAR *pszFile);

  private:
    const SEncodingConfig &m_sEncodingConfig;
};
