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

#ifndef SAKURA_TYPES_CPROPTYPES_H_
#define SAKURA_TYPES_CPROPTYPES_H_

class CPropTypes;


#include "CShareData.h"
//2007.11.29 kobake 変数の意味を明確にするため、nMethos を テンプレート化。
template <class TYPE>
struct TYPE_NAME {
	TYPE		nMethod;
	const TCHAR*	pszName;
};

//!< プロパティシート番号
enum PropTypeSheetOrder {
	ID_PROPTYPE_PAGENUM_SCREEN = 0,		//!< スクリーン
	ID_PROPTYPE_PAGENUM_COLOR,			//!< カラー
	ID_PROPTYPE_PAGENUM_SUPPORT,		//!< 支援
	ID_PROPTYPE_PAGENUM_REGEX,			//!< 正規表現キーワード
	ID_PROPTYPE_PAGENUM_KEYHELP,		//!< ステータスバー
	ID_PROPTYPE_PAGENUM_MAX,
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief タイプ別設定ダイアログボックス

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CPropTypes{

public:
	//生成と破棄
	CPropTypes();
	~CPropTypes();
	void Create( HINSTANCE, HWND );	//!< 初期化
	INT_PTR DoPropertySheet( int );		//!< プロパティシートの作成

	//インターフェース	
	void SetTypeData( const STypeConfig& t ){ m_Types = t; }	//!< タイプ別設定データの設定  Jan. 23, 2005 genta
	void GetTypeData( STypeConfig& t ) const { t = m_Types; }	//!< タイプ別設定データの取得  Jan. 23, 2005 genta
	int GetPageNum(){ return m_nPageNum; }

protected:
	//イベント
	void OnHelp( HWND , int );	//!< ヘルプ

protected:
	//各種参照
	HINSTANCE	m_hInstance;	//!< アプリケーションインスタンスのハンドル
	HWND		m_hwndParent;	//!< オーナーウィンドウのハンドル
	HWND		m_hwndThis;		//!< このダイアログのハンドル

	//ダイアログデータ
	PropTypeSheetOrder	m_nPageNum;
	DLLSHAREDATA*		m_pShareData;
	STypeConfig			m_Types;

	// カラー用データ
	DWORD			m_dwCustColors[16];						//!< フォントDialogカスタムパレット
	int				m_nSet[ MAX_KEYWORDSET_PER_TYPE ];		//!< keyword set index  2005.01.13 MIK
	int				m_nCurrentColorType;					//!< 現在選択されている色タイプ
	CKeyWordSetMgr*	m_pCKeyWordSetMgr;						//!< メモリ削減のためポインタに  Mar. 31, 2003 genta

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      各プロパティページ                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< メッセージ処理
protected:
	void SetData( HWND );											//!< ダイアログデータの設定
	int  GetData( HWND );											//!< ダイアログデータの取得
	bool Import( HWND );											//!< インポート
	bool Export( HWND );											//!< エクスポート
};


/*!
	@brief タイプ別設定プロパティページクラス

	プロパティページ毎に定義
	変数の定義はCPropTypesで行う
*/
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        スクリーン                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CPropTypesScreen : public CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< メッセージ処理
protected:
	void SetData( HWND );											//!< ダイアログデータの設定
	int  GetData( HWND );											//!< ダイアログデータの取得
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          カラー                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CPropTypesColor : public CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< メッセージ処理
protected:
	void SetData( HWND );											//!< ダイアログデータの設定
	int  GetData( HWND );											//!< ダイアログデータの取得
	bool Import( HWND );											//!< インポート
	bool Export( HWND );											//!< エクスポート

protected:
	void DrawColorListItem( DRAWITEMSTRUCT* );				//!< 色種別リスト オーナー描画
	void EnableTypesPropInput( HWND hwndDlg );				//!< タイプ別設定のカラー設定のON/OFF
	void RearrangeKeywordSet( HWND );						//!< キーワードセット再配置  Jan. 23, 2005 genta
	void DrawColorButton( DRAWITEMSTRUCT* , COLORREF );		//!< 色ボタンの描画
public:
	static BOOL SelectColor( HWND , COLORREF*, DWORD* );	//!< 色選択ダイアログ
private:
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           支援                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CPropTypesSupport : public CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< メッセージ処理
protected:
	void SetData( HWND );											//!< ダイアログデータの設定
	int  GetData( HWND );											//!< ダイアログデータの取得
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    正規表現キーワード                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CPropTypesRegex : public CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< メッセージ処理
protected:
	void SetData( HWND );											//!< ダイアログデータの設定
	void SetDataKeywordList( HWND );								//!< ダイアログデータの設定リスト部分
	int  GetData( HWND );											//!< ダイアログデータの取得
	bool Import( HWND );											//!< インポート
	bool Export( HWND );											//!< エクスポート
private:
	BOOL RegexKakomiCheck(const char *s);	//@@@ 2001.11.17 add MIK

};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キーワードヘルプ                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class CPropTypesKeyHelp : public CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< メッセージ処理
protected:
	void SetData( HWND );											//!< ダイアログデータの設定
	int  GetData( HWND );											//!< ダイアログデータの取得
	bool Import( HWND );											//!< インポート
	bool Export( HWND );											//!< エクスポート
};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_TYPES_CPROPTYPES_H_ */


/*[EOF]*/
