//	$Id$
/************************************************************************

	CPropCommon.h
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/12/24  新規作成


************************************************************************/

class CPropCommon;

#ifndef _CPROP1_H_
#define _CPROP1_H_

#include <windows.h>
#include "CShareData.h"
#include "CMemory.h"
#include "CKeyBind.h"
#include "CKeyWordSetMgr.h"
#include "CImageListMgr.h"
#include "sakura_rc.h"

#define ID_PAGENUM_ZENPAN	0	//Oct. 25, 2000 JEPRO ZENPAN1→ZENPAN に変更
#define ID_PAGENUM_WIN		1	//Oct. 25, 2000 JEPRO  5→ 1 に変更
#define ID_PAGENUM_EDIT		2
#define ID_PAGENUM_FILE		3
#define ID_PAGENUM_BACKUP	4
#define ID_PAGENUM_FORMAT	5	//Oct. 25, 2000 JEPRO  1→ 5 に変更
#define ID_PAGENUM_URL		6
#define ID_PAGENUM_GREP		7
#define ID_PAGENUM_KEYBOARD	8
#define ID_PAGENUM_CUSTMENU	9	//Oct. 25, 2000 JEPRO 11→ 9 に変更
#define ID_PAGENUM_TOOLBAR	10	//Oct. 25, 2000 JEPRO  9→10 に変更
#define ID_PAGENUM_KEYWORD	11	//Oct. 25, 2000 JEPRO 10→11 に変更
#define ID_PAGENUM_HELPER	12


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CPropCommon
{
public:
	/*
	||  Constructors
	*/
	CPropCommon();
	~CPropCommon();
	void Create( HINSTANCE, HWND, CImageListMgr* );	/* 初期化 */

	/*
	||  Attributes & Operations
	*/
	int DoPropertySheet( int/*, int*/ );	/* プロパティシートの作成 */
	BOOL DispatchEvent_p1( HWND, UINT, WPARAM, LPARAM );	/* p1 メッセージ処理 */
	BOOL DispatchEvent_p2( HWND, UINT, WPARAM, LPARAM );	/* p2 メッセージ処理 */
	BOOL DispatchEvent_p3( HWND, UINT, WPARAM, LPARAM );	/* p3 メッセージ処理 */
	BOOL DispatchEvent_p4( HWND, UINT, WPARAM, LPARAM );	/* p4 メッセージ処理 */
	BOOL DispatchEvent_p5( HWND, UINT, WPARAM, LPARAM );	/* p5 メッセージ処理 */
	BOOL DispatchEvent_p6( HWND, UINT, WPARAM, LPARAM );	/* p6 メッセージ処理 */
	BOOL DispatchEvent_p7( HWND, UINT, WPARAM, LPARAM );	/* p7 メッセージ処理 */
	BOOL DispatchEvent_p8( HWND, UINT, WPARAM, LPARAM );	/* p8 メッセージ処理 */
	BOOL DispatchEvent_p9( HWND, UINT, WPARAM, LPARAM );	/* p9 メッセージ処理 */
	BOOL DispatchEvent_p10( HWND, UINT, WPARAM, LPARAM );	/* p10 メッセージ処理 */



	BOOL DispatchEvent_PROP_BACKUP( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_BACKUP( HWND );
	int GetData_PROP_BACKUP( HWND );

	BOOL DispatchEvent_PROP_WIN( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_WIN( HWND );
	int GetData_PROP_WIN( HWND );

	BOOL DispatchEvent_PROP_URL( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_URL( HWND );
	int GetData_PROP_URL( HWND );

	BOOL DispatchEvent_PROP_EDIT( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_EDIT( HWND );
	int GetData_PROP_EDIT( HWND );

	BOOL DispatchEvent_PROP_GREP( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_GREP( HWND );
	int GetData_PROP_GREP( HWND );



	HINSTANCE		m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND			m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND			m_hwndThis;		/* このダイアログのハンドル */
	int				m_nPageNum;
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
	int				m_nSettingType;
//	int				m_nActiveItem;

	//	Oct. 16, 2000 genta
	CImageListMgr*	m_pcIcons;	//	Image List

	/*
	|| ダイアログデータ
	*/
	char			m_szHelpFile[_MAX_PATH + 1];
	Common			m_Common;
	short			m_nKeyNameArrNum;				/* キー割り当て表の有効データ数 */
	KEYDATA			m_pKeyNameArr[100];				/* キー割り当て表 */

	CKeyWordSetMgr	m_CKeyWordSetMgr;
	Types			m_Types[MAX_TYPES];
protected:
	/*
	||  実装ヘルパ関数
	*/
	void OnHelp( HWND, int );	/* ヘルプ */
	int	SearchIntArr( int , int* , int );
	void DrawToolBarItemList( DRAWITEMSTRUCT* );	/* ツールバーボタンリストのアイテム描画 */
	void DrawColorButton( DRAWITEMSTRUCT* , COLORREF );	/* 色ボタンの描画 */
	BOOL SelectColor( HWND , COLORREF* );	/* 色選択ダイアログ */
	void SetData_p1( HWND );	/* ダイアログデータの設定 p1 */
	int  GetData_p1( HWND );	/* ダイアログデータの取得 p1 */
	void SetData_p2( HWND );	/* ダイアログデータの設定 p2 */
	int  GetData_p2( HWND );	/* ダイアログデータの取得 p2 */
	void SetData_p3( HWND );	/* ダイアログデータの設定 p3 */
	int  GetData_p3( HWND );	/* ダイアログデータの取得 p3 */
	void SetData_p4( HWND );	/* ダイアログデータの設定 p4 */
	int  GetData_p4( HWND );	/* ダイアログデータの取得 p4 */
	void SetData_p5( HWND );	/* ダイアログデータの設定 p5 */
	int  GetData_p5( HWND );	/* ダイアログデータの取得 p5 */
	void p5_Import_KeySetting( HWND );	/* p5:キー割り当て設定をインポートする */
	void p5_Export_KeySetting( HWND );	/* p5:キー割り当て設定をエクスポートする */

	void SetData_p6( HWND );	/* ダイアログデータの設定 p6 */
	int  GetData_p6( HWND );	/* ダイアログデータの取得 p6 */
	void SetData_p7( HWND );	/* ダイアログデータの設定 p7 */
	void SetData_p7_KeyWordSet( HWND , int );	/* ダイアログデータの設定 p7 指定キーワードセットの設定 */
	int  GetData_p7( HWND );	/* ダイアログデータの取得 p7 */
	void GetData_p7_KeyWordSet( HWND , int );	/* ダイアログデータの取得 p7 指定キーワードセットの取得 */
	void SetData_p8( HWND );	/* ダイアログデータの設定 p8 */
	int  GetData_p8( HWND );	/* ダイアログデータの取得 p8 */
	void SetData_p9( HWND );	/* ダイアログデータの設定 p9 */
	int  GetData_p9( HWND );	/* ダイアログデータの取得 p9 */
	void ChangeDateExample( HWND hwndDlg );
	void ChangeTimeExample( HWND hwndDlg );

	
	void SetData_p10( HWND );	/* ダイアログデータの設定 p9 */
	int  GetData_p10( HWND );	/* ダイアログデータの取得 p9 */

	void p7_Edit_List_KeyWord( HWND, HWND );	/* p7:リスト中で選択されているキーワードを編集する */
	void p7_Delete_List_KeyWord( HWND , HWND );	/* p7:リスト中で選択されているキーワードを削除する */
	void p7_Import_List_KeyWord( HWND , HWND );	/* p7:リスト中のキーワードをインポートする */
	void p7_Export_List_KeyWord( HWND , HWND );	/* p7:リスト中のキーワードをエクスポートする */

	void p8_Import_CustMenuSetting( HWND );	/* p8:カスタムメニュー設定をインポートする */
	void p8_Export_CustMenuSetting( HWND );	/* p8:カスタムメニュー設定をエクスポートする */

	//	Aug. 16, 2000 genta
	void EnableBackupInput(HWND hwndDlg);	//	バックアップ設定のON/OFF
	//	Aug. 21, 2000 genta
	void EnableFilePropInput(HWND hwndDlg);	//	ファイル設定のON/OFF
	//	Sept. 9, 2000 JEPRO		次行を追加
	void EnableWinPropInput( HWND hwndDlg) ;	//	ウィンドウ設定のON/OFF
	//	Sept. 10, 2000 JEPRO	次行を追加
	void CPropCommon::EnableFormatPropInput( HWND hwndDlg );	//	書式設定のON/OFF
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPROP1_H_ */


/*[EOF]*/
