/*!	@file
	@brief マクロ

	@author Norio Nakatani
	@author genta
	@date Sep. 29, 2001 作成
	@date 20011229 aroka バグ修正、コメント追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, genta, aroka, MIK, asa-o, hor, Misaka, jepro, Stonee
	Copyright (C) 2002, YAZAKI, MIK, aroka, hor, genta, ai
	Copyright (C) 2003, MIK, genta, Moca
	Copyright (C) 2004, genta, zenryaku
	Copyright (C) 2005, MIK, genta, maru, FILE
	Copyright (C) 2006, かろと, fon, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, nasukoji, ryoji
	Copyright (C) 2011, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "macro/CSMacroMgr.h"
#include "macro/CPPAMacroMgr.h"
#include "macro/CWSHManager.h"
#include "macro/CMacroFactory.h"
#include "env/CShareData.h"
#include "view/CEditView.h"
#include "debug/CRunningTimer.h"

VARTYPE s_MacroArgEx_i[] = {VT_I4};
MacroFuncInfoEx s_MacroInfoEx_i = {5, 5, s_MacroArgEx_i};
VARTYPE s_MacroArgEx_ii[] = {VT_I4, VT_I4};
MacroFuncInfoEx s_MacroInfoEx_ii = {6, 6, s_MacroArgEx_ii};
#if 0
VARTYPE s_MacroArgEx_s[] = {VT_BSTR};
MacroFuncInfoEx s_MacroInfoEx_s = {5, 5, s_MacroArgEx_s};
#endif

MacroFuncInfo CSMacroMgr::m_MacroFuncInfoCommandArr[] = 
{
	// 何もしない
	#define DEF_COMMAND(   id, val )
	#define DEF_MACROFUNC( id, name, param0, param1, param2, param3, ret, ext )
	#define DEF_MACROFUNC2(id, name, param0, param1, param2, param3, ret, ext )
	
	// マクロコマンド定義
	#define DEF_MACROCMD(  id, val, name, param0, param1, param2, param3, ret, ext ) \
		{ id, name, { param0, param1, param2, param3 }, ret, ext },
	#define DEF_MACROCMD2( id,      name, param0, param1, param2, param3, ret, ext ) \
		{ id, name, { param0, param1, param2, param3 }, ret, ext },
	
	#include "Funccode_x.h"
	
	//	終端
	//	Jun. 27, 2002 genta
	//	終端としては決して現れないものを使うべきなので，
	//	FuncIDを-1に変更．(0は使われる)
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

MacroFuncInfo CSMacroMgr::m_MacroFuncInfoArr[] = 
{
	// 何もしない
	#define DEF_COMMAND(   id, val )
	#define DEF_MACROCMD(  id, val, name, param0, param1, param2, param3, ret, ext )
	#define DEF_MACROCMD2( id,      name, param0, param1, param2, param3, ret, ext )
	
	// マクロ専用関数定義
	#define DEF_MACROFUNC( id, name, param0, param1, param2, param3, ret, ext ) \
		{ id, name, { param0, param1, param2, param3 }, ret, ext },
	#define DEF_MACROFUNC2(id, name, param0, param1, param2, param3, ret, ext ) \
		{ id, name, { param0, param1, param2, param3 }, ret, ext },
	
	#include "Funccode_x.h"
	
	//	終端
	//	Jun. 27, 2002 genta
	//	終端としては決して現れないものを使うべきなので，
	//	FuncIDを-1に変更．(0は使われる)
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

/*!
	@date 2002.02.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2002.04.29 genta オブジェクトの実体は実行時まで生成しない。
*/
CSMacroMgr::CSMacroMgr()
{
	MY_RUNNINGTIMER( cRunningTimer, "CSMacroMgr::CSMacroMgr" );
	
	m_pShareData = &GetDllShareData();
	
	CPPAMacroMgr::declare();
	CKeyMacroMgr::declare();
	CWSHMacroManager::declare();
	
	int i;
	for ( i = 0 ; i < MAX_CUSTMACRO ; i++ ){
		m_cSavedKeyMacro[i] = NULL;
	}
	//	Jun. 16, 2002 genta
	m_pKeyMacro = NULL;
	m_pTempMacro = NULL;

	//	Sep. 15, 2005 FILE
	SetCurrentIdx( INVALID_MACRO_IDX );
}

CSMacroMgr::~CSMacroMgr()
{
	//- 20011229 add by aroka
	ClearAll();
	
	//	Jun. 16, 2002 genta
	//	ClearAllと同じ処理だったので削除
}

/*! キーマクロのバッファをクリアする */
void CSMacroMgr::ClearAll( void )
{
	int i;
	for (i = 0; i < MAX_CUSTMACRO; i++){
		//	Apr. 29, 2002 genta
		delete m_cSavedKeyMacro[i];
		m_cSavedKeyMacro[i] = NULL;
	}
	//	Jun. 16, 2002 genta
	delete m_pKeyMacro;
	m_pKeyMacro = NULL;
	delete m_pTempMacro;
	m_pTempMacro = NULL;
}

/*! @briefキーマクロのバッファにデータ追加

	@param mbuf [in] 読み込み先マクロバッファ
	
	@date 2002.06.16 genta キーマクロの多種対応のため変更
*/
int CSMacroMgr::Append(
	int				idx,		//!<
	EFunctionCode	nFuncID,	//!< [in] 機能番号
	const LPARAM*	lParams,	//!< [in] パラメータ。
	CEditView*		pcEditView	//!< 
)
{
	assert( idx == STAND_KEYMACRO );
	if (idx == STAND_KEYMACRO){
		CKeyMacroMgr* pKeyMacro = dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro );
		if( pKeyMacro == NULL ){
			//	1. 実体がまだ無い場合
			//	2. CKeyMacroMgr以外の物が入っていた場合
			//	いずれにしても再生成する．
			delete m_pKeyMacro;
			m_pKeyMacro = new CKeyMacroMgr;
			pKeyMacro = dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro );
		}
		pKeyMacro->Append( nFuncID, lParams, pcEditView );
	}
	return TRUE;
}


/*!	@brief キーボードマクロの実行

	CShareDataからファイル名を取得し、実行する。

	@param hInstance [in] インスタンス
	@param hwndParent [in] 親ウィンドウの
	@param pViewClass [in] macro実行対象のView
	@param idx [in] マクロ番号。
	@param flags [in] マクロ実行フラグ．HandleCommandに渡すオプション．

	@date 2007.07.16 genta flags追加
*/
BOOL CSMacroMgr::Exec( int idx , HINSTANCE hInstance, CEditView* pcEditView, int flags )
{
	if( idx == STAND_KEYMACRO ){
		//	Jun. 16, 2002 genta
		//	キーマクロ以外のサポートによりNULLの可能性が出てきたので判定追加
		if( m_pKeyMacro != NULL ){
			//	Sep. 15, 2005 FILE
			//	Jul. 01, 2007 マクロの多重実行時に備えて直前のマクロ番号を退避
			int prevmacro = SetCurrentIdx( idx );
			m_pKeyMacro->ExecKeyMacro2( pcEditView, flags );
			SetCurrentIdx( prevmacro );
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	if( idx == TEMP_KEYMACRO ){		// 一時マクロ
		if( m_pTempMacro != NULL ){
			int prevmacro = SetCurrentIdx( idx );
			m_pTempMacro->ExecKeyMacro2( pcEditView, flags );
			SetCurrentIdx( prevmacro );
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	if( idx < 0 || MAX_CUSTMACRO <= idx )	//	範囲チェック
		return FALSE;

	/* 読み込み前か、毎回読み込む設定の場合は、ファイルを読み込みなおす */
	//	Apr. 29, 2002 genta
	if( m_cSavedKeyMacro[idx] == NULL || CShareData::getInstance()->BeReloadWhenExecuteMacro( idx )){
		//	CShareDataから、マクロファイル名を取得
		//	Jun. 08, 2003 Moca 呼び出し側でパス名を用意
		//	Jun. 16, 2003 genta 書式をちょっと変更
		TCHAR ptr[_MAX_PATH * 2];
		int n = CShareData::getInstance()->GetMacroFilename( idx, ptr, _countof(ptr) );
		if ( n <= 0 ){
			return FALSE;
		}

		if( !Load( idx, hInstance, ptr, NULL ) )
			return FALSE;
	}

	//	Sep. 15, 2005 FILE
	//	Jul. 01, 2007 マクロの多重実行時に備えて直前のマクロ番号を退避
	int prevmacro = SetCurrentIdx( idx );
	SetCurrentIdx( idx );
	m_cSavedKeyMacro[idx]->ExecKeyMacro2(pcEditView, flags);
	SetCurrentIdx( prevmacro );

	return TRUE;
}

/*! キーボードマクロの読み込み

	@param idx [in] 読み込み先マクロバッファ番号
	@param pszPath [in] マクロファイル名、またはコード文字列
	@param pszType [in] 種別。NULLの場合ファイルから読み込む。NULL以外の場合は言語の拡張子

	読み込みに失敗したときはマクロバッファのオブジェクトは解放され，
	NULLが設定される．

	@author Norio Nakatani, YAZAKI, genta
*/
BOOL CSMacroMgr::Load( int idx, HINSTANCE hInstance, const TCHAR* pszPath, const TCHAR* pszType )
{
	CMacroManagerBase** ppMacro = Idx2Ptr( idx );

	if( ppMacro == NULL ){
		DEBUG_TRACE( _T("CSMacroMgr::Load() Out of range: idx=%d Path=%ts\n"), idx, pszPath);
	}

	//	バッファクリア
	delete *ppMacro;
	*ppMacro = NULL;
	
	const TCHAR *ext;
	if( pszType == NULL ){				//ファイル指定
		//ファイルの拡張子を取得する
		ext = _tcsrchr( pszPath, _T('.'));
		//	Feb. 02, 2004 genta .が無い場合にext==NULLとなるのでNULLチェック追加
		if( ext != NULL ){
			const TCHAR *chk = _tcsrchr( ext, _T('\\') );
			if( chk != NULL ){	//	.のあとに\があったらそれは拡張子の区切りではない
								//	\が漢字の2バイト目の場合も拡張子ではない。
				ext = NULL;
			}
		}
		if(ext != NULL){
			++ext;
		}
	}else{								//コード指定
		ext = pszType;
	}

	m_sMacroPath = _T("");
	*ppMacro = CMacroFactory::getInstance()->Create(ext);
	if( *ppMacro == NULL )
		return FALSE;
	BOOL bRet;
	if( pszType == NULL ){
		bRet = (*ppMacro)->LoadKeyMacro(hInstance, pszPath);
		if (idx == STAND_KEYMACRO || idx == TEMP_KEYMACRO) {
			m_sMacroPath = pszPath;
		}
	}else{
		bRet = (*ppMacro)->LoadKeyMacroStr(hInstance, pszPath);
	}

	//	From Here Jun. 16, 2002 genta
	//	読み込みエラー時はインスタンス削除
	if( bRet ){
		return TRUE;
	}
	else {
		delete *ppMacro;
		*ppMacro = NULL;
	}
	//	To Here Jun. 16, 2002 genta
	return FALSE;
}

/** マクロオブジェクトをすべて破棄する(キーボードマクロ以外)

	マクロの登録を変更した場合に，変更前のマクロが
	引き続き実行されてしまうのを防ぐ．

	@date 2007.10.19 genta 新規作成
*/
void CSMacroMgr::UnloadAll(void)
{
	for ( int idx = 0; idx < MAX_CUSTMACRO; idx++ ){
		delete m_cSavedKeyMacro[idx];
		m_cSavedKeyMacro[idx] = NULL;
	}

}

/*! キーボードマクロの保存

	@param idx [in] 読み込み先マクロバッファ番号
	@param pszPath [in] マクロファイル名
	@param hInstance [in] インスタンスハンドル

	@author YAZAKI
*/
BOOL CSMacroMgr::Save( int idx, HINSTANCE hInstance, const TCHAR* pszPath )
{
	assert( idx == STAND_KEYMACRO );
	if ( idx == STAND_KEYMACRO ){
		CKeyMacroMgr* pKeyMacro = dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro );
		if( pKeyMacro != NULL ){
			return pKeyMacro->SaveKeyMacro(hInstance, pszPath );
		}
		//	Jun. 27, 2002 genta
		//	空マクロの場合は正常終了と見なす．
		if( m_pKeyMacro == NULL ){
			return TRUE;
		}

	}
//	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
//		return m_cSavedKeyMacro[idx]->SaveKeyMacro(hInstance, pszPath );
//	}
	return FALSE;
}

/*
	指定されたマクロをクリアする
	
	@param idx [in] マクロ番号(0-), STAND_KEYMACROは標準キーマクロバッファを表す．
*/
void CSMacroMgr::Clear( int idx )
{
	CMacroManagerBase **ppMacro = Idx2Ptr( idx );
	if( ppMacro != NULL ){
		delete *ppMacro;
		*ppMacro = NULL;
	}
}

/*
||  Attributes & Operations
*/
/*
	指定されたIDに対応するMacroInfo構造体へのポインタを返す．
	該当するIDに対応する構造体がなければNULLを返す．

	@param nFuncID [in] 機能番号
	@return 構造体へのポインタ．見つからなければNULL
	
	@date 2002.06.16 genta
	@date 2003.02.24 m_MacroFuncInfoArrも検索対象にする
*/
const MacroFuncInfo* CSMacroMgr::GetFuncInfoByID( int nFuncID )
{
	int i;
	//	Jun. 27, 2002 genta
	//	番人をコード0として拾ってしまうので，配列サイズによる判定をやめた．
	for( i = 0; m_MacroFuncInfoCommandArr[i].m_pszFuncName != NULL; ++i ){
		if( m_MacroFuncInfoCommandArr[i].m_nFuncID == nFuncID ){
			return &m_MacroFuncInfoCommandArr[i];
		}
	}
	for( i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( m_MacroFuncInfoArr[i].m_nFuncID == nFuncID ){
			return &m_MacroFuncInfoArr[i];
		}
	}
	return NULL;
}

/*!
	機能番号から関数名と機能名日本語を取得
	
	@return 成功したときはpszFuncName．見つからなかったときはNULL．
	
	@note
	それぞれ，文字列格納領域の指す先がNULLの時は文字列を格納しない．
	ただし，pszFuncNameをNULLにしてしまうと戻り値が常にNULLになって
	成功判定が行えなくなる．
	各国語メッセージリソース対応により機能名が日本語でない場合がある	

	@date 2002.06.16 genta 新設のGetFuncInfoById(int)を内部で使うように．
	@date 2011.04.10 nasukoji 各国語メッセージリソース対応
*/
WCHAR* CSMacroMgr::GetFuncInfoByID(
	HINSTANCE	hInstance,			//!< [in] リソース取得のためのInstance Handle
	int			nFuncID,			//!< [in] 機能番号
	WCHAR*		pszFuncName,		//!< [out] 関数名．この先には最長関数名＋1バイトのメモリが必要．
	WCHAR*		pszFuncNameJapanese	//!< [out] 機能名日本語．NULL許容. この先には256バイトのメモリが必要．
)
{
	const MacroFuncInfo* MacroInfo = GetFuncInfoByID( nFuncID );
	if( MacroInfo != NULL ){
		if( pszFuncName != NULL ){
			auto_strcpy( pszFuncName, MacroInfo->m_pszFuncName );
			WCHAR *p = pszFuncName;
			while (*p){
				if (*p == LTEXT('(')){
					*p = LTEXT('\0');
					break;
				}
				p++;
			}
		}
		//	Jun. 16, 2002 genta NULLのときは何もしない．
		if( pszFuncNameJapanese != NULL ){
			wcsncpy( pszFuncNameJapanese, LSW( nFuncID ), 255 );
		}
		return pszFuncName;
	}
	return NULL;
}

/*!
	関数名（S_xxxx）から機能番号と機能名日本語を取得．
	関数名はS_で始まる場合と始まらない場合の両方に対応．

	@return 成功したときは機能番号．見つからなかったときは-1．
	
	@note
	pszFuncNameJapanese の指す先がNULLの時は日本語名を格納しない．
	
	@date 2002.06.16 genta ループ内の文字列コピーを排除
*/
EFunctionCode CSMacroMgr::GetFuncInfoByName(
	HINSTANCE		hInstance,				//!< [in]  リソース取得のためのInstance Handle
	const WCHAR*	pszFuncName,			//!< [in]  関数名
	WCHAR*			pszFuncNameJapanese		//!< [out] 機能名日本語．この先には256バイトのメモリが必要．
)
{
	//	Jun. 16, 2002 genta
	const WCHAR* normalizedFuncName;
	
	//	S_で始まっているか
	if( pszFuncName == NULL ){
		return F_INVALID;
	}
	if( pszFuncName[0] == LTEXT('S') && pszFuncName[1] == LTEXT('_') ){
		normalizedFuncName = pszFuncName + 2;
	}
	else {
		normalizedFuncName = pszFuncName;
	}

	// コマンド関数を検索
	for( int i = 0; m_MacroFuncInfoCommandArr[i].m_pszFuncName != NULL; ++i ){
		if( 0 == auto_strcmp( normalizedFuncName, m_MacroFuncInfoCommandArr[i].m_pszFuncName )){
			EFunctionCode nFuncID = EFunctionCode(m_MacroFuncInfoCommandArr[i].m_nFuncID);
			if( pszFuncNameJapanese != NULL ){
				wcsncpy( pszFuncNameJapanese, LSW( nFuncID ), 255 );
				pszFuncNameJapanese[255] = L'\0';
			}
			return nFuncID;
		}
	}
	// 非コマンド関数を検索
	for( int i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( 0 == auto_strcmp( normalizedFuncName, m_MacroFuncInfoArr[i].m_pszFuncName )){
			EFunctionCode nFuncID = EFunctionCode(m_MacroFuncInfoArr[i].m_nFuncID);
			if( pszFuncNameJapanese != NULL ){
				wcsncpy( pszFuncNameJapanese, LSW( nFuncID ), 255 );
				pszFuncNameJapanese[255] = L'\0';
			}
			return nFuncID;
		}
	}
	return F_INVALID;
}

/* キーマクロに記録可能な機能かどうかを調べる */
BOOL CSMacroMgr::CanFuncIsKeyMacro( int nFuncID )
{
	switch( nFuncID ){
	/* ファイル操作系 */
//	case F_FILENEW					://新規作成
//	case F_FILEOPEN					://開く
//	case F_FILESAVE					://上書き保存
//	case F_FILESAVEAS_DIALOG		://名前を付けて保存
//	case F_FILECLOSE				://閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
//	case F_FILECLOSE_OPEN			://閉じて開く
	case F_FILE_REOPEN				://開き直す	//Dec. 4, 2002 genta
	case F_FILE_REOPEN_SJIS			://SJISで開き直す
	case F_FILE_REOPEN_JIS			://JISで開き直す
	case F_FILE_REOPEN_EUC			://EUCで開き直す
	case F_FILE_REOPEN_LATIN1		://Latin1で開き直す	// 2010/3/20 Uchi
	case F_FILE_REOPEN_UNICODE		://Unicodeで開き直す
	case F_FILE_REOPEN_UNICODEBE	://UnicodeBEで開き直す
	case F_FILE_REOPEN_UTF8			://UTF-8で開き直す
	case F_FILE_REOPEN_CESU8		://CESU-8で開き直す	// 2010/3/20 Uchi
	case F_FILE_REOPEN_UTF7			://UTF-7で開き直す
//	case F_PRINT					://印刷
//	case F_PRINT_DIALOG				://印刷ダイアログ
//	case F_PRINT_PREVIEW			://印刷プレビュー
//	case F_PRINT_PAGESETUP			://印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
//	case F_OPEN_HfromtoC:			://同名のC/C++ヘッダ(ソース)を開く	//Feb. 9, 2001 JEPRO 追加
//	case F_OPEN_HHPP				://同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
//	case F_OPEN_CCPP				://同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
//	case F_ACTIVATE_SQLPLUS			:/* Oracle SQL*Plusをアクティブ表示 */
//	case F_PLSQL_COMPILE_ON_SQLPLUS	:/* Oracle SQL*Plusで実行 */	//Sept. 17, 2000 jepro 説明の「コンパイル」を「実行」に統一
///	case F_BROWSE					://ブラウズ
//	case F_PROPERTY_FILE			://ファイルのプロパティ
//	case F_EXITALLEDITORS			://編集の全終了	// 2007.02.13 ryoji 追加
//	case F_EXITALL					://サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加
//	case F_PUTFILE					://作業中ファイルの一時出力	2006.12.10 maru
//	case F_INSFILE					://キャレット位置にファイル挿入	2006.12.10 maru


	/* 編集系 */
	case F_WCHAR					://文字入力
	case F_IME_CHAR					://全角文字入力
	case F_UNDO						://元に戻す(Undo)
	case F_REDO						://やり直し(Redo)
	case F_DELETE					://削除
	case F_DELETE_BACK				://カーソル前を削除
	case F_WordDeleteToStart		://単語の左端まで削除
	case F_WordDeleteToEnd			://単語の右端まで削除
	case F_WordCut					://単語切り取り
	case F_WordDelete				://単語削除
	case F_LineCutToStart			://行頭まで切り取り(改行単位)
	case F_LineCutToEnd				://行末まで切り取り(改行単位)
	case F_LineDeleteToStart		://行頭まで削除(改行単位)
	case F_LineDeleteToEnd			://行末まで削除(改行単位)
	case F_CUT_LINE					://行切り取り(折り返し単位)
	case F_DELETE_LINE				://行削除(折り返し単位)
	case F_DUPLICATELINE			://行の二重化(折り返し単位)
	case F_INDENT_TAB				://TABインデント
	case F_UNINDENT_TAB				://逆TABインデント
	case F_INDENT_SPACE				://SPACEインデント
	case F_UNINDENT_SPACE			://逆SPACEインデント
	case F_LTRIM					:// 2001.12.03 hor
	case F_RTRIM					:// 2001.12.03 hor
	case F_SORT_ASC					:// 2001.12.06 hor
	case F_SORT_DESC				:// 2001.12.06 hor
	case F_MERGE					:// 2001.12.06 hor

	/* カーソル移動系 */
	case F_UP						://カーソル上移動
	case F_DOWN						://カーソル下移動
	case F_LEFT						://カーソル左移動
	case F_RIGHT					://カーソル右移動
//	case F_ROLLDOWN					://スクロールダウン
//	case F_ROLLUP					://スクロールアップ
	// 2014.01.15 (Half)Page[Up/down] を有効化
	case F_HalfPageUp				://半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_HalfPageDown				://半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_1PageUp					://１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	case F_1PageDown				://１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	case F_UP2						://カーソル上移動(２行ごと)
	case F_DOWN2					://カーソル下移動(２行ごと)
	case F_GOLINETOP				://行頭に移動(折り返し単位)
	case F_GOLINEEND				://行末に移動(折り返し単位)
	case F_GOFILETOP				://ファイルの先頭に移動
	case F_GOFILEEND				://ファイルの最後に移動
	case F_WORDLEFT					://単語の左端に移動
	case F_WORDRIGHT				://単語の右端に移動
	case F_CURLINECENTER			://カーソル行をウィンドウ中央へ
	case F_JUMPHIST_PREV			://移動履歴: 前へ
	case F_JUMPHIST_NEXT			://移動履歴: 次へ
	case F_JUMPHIST_SET				://現在位置を移動履歴に登録
	case F_MODIFYLINE_NEXT			://次の変更行へ移動
	case F_MODIFYLINE_PREV			://前の変更行へ移動

	/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」が多くなったので独立化して(選択)を移動(サブメニュー化は構造上できないので)
	case F_SELECTWORD				://現在位置の単語選択
	case F_SELECTALL				://すべて選択
	case F_SELECTLINE				://1行選択	// 2007.10.06 nasukoji
	case F_BEGIN_SEL				://範囲選択開始
	case F_UP_SEL					://(範囲選択)カーソル上移動
	case F_DOWN_SEL					://(範囲選択)カーソル下移動
	case F_LEFT_SEL					://(範囲選択)カーソル左移動
	case F_RIGHT_SEL				://(範囲選択)カーソル右移動
	case F_UP2_SEL					://(範囲選択)カーソル上移動(２行ごと)
	case F_DOWN2_SEL				://(範囲選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_SEL				://(範囲選択)単語の左端に移動
	case F_WORDRIGHT_SEL			://(範囲選択)単語の右端に移動
	case F_GOLINETOP_SEL			://(範囲選択)行頭に移動(折り返し単位)
	case F_GOLINEEND_SEL			://(範囲選択)行末に移動(折り返し単位)
//	case F_ROLLDOWN_SEL				://(範囲選択)スクロールダウン
//	case F_ROLLUP_SEL				://(範囲選択)スクロールアップ
	// 2014.01.15 (Half)Page[Up/down] を有効化
	case F_HalfPageUp_Sel			://(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_HalfPageDown_Sel			://(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	case F_1PageUp_Sel				://(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	case F_1PageDown_Sel			://(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	case F_GOFILETOP_SEL			://(範囲選択)ファイルの先頭に移動
	case F_GOFILEEND_SEL			://(範囲選択)ファイルの最後に移動
	case F_MODIFYLINE_NEXT_SEL		://(範囲選択)次の変更行へ移動
	case F_MODIFYLINE_PREV_SEL		://(範囲選択)前の変更行へ移動

	/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
//	case F_BOXSELALL				//矩形ですべて選択
	case F_BEGIN_BOX				://矩形範囲選択開始

	case F_UP_BOX					://(矩形選択)カーソル上移動
	case F_DOWN_BOX					://(矩形選択)カーソル下移動
	case F_LEFT_BOX					://(矩形選択)カーソル左移動
	case F_RIGHT_BOX				://(矩形選択)カーソル右移動
	case F_UP2_BOX					://(矩形選択)カーソル上移動(２行ごと)
	case F_DOWN2_BOX				://(矩形選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_BOX				://(矩形選択)単語の左端に移動
	case F_WORDRIGHT_BOX			://(矩形選択)単語の右端に移動
	case F_GOLOGICALLINETOP_BOX		://(矩形選択)行頭に移動(改行単位)
	case F_GOLINETOP_BOX			://(矩形選択)行頭に移動(折り返し単位)
	case F_GOLINEEND_BOX			://(矩形選択)行末に移動(折り返し単位)
	case F_HalfPageUp_BOX			://(矩形選択)半ページアップ
	case F_HalfPageDown_BOX			://(矩形選択)半ページダウン
	case F_1PageUp_BOX				://(矩形選択)１ページアップ
	case F_1PageDown_BOX			://(矩形選択)１ページダウン
	case F_GOFILETOP_BOX			://(矩形選択)ファイルの先頭に移動
	case F_GOFILEEND_BOX			://(矩形選択)ファイルの最後に移動

	/* クリップボード系 */
	case F_CUT						://切り取り(選択範囲をクリップボードにコピーして削除)
	case F_COPY						://コピー(選択範囲をクリップボードにコピー)
	case F_COPY_ADDCRLF				://折り返し位置に改行をつけてコピー
	case F_COPY_CRLF				://CRLF改行でコピー(選択範囲を改行コード=CRLFでコピー)
	case F_PASTE					://貼り付け(クリップボードから貼り付け)
	case F_PASTEBOX					://矩形貼り付け(クリップボードから矩形貼り付け)
	case F_INSTEXT_W					://テキストを貼り付け
//	case F_ADDTAIL_W					://最後にテキストを追加
	case F_COPYLINES				://選択範囲内全行コピー
	case F_COPYLINESASPASSAGE		://選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER 	://選択範囲内全行行番号付きコピー
	case F_COPY_COLOR_HTML			://選択範囲内色付きHTMLコピー
	case F_COPY_COLOR_HTML_LINENUMBER://選択範囲内行番号色付きHTMLコピー
	case F_COPYPATH					://このファイルのパス名をクリップボードにコピー
	case F_COPYTAG					://このファイルのパス名とカーソル位置をコピー	//Sept. 15, 2000 jepro 上と同じ説明になっていたのを修正
	case F_COPYFNAME				://このファイル名をクリップボードにコピー // 2002/2/3 aroka
	case F_CREATEKEYBINDLIST		://キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO 追加	//Dec. 25, 2000 復活

	/* 挿入系 */
	case F_INS_DATE					:// 日付挿入
	case F_INS_TIME					:// 時刻挿入
//	case F_CTRL_CODE_DIALOG			://コントロールコードの入力(ダイアログ)	//@@@ 2002.06.02 MIK
	case F_CTRL_CODE				://コントロールコードの入力 2013.12.12

	/* 変換系 */
	case F_TOLOWER		 			://小文字
	case F_TOUPPER		 			://大文字
	case F_TOHANKAKU		 		:/* 全角→半角 */
	case F_TOHANKATA		 		:/* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	case F_TOZENEI			 		:/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	case F_TOHANEI			 		:/* 全角英数→半角英数 */
	case F_TOZENKAKUKATA	 		:/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	case F_TOZENKAKUHIRA	 		:/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	case F_HANKATATOZENKATA			:/* 半角カタカナ→全角カタカナ */
	case F_HANKATATOZENHIRA			:/* 半角カタカナ→全角ひらがな */
	case F_TABTOSPACE				:/* TAB→空白 */
	case F_SPACETOTAB				:/* 空白→TAB */  //---- Stonee, 2001/05/27
	case F_CODECNV_AUTO2SJIS		:/* 自動判別→SJISコード変換 */
	case F_CODECNV_EMAIL			://E-Mail(JIS→SJIS)コード変換
	case F_CODECNV_EUC2SJIS			://EUC→SJISコード変換
	case F_CODECNV_UNICODE2SJIS		://Unicode→SJISコード変換
	case F_CODECNV_UNICODEBE2SJIS	://UnicodeBE→SJISコード変換
	case F_CODECNV_UTF82SJIS		:/* UTF-8→SJISコード変換 */
	case F_CODECNV_UTF72SJIS		:/* UTF-7→SJISコード変換 */
	case F_CODECNV_SJIS2JIS			:/* SJIS→JISコード変換 */
	case F_CODECNV_SJIS2EUC			:/* SJIS→EUCコード変換 */
	case F_CODECNV_SJIS2UTF8		:/* SJIS→UTF-8コード変換 */
	case F_CODECNV_SJIS2UTF7		:/* SJIS→UTF-7コード変換 */
//	case F_BASE64DECODE	 			://Base64デコードして保存
//	case F_UUDECODE		 			://uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更

	/* 検索系 */
//	case F_SEARCH_DIALOG			://検索(単語検索ダイアログ)
	case F_SEARCH_NEXT				://次を検索
	case F_SEARCH_PREV				://前を検索
	case F_REPLACE					://置換(実行)
	case F_REPLACE_ALL				://すべて置換(実行)
	case F_SEARCH_CLEARMARK			://検索マークのクリア
	case F_JUMP_SRCHSTARTPOS		://検索開始位置へ戻る		// 02/06/26 ai
	case F_GREP						://Grep
//	case F_JUMP_DIALOG				://指定行ヘジャンプ
	case F_JUMP						://指定行へジャンプ @@@ 2002.2.2 YAZAKI
//	case F_OUTLINE					://アウトライン解析
	case F_TAGJUMP					://タグジャンプ機能
	case F_TAGJUMPBACK				://タグジャンプバック機能
//	case F_TAGS_MAKE				://タグファイルの作成	//@@@ 2003.04.13 MIK
//	case F_COMPARE					://ファイル内容比較
//	case F_DIFF_DIALOG				://DIFF差分表示(ダイアログ)	//@@@ 2002.05.25 MIK
//	case F_DIFF						://DIFF差分表示				//@@@ 2002.05.25 MIK
//	case F_DIFF_NEXT				://DIFF差分表示(次へ)		//@@@ 2002.05.25 MIK
//	case F_DIFF_PREV				://DIFF差分表示(前へ)		//@@@ 2002.05.25 MIK
//	case F_DIFF_RESET				://DIFF差分表示(全解除)		//@@@ 2002.05.25 MIK
	case F_BRACKETPAIR				://対括弧の検索
// From Here 2001.12.03 hor
	case F_BOOKMARK_SET				://ブックマーク設定・解除
	case F_BOOKMARK_NEXT			://次のブックマークへ
	case F_BOOKMARK_PREV			://前のブックマークへ
	case F_BOOKMARK_RESET			://ブックマークの全解除
//	case F_BOOKMARK_VIEW			://ブックマークの一覧
// To Here 2001.12.03 hor
	case F_BOOKMARK_PATTERN			://検索しして該当行をマーク	// 2002.02.08 hor
	case F_FUNCLIST_NEXT			://次の関数リストマークへ
	case F_FUNCLIST_PREV			://前の関数リストマークへ

	/* モード切り替え系 */
	case F_CHGMOD_INS				://挿入／上書きモード切り替え
	case F_CHG_CHARSET				://文字コードセット指定	2010/6/14 Uchi
	case F_CHGMOD_EOL				://入力改行コード指定	2003.06.23 Moca

	case F_CANCEL_MODE				://各種モードの取り消し

	/* マクロ系 */
//	case F_RECKEYMACRO				://キーマクロの記録開始／終了
//	case F_SAVEKEYMACRO				://キーマクロの保存
//	case F_LOADKEYMACRO				://キーマクロの読み込み
//	case F_EXECKEYMACRO				://キーマクロの実行
	case F_EXECEXTMACRO				://名前を指定してマクロ実行

	/* 設定系 */
//	case F_SHOWTOOLBAR				:/* ツールバーの表示 */
//	case F_SHOWFUNCKEY				:/* ファンクションキーの表示 */
//	case F_SHOWTAB					:/* タブの表示 */
//	case F_SHOWSTATUSBAR			:/* ステータスバーの表示 */
//	case F_TYPE_LIST				:/* タイプ別設定一覧 */
//	case F_OPTION_TYPE				:/* タイプ別設定 */
//	case F_OPTION					:/* 共通設定 */
//	case F_FONT						:/* フォント設定 */
	case F_SETFONTSIZE				:// フォントサイズ設定
//	case F_WRAPWINDOWWIDTH			:/* 現在のウィンドウ幅で折り返し */	//Oct. 15, 2000 JEPRO
//	case F_FAVORITE					:/* 履歴の管理 */	//@@@ 2003.04.08 MIK
//	case F_TMPWRAPNOWRAP			:// 折り返さない（一時設定）		// 2008.05.30 nasukoji
//	case F_TMPWRAPSETTING			:// 指定桁で折り返す（一時設定）	// 2008.05.30 nasukoji
//	case F_TMPWRAPWINDOW			:// 右端で折り返す（一時設定）		// 2008.05.30 nasukoji
	case F_TEXTWRAPMETHOD			:// テキストの折り返し方法			// 2008.05.30 nasukoji
	case F_SELECT_COUNT_MODE		:// 文字カウントの方法を取得、設定	// 2009.07.06 syat

	case F_EXECMD					:/* 外部コマンド実行 */	//@@@2002.2.2 YAZAKI 追加

	/* カスタムメニュー */
//	case F_MENU_RBUTTON				:/* 右クリックメニュー */
//	case F_CUSTMENU_1				:/* カスタムメニュー1 */
//	case F_CUSTMENU_2				:/* カスタムメニュー2 */
//	case F_CUSTMENU_3				:/* カスタムメニュー3 */
//	case F_CUSTMENU_4				:/* カスタムメニュー4 */
//	case F_CUSTMENU_5				:/* カスタムメニュー5 */
//	case F_CUSTMENU_6				:/* カスタムメニュー6 */
//	case F_CUSTMENU_7				:/* カスタムメニュー7 */
//	case F_CUSTMENU_8				:/* カスタムメニュー8 */
//	case F_CUSTMENU_9				:/* カスタムメニュー9 */
//	case F_CUSTMENU_10				:/* カスタムメニュー10 */
//	case F_CUSTMENU_11				:/* カスタムメニュー11 */
//	case F_CUSTMENU_12				:/* カスタムメニュー12 */
//	case F_CUSTMENU_13				:/* カスタムメニュー13 */
//	case F_CUSTMENU_14				:/* カスタムメニュー14 */
//	case F_CUSTMENU_15				:/* カスタムメニュー15 */
//	case F_CUSTMENU_16				:/* カスタムメニュー16 */
//	case F_CUSTMENU_17				:/* カスタムメニュー17 */
//	case F_CUSTMENU_18				:/* カスタムメニュー18 */
//	case F_CUSTMENU_19				:/* カスタムメニュー19 */
//	case F_CUSTMENU_20				:/* カスタムメニュー20 */
//	case F_CUSTMENU_21				:/* カスタムメニュー21 */
//	case F_CUSTMENU_22				:/* カスタムメニュー22 */
//	case F_CUSTMENU_23				:/* カスタムメニュー23 */
//	case F_CUSTMENU_24				:/* カスタムメニュー24 */

	/* ウィンドウ系 */
//	case F_SPLIT_V					://上下に分割	//Sept. 16, 2000 jepro 説明を「縦」から「上下に」に変更
//	case F_SPLIT_H					://左右に分割	//Sept. 16, 2000 jepro 説明を「横」から「左右に」に変更
//	case F_SPLIT_VH					://縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
//	case F_WINCLOSE					://ウィンドウを閉じる
//	case F_WIN_CLOSEALL				://すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
//	case F_NEXTWINDOW				://次のウィンドウ
//	case F_PREVWINDOW				://前のウィンドウ
//	case F_CASCADE					://重ねて表示
//	case F_TILE_V					://上下に並べて表示
//	case F_TILE_H					://左右に並べて表示
//	case F_MAXIMIZE_V				://縦方向に最大化
//	case F_MINIMIZE_ALL				://すべて最小化	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	case F_REDRAW					://再描画
	case F_WIN_OUTPUT				://アウトプットウィンドウ表示
//	case F_TRACEOUT					://マクロ用アウトプットウィンドウに表示	2006.04.26 maru
	case F_TOPMOST					://常に手前に表示
//	case F_GROUPCLOSE				://グループを閉じる	// 2007.06.20 ryoji
//	case F_NEXTGROUP				://次のグループ	// 2007.06.20 ryoji
//	case F_PREVGROUP				://前のグループ	// 2007.06.20 ryoji
//	case F_TAB_MOVERIGHT			://タブを右に移動	// 2007.06.20 ryoji
//	case F_TAB_MOVELEFT				://タブを左に移動	// 2007.06.20 ryoji
//	case F_TAB_SEPARATE				://新規グループ	// 2007.06.20 ryoji
//	case F_TAB_JOINTNEXT			://次のグループに移動	// 2007.06.20 ryoji
//	case F_TAB_JOINTPREV			://前のグループに移動	// 2007.06.20 ryoji

	/* 支援 */
//  case F_HOKAN					:/* 入力補完 */				//Oct. 15, 2000 JEPRO 入ってなかったので入れてみた
//	case F_HELP_CONTENTS			:/* ヘルプ目次 */			//Dec. 25, 2000 JEPRO 追加
//	case F_HELP_SEARCH				:/* ヘルプキーワード検索 */	//Dec. 25, 2000 JEPRO 追加
//	case F_MENU_ALLFUNC				:/* コマンド一覧 */
//	case F_EXTHELP1					:/* 外部ヘルプ１ */
//	case F_EXTHTMLHELP				:/* 外部HTMLヘルプ */
//	case F_ABOUT					:/* バージョン情報 */		//Dec. 25, 2000 JEPRO 追加

	/* その他 */
		return TRUE;
	}
	return FALSE;

}

/*!
	マクロ番号から対応するマクロオブジェクト格納位置へのポインタへの変換
	
	@param idx [in] マクロ番号(0-), STAND_KEYMACROは標準キーマクロバッファ、TEMP_KEYMACROは一時マクロバッファを表す．
	@return オブジェクト位置へのポインタ．マクロ番号が不当な場合はNULL．
*/
CMacroManagerBase** CSMacroMgr::Idx2Ptr(int idx)
{
	//	Jun. 16, 2002 genta
	//	キーマクロ以外のマクロを読み込めるように
	if ( idx == STAND_KEYMACRO ){
		return &m_pKeyMacro;
	}
	else if ( idx == TEMP_KEYMACRO ){
		return &m_pTempMacro;
	}
	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
		return &m_cSavedKeyMacro[idx];
	}

	DEBUG_TRACE( _T("CSMacroMgr::Idx2Ptr() Out of range: idx=%d\n"), idx);

	return NULL;
}

/*!
	キーボードマクロの保存が可能かどうか
	
	@retval true 保存可能
	@retval false 保存不可
*/
bool CSMacroMgr::IsSaveOk(void)
{
	return dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro ) == NULL ? false : true;
}

/*!
	一時マクロを交換する
	
	@param newMacro [in] 新しいマクロバッファのポインタ．
	@return 前の一時マクロバッファのポインタ．
*/
CMacroManagerBase* CSMacroMgr::SetTempMacro( CMacroManagerBase *newMacro )
{
	CMacroManagerBase *oldMacro = m_pTempMacro;

	m_pTempMacro = newMacro;

	return oldMacro;
}
