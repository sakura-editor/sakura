/*!	@file
	@brief 編集操作要素

	@author Norio Nakatani
	@date 1998/06/09 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class COpe;

#ifndef _COPE_H_
#define _COPE_H_

class CMemory;// 2002/2/10 aroka


// アンドゥバッファ用 操作コード
enum EOpeCode {
	OPE_UNKNOWN		= 0,
	OPE_INSERT		= 1,
	OPE_DELETE		= 2,
	OPE_MOVECARET	= 3,
};


/*!
	編集操作要素
	
	Undoのためにに操作手順を記録するために用いる。
	1オブジェクトが１つの操作を表す。
*/
class COpe {
public:
	COpe();		/* COpeクラス構築 */
	~COpe();	/* COpeクラス消滅 */

	void DUMP( void );	/* 編集操作要素のダンプ */

	EOpeCode	m_nOpe;						//!< 操作種別

	CLogicPoint	m_ptCaretPos_PHY_Before;	//!< カーソル位置
	CLogicPoint	m_ptCaretPos_PHY_To;		//!< 操作前のキャレット位置
	CLogicPoint	m_ptCaretPos_PHY_After;		//!< カーソル位置

	int			m_nDataLen;			//!< 操作に関連するデータのサイズ
	CMemory*	m_pcmemData;		//!< 操作に関連するデータ

};



///////////////////////////////////////////////////////////////////////
#endif /* _COPE_H_ */


/*[EOF]*/
