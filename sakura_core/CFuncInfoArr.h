/*!	@file
	@brief アウトライン解析 データ配列

	@author Norio Nakatani
	@date	1998/06/23 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CFuncInfoArr;

#ifndef _CFUNCINFOARR_H_
#define _CFUNCINFOARR_H_

#include "CFuncInfo.h"

//! アウトライン解析 データ配列
class CFuncInfoArr {
public:
	CFuncInfoArr();	/* CFuncInfoArrクラス構築 */
	~CFuncInfoArr();	/* CFuncInfoArrクラス消滅 */
	CFuncInfo* GetAt( int );	/* 0<=の指定番号のデータを返す */
	void AppendData( CFuncInfo* );	/* 配列の最後にデータを追加する */
	void AppendData( int, int, const TCHAR*, int, int nDepth = 0 );	/* 配列の最後にデータを追加する 2002.04.01 YAZAKI 深さ導入*/
	int	GetNum( void ){	return m_nFuncInfoArrNum; }	/* 配列要素数を返す */
	void Empty( void );
	void DUMP( void );




public:
	char		m_szFilePath[_MAX_PATH + 1];	/*!< 解析対象ファイル名 */
private:
	int			m_nFuncInfoArrNum;	/*!< 配列要素数 */
	CFuncInfo**	m_ppcFuncInfoArr;	/*!< 配列 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFOARR_H_ */


/*[EOF]*/
