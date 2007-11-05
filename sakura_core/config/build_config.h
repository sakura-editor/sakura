//ビルド(コンパイル)設定
//2007.10.18 kobake 作成

#pragma once

/*!
	厳格なintを使うかどうか。

	主にエディタ部分の座標系単位に関して
	コンパイル時に静的な型チェックがされるようになります。
	ただしその分コンパイル時間もかかります。

	実行時挙動は変化無し。
	実行時オーバーヘッド不明。コンパイラが賢ければオーバーヘッドゼロ。

	リリースビルドでは無効にしておくと良い。

	@date 2007.10.18 kobake
*/
#ifdef _DEBUG
#define USE_STRICT_INT //←これをコメントアウトすると厳格なintが無効になります。リリースビルドでは常に無効。
#endif


//! USE_UNFIXED_FONT を定義すると、フォント選択ダイアログで等幅フォント以外も選べるようになる
//#define USE_UNFIXED_FONT


//UNICODE BOOL定数
#ifdef _UNICODE
static const bool UNICODE_BOOL=true;
#else
static const bool UNICODE_BOOL=false;
#endif


//DebugMonitorLib(仮)を使うかどうか
//#define USE_DEBUGMON
