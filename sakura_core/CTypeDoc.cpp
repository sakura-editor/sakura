#include "stdafx.h"
#include "CTypeDoc.h"

void CTypeDoc::OnBeforeLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	/* 共有データ構造体のアドレスを返す */
	CDocumentType	doctype = CShareData::getInstance()->GetDocumentType( pcDoc->m_cDocFile.GetFilePath() );
	pcDoc->m_cDocType.SetDocumentType( doctype, true );
}
