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
//2007.10.17 kobake 解放漏れを防ぐため、データをポインタではなくインスタンス実体で持つように変更
class COpe {
public:
	COpe(EOpeCode eCode = OPE_UNKNOWN);		/* COpeクラス構築 */
	virtual ~COpe();	/* COpeクラス消滅 */

	void DUMP( void );	/* 編集操作要素のダンプ */

	EOpeCode	GetCode() const{ return m_nOpe; }

private:
	EOpeCode	m_nOpe;						//!< 操作種別

public:
	CLogicPoint	m_ptCaretPos_PHY_Before;	//!< キャレット位置。文字単位。			[共通]
	CLogicPoint	m_ptCaretPos_PHY_After;		//!< キャレット位置。文字単位。			[共通]
};

//!削除
class CDeleteOpe : public COpe{
public:
	CDeleteOpe() : COpe(OPE_DELETE)
	{
		m_ptCaretPos_PHY_To.Set(CLogicInt(0),CLogicInt(0));
		m_nDataLen = CLogicInt(0);
	}
public:
	CLogicPoint	m_ptCaretPos_PHY_To;		//!< 操作前のキャレット位置。文字単位。	[DELETE]
	CLogicInt	m_nDataLen;					//!< 操作に関連するデータのサイズ		[DELETE]
	CNativeW	m_pcmemData;				//!< 操作に関連するデータ				[DELETE/INSERT]
};

//!挿入
class CInsertOpe : public COpe{
public:
	CInsertOpe() : COpe(OPE_INSERT) { }
public:
	CNativeW	m_pcmemData;				//!< 操作に関連するデータ				[DELETE/INSERT]
};

//!キャレット移動
class CMoveCaretOpe : public COpe{
public:
	CMoveCaretOpe() : COpe(OPE_MOVECARET) { }
	CMoveCaretOpe(const CLogicPoint& ptBefore, const CLogicPoint& ptAfter)
	: COpe(OPE_MOVECARET)
	{
		m_ptCaretPos_PHY_Before = ptBefore;
		m_ptCaretPos_PHY_After = ptAfter;
	}
};









///////////////////////////////////////////////////////////////////////
#endif /* _COPE_H_ */



