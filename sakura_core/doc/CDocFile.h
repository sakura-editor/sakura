/*
	Copyright (C) 2008, kobake
	Copyright (C) 2013, Uchi

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

//####�{���͂����ɂ���ׂ��ł͖���
struct SFileInfo{
	friend class CDocFile;
protected:
	ECodeType	eCharCode;
	bool		bBomExist;
	ECodeType	eCharCodeLoad;
	bool		bBomExistLoad;
	CFileTime	cFileTime;

public:
	SFileInfo()
	{
		eCharCode = eCharCodeLoad = CODE_DEFAULT;
		bBomExist = bBomExistLoad = false;
		cFileTime.ClearFILETIME();
	}
	void	SetCodeSet(ECodeType eSet, bool bBom)	{ eCharCode = eCharCodeLoad = eSet; bBomExist = bBomExistLoad = bBom; }	//!< �����R�[�h�Z�b�g��ݒ�
	void	SetBomExist(bool bBom)					{ bBomExist = bBomExistLoad = bBom; }	//!< BOM�t����ݒ�
	void	SetFileTime( FILETIME& Time )			{ cFileTime.SetFILETIME( Time ); }
};


class CDocFile : public CFile{
public:
	CDocFile(CEditDoc* pcDoc) : m_pcDocRef(pcDoc) {}

	void			SetCodeSet(ECodeType eCodeSet , bool bBomExist)	{ m_sFileInfo.SetCodeSet(eCodeSet, bBomExist); }	//!< �����R�[�h�Z�b�g��ݒ�
	void			SetCodeSetChg(ECodeType eCodeSet , bool bBomExist)	{ m_sFileInfo.eCharCode = eCodeSet; m_sFileInfo.bBomExist = bBomExist; }	//!< �����R�[�h�Z�b�g��ݒ�(�����R�[�h�w��p)
	ECodeType		GetCodeSet() const			{ return m_sFileInfo.eCharCode; }		//!< �����R�[�h�Z�b�g���擾
	void			SetBomDefoult()				{ m_sFileInfo.bBomExist= CCodeTypeName( m_sFileInfo.eCharCode ).IsBomDefOn(); }	//!< BOM�t���̃f�t�H���g�l��ݒ肷��
	void			CancelChgCodeSet()			{ m_sFileInfo.eCharCode = m_sFileInfo.eCharCodeLoad; m_sFileInfo.bBomExist = m_sFileInfo.bBomExistLoad; }		//!< �����R�[�h�Z�b�g1�̕ύX���L�����Z������
	bool			IsBomExist() const			{ return m_sFileInfo.bBomExist; }		//!< �ۑ�����BOM��t�����邩�ǂ������擾
	bool			IsChgCodeSet() const		{ return (!IsFileTimeZero()) && ((m_sFileInfo.eCharCode != m_sFileInfo.eCharCodeLoad) || (m_sFileInfo.bBomExist != m_sFileInfo.bBomExistLoad)); }		//!< �����R�[�h�Z�b�g���ύX���ꂽ���H

	CFileTime&		GetFileTime()				{ return m_sFileInfo.cFileTime; }
	void			ClearFileTime()				{ m_sFileInfo.cFileTime.ClearFILETIME(); }
	bool			IsFileTimeZero() const		{ return m_sFileInfo.cFileTime.IsZero(); }	// �V�K�t�@�C���H
	const SYSTEMTIME	GetFileSysTime() const	{ return m_sFileInfo.cFileTime.GetSYSTEMTIME(); }
	void			SetFileTime( FILETIME& Time )	{ m_sFileInfo.cFileTime.SetFILETIME( Time ); }

	const TCHAR*	GetFileName() const{ return GetFileTitlePointer(GetFilePath()); }	//!< �t�@�C����(�p�X�Ȃ�)���擾
	const TCHAR*	GetSaveFilePath(void) const;
	void			SetSaveFilePath(LPCTSTR pszPath){ m_szSaveFilePath.Assign(pszPath); }
public: //####
	CEditDoc*	m_pcDocRef;
	SFileInfo	m_sFileInfo;
	CFilePath	m_szSaveFilePath;	/* �ۑ����̃t�@�C���̃p�X�i�}�N���p�j */	// 2006.09.04 ryoji
};

#endif /* SAKURA_CDOCFILE_C6DA01C5_5BB2_4361_9B6A_648953CB9CA19_H_ */
/*[EOF]*/
