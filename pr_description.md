# PR対象

- アプリ(サクラエディタ本体)
- テストコード

## カテゴリ

- 不具合修正
- 改善

## PR の背景

PR #2067 において SAKURAClipW 形式のヘッダ型が `int` から `size_t` に変更されたことにより、32bit版（`size_t` = 4バイト）と64bit版（`size_t` = 8バイト）の間でクリップボードのバイナリレイアウトが不一致となり、クロスビット間のコピー＆ペースト時にヘッダの読み取り位置がずれて異常なサイズを確保しようとしクラッシュする問題が発生しています。

関連: #2450, #2067, Issue #2325

## 仕様・動作説明

### 1. SAKURAClipW ヘッダ型の `int32_t` 固定化

ヘッダ型を環境依存の `size_t` から固定長の `int32_t`（4バイト）に変更し、v2.4.2 リリース版（`int` = 4バイト）とのバイナリ互換性を回復する。ヘッダ構造体 `SSakuraClipHeader` を新設し、`#pragma pack(push, 1)` + `static_assert` でレイアウトを保証する。

```cpp
#pragma pack(push, 1)
struct SSakuraClipHeader {
    int32_t cchData;
};
#pragma pack(pop)
static_assert(sizeof(SSakuraClipHeader) == 4, "SSakuraClipHeader must be exactly 4 bytes");
```

フォーマット名 `SAKURAClipW` は変更しない（バイナリレイアウトが v2.4.2 と一致するため）。

### 2. SAKURAClipW メモリレイアウト

```
オフセット    サイズ        内容
──────────────────────────────────────
0x00          4 bytes      int32_t  cchData（文字数、符号付き）
0x04          N * 2 bytes  wchar_t  szData[cchData]（文字データ）
0x04 + N*2    2 bytes      wchar_t  L'\0'（終端ヌル）
```

### 3. CLIPBOARD_MAX_CHARS 定数と早期リターン

`CClipboard::CLIPBOARD_MAX_CHARS = INT32_MAX` を定義し、クリップボード操作全般の安全な上限とする。

**SetText 書き込み時**: 関数冒頭で `nDataLen > CLIPBOARD_MAX_CHARS` を判定し、超過時は `return false` で即座に終了する。`CLIPBOARD_MAX_CHARS` は `INT32_MAX` と等価であり、この早期リターン以降は `nDataLen` が `int32_t` の表現範囲に収まることが保証される。SAKURAClipW ヘッダへの `static_cast<int32_t>(nDataLen)` は常に安全であり、CF_UNICODETEXT・矩形選択フラグ・行選択フラグを含む全形式で無条件に書き込み処理を行う。

**GetText 読み込み時**: CF_UNICODETEXT / CF_OEMTEXT / GetClipboardByFormat のデータ長を `CLIPBOARD_MAX_CHARS` で切り詰める。

### 4. GlobalSize() による3段階フェイルセーフ（GetText 読み込み時）

SAKURAClipW 形式の読み込み時に、ヘッダの自己申告値を鵜呑みにせず `GlobalSize()` が返す実際のメモリサイズと突き合わせて検証する。検証ロジックはヘルパー関数 `TryReadSakuraClipData` に分離し、`GetText` 側のネストを浅く保つ。

| 段階 | チェック内容 | 防御対象 |
|------|-------------|---------|
| 第1段階 | `pData == nullptr \|\| cbData < sizeof(SSakuraClipHeader)` | ロック失敗・データ不足 |
| 第2段階 | `cchRaw < 0` | 符号反転による不正値（32/64bit混在時） |
| 第3段階 | `cchData > cchMax` | ヘッダの自己申告値が実メモリ超過 → 破損データとして拒否 |

フォーマット未指定（デフォルト）の場合、SAKURAClipW の検証が失敗すると CF_UNICODETEXT へ自動フォールバックする。

### 5. SIZE_T オーバーフロー防止（SetText CF_UNICODETEXT 書き込み時）

`nDataLen + 1` の加算オーバーフローと `cchUnicode * sizeof(wchar_t)` の乗算オーバーフローを1つの `if` 文に統合し、発生時は単一の `break` で CF_UNICODETEXT の `GlobalAlloc` をスキップする。`GlobalAlloc` 成功時の処理を正論理の `if( hgClipText )` に変更し、`while(bUnicodeText)` ループ内の `break` を3箇所から1箇所に削減する（SonarCloud S924: brain-overload 対応）。`CLIPBOARD_MAX_CHARS` による早期リターンが先に効くため実質到達しないが、低コストな多重防御として残置する。

### 6. SEH 例外ハンドリング（STATUS_NO_MEMORY 対策）

大容量データのペースト時に Windows ヒープが投げる SEH 例外 `STATUS_NO_MEMORY`（`0xC0000017`）に対応するため、`__try/__except` を使用した安全なラッパー関数を導入する。

`CMemory::AllocBuffer` 内部の `malloc`/`realloc` は C++ 例外を投げず、`std::wstring::append` は `std::bad_alloc` の前に Windows ヒープが SEH 例外を投げるため、C++ の `try-catch(std::bad_alloc&)` では捕捉できない。MSVC の制約上、`__try/__except` はデストラクタを持つ C++ オブジェクトと共存できないため、独立した関数に分離している。

| ヘルパー関数 | 用途 | 配置ファイル |
|-------------|------|-------------|
| `SafeAppend` | `IWBuffer::Append` の保護 | `CClipboard.cpp` |
| `TryReadSakuraClipData` | SAKURAClipW データの検証と読み取り | `CClipboard.cpp` |
| `SafeSetString` | `CNativeW::SetString` の保護 | `CEditView_Mouse.cpp` |
| `AdjustTripleClickCursorAbove` | トリプルクリック上方ドラッグ時のカーソル行頭補正 | `CEditView_Mouse.cpp` |
| `AdjustTripleClickCursorBelow` | トリプルクリック下方ドラッグ時のカーソル次行頭補正 | `CEditView_Mouse.cpp` |
| `SafeNewBytes` | `new BYTE[]` の保護 | `CDropTarget.cpp` |

```cpp
static bool SafeAppend(IWBuffer* cmemBuf, const wchar_t* pData, size_t nLen)
{
    __try {
        cmemBuf->Append(pData, nLen);
        return true;
    }
    __except( GetExceptionCode() == STATUS_NO_MEMORY
        ? EXCEPTION_EXECUTE_HANDLER
        : EXCEPTION_CONTINUE_SEARCH )
    {
        return false;
    }
}
```

### 7. CDropTarget::CDataObject の安全化

- `SafeNewBytes` による全メモリ確保の SEH 保護。確保失敗時は `goto fail` で全エントリをクリーンアップする。
- `m_pData` 配列のゼロ初期化（`goto fail` 時に未初期化ポインタを `delete[]` する未定義動作を防止）。
- SAKURAClipW 形式のヘッダを `int32_t` で書き込み（`memcpy_raw` 使用）。
- `nTextLen > INT32_MAX` の場合は SAKURAClipW エントリを除外して `m_nFormat` を調整し、CF_UNICODETEXT・CF_TEXT・矩形選択フラグは維持する。
- `lpszText == nullptr` 経路が `fail:` ラベルへフォールスルーして `m_pData`（null）を参照する Null pointer dereference を修正。`fail:` 直前に `return;` を追加し、`fail:` を `goto fail` 経由のみの到達に限定する。

### 8. CEditView::Drop() の安全化

- `for(;;)` ループの導入: SAKURAClipW → CF_UNICODETEXT → CF_TEXT の順にフォールバックを試行するループ構造に変更。
- SAKURAClipW パス: `memcpy_raw` でエイリアシング安全にヘッダを読み取り、`header.cchData >= 0` かつ `cchData <= cchMax` で検証。検証失敗時は `GlobalUnlock` → `GlobalFree` → 次の形式へフォールバック。
- CF_UNICODETEXT パス: `wcsnlen` で文字数を算出し、`CLIPBOARD_MAX_CHARS` で切り詰め。`SafeSetString` で SEH を保護。失敗時は `E_OUTOFMEMORY` を返す。
- CF_TEXT パス: `CLIPBOARD_MAX_CHARS * sizeof(wchar_t)` でバイト数を切り詰めてから SJIS→UNICODE 変換。変換後の文字数も `CLIPBOARD_MAX_CHARS` で再切り詰め。

### 9. CEditView::Drop() の二重解放の修正

ドロップデータ取得ループ導入により、`hData` はループ直後の解放ブロックで `GlobalUnlock`/`GlobalFree` される。従来の関数末尾にあった同処理（`// 2004.07.12 fotomo/もか メモリリークの修正`）は解放済みハンドルへの二重操作となるため、末尾ブロックを削除して解放を1箇所に一本化する。

---

## 仕様・動作説明（ファイル別ロジック詳細）

---

### 1. `sakura_core/_os/CClipboard.h`

**SSakuraClipHeader 構造体の新設**

SAKURAClipW 独自クリップボード形式のバイナリヘッダを、`#pragma pack(push, 1)` で1バイトアラインメントに固定した構造体として定義する。`static_assert` でサイズが正確に4バイトであることをコンパイル時に保証する。従来は `size_t` をキャストして直接書き込んでおり、32bit（4バイト）と64bit（8バイト）でレイアウトが不一致になっていた。

```
オフセット    サイズ        内容
0x00          4 bytes      int32_t  cchData（文字数）
0x04          N * 2 bytes  wchar_t  szData[cchData]
0x04 + N*2    2 bytes      wchar_t  L'\0'（終端ヌル）
```

**CLIPBOARD_MAX_CHARS 定数の追加**

`CClipboard` クラスの `static constexpr` メンバとして `CLIPBOARD_MAX_CHARS = INT32_MAX` を定義する。ヘッダ末尾にグローバルスコープの `static constexpr` エイリアスも配置し、無名名前空間内の `SafeAppend` 等から参照可能にしている。

**変更なしの項目**

`SetText()` / `GetText()` の関数シグネチャ — パラメータ型 `size_t nDataLen` は PR #2067 で `int` から変更されたものだが、関数インターフェースとしては大容量データの受け渡しに必要なため、`int` には戻さずそのまま維持する。`int32_t` に固定化したのは SAKURAClipW のバイナリヘッダのみであり、関数の引数型とは独立している。

---

### 2. `sakura_core/_os/CClipboard.cpp`

**SafeAppend ヘルパー関数の追加（無名名前空間）**

`IWBuffer::Append` の呼び出しを Windows SEH（`__try/__except`）で保護する static 関数。`STATUS_NO_MEMORY`（`0xC0000017`）を捕捉した場合のみ `EXCEPTION_EXECUTE_HANDLER` を返し `false` で復帰する。それ以外の例外は `EXCEPTION_CONTINUE_SEARCH` で上位に伝播させる。C++ の `try-catch(std::bad_alloc&)` ではなく SEH を使用する理由は、`CMemory::AllocBuffer` 内部の `malloc`/`realloc` が C++ 例外を投げないこと、および `std::wstring::append` が `std::bad_alloc` を投げる前に Windows ヒープが SEH 例外を投げる場合があるため。MSVC の制約上、`__try/__except` はデストラクタを持つ C++ オブジェクトと同一関数に配置できないため、独立した関数に分離している。

**TryReadSakuraClipData ヘルパー関数の追加（無名名前空間）**

ロック済み SAKURAClipW データの3段階検証（ヘッダサイズ・負値・実メモリ超過）と `SafeAppend` による追記を行う static 関数。`GetText` 内のネストを3段以下に抑えるために分離している。検証失敗または `SafeAppend` 失敗時は `false` を返し、呼び出し元がフォーマット指定に応じて `return false`（明示指定）またはフォールスルー（既定取得）を判断する。

**SetText() の変更**

- CLIPBOARD_MAX_CHARS による早期リターン: 関数冒頭で `nDataLen > CLIPBOARD_MAX_CHARS` を判定し、超過時は `return false` で全形式の書き込みを行わず即座に終了する。この時点で `nDataLen <= INT32_MAX` が保証されるため、以降の SAKURAClipW ヘッダへの `int32_t` キャストは安全である。
- CF_UNICODETEXT の SIZE_T オーバーフロー防止: `nDataLen + 1` の加算オーバーフローと `cchUnicode * sizeof(wchar_t)` の乗算オーバーフローを1つの `if` に統合し、発生時は単一の `break` で書き込みをスキップする。`GlobalAlloc` 成功時の処理を正論理の `if( hgClipText )` に変更し、`while(bUnicodeText)` 内の `break` を3→1に削減する（SonarCloud S924 対応）。
- SAKURAClipW ヘッダの `int32_t` 書き込み: `memcpy(pClip, &header, sizeof(header))` で `SSakuraClipHeader` サイズ分（固定4バイト）を書き込む。
- SAKURAClipW のコメント更新: データレイアウトの説明を `SSakuraClipHeader` ベースに修正。
- 関数末尾の成否判定: `hgClipText` と `hgClipSakura` を個別にチェックする。

**GetText(IWBuffer\*) の変更**

- SAKURAClipW 読み込み: `TryReadSakuraClipData` を呼び出してヘッダ検証とデータコピーを行う。呼び出し元は戻り値に応じて `GlobalUnlock` → `return true`（成功）、`return false`（明示指定での失敗）、またはフォールスルー（既定取得での失敗→CF_UNICODETEXT へ）を判断する。
- CF_UNICODETEXT 読み込み: `GlobalSize()` から `wcsnlen` で実文字数を算出し、`std::min(cchTotal, CLIPBOARD_MAX_CHARS)` で上限に切り詰める。`SafeAppend` で SEH を保護し、失敗時は `GlobalUnlock` → `return false`。
- CF_OEMTEXT 読み込み: `GlobalSize()` を `CLIPBOARD_MAX_CHARS * sizeof(wchar_t)` で切り詰めてから SJIS→UNICODE 変換する。変換後の文字数も `CLIPBOARD_MAX_CHARS` で再切り詰める。`SafeAppend` で保護する。

**GetClipboardByFormat() の変更**

- `GetLengthByMode()` で取得した `nLength` を、モードに応じた上限値（バイナリモードは `CLIPBOARD_MAX_CHARS`、それ以外は `CLIPBOARD_MAX_CHARS * sizeof(wchar_t)`）で切り詰める。
- `nEndMode == -1` で再取得した `nLength` にも同じ上限を適用する。

---

### 3. `sakura_core/_os/CDropTarget.h`

`#include "CClipboard.h"` を追加。`CDataObject::SetText` 内で `SSakuraClipHeader` と `CClipboard::GetSakuraFormat()` を参照するため。

---

### 4. `sakura_core/_os/CDropTarget.cpp`

**SafeNewBytes ヘルパー関数の追加（無名名前空間）**

`new BYTE[]` を `__try/__except` で保護する static 関数。`STATUS_NO_MEMORY` 捕捉時は `*ppOut = nullptr` を設定して `false` を返す。

**CDataObject::SetText() の変更**

- `bUseSakuraFormat` フラグの導入: `nTextLen <= INT32_MAX` を評価する。`false` の場合は SAKURAClipW エントリを除外して `m_nFormat` を調整（2 or 3）し、CF_UNICODETEXT・CF_TEXT・矩形選択フラグは維持する。
- `m_pData` 配列のゼロ初期化: `new DATA[m_nFormat]` の直後に全エントリの `data` を `nullptr` に初期化する。`goto fail` 時に未初期化ポインタを `delete[]` する未定義動作を防止する。
- 全 `new BYTE[]` を `SafeNewBytes` に置き換え: CF_UNICODETEXT、CF_TEXT、SAKURAClipW、MSDEVColumnSelect の各確保を SEH で保護する。失敗時は `goto fail` で全エントリをクリーンアップする。
- SAKURAClipW ヘッダの `int32_t` 書き込み: `memcpy_raw(m_pData[i].data, &cchData, sizeof(cchData))` で `SSakuraClipHeader` サイズ分を書き込む。

---

### 5. `sakura_core/view/CEditView_Mouse.cpp`

**SafeSetString ヘルパー関数の追加（無名名前空間）**

`CNativeW::SetString` を `__try/__except` で保護する static 関数。`STATUS_NO_MEMORY` 捕捉時は `false` を返す。

**Drop() 関数の変更 — ドロップデータ取得ブロック**

従来はフォーマットごとの直列的な `if-else` で1回だけデータ取得していたが、`for(;;)` ループに変更して SAKURAClipW → CF_UNICODETEXT → CF_TEXT の順にフォールバックを試行する構造とする。

- SAKURAClipW パス: `memcpy_raw(&header, pData, sizeof(header))` でヘッダをエイリアシング安全に読み取る。`header.cchData >= 0` かつ `cchData <= cchMax`（`cchMax = (nSize - sizeof(SSakuraClipHeader)) / sizeof(wchar_t)`）で検証する。条件判定は `if( pData != nullptr && nSize >= sizeof(SSakuraClipHeader) )` に正論理で記述する。検証失敗時は `GlobalUnlock` → `GlobalFree` → CF_UNICODETEXT / CF_TEXT へフォールバックする。
- CF_UNICODETEXT パス: `wcsnlen` で文字数を算出し、`CLIPBOARD_MAX_CHARS` で切り詰める。`SafeSetString` で SEH を保護し、失敗時は `GlobalUnlock` → `GlobalFree` → `E_OUTOFMEMORY` を返す。
- CF_TEXT パス: `CLIPBOARD_MAX_CHARS * sizeof(wchar_t)` でバイト数を切り詰めてから SJIS→UNICODE 変換する。変換後の文字数も `CLIPBOARD_MAX_CHARS` で再切り詰める。

**Drop() 関数の変更 — 二重解放の修正**

ドロップデータ取得ループ導入により、`hData` はループ直後で `GlobalUnlock`/`GlobalFree` される。関数末尾にあった同処理（`// 2004.07.12 fotomo/もか メモリリークの修正`）は解放済みハンドルへの二重操作となるため、末尾ブロックを削除して解放を1箇所に一本化する。

**AdjustTripleClickCursorAbove / AdjustTripleClickCursorBelow ヘルパー関数の追加（無名名前空間）**

`OnMOUSEMOVE` 内のトリプルクリック＋ドラッグ選択処理でネスト深度が5段に達していた部分を2つの static ヘルパー関数に抽出し、ネストを3段以下に抑える（SonarCloud S134: brain-overload 対応）。

| ヘルパー関数 | 用途 |
|-------------|------|
| `AdjustTripleClickCursorAbove` | 上方ドラッグ時にカーソルを論理行頭に移動する |
| `AdjustTripleClickCursorBelow` | 下方ドラッグ時にカーソルが折り返し行内に収まる場合に次の論理行頭へ補正する |

---

### 6. `sakura_core/util/os.h`

**GlobalSakura::size_type の `int32_t` 変更**

テストヘルパークラス `cxx::GlobalSakura` の `size_type` を `size_t` から `int32_t` に変更する。本体の `SSakuraClipHeader`（4バイト）とメモリレイアウトを一致させるため。`size_t`（x64 で8バイト）のままではテストの `CalcSize`・`SetText`・`wstring` が `SSakuraClipHeader` と異なるオフセットでデータにアクセスする。

---

### 7. `src/test/cpp/tests1/test-cclipboard.cpp`

**新規追加テスト**

| テスト名 | 検証内容 |
|---------|---------|
| `ClipboardMaxCharsConstant` | `CLIPBOARD_MAX_CHARS == INT32_MAX` かつ正値であることの確認 |
| `SetText7` | `nDataLen > INT32_MAX` でサクラ形式指定時に `SetClipboardData` が呼ばれず `false` を返す |
| `SetText8` | `nDataLen > INT32_MAX` でフォーマット未指定時に `SetClipboardData` が呼ばれず `false` を返す |
| `SetTextSizeTOverflow` | `nDataLen = SIZE_MAX` で `CLIPBOARD_MAX_CHARS` 超過により `SetClipboardData` が呼ばれず `false` を返す |
| `SetTextEmpty` | `nDataLen = 0` で SAKURAClipW・CF_UNICODETEXT が正常に書き込まれ `true` を返す |
| `SakuraCorruptTooSmall` | `cbData < sizeof(SSakuraClipHeader)` で SAKURAClipW 検証失敗→CF_UNICODETEXT へフォールバック |
| `SakuraCorruptNegativeHeader` | `cchData = -1`（負値ヘッダ）で明示指定時に `false` を返す |
| `SakuraCorruptLengthOverrun` | `cchData > cchMax`（実メモリ超過）で CF_UNICODETEXT へフォールバック |
| `SakuraEmptyData` | `cchData = 0`（空データ）で空文字列として正常に取得成功 |

**既存テストへの影響**

- `#include <limits>` を追加（`std::numeric_limits<size_t>::max()` 使用のため）。
- `cxx::GlobalSakura` の `size_type` 変更に伴い、SAKURAClipW 関連テスト（SetText1, SetText3, NoSpecifiedFormat1, SakuraFormatSuccess 等）は追加修正なしで PASS する。

---

## PR の影響範囲

### 修正対象ファイル

| ファイル | 変更内容 |
|---------|---------|
| `sakura_core/_os/CClipboard.h` | `SSakuraClipHeader` 構造体新設、`CLIPBOARD_MAX_CHARS` 定数追加 |
| `sakura_core/_os/CClipboard.cpp` | `SafeAppend`・`TryReadSakuraClipData` 追加、SetText 早期リターン・オーバーフロー防止・S924 break 削減・`int32_t` 対応、GetText `GlobalSize()` 3段階チェック・上限切り詰め |
| `sakura_core/_os/CDropTarget.h` | `#include "CClipboard.h"` 追加 |
| `sakura_core/_os/CDropTarget.cpp` | `SafeNewBytes` 追加、`int32_t` 対応、ゼロ初期化、SAKURAClipW スキップ、`fail:` フォールスルー null deref 修正 |
| `sakura_core/view/CEditView_Mouse.cpp` | `SafeSetString`・`AdjustTripleClickCursorAbove`/`Below` 追加、Drop() フォールバックチェーン・`int32_t` 対応・二重解放修正、OnMOUSEMOVE S134 ネスト削減 |
| `sakura_core/util/os.h` | `GlobalSakura::size_type` を `int32_t` に変更 |
| `src/test/cpp/tests1/test-cclipboard.cpp` | `CLIPBOARD_MAX_CHARS` 定数テスト・SetText 早期リターンテスト・SetTextEmpty・SAKURAClipW 破損検出テスト5件追加 |

### 変更しないもの

- フォーマット名 `SAKURAClipW`（バイナリレイアウトが v2.4.2 と一致するため）
- `SetText()` / `GetText()` の関数シグネチャ — パラメータ型 `size_t nDataLen` は PR #2067 で `int` から変更されたものだが、関数インターフェースとしては大容量データの受け渡しに必要なため、`int` には戻さず維持する。`int32_t` に固定化したのは SAKURAClipW のバイナリヘッダのみであり、関数の引数型とは独立している。
- CF_UNICODETEXT 関連の基本的な処理フロー
- `SetHtmlText()` / `HDROP` 処理（大容量データに該当しない）

## テスト内容

### ユニットテスト（test-cclipboard.cpp）

#### SetText 系

| テスト | 内容 |
|-------|------|
| SetText1〜6 | 既存テスト（回帰確認） |
| SetText7 | `nDataLen > INT32_MAX` でサクラ形式指定時に `false` を返す |
| SetText8 | `nDataLen > INT32_MAX` でフォーマット未指定時に `false` を返す |
| SetTextSizeTOverflow | `nDataLen = SIZE_MAX` で `false` を返す |
| SetTextEmpty | `nDataLen = 0` で正常書き込み成功 |

#### GetText 系

| テスト | 内容 |
|-------|------|
| NoSpecifiedFormat1〜6 | 既存テスト（回帰確認） |
| SakuraFormatNegativeLength | ヘッダ負値で拒否 |
| SakuraFormatOverflowLength | ヘッダ値が実メモリ超過で拒否 |
| CorruptedSakuraFallsBackToUnicode | 破損 SAKURAClipW → CF_UNICODETEXT フォールバック |
| SakuraCorruptTooSmall | `cbData < sizeof(SSakuraClipHeader)` でフォールバック |
| SakuraCorruptNegativeHeader | `cchData = -1` で明示指定時に `false` |
| SakuraCorruptLengthOverrun | `cchData > cchMax` でフォールバック |
| SakuraEmptyData | `cchData = 0` で空文字列として成功 |
| SakuraFormatSuccess / Failure | サクラ形式の正常取得・失敗 |
| UnicodeTextSuccess / Failure | CF_UNICODETEXT の正常取得・失敗 |
| OemTextSuccess / Failure | CF_OEMTEXT の正常取得・失敗 |

### 動作テスト

| # | テストシナリオ | 期待結果 |
|---|--------------|---------|
| 1 | 64bit版で文字列をコピー → 64bit版でペースト | 正常にペーストされる |
| 2 | 32bit版（v2.4.2）で文字列をコピー → 64bit版でペースト | 正常にペーストされる（互換性回復） |
| 3 | 64bit版で文字列をコピー → 32bit版（v2.4.2）でペースト | 正常にペーストされる（互換性回復） |
| 4 | 外部アプリから CF_UNICODETEXT でペースト | 正常にペーストされる |
| 5 | 64bit版でドラッグ＆ドロップ（SAKURAClipW形式） | 正常にドロップされる |
| 6 | 不正なクリップボードデータ（ヘッダ負値） | クラッシュせず、ペースト失敗として処理される |
| 7 | 不正なクリップボードデータ（ヘッダ値 > 実メモリサイズ） | クラッシュせず、CF_UNICODETEXT へフォールバックする |
| 8 | nDataLen > INT32_MAX のコピー | `SetText` が `false` を返し、クリップボードへの書き込みは行われない |
| 9 | 64bit版で 2GiB 超テキストをコピー → 32bit版でペースト | クラッシュせず、`STATUS_NO_MEMORY` を `SafeAppend` が捕捉して `false` を返す |

### ビルド確認

| 構成 | 確認内容 |
|------|---------|
| Win32 Release | ビルド成功、警告なし（C4018, C4267, C4244） |
| x64 Release | ビルド成功 |
| Win32 Debug | ビルド成功 |
| x64 Debug | ビルド成功 |
| ユニットテスト | 全テスト PASS |

## 関連 issue, PR

- #2450 — クリップボードからの貼り付け時のクラッシュ対応（元PR）
- #2067 — `size_t` 変更の元凶となったPR
- #2451 — OLEクリップボードのテスト追加（berryzplus氏）
- Issue #2325 — SAKURAClipW 形式の改善検討

## 参考資料

- [例外ハンドラーの使用 - Win32 apps | Microsoft Learn](https://learn.microsoft.com/ja-jp/windows/win32/debug/using-an-exception-handler) — SEH `__try/__except` のリファレンス
- PR #2067 のディスカッション — `size_t` 変更の経緯とレビュアー指摘
- PR #2450 のディスカッション — berryzplus氏による外部インターフェース互換性の指摘
