#pragma once

#include "doc/CDocListener.h"
class CDocLineMgr;

class CReadManager : public CProgressSubject{
public:
	//	Nov. 12, 2000 genta �����ǉ�
	//	Jul. 26, 2003 ryoji BOM�����ǉ�
	EConvertResult ReadFile_To_CDocLineMgr(
		CDocLineMgr*		pcDocLineMgr,
		const SLoadInfo&	sLoadInfo,
		SFileInfo*			pFileInfo
	);
};
