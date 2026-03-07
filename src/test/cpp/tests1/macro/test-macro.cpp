/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include "macro/CKeyMacroMgr.h"
#include "macro/CMacroFactory.h"
#include "macro/CPPAMacroMgr.h"
#include "macro/CPythonMacroManager.h"
#include "macro/CWSHManager.h"

#include "window/EditorTestSuite.hpp"

#include "util/module.h"

#include <fstream>

std::filesystem::path GetTempFilePathWithExt(std::wstring_view prefix, std::wstring_view extension);

namespace macro {

std::filesystem::path find_dll_in_the_path(const std::filesystem::path& dllname)
{
	std::wstring envPath(std::numeric_limits<short>::max() + 1, L'\0');
	if (const auto ret = ::GetEnvironmentVariableW(L"PATH", std::data(envPath), DWORD(std::size(envPath)))) {
		envPath.resize(ret);
	}

	std::wstringstream ss(envPath);
	std::wstring dir;

	while (std::getline(ss, dir, L';')) {
		const auto p = std::filesystem::path{ dir };
		if (!fexist(p / dllname)) {
			continue;
		}
		std::wcout << L"dll found: '" << (p / dllname) << L"'" << std::endl;

		using DllHolder = cxx::ResourceHolder<&::FreeLibrary>;
		if (DllHolder dllHolder = LoadLibraryExedir((p / dllname).c_str()); !dllHolder) {
			//エラーコード取得
			const auto err = ::GetLastError();

			LPWSTR msg = nullptr;
			::FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				err,
				0,
				LPWSTR(&msg),
				0,
				nullptr
			);

			std::wcout << L"fail to load: '" << (p / dllname) << L"' caused by '" << msg << L"'" << std::endl;

			::LocalFree(msg);

			continue;
		}

		return p;
	}

	return "";
}

struct MacroMgrTest : public ::testing::Test, public window::EditorTestSuite {
	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		SetUpEditor();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		TearDownEditor();
	}
};

/*!
 * キーマクロマネージャーのテスト
 */
TEST_F(MacroMgrTest, CKeyMacroMgr001)
{
	EXPECT_THAT(CKeyMacroMgr::Creator(L"ppa"), IsNull());

	CKeyMacroMgr::declare();

	auto mgr = std::unique_ptr<CMacroManagerBase>(CMacroFactory::getInstance()->Create(L"mac"));

	const HINSTANCE unusedArg1 = nullptr;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L""), IsTrue());

	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	const auto path = GetTempFilePathWithExt(L"tes", L"mac");
	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsFalse());

	std::wofstream fs(path);
	fs << L" \t" << std::endl;
	fs << L"// comment." << std::endl;
	fs << L"S_ChgmodINS()" << std::endl;
	fs << L"S_ChgmodINS( 8 )" << std::endl;
	fs << L"S_ChgmodINS( 'dummy' )" << std::endl;
	fs.close();

	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsTrue());

	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	std::filesystem::remove(path);

	// 保存するだけ
	EXPECT_THAT(((CKeyMacroMgr*)mgr.get())->SaveKeyMacro(unusedArg1, path.c_str()), IsTrue());

	std::filesystem::remove(path);

	// 引数が足りない
	fs = std::wofstream(path);
	fs << L"S_Char()" << std::endl;
	fs.close();

	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsTrue());

	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsFalse());

	std::filesystem::remove(path);

	// 構文エラー
	fs = std::wofstream(path);
	fs << L"S_ChgmodINS( , )" << std::endl;
	fs.close();

	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsFalse());

	std::filesystem::remove(path);

	// 未定義のマクロコマンド
	fs = std::wofstream(path);
	fs << L"UnDefinedCommand()" << std::endl;
	fs.close();

	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsFalse());

	std::filesystem::remove(path);

	mgr = nullptr;

	CMacroFactory::getInstance()->Unregister(CKeyMacroMgr::Creator);

	std::array<LPARAM, 4> lparams = {};
	pcSMacroMgr->Append(STAND_KEYMACRO, F_CHGMOD_INS, std::data(lparams), &pcEditWnd->GetActiveView());
}

/*!
 * CPPAマクロマネージャーのテスト
 */
TEST_F(MacroMgrTest, CPPAMacroMgr001)
{
	EXPECT_THAT(CPPAMacroMgr::Creator(L"mac"), IsNull());

	// スタブDLLを読み込む
	CPPAMacroMgr::m_cPPA.InitDll(L"ppa_stub.dll");

	CPPAMacroMgr::declare();

	auto mgr = std::unique_ptr<CMacroManagerBase>(CMacroFactory::getInstance()->Create(L"ppa"));

	const HINSTANCE unusedArg1 = nullptr;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"macro str;"), IsTrue());

	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	const auto path = GetTempFilePathWithExt(L"tes", L"ppa");
	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsFalse());

	std::wofstream fs(path);
	fs << L"macro str1;" << std::endl;
	fs << L"macro str2;" << std::endl;
	fs << L"macro str3;" << std::endl;
	fs.close();

	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsTrue());

	std::filesystem::remove(path);

	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	mgr = nullptr;

	CMacroFactory::getInstance()->Unregister(CPPAMacroMgr::Creator);
}

/*!
 * Pythonマクロマネージャーのテスト
 */
TEST_F(MacroMgrTest, CPythonMacroManager001)
{
	EXPECT_THAT(CPythonMacroManager::Creator(L"mac"), IsNull());

	CPythonMacroManager::declare();

	auto mgr = std::unique_ptr<CMacroManagerBase>(CMacroFactory::getInstance()->Create(L"py"));

	// DLLパスをバックアップする
	const auto pathOld = GetDllShareData().m_Common.m_sMacro.m_szPythonDirectory;

	std::wcout << L"dll path is '" << pathOld.c_str() << L"'" << std::endl;

	// 不正なDLLのパスを作る
	const auto badDllPath = GetExeFileName().replace_filename(L"python3.dll");

	// 不正なDLLなDLLを作る
	std::wofstream fs(badDllPath);
	fs << L"dummy dll file" << std::endl;
	fs.close();

	// DLLパスに不正なDLLのパスを入れる
	GetDllShareData().m_Common.m_sMacro.m_szPythonDirectory = badDllPath.parent_path();

	const HINSTANCE unusedArg1 = nullptr;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L""), IsTrue());

	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsFalse());

	std::filesystem::remove(badDllPath);

	// x86_64環境であれば、実際に python3.dll を読み込むテストを行う
#ifdef _M_AMD64

	// DLLを解放する
	CPythonMacroManager::s_hModule = nullptr;

	// PATH環境変数からDLLを探す
	const auto python3Path = find_dll_in_the_path("python3.dll");
	GetDllShareData().m_Common.m_sMacro.m_szPythonDirectory = python3Path;

	::SetDllDirectoryW(python3Path.c_str());

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L""), IsTrue());

	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	const auto path = GetTempFilePathWithExt(L"tes", L"py");
	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsFalse());

	fs = std::wofstream(path);
	fs << L"import SakuraEditor" << std::endl;
	fs << L"# comment." << std::endl;
	fs << L"SakuraEditor.ExpandParameter('$i')" << std::endl;
	fs << L"SakuraEditor.ChgmodINS()" << std::endl;
	fs.close();

	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsTrue());

	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	std::filesystem::remove(path);

	::SetDllDirectoryW(L"");

#endif // _M_AMD64

	// DLLパスを元に戻す
	GetDllShareData().m_Common.m_sMacro.m_szPythonDirectory = pathOld;

	mgr = nullptr;

	CMacroFactory::getInstance()->Unregister(CPythonMacroManager::Creator);
}

/*!
 * WSHマクロマネージャーのテスト
 */
TEST_F(MacroMgrTest, CWSHMacroManager001)
{
	EXPECT_THAT(CWSHMacroManager::Creator(L"mac"), IsNull());

	CWSHMacroManager::declare();

	auto mgr = std::unique_ptr<CMacroManagerBase>(CMacroFactory::getInstance()->Create(L"js"));

	const HINSTANCE unusedArg1 = nullptr;

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L""), IsTrue());

	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	const auto path = GetTempFilePathWithExt(L"tes", L"js");
	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsFalse());

	std::wofstream fs(path);
	fs << L"" << std::endl;
	fs << L"// comment." << std::endl;
	fs << L"ChgmodINS(8);" << std::endl;
	fs.close();

	EXPECT_THAT(mgr->LoadKeyMacro(unusedArg1, path.c_str()), IsTrue());

	std::filesystem::remove(path);

	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	mgr = nullptr;

	CMacroFactory::getInstance()->Unregister(CWSHMacroManager::Creator);
}

TEST(CSMacroMgr, GetFuncInfoByID001)
{
	EXPECT_THAT(CSMacroMgr::GetFuncInfoByID(F_FILENEW), NotNull());
}

TEST(CSMacroMgr, GetFuncInfoByID101)
{
	EXPECT_THAT(CSMacroMgr::GetFuncInfoByID(F_INVALID), IsNull());
}

TEST(CSMacroMgr, GetFuncInfoByID102)
{
	EXPECT_THAT(CSMacroMgr::GetFuncInfoByID(F_0), IsNull());
}

TEST(CSMacroMgr, GetFuncInfoByName001)
{
	EXPECT_THAT(CSMacroMgr::GetFuncInfoByName(nullptr, L"S_FileNew", nullptr), F_FILENEW);
}

TEST(CSMacroMgr, GetFuncInfoByName002)
{
	EXPECT_THAT(CSMacroMgr::GetFuncInfoByName(nullptr, L"ChangeInsertSpace", nullptr), F_CHGINSSPACE);
}

TEST(CSMacroMgr, GetFuncInfoByName101)
{
	EXPECT_THAT(CSMacroMgr::GetFuncInfoByName(nullptr, nullptr, nullptr), F_INVALID);
}

} // namespace macro
