#include "StdAfx.h"
#include "CDocFile.h"
#include "util/window.h"

CDocFile::CDocFile(CEditDoc* pcDoc)
: m_pcDocRef(pcDoc)
{
}


/*! ファイル名(パスなし)を取得する
	@author Moca
	@date 2002.10.13
*/
const TCHAR* CDocFile::GetFileName() const
{
	return GetFileTitlePointer(GetFilePath());
}
