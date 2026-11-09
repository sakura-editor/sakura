/*!	@file
	@brief インポート、エクスポートマネージャ

	@author Uchi
	@date 2010/4/22 新規作成
*/
/*
	Copyright (C) 2010, Uchi, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include <memory>
#include "CImpExpManager.h"
#include "typeprop/CDlgTypeAscertain.h"

#include "dlg/CDlgOpenFile.h"
#include "io/CTextStream.h"
#include "env/CShareData_IO.h"
#include "plugin/CPlugin.h"
#include "view/CEditView.h"
#include "view/colors/CColorStrategy.h"
#include "apiwrap/StdControl.h"
#include "CSelectLang.h"
#include "config/app_constants.h"

/*-----------------------------------------------------------------------
定数
-----------------------------------------------------------------------*/
static const WCHAR	szSecInfo[]				= LTEXT("Info");

// タイプ別設定
static const WCHAR	WSTR_TYPE_HEAD[]		= LTEXT(" タイプ別設定 Ver1");

static const WCHAR	szSecTypeEx[]			= LTEXT("TypeEx");
static const WCHAR	szSecTypes[]			= LTEXT("Types");

static const WCHAR	szKeyKeywordTemp[]				= LTEXT("szKeyword[%d]");
static const WCHAR	szKeyKeywordFileTemp[]			= LTEXT("szKeywordFile[%d]");
static const WCHAR	szKeyKeywordCaseTemp[]			= LTEXT("szKeywordCase[%d]");
static const WCHAR	szKeyPluginOutlineName[]		= LTEXT("szPluginOutlineName");
static const WCHAR	szKeyPluginOutlineId[]			= LTEXT("szPluginOutlineId");
static const WCHAR	szKeyPluginSmartIndentName[]	= LTEXT("szPluginSmartIndentName");
static const WCHAR	szKeyPluginSmartIndentId[]		= LTEXT("szPluginSmartIndentId");
static const WCHAR	szKeyVersion[]					= LTEXT("szVersion");
static const WCHAR	szKeyStructureVersion[]			= LTEXT("vStructureVersion");

// カラー（ CPropTypes.hからコピー改変 ）
//static const wchar_t	WSTR_COLORDATA_HEAD2[]	=  L" テキストエディタ色設定 Ver2";
//static const wchar_t	WSTR_COLORDATA_HEAD21[]	=  L" テキストエディタ色設定 Ver2.1";	//Nov. 2, 2000 JEPRO 変更 [注]. 0.3.9.0:ur3β10以降、設定項目の番号を入れ替えたため
static const WCHAR	WSTR_COLORDATA_HEAD3[]	=  LTEXT(" テキストエディタ色設定 Ver3");		//Jan. 15, 2001 Stonee  色設定Ver3ドラフト(設定ファイルのキーを連番→文字列に)	//Feb. 11, 2001 JEPRO 有効にした
//static const wchar_t	WSTR_COLORDATA_HEAD4[]	=  L" テキストエディタ色設定 Ver4";		//2007.10.02 kobake UNICODE化に際してカラーファイル仕様も変更
static const WCHAR	szSecColor[]			=  LTEXT("SakuraColor");

// 正規表現キーワード
static const WCHAR	WSTR_REGEXKW_HEAD[]		= LTEXT("// 正規表現キーワード Ver1\n");

// キーワードヘルプ
static const WCHAR	WSTR_KEYHELP_HEAD[]		= LTEXT("// キーワード辞書設定 Ver1\n");

// キー割り当て
static const WCHAR	WSTR_KEYBIND_HEAD4[]		= LTEXT("SakuraKeyBind_Ver4");	//2013.12.05 syat 多言語対応
static const WCHAR	WSTR_KEYBIND_HEAD3[]	= LTEXT("SakuraKeyBind_Ver3");	//2007.10.05 kobake ファイル形式をini形式に変更
static const WCHAR	WSTR_KEYBIND_HEAD2[]	= LTEXT("// テキストエディタキー設定 Ver2");	// (旧バージョン(ANSI版)） 読み込みのみ対応 2008/5/3 by Uchi

// カスタムメニューファイル
// 2007.10.02 kobake UNICODE化に際して、カスタムメニューファイルの仕様を変更
static       WCHAR	WSTR_CUSTMENU_HEAD_V2[]	= LTEXT("SakuraEditorMenu_Ver2");

// キーワード定義ファイル
static const WCHAR	WSTR_KEYWORD_HEAD[]		= LTEXT(" キーワード定義ファイル\n");
static const WCHAR	WSTR_KEYWORD_CASE[]		= LTEXT("// CASE=");
static const WCHAR	WSTR_CASE_TRUE[]		= LTEXT("// CASE=True");
static const WCHAR	WSTR_CASE_FALSE[]		= LTEXT("// CASE=False");

// メインメニューファイル
static       WCHAR	WSTR_MAINMENU_HEAD_V1[]	= LTEXT("SakuraEditorMainMenu Ver1");

static       WCHAR	WSTR_FILETREE_HEAD_V1[]	= LTEXT("SakuraEditorFileTree_Ver1");

// Exportファイル名の作成
//	  タイプ名などファイルとして扱うことを考えていない文字列を扱う
//		2010/4/12 Uchi
template <basis::WritableBuffer<WCHAR> A>
static WCHAR* MakeExportFileName(A& dst, const WCHAR* trg, const WCHAR* ext)
{
	assert(trg);
	assert(ext);

	auto res = std::data(dst);

	// 作業バッファにコピーする
	SFilePath work;
	::wcscpy_s(work, trg);

	// 書き込み可能な生ポインタを得る
	auto conv = work.data();

	// 作業バッファの先頭を得る
	auto p = conv;
	while ( (p = wcspbrk( p, LTEXT("\t\\:*?\"<>|") )) != nullptr ) {
		// ファイル名に使えない文字を _ に置き換える
		*p++ = LCHAR('_');
	}

	// 作業バッファの先頭を得る
	p = conv;
	while ( (p = wcspbrk( p, LTEXT("/") )) != nullptr ) {
		// ファイル名に使えない文字を ／ に置き換える
		*p++ = LCHAR('／');
	}

	auto_sprintf_s(dst, LTEXT("%ls.%ls"), conv, ext);

	return res;
}

// インポート ファイル指定付き
bool CImpExpManager::ImportUI( HINSTANCE hInstance, HWND hwndParent )
{
	/* ファイルオープンダイアログの初期化 */
	auto& cDlgOpenFile = *CDlgOpenFile::getInstance();
	cDlgOpenFile.Create(
		hInstance,
		hwndParent,
		GetDefaultExtension(),
		GetDllShareData().m_sHistory.m_szIMPORTFOLDER // インポート用フォルダー
	);
	WCHAR	szPath[_MAX_PATH + 1];
	szPath[0] = LCHAR('\0');
	if( !GetFileName().empty() ){
		wcscpy( szPath, GetFullPath().c_str());
	}
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return false;
	}

	const sakura::wstring sPath = szPath;
	sakura::wstring	sErrMsg;

	// 確認
	if (!ImportAscertain( hInstance, hwndParent, sPath, sErrMsg )) {
		if (sErrMsg.length() > 0) {
			ErrorMessage( hwndParent, LTEXT("%ls"), sErrMsg.c_str() );
		}
		return false;
	}

	// Import Folderの設定
	SetImportFolder( szPath );

	// Import
	if (!Import( sPath, sErrMsg )) {
		ErrorMessage( hwndParent, LTEXT("%ls"), sErrMsg.c_str() );
		return false;
	}

	if (sErrMsg.length() > 0) {
		InfoMessage( hwndParent, LTEXT("%ls"), sErrMsg.c_str() );
	}

	return true;
}

// エクスポート ファイル指定付き
bool CImpExpManager::ExportUI( HINSTANCE hInstance, HWND hwndParent )
{
	/* ファイルオープンダイアログの初期化 */
	auto& cDlgOpenFile = *CDlgOpenFile::getInstance();
	cDlgOpenFile.Create(
		hInstance,
		hwndParent,
		GetDefaultExtension(),
		GetDllShareData().m_sHistory.m_szIMPORTFOLDER // インポート用フォルダー
	);
	WCHAR			szPath[_MAX_PATH + 1];
	szPath[0] = LCHAR('\0');
	if( !GetFileName().empty() ){
		wcscpy( szPath, GetFullPath().c_str());
	}
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return false;
	}

	// Import Folderの設定
	SetImportFolder( szPath );

	// Export
	const sakura::wstring sPath = szPath;
	sakura::wstring	sErrMsg;

	if (!Export( sPath, sErrMsg )) {
		ErrorMessage( hwndParent, LTEXT("%ls"), sErrMsg.c_str() );
		return false;
	}

	if (sErrMsg.length() == 0) {
		sErrMsg = LS(STR_IMPEXP_OK_EXPORT) + sPath;
	}
	InfoMessage( hwndParent, LTEXT("%ls"), sErrMsg.c_str() );

	return true;
}

// インポート確認
bool CImpExpManager::ImportAscertain( [[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HWND hwndParent, [[maybe_unused]] const sakura::wstring& sFileName, [[maybe_unused]] sakura::wstring& sErrMsg )
{
	return true;
}

// デフォルト拡張子の取得
const WCHAR* CImpExpManager::GetDefaultExtension()
{
	return LTEXT("");
}
const WCHAR* CImpExpManager::GetOriginExtension()
{
	return LTEXT("");
}

// ファイル名の初期値を設定
void CImpExpManager::SetBaseName(const sakura::wstring& sBase)
{
	WCHAR		wbuff[_MAX_PATH + 1];

	m_sBase = sBase;
	m_sOriginName = MakeExportFileName( wbuff, sBase.c_str(), GetOriginExtension() );
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          タイプ別設定                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// インポート確認
bool CImpExpType::ImportAscertain( HINSTANCE hInstance, HWND hwndParent, const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	const auto& sPath = sFileName;

	m_cProfile.SetReadingMode();

	if (!m_cProfile.ReadProfile( sPath.c_str() )) {
		/* 設定ファイルが存在しない */
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}

	// Check Version
	int		nStructureVersion = 0;
	if (!m_cProfile.IOProfileData( szSecInfo, szKeyStructureVersion, nStructureVersion )) {
		sErrMsg = LS(STR_IMPEXP_ERR_TYPE);
		return false;
	}
	if ((unsigned int)nStructureVersion != m_pShareData->m_vStructureVersion) {
		sakura::wstring	strKeyVerValue = LTEXT("?");
		m_cProfile.IOProfileData(szSecInfo, szKeyVersion, strKeyVerValue);
		int nRet = ConfirmMessage( hwndParent,
			LS(STR_IMPEXP_VER), 
			GSTR_APPNAME, strKeyVerValue.c_str(), nStructureVersion );
		if ( IDYES != nRet ) {
			return false;
		}
	}

	// 確認＆色指定
	CDlgTypeAscertain::SAscertainInfo	sAscertainInfo;
	CDlgTypeAscertain	cDlgTypeAscertain;
	WCHAR szLabel[1024];
	STypeConfig TmpType;

	// パラメータの設定
	sAscertainInfo.sImportFile = sPath;
	ApiWrap::List_GetText( m_hwndList, m_nIdx, szLabel );
	sAscertainInfo.sTypeNameTo = szLabel;
	szLabel[0] = LCHAR('\0');
	m_cProfile.IOProfileData(szSecTypes, LTEXT("szTypeName"), StringBufferW(szLabel));
	sAscertainInfo.sTypeNameFile = szLabel;

	// 確認
	if (!cDlgTypeAscertain.DoModal( hInstance, hwndParent, &sAscertainInfo )) {
		return false;
	}

	m_nColorType = sAscertainInfo.nColorType;
	m_sColorFile = sAscertainInfo.sColorFile;
	m_bAddType   = sAscertainInfo.bAddType;

	return true;
}

// インポート
bool CImpExpType::Import( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	sakura::wstring	files = LTEXT("");
	sakura::wstring TmpMsg;
	decltype(m_Types.m_ColorInfoArr)	colorInfoArr{};				// 色設定配列(バックアップ)
	int		i;

	// 色の変更
	if (m_nColorType >= MAX_TYPES) {
		// 色設定インポート
		CImpExpColors	cImpExpColors( colorInfoArr );
		if (cImpExpColors.Import( cImpExpColors.MakeFullPath( m_sColorFile ), TmpMsg)) {
			files += sakura::wstring(LTEXT("\n")) + m_sColorFile;
		}
		else {
			// 失敗したら基本をコピー(メッセージは出さない)
			memcpy( &colorInfoArr, GetDllShareData().m_TypeBasis.m_ColorInfoArr, sizeof(colorInfoArr) );
			files += sakura::wstring(LTEXT("\n× ")) + m_sColorFile;	// 失敗
		}
	}
	else if (m_nColorType >= 0 ) {
		// 色指定(内部)
		STypeConfig type;
		CDocTypeManager().GetTypeConfig(CTypeConfig(m_nColorType), type);
		memcpy( &colorInfoArr, type.m_ColorInfoArr, sizeof(colorInfoArr) );
	}

	// 読み込み
	ShareData_IO_TypeConfig(m_cProfile, szSecTypes, m_Types);

	m_nIdx = m_Types.m_nIdx;
	if (m_nIdx == 0) {
		// 基本の場合の名前と拡張子を初期化
		wcscpy( m_Types.m_szTypeName, LS(STR_TYPE_NAME_BASIS) );
		m_Types.m_szTypeExts[0] = LCHAR('\0');
		m_Types.m_id = 0;
	}else{
		// 基本じゃなかった場合、id番号をランダムに仮採番(あとで振りなおす)
		m_Types.m_id = (::GetTickCount() & 0x3fffffff) + m_nIdx * 0x10000;
	}

	// 色の設定
	if (m_nColorType >= 0 ) {
		// 色指定あり
		for (i = 0; i < int(std::size(colorInfoArr)); i++) {
			bool bDisp = m_Types.m_ColorInfoArr[i].m_bDisp;
			m_Types.m_ColorInfoArr[i] = colorInfoArr[i];
			m_Types.m_ColorInfoArr[i].m_bDisp = bDisp;		// 表示フラグはファイルのものを使用する
		}
	}

	// 共通設定との連結部
	WCHAR	szKeyName[64];
	WCHAR	szKeyData[1024];
	int		nIdx;
	int		nPlug = 0;
	int		nDataLen;
	WCHAR* pSlashPos;
	WCHAR	szFileName[_MAX_PATH+1];
	bool	bCase;
	CommonSetting& common = m_pShareData->m_Common;

	// 強調キーワード
	CKeyWordSetMgr&	cKeyWordSetMgr = common.m_sSpecialKeyword.m_CKeyWordSetMgr;
	for (i=0; i < MAX_KEYWORDSET_PER_TYPE; i++) {
		//types.m_nKeyWordSetIdx[i] = -1;
		auto_sprintf( szKeyName, szKeyKeywordTemp, i+1 );
		if (m_cProfile.IOProfileData(szSecTypeEx, szKeyName, StringBufferW(szKeyData))) {
			nIdx = cKeyWordSetMgr.SearchKeyWordSet( szKeyData );
			if (nIdx < 0) {
				// エントリ作成
				cKeyWordSetMgr.AddKeyWordSet( szKeyData, false );
				nIdx = cKeyWordSetMgr.SearchKeyWordSet( szKeyData );
			}
			if (nIdx >= 0) {
				auto_sprintf( szKeyName, szKeyKeywordCaseTemp, i+1 );
				bCase = false;		// 大文字小文字区別しない (Defaule)
				m_cProfile.IOProfileData( szSecTypeEx, szKeyName, bCase );

				// キーワード定義ファイル入力
				CImpExpKeyWord	cImpExpKeyWord( common, nIdx, bCase );

				auto_sprintf( szKeyName, szKeyKeywordFileTemp, i+1 );
				szFileName[0] = LCHAR('\0');
				if (m_cProfile.IOProfileData(szSecTypeEx, szKeyName, StringBufferW(szFileName))) {
					if( cImpExpKeyWord.Import( cImpExpKeyWord.MakeFullPath( szFileName ), TmpMsg )) {
						files += sakura::wstring(LTEXT("\n")) + szFileName;
					} else {
						files += sakura::wstring(LTEXT("\n× ")) + szFileName;	// 失敗
					}
				}
			}
			m_Types.m_nKeyWordSetIdx[i] = nIdx;
		}
	}

	// Plugin
	//  アウトライン解析方法
	CommonSetting_Plugin& plugin = common.m_sPlugin;
	if (m_cProfile.IOProfileData(szSecTypeEx, szKeyPluginOutlineId, StringBufferW(szKeyData))) {
		nDataLen = (int)wcsnlen( szKeyData, _countof(szKeyData) );
		pSlashPos = wcschr( szKeyData, LCHAR('/') );
		nIdx = -1;
		for (i = 0; i < MAX_PLUGIN; i++) {
			if (wcsncmp(szKeyData, plugin.m_PluginTable[i].m_szId, pSlashPos ? pSlashPos-szKeyData : nDataLen) == 0) {
				nIdx = i;
				if (pSlashPos) {	// スラッシュの後ろのプラグIDを取得
					nPlug = _wtoi( pSlashPos + 1 );
				} else {
					nPlug = 0;
				}
				break;
			}
		}
		// 2010.08.21 0が範囲から漏れていた
		if (nIdx >= 0) {
			m_Types.m_eDefaultOutline = CPlug::GetOutlineType( CPlug::GetPluginFunctionCode(nIdx, nPlug) );
		}
	}
	//  スマートインデント
	if (m_cProfile.IOProfileData(szSecTypeEx, szKeyPluginSmartIndentId, StringBufferW(szKeyData))) {
		nDataLen = (int)wcsnlen( szKeyData, _countof(szKeyData) );
		pSlashPos = wcschr( szKeyData, LCHAR('/') );
		nIdx = -1;
		for (i = 0; i < MAX_PLUGIN; i++) {
			if (wcsncmp(szKeyData, plugin.m_PluginTable[i].m_szId, pSlashPos ? pSlashPos-szKeyData : nDataLen) == 0) {
				nIdx = i;
				if (pSlashPos) {	// スラッシュの後ろのプラグIDを取得
					nPlug = _wtoi( pSlashPos + 1 );
				} else {
					nPlug = 0;
				}
				break;
			}
		}
		// 2010.08.21 0が範囲から漏れていた
		if (nIdx >= 0) {
			m_Types.m_eSmartIndent = CPlug::GetSmartIndentType( CPlug::GetPluginFunctionCode(nIdx, nPlug) );
		}
	}

	sErrMsg = LS(STR_IMPEXP_OK_IMPORT);
	sErrMsg += sFileName;
	sErrMsg += files;
	return true;
}

// エクスポート
bool CImpExpType::Export( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	CDataProfile	cProfile;

	cProfile.SetWritingMode();

	ShareData_IO_TypeConfig(cProfile, szSecTypes, m_Types);

	// 共通設定との連結部
	int		i;
	WCHAR	szKeyName[64];
	WCHAR buff[MAX_SETNAMELEN+1];
	WCHAR	szFileName[_MAX_PATH+1];
	bool	bCase;
	sakura::wstring	files = LTEXT("");
	sakura::wstring	sTmpMsg;
	int		nIdx;
	CommonSetting& common  = m_pShareData->m_Common;

	// 強調キーワード
	CKeyWordSetMgr&	cKeyWordSetMgr = common.m_sSpecialKeyword.m_CKeyWordSetMgr;
	for (i=0; i < MAX_KEYWORDSET_PER_TYPE; i++) {
		if (m_Types.m_nKeyWordSetIdx[i] >= 0) {
			nIdx = m_Types.m_nKeyWordSetIdx[i];
			auto_sprintf( szKeyName, szKeyKeywordTemp, i+1 );
			wcscpy( buff, cKeyWordSetMgr.GetTypeName( nIdx ));
			cProfile.IOProfileData(szSecTypeEx, szKeyName, StringBufferW(buff));

			// 大文字小文字区別
			bCase = common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordCase( nIdx );

			// キーワード定義ファイル出力
			CImpExpKeyWord	cImpExpKeyWord( common, m_Types.m_nKeyWordSetIdx[i], bCase );
			cImpExpKeyWord.SetBaseName( common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( nIdx ));

			if ( cImpExpKeyWord.Export( cImpExpKeyWord.GetFullPath(), sTmpMsg ) ) {
				wcscpy( szFileName, cImpExpKeyWord.GetFileName().c_str());
				auto_sprintf( szKeyName, szKeyKeywordFileTemp, i+1 );
				if (cProfile.IOProfileData(szSecTypeEx, szKeyName, StringBufferW(szFileName))) {
					files += sakura::wstring( LTEXT("\n") ) + cImpExpKeyWord.GetFileName();
				}
			}

			auto_sprintf( szKeyName, szKeyKeywordCaseTemp, i+1 );
			cProfile.IOProfileData( szSecTypeEx, szKeyName, bCase );
		}
	}

	// Plugin
	//  アウトライン解析方法
	CommonSetting_Plugin& plugin = common.m_sPlugin;
	int		nPIdx;
	int		nPlug;
	WCHAR szId[ MAX_PLUGIN_ID + 1 + 2 ];
	if ((nPIdx = CPlug::GetPluginId( static_cast<EFunctionCode>( m_Types.m_eDefaultOutline ))) >= 0) {
		cProfile.IOProfileData(szSecTypeEx, szKeyPluginOutlineName, StringBufferW(plugin.m_PluginTable[nPIdx].m_szName));
		::wcsncpy_s(szId, plugin.m_PluginTable[nPIdx].m_szId, _TRUNCATE);
		if( (nPlug = CPlug::GetPlugId( static_cast<EFunctionCode>( m_Types.m_eDefaultOutline ))) != 0 ){
			WCHAR szPlug[8];
			_swprintf( szPlug, LTEXT("/%d"), nPlug );
			wcscat( szId, szPlug );
		}
		cProfile.IOProfileData(szSecTypeEx, szKeyPluginOutlineId, StringBufferW(szId));
	}
	//  スマートインデント
	if ((nPIdx = CPlug::GetPluginId( static_cast<EFunctionCode>( m_Types.m_eSmartIndent ))) >= 0) {
		cProfile.IOProfileData(szSecTypeEx, szKeyPluginSmartIndentName, StringBufferW(plugin.m_PluginTable[nPIdx].m_szName));
		::wcsncpy_s(szId, plugin.m_PluginTable[nPIdx].m_szId, _TRUNCATE);
		if( (nPlug = CPlug::GetPlugId( static_cast<EFunctionCode>( m_Types.m_eSmartIndent ))) != 0 ){
			WCHAR szPlug[8];
			_swprintf( szPlug, LTEXT("/%d"), nPlug );
			wcscat( szId, szPlug );
		}
		cProfile.IOProfileData(szSecTypeEx, szKeyPluginSmartIndentId, StringBufferW(szId));
	}

	// Version
	DLLSHAREDATA* pShare = &GetDllShareData();
	int		nStructureVersion;
	WCHAR	wbuff[_MAX_PATH + 1];
	auto_sprintf( wbuff, LTEXT("%d.%d.%d.%d"), 
				HIWORD( pShare->m_sVersion.m_dwProductVersionMS ),
				LOWORD( pShare->m_sVersion.m_dwProductVersionMS ),
				HIWORD( pShare->m_sVersion.m_dwProductVersionLS ),
				LOWORD( pShare->m_sVersion.m_dwProductVersionLS ) );
	cProfile.IOProfileData(szSecInfo, szKeyVersion, StringBufferW(wbuff));
	nStructureVersion = int(pShare->m_vStructureVersion);
	cProfile.IOProfileData( szSecInfo, szKeyStructureVersion, nStructureVersion );

	// 書き込み
	if (!cProfile.WriteProfile( sFileName.c_str(), WSTR_TYPE_HEAD )) {
		sErrMsg = LS(STR_IMPEXP_ERR_EXPORT);
		sErrMsg += sFileName;
		return false;
	}

	sErrMsg = LS(STR_IMPEXP_OK_EXPORT);
	sErrMsg += sFileName;
	sErrMsg += files;

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          カラー                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// インポート
bool CImpExpColors::Import( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	const auto& strPath = sFileName;

	// 開けるか
	CTextInputStream in( strPath.c_str() );
	if (!in) {
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}

	/* ファイル先頭 */
	//ヘッダー読取
	sakura::wstring szHeader = in.ReadLineW();
	if(szHeader.length()>=2) {
		//コメントを抜く
		szHeader = &szHeader.c_str()[ szHeader.c_str()[0] == LCHAR(';') ? 1 : 2];
	}
	//比較
	if (szHeader != WSTR_COLORDATA_HEAD3) {
		in.Close();
		sErrMsg = LS(STR_IMPEXP_ERR_COLOR_OLD);	// 旧バージョンの説明の削除 2010/4/22 Uchi
		sErrMsg += sFileName;
		return false;
	}
	in.Close();

	CDataProfile	cProfile;
	cProfile.SetReadingMode();

	/* 色設定Ver3 */
	if( !cProfile.ReadProfile( strPath.c_str() ) ){
		return false;
	}

	/* 色設定 I/O */
	ShareData_IO_ColorSet(cProfile, szSecColor, std::span{ m_ColorInfoArr, size_t(COLORIDX_LAST) });

	return true;
}

// エクスポート
bool CImpExpColors::Export( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	/* 色設定 I/O */
	CDataProfile	cProfile;
	cProfile.SetWritingMode();
	ShareData_IO_ColorSet(cProfile, szSecColor, std::span{ m_ColorInfoArr, size_t(COLORIDX_LAST) });
	if (!cProfile.WriteProfile( sFileName.c_str(), WSTR_COLORDATA_HEAD3 )) { //Jan. 15, 2001 Stonee
		sErrMsg = LS(STR_IMPEXP_ERR_EXPORT);
		sErrMsg += sFileName;
		return false;
	}

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    正規表現キーワード                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// インポート
bool CImpExpRegex::Import( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	CTextInputStream in( sFileName.c_str() );
	if( !in ){
		// L"ファイルを開けませんでした。\n\n"
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}
	auto& regexKeyArr = m_Types.m_RegexKeywordArr;
	auto& szKeyWordList = m_Types.m_RegexKeywordList;
	WCHAR* pKeyword = &szKeyWordList[0];
	size_t keywordPos = 0;
	size_t count = 0;
	while( in ){
		//1行読み込み
		sakura::wstring line = in.ReadLineW();
		//RxKey[999]=ColorName,RegexKeyword
		if( line.length() < 12 ) continue;
		if( wmemcmp(&line[0], LTEXT("RxKey["), 6) != 0 ) continue;
		if( wmemcmp(&line[9], LTEXT("]="), 2) != 0 ) continue;
		auto sepPos = line.find_first_of(LCHAR(','), 11); // ColorName と RegexKeyword の間の区切り文字の存在を確認する
		if( sepPos == decltype(line)::npos ) continue;
		line[sepPos] = LCHAR('\0'); // 区切り文字をNULL終端に置き換える事で標準Cライブラリの関数で ColorName を読み取りやすくする
		const WCHAR* pColorNameInLine = &line[11];
		const WCHAR* pRegexKeywordInLine = &line[sepPos + 1]; // 区切り文字の後には RegexKeyword
		if( !*pRegexKeywordInLine || !CRegexKeyword::RegexKeyCheckSyntax(pRegexKeywordInLine) ){
			// L"不正なキーワードを無視しました。"
			sErrMsg = LS(STR_IMPEXP_REGEX3);
			continue;
		}
		//色指定名に対応する番号を探す
		//3文字カラー名からインデックス番号に変換
		int k = GetColorIndexByName( pColorNameInLine );	//@@@ 2002.04.30
		if( k == -1 ){
			/* 日本語名からインデックス番号に変換する */
			for(int m = 0; m < COLORIDX_LAST; m++){
				if( wcscmp(m_Types.m_ColorInfoArr[m].m_szName, pColorNameInLine) == 0 ){
					k = m;
					break;
				}
			}
		}
		if( k == -1 ){
			// 対応する番号が見つからない
			continue;
		}
		if( count >= MAX_REGEX_KEYWORD ){
			// L"キーワード数が上限に達したため切り捨てました。"
			sErrMsg = LS(STR_IMPEXP_REGEX1);
			break;
		}
		// 正規表現キーワード文字列の長さ
		const size_t regKeyLen = line.length() - (pRegexKeywordInLine - &line[0]);
		if( regKeyLen >= MAX_REGEX_KEYWORDLEN ){
			// L"キーワードが長過ぎるため切り捨てました。"
			sErrMsg = LS(STR_IMPEXP_REGEX4);
			continue;
		}

		if( keywordPos + regKeyLen + 1 >= MAX_REGEX_KEYWORDLISTLEN ){
			// L"キーワード領域がいっぱいなため切り捨てました。"
			sErrMsg = LS(STR_IMPEXP_REGEX2);
			continue;
		}
		std::copy(pRegexKeywordInLine, pRegexKeywordInLine + regKeyLen + 1, &pKeyword[keywordPos]);
		regexKeyArr[count].m_nColorIndex = k;
		count++;
		keywordPos += regKeyLen + 1;
	}
	pKeyword[keywordPos] = LCHAR('\0');	// 最後のNUL終端を書き込む
	in.Close();
	return true;
}

// エクスポート
bool CImpExpRegex::Export( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	CTextOutputStream out( sFileName.c_str() );
	if(!out){
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}

	out.WriteF( WSTR_REGEXKW_HEAD );

	const WCHAR* regex = m_Types.m_RegexKeywordList;
	for (int i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( regex[0] == LCHAR('\0') ) break;
		
		const WCHAR* name  = GetColorNameByIndex(m_Types.m_RegexKeywordArr[i].m_nColorIndex);
		out.WriteF( LTEXT("RxKey[%03d]=%s,%ls\n"), i, name, regex);

		for(; *regex != '\0'; regex++ ){}
		regex++;
	}

	out.Close();

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キーワードヘルプ                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*! インポート
	@date 2010.07.14 Moca ListViewへの設定からm_Typesへの設定に変更
*/
bool CImpExpKeyHelp::Import( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	WCHAR			msgBuff[_MAX_PATH + 1];

	CTextInputStream in( sFileName.c_str());
	if (!in) {
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}

	/* データ取得 */
	int invalid_record = 0; // 不正な行
	int i=0;
	while (in && i<MAX_KEYHELP_FILE) {
		sakura::wstring buff=in.ReadLineW();

		// 2007.02.03 genta コメントみたいな行は黙ってスキップ
		// 2007.10.08 kobake 空行もスキップ
		if( buff[0] == LCHAR('\0') ||
			buff[0] == LCHAR('\n') ||
			buff[0] == LCHAR('#') ||
			buff[0] == LCHAR(';') ||
			( buff[0] == LCHAR('/') && buff[1] == LCHAR('/') )){
				//	2007.02.03 genta 処理を継続
				continue;
		}

		//KDct[99]=ON/OFF,DictAbout,KeyHelpPath
		if( buff.length() < 10 ||
			wmemcmp(buff.c_str(), LTEXT("KDct["), 5) != 0 ||
			wmemcmp(&buff[7], LTEXT("]="), 2) != 0
			){
			//	2007.02.03 genta 処理を継続
			++invalid_record;
			continue;
		}

		auto p1 = &buff[9];
		auto p2 = wcschr(p1, LCHAR(','));
		auto p3 = p1;					//結果確認用に初期化
		if (p2) {
			*p2 = LCHAR('\0');
			p2 += 1;				//カンマの次が、次の要素
			if( nullptr != (p3=wcschr(p2,LCHAR(','))) ){
				*p3 = LCHAR('\0');
				p3 += 1;			//カンマの次が、次の要素
			}
		}/* 結果の確認 */
		if( (p3==nullptr) ||			//カンマが1個足りない
			(p3==p1) //||			//カンマが2個足りない
			//	2007.02.03 genta ファイル名にカンマがあるかもしれない
			//(NULL!=wcsstr(p3,","))	//カンマが多すぎる
		){
			//	2007.02.03 genta 処理を継続
			++invalid_record;
			continue;
		}
		/* valueのチェック */
		//ON/OFF
		//	2007.02.03 genta 1でなければ1にする
		unsigned int b_enable_flag = (unsigned int)_wtoi(p1);
		if( b_enable_flag > 1){
			b_enable_flag = 1;
		}
		//Path
		FILE* fp2;
		const WCHAR* p4 = p2;
		if( (fp2=_wfopen_absini(p3,LTEXT("r"))) == nullptr ){	// 2007.02.03 genta 相対パスはsakura.exe基準で開く	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
			// 2007.02.03 genta 辞書が見つからない場合の措置．警告を出すが取り込む
			p4 = LS(STR_IMPEXP_DIC_NOTFOUND);
			b_enable_flag = 0;
		}
		else
			fclose(fp2);

		//About
		// 2026.06.23 CWE-787 fix: the original '>' allowed wcslen(p2)==DICT_ABOUT_LEN, so the
		//            following copy wrote DICT_ABOUT_LEN+1 WCHAR into m_szAbout[DICT_ABOUT_LEN]
		//            (a 1-WCHAR overflow). Reject when the value cannot hold its NUL terminator.
		//  p3 was advanced past the '\0' placed at the second comma, so
		// (p3 - 1) - p2 gives the exact length of p2 without walking the string.
		if (DICT_ABOUT_LEN <= (p3 - 1) - p2) {
			auto_sprintf( msgBuff, LS(STR_IMPEXP_DIC_LENGTH), DICT_ABOUT_LEN - 1 );
			sErrMsg = msgBuff;
			++invalid_record;
			continue;
		}

		// 2026.06.23 CWE-787 fix: p3 (the dictionary path) is attacker-controlled and unbounded
		//            (it is the remainder of a line read by CTextInputStream::ReadLineW with no
		//            length limit). The original wcscpy into m_szPath (StaticString<_MAX_PATH>)
		//            therefore overflowed the type-settings object on a crafted import file.
		//            Reject over-long paths, consistent with the About-length handling above.
		//Path
		// p3 points into buff; end-p3 gives the remaining length without scanning the string.
		if (const auto end = std::data(buff) + std::size(buff);
			std::size(m_Types.m_KeyHelpArr[i].m_szPath) <= end - p3) {
			auto_sprintf( msgBuff, LS(STR_IMPEXP_DIC_LENGTH), int(m_Types.m_KeyHelpArr[i].m_szPath.size()) - 1 );
			sErrMsg = msgBuff;
			++invalid_record;
			continue;
		}

		//良さそうなら
		m_Types.m_KeyHelpArr[i].m_bUse = (b_enable_flag!=0);	// 2007.02.03 genta
		wcsncpy_s(m_Types.m_KeyHelpArr[i].m_szAbout, p4, _TRUNCATE);	// 2026.06.23 bounded copy (defense in depth)
		m_Types.m_KeyHelpArr[i].m_szPath = p3;				// 2026.06.23 StaticString::operator= copies with bounds + NUL
		i++;
	}
	in.Close();

	// 空きがあるなら番兵を設定
	if( i < int(std::size(m_Types.m_KeyHelpArr)) ){
		m_Types.m_KeyHelpArr[i].m_bUse = false;
		m_Types.m_KeyHelpArr[i].m_szAbout[0] = LCHAR('\0');
		m_Types.m_KeyHelpArr[i].m_szPath[0]  = LCHAR('\0');
	}
	m_Types.m_nKeyHelpNum = i;

	// 2007.02.03 genta 失敗したら警告する
	if( invalid_record > 0 ){
		auto_sprintf( msgBuff, LS(STR_IMPEXP_DIC_RECORD), invalid_record );
		sErrMsg = msgBuff;
	}

	return true;
}

/*! エクスポート
	@date 2010.07.14 Moca ListViewからm_Typesからのエクスポートに変更
*/
bool CImpExpKeyHelp::Export( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	CTextOutputStream out( sFileName.c_str() );
	if (!out) {
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}

	out.WriteF( WSTR_KEYHELP_HEAD );

	for (int i = 0; i < m_Types.m_nKeyHelpNum; i++) {
		out.WriteF(
			LTEXT("KDct[%02d]=%d,%s,%s\n"),
			i,
			m_Types.m_KeyHelpArr[i].m_bUse?1:0,
			m_Types.m_KeyHelpArr[i].m_szAbout.c_str(),
			m_Types.m_KeyHelpArr[i].m_szPath.c_str()
		);
	}
	out.Close();

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キー割り当て                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// インポート
bool CImpExpKeybind::Import( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	constexpr auto KEYNAME_SIZE = _countof(m_Common.m_sKeyBind.m_pKeyNameArr) - 1;// 最後の１要素はダミー用に予約 2012.11.25 aroka

	const auto& strPath = sFileName;
	CommonSetting_KeyBind sKeyBind = m_Common.m_sKeyBind;

	//オープン
	CDataProfile in;
	in.SetReadingMode();
	if (!in.ReadProfile( sFileName.c_str())) {
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}

	//バージョン確認
	bool	bVer4;			// 新バージョン（多言語対応）のファイル
	bool	bVer3;			// 新バージョンのファイル
	bool	bVer2;
	WCHAR szHeader[256];
	bVer4 = false;
	bVer3 = false;
	bVer2 = false;
	in.IOProfileData(szSecInfo, LTEXT("KEYBIND_VERSION"), StringBufferW(szHeader));
	if(wcscmp(szHeader,WSTR_KEYBIND_HEAD4)==0)	bVer4=true;
	else if(wcscmp(szHeader,WSTR_KEYBIND_HEAD3)==0)	bVer3=true;

	//int	nKeyNameArrNum;			// キー割り当て表の有効データ数
	if ( bVer3 || bVer4 ) {
		//Count取得 -> nKeyNameArrNum
		in.IOProfileData(szSecInfo, LTEXT("KEYBIND_COUNT"), sKeyBind.m_nKeyNameArrNum);
		if (sKeyBind.m_nKeyNameArrNum < 0 || sKeyBind.m_nKeyNameArrNum > KEYNAME_SIZE){	bVer3=false; bVer4=false; } //範囲チェック

		ShareData_IO_KeyBind(in, sKeyBind, true);	// 2008/5/25 Uchi
	}

	if (!bVer3 && !bVer4) {
		// 新バージョンでない
		CTextInputStream in2(strPath.c_str());
		if (!in2) {
			sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
			sErrMsg += sFileName;
			return false;
		}
		// ヘッダーチェック
		sakura::wstring	szLine = in2.ReadLineW();
		bVer2 = true;
		if ( wcscmp(szLine.c_str(), WSTR_KEYBIND_HEAD2) != 0)	bVer2 = false;
		// カウントチェック
		int	i, cnt;
		if ( bVer2 ) {
			int	an;
			szLine = in2.ReadLineW();
			cnt = swscanf(szLine.c_str(), LTEXT("Count=%d"), &an);
			if ( cnt != 1 || an < 0 || an > KEYNAME_SIZE ) {
				bVer2 = false;
			}
			else {
				sKeyBind.m_nKeyNameArrNum = an;
			}
		}
		if ( bVer2 ) {
			//各要素取得
			for(i = 0; i < KEYNAME_SIZE; i++) {
				int n, kc, nc;
				//値 -> szData
				WCHAR szData[1024];
				wcsncpy(szData, in2.ReadLineW().c_str(), int(std::size(szData)) - 1);
				szData[std::size(szData) - 1] = LCHAR('\0');

				//解析開始
				cnt = swscanf(szData, LTEXT("KeyBind[%03d]=%04x,%n"),
												&n,   &kc, &nc);
				if( cnt !=2 && cnt !=3 )	{ bVer2= false; break;}
				if( i != n ) break;
				sKeyBind.m_pKeyNameArr[i].m_nKeyCode = (short)kc;
				WCHAR* p = szData + nc;

				//後に続くトークン
				for(int j=0;j<8;j++)
				{
					WCHAR* q=wcschr(p,LCHAR(','));
					if(!q)	{ bVer2= false; break;}
					*q=LCHAR('\0');

					//機能名を数値に置き換える。(数値の機能名もあるかも)
					//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
					EFunctionCode n2 = CSMacroMgr::GetFuncInfoByName(G_AppInstance(), p, nullptr);
					if( n2 == F_INVALID )
					{
						if( WCODE::Is09(*p) )
						{
							n2 = (EFunctionCode)_wtol(p);
						}
						else
						{
							n2 = F_DEFAULT;
						}
					}
					sKeyBind.m_pKeyNameArr[i].m_nFuncCodeArr[j] = n2;
					p = q + 1;
				}

				wcsncpy(sKeyBind.m_pKeyNameArr[i].m_szKeyName, p, _countof(sKeyBind.m_pKeyNameArr[i].m_szKeyName)-1);
				sKeyBind.m_pKeyNameArr[i].m_szKeyName[_countof(sKeyBind.m_pKeyNameArr[i].m_szKeyName)-1] = '\0';
			}
		}
	}
	if (!bVer4 && !bVer3 && !bVer2) {
		sErrMsg = sakura::wstring(LS(STR_IMPEXP_KEY_FORMAT)) + sFileName;
		return false;
	}

	// データのコピー 	// マウスコードの固定と重複排除 2012.11.19 aroka
	//m_Common.m_sKeyBind.m_nKeyNameArrNum = nKeyNameArrNum;
	//memcpy_raw( m_Common.m_sKeyBind.m_pKeyNameArr, pKeyNameArr, sizeof_raw( pKeyNameArr ) );
	int nKeyNameArrUsed = m_Common.m_sKeyBind.m_nKeyNameArrNum; // 使用済み領域
	for( int j=sKeyBind.m_nKeyNameArrNum-1; j>=0; j-- ){
		if( (bVer2 || bVer3) && sKeyBind.m_pKeyNameArr[j].m_nKeyCode <= 0 ){ // マウスコードは先頭に固定されている KeyCodeが同じなのでKeyNameで判別
			for( int im=0; im< MOUSEFUNCTION_KEYBEGIN; im++ ){
				if( wcscmp( sKeyBind.m_pKeyNameArr[j].m_szKeyName, m_Common.m_sKeyBind.m_pKeyNameArr[im].m_szKeyName ) == 0 ){
					m_Common.m_sKeyBind.m_pKeyNameArr[im] = sKeyBind.m_pKeyNameArr[j];
				}
			}
		}
		else{
			// 割り当て済みキーコードは上書き
			int idx = sKeyBind.m_VKeyToKeyNameArr[sKeyBind.m_pKeyNameArr[j].m_nKeyCode];
			if( idx != KEYNAME_SIZE ){
				m_Common.m_sKeyBind.m_pKeyNameArr[idx] = sKeyBind.m_pKeyNameArr[j];
			}
		}
	}
	// 未割り当てのキーコードは空き領域が一杯になるまで追加
	for( int j2=0; j2<sKeyBind.m_nKeyNameArrNum; j2++ ){
		int idx = sKeyBind.m_VKeyToKeyNameArr[sKeyBind.m_pKeyNameArr[j2].m_nKeyCode];
		if( idx == KEYNAME_SIZE ){// not assigned
			if (nKeyNameArrUsed < 0 || KEYNAME_SIZE < nKeyNameArrUsed) continue;
			m_Common.m_sKeyBind.m_pKeyNameArr[nKeyNameArrUsed] = sKeyBind.m_pKeyNameArr[j2];
			sKeyBind.m_VKeyToKeyNameArr[sKeyBind.m_pKeyNameArr[j2].m_nKeyCode] = (BYTE)nKeyNameArrUsed++;
		}
	}
	m_Common.m_sKeyBind.m_nKeyNameArrNum = nKeyNameArrUsed;
	memcpy_raw( m_Common.m_sKeyBind.m_VKeyToKeyNameArr, sKeyBind.m_VKeyToKeyNameArr, sizeof_raw(sKeyBind.m_VKeyToKeyNameArr) );

	return true;
}

// エクスポート
bool CImpExpKeybind::Export( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	const auto& strPath = sFileName;

	CTextOutputStream out( strPath.c_str() );
	if (!out) {
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}

	out.Close();

	/* キー割り当て情報 */
	CDataProfile cProfile;

	// 書き込みモード設定
	cProfile.SetWritingMode();

	// ヘッダー
	StaticString<256> szKeydataHead{ WSTR_KEYBIND_HEAD4 };
	cProfile.IOProfileData( szSecInfo, LTEXT("KEYBIND_VERSION"), szKeydataHead );
	cProfile.IOProfileData(szSecInfo, LTEXT("KEYBIND_COUNT"), m_Common.m_sKeyBind.m_nKeyNameArrNum );

	//内容
	ShareData_IO_KeyBind(cProfile, m_Common.m_sKeyBind, true);

	// 書き込み
	if (!cProfile.WriteProfile( strPath.c_str(), WSTR_KEYBIND_HEAD4)) {
		sErrMsg = LS(STR_IMPEXP_ERR_EXPORT);
		sErrMsg += sFileName;
		return false;
	}

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     カスタムメニュー                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// インポート
bool CImpExpCustMenu::Import( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	const auto& strPath = sFileName;

	//ヘッダー確認
	CTextInputStream in(strPath.c_str());
	if (!in) {
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}

	CDataProfile cProfile;
	cProfile.SetReadingMode();
	cProfile.ReadProfile(strPath.c_str());

	//バージョン確認
	WCHAR szHeader[256];
	cProfile.IOProfileData(szSecInfo, LTEXT("MENU_VERSION"), StringBufferW(szHeader));
	if(wcscmp(szHeader, WSTR_CUSTMENU_HEAD_V2)!=0) {
		sErrMsg = sakura::wstring(LS(STR_IMPEXP_CUSTMENU_FORMAT)) + sFileName;
		return false;
	}

	CShareData_IO::IO_CustMenu(cProfile,m_Common.m_sCustomMenu, true);			// 2008/5/24 Uchi

	return true;
}

// エクスポート
bool CImpExpCustMenu::Export( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	const auto& strPath = sFileName;

	// オープン
	CTextOutputStream out(strPath.c_str());
	if (!out) {
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}

	out.Close();

	/* カスタムメニュー情報 */
	//ヘッダー
	CDataProfile	cProfile;
	CommonSetting_CustomMenu* menu=&m_Common.m_sCustomMenu;

	// 書き込みモード設定
	cProfile.SetWritingMode();

	//ヘッダー
	cProfile.IOProfileData(szSecInfo, LTEXT("MENU_VERSION"), StringBufferW(WSTR_CUSTMENU_HEAD_V2));
	int iWork = MAX_CUSTOM_MENU;
	cProfile.IOProfileData(szSecInfo, LTEXT("MAX_CUSTOM_MENU"), iWork );
	
	//内容
	CShareData_IO::IO_CustMenu(cProfile, *menu, true);

	// 書き込み
	if (!cProfile.WriteProfile( strPath.c_str(), WSTR_CUSTMENU_HEAD_V2)) {
		sErrMsg = LS(STR_IMPEXP_ERR_EXPORT);
		sErrMsg += sFileName;
		return false;
	}

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     強調キーワード                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// インポート
bool CImpExpKeyWord::Import( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	bool			bAddError = false;

	CTextInputStream in(sFileName.c_str());
	if (!in) {
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}
	while( in ){
		sakura::wstring szLine = in.ReadLineW();

		// コメント無視
		if (szLine.length() == 0) {
			continue;
		}
		if (2 <= szLine.length() && 0 == wmemcmp( szLine.c_str(), LTEXT("//"), 2 )) {
			if (szLine == WSTR_CASE_TRUE) {
				m_bCase = true;
			}
			else if (szLine == WSTR_CASE_FALSE) {
				m_bCase = false;
			}
			continue;
		}
		
		//解析
		if( 0 < szLine.length() ){
			/* ｎ番目のセットにキーワードを追加 */
			int nRetValue = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWord( m_nIdx, szLine.c_str() );
			if( 2 == nRetValue ){
				bAddError = true;
				break;
			}
		}
	}
	in.Close();

	// 大文字小文字区別
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetKeyWordCase( m_nIdx, m_bCase );

	if (bAddError) {
		sErrMsg = LS(STR_IMPEXP_KEYWORD);
	}

	return true;
}

// エクスポート
bool CImpExpKeyWord::Export( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	int 		nKeyWordNum;
	int 		i;

	CTextOutputStream out(sFileName.c_str());
	if (!out) {
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}
	out.WriteF( LTEXT("// ") );
	// 2012.03.10 syat キーワードに「%」を含む場合にエクスポート結果が不正
	out.WriteString( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( m_nIdx ) );
	out.WriteF( WSTR_KEYWORD_HEAD );

	out.WriteF( WSTR_KEYWORD_CASE );
	out.WriteF( m_bCase ? LTEXT("True") : LTEXT("False") );
	out.WriteF( LTEXT("\n\n") );

	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SortKeyWord(m_nIdx);	//MIK 2000.12.01 sort keyword

	/* ｎ番目のセットのキーワードの数を返す */
	nKeyWordNum = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordNum( m_nIdx );
	for( i = 0; i < nKeyWordNum; ++i ){
		/* ｎ番目のセットのｍ番目のキーワードを返す */
		// 2012.03.10 syat キーワードに「%」を含む場合にエクスポート結果が不正
		out.WriteString( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( m_nIdx, i ) );
		out.WriteF( LTEXT("\n") );
	}
	out.Close();

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     メインメニュー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//	2010/5/23 Uchi
// インポート
bool CImpExpMainMenu::Import( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	const auto& strPath = sFileName;

	//ヘッダー確認
	CTextInputStream in(strPath.c_str());
	if (!in) {
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}

	CDataProfile cProfile;
	cProfile.SetReadingMode();
	cProfile.ReadProfile( strPath.c_str() );

	//バージョン確認
	WCHAR szHeader[256];
	cProfile.IOProfileData(szSecInfo, LTEXT("MENU_VERSION"), StringBufferW(szHeader));
	if(wcscmp(szHeader, WSTR_MAINMENU_HEAD_V1)!=0) {
		sErrMsg = sakura::wstring(LS(STR_IMPEXP_MEINMENU)) + sFileName;
		return false;
	}

	CShareData_IO::IO_MainMenu(cProfile,m_Common.m_sMainMenu, true);

	return true;
}

// エクスポート
bool CImpExpMainMenu::Export( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	const auto& strPath = sFileName;

	// オープン
	CTextOutputStream out( strPath.c_str() );
	if (!out) {
		sErrMsg = LS(STR_IMPEXP_ERR_FILEOPEN);
		sErrMsg += sFileName;
		return false;
	}

	out.Close();

	//ヘッダー
	CDataProfile	cProfile;
	CommonSetting_MainMenu* menu=&m_Common.m_sMainMenu;

	// 書き込みモード設定
	cProfile.SetWritingMode();

	//ヘッダー
	cProfile.IOProfileData(szSecInfo, LTEXT("MENU_VERSION"), StringBufferW(WSTR_MAINMENU_HEAD_V1));
	
	//内容
	CShareData_IO::IO_MainMenu(cProfile, *menu, true);

	// 書き込み
	if (!cProfile.WriteProfile( strPath.c_str(), WSTR_MAINMENU_HEAD_V1)) {
		sErrMsg = LS(STR_IMPEXP_ERR_EXPORT);
		sErrMsg += sFileName;
		return false;
	}

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ファイルツリー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//	2014.06.07 Moca
// インポート
bool CImpExpFileTree::Import( [[maybe_unused]] const sakura::wstring& sFileName, [[maybe_unused]] sakura::wstring& sErrMsg )
{
	const auto& strPath = sFileName;

	CDataProfile cProfile;
	cProfile.SetReadingMode();
	cProfile.ReadProfile( strPath.c_str() );

	IO_FileTreeIni( cProfile, m_aFileTreeItems );

	return true;
}

// エクスポート
bool CImpExpFileTree::Export( const sakura::wstring& sFileName, sakura::wstring& sErrMsg )
{
	const auto& strPath = sFileName;

	CDataProfile	cProfile;

	// 書き込みモード設定
	cProfile.SetWritingMode();

	IO_FileTreeIni( cProfile, m_aFileTreeItems );

	// 書き込み
	if (!cProfile.WriteProfile( strPath.c_str(), WSTR_FILETREE_HEAD_V1)) {
		sErrMsg = LS(STR_IMPEXP_ERR_EXPORT);
		sErrMsg += sFileName;
		return false;
	}

	return true;
}

void CImpExpFileTree::IO_FileTreeIni( CDataProfile& cProfile, std::vector<SFileTreeItem>& data )
{
	const WCHAR* pszSecName = LTEXT("FileTree");

	int nItemCount = (int)data.size();
	cProfile.IOProfileData( pszSecName, LTEXT("nFileTreeItemCount"), nItemCount );
	if( nItemCount < 0 ){
		nItemCount = 0;
	}

	if( cProfile.IsReadingMode() ){
		data.resize( nItemCount );
	}

	for (int i = 0; i < nItemCount; ++i) {
		cProfile.IOProfileData(pszSecName, sakura::format(LTEXT("FileTree({})"), i), data[i]);
	}
}
