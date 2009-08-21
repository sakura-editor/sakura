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
	const TCHAR*	GetSaveFilePath(void) const { return (m_szSaveFilePath.IsValidPath())? m_szSaveFilePath: GetFilePath(); }
	void			SetSaveFilePath(LPCTSTR pszPath){ m_szSaveFilePath.Assign(pszPath); }
public: //####
	CEditDoc*	m_pcDocRef;
	SFileInfo	m_sFileInfo;
	CFilePath	m_szSaveFilePath;	/* �ۑ����̃t�@�C���̃p�X�i�}�N���p�j */	// 2006.09.04 ryoji
};
