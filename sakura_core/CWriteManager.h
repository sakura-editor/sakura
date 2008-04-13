#pragma once


#include "doc/CDocListener.h"

class CWriteManager : public CProgressSubject{
public:
	//	Feb. 6, 2001 genta 引数追加(改行コード設定)
	//	Jul. 26, 2003 ryoji BOM引数追加
	EConvertResult WriteFile_From_CDocLineMgr(
		const CDocLineMgr&	pcDocLineMgr,	//!< [in]
		const SSaveInfo&	sSaveInfo		//!< [in]
	);
};
