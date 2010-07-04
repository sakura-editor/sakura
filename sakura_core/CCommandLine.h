/*!	@file
	@brief �R�}���h���C���p�[�T �w�b�_�t�@�C��

	@author aroka
	@date	2002/01/08 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CControlTray��蕪��
	Copyright (C) 2002, genta
	Copyright (C) 2005, D.S.Koba
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CCOMMANDLINE_H_
#define _CCOMMANDLINE_H_

#include "global.h"
#include "EditInfo.h"
class CMemory;
/*!	�����I�v�V����
	20020118 aroka
*/
struct GrepInfo {
	CNativeW		cmGrepKey;			//!< �����L�[
	CNativeT		cmGrepFile;			//!< �����Ώۃt�@�C��
	CNativeT		cmGrepFolder;		//!< �����Ώۃt�H���_
	SSearchOption	sGrepSearchOption;	//!< �����I�v�V����
	bool			bGrepSubFolder;		//!< �T�u�t�H���_����������
	bool			bGrepOutputLine;	//!< ���ʏo�͂ŊY���s���o�͂���
	int				nGrepOutputStyle;	//!< ���ʏo�͌`��
	ECodeType		nGrepCharSet;		//!< �����R�[�h�Z�b�g
};


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/

/*!
	@brief �R�}���h���C���p�[�T �N���X
*/
class SAKURA_CORE_API CCommandLine {
public:
	static CCommandLine* Instance(LPTSTR cmd=NULL);

private:
	// 2005-08-24 D.S.Koba �����폜
	void ParseCommandLine( void );
	
	static int CheckCommandLine(
		LPTSTR	str,		//!< [in] ���؂��镶����i�擪��-�͊܂܂Ȃ��j
		TCHAR**	arg,		//!< [out] ����������ꍇ�͂��̐擪�ւ̃|�C���^
		int*	arglen		//!< [out] �����̒���
	);
	
	// �O�����点�Ȃ��B
	CCommandLine();
	CCommandLine(LPTSTR cmd);

	/*!
		���p���ň͂܂�Ă��鐔�l��F������悤�ɂ���
		@date 2002.12.05 genta
	*/
	static int AtoiOptionInt(const TCHAR* arg){
		return ( arg[0] == _T('"') || arg[0] == _T('\'') ) ?
			_ttoi( arg + 1 ) : _ttoi( arg );
	}

// member accessor method
public:
	bool IsNoWindow() const {return m_bNoWindow;};
	bool IsWriteQuit() const {return m_bWriteQuit;};	// 2007.05.19 ryoji sakuext�p�ɒǉ�
	bool IsGrepMode() const {return m_bGrepMode;};
	bool IsGrepDlg() const {return m_bGrepDlg;};
	bool IsDebugMode() const {return m_bDebugMode;};
	bool IsViewMode() const {return m_bViewMode;};
	bool GetEditInfo(EditInfo* fi) const { *fi = m_fi; return true; }
	bool GetGrepInfo(GrepInfo* gi) const { *gi = m_gi; return true; }
	int GetGroupId() const {return m_nGroup;};	// 2007.06.26 ryoji
	LPCWSTR GetMacro() const{ return m_cmMacro.GetStringPtr(); }
	LPCWSTR GetMacroType() const{ return m_cmMacroType.GetStringPtr(); }

// member valiables
private:
	static CCommandLine* _instance;
	LPCTSTR		m_pszCmdLineSrc;	//! [in]�R�}���h���C��������
	bool		m_bGrepMode;		//! [out] TRUE: Grep Mode
	bool		m_bGrepDlg;			//  Grep�_�C�A���O
	bool		m_bDebugMode;		
	bool		m_bNoWindow;		//! [out] TRUE: �ҏWWindow���J���Ȃ�
	bool		m_bWriteQuit;		//! [out] TRUE: �ݒ��ۑ����ďI��	// 2007.05.19 ryoji sakuext�p�ɒǉ�
	EditInfo	m_fi;				//!
	GrepInfo	m_gi;				//!
	bool		m_bViewMode;		//! [out] TRUE: Read Only
	int			m_nGroup;			//! �O���[�vID	// 2007.06.26 ryoji
	CNativeW	m_cmMacro;			//! [out] �}�N���t�@�C�����^�}�N����
	CNativeW	m_cmMacroType;		//! [out] �}�N�����
};

///////////////////////////////////////////////////////////////////////
#endif /* _CCOMMANDLINE_H_ */



