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

#include <fstream>

std::filesystem::path GetTempFilePathWithExt(std::wstring_view prefix, std::wstring_view extension);

namespace macro {

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
