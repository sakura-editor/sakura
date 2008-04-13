#include "stdafx.h"
#include "CDocFile.h"
#include "util/window.h"

CDocFile::CDocFile(CEditDoc* pcDoc)
: m_pcDocRef(pcDoc)
{
}


/*! �t�@�C����(�p�X�Ȃ�)���擾����
	@author Moca
	@date 2002.10.13
*/
const TCHAR* CDocFile::GetFileName() const
{
	return GetFileTitlePointer(GetFilePath());
}
