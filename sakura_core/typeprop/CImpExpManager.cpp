/*!	@file
	@brief インポート、エクスポートマネージャ

	@author Uchi
	@date 2010/4/22 新規作成
*/
/*
	Copyright (C) 2010, Uchi, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "CImpExpManager.h"
#include "typeprop/CDlgTypeAscertain.h"

#include "dlg/CDlgOpenFile.h"
#include "io/CTextStream.h"
#include "env/CShareData_IO.h"
#include "plugin/CPlugin.h"
#include "view/CEditView.h"
#include "view/colors/CColorStrategy.h"
#include "util/other_util.h"

/*-----------------------------------------------------------------------
定数
-----------------------------------------------------------------------*/
static const wchar_t	szSecInfo[]				= L"Info";

// タイプ別設定
static const wchar_t	WSTR_TYPE_HEAD[]		= L" タイプ別設定 Ver1";

static const wchar_t	szSecTypeEx[]			= L"TypeEx";
static const wchar_t	szSecTypes[]			= L"Types";

static const wchar_t	szKeyKeywordTemp[]				= L"szKeyword[%d]";
static const wchar_t	szKeyKeywordFileTemp[]			= L"szKeywordFile[%d]";
static const wchar_t	szKeyKeywordCaseTemp[]			= L"szKeywordCase[%d]";
static const wchar_t	szKeyPluginOutlineName[]		= L"szPluginOutlineName";
static const wchar_t	szKeyPluginOutlineId[]			= L"szPluginOutlineId";
static const wchar_t	szKeyPluginSmartIndentName[]	= L"szPluginSmartIndentName";
static const wchar_t	szKeyPluginSmartIndentId[]		= L"szPluginSmartIndentId";
static const wchar_t	szKeyVersion[]					= L"szVersion";
static const wchar_t	szKeyStructureVersion[]			= L"vStructureVersion";

// カラー（ CPropTypes.hからコピー改変 ）
//static const wchar_t	WSTR_COLORDATA_HEAD2[]	=  L" テキストエディタ色設定 Ver2";
//static const wchar_t	WSTR_COLORDATA_HEAD21[]	=  L" テキストエディタ色設定 Ver2.1";	//Nov. 2, 2000 JEPRO 変更 [注]. 0.3.9.0:ur3β10以降、設定項目の番号を入れ替えたため
static const wchar_t	WSTR_COLORDATA_HEAD3[]	=  L" テキストエディタ色設定 Ver3";		//Jan. 15, 2001 Stonee  色設定Ver3ドラフト(設定ファイルのキーを連番→文字列に)	//Feb. 11, 2001 JEPRO 有効にした
//static const wchar_t	WSTR_COLORDATA_HEAD4[]	=  L" テキストエディタ色設定 Ver4";		//2007.10.02 kobake UNICODE化に際してカラーファイル仕様も変更
static const wchar_t	szSecColor[]			=  L"SakuraColor";

// 正規表現キーワード
static const wchar_t	WSTR_REGEXKW_HEAD[]		= L"// 正規表現キーワード Ver1\n";

// キーワードヘルプ
static const wchar_t	WSTR_KEYHELP_HEAD[]		= L"// キーワード辞書設定 Ver1\n";

// キー割り当て
static const wchar_t	WSTR_KEYBIND_HEAD4[]		= L"SakuraKeyBind_Ver4";	//2013.12.05 syat 多言語対応
static const wchar_t	WSTR_KEYBIND_HEAD3[]	= L"SakuraKeyBind_Ver3";	//2007.10.05 kobake ファイル形式をini形式に変更
static const wchar_t	WSTR_KEYBIND_HEAD2[]	= L"// テキストエディタキー設定 Ver2";	// (旧バージョン(ANSI版)） 読み込みのみ対応 2008/5/3 by Uchi

// カスタムメニューファイル
// 2007.10.02 kobake UNICODE化に際して、カスタムメニューファイルの仕様を変更
static       wchar_t	WSTR_CUSTMENU_HEAD_V2[]	= L"SakuraEditorMenu_Ver2";

// キーワード定義ファイル
static const wchar_t	WSTR_KEYWORD_HEAD[]		= L" キーワード定義ファイル\n";
static const wchar_t	WSTR_KEYWORD_CASE[]		= L"// CASE=";
static const wchar_t	WSTR_CASE_TRUE[]		= L"// CASE=True";
static const wchar_t	WSTR_CASE_FALSE[]		= L"// CASE=False";

// メインメニューファイル
static       wchar_t	WSTR_MAINMENU_HEAD_V1[]	= L"SakuraEditorMainMenu Ver1";

static       wchar_t	WSTR_FILETREE_HEAD_V1[]	= L"SakuraEditorFileTree_Ver1";

// Exportファイル名の作成
//	  タイプ名などファイルとして扱うことを考えていない文字列を扱う
//		2010/4/12 Uchi
static wchar_t* MakeExportFileName(wchar_t* res, const wchar_t* trg, const wchar_t* ext)
{
	wchar_t		conv[_MAX_PATH+1];
	wchar_t*	p;

	auto_strcpy( conv, trg );

	p = conv;
	while ( (p = wcspbrk( p, L"\t\\:*?\"<>|" )) != NULL ) {
		// ファイル名に使えない文字を _ に置き換える
		*p++ = L'_';
	}
	p = conv;
	while ( (p = wcspbrk( p, L"/" )) != NULL ) {
		// ファイル名に使えない文字を ／ に置き換える
		*p++ = L'／';
	}
	auto_sprintf_s(res, _MAX_PATH, L"%ls.%ls", conv, ext);

	return res;
}

// インポート ファイル指定付き
bool CImpExpManager::ImportUI( HINSTANCE hInstance, HWND hwndParent )
{
	/* ファイルオープンダイアログの初期化 */
	CDlgOpenFile	cDlgOpenFile;
	cDlgOpenFile.Create(
		hInstance,
		hwndParent,
		GetDefaultExtension(),
		GetDllShareData().m_sHistory.m_szIMPORTFOLDER // インポート用フォルダ
	);
	TCHAR	szPath[_MAX_PATH + 1];
	szPath[0] = _T('\0');
	if( !GetFileName().empty() ){
		auto_strcpy( szPath, to_tchar( GetFullPath().c_str() ));
	}
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return false;
	}

	const wstring	sPath = to_wchar(szPath);
	wstring	sErrMsg;

	// 確認
	if (!ImportAscertain( hInstance, hwndParent, sPath, sErrMsg )) {
		if (sErrMsg.length() > 0) {
			ErrorMessage( hwndParent, _T("%ls"), sErrMsg.c_str() );
		}
		return false;
	}

	// Import Folderの設定
	SetImportFolder( szPath );

	// Import
	if (!Import( sPath, sErrMsg )) {
		ErrorMessage( hwndParent, _T("%ls"), sErrMsg.c_str() );
		return false;
	}

	if (sErrMsg.length() > 0) {
		InfoMessage( hwndParent, _T("%ls"), sErrMsg.c_str() );
	}

	return true;
}

// エクスポート ファイル指定付き
bool CImpExpManager::ExportUI( HINSTANCE hInstance, HWND hwndParent )
{
	/* ファイルオープンダイアログの初期化 */
	CDlgOpenFile	cDlgOpenFile;
	cDlgOpenFile.Create(
		hInstance,
		hwndParent,
		GetDefaultExtension(),
		GetDllShareData().m_sHistory.m_szIMPORTFOLDER // インポート用フォルダ
	);
	TCHAR			szPath[_MAX_PATH + 1];
	szPath[0] = _T('\0');
	if( !GetFileName().empty() ){
		auto_strcpy( szPath, to_tchar( GetFullPath().c_str() ));
	}
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return false;
	}

	// Import Folderの設定
	SetImportFolder( szPath );

	// Export
	const wstring	sPath = to_wchar(szPath);
	wstring	sErrMsg;

	if (!Export( sPath, sErrMsg )) {
		ErrorMessage( hwndParent, _T("%ls"), sErrMsg.c_str() );
		return false;
	}

	if (sErrMsg.length() == 0) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_OK_EXPORT)) + to_wchar( szPath );
	}
	InfoMessage( hwndParent, _T("%ls"), sErrMsg.c_str() );

	return true;
}

// インポート確認
bool CImpExpManager::ImportAscertain( HINSTANCE hInstance, HWND hwndParent, const wstring& sFileName, wstring& sErrMsg )
{
	return true;
}

// デフォルト拡張子の取得
const TCHAR* CImpExpManager::GetDefaultExtension()
{
	return _T("");
}
const wchar_t* CImpExpManager::GetOriginExtension()
{
	return L"";
}

// ファイル名の初期値を設定
void CImpExpManager::SetBaseName(const wstring& sBase)
{
	wchar_t		wbuff[_MAX_PATH + 1];

	m_sBase = sBase;
	m_sOriginName = MakeExportFileName( wbuff, sBase.c_str(), GetOriginExtension() );
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          タイプ別設定                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// インポート確認
bool CImpExpType::ImportAscertain( HINSTANCE hInstance, HWND hwndParent, const wstring& sFileName, wstring& sErrMsg )
{
	const tstring	sPath = to_tchar( sFileName.c_str() );

	m_cProfile.SetReadingMode();

	if (!m_cProfile.ReadProfile( sPath.c_str() )) {
		/* 設定ファイルが存在しない */
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	// Check Version
	int		nStructureVersion = 0;
	wchar_t	szKeyVersion[64];
	if (!m_cProfile.IOProfileData( szSecInfo, szKeyStructureVersion, nStructureVersion )) {
		sErrMsg = LSW(STR_IMPEXP_ERR_TYPE);
		return false;
	}
	if ((unsigned int)nStructureVersion != m_pShareData->m_vStructureVersion) {
		auto_strcpy( szKeyVersion, L"?" );
		m_cProfile.IOProfileData( szSecInfo, szKeyVersion, MakeStringBufferW( szKeyVersion ) );
		int nRet = ConfirmMessage( hwndParent,
			LS(STR_IMPEXP_VER), 
			_APP_NAME_(LTEXT), szKeyVersion, nStructureVersion );
		if ( IDYES != nRet ) {
			return false;
		}
	}

	// 確認＆色指定
	CDlgTypeAscertain::SAscertainInfo	sAscertainInfo;
	CDlgTypeAscertain	cDlgTypeAscertain;
	wchar_t wszLabel[1024];
	STypeConfig TmpType;

	// パラメータの設定
	sAscertainInfo.sImportFile = sPath;
	List_GetText( m_hwndList, m_nIdx, wszLabel );
	sAscertainInfo.sTypeNameTo = wszLabel;
	wszLabel[0] = L'\0';
	m_cProfile.IOProfileData( szSecTypes, L"szTypeName", MakeStringBufferW( wszLabel ));
	sAscertainInfo.sTypeNameFile = wszLabel;

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
bool CImpExpType::Import( const wstring& sFileName, wstring& sErrMsg )
{
	wstring	files = L"";
	wstring TmpMsg;
	ColorInfo	colorInfoArr[_countof(m_Types.m_ColorInfoArr)];				// 色設定配列(バックアップ)
	int		i;

	// 色の変更
	if (m_nColorType >= MAX_TYPES) {
		// 色設定インポート
		CImpExpColors	cImpExpColors( colorInfoArr );
		if (cImpExpColors.Import( cImpExpColors.MakeFullPath( m_sColorFile ), TmpMsg)) {
			files += wstring(L"\n") + m_sColorFile;
		}
		else {
			// 失敗したら基本をコピー(メッセージは出さない)
			memcpy( &colorInfoArr, GetDllShareData().m_TypeBasis.m_ColorInfoArr, sizeof(colorInfoArr) );
			files += wstring(L"\n× ") + m_sColorFile;	// 失敗
		}
	}
	else if (m_nColorType >= 0 ) {
		// 色指定(内部)
		STypeConfig type;
		CDocTypeManager().GetTypeConfig(CTypeConfig(m_nColorType), type);
		memcpy( &colorInfoArr, type.m_ColorInfoArr, sizeof(colorInfoArr) );
	}

	// 読み込み
	CShareData_IO::ShareData_IO_Type_One( m_cProfile, m_Types, szSecTypes );

	m_nIdx = m_Types.m_nIdx;
	if (m_nIdx == 0) {
		// 基本の場合の名前と拡張子を初期化
		_tcscpy( m_Types.m_szTypeName, LS(STR_TYPE_NAME_BASIS) );
		m_Types.m_szTypeExts[0] = _T('\0');
		m_Types.m_id = 0;
	}else{
		// 基本じゃなかった場合、id番号をランダムに仮採番(あとで振りなおす)
		m_Types.m_id = (::GetTickCount() & 0x3fffffff) + m_nIdx * 0x10000;
	}

	// 色の設定
	if (m_nColorType >= 0 ) {
		// 色指定あり
		for (i = 0; i < _countof(colorInfoArr); i++) {
			bool bDisp = m_Types.m_ColorInfoArr[i].m_bDisp;
			m_Types.m_ColorInfoArr[i] = colorInfoArr[i];
			m_Types.m_ColorInfoArr[i].m_bDisp = bDisp;		// 表示フラグはファイルのものを使用する
		}
	}

	// 共通設定との連結部
	wchar_t	szKeyName[64];
	wchar_t	szKeyData[1024];
	int		nIdx;
	int		nPlug = 0;
	int		nDataLen;
	wchar_t* pSlashPos;
	wchar_t	szFileName[_MAX_PATH+1];
	bool	bCase;
	wstring	sErrMag;
	CommonSetting& common = m_pShareData->m_Common;

	// 強調キーワード
	CKeyWordSetMgr&	cKeyWordSetMgr = common.m_sSpecialKeyword.m_CKeyWordSetMgr;
	for (i=0; i < MAX_KEYWORDSET_PER_TYPE; i++) {
		//types.m_nKeyWordSetIdx[i] = -1;
		auto_sprintf( szKeyName, szKeyKeywordTemp, i+1 );
		if (m_cProfile.IOProfileData( szSecTypeEx, szKeyName, MakeStringBufferW( szKeyData ))) {
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
				szFileName[0] = L'\0';
				if (m_cProfile.IOProfileData( szSecTypeEx, szKeyName, MakeStringBufferW( szFileName ))) {
					if( cImpExpKeyWord.Import( cImpExpKeyWord.MakeFullPath( szFileName ), TmpMsg )) {
						files += wstring(L"\n") + szFileName;
					} else {
						files += wstring(L"\n× ") + szFileName;	// 失敗
					}
				}
			}
			m_Types.m_nKeyWordSetIdx[i] = nIdx;
		}
	}

	// Plugin
	//  アウトライン解析方法
	CommonSetting_Plugin& plugin = common.m_sPlugin;
	if (m_cProfile.IOProfileData( szSecTypeEx, szKeyPluginOutlineId, MakeStringBufferW( szKeyData ))) {
		nDataLen = wcslen( szKeyData );
		pSlashPos = wcschr( szKeyData, L'/' );
		nIdx = -1;
		for (i = 0; i < MAX_PLUGIN; i++) {
			if (auto_strncmp(szKeyData, plugin.m_PluginTable[i].m_szId, pSlashPos ? pSlashPos-szKeyData : nDataLen) == 0) {
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
	if (m_cProfile.IOProfileData( szSecTypeEx, szKeyPluginSmartIndentId, MakeStringBufferW( szKeyData ))) {
		nDataLen = wcslen( szKeyData );
		pSlashPos = wcschr( szKeyData, L'/' );
		nIdx = -1;
		for (i = 0; i < MAX_PLUGIN; i++) {
			if (auto_strncmp(szKeyData, plugin.m_PluginTable[i].m_szId, pSlashPos ? pSlashPos-szKeyData : nDataLen) == 0) {
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

	sErrMsg = std::wstring(LSW(STR_IMPEXP_OK_IMPORT)) + sFileName + files;

	return true;
}


// エクスポート
bool CImpExpType::Export( const wstring& sFileName, wstring& sErrMsg )
{
	CDataProfile	cProfile;

	cProfile.SetWritingMode();

	CShareData_IO::ShareData_IO_Type_One( cProfile , m_Types, szSecTypes );

	// 共通設定との連結部
	int		i;
	wchar_t	szKeyName[64];
	wchar_t buff[MAX_SETNAMELEN+1];
	wchar_t	szFileName[_MAX_PATH+1];
	bool	bCase;
	wstring	files = L"";
	wstring	sTmpMsg;
	int		nIdx;
	CommonSetting& common  = m_pShareData->m_Common;

	// 強調キーワード
	CKeyWordSetMgr&	cKeyWordSetMgr = common.m_sSpecialKeyword.m_CKeyWordSetMgr;
	for (i=0; i < MAX_KEYWORDSET_PER_TYPE; i++) {
		if (m_Types.m_nKeyWordSetIdx[i] >= 0) {
			nIdx = m_Types.m_nKeyWordSetIdx[i];
			auto_sprintf( szKeyName, szKeyKeywordTemp, i+1 );
			auto_strcpy( buff, cKeyWordSetMgr.GetTypeName( nIdx ));
			cProfile.IOProfileData( szSecTypeEx, szKeyName, MakeStringBufferW( buff ));

			// 大文字小文字区別
			bCase = common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordCase( nIdx );

			// キーワード定義ファイル出力
			CImpExpKeyWord	cImpExpKeyWord( common, m_Types.m_nKeyWordSetIdx[i], bCase );
			cImpExpKeyWord.SetBaseName( common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( nIdx ));

			if ( cImpExpKeyWord.Export( cImpExpKeyWord.GetFullPath(), sTmpMsg ) ) {
				auto_strcpy( szFileName, cImpExpKeyWord.GetFileName().c_str());
				auto_sprintf( szKeyName, szKeyKeywordFileTemp, i+1 );
				if (cProfile.IOProfileData( szSecTypeEx, szKeyName, MakeStringBufferW( szFileName ))) {
					files += wstring( L"\n" ) + cImpExpKeyWord.GetFileName();
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
	wchar_t szId[ MAX_PLUGIN_ID + 1 + 2 ];
	if ((nPIdx = CPlug::GetPluginId( static_cast<EFunctionCode>( m_Types.m_eDefaultOutline ))) >= 0) {
		cProfile.IOProfileData( szSecTypeEx, szKeyPluginOutlineName, MakeStringBufferW(plugin.m_PluginTable[nPIdx].m_szName));
		wcscpyn( szId, plugin.m_PluginTable[nPIdx].m_szId, _countof(szId) );
		if( (nPlug = CPlug::GetPlugId( static_cast<EFunctionCode>( m_Types.m_eDefaultOutline ))) != 0 ){
			wchar_t szPlug[8];
			swprintf( szPlug, L"/%d", nPlug );
			wcscat( szId, szPlug );
		}
		cProfile.IOProfileData( szSecTypeEx, szKeyPluginOutlineId,   MakeStringBufferW(szId) );
	}
	//  スマートインデント
	if ((nPIdx = CPlug::GetPluginId( static_cast<EFunctionCode>( m_Types.m_eSmartIndent ))) >= 0) {
		cProfile.IOProfileData( szSecTypeEx, szKeyPluginSmartIndentName, MakeStringBufferW(plugin.m_PluginTable[nPIdx].m_szName));
		wcscpyn( szId, plugin.m_PluginTable[nPIdx].m_szId, _countof(szId) );
		if( (nPlug = CPlug::GetPlugId( static_cast<EFunctionCode>( m_Types.m_eSmartIndent ))) != 0 ){
			wchar_t szPlug[8];
			swprintf( szPlug, L"/%d", nPlug );
			wcscat( szId, szPlug );
		}
		cProfile.IOProfileData( szSecTypeEx, szKeyPluginSmartIndentId,   MakeStringBufferW(szId) );
	}

	// Version
	DLLSHAREDATA* pShare = &GetDllShareData();
	int		nStructureVersion;
	wchar_t	wbuff[_MAX_PATH + 1];
	auto_sprintf( wbuff, L"%d.%d.%d.%d", 
				HIWORD( pShare->m_sVersion.m_dwProductVersionMS ),
				LOWORD( pShare->m_sVersion.m_dwProductVersionMS ),
				HIWORD( pShare->m_sVersion.m_dwProductVersionLS ),
				LOWORD( pShare->m_sVersion.m_dwProductVersionLS ) );
	cProfile.IOProfileData( szSecInfo, szKeyVersion, MakeStringBufferW(wbuff) );
	nStructureVersion = int(pShare->m_vStructureVersion);
	cProfile.IOProfileData( szSecInfo, szKeyStructureVersion, nStructureVersion );

	// 書き込み
	if (!cProfile.WriteProfile( to_tchar(sFileName.c_str()), WSTR_TYPE_HEAD )) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_EXPORT)) + sFileName;
		return false;
	}

	sErrMsg =  std::wstring(LSW(STR_IMPEXP_OK_EXPORT)) + sFileName + files;

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          カラー                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// インポート
bool CImpExpColors::Import( const wstring& sFileName, wstring& sErrMsg )
{
	const tstring strPath = to_tchar( sFileName.c_str() );

	// 開けるか
	CTextInputStream in( strPath.c_str() );
	if (!in) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	/* ファイル先頭 */
	//ヘッダ読取
	wstring szHeader = in.ReadLineW();
	if(szHeader.length()>=2) {
		//コメントを抜く
		szHeader = &szHeader.c_str()[ szHeader.c_str()[0] == _T(';') ? 1 : 2];
	}
	//比較
	if (szHeader != WSTR_COLORDATA_HEAD3) {
		in.Close();
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_COLOR_OLD))	// 旧バージョンの説明の削除 2010/4/22 Uchi
			+ sFileName;
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
	CShareData_IO::IO_ColorSet( &cProfile, szSecColor, m_ColorInfoArr );

	return true;
}

// エクスポート
bool CImpExpColors::Export( const wstring& sFileName, wstring& sErrMsg )
{
	/* 色設定 I/O */
	CDataProfile	cProfile;
	cProfile.SetWritingMode();
	CShareData_IO::IO_ColorSet( &cProfile, szSecColor, m_ColorInfoArr );
	if (!cProfile.WriteProfile( to_tchar( sFileName.c_str() ), WSTR_COLORDATA_HEAD3 )) { //Jan. 15, 2001 Stonee
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_EXPORT)) + sFileName;
		return false;
	}

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    正規表現キーワード                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// インポート
bool CImpExpRegex::Import( const wstring& sFileName, wstring& sErrMsg )
{
	CTextInputStream	in( to_tchar( sFileName.c_str() ) );
	if (!in) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	RegexKeywordInfo	regexKeyArr[MAX_REGEX_KEYWORD];
	auto_array_ptr<wchar_t> szKeyWordList(new wchar_t [ MAX_REGEX_KEYWORDLISTLEN ]);
	wchar_t*	pKeyword = &szKeyWordList[0];
	int	keywordPos = 0;
	TCHAR				buff[MAX_REGEX_KEYWORDLEN + 20];
	int count = 0;
	while(in)
	{
		//1行読み込み
		wstring line=in.ReadLineW();
		_wcstotcs(buff,line.c_str(),_countof(buff));

		if(count >= MAX_REGEX_KEYWORD){
			sErrMsg = LSW(STR_IMPEXP_REGEX1);
			break;
		}

		//RxKey[999]=ColorName,RegexKeyword
		if( auto_strlen(buff) < 12 ) continue;
		if( auto_memcmp(buff, _T("RxKey["), 6) != 0 ) continue;
		if( auto_memcmp(&buff[9], _T("]="), 2) != 0 ) continue;
		TCHAR	*p;
		p = auto_strstr(&buff[11], _T(","));
		if( p )
		{
			*p = _T('\0');
			p++;
			if( p[0] && CRegexKeyword::RegexKeyCheckSyntax(to_wchar(p)) )	//囲みがある
			{
				//色指定名に対応する番号を探す
				int k = GetColorIndexByName( &buff[11] );	//@@@ 2002.04.30
				if( k == -1 ){
					/* 日本語名からインデックス番号に変換する */
					for(int m = 0; m < COLORIDX_LAST; m++){
						if( auto_strcmp(m_Types.m_ColorInfoArr[m].m_szName, &buff[11]) == 0 ){
							k = m;
							break;
						}
					}
				}
				if( k != -1 )	/* 3文字カラー名からインデックス番号に変換 */
				{
					if( 0 < MAX_REGEX_KEYWORDLISTLEN - keywordPos - 1 ){
						regexKeyArr[count].m_nColorIndex = k;
						_tcstowcs(&pKeyword[keywordPos], p, t_min<int>(MAX_REGEX_KEYWORDLEN, MAX_REGEX_KEYWORDLISTLEN - keywordPos - 1));
						count++;
						keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;
					}else{
						sErrMsg = LSW(STR_IMPEXP_REGEX2);
					}
				}
			}else{
				sErrMsg = LSW(STR_IMPEXP_REGEX3);
			}
		}
	}
	pKeyword[keywordPos] = L'\0';

	in.Close();

	
	for(int i = 0; i < count; i++ ){
		m_Types.m_RegexKeywordArr[i] = regexKeyArr[i];
	}
	for( int i = 0; i <= keywordPos; i++ ){
		m_Types.m_RegexKeywordList[i] = pKeyword[i];
	}

	return true;
}

// エクスポート
bool CImpExpRegex::Export( const wstring& sFileName, wstring& sErrMsg )
{
	CTextOutputStream out( to_tchar( sFileName.c_str() ) );
	if(!out){
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	out.WriteF( WSTR_REGEXKW_HEAD );

	const wchar_t* regex = m_Types.m_RegexKeywordList;
	for (int i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( regex[0] == L'\0' ) break;
		
		const TCHAR* name  = GetColorNameByIndex(m_Types.m_RegexKeywordArr[i].m_nColorIndex);
		out.WriteF( L"RxKey[%03d]=%ts,%ls\n", i, name, regex);

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
bool CImpExpKeyHelp::Import( const wstring& sFileName, wstring& sErrMsg )
{
	wchar_t			msgBuff[_MAX_PATH + 1];

	CTextInputStream in( to_tchar( sFileName.c_str() ));
	if (!in) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	/* データ取得 */
	int invalid_record = 0; // 不正な行
	int i=0;
	while (in && i<MAX_KEYHELP_FILE) {
		wstring buff=in.ReadLineW();

		// 2007.02.03 genta コメントみたいな行は黙ってスキップ
		// 2007.10.08 kobake 空行もスキップ
		if( buff[0] == LTEXT('\0') ||
			buff[0] == LTEXT('\n') ||
			buff[0] == LTEXT('#') ||
			buff[0] == LTEXT(';') ||
			( buff[0] == LTEXT('/') && buff[1] == LTEXT('/') )){
				//	2007.02.03 genta 処理を継続
				continue;
		}

		//KDct[99]=ON/OFF,DictAbout,KeyHelpPath
		if( buff.length() < 10 ||
			auto_memcmp(buff.c_str(), LTEXT("KDct["), 5) != 0 ||
			auto_memcmp(&buff[7], LTEXT("]="), 2) != 0
			){
			//	2007.02.03 genta 処理を継続
			++invalid_record;
			continue;
		}

		WCHAR *p1, *p2, *p3;
		p1 = &buff[9];
		p3 = p1;					//結果確認用に初期化
		if( NULL != (p2=wcsstr(p1,LTEXT(","))) ){
			*p2 = LTEXT('\0');
			p2 += 1;				//カンマの次が、次の要素
			if( NULL != (p3=wcsstr(p2,LTEXT(","))) ){
				*p3 = LTEXT('\0');
				p3 += 1;			//カンマの次が、次の要素
			}
		}/* 結果の確認 */
		if( (p3==NULL) ||			//カンマが1個足りない
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
		if( (fp2=_tfopen_absini(to_tchar(p3),_T("r"))) == NULL ){	// 2007.02.03 genta 相対パスはsakura.exe基準で開く	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
			// 2007.02.03 genta 辞書が見つからない場合の措置．警告を出すが取り込む
			p4 = LSW(STR_IMPEXP_DIC_NOTFOUND);
			b_enable_flag = 0;
		}
		else
			fclose(fp2);

		//About
		if (wcslen(p2) > DICT_ABOUT_LEN) {
			auto_sprintf( msgBuff, LSW(STR_IMPEXP_DIC_LENGTH), DICT_ABOUT_LEN );
			sErrMsg = msgBuff;
			++invalid_record;
			continue;
		}

		//良さそうなら
		m_Types.m_KeyHelpArr[i].m_bUse = (b_enable_flag!=0);	// 2007.02.03 genta
		_tcscpy(m_Types.m_KeyHelpArr[i].m_szAbout, to_tchar(p4));
		_tcscpy(m_Types.m_KeyHelpArr[i].m_szPath,  to_tchar(p3));
		i++;
	}
	in.Close();

	// 空きがあるなら番兵を設定
	if( i < _countof(m_Types.m_KeyHelpArr) ){
		m_Types.m_KeyHelpArr[i].m_bUse = false;
		m_Types.m_KeyHelpArr[i].m_szAbout[0] = _T('\0');
		m_Types.m_KeyHelpArr[i].m_szPath[0]  = _T('\0');
	}
	m_Types.m_nKeyHelpNum = i;

	// 2007.02.03 genta 失敗したら警告する
	if( invalid_record > 0 ){
		auto_sprintf( msgBuff, LSW(STR_IMPEXP_DIC_RECORD), invalid_record );
		sErrMsg = msgBuff;
	}

	return true;
}

/*! エクスポート
	@date 2010.07.14 Moca ListViewからm_Typesからのエクスポートに変更
*/
bool CImpExpKeyHelp::Export( const wstring& sFileName, wstring& sErrMsg )
{
	CTextOutputStream out( to_tchar( sFileName.c_str() ) );
	if (!out) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	out.WriteF( WSTR_KEYHELP_HEAD );

	for (int i = 0; i < m_Types.m_nKeyHelpNum; i++) {
		out.WriteF(
			L"KDct[%02d]=%d,%ts,%ts\n",
			i,
			m_Types.m_KeyHelpArr[i].m_bUse?1:0,
			m_Types.m_KeyHelpArr[i].m_szAbout,
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
bool CImpExpKeybind::Import( const wstring& sFileName, wstring& sErrMsg )
{
	const tstring	strPath = to_tchar( sFileName.c_str() );
	const int KEYNAME_SIZE = _countof(m_Common.m_sKeyBind.m_pKeyNameArr)-1;// 最後の１要素はダミー用に予約 2012.11.25 aroka
	CommonSetting_KeyBind sKeyBind = m_Common.m_sKeyBind;

	//オープン
	CDataProfile in;
	in.SetReadingMode();
	if (!in.ReadProfile( to_tchar( sFileName.c_str() ))) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
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
	in.IOProfileData(szSecInfo, L"KEYBIND_VERSION", MakeStringBufferW(szHeader));
	if(wcscmp(szHeader,WSTR_KEYBIND_HEAD4)==0)	bVer4=true;
	else if(wcscmp(szHeader,WSTR_KEYBIND_HEAD3)==0)	bVer3=true;

	//int	nKeyNameArrNum;			// キー割り当て表の有効データ数
	if ( bVer3 || bVer4 ) {
		//Count取得 -> nKeyNameArrNum
		in.IOProfileData(szSecInfo, L"KEYBIND_COUNT", sKeyBind.m_nKeyNameArrNum);
		if (sKeyBind.m_nKeyNameArrNum < 0 || sKeyBind.m_nKeyNameArrNum > KEYNAME_SIZE){	bVer3=false; bVer4=false; } //範囲チェック

		CShareData_IO::IO_KeyBind(in, sKeyBind, true);	// 2008/5/25 Uchi
	}

	if (!bVer3 && !bVer4) {
		// 新バージョンでない
		CTextInputStream in(strPath.c_str());
		if (!in) {
			sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
			return false;
		}
		// ヘッダチェック
		wstring	szLine = in.ReadLineW();
		bVer2 = true;
		if ( wcscmp(szLine.c_str(), WSTR_KEYBIND_HEAD2) != 0)	bVer2 = false;
		// カウントチェック
		int	i, cnt;
		if ( bVer2 ) {
			int	an;
			szLine = in.ReadLineW();
			cnt = swscanf(szLine.c_str(), L"Count=%d", &an);
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
				wchar_t szData[1024];
				auto_strncpy(szData, in.ReadLineW().c_str(), _countof(szData) - 1);
				szData[_countof(szData) - 1] = L'\0';

				//解析開始
				cnt = swscanf(szData, L"KeyBind[%03d]=%04x,%n",
												&n,   &kc, &nc);
				if( cnt !=2 && cnt !=3 )	{ bVer2= false; break;}
				if( i != n ) break;
				sKeyBind.m_pKeyNameArr[i].m_nKeyCode = (short)kc;
				wchar_t* p = szData + nc;

				//後に続くトークン
				for(int j=0;j<8;j++)
				{
					wchar_t* q=auto_strchr(p,L',');
					if(!q)	{ bVer2= false; break;}
					*q=L'\0';

					//機能名を数値に置き換える。(数値の機能名もあるかも)
					//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
					EFunctionCode n = CSMacroMgr::GetFuncInfoByName(G_AppInstance(), p, NULL);
					if( n == F_INVALID )
					{
						if( WCODE::Is09(*p) )
						{
							n = (EFunctionCode)auto_atol(p);
						}
						else
						{
							n = F_DEFAULT;
						}
					}
					sKeyBind.m_pKeyNameArr[i].m_nFuncCodeArr[j] = n;
					p = q + 1;
				}

				auto_strncpy(sKeyBind.m_pKeyNameArr[i].m_szKeyName, to_tchar(p), _countof(sKeyBind.m_pKeyNameArr[i].m_szKeyName)-1);
				sKeyBind.m_pKeyNameArr[i].m_szKeyName[_countof(sKeyBind.m_pKeyNameArr[i].m_szKeyName)-1] = '\0';
			}
		}
	}
	if (!bVer4 && !bVer3 && !bVer2) {
		sErrMsg = wstring(LSW(STR_IMPEXP_KEY_FORMAT)) + sFileName;
		return false;
	}

	// データのコピー 	// マウスコードの固定と重複排除 2012.11.19 aroka
	//m_Common.m_sKeyBind.m_nKeyNameArrNum = nKeyNameArrNum;
	//memcpy_raw( m_Common.m_sKeyBind.m_pKeyNameArr, pKeyNameArr, sizeof_raw( pKeyNameArr ) );
	int nKeyNameArrUsed = m_Common.m_sKeyBind.m_nKeyNameArrNum; // 使用済み領域
	for( int j=sKeyBind.m_nKeyNameArrNum-1; j>=0; j-- ){
		if( (bVer2 || bVer3) && sKeyBind.m_pKeyNameArr[j].m_nKeyCode <= 0 ){ // マウスコードは先頭に固定されている KeyCodeが同じなのでKeyNameで判別
			for( int im=0; im< MOUSEFUNCTION_KEYBEGIN; im++ ){
				if( _tcscmp( sKeyBind.m_pKeyNameArr[j].m_szKeyName, m_Common.m_sKeyBind.m_pKeyNameArr[im].m_szKeyName ) == 0 ){
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
			if( nKeyNameArrUsed >= KEYNAME_SIZE ) continue;
			m_Common.m_sKeyBind.m_pKeyNameArr[nKeyNameArrUsed] = sKeyBind.m_pKeyNameArr[j2];
			sKeyBind.m_VKeyToKeyNameArr[sKeyBind.m_pKeyNameArr[j2].m_nKeyCode] = (BYTE)nKeyNameArrUsed++;
		}
	}
	m_Common.m_sKeyBind.m_nKeyNameArrNum = nKeyNameArrUsed;
	memcpy_raw( m_Common.m_sKeyBind.m_VKeyToKeyNameArr, sKeyBind.m_VKeyToKeyNameArr, sizeof_raw(sKeyBind.m_VKeyToKeyNameArr) );

	return true;
}

// エクスポート
bool CImpExpKeybind::Export( const wstring& sFileName, wstring& sErrMsg )
{
	const tstring	strPath = to_tchar( sFileName.c_str() );

	CTextOutputStream out( strPath.c_str() );
	if (!out) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	out.Close();

	/* キー割り当て情報 */
	CDataProfile cProfile;

	// 書き込みモード設定
	cProfile.SetWritingMode();

	// ヘッダ
	StaticString<wchar_t,256> szKeydataHead = WSTR_KEYBIND_HEAD4;
	cProfile.IOProfileData( szSecInfo, L"KEYBIND_VERSION", szKeydataHead );
	cProfile.IOProfileData_WrapInt( szSecInfo, L"KEYBIND_COUNT", m_Common.m_sKeyBind.m_nKeyNameArrNum );

	//内容
	CShareData_IO::IO_KeyBind(cProfile, m_Common.m_sKeyBind, true);

	// 書き込み
	if (!cProfile.WriteProfile( strPath.c_str(), WSTR_KEYBIND_HEAD4)) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_EXPORT)) + sFileName;
		return false;
	}

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     カスタムメニュー                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// インポート
bool CImpExpCustMenu::Import( const wstring& sFileName, wstring& sErrMsg )
{
	const tstring	strPath = to_tchar( sFileName.c_str() );

	//ヘッダ確認
	CTextInputStream in(strPath.c_str());
	if (!in) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	CDataProfile cProfile;
	cProfile.SetReadingMode();
	cProfile.ReadProfile(strPath.c_str());

	//バージョン確認
	WCHAR szHeader[256];
	cProfile.IOProfileData(szSecInfo, L"MENU_VERSION", MakeStringBufferW(szHeader));
	if(wcscmp(szHeader, WSTR_CUSTMENU_HEAD_V2)!=0) {
		sErrMsg = wstring(LSW(STR_IMPEXP_CUSTMENU_FORMAT)) + sFileName;
		return false;
	}

	CShareData_IO::IO_CustMenu(cProfile,m_Common.m_sCustomMenu, true);			// 2008/5/24 Uchi

	return true;
}

// エクスポート
bool CImpExpCustMenu::Export( const wstring& sFileName, wstring& sErrMsg )
{
	const tstring	strPath = to_tchar( sFileName.c_str() );

	// オープン
	CTextOutputStream out(strPath.c_str());
	if (!out) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	out.Close();

	/* カスタムメニュー情報 */
	//ヘッダ
	CDataProfile	cProfile;
	CommonSetting_CustomMenu* menu=&m_Common.m_sCustomMenu;

	// 書き込みモード設定
	cProfile.SetWritingMode();

	//ヘッダ
	cProfile.IOProfileData( szSecInfo, L"MENU_VERSION", MakeStringBufferW(WSTR_CUSTMENU_HEAD_V2) );
	int iWork = MAX_CUSTOM_MENU;
	cProfile.IOProfileData_WrapInt( szSecInfo, L"MAX_CUSTOM_MENU", iWork );
	
	//内容
	CShareData_IO::IO_CustMenu(cProfile, *menu, true);

	// 書き込み
	if (!cProfile.WriteProfile( strPath.c_str(), WSTR_CUSTMENU_HEAD_V2)) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_EXPORT)) + sFileName;
		return false;
	}

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     強調キーワード                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// インポート
bool CImpExpKeyWord::Import( const wstring& sFileName, wstring& sErrMsg )
{
	bool			bAddError = false;

	CTextInputStream in(to_tchar(sFileName.c_str()));
	if (!in) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}
	while( in ){
		wstring szLine = in.ReadLineW();

		// コメント無視
		if (szLine.length() == 0) {
			continue;
		}
		if (2 <= szLine.length() && 0 == auto_memcmp( szLine.c_str(), L"//", 2 )) {
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
		sErrMsg = LSW(STR_IMPEXP_KEYWORD);
	}

	return true;
}

// エクスポート
bool CImpExpKeyWord::Export( const wstring& sFileName, wstring& sErrMsg )
{
	int 		nKeyWordNum;
	int 		i;

	CTextOutputStream out(to_tchar(sFileName.c_str()));
	if (!out) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}
	out.WriteF( L"// " );
	// 2012.03.10 syat キーワードに「%」を含む場合にエクスポート結果が不正
	out.WriteString( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( m_nIdx ) );
	out.WriteF( WSTR_KEYWORD_HEAD );

	out.WriteF( WSTR_KEYWORD_CASE );
	out.WriteF( m_bCase ? L"True" : L"False" );
	out.WriteF( L"\n\n" );

	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SortKeyWord(m_nIdx);	//MIK 2000.12.01 sort keyword

	/* ｎ番目のセットのキーワードの数を返す */
	nKeyWordNum = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordNum( m_nIdx );
	for( i = 0; i < nKeyWordNum; ++i ){
		/* ｎ番目のセットのｍ番目のキーワードを返す */
		// 2012.03.10 syat キーワードに「%」を含む場合にエクスポート結果が不正
		out.WriteString( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( m_nIdx, i ) );
		out.WriteF( L"\n" );
	}
	out.Close();

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     メインメニュー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//	2010/5/23 Uchi
// インポート
bool CImpExpMainMenu::Import( const wstring& sFileName, wstring& sErrMsg )
{
	const tstring strPath = to_tchar( sFileName.c_str() );

	//ヘッダ確認
	CTextInputStream in(strPath.c_str());
	if (!in) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	CDataProfile cProfile;
	cProfile.SetReadingMode();
	cProfile.ReadProfile( strPath.c_str() );

	//バージョン確認
	WCHAR szHeader[256];
	cProfile.IOProfileData(szSecInfo, L"MENU_VERSION", MakeStringBufferW(szHeader));
	if(wcscmp(szHeader, WSTR_MAINMENU_HEAD_V1)!=0) {
		sErrMsg = wstring(LSW(STR_IMPEXP_MEINMENU)) + sFileName;
		return false;
	}

	CShareData_IO::IO_MainMenu(cProfile,m_Common.m_sMainMenu, true);

	return true;
}

// エクスポート
bool CImpExpMainMenu::Export( const wstring& sFileName, wstring& sErrMsg )
{
	const tstring strPath = to_tchar( sFileName.c_str() );

	// オープン
	CTextOutputStream out( strPath.c_str() );
	if (!out) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	out.Close();

	//ヘッダ
	CDataProfile	cProfile;
	CommonSetting_MainMenu* menu=&m_Common.m_sMainMenu;

	// 書き込みモード設定
	cProfile.SetWritingMode();

	//ヘッダ
	cProfile.IOProfileData( szSecInfo, L"MENU_VERSION", MakeStringBufferW(WSTR_MAINMENU_HEAD_V1) );
	
	//内容
	CShareData_IO::IO_MainMenu(cProfile, *menu, true);

	// 書き込み
	if (!cProfile.WriteProfile( strPath.c_str(), WSTR_MAINMENU_HEAD_V1)) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_EXPORT)) + sFileName;
		return false;
	}

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ファイルツリー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//	2014.06.07 Moca
// インポート
bool CImpExpFileTree::Import( const wstring& sFileName, wstring& sErrMsg )
{
	const tstring strPath = to_tchar( sFileName.c_str() );

	CDataProfile cProfile;
	cProfile.SetReadingMode();
	cProfile.ReadProfile( strPath.c_str() );

	IO_FileTreeIni( cProfile, m_aFileTreeItems );

	return true;
}

// エクスポート
bool CImpExpFileTree::Export( const wstring& sFileName, wstring& sErrMsg )
{
	const tstring strPath = to_tchar( sFileName.c_str() );

	CDataProfile	cProfile;

	// 書き込みモード設定
	cProfile.SetWritingMode();

	IO_FileTreeIni( cProfile, m_aFileTreeItems );

	// 書き込み
	if (!cProfile.WriteProfile( strPath.c_str(), WSTR_FILETREE_HEAD_V1)) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_EXPORT)) + sFileName;
		return false;
	}

	return true;
}

void CImpExpFileTree::IO_FileTreeIni( CDataProfile& cProfile, std::vector<SFileTreeItem>& data )
{
	const WCHAR* pszSecName = L"FileTree";
	int nItemCount = (int)data.size();
	cProfile.IOProfileData( pszSecName, L"nFileTreeItemCount", nItemCount );
	if( nItemCount < 0 ){
		nItemCount = 0;
	}
	int i = 0;
	if( cProfile.IsReadingMode() ){
		data.resize( nItemCount );
	}
	for( ;i < nItemCount; i++ ){
		CShareData_IO::ShareData_IO_FileTreeItem( cProfile, data[i], pszSecName, i );
	}
}
