/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CDOCFILE_C6DA01C5_5BB2_4361_9B6A_648953CB9CA19_H_
#define SAKURA_CDOCFILE_C6DA01C5_5BB2_4361_9B6A_648953CB9CA19_H_

#include "io/CFile.h"
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
	const TCHAR*	GetSaveFilePath(void) const {
		if (m_szSaveFilePath.IsValidPath()) {
			return m_szSaveFilePath;
		} else {
			return GetFilePath();
		}
	}
	void			SetSaveFilePath(LPCTSTR pszPath){ m_szSaveFilePath.Assign(pszPath); }
public: //####
	CEditDoc*	m_pcDocRef;
	SFileInfo	m_sFileInfo;
	CFilePath	m_szSaveFilePath;	/* 保存時のファイルのパス（マクロ用） */	// 2006.09.04 ryoji
};

#endif /* SAKURA_CDOCFILE_C6DA01C5_5BB2_4361_9B6A_648953CB9CA19_H_ */
/*[EOF]*/
