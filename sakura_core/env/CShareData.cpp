/*!	@file
	@brief プロセス間共有データへのアクセス

	@author Norio Nakatani
	@date 1998/05/26  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta, MIK
	Copyright (C) 2001, jepro, genta, asa-o, MIK, YAZAKI, hor
	Copyright (C) 2002, genta, ai, Moca, MIK, YAZAKI, hor, KK, aroka
	Copyright (C) 2003, Moca, aroka, MIK, genta, wmlhq, sui
	Copyright (C) 2004, Moca, novice, genta, isearch, MIK
	Copyright (C) 2005, Moca, MIK, genta, ryoji, りんご, aroka
	Copyright (C) 2006, aroka, ryoji, genta
	Copyright (C) 2007, ryoji, genta, maru
	Copyright (C) 2008, ryoji, Uchi, nasukoji
	Copyright (C) 2009, nasukoji, ryoji
	Copyright (C) 2011, nasukoji
	Copyright (C) 2012, Moca, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CShareData_IO.h"
#include "env/CSakuraEnvironment.h"
#include "doc/CDocListener.h" // SLoadInfo
#include "_main/CControlProcess.h"
#include "_main/CControlTray.h"
#include "_main/CCommandLine.h"
#include "_main/CMutex.h"
#include "charset/CCodePage.h"
#include "debug/CRunningTimer.h"
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"
#include "util/module.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "util/os.h"
#include "CDataProfile.h"
#include "apiwrap/StdApi.h"
#include "sakura_rc.h"
#include "config/system_constants.h"
#include "String_define.h"

struct ARRHEAD {
	int		nLength;
	int		nItemNum;
};

const unsigned int uShareDataVersion = N_SHAREDATA_VERSION;

UINT GetPrivateProfileIntW(
	_In_ LPCWSTR lpAppName,
	_In_ LPCWSTR lpKeyName,
	_In_ INT nDefault,
	std::optional<std::filesystem::path> iniPath
)
{
	return GetPrivateProfileIntW(lpAppName, lpKeyName, nDefault, iniPath.has_value() ? iniPath.value().c_str() : nullptr);
}

std::wstring GetPrivateProfileStringW(
	_In_opt_ LPCWSTR lpAppName,
	_In_opt_ LPCWSTR lpKeyName,
	_In_opt_ LPCWSTR lpDefault,
	_In_     DWORD nSize,
	std::optional<std::filesystem::path> iniPath
)
{
	std::wstring buffer(nSize, L'\0');
	if (!GetPrivateProfileStringW(lpAppName, lpKeyName, lpDefault, buffer.data(), DWORD(buffer.size()), iniPath.has_value() ? iniPath.value().c_str() : nullptr) && lpDefault && *lpDefault) {
		buffer = lpDefault;
	}
	return buffer.data();
}

std::filesystem::path SHGetKnownFolderPath(
	_In_ REFKNOWNFOLDERID refFolderId,
	_In_ DWORD dwFlags,
	_In_opt_ HANDLE hToken = nullptr
)
{
	PWSTR pFolderPath = nullptr;
	if (FAILED(::SHGetKnownFolderPath(refFolderId, dwFlags, hToken, &pFolderPath))) {
		return std::filesystem::path();
	}
	std::filesystem::path privateIniPath(pFolderPath);
	::CoTaskMemFree(pFolderPath);
	return privateIniPath;
}

/*!
	@brief マルチユーザー用のiniファイルパスを組み立てる
 */
/* static */ std::filesystem::path CShareData::BuildPrivateIniFileName(
	const std::filesystem::path& iniFolder,
	bool isMultiUserSettings,
	UINT userRootFolder,
	const std::wstring& userSubFolder,
	_In_opt_z_ LPCWSTR szProfileName,
	const std::wstring& iniFileName
)
{
	std::filesystem::path privateIniPath = iniFolder;
	if (isMultiUserSettings) {
		KNOWNFOLDERID refFolderId;
		switch (userRootFolder) {
		case 1:
		case 3:
			refFolderId = FOLDERID_Profile;			// ユーザーのルートフォルダー
			break;

		case 2:
			refFolderId = FOLDERID_Documents;		// ユーザーのドキュメントフォルダー
			break;

		default:
			refFolderId = FOLDERID_RoamingAppData;	// ユーザーのアプリケーションデータフォルダー
			break;
		}

		privateIniPath = SHGetKnownFolderPath(refFolderId, KF_FLAG_DEFAULT_PATH);
		if (3 == userRootFolder) {
			privateIniPath /= L"Desktop";
		}

		assert(!userSubFolder.empty());
		privateIniPath /= userSubFolder;
	}

	if (szProfileName && *szProfileName) {
		privateIniPath /= szProfileName;
	}

	privateIniPath /= iniFileName;

	return privateIniPath;
}

/*!
 * コンストラクタ
 * CShareData_new2.cppと統合
 * 
 * @date 2002/01/03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動
 */
CShareData::CShareData(
	_In_opt_z_ LPCWSTR profileName	//!< [in] プロファイル名
) noexcept
	: m_szProfileName(profileName)
{
}

/*!
	共有メモリ領域がある場合はプロセスのアドレス空間から､
	すでにマップされているファイル ビューをアンマップする。
*/
CShareData::~CShareData()
{
	if( m_pShareData ){
		/* プロセスのアドレス空間から､ すでにマップされているファイル ビューをアンマップします */
		SetDllShareData( NULL );
		::UnmapViewOfFile( m_pShareData );
		m_pShareData = NULL;
	}
	if( m_hFileMap ){
		CloseHandle( m_hFileMap );
	}
	if( m_pvTypeSettings ){
		for( int i = 0; i < (int)m_pvTypeSettings->size(); i++ ){
			delete (*m_pvTypeSettings)[i];
			(*m_pvTypeSettings)[i] = NULL;
		}
		delete m_pvTypeSettings;
		m_pvTypeSettings = NULL;
	}
}

static CMutex g_cMutexShareWork( FALSE, GSTR_MUTEX_SAKURA_SHAREWORK );
 
CMutex& CShareData::GetMutexShareWork(){
	return g_cMutexShareWork;
}

//! CShareDataクラスの初期化処理
/*!
	CShareDataクラスを利用する前に必ず呼び出すこと。

	@retval true 初期化成功
	@retval false 初期化失敗

	@note 既に存在する共有メモリのバージョンがこのエディタが使うものと
	異なる場合は致命的エラーを防ぐためにfalseを返します。CProcess::Initialize()
	でInit()に失敗するとメッセージを出してエディタの起動を中止します。

	@date 2018/06/01 仕様変更 https://github.com/sakura-editor/sakura/issues/29
*/
bool CShareData::InitShareData(const std::optional<SMultiUserSettings>& multiUserSettings)
{
	MY_RUNNINGTIMER(cRunningTimer,L"CShareData::InitShareData" );

	m_hwndTraceOutSource = NULL;	// 2006.06.26 ryoji

	const ULARGE_INTEGER maxShareDataSize = { ULONGLONG(sizeof(DLLSHAREDATA)) };

	SFilePath szShareDataName = GSTR_SHAREDATA;
	if (m_szProfileName && *m_szProfileName) {
		szShareDataName += m_szProfileName;
	}

	/* ファイルマッピングオブジェクト */
	{
		m_hFileMap = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			LPSECURITY_ATTRIBUTES(nullptr),
			PAGE_READWRITE | SEC_COMMIT,
			maxShareDataSize.HighPart,
			maxShareDataSize.LowPart,
			szShareDataName
		);
	}
	if( NULL == m_hFileMap ){
		::MessageBox(
			NULL,
			L"CreateFileMapping()に失敗しました",
			L"予期せぬエラー",
			MB_OK | MB_APPLMODAL | MB_ICONSTOP
		);
		return false;
	}


	if( GetLastError() != ERROR_ALREADY_EXISTS ){
		/* オブジェクトが存在していなかった場合 */
		const auto isMultiUserSettings = multiUserSettings.has_value();
		const auto userSettings = multiUserSettings.value_or(SMultiUserSettings());
		const auto userRootFolder = userSettings.userRootFolder;
		const auto& userSubFolder = userSettings.userSubFolder;

		// exe基準のiniファイルパスを得る
		const auto iniPath = GetExeFileName().replace_extension(L".ini");

		// 設定ファイルフォルダー
		auto iniFolder = iniPath;
		iniFolder.remove_filename();

		// iniファイル名を得る
		const auto filename = iniPath.filename();

		// マルチユーザー用のiniファイルパスを組み立てる
		auto privateIniPath = BuildPrivateIniFileName(iniFolder, isMultiUserSettings, userRootFolder, userSubFolder, m_szProfileName, filename);

		/* ファイルのビューを､ 呼び出し側プロセスのアドレス空間にマップします */
		auto mappedView = ::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		CreateTypeSettings();
		m_pShareData = new(mappedView) DLLSHAREDATA(iniPath, privateIniPath, iniFolder, *m_pvTypeSettings);
		SetDllShareData( m_pShareData );

	}else{
		/* オブジェクトがすでに存在する場合 */
		/* ファイルのビューを､ 呼び出し側プロセスのアドレス空間にマップします */
		m_pShareData = (DLLSHAREDATA*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		SetDllShareData( m_pShareData );

		SelectCharWidthCache( CWM_FONT_EDIT, CWM_CACHE_SHARE );
		InitCharWidthCache(m_pShareData->m_Common.m_sView.m_lf);	// 2008/5/15 Uchi

		//	From Here Oct. 27, 2000 genta
		//	2014.01.08 Moca サイズチェック追加
		if( m_pShareData->m_vStructureVersion != uShareDataVersion ||
			m_pShareData->m_nSize != sizeof(*m_pShareData) ){
			//	この共有データ領域は使えない．
			//	ハンドルを解放する
			SetDllShareData( NULL );
			::UnmapViewOfFile( m_pShareData );
			m_pShareData = NULL;
			return false;
		}
		//	To Here Oct. 27, 2000 genta
	}
	return true;
}

static void ConvertLangString( wchar_t* pBuf, size_t chBufSize, std::wstring& org, std::wstring& to )
{
	CNativeW mem;
	mem.SetString(pBuf);
	mem.Replace(org.c_str(), to.c_str());
	wcsncpy(pBuf, mem.GetStringPtr(), chBufSize);
	pBuf[chBufSize - 1] = L'\0';
}

static void ConvertLangValueImpl( wchar_t* pBuf, size_t chBufSize, int nStrId, std::vector<std::wstring>& values, int& index, bool setValues, bool bUpdate )
{
	if( setValues ){
		if( bUpdate ){
			values.push_back( LS(nStrId) );
		}
		return;
	}
	std::wstring to = LS(nStrId);
	ConvertLangString( pBuf, chBufSize, values[index], to );
	index++;
}

#define ConvertLangValue(buf, id)  ConvertLangValueImpl(buf, _countof(buf), id, values, index, bSetValues, true)
#define ConvertLangValue2(buf, id) ConvertLangValueImpl(buf, _countof(buf), id, values, index, bSetValues, false)

/*!
	国際化対応のための文字列を変更する

	1. 1回目呼び出し、setValuesをtrueにして、valuesに旧設定の言語文字列を読み込み
	2. SelectLang呼び出し
	3. 2回目呼び出し、valuesを使って新設定の言語に書き換え
*/
void CShareData::ConvertLangValues(std::vector<std::wstring>& values, bool bSetValues)
{
	DLLSHAREDATA&	shareData = *m_pShareData;
	int i;
	int index = 0;
	int indexBackup;
	CommonSetting& common = shareData.m_Common;
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_TAB_CAPTION_OUTPUT );
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_TAB_CAPTION_GREP );
	indexBackup = index;
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_CAPTION_ACTIVE_OUTPUT );
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_CAPTION_ACTIVE_UPDATE );
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_CAPTION_ACTIVE_VIEW );
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_CAPTION_ACTIVE_OVERWRITE );
	ConvertLangValue( common.m_sTabBar.m_szTabWndCaption, STR_CAPTION_ACTIVE_KEYMACRO );
	index = indexBackup;
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionActive, STR_CAPTION_ACTIVE_OUTPUT );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionActive, STR_CAPTION_ACTIVE_UPDATE );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionActive, STR_CAPTION_ACTIVE_VIEW );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionActive, STR_CAPTION_ACTIVE_OVERWRITE );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionActive, STR_CAPTION_ACTIVE_KEYMACRO );
	index = indexBackup;
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionInactive, STR_CAPTION_ACTIVE_OUTPUT );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionInactive, STR_CAPTION_ACTIVE_UPDATE );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionInactive, STR_CAPTION_ACTIVE_VIEW );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionInactive, STR_CAPTION_ACTIVE_OVERWRITE );
	ConvertLangValue2( common.m_sWindow.m_szWindowCaptionInactive, STR_CAPTION_ACTIVE_KEYMACRO );
	ConvertLangValue( common.m_sFormat.m_szDateFormat, STR_DATA_FORMAT );
	ConvertLangValue( common.m_sFormat.m_szTimeFormat, STR_TIME_FORMAT );
	indexBackup = index;
	for( i = 0; i < common.m_sFileName.m_nTransformFileNameArrNum; i++ ){
		index = indexBackup;
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_COMDESKTOP );
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_COMDOC );
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_DESKTOP );
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_MYDOC );
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_IE );
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_TEMP );
		ConvertLangValue( common.m_sFileName.m_szTransformFileNameTo[i], STR_TRANSNAME_APPDATA );
		if( bSetValues ){
			break;
		}
	}
	indexBackup = index;
	for( i = 0; i < MAX_PRINTSETTINGARR; i++ ){
		index = indexBackup;
		ConvertLangValue( shareData.m_PrintSettingArr[i].m_szPrintSettingName, STR_PRINT_SET_NAME );
		if( bSetValues ){
			break;
		}
	}
	assert( m_pvTypeSettings != NULL );
	indexBackup = index;
	ConvertLangValue( shareData.m_TypeBasis.m_szTypeName, STR_TYPE_NAME_BASIS );
	for( i = 0; i < (int)GetTypeSettings().size(); i++ ){
		index = indexBackup;
		STypeConfig& type = *(GetTypeSettings()[i]);
		ConvertLangValue2( type.m_szTypeName, STR_TYPE_NAME_BASIS );
		ConvertLangValue( type.m_szTypeName, STR_TYPE_NAME_RICHTEXT );
		ConvertLangValue( type.m_szTypeName, STR_TYPE_NAME_TEXT );
		ConvertLangValue( type.m_szTypeName, STR_TYPE_NAME_DOS );
		ConvertLangValue( type.m_szTypeName, STR_TYPE_NAME_ASM );
		ConvertLangValue( type.m_szTypeName, STR_TYPE_NAME_INI );
		index = indexBackup;
		ConvertLangValue2( shareData.m_TypeMini[i].m_szTypeName, STR_TYPE_NAME_BASIS );
		ConvertLangValue2( shareData.m_TypeMini[i].m_szTypeName, STR_TYPE_NAME_RICHTEXT );
		ConvertLangValue2( shareData.m_TypeMini[i].m_szTypeName, STR_TYPE_NAME_TEXT );
		ConvertLangValue2( shareData.m_TypeMini[i].m_szTypeName, STR_TYPE_NAME_DOS );
		ConvertLangValue2( shareData.m_TypeMini[i].m_szTypeName, STR_TYPE_NAME_ASM );
		ConvertLangValue2( shareData.m_TypeMini[i].m_szTypeName, STR_TYPE_NAME_INI );
		if( bSetValues ){
			break;
		}
	}
}

/*!
	@brief	指定ファイルが開かれているか調べる
	
	指定のファイルが開かれている場合は開いているウィンドウのハンドルを返す

	@retval	TRUE すでに開いていた
	@retval	FALSE 開いていなかった
*/
BOOL CShareData::IsPathOpened( const WCHAR* pszPath, HWND* phwndOwner )
{
	EditInfo*	pfi;
	*phwndOwner = NULL;

	//	2007.10.01 genta 相対パスを絶対パスに変換
	//	変換しないとIsPathOpenedで正しい結果が得られず，
	//	同一ファイルを複数開くことがある．
	WCHAR	szBuf[_MAX_PATH];
	if( GetLongFileName( pszPath, szBuf )){
		pszPath = szBuf;
	}

	// 現在の編集ウィンドウの数を調べる
	if( 0 == CAppNodeGroupHandle(0).GetEditorWindowsNum() ){
		return FALSE;
	}
	
	for( int i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; ++i ){
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd ) ){
			// トレイからエディタへの編集ファイル名要求通知
			::SendMessageAny( m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd, MYWM_GETFILEINFO, 1, 0 );
			pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

			// 同一パスのファイルが既に開かれているか
			if( 0 == _wcsicmp( pfi->m_szPath, pszPath ) ){
				*phwndOwner = m_pShareData->m_sNodes.m_pEditArr[i].m_hWnd;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*!
	@brief	指定ファイルが開かれているか調べつつ、多重オープン時の文字コード衝突も確認

	もしすでに開いていればアクティブにして、ウィンドウのハンドルを返す。
	さらに、文字コードが異なるときのワーニングも処理する。
	あちこちに散らばった多重オープン処理を集結させるのが目的。

	@retval	開かれている場合は開いているウィンドウのハンドル

	@note	CEditDoc::FileLoadに先立って実行されることもあるが、
			CEditDoc::FileLoadからも実行される必要があることに注意。
			(フォルダー指定の場合やCEditDoc::FileLoadが直接実行される場合もあるため)

	@retval	TRUE すでに開いていた
	@retval	FALSE 開いていなかった

	@date 2007.03.12 maru 新規作成
*/
BOOL CShareData::ActiveAlreadyOpenedWindow( const WCHAR* pszPath, HWND* phwndOwner, ECodeType nCharCode )
{
	if( IsPathOpened( pszPath, phwndOwner ) ){
		
		//文字コードの一致確認
		EditInfo*		pfi;
		::SendMessageAny( *phwndOwner, MYWM_GETFILEINFO, 0, 0 );
		pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;
		if(nCharCode != CODE_AUTODETECT){
			WCHAR szCpNameCur[100];
			CCodePage::GetNameLong(szCpNameCur, pfi->m_nCharCode);
			WCHAR szCpNameNew[100];
			CCodePage::GetNameLong(szCpNameNew, pfi->m_nCharCode);
			if(szCpNameCur[0] && szCpNameNew[0]){
				if(nCharCode != pfi->m_nCharCode){
					TopWarningMessage( *phwndOwner,
						LS(STR_ERR_CSHAREDATA20),
						pszPath,
						szCpNameCur,
						szCpNameNew
					);
				}
			}
			else{
				TopWarningMessage( *phwndOwner,
					LS(STR_ERR_CSHAREDATA21),
					pszPath,
					pfi->m_nCharCode,
					0==szCpNameCur[0]?LS(STR_ERR_CSHAREDATA22):szCpNameCur,
					nCharCode,
					0==szCpNameNew[0]?LS(STR_ERR_CSHAREDATA22):szCpNameNew
				);
			}
		}

		// 開いているウィンドウをアクティブにする
		ActivateFrameWindow( *phwndOwner );

		// MRUリストへの登録
		CMRUFile().Add( pfi );
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/*!
	アウトプットウインドウに出力(文字列指定)

	長い場合は分割して送る
	アウトプットウインドウが無ければオープンする
	@param  pStr  出力する文字列
	@param  len   pStrの文字数(終端NULを含まない) -1で自動計算
	@date 2010.05.11 Moca 新設
*/
void CShareData::TraceOutString( const wchar_t* pStr, int len )
{
	if( false == OpenDebugWindow( m_hwndTraceOutSource, false ) ){
		return;
	}
	if( -1 == len ){
		len = wcslen(pStr);
	}
	// m_sWorkBufferぎりぎりでも問題ないけれど、念のため\0終端にするために余裕をとる
	// -1 より 8,4バイト境界のほうがコピーが早いはずなので、-4にする
	const int buffLen = (int)m_pShareData->m_sWorkBuffer.GetWorkBufferCount<WCHAR>() - 4;
	wchar_t*  pOutBuffer = m_pShareData->m_sWorkBuffer.GetWorkBuffer<WCHAR>();
	int outPos = 0;
	if(0 == len){
		// 0のときは何も追加しないが、カーソル移動が発生する
		LockGuard<CMutex> guard( CShareData::GetMutexShareWork() );
		pOutBuffer[0] = L'\0';
		::SendMessage( m_pShareData->m_sHandles.m_hwndDebug, MYWM_ADDSTRINGLEN_W, 0, 0 );
	}else{
		while(outPos < len){
			int outLen = buffLen;
			if(len - outPos < buffLen){
				// 残り全部
				outLen = len - outPos;
			}
			// あまりが1文字以上ある
			if( outPos + outLen < len ){
				// CRLF(\r\n)とUTF-16が分離されないように
				if( (pStr[outPos + outLen - 1] == WCODE::CR && pStr[outPos + outLen] == WCODE::LF)
					|| (IsUtf16SurrogHi( pStr[outPos + outLen - 1] ) && IsUtf16SurrogLow( pStr[outPos + outLen] )) ){
					--outLen;
				}
			}
			LockGuard<CMutex> guard( CShareData::GetMutexShareWork() );
			wmemcpy( pOutBuffer, pStr + outPos, outLen );
			pOutBuffer[outLen] = L'\0';
			DWORD_PTR	dwMsgResult;
			if( 0 == ::SendMessageTimeout( m_pShareData->m_sHandles.m_hwndDebug, MYWM_ADDSTRINGLEN_W, outLen, 0,
				SMTO_NORMAL, 10000, &dwMsgResult ) ){
				// エラーかタイムアウト
				break;
			}
			outPos += outLen;
		}
	}
}

/*
	デバッグウィンドウを表示
	@param hwnd 新規ウィンドウのときのデバッグウィンドウの所属グループ
	@param bAllwaysActive 表示済みウィンドウでもアクティブにする
	@return true:表示できた。またはすでに表示されている。
	@date 2010.05.11 Moca TraceOutから分離
*/
bool CShareData::OpenDebugWindow( HWND hwnd, bool bAllwaysActive )
{
	bool ret = true;
	if( NULL == m_pShareData->m_sHandles.m_hwndDebug
	|| !IsSakuraMainWindow( m_pShareData->m_sHandles.m_hwndDebug )
	){
		// 2007.06.26 ryoji
		// アウトプットウィンドウを作成元と同じグループに作成するために m_hwndTraceOutSource を使っています
		// （m_hwndTraceOutSource は CEditWnd::Create() で予め設定）
		// ちょっと不恰好だけど、TraceOut() の引数にいちいち起動元を指定するのも．．．
		// 2010.05.11 Moca m_hwndTraceOutSourceは依然として使っていますが引数にしました
		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = L"";
		// CODE_SJIS->CODE_UNICODE	2008/6/8 Uchi
		// CODE_UNICODE->CODE_NONE	2010.05.11 Moca デフォルト文字コードで設定できるように無指定に変更
		sLoadInfo.eCharCode = CODE_NONE;
		sLoadInfo.bViewMode = false;
		ret = CControlTray::OpenNewEditor( NULL, hwnd, sLoadInfo, L"-DEBUGMODE", true );
		//	2001/06/23 N.Nakatani 窓が出るまでウエイトをかけるように修正
		//アウトプットウインドウが出来るまで5秒ぐらい待つ。
		//	Jun. 25, 2001 genta OpenNewEditorの同期機能を利用するように変更
		bAllwaysActive = true; // 新しく作ったときはactive
	}
	/* 開いているウィンドウをアクティブにする */
	if(ret && bAllwaysActive){
		ActivateFrameWindow( m_pShareData->m_sHandles.m_hwndDebug );
	}
	return ret;
}

/*!
	設定フォルダーがEXEフォルダーと別かどうかを返す

	iniファイルの保存先がユーザー別設定フォルダーかどうか 2007.05.25 ryoji
*/
[[nodiscard]]  bool CShareData::IsPrivateSettings( void ) const noexcept
{
	return m_pShareData != nullptr && 0 != ::wcscmp(m_pShareData->m_szPrivateIniFile, m_pShareData->m_szIniFile);
}

/*
	CShareData::CheckMRUandOPENFOLDERList
	MRUとOPENFOLDERリストの存在チェックなど
	存在しないファイルやフォルダーはMRUやOPENFOLDERリストから削除する

	@note 現在は使われていないようだ。
	@par History
	2001.12.26 削除した。（YAZAKI）
	
*/
/*!	idxで指定したマクロファイル名（フルパス）を取得する．

	@param pszPath [in]	パス名の出力先．長さのみを知りたいときはNULLを入れる．
	@param idx [in]		マクロ番号
	@param nBufLen [in]	pszPathで指定されたバッファのバッファサイズ

	@retval >0 : パス名の長さ．
	@retval  0 : エラー，そのマクロは使えない，ファイル名が指定されていない．
	@retval <0 : バッファ不足．必要なバッファサイズは -(戻り値)+1

	@author YAZAKI
	@date 2003.06.08 Moca ローカル変数へのポインタを返さないように仕様変更
	@date 2003.06.14 genta 文字列長，ポインタのチェックを追加
	@date 2003.06.24 Moca idxが-1のとき、キーマクロのフルパスを返す.
	
	@note idxは正確なものでなければならない。(内部で正当性チェックを行っていない)
*/
int CShareData::GetMacroFilename( int idx, WCHAR *pszPath, int nBufLen )
{
	if( -1 != idx && !m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].IsEnabled() )
		return 0;
	const WCHAR *ptr;
	const WCHAR *pszFile;

	if( -1 == idx ){
		pszFile = L"RecKey.mac";
	}else{
		pszFile = m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].m_szFile;
	}
	if( pszFile[0] == L'\0' ){	//	ファイル名が無い
		if( pszPath != NULL ){
			pszPath[0] = L'\0';
		}
		return 0;
	}
	ptr = pszFile;
	int nLen = wcslen( ptr ); // Jul. 21, 2003 genta wcslen対象が誤っていたためマクロ実行ができない

	if( !_IS_REL_PATH( pszFile )	// 絶対パス
		|| m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER[0] == L'\0' ){	//	フォルダー指定なし
		if( pszPath == NULL || nBufLen <= nLen ){
			return -nLen;
		}
		wcscpy( pszPath, pszFile );
		return nLen;
	}
	else {	//	フォルダー指定あり
		//	相対パス→絶対パス
		int nFolderSep = AddLastChar( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER, _countof2(m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER), L'\\' );
		int nAllLen;
		WCHAR *pszDir;
		WCHAR szDir[_MAX_PATH + _countof2( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER )];

		 // 2003.06.24 Moca フォルダーも相対パスなら実行ファイルからのパス
		// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
		if( _IS_REL_PATH( m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER ) ){
			GetInidirOrExedir( szDir, m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER );
			pszDir = szDir;
		}else{
			pszDir = m_pShareData->m_Common.m_sMacro.m_szMACROFOLDER;
		}

		int nDirLen = wcslen( pszDir );
		nAllLen = nDirLen + nLen + ( -1 == nFolderSep ? 1 : 0 );
		if( pszPath == NULL || nBufLen <= nAllLen ){
			return -nAllLen;
		}

		wcscpy( pszPath, pszDir );
		WCHAR *ptr = pszPath + nDirLen;
		if( -1 == nFolderSep ){
			*ptr++ = L'\\';
		}
		wcscpy( ptr, pszFile );
		return nAllLen;
	}
}

/*!	idxで指定したマクロのm_bReloadWhenExecuteを取得する。
	idxは正確なものでなければならない。
	YAZAKI
*/
bool CShareData::BeReloadWhenExecuteMacro( int idx )
{
	if( !m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].IsEnabled() )
		return false;

	return m_pShareData->m_Common.m_sMacro.m_MacroTable[idx].m_bReloadWhenExecute;
}

/*!	@brief 共有メモリ初期化/ツールバー

	ツールバー関連の初期化処理

	@author genta
	@date 2005.01.30 genta CShareData::Init()から分離．
		一つずつ設定しないで一気にデータ転送するように．
*/
CommonSetting_ToolBar::CommonSetting_ToolBar() noexcept
{
		/* ツールバーボタン構造体 */
//Sept. 16, 2000 JEPRO
//	CShareData_new2.cppでできるだけ系ごとに集まるようにアイコンの順番を大幅に入れ替えたのに伴い以下の初期設定値を変更
	// 2010.06.26 Moca 内容は CMenuDrawer::FindToolbarNoFromCommandId の戻り値です
	static const int DEFAULT_TOOL_BUTTONS[] = {
		1,	//新規作成
		25,		//ファイルを開く(DropDown)
		3,		//上書き保存		//Sept. 16, 2000 JEPRO 3→11に変更	//Oct. 25, 2000 11→3
		4,		//名前を付けて保存	//Sept. 19, 2000 JEPRO 追加
		0,

		33,	//元に戻す(Undo)	//Sept. 16, 2000 JEPRO 7→19に変更	//Oct. 25, 2000 19→33
		34,	//やり直し(Redo)	//Sept. 16, 2000 JEPRO 8→20に変更	//Oct. 25, 2000 20→34
		0,

		87,	//移動履歴: 前へ	//Dec. 24, 2000 JEPRO 追加
		88,	//移動履歴: 次へ	//Dec. 24, 2000 JEPRO 追加
		0,

		225,	//検索		//Sept. 16, 2000 JEPRO 9→22に変更	//Oct. 25, 2000 22→225
		226,	//次を検索	//Sept. 16, 2000 JEPRO 16→23に変更	//Oct. 25, 2000 23→226
		227,	//前を検索	//Sept. 16, 2000 JEPRO 17→24に変更	//Oct. 25, 2000 24→227
		228,	//置換		// Oct. 7, 2000 JEPRO 追加
		229,	//検索マークのクリア	//Sept. 16, 2000 JEPRO 41→25に変更(Oct. 7, 2000 25→26)	//Oct. 25, 2000 25→229
		230,	//Grep		//Sept. 16, 2000 JEPRO 14→31に変更	//Oct. 25, 2000 31→230
		0,

		232,	//アウトライン解析	//Dec. 24, 2000 JEPRO 追加
		0,

		264,	//タイプ別設定一覧	//Sept. 16, 2000 JEPRO 追加
		265,	//タイプ別設定		//Sept. 16, 2000 JEPRO 18→36に変更	//Oct. 25, 2000 36→265
		266,	//共通設定			//Sept. 16, 2000 JEPRO 10→37に変更 説明を「設定プロパティシート」から変更	//Oct. 25, 2000 37→266
		0,		//Oct. 8, 2000 jepro 次行のために追加
		346,	//コマンド一覧	//Oct. 8, 2000 JEPRO 追加
	};

	//	ツールバーアイコン数の最大値を超えないためのおまじない
	//	最大値を超えて定義しようとするとここでコンパイルエラーになります．
	char dummy[ _countof(DEFAULT_TOOL_BUTTONS) < MAX_TOOLBAR_BUTTON_ITEMS ? 1:0 ];
	dummy[0]=0;

	memcpy_raw(
		m_nToolBarButtonIdxArr,
		DEFAULT_TOOL_BUTTONS,
		sizeof(DEFAULT_TOOL_BUTTONS)
	);

	/* ツールバーボタンの数 */
	m_nToolBarButtonNum = _countof(DEFAULT_TOOL_BUTTONS);
	m_bToolBarIsFlat = !IsVisualStyle();			/* フラットツールバーにする／しない */	// 2006.06.23 ryoji ビジュアルスタイルでは初期値をノーマルにする
}

/*!	@brief 共有メモリ初期化/ポップアップメニュー

	ポップアップメニューの初期化処理

	@date 2005.01.30 genta CShareData::Init()から分離．
*/
CommonSetting_CustomMenu::CommonSetting_CustomMenu() noexcept
{
	/* カスタムメニュー 規定値 */

	for( int i = 0; i < MAX_CUSTOM_MENU; ++i ){
		m_szCustMenuNameArr[i][0] = '\0';
		m_nCustMenuItemNumArr[i] = 0;
		for( int j = 0; j < MAX_CUSTOM_MENU_ITEMS; ++j ){
			m_nCustMenuItemFuncArr[i][j] = F_0;
			m_nCustMenuItemKeyArr [i][j] = '\0';
		}
		m_bCustMenuPopupArr[i] = true;
	}
	m_szCustMenuNameArr[CUSTMENU_INDEX_FOR_TABWND][0] = '\0';	//@@@ 2003.06.13 MIK
	
	/* 右クリックメニュー */
	int n = 0;
	m_nCustMenuItemFuncArr[0][n] = F_UNDO;
	m_nCustMenuItemKeyArr [0][n] = 'U';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_REDO;
	m_nCustMenuItemKeyArr [0][n] = 'R';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_0;
	m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_CUT;
	m_nCustMenuItemKeyArr [0][n] = 'T';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_COPY;
	m_nCustMenuItemKeyArr [0][n] = 'C';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_PASTE;
	m_nCustMenuItemKeyArr [0][n] = 'P';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_DELETE;
	m_nCustMenuItemKeyArr [0][n] = 'D';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_0;
	m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_COPY_CRLF;	//Nov. 9, 2000 JEPRO 「CRLF改行でコピー」を追加
	m_nCustMenuItemKeyArr [0][n] = 'L';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_COPY_ADDCRLF;
	m_nCustMenuItemKeyArr [0][n] = 'H';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_PASTEBOX;	//Nov. 9, 2000 JEPRO 「矩形貼り付け」を復活
	m_nCustMenuItemKeyArr [0][n] = 'X';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_0;
	m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_SELECTALL;
	m_nCustMenuItemKeyArr [0][n] = 'A';
	n++;

	m_nCustMenuItemFuncArr[0][n] = F_0;		//Oct. 3, 2000 JEPRO 以下に「タグジャンプ」と「タグジャンプバック」を追加
	m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_TAGJUMP;
	m_nCustMenuItemKeyArr [0][n] = 'G';		//Nov. 9, 2000 JEPRO 「コピー」とバッティングしていたアクセスキーを変更(T→G)
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_TAGJUMPBACK;
	m_nCustMenuItemKeyArr [0][n] = 'B';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_0;		//Oct. 15, 2000 JEPRO 以下に「選択範囲内全行コピー」と「引用符付きコピー」を追加
	m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_COPYLINES;
	m_nCustMenuItemKeyArr [0][n] = '@';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_COPYLINESASPASSAGE;
	m_nCustMenuItemKeyArr [0][n] = '.';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_0;
	m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_COPYFNAME;
	m_nCustMenuItemKeyArr [0][n] = 'F';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_COPYPATH;
	m_nCustMenuItemKeyArr [0][n] = '\\';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_COPYDIRPATH;
	m_nCustMenuItemKeyArr [0][n] = 'O';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_0;
	m_nCustMenuItemKeyArr [0][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_OPEN_FOLDER_IN_EXPLORER;
	m_nCustMenuItemKeyArr[0][n] = 'E';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_OPEN_COMMAND_PROMPT;
	m_nCustMenuItemKeyArr[0][n] = 'W';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_OPEN_COMMAND_PROMPT_AS_ADMIN;
	m_nCustMenuItemKeyArr[0][n] = 'w';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_OPEN_POWERSHELL;
	m_nCustMenuItemKeyArr[0][n] = 'P';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_OPEN_POWERSHELL_AS_ADMIN;
	m_nCustMenuItemKeyArr[0][n] = 'p';
	n++;
	m_nCustMenuItemFuncArr[0][n] = F_PROPERTY_FILE;
	m_nCustMenuItemKeyArr [0][n] = 'F';		//Nov. 9, 2000 JEPRO 「やり直し」とバッティングしていたアクセスキーを変更(R→F)
	n++;
	m_nCustMenuItemNumArr[0] = n;

	/* カスタムメニュー１ */
	m_nCustMenuItemNumArr[1] = 7;
	m_nCustMenuItemFuncArr[1][0] = F_FILEOPEN;
	m_nCustMenuItemKeyArr [1][0] = 'O';		//Sept. 14, 2000 JEPRO できるだけ標準設定値に合わせるように変更 (F→O)
	m_nCustMenuItemFuncArr[1][1] = F_FILESAVE;
	m_nCustMenuItemKeyArr [1][1] = 'S';
	m_nCustMenuItemFuncArr[1][2] = F_NEXTWINDOW;
	m_nCustMenuItemKeyArr [1][2] = 'N';		//Sept. 14, 2000 JEPRO できるだけ標準設定値に合わせるように変更 (O→N)
	m_nCustMenuItemFuncArr[1][3] = F_TOLOWER;
	m_nCustMenuItemKeyArr [1][3] = 'L';
	m_nCustMenuItemFuncArr[1][4] = F_TOUPPER;
	m_nCustMenuItemKeyArr [1][4] = 'U';
	m_nCustMenuItemFuncArr[1][5] = F_0;
	m_nCustMenuItemKeyArr [1][5] = '\0';
	m_nCustMenuItemFuncArr[1][6] = F_WINCLOSE;
	m_nCustMenuItemKeyArr [1][6] = 'C';

	/* タブメニュー */	//@@@ 2003.06.14 MIK
	n = 0;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILESAVE;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'S';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILESAVEAS_DIALOG;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'A';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILECLOSE;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'R';	// 2007.06.26 ryoji B -> R
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILECLOSE_OPEN;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'L';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_WINCLOSE;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'C';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILE_REOPEN;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'W';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_COPYFNAME;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'F';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_COPYPATH;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_COPYDIRPATH;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_OPEN_FOLDER_IN_EXPLORER; 
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'F';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_OPEN_COMMAND_PROMPT;
	m_nCustMenuItemKeyArr[CUSTMENU_INDEX_FOR_TABWND][n]  = 'W';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_OPEN_COMMAND_PROMPT_AS_ADMIN;
	m_nCustMenuItemKeyArr[CUSTMENU_INDEX_FOR_TABWND][n]  = 'w';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_OPEN_POWERSHELL;
	m_nCustMenuItemKeyArr[CUSTMENU_INDEX_FOR_TABWND][n]  = 'P';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_OPEN_POWERSHELL_AS_ADMIN;
	m_nCustMenuItemKeyArr[CUSTMENU_INDEX_FOR_TABWND][n]  = 'p';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_GROUPCLOSE;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'G';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSEOTHER;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'O';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSELEFT;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'H';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSERIGHT;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'M';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_MOVERIGHT;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '0';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_MOVELEFT;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '1';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_SEPARATE;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'E';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_JOINTNEXT;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'X';
	n++;
	m_nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_JOINTPREV;
	m_nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'V';
	n++;
	m_nCustMenuItemNumArr[CUSTMENU_INDEX_FOR_TABWND] = n;
}

/* 言語選択後に共有メモリ内の文字列を更新する */
void CShareData::RefreshString()
{
	RefreshKeyAssignString( m_pShareData );
}

void CShareData::CreateTypeSettings()
{
	if( NULL == m_pvTypeSettings ){
		m_pvTypeSettings = new std::vector<STypeConfig*>();
	}
}

std::vector<STypeConfig*>& CShareData::GetTypeSettings()
{
	return *m_pvTypeSettings;
}

SFileTree::SFileTree() noexcept
{
	m_nItemCount = 1;
	m_aItems[0].m_szTargetPath = L".";
	m_aItems[0].m_szTargetFile = L"*.*";
}
