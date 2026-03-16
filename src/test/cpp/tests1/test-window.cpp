/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "util/window.h"

#include "env/ShareDataTestSuite.hpp"
#include "window/EditorTestSuite.hpp"

#include "dlg/CDlgCancel.h"
#include "macro/CKeyMacroMgr.h"
#include "macro/CMacroFactory.h"

#include "_main/CControlTray.h"

#include "config/system_constants.h"

namespace window {

struct TrayWndTest : public ::testing::Test, public env::ShareDataTestSuite {
	using CControlTrayHolder = std::unique_ptr<CControlTray>;

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		SetUpShareData();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		TearDownShareData();
	}

	CControlTrayHolder pcTrayWnd = nullptr;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// テストクラスをインスタンス化する
		pcTrayWnd = std::make_unique<CControlTray>();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// テストクラスのインスタンスを破棄する
		pcTrayWnd = nullptr;
	}
};

TEST_F(TrayWndTest, OnGetTypeSetting001)
{
	// 受け取りバッファに値を設定
	GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_nIdx = int(-1);

	HWND hWndTray = nullptr;
	int index = 0;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_GET_TYPESETTING, index, 0), IsTrue());

	// 結果確認
	EXPECT_THAT(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_nIdx, Eq(index));
}

TEST_F(TrayWndTest, OnGetTypeSetting102)
{
	// 取得の空振り(上限値オーバー)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_GET_TYPESETTING, GetDllShareData().m_nTypesCount, 0), IsFalse());
}

TEST_F(TrayWndTest, OnAddTypeSetting001)
{
	// テキストの前に追加
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_ADD_TYPESETTING, 1, 0), IsTrue());
}

TEST_F(TrayWndTest, OnAddTypeSetting002)
{
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_GET_TYPESETTING, 1, 0), IsTrue());

	auto typeName = std::format(L"設定{}", 2);
	if (typeName != GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName) {
		// 受け渡しバッファに値を設定
		::wcscpy_s(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, typeName.c_str());

		EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_SET_TYPESETTING, 1, 0), IsTrue());
	}

	// 重複する名前を追加
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_ADD_TYPESETTING, 1, 0), IsTrue());

	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_GET_TYPESETTING, 1, 0), IsTrue());

	typeName = std::format(L"設定{}", 3);
	EXPECT_THAT(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, StrEq(typeName));
}

TEST_F(TrayWndTest, OnAddTypeSetting101)
{
	// 追加の空振り(基本の前には入れない)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_ADD_TYPESETTING, 0, 0), IsFalse());
}

TEST_F(TrayWndTest, OnAddTypeSetting102)
{
	// 追加の空振り(上限値オーバー、「指定したインデックスの前」なので他と上限が違う)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_ADD_TYPESETTING, GetDllShareData().m_nTypesCount + 1, 0), IsFalse());
}

TEST_F(TrayWndTest, OnAddTypeSetting103)
{
	// 元の数をバックアップして最大数にする
	const auto defaultCount = GetDllShareData().m_nTypesCount;
	GetDllShareData().m_nTypesCount = int(MAX_TYPES);

	// 追加の空振り(もう追加できない)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_ADD_TYPESETTING, GetDllShareData().m_nTypesCount - 1, 0), IsFalse());

	// 数を元に戻す
	GetDllShareData().m_nTypesCount = defaultCount;
}

TEST_F(TrayWndTest, OnSetTypeSetting001)
{
	// 受け渡しバッファに値を設定
	::wcscpy_s(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, L"テスト");

	// 更新してみる
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_SET_TYPESETTING, 1, 0), IsTrue());

	// 受け取りバッファに値を設定
	::wcscpy_s(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, L"");

	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_GET_TYPESETTING, 1, 0), IsTrue());

	EXPECT_THAT(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, StrEq(L"テスト"));
}

TEST_F(TrayWndTest, OnSetTypeSetting002)
{
	// 受け渡しバッファに値を設定
	::wcscpy_s(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, L"テスト");

	// 更新してみる
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_SET_TYPESETTING, 0, 0), IsTrue());

	// 受け取りバッファに値を設定
	::wcscpy_s(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, L"");

	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_GET_TYPESETTING, 0, 0), IsTrue());

	EXPECT_THAT(GetDllShareData().m_sWorkBuffer.m_TypeConfig.m_szTypeName, StrEq(L"テスト"));
}

TEST_F(TrayWndTest, OnSetTypeSetting102)
{
	// 更新の空振り(上限値オーバー)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_SET_TYPESETTING, GetDllShareData().m_nTypesCount, 0), IsFalse());
}

TEST_F(TrayWndTest, OnDelTypeSetting001)
{
	// 削除してみる
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_DEL_TYPESETTING, 8, 0), IsTrue());
}

TEST_F(TrayWndTest, OnDelTypeSetting101)
{
	// 削除の空振り(0は削除させない)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_DEL_TYPESETTING, 0, 0), IsFalse());
}

TEST_F(TrayWndTest, OnDelTypeSetting102)
{
	// 削除の空振り(上限値オーバー)
	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_DEL_TYPESETTING, GetDllShareData().m_nTypesCount, 0), IsFalse());
}

TEST_F(TrayWndTest, OnChangeSetting001)
{
	// 英語にする
	::wcscpy_s(GetDllShareData().m_Common.m_sWindow.m_szLanguageDll, L"sakura_lang_en_US.dll");

	HWND hWndTray = nullptr;
	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_CHANGESETTING, 0, int(PM_CHANGESETTING_ALL)), 0);

	// 日本語に戻す
	::wcscpy_s(GetDllShareData().m_Common.m_sWindow.m_szLanguageDll, L"");

	EXPECT_THAT(pcTrayWnd->DispatchEvent(hWndTray, MYWM_CHANGESETTING, 0, int(PM_CHANGESETTING_ALL)), 0);
}

struct EditWndTest : public ::testing::Test, public window::EditorTestSuite {
	static constexpr HINSTANCE unusedArg1 = nullptr;

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		SetUpEditor();

		CKeyMacroMgr::declare();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		CMacroFactory::getInstance()->Unregister(CKeyMacroMgr::Creator);

		TearDownEditor();
	}

	std::unique_ptr<CMacroManagerBase> mgr = nullptr;

	/*!
	 * テストが実行された直前に毎回呼ばれる関数
	 */
	void SetUp() override
	{
		mgr = std::unique_ptr<CMacroManagerBase>(CMacroFactory::getInstance()->Create(L"mac"));
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override
	{
		mgr = nullptr;
	}
};

/*!
 * キャンセルダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgCancel001)
{
	CDlgCancel cDlgCancel;

	const auto hWnd = pcEditWnd->GetHwnd();

	cDlgCancel.DoModeless(unusedArg1, hWnd, IDD_GREPRUNNING);

	cDlgCancel.CloseDialog(0);
}

/*!
 * 検索ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFind001)
{
	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"SearchDialog()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	pcEditWnd->m_cDlgFind.CloseDialog(0);
}

/*!
 * アウトライン解析ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgFuncList001)
{
	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"Outline(0)"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"Outline(1)"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	pcEditWnd->m_cDlgFuncList.CloseDialog(0);
}

/*!
 * 補完ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgHokan001)
{
	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"Complete()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	pcEditWnd->m_cHokanMgr.CloseDialog(0);
}

/*!
 * 置換ダイアログの表示テスト
 */
TEST_F(EditWndTest, ShowDlgReplace001)
{
	EXPECT_THAT(mgr->LoadKeyMacroStr(unusedArg1, L"ReplaceDialog()"), IsTrue());
	EXPECT_THAT(mgr->ExecKeyMacro(&pcEditWnd->GetActiveView(), 0), IsTrue());

	pcEditWnd->m_cDlgReplace.CloseDialog(0);
}

/*!
 * @brief CFontAutoDeleterのテスト
 */
TEST( CFontAutoDeleter, test )
{
	CFontAutoDeleter deleter;
	ASSERT_EQ(nullptr, deleter.GetFont());

	if (const auto hGdiFont = GetStockFont(DEFAULT_GUI_FONT)) {
		if (LOGFONT lf = {};
			::GetObject(hGdiFont, sizeof(lf), &lf)) {
			if (const auto hFont = ::CreateFontIndirect(&lf)) {
				deleter.SetFont(nullptr, hFont, nullptr);
				ASSERT_EQ(hFont, deleter.GetFont());
			}
		}
	}

	ASSERT_NE(nullptr, deleter.GetFont());
	if (const auto hFont = deleter.GetFont()) {
		CFontAutoDeleter other(deleter);
		ASSERT_NE(hFont, other.GetFont());

		other.ReleaseOnDestroy();
		ASSERT_EQ(nullptr, other.GetFont());

		CFontAutoDeleter another(std::move(deleter));
		ASSERT_EQ(hFont, another.GetFont());
		ASSERT_EQ(nullptr, deleter.GetFont());
	}
}

} // namespace window
