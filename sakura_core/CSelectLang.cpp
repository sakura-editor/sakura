/*!	@file
	@brief 各国語メッセージリソース対応

	@author nasukoji
	@date 2011.04.10	新規作成
*/
/*
	Copyright (C) 2011, nasukoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CSelectLang.h"

#include "_main/CProcess.h"

//! メッセージリソース用コンストラクタ
CSelectLang::SSelLangInfo::SSelLangInfo(const std::filesystem::path& path)
	: m_Path(path)
{
}

CSelectLang::SSelLangInfo::~SSelLangInfo() noexcept = default;

/*!
 * リソースDLLを読み込む
 *
 * @retval true 成功
 * @retval false 失敗
 * @throw std::out_of_range リソースDLLが使えなかった場合
 */
bool CSelectLang::SSelLangInfo::Load()
{
	if (m_Path.empty()) {
		return true;
	}

	const auto hModule = ::LoadLibraryExW(m_Path.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
	if( !hModule ){
		return false;
	}

	// 言語名を取得
	m_LangName = cxx::load_string(STR_SELLANG_NAME, hModule);
	if (m_LangName.empty()) {
		throw std::out_of_range("missing language name!");
	}

	// 言語IDを取得 "0x" + 4桁
	std::wstring langIdStr{ cxx::load_string(STR_SELLANG_LANGID, hModule) };
	if (langIdStr.empty()) {
		throw std::out_of_range("missing language id!");
	}

	if (const auto langId = static_cast<WORD>(std::stoi(langIdStr, nullptr, 16)); langId != m_LangId) {
		throw std::out_of_range(std::format("unexpected language id: {}, expected: {}", langId, m_LangId));
	}

	m_Module = hModule;

	return true;
}

void CSelectLang::SSelLangInfo::Unload() noexcept
{
	m_Module = nullptr;
}

/*!
	@brief メッセージリソースDLLのインスタンスハンドルを返す

	@retval メッセージリソースDLLのインスタンスハンドル

	@note メッセージリソースDLLをロードしていない場合exeのインスタンスハンドルが返る

	@date 2011.04.10 nasukoji	新規作成
*/
/* static */ HMODULE CSelectLang::getLangRsrcInstance() noexcept
{
	return !gm_Langs.empty() && gm_Selected ? HMODULE(gm_Langs[gm_Selected]->m_Module) : GetAppInstance();
}

/*!
	@brief メッセージリソースDLL未読み込み時のデフォルト言語の文字列を返す

	@retval デフォルト言語の文字列（"(Japanese)" または "(English(United States))"）

	@note アプリケーションリソースより読み込んだ "(Japanese)" または "(English(United States))"

	@date 2011.04.10 nasukoji	新規作成
*/
/* static */ LPCWSTR CSelectLang::getDefaultLangString() noexcept
{
	return !gm_Langs.empty() && gm_Selected ? gm_Langs[gm_Selected]->m_LangName.c_str() : LS(STR_SELLANG_NAME);
}

// 言語IDを返す
/* static */ WORD CSelectLang::getDefaultLangId() noexcept
{
	return !gm_Langs.empty() && gm_Selected ? gm_Langs[gm_Selected]->m_LangId : MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
}

/*!
	@brief 言語環境を初期化する
	
	@retval メッセージリソースDLLのインスタンスハンドル

	@note メッセージリソースDLLが未指定、または読み込みエラー発生の時はexeのインスタンスハンドルが返る
	@note （LoadString()の引数としてそのまま使用するため）
	@note デフォルト言語の文字列の読み込みも行う
	@note プロセス毎にProcessFactoryの最初に1回だけ呼ばれる

	@date 2011.04.10 nasukoji	新規作成
*/
/* static */ HMODULE CSelectLang::InitializeLanguageEnvironment()
{
	// 言語情報をクリアする
	gm_Langs.clear();

	// デフォルト情報を作成する
	gm_Langs.emplace_back(std::make_unique<SSelLangInfo>());
	gm_Langs.back()->m_LangId = MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
	gm_Langs.back()->m_LangName = cxx::load_string(STR_SELLANG_NAME);

	// 言語DLLの検索パターンを生成する
	const auto searchPattern = GetExeFileName().replace_filename(L"sakura_lang_*.dll");

	// 言語DLLの検索を開始する
	WIN32_FIND_DATA w32fd{};
	HANDLE hFindFile = ::FindFirstFileW(searchPattern.c_str(), &w32fd);

	// 検索を開始できなかった場合
	if (INVALID_HANDLE_VALUE == hFindFile) {
		return getLangRsrcInstance();
	}

	// 検索ハンドルをスマートポインタに入れる
	using FindFileHolder = cxx::ResourceHolder<&::FindClose>;
	FindFileHolder hFindFileHolder{ hFindFile };

	do {
		// ディレクトリーはスキップ
		if (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}

		// 言語タグを取得
		LCID lcid;
		if (std::wcmatch m; std::regex_match(w32fd.cFileName, m, std::wregex(LR"(^sakura_lang_([a-z]{2,3})[_\-]([A-Z]{2})\.dll$)"))) {
			// ロケール名を抽出して LCID を得る
			const std::wstring localeName = std::wstring(m[1]) + L'-' + std::wstring(m[2]);
			lcid = LocaleNameToLCID(localeName.c_str(), 0);
		}
		else {
			// ファイル名が不正ならスキップ
			continue;
		}

		// バッファに登録する。
		gm_Langs.emplace_back(std::make_unique<SSelLangInfo>(w32fd.cFileName));

		// 言語IDを設定
		gm_Langs.back()->m_LangId = LANGIDFROMLCID(lcid);

		try {
			// リソースDLLを読み込む
			if (!gm_Langs.back()->Load()) {
				// ロードできなかったらスキップ
				gm_Langs.pop_back();
				continue;
			}
		}
		catch (const std::out_of_range&) {
			// 必要なリソースを読めなかったらスキップ
			gm_Langs.pop_back();
			continue;
		}

		// ここでは読み込みチェックだけなので、アンロードしておく。
		gm_Langs.back()->Unload();
	}
	while (::FindNextFileW(hFindFile, &w32fd));

	// この時点ではexeのインスタンスハンドルで起動し、共有メモリ初期化後にChangeLangする

	return getLangRsrcInstance();
}

//! 言語を変更する
/* static */ void CSelectLang::ChangeLang(const std::filesystem::path& dllName)
{
	size_t index;

	if (dllName.empty()) {
		// デフォルト言語に戻す
		index = 0;
	}
	else if(const auto found = std::ranges::find_if(
		gm_Langs,
		[&dllName](const auto& langInfo) {
			return langInfo->m_Path == dllName;
		}
	); found != gm_Langs.end()) {
		// 言語を選択する
		index = std::distance(gm_Langs.begin(), found);
	}
	else {
		// 指定されたリソースDLLがリストになかった場合
		return;
	}

	ChangeLang(index);
}

//! 言語を変更する
/* static */ void CSelectLang::ChangeLang(size_t nIndex)
{
	// 現在選択されている言語を保存
	const auto oldSelected = gm_Selected;

	// 変更がない、または、インデックスが範囲外なら何もせず抜ける
	if (nIndex == oldSelected || std::size(gm_Langs) <= nIndex) {
		return;
	}

	// 新しい言語がリソースDLLの場合、リソースDLLを読み込む
	if (nIndex) {
		gm_Langs[nIndex]->Load();
	}

	// 選択中の言語を更新
	gm_Selected = nIndex;

	// アプリ名をリソースから読み込む
	if (auto pcProcess = CProcess::getInstance()) {
		pcProcess->UpdateAppName(LoadStringW(STR_GSTR_APPNAME));
	}

	// ロケールを設定
	::SetThreadUILanguage(getDefaultLangId());

	// 変更前言語がリソースDLLの場合、リソースDLLをアンロードする
	if (oldSelected) {
		gm_Langs[oldSelected]->Unload();
	}
}

/* static */ std::wstring_view CSelectLang::LoadStringW(UINT id)
{
	const auto optModule = gm_Selected ? std::optional<HMODULE>(HMODULE(gm_Langs[gm_Selected]->m_Module)) : std::nullopt;
	return cxx::load_string(id, optModule);
}

/*!
	@brief 静的バッファに文字列リソースを読み込む（各国語メッセージリソース対応）

	@param[in] uid リソースID

	@retval 読み込んだ文字列（文字列無しの時 "" が返る）

	@note 静的バッファ（m_acLoadStrBufferTemp[?]）に文字列リソースを読み込む。
	@note バッファは複数準備しているが、呼び出す毎に更新するのでバッファ個数を
	@note 超えて呼び出すと順次内容が失われていく。
	@note 呼び出し直後での使用や関数の引数などでの使用を想定しており、前回値を
	@note 取り出すことはできない。
	@note 使用例）::SetWindowText( m_hWnd, CLoadString::LoadStringSt(STR_ERR_DLGSMCLR1) );
	@note アプリケーション内の関数への引数とする場合、その関数が本関数を使用
	@note しているか意識する必要がある（上限を超えれば内容が更新されるため）
	@note 内容を保持したい場合は CLoadString::LoadString() を使用する。

	@date 2011.06.01 nasukoji	新規作成
*/
LPCWSTR CLoadString::LoadStringSt(UINT uid)
{
	// 使用するバッファの現在位置を進める
	++gm_LastUsedIndex;

	if (std::size(gm_Buffers) <= gm_LastUsedIndex) {
		gm_LastUsedIndex = 0;
	}

	auto& workString = gm_Buffers[gm_LastUsedIndex];

	workString.LoadStringW(uid);

	return workString.GetStringPtr();
}

/*!
	@brief 文字列リソースを読み込む（各国語メッセージリソース対応）

	@param[in] uid リソースID

	@retval 読み込んだ文字列（文字列無しの時 "" が返る）

	@note メンバ変数内に記憶されるため  CLoadString::LoadStringSt() の様に
	@note 不用意に破壊されることはない。
	@note ただし、変数を準備する必要があるのが不便。
	@note 使用例）
	@note   CLoadString cStr[2];
	@note   cDlgInput1.DoModal( m_hInstance, m_hWnd,
	@note       cStr[0].LoadString(STR_ERR_DLGPRNST1),
	@note       cStr[1].LoadString(STR_ERR_DLGPRNST2),
	@note       sizeof( m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName ) - 1, szWork ) )

	@date 2011.06.01 nasukoji	新規作成
*/
LPCWSTR CLoadString::LoadStringW(UINT uid)
{
	m_Buffer.LoadStringW(uid);

	return m_Buffer.GetStringPtr();
}

/*!
	@brief 文字列リソースを読み込む（読み込み実行部）

	@param[in] uid  リソースID

	@retval 読み込んだ文字数（WCHAR単位）

	@note メッセージリソースより文字列を読み込む。メッセージリソースDLLに指定の
	@note リソースが存在しない、またはメッセージリソースDLL自体が読み込まれて
	@note いない場合、内部リソースより文字列を読み込む。
	@note 最初は静的バッファに読み込むがバッファ不足となったらバッファを拡張
	@note して読み直す。
	@note 取得したバッファはデストラクタで解放する。
	@note ANSI版は2バイト文字の都合により（バッファ - 2）バイトまでしか読まない
	@note 場合があるので1バイト少ない値でバッファ拡張を判定する。

	@date 2011.06.01 nasukoji	新規作成
*/
size_t CLoadString::CLoadStrBuffer::LoadStringW(UINT uid)
{
	// バッファをクリア
	if (!m_String.empty()) {
		m_String.clear();
		m_ResString = L"";
	}

	try {
		// 選択されたリソースDLLからの取得を試みる
		m_ResString = CSelectLang::LoadStringW(uid);
	}
	catch(const std::out_of_range&){
		// 存在しない場合はアプリリソースから取得する
		m_ResString = cxx::load_string(uid);
	}

	m_String = m_ResString;

	return m_String.length();
}
