/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_CREADMANAGER_BF5A195D_BEA1_4508_8BC7_DB5316B5B66E_H_
#define SAKURA_CREADMANAGER_BF5A195D_BEA1_4508_8BC7_DB5316B5B66E_H_
#pragma once

#include "doc/CDocListener.h" // CProgressSubject
#include "charset/CCodeBase.h" // EConvertResult

class CDocLineMgr;
struct SFileInfo; // doc/CDocFile.h

class CReadManager : public CProgressSubject{
public:
	//	Nov. 12, 2000 genta 引数追加
	//	Jul. 26, 2003 ryoji BOM引数追加
	EConvertResult ReadFile_To_CDocLineMgr(
		CDocLineMgr*		pcDocLineMgr,
		const SLoadInfo&	sLoadInfo,
		SFileInfo*			pFileInfo
	);
};
#endif /* SAKURA_CREADMANAGER_BF5A195D_BEA1_4508_8BC7_DB5316B5B66E_H_ */
