//	$Id$
/*!	@file
	アプリケーション定義のメッセージ

	@author Norio Nakatani
	@date 1998/5/15 新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <windows.h>

#define MYWM_DOSPLIT        (WM_APP+1500)
#define MYWM_SETACTIVEPANE  (WM_APP+1510)
#define MYWM_CHANGESETTING  (WM_APP+1520)	/* 設定が変更されたことの通知メッセージ */

/* タスクトレイからの通知メッセージ */
#define MYWM_NOTIFYICON		(WM_APP+100)




//	/* エディタからトレイへの自己登録要求 */
//	#define	MYWM_ADD_EDITOR	(WM_APP+101)

//	/* エディタからトレイへの自己削除要求 */
//	#define	MYWM_DEL_EDITOR	(WM_APP+102)

//	/* エディタからトレイへのアクセラレータ解読要求 */
//	#define	MYWM_GETACCELFUNC	(WM_APP+103)




/* トレイからエディタへの終了要求 */
#define	MYWM_CLOSE			(WM_APP+200)

//	/* トレイからエディタへのアクセラレータテーブル更新通知 */
//	#define	MYWM_SETACCEL		(WM_APP+201)

///* トレイからエディタへの設定変更通知 */
//#define	MYWM_SETOPTION		(WM_APP+202)

/* トレイからエディタへの編集ファイル情報 要求通知 */
#define	MYWM_GETFILEINFO	(WM_APP+203)

/* カーソル位置変更通知 */
#define	MYWM_SETCARETPOS	(WM_APP+204)

/* カーソル位置取得要求 */
#define	MYWM_GETCARETPOS	(WM_APP+205)

/* テキスト追加通知(共有データ経由) */
#define	MYWM_ADDSTRING		(WM_APP+206)

/* タグジャンプ元通知 */
#define	MYWM_SETREFERER		(WM_APP+207)

/* 行(改行単位)データの要求 */
#define	MYWM_GETLINEDATA	(WM_APP+208)


/* 編集ウィンドウオブジェクトからのオブジェクト削除要求 */
#define	MYWM_DELETE_ME		(WM_APP+209)

/* 新しい編集ウィンドウの作成依頼(コマンドラインを渡す) */
#define	MYWM_OPENNEWEDITOR	(WM_APP+210)

///* 編集ウィンドウオブジェクトからのアクティブ要求 */
//#define	MYWM_ACTIVATE_ME		(WM_APP+211)

#define	MYWM_HTMLHELP			(WM_APP+212)

///* あんたぐれっぷ? */
//#define	MYWM_AREYOUGREP			(WM_APP+212)


//	/* エディタからトレイへの編集ファイル情報 設定通知 */
//	#define	MYWM_SETFILEINFO	(WM_APP+204)


//	/* トレイ管理ウィンドウからの全終了メッセージ */
//	#define	MYWM_CLOSEALL		(WM_APP+900)


/*[EOF]*/
