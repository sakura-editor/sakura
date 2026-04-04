/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "apiwrap/StdControl.h"

#include "cxx/ResourceHolder.hpp"
#include "dlg/CDialog.h"
#include "util/window.h"

#include "sakura_rc.h"

using namespace std::literals::string_literals;

/*!
 * 仮定義用namespace
 *
 * 実装に必要だが未定義の関数を、ここに仮定義する
 */
namespace ApiWrap {

/*!
 * @brief テキスト取得結果構造体
 */
struct SGetTextResult {
	bool success = false;	//!< テキストの取得に成功したか
	std::wstring text;		//!< 取得したテキスト

	//! デフォルトコンストラクタは失敗状態を表す
	SGetTextResult() = default;

	//! 成功状態を表すコンストラクタ
	explicit SGetTextResult(std::wstring&& result) noexcept
		: success(true)
		, text(std::move(result))
	{
	}

	/*!
	 * @brief 成功状態を返す変換演算子
	 *
	 * @retval true 取得成功。
	 * @note テキストが空でもtrue。
	 */
	explicit operator bool() const noexcept { return success; }

	/*!
	 * @brief 取得した文字列を返す変換演算子
	 *
	 * @note 失敗状態でも空文字列が返る。
	 */
	explicit operator const std::wstring& () const noexcept { return text; }
};

/*!
 * @brief 指定したウインドウのテキストを取得する
 */
SGetTextResult GetWindowTextW(HWND hWnd)
{
	if (std::wstring buffer; Wnd_GetText(hWnd, buffer)) {
		return SGetTextResult(std::move(buffer));
	}
	return SGetTextResult();
}

/*!
 * @brief 指定したウインドウのテキストを変更する
 */
bool SetWindowTextW(HWND hWnd, const std::wstring& text)
{
	return Wnd_SetText(hWnd, std::data(text));
}

/*!
 * @brief 指定したエディットコントロールの入力文字数を制限する
 */
template<std::ranges::sized_range T>
void LimitEditText(HWND hWnd, const T& buffer)
{
	EditCtl_LimitText(hWnd, std::size(buffer) - 1);
}

/*!
 * @brief リストボックスに項目を追加する
 */
template<std::ranges::input_range R>
void AddLbItems(HWND hWnd, const R& items)
{
	for (const auto& item : items) {
		ApiWrap::List_AddString(hWnd, std::data(item));
	}
}

/*!
 * @brief リストボックスの項目テキストを取得する
 *
 * @note このメソッドが必要な実装には、おそらく問題がある。
 */
SGetTextResult GetLbItemText(HWND hWnd, size_t index)
{
	if (std::wstring buffer; List_GetText(hWnd, static_cast<int>(index), buffer)) {
		return SGetTextResult(std::move(buffer));
	}
	return SGetTextResult();
}

/*!
 * @brief 指定したkコンボボックスの入力文字数を制限する
 */
template<std::ranges::sized_range T>
int LimitCbText(HWND hWnd, const T& buffer)
{
	return Combo_LimitText(hWnd, std::size(buffer) - 1);
}

/*!
 * @brief コンボボックスに項目を追加する
 */
template<std::ranges::input_range R>
void AddCbItems(HWND hWnd, const R& items)
{
	for (const auto& item : items) {
		ApiWrap::Combo_AddString(hWnd, std::data(item));
	}
}

/*!
 * @brief コンボボックスの項目テキストを取得する
 *
 * @note このメソッドが必要な実装には、おそらく問題がある。
 */
SGetTextResult GetCbItemText(HWND hWnd, size_t index)
{
	if (const auto itemLength = Combo_GetLBTextLen(hWnd, static_cast<int>(index)); 0 < itemLength) {
		if (std::wstring buffer(itemLength, L'\0'); Combo_GetLBText(hWnd, static_cast<int>(index), std::data(buffer))) {
			return SGetTextResult(std::move(buffer));
		}
	}
	return SGetTextResult();
}

/*!
 * @brief ダイアログボックス項目のテキストを取得する
 */
SGetTextResult GetDlgItemTextW(HWND hDlg, int nIdDlgItem)
{
	if (std::wstring buffer; DlgItem_GetText(hDlg, nIdDlgItem, buffer)) {
		return SGetTextResult(std::move(buffer));
	}
	return SGetTextResult();
}

/*!
 * @brief 指定したウインドウのテキストを変更する
 */
bool SetDlgItemTextW(HWND hDlg, int nIdDlgItem, const std::wstring& text)
{
	return DlgItem_SetText(hDlg, nIdDlgItem, std::data(text));
}

void CheckDlgButton(HWND hDlg, int nIDButton, bool bCheck = true)
{
	CheckDlgButtonBool(hDlg, nIDButton, bCheck);
}

bool IsDlgButtonChecked(HWND hDlg, int nIDButton)
{
	return IsDlgButtonCheckedBool(hDlg, nIDButton);
}

bool EnableDlgItem(HWND hDlg, int nIDDlgItem, bool enable = true)
{
	return DlgItem_Enable(hDlg, nIDDlgItem, enable);
}

} // namespace ApiWrap

namespace window {

using WindowHolder = cxx::ResourceHolder<&::DestroyWindow>;

/*!
 * ウインドウ全般を対象とするラッパーメソッドのテスト
 */
TEST(ApiWrap, WndTest001)
{
	EXPECT_THAT(ApiWrap::GetWindowTextW(nullptr), IsFalse());

	const auto expected = L"0123456789012345678901234567890123456789"s;

	const auto hInstance = ::GetModuleHandleW(nullptr);
	const auto hWnd = ::CreateWindowExW(0, WC_STATIC, std::data(expected), 0, 1, 1, 1, 1, HWND(nullptr), HMENU(nullptr), hInstance, nullptr);

	WindowHolder windowHolder{ hWnd };

	// 単純な取得
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), StrEq(expected));

	// GitHub #1528 の退行防止テストケース。
	// 取得する文字列の長さが basic_string::capacity と同じだった場合に一文字取りこぼしていた。
	std::wstring s(std::size(expected) - 1, L'\0');
	EXPECT_THAT(ApiWrap::Wnd_GetText(hWnd, s), IsTrue());
	EXPECT_THAT(s, StrEq(expected));

	ApiWrap::SetWindowTextW(hWnd, L"test"s);
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), StrEq(L"test"));

	CNativeW cmemText;
	EXPECT_THAT(ApiWrap::Wnd_GetText(hWnd, cmemText), IsTrue());
	EXPECT_THAT(cmemText.GetStringPtr(), StrEq(L"test"));
}

/*!
 * エディットコントロールを対象とするラッパーメソッドのテスト
 */
TEST(ApiWrap, EditCtlTest001)
{
	const auto hInstance = ::GetModuleHandleW(nullptr);
	const auto hWnd = ::CreateWindowExW(0, WC_EDIT, L"", 0, 1, 1, 1, 1, HWND(nullptr), HMENU(nullptr), hInstance, nullptr);

	WindowHolder windowHolder{ hWnd };

	// サイズ20の配列バッファを用意する
	StaticString<20> arrayBuffer{};

	// テキストサイズに制限をかける
	ApiWrap::LimitEditText(hWnd, arrayBuffer);

	// 現在のテキストを取得
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), StrEq(L""));

	// 制限を無視してテキストを変更（変更できてしまう仕様）
	ApiWrap::SetWindowTextW(hWnd, std::format(L"{:a<20}", L'a'));
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), StrEq(std::format(L"{:a<20}", L'a')));
}

/*!
 * リストボックスを対象とするラッパーメソッドのテスト
 */
TEST(ApiWrap, ListTest001) {
	const auto expected = L"0123456789abcdef"s;

	const auto hInstance = ::GetModuleHandleW(nullptr);
	const auto hWnd = ::CreateWindowExW(0, WC_LISTBOX, L"", 0, 1, 1, 1, 1, HWND(nullptr), HMENU(nullptr), hInstance, nullptr);

	WindowHolder windowHolder{ hWnd };

	// アイテムを追加
	ApiWrap::AddLbItems(hWnd, std::array{ expected, L""s });

	// アイテム数を取得
	EXPECT_THAT(ApiWrap::List_GetCount(hWnd), Eq(2));

	// 初期状態は未選択
	EXPECT_THAT(ApiWrap::List_GetCurSel(hWnd), Eq(LB_ERR));

	// プログラム的にアイテム選択する
	EXPECT_THAT(ApiWrap::List_SetCurSel(hWnd, 1), Eq(1));
	EXPECT_THAT(ApiWrap::List_GetCurSel(hWnd), Eq(1));

	// 指定したアイテムの表示文字列を取得（この機能はあまり重要でない）
	EXPECT_THAT(ApiWrap::GetLbItemText(hWnd, 0), StrEq(expected));
	EXPECT_THAT(ApiWrap::GetLbItemText(hWnd, 1), StrEq(L""));
	EXPECT_THAT(ApiWrap::GetLbItemText(hWnd, 2), IsFalse());

	// 指定したアイテムの表示文字列を取得（この機能はあまり重要でない）
	std::wstring s(std::size(expected), L'\0');
	EXPECT_THAT(ApiWrap::List_GetText(hWnd, 0, std::span{ std::data(s), std::size(expected) - 1 }), LB_ERRSPACE);
	EXPECT_THAT(ApiWrap::List_GetText(hWnd, 0, std::span{ std::data(s), std::size(expected) + 0 }), LB_ERRSPACE);
	EXPECT_THAT(ApiWrap::List_GetText(hWnd, 0, std::span{ std::data(s), std::size(expected) + 1 }), int(std::size(expected)));
	EXPECT_THAT(s, StrEq(expected));

	EXPECT_THAT(ApiWrap::List_GetText(hWnd, 2, std::span{ std::data(s), std::size(expected) + 1 }), LB_ERR);
}

/*!
 * コンボボックスを対象とするラッパーメソッドのテスト
 */
TEST(ApiWrap, ComboTest001) {
	const auto expected = L"0123456789abcdef"s;

	const auto hInstance = ::GetModuleHandleW(nullptr);
	const auto hWnd = ::CreateWindowExW(0, WC_COMBOBOX, L"", 0, 1, 1, 1, 1, HWND(nullptr), HMENU(nullptr), hInstance, nullptr);

	WindowHolder windowHolder{ hWnd };

	// アイテムを追加
	ApiWrap::AddCbItems(hWnd, std::array{ expected, L""s });

	// アイテム数を取得
	EXPECT_THAT(ApiWrap::Combo_GetCount(hWnd), Eq(2));

	// 初期状態は未選択
	EXPECT_THAT(ApiWrap::Combo_GetCurSel(hWnd), Eq(CB_ERR));

	// プログラム的にアイテム選択する
	EXPECT_THAT(ApiWrap::Combo_SetCurSel(hWnd, 1), Eq(1));
	EXPECT_THAT(ApiWrap::Combo_GetCurSel(hWnd), Eq(1));

	// 指定したアイテムの表示文字列を取得（この機能はあまり重要でない）
	EXPECT_THAT(ApiWrap::GetCbItemText(hWnd, 0), StrEq(expected));
	EXPECT_THAT(ApiWrap::GetCbItemText(hWnd, 1), StrEq(L""));
	EXPECT_THAT(ApiWrap::GetCbItemText(hWnd, 2), IsFalse());

	// サイズ20の配列バッファを用意する
	StaticString<20> arrayBuffer{};

	// アイテムサイズに制限をかける
	ApiWrap::LimitCbText(hWnd, arrayBuffer);

	// 選択中アイテムのテキストを取得
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), StrEq(L""));

	// 制限を無視してテキストを変更（変更できてしまう仕様）
	ApiWrap::SetWindowTextW(hWnd, std::format(L"{:a<20}", L'a'));
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), StrEq(std::format(L"{:a<20}", L'a')));

	// アイテムを追加（追加できてしまう仕様）
	ApiWrap::AddCbItems(hWnd, std::array{ std::format(L"{:a<20}", L'a') });
	EXPECT_THAT(ApiWrap::GetCbItemText(hWnd, 2), StrEq(std::format(L"{:a<20}", L'a')));

	CNativeW cmemText;
	EXPECT_THAT(ApiWrap::Combo_GetLBText(hWnd, 0, cmemText), int(std::size(expected)));
	EXPECT_THAT(cmemText.GetStringPtr(), StrEq(expected));
}

/*!
 * ダイアログボックス項目を対象とするラッパーメソッドのテスト
 */
TEST(ApiWrap, DlgItemTest001) {
	const auto expected = L"0123456789abcdef"s;

	const auto hInstance = ::GetModuleHandleW(nullptr);
	const HWND hWnd = nullptr;

	struct CDlgTest : public CDialog {
		CDlgTest() noexcept
			: CDialog(false, false)
		{
		}

		BOOL OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam) override {
			OnFirstIdle(hDlg);

			::EndDialog(hDlg, IDCANCEL);

			return TRUE;
		}

		void OnFirstIdle(HWND hDlg) const {
			// アイテムにテキストを設定しておく
			ApiWrap::SetDlgItemTextW(hDlg, IDC_COMBO_TEXT, L"test item"s);
			EXPECT_THAT(ApiWrap::GetDlgItemTextW(hDlg, IDC_COMBO_TEXT), StrEq(L"test item"));

			EXPECT_THAT(ApiWrap::IsDlgButtonChecked(hDlg, IDC_CHK_REGULAREXP), IsFalse());
			EXPECT_THAT(IsDlgButtonCheckedBool(hDlg, IDC_CHK_REGULAREXP), IsFalse());

			ApiWrap::CheckDlgButton(hDlg, IDC_CHK_REGULAREXP, true);
			EXPECT_THAT(ApiWrap::IsDlgButtonChecked(hDlg, IDC_CHK_REGULAREXP), IsTrue());
			EXPECT_THAT(IsDlgButtonCheckedBool(hDlg, IDC_CHK_REGULAREXP), IsTrue());

			ApiWrap::CheckDlgButton(hDlg, IDC_CHK_REGULAREXP, false);
			EXPECT_THAT(ApiWrap::IsDlgButtonChecked(hDlg, IDC_CHK_REGULAREXP), IsFalse());
			EXPECT_THAT(IsDlgButtonCheckedBool(hDlg, IDC_CHK_REGULAREXP), IsFalse());

			CheckDlgButtonBool(hDlg, IDC_CHK_REGULAREXP, true);
			EXPECT_THAT(ApiWrap::IsDlgButtonChecked(hDlg, IDC_CHK_REGULAREXP), IsTrue());
			EXPECT_THAT(IsDlgButtonCheckedBool(hDlg, IDC_CHK_REGULAREXP), IsTrue());

			CheckDlgButtonBool(hDlg, IDC_CHK_REGULAREXP, false);
			EXPECT_THAT(ApiWrap::IsDlgButtonChecked(hDlg, IDC_CHK_REGULAREXP), IsFalse());
			EXPECT_THAT(IsDlgButtonCheckedBool(hDlg, IDC_CHK_REGULAREXP), IsFalse());

			EXPECT_THAT(apiwrap::IsDlgItemEnabled(hDlg, IDC_CHK_REGULAREXP), IsTrue());

			DlgItem_Enable(hDlg, IDC_CHK_REGULAREXP, false);
			EXPECT_THAT(apiwrap::IsDlgItemEnabled(hDlg, IDC_CHK_REGULAREXP), IsFalse());

			DlgItem_Enable(hDlg, IDC_CHK_REGULAREXP, true);
			EXPECT_THAT(apiwrap::IsDlgItemEnabled(hDlg, IDC_CHK_REGULAREXP), IsTrue());

			CTextWidthCalc calc{ hDlg, IDC_COMBO_TEXT };
			calc.Reset();
		}
	};

	CDlgTest cDlg{};
	cDlg.DoModal(hInstance, hWnd, IDD_FIND, 0L);
}

} // namespace window
