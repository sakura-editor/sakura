/*!	@file
	@brief アプリケーション定義のメッセージ
	
	主にエディタウィンドウとコントロールプロセスの通信に使われる。

	@author Norio Nakatani
	@date 1998/5/15 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka
	Copyright (C) 2003, MIK
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _MYMESSAGE_H_
#define _MYMESSAGE_H_

#include <windows.h>


#define _SHIFT	0x00000001
#define _CTRL	0x00000002
#define _ALT	0x00000004

//wParam: X
//lParam: Y
#define MYWM_DOSPLIT        (WM_APP+1500)

//wParam: なんかのインデックス
//lParam: boolっぽい何か
#define MYWM_SETACTIVEPANE  (WM_APP+1510)

//!設定が変更されたことの通知メッセージ
//wParam:未使用
//lParam:未使用
#define MYWM_CHANGESETTING  (WM_APP+1520)

//! タスクトレイからの通知メッセージ
#define MYWM_NOTIFYICON		(WM_APP+100)



/*! トレイからエディタへの終了要求 */
#define	MYWM_CLOSE			(WM_APP+200)

/*! トレイからエディタへの編集ファイル情報 要求通知 */
#define	MYWM_GETFILEINFO	(WM_APP+203)

/*! カーソル位置変更通知 */
#define	MYWM_SETCARETPOS	(WM_APP+204)
/// MYWM_SETCARETPOSメッセージのLPARAM
enum e_PM_SETCARETPOS_SELECTSTATE {
	PM_SETCARETPOS_NOSELECT		= 0, /*!<選択解除 */
	PM_SETCARETPOS_SELECT		= 1, /*!<選択開始・変更 */
	PM_SETCARETPOS_KEEPSELECT	= 2, /*!<現在の選択状態を保って移動 */
};

/*! カーソル位置取得要求 */
#define	MYWM_GETCARETPOS	(WM_APP+205)

//! テキスト追加通知(共有データ経由)
//wParam:未使用
//lParam:未使用
#define	MYWM_ADDSTRING		(WM_APP+206)

/*! タグジャンプ元通知 */
#define	MYWM_SETREFERER		(WM_APP+207)

/*! 行(改行単位)データの要求 */
#define	MYWM_GETLINEDATA	(WM_APP+208)


/*! 編集ウィンドウオブジェクトからのオブジェクト削除要求 */
#define	MYWM_DELETE_ME		(WM_APP+209)

/*! 新しい編集ウィンドウの作成依頼(コマンドラインを渡す) */
#define	MYWM_OPENNEWEDITOR	(WM_APP+210)

//ヘルプっぽい何か
#define	MYWM_HTMLHELP			(WM_APP+212)

/*! タブウインドウ用メッセージ */
#define	MYWM_TAB_WINDOW_NOTIFY	(WM_APP+213)	//@@@ 2003.05.31 MIK

/*! バーの表示・非表示変更メッセージ */
#define	MYWM_BAR_CHANGE_NOTIFY	(WM_APP+214)	//@@@ 2003.06.10 MIK

/*! エディタ－トレイ間でのUI特権分離の確認メッセージ */
#define	MYWM_UIPI_CHECK	(WM_APP+215)	//@@@ 2007.06.07 ryoji

/*! ポップアップウィンドウの表示切替指示 */
#define MYWM_SHOWOWNEDPOPUPS (WM_APP+216)	//@@@ 2007.10.22 ryoji

/*! プロセスの初回アイドリング通知 */
#define MYWM_FIRST_IDLE (WM_APP+217)	//@@@ 2008.04.19 ryoji

/*! 独自のドロップファイル通知 */
#define MYWM_DROPFILES (WM_APP+218)	//@@@ 2008.06.18 ryoji

/* 再変換対応 */ // 20020331 aroka
#ifndef WM_IME_REQUEST
#define MYWM_IME_REQUEST 0x288  // ==WM_IME_REQUEST
#else
#define MYWM_IME_REQUEST WM_IME_REQUEST
#endif
#define MSGNAME_ATOK_RECONVERT TEXT("Atok Message for ReconvertString")
#define RWM_RECONVERT TEXT("MSIMEReconvert")

#define	VK_XBUTTON1		0x05	// マウスサイドボタン1（Windows2000以降で使用可能）	// 2009.01.12 nasukoji
#define	VK_XBUTTON2		0x06    // マウスサイドボタン2（Windows2000以降で使用可能）	// 2009.01.12 nasukoji

#endif /* _MYMESSAGE_H_ */
/*[EOF]*/
