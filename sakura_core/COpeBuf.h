//	$Id$
/************************************************************************

	COpeBuf.h

	アンドゥ・リドゥバッファ
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1998/6/9  新規作成

************************************************************************/

class COpeBuf;

#ifndef _COPEBUF_H_
#define _COPEBUF_H_

/* アンドゥバッファ用　操作コード */
enum enumOPECODE {
	OPE_INSERT		= 1,
	OPE_DELETE		= 2,
	OPE_MOVECARET	= 3,
};


#include "COpeBlk.h"




/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/* アンドゥ・リドゥバッファ */
class SAKURA_CORE_API COpeBuf {
	public:
		COpeBuf();	/* COpeBufクラス構築 */
		~COpeBuf();	/* COpeBufクラス消滅 */
		void ClearAll( void );	/* 全要素のクリア */
		int AppendOpeBlk( COpeBlk* );	/* 操作ブロックの追加 */
		int	IsEnableUndo( void );	/* Undo可能な状態か */
		int	IsEnableRedo( void );	/* Redo可能な状態か */
		COpeBlk* DoUndo( int* );	/* 現在のUndo対象の操作ブロックを返す */
		COpeBlk* DoRedo( int* );	/* 現在のRedo対象の操作ブロックを返す */
		void SetNoModified( void );	/* 現在位置で無変更な状態になったことを通知 */

		void DUMP( void );	/* 編集操作要素ブロックのダンプ */
	private:
		int			 m_nCOpeBlkArrNum;	/* 操作ブロックの数 */
		COpeBlk**	m_ppCOpeBlkArr;	/* 操作ブロックの配列 */
		int			m_nCurrentPointer;	/* 現在位置 */
		int			m_nNoModifiedIndex;	/* 無変更な状態になった位置 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _COPEBUF_H_ */

/*[EOF]*/
