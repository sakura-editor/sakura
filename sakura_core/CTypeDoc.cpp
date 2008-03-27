#include "stdafx.h"
#include "CTypeDoc.h"

void CTypeDoc::OnBeforeLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	CDocumentType	doctype = CShareData::getInstance()->GetDocumentType( pcDoc->m_cDocFile.GetFilePath() );
	pcDoc->m_cDocType.SetDocumentType( doctype, true );
}
