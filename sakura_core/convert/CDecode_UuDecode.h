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
#include "util/string_ex.h"

class CDecode_UuDecode final : public CDecode{
private:
	SFilePath m_aFilename;

public:
	bool DoDecode(const CNativeW& cData, CMemory* pDst) override;

	template <basis::WritableBuffer<WCHAR> A>
	void CopyFilename(A& dst) const
	{
		wcscpy_s(dst, m_aFilename);
	}
};

#endif /* SAKURA_CDECODE_UUDECODE_E299AA83_2825_45E6_866A_A08D73F8F47F_H_ */
