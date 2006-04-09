/*!	@file
	@brief �R�}���h���C���p�[�T �w�b�_�t�@�C��

	@author aroka
	@date	2002/01/08 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CEditApp��蕪��
	Copyright (C) 2002, genta
	Copyright (C) 2005, D.S.Koba

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
	// 2005-08-24 D.S.Koba �����폜
	void ParseCommandLine( void );
	
	static int CheckCommandLine(

		LPSTR  str, //!< [in] ���؂��镶����i�擪��-�͊܂܂Ȃ��j
		char** arg	//!< [out] ����������ꍇ�͂��̐擪�ւ̃|�C���^
	);
	
	// �O�����点�Ȃ��B
	CCommandLine();
	CCommandLine(LPSTR cmd);

	/*!
		���p���ň͂܂�Ă��鐔�l��F������悤�ɂ���
		@date 2002.12.05 genta
	*/
	static int AtoiOptionInt(const char* arg){
		return ( arg[0] == '"' || arg[0] == '\'' ) ?
			atoi( arg + 1 ) : atoi( arg );
	}

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
