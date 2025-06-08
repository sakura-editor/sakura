/*!	@file
	@brief Unix-to-Unix Decode

	@author 
*/

/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDECODE_UUDECODE_E299AA83_2825_45E6_866A_A08D73F8F47F_H_
#define SAKURA_CDECODE_UUDECODE_E299AA83_2825_45E6_866A_A08D73F8F47F_H_
#pragma once

#include "convert/CDecode.h"

class CDecode_UuDecode final : public CDecode{

	WCHAR m_aFilename[_MAX_PATH];
public:
	bool DoDecode(const CNativeW& cData, CMemory* pDst) override;
	void CopyFilename( WCHAR *pcDst ) const { wcscpy( pcDst, m_aFilename ); }
};
#endif /* SAKURA_CDECODE_UUDECODE_E299AA83_2825_45E6_866A_A08D73F8F47F_H_ */
