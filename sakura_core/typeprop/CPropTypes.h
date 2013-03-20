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
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#ifndef SAKURA_TYPES_CPROPTYPES_H_
#define SAKURA_TYPES_CPROPTYPES_H_

#include "types/CType.h" // STypeConfig

class CPropTypes;
class CKeyWordSetMgr;

/*-----------------------------------------------------------------------
定数
-----------------------------------------------------------------------*/

//2007.11.29 kobake 変数の意味を明確にするため、nMethos を テンプレート化。
template <class TYPE>
struct TYPE_NAME {
	TYPE		nMethod;
	const TCHAR*	pszName;
};

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
	void SetTypeData( const STypeConfig& t ){ m_Types = t; }	//!< タイプ別設定データの設定  Jan. 23, 2005 genta
	void GetTypeData( STypeConfig& t ) const { t = m_Types; }	//!< タイプ別設定データの取得  Jan. 23, 2005 genta

protected:
	//イベント
	void OnHelp( HWND , int );	//!< ヘルプ

protected:
	//各種参照
	HINSTANCE	m_hInstance;	//!< アプリケーションインスタンスのハンドル
	HWND		m_hwndParent;	//!< オーナーウィンドウのハンドル
	HWND		m_hwndThis;		//!< このダイアログのハンドル
	LPCTSTR		m_pszHelpFile;

	//ダイアログデータ
	int			m_nPageNum;
	STypeConfig		m_Types;

	// スクリーン用データ	2010/5/10 CPropTypes_P1_Screen.cppから移動
	static std::vector<TYPE_NAME<EOutlineType> > m_OlmArr;			//!<アウトライン解析ルール配列
	static std::vector<TYPE_NAME<ESmartIndentType> > m_SIndentArr;	//!<スマートインデントルール配列

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
class SAKURA_CORE_API CPropTypesScreen : CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< メッセージ処理
protected:
	void SetData( HWND );											//!< ダイアログデータの設定
	int  GetData( HWND );											//!< ダイアログデータの取得

public:
	static void AddOutlineMethod(int nMethod, const WCHAR* szName);	//!<アウトライン解析ルールの追加
	static void AddSIndentMethod(int nMethod, const WCHAR* szName);	//!<スマートインデントルールの追加
	void CPropTypes_Screen();										//!<スクリーンタブのコンストラクタ
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ウィンドウ                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class SAKURA_CORE_API CPropTypesWindow : CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< メッセージ処理
protected:
	void SetData( HWND );											//!< ダイアログデータの設定
	int  GetData( HWND );											//!< ダイアログデータの取得

protected:
	void SetCombobox(HWND hwndWork, const TCHAR** pszLabels, int nCount, int select);
	void EnableTypesPropInput( HWND hwndDlg );						//!< タイプ別設定のON/OFF
public:
private:
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          カラー                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class SAKURA_CORE_API CPropTypesColor : CPropTypes
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
class SAKURA_CORE_API CPropTypesSupport : CPropTypes
{
public:
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );			//!< メッセージ処理
protected:
	void SetData( HWND );											//!< ダイアログデータの設定
	int  GetData( HWND );											//!< ダイアログデータの取得
public:
	static void AddHokanMethod(int nMethod, const WCHAR* szName);	//!<補完種別の追加
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    正規表現キーワード                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class SAKURA_CORE_API CPropTypesRegex : CPropTypes
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
	BOOL RegexKakomiCheck(const wchar_t *s);	//@@@ 2001.11.17 add MIK

	bool CheckKeywordList(HWND, const TCHAR*, int);

};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キーワードヘルプ                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class SAKURA_CORE_API CPropTypesKeyHelp : CPropTypes
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



