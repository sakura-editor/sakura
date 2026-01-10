/*!	@file
	@brief Unix-to-Unix Decode

	@author 
*/

/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDECODE_UUDECODE_E299AA83_2825_45E6_866A_A08D73F8F47F_H_
#define SAKURA_CDECODE_UUDECODE_E299AA83_2825_45E6_866A_A08D73F8F47F_H_
#pragma once

#include "convert/CDecode.h"

class CDecode_UuDecode final : public CDecode{
private:
	SFilePath m_aFilename;

public:
	bool DoDecode(const CNativeW& cData, CMemory* pDst) override;
	void CopyFilename(std::span<WCHAR> dst) const { ::wcsncpy_s(std::data(dst), std::size(dst), m_aFilename, _TRUNCATE); }
};

#endif /* SAKURA_CDECODE_UUDECODE_E299AA83_2825_45E6_866A_A08D73F8F47F_H_ */
