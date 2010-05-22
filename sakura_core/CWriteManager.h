#pragma once


#include "doc/CDocListener.h"
#include "charset/CCodeBase.h"

class CDocLineMgr;
struct SSaveInfo;

class CWriteManager : public CProgressSubject{
public:
	//	Feb. 6, 2001 genta �����ǉ�(���s�R�[�h�ݒ�)
	//	Jul. 26, 2003 ryoji BOM�����ǉ�
	EConvertResult WriteFile_From_CDocLineMgr(
		const CDocLineMgr&	pcDocLineMgr,	//!< [in]
		const SSaveInfo&	sSaveInfo		//!< [in]
	);
};
