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
#ifndef SAKURA_CWRITEMANAGER_2F063818_9A9D_4BEF_830F_987AEBBCBB07_H_
#define SAKURA_CWRITEMANAGER_2F063818_9A9D_4BEF_830F_987AEBBCBB07_H_

#include "doc/CDocListener.h"
#include "charset/CCodeBase.h"

class CDocLineMgr;
struct SSaveInfo;

class CWriteManager : public CProgressSubject{
public:
	//	Feb. 6, 2001 genta 引数追加(改行コード設定)
	//	Jul. 26, 2003 ryoji BOM引数追加
	EConvertResult WriteFile_From_CDocLineMgr(
		const CDocLineMgr&	pcDocLineMgr,	//!< [in]
		const SSaveInfo&	sSaveInfo		//!< [in]
	);
};

#endif /* SAKURA_CWRITEMANAGER_2F063818_9A9D_4BEF_830F_987AEBBCBB07_H_ */
/*[EOF]*/
