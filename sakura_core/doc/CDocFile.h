#pragma once

#include "CFile.h"
#include "util/file.h"
class CEditDoc;

//####本来はここにあるべきでは無い
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
	bool			IsBomExist() const			{ return m_sFileInfo.bBomExist; }		//!< 保存時にBOMを付加するかどうかを取得
	void			SetBomMode(bool bBomExist)	{ m_sFileInfo.bBomExist = bBomExist; }	//!< 保存時にBOMを付加するかどうかを設定
	CFileTime		GetDocFileTime() const		{ return m_sFileInfo.cFileTime; }
	const TCHAR*	GetFileName() const; //!< ファイル名(パスなし)を取得
public: //####
	CEditDoc*	m_pcDocRef;
	SFileInfo	m_sFileInfo;
};
