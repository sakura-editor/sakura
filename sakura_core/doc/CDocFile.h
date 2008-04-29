#pragma once

#include "io/CFile.h"
#include "util/file.h"
class CEditDoc;

//####�{���͂����ɂ���ׂ��ł͖���
struct SFileInfo{
	ECodeType	eCharCode;
	CFileTime	cFileTime;
	bool		bBomExist;

	SFileInfo()
	{
		eCharCode = CODE_DEFAULT;
		cFileTime.ClearFILETIME();
		bBomExist = false;
	}
};

class CDocFile : public CFile{
public:
	CDocFile(CEditDoc* pcDoc);
	bool			IsBomExist() const			{ return m_sFileInfo.bBomExist; }		//!< �ۑ�����BOM��t�����邩�ǂ������擾
	void			SetBomMode(bool bBomExist)	{ m_sFileInfo.bBomExist = bBomExist; }	//!< �ۑ�����BOM��t�����邩�ǂ�����ݒ�
	CFileTime		GetDocFileTime() const		{ return m_sFileInfo.cFileTime; }
	const TCHAR*	GetFileName() const; //!< �t�@�C����(�p�X�Ȃ�)���擾
public: //####
	CEditDoc*	m_pcDocRef;
	SFileInfo	m_sFileInfo;
};
