//	$Id$
/*!	@file
	@brief �R�}���h���C���p�[�T �w�b�_�t�@�C��

	@author aroka
	@date	2002/01/08 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CEditApp��蕪��

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CCOMMANDLINE_H_
#define _CCOMMANDLINE_H_

#include "global.h"
#include "CShareData.h"	// FileInfo, GrepInfo
class CMemory;
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/

/*!
	@brief �R�}���h���C���p�[�T �N���X
*/
class SAKURA_CORE_API CCommandLine {
public:
	static CCommandLine* Instance(LPSTR cmd=NULL);

private:
	static void ParseCommandLine(
		LPCSTR		pszCmdLineSrc,	//!< [in]�R�}���h���C��������
		bool*		pbGrepMode,	//!< [out] true: Grep Mode
		bool*		pbGrepDlg,	//!< [out] true: Grep Dialog�\��
		CMemory*	pcmGrepKey,	//!< [out] Grep��Key
		CMemory*	pcmGrepFile,
		CMemory*	pcmGrepFolder,
		bool*		pbGrepSubFolder,
		bool*		pbGrepLoHiCase,
		bool*		pbGrepRegularExp,
		int *		pnGrepCharSet,
		bool*		pbGrepOutputLine,
		bool*		pbGrepWordOnly,
		int	*		pnGrepOutputStyle,
		bool*		pbDebugMode,
		bool*		pbNoWindow,	//!< [out] TRUE: �ҏWWindow���J���Ȃ�
		FileInfo*	pfi,
		bool*		pbReadOnly	//!< [out] TRUE: Read Only
	);
	static int CCommandLine::CheckCommandLine(
		LPSTR  str, //!< [in] ���؂��镶����i�擪��-�͊܂܂Ȃ��j
		char** arg	//!< [out] ����������ꍇ�͂��̐擪�ւ̃|�C���^
	);
	
	// �O�����点�Ȃ��B
	CCommandLine();
	CCommandLine(LPSTR cmd);

// member accessor method
public:
	bool IsNoWindow() const {return m_bNoWindow;};
	bool IsGrepMode() const {return m_bGrepMode;};
	bool IsGrepDlg() const {return m_bGrepDlg;};
	bool IsDebugMode() const {return m_bDebugMode;};
	bool IsReadOnly() const {return m_bReadOnly;};
	bool GetFileInfo(FileInfo& fi) const {fi = m_fi;return true;};
	bool GetGrepInfo(GrepInfo& gi) const {gi = m_gi;return true;};

// member valiables
private:
	static CCommandLine* _instance;
	LPCSTR		m_pszCmdLineSrc;	//! [in]�R�}���h���C��������
	bool		m_bGrepMode;		//! [out] TRUE: Grep Mode
	bool		m_bGrepDlg;			//  Grep�_�C�A���O
	bool		m_bDebugMode;		
	bool		m_bNoWindow;		//! [out] TRUE: �ҏWWindow���J���Ȃ�
	FileInfo	m_fi;				//!
	GrepInfo	m_gi;				//!
	bool		m_bReadOnly;		//! [out] TRUE: Read Only
};

///////////////////////////////////////////////////////////////////////
#endif /* _CCOMMANDLINE_H_ */


/*[EOF]*/
