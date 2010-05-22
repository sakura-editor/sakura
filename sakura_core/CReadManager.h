#pragma once

#include "doc/CDocListener.h" // CProgressSubject
#include "charset/CCodeBase.h" // EConvertResult

class CDocLineMgr;
struct SFileInfo; // doc/CDocFile.h

class CReadManager : public CProgressSubject{
public:
	//	Nov. 12, 2000 genta ˆø”’Ç‰Á
	//	Jul. 26, 2003 ryoji BOMˆø”’Ç‰Á
	EConvertResult ReadFile_To_CDocLineMgr(
		CDocLineMgr*		pcDocLineMgr,
		const SLoadInfo&	sLoadInfo,
		SFileInfo*			pFileInfo
	);
};
