//	$Id$
/************************************************************************

	CFuncInfoArr.h

	アウトライン解析　データ配列
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1998/6/23  新規作成

************************************************************************/

class CFuncInfoArr;

#ifndef _CFUNCINFOARR_H_
#define _CFUNCINFOARR_H_

#include "CFuncInfo.h"




/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/* アンドゥ・リドゥバッファ */
class CFuncInfoArr {
	public:
		CFuncInfoArr();	/* CFuncInfoArrクラス構築 */
		~CFuncInfoArr();	/* CFuncInfoArrクラス消滅 */
		CFuncInfo* GetAt( int );	/* 0<=の指定番号のデータを返す */
		void AppendData( CFuncInfo* );	/* 配列の最後にデータを追加する */
		void AppendData( int, int, char*, int );	/* 配列の最後にデータを追加する */
		int	GetNum( void ){	return m_nFuncInfoArrNum; }	/* 配列要素数を返す */
		void Empty( void );
		void DUMP( void );



		char		m_szFilePath[_MAX_PATH + 1];	/* 解析対象ファイル名 */	
	private:
		int			m_nFuncInfoArrNum;	/* 配列要素数 */
		CFuncInfo**	m_ppcFuncInfoArr;	/* 配列 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFOARR_H_ */

/*[EOF]*/
