//	$Id$
/*!	@file
	@brief コマンドラインパーサ ヘッダファイル

	@author aroka
	@date	2002/01/08 作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CEditAppより分離

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CCOMMANDLINE_H_
#define _CCOMMANDLINE_H_

#include "global.h"
#include "CShareData.h"	// FileInfo, GrepInfo
class CMemory;
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

/*!
	@brief コマンドラインパーサ クラス
*/
class SAKURA_CORE_API CCommandLine {
public:
	static CCommandLine* Instance(LPSTR cmd=NULL);

private:
	static void ParseCommandLine(
		LPCSTR		pszCmdLineSrc,	//!< [in]コマンドライン文字列
		bool*		pbGrepMode,	//!< [out] true: Grep Mode
		bool*		pbGrepDlg,	//!< [out] true: Grep Dialog表示
		CMemory*	pcmGrepKey,	//!< [out] GrepのKey
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
		bool*		pbNoWindow,	//!< [out] TRUE: 編集Windowを開かない
		FileInfo*	pfi,
		bool*		pbReadOnly	//!< [out] TRUE: Read Only
	);
	static int CCommandLine::CheckCommandLine(
		LPSTR  str, //!< [in] 検証する文字列（先頭の-は含まない）
		char** arg	//!< [out] 引数がある場合はその先頭へのポインタ
	);
	
	// 外から作らせない。
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
	LPCSTR		m_pszCmdLineSrc;	//! [in]コマンドライン文字列
	bool		m_bGrepMode;		//! [out] TRUE: Grep Mode
	bool		m_bGrepDlg;			//  Grepダイアログ
	bool		m_bDebugMode;		
	bool		m_bNoWindow;		//! [out] TRUE: 編集Windowを開かない
	FileInfo	m_fi;				//!
	GrepInfo	m_gi;				//!
	bool		m_bReadOnly;		//! [out] TRUE: Read Only
};

///////////////////////////////////////////////////////////////////////
#endif /* _CCOMMANDLINE_H_ */


/*[EOF]*/
