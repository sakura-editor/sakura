/*!	@file
	@brief タイプ別設定ダイアログボックス

	@author Norio Nakatani
	@date 1998/05/08  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, MIK, asa-o
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, genta
	Copyright (C) 2005, MIK, aroka, genta
	Copyright (C) 2006, fon

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

class CPropTypes;

#ifndef _CPROPTYPES_H_
#define _CPROPTYPES_H_

#include <windows.h>
#include "CShareData.h"

/*-----------------------------------------------------------------------
定数
-----------------------------------------------------------------------*/
//#define STR_COLORDATA_HEAD2	" テキストエディタ色設定 Ver2"
//#define STR_COLORDATA_HEAD21	" テキストエディタ色設定 Ver2.1"	//Nov. 2, 2000 JEPRO 変更 [注]. 0.3.9.0:ur3β10以降、設定項目の番号を入れ替えたため
#define STR_COLORDATA_HEAD3	" テキストエディタ色設定 Ver3"		//Jan. 15, 2001 Stonee  色設定Ver3ドラフト(設定ファイルのキーを連番→文字列に)	//Feb. 11, 2001 JEPRO 有効にした
//#define STR_COLORDATA_HEAD4		" テキストエディタ色設定 Ver4"		//2007.10.02 kobake UNICODE化に際してカラーファイル仕様も変更
#define STR_COLORDATA_SECTION	"SakuraColor"


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief タイプ別設定ダイアログボックス

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class SAKURA_CORE_API CPropTypes{
public:
	//生成と破棄
	CPropTypes();
	~CPropTypes();
	void Create( HINSTANCE, HWND );	//!< 初期化
	int DoPropertySheet( int );		//!< プロパティシートの作成

	//インターフェース	
	void SetTypeData( const Types& t ){ m_Types = t; }	//!< タイプ別設定データの設定  Jan. 23, 2005 genta
	void GetTypeData( Types& t ) const { t = m_Types; }	//!< タイプ別設定データの取得  Jan. 23, 2005 genta

protected:
	//イベント
	void OnHelp( HWND , int );	//!< ヘルプ

private:
	//各種参照
	HINSTANCE	m_hInstance;	//!< アプリケーションインスタンスのハンドル
	HWND		m_hwndParent;	//!< オーナーウィンドウのハンドル
	HWND		m_hwndThis;		//!< このダイアログのハンドル
	SFilePath	m_szHelpFile;

	//ダイアログデータ
	int			m_nPageNum;
	Types		m_Types;


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        スクリーン                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent_Screen( HWND, UINT, WPARAM, LPARAM );		//!< p1 メッセージ処理
protected:
	void SetData_p1( HWND );	//!< ダイアログデータの設定 p1
	int  GetData_p1( HWND );	//!< ダイアログデータの取得 p1


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          カラー                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent_Color( HWND, UINT, WPARAM, LPARAM );		//!< p3 メッセージ処理
protected:
	void SetData_Color( HWND );								//!< ダイアログデータの設定 p3
	int  GetData_Color( HWND );								//!< ダイアログデータの取得 p3
	void _Import_Colors( HWND );							//!< 色の設定をインポート
	void _Export_Colors( HWND );							//!< 色の設定をエクスポート
	void DrawColorListItem( DRAWITEMSTRUCT*);				//!< 色種別リスト オーナー描画
	void EnableTypesPropInput( HWND hwndDlg );				//!< タイプ別設定のカラー設定のON/OFF
	void RearrangeKeywordSet( HWND );						//!< キーワードセット再配置  Jan. 23, 2005 genta
	void _DrawColorButton( DRAWITEMSTRUCT* , COLORREF );	//!< 色ボタンの描画
public:
	static BOOL SelectColor( HWND , COLORREF*, DWORD* );	//!< 色選択ダイアログ
private:
	DWORD			m_dwCustColors[16];						//!< フォントDialogカスタムパレット
	int				m_nSet[ MAX_KEYWORDSET_PER_TYPE ];		//!< keyword set index  2005.01.13 MIK
	int				m_nCurrentColorType;					//!< 現在選択されている色タイプ
	CKeyWordSetMgr*	m_pCKeyWordSetMgr;						//!< メモリ削減のためポインタに  Mar. 31, 2003 genta


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           支援                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent_Support( HWND, UINT, WPARAM, LPARAM );	//!< p2 メッセージ処理 支援タブ // 2001/06/14 asa-o
protected:
	void SetData_Support( HWND );	//!< ダイアログデータの設定 p2 支援タブ
	int  GetData_Support( HWND );	//!< ダイアログデータの取得 p2 支援タブ


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                    正規表現キーワード                       //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent_Regex( HWND, UINT, WPARAM, LPARAM );		//!< メッセージ処理 正規表現キーワード	//@@@ 2001.11.17 add MIK
protected:
	void SetData_Regex( HWND );	//!< ダイアログデータの設定 正規表現キーワード	//@@@ 2001.11.17 add MIK
	int  GetData_Regex( HWND );	//!< ダイアログデータの取得 正規表現キーワード	//@@@ 2001.11.17 add MIK
	BOOL Import_Regex( HWND );	//@@@ 2001.11.17 add MIK
	BOOL Export_Regex( HWND );	//@@@ 2001.11.17 add MIK
	BOOL RegexKakomiCheck(const wchar_t *s);	//@@@ 2001.11.17 add MIK


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     キーワードヘルプ                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent_KeyHelp( HWND, UINT, WPARAM, LPARAM );	//!< メッセージ処理 キーワード辞書選択	//@@@ 2006.04.10 fon
protected:
	void SetData_KeyHelp( HWND );	//!< ダイアログデータの設定 キーワード辞書選択
	int  GetData_KeyHelp( HWND );	//!< ダイアログデータの取得 キーワード辞書選択
	BOOL Import_KeyHelp( HWND );	//@@@ 2006.04.10 fon
	BOOL Export_KeyHelp( HWND );	//@@@ 2006.04.10 fon
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPROPTYPES_H_ */



