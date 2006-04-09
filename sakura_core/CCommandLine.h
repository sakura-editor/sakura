/*!	@file
	@brief コマンドラインパーサ ヘッダファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CEditAppより分離
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
クラスの宣言
-----------------------------------------------------------------------*/

/*!
	@brief コマンドラインパーサ クラス
*/
class SAKURA_CORE_API CCommandLine {
public:
	static CCommandLine* Instance(LPSTR cmd=NULL);

private:
	// 2005-08-24 D.S.Koba 引数削除
	void ParseCommandLine( void );
	
	static int CheckCommandLine(

		LPSTR  str, //!< [in] 検証する文字列（先頭の-は含まない）
		char** arg	//!< [out] 引数がある場合はその先頭へのポインタ
	);
	
	// 外から作らせない。
	CCommandLine();
	CCommandLine(LPSTR cmd);

	/*!
		引用符で囲まれている数値を認識するようにする
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
