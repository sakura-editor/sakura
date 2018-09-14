// code-util_string_ex.cpp
// util/string_ex.cpp 取込用スタブファイル

// リンク依存関係を取り込まないためにダミーのマクロ定義をしておく
//#include "debug/Debug2.cpp"
#define debug_output(str, ...)
#define debug_exit2(file, line, exp)


// sakura_core/StdAfx.h にある前提インクルードを取り込む
#define NOMINMAX
#include <tchar.h>
#include <Windows.h>

// 独自シンボル USE_STRICT_INT を解決するために参照が必要
#include "config/build_config.h"

// 独自定義型 ACHAR を解決するために参照が必要
#include "basis/primitive.h"

// 独自定義型 CLogicInt を解決するために参照が必要
// util/string_ex.hを読み込む前に、これをインクルードしておく必要がある
#include "basis/SakuraBasis.h"


// cppファイルを埋め込みで取り込む
// 他ファイルで同じファイルを取り込んではいけない
#include "util/string_ex.cpp"
