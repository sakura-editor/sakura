#ifndef SAKURA_ECOLORINDEXTYPE_H_
#define SAKURA_ECOLORINDEXTYPE_H_

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          色定数                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Stonee 注： 2000/01/12
// ここを変更したときは、global.cpp のg_ColorAttributeArrの定義も変更して下さい。
//	From Here Sept. 18, 2000 JEPRO 順番を大幅に入れ替えた
//	2007.09.09 Moca  中間の定義はお任せに変更
SAKURA_CORE_API enum EColorIndexType {
	COLORIDX_TEXT = 0,		// テキスト
	COLORIDX_RULER,			// ルーラー
	COLORIDX_CARET,			// キャレット	// 2006.12.07 ryoji
	COLORIDX_CARET_IME,		// IMEキャレット // 2006.12.07 ryoji
	COLORIDX_UNDERLINE,		// カーソル行アンダーライン
	COLORIDX_CURSORVLINE,	// カーソル位置縦線 // 2006.05.13 Moca
	COLORIDX_GYOU,			// 行番号
	COLORIDX_GYOU_MOD,		// 行番号(変更行)
	COLORIDX_TAB,			// TAB記号
	COLORIDX_SPACE,			// 半角空白 //2002.04.28 Add by KK 以降全て+1
	COLORIDX_ZENSPACE,		// 日本語空白
	COLORIDX_CTRLCODE,		// コントロールコード
	COLORIDX_EOL,			// 改行記号
	COLORIDX_WRAP,			// 折り返し記号
	COLORIDX_VERTLINE,		// 指定桁縦線	// 2005.11.08 Moca
	COLORIDX_EOF,			// EOF記号
	COLORIDX_DIGIT,			// 半角数値	 //@@@ 2001.02.17 by MIK //色設定Ver.3からユーザファイルに対しては文字列で処理しているのでリナンバリングしてもよい. Mar. 7, 2001 JEPRO noted
	COLORIDX_BRACKET_PAIR,	// 対括弧	  // 02/09/18 ai Add
	COLORIDX_SELECT,		// 選択範囲
	COLORIDX_SEARCH,		// 検索文字列
	COLORIDX_SEARCH2,		// 検索文字列2
	COLORIDX_SEARCH3,		// 検索文字列3
	COLORIDX_SEARCH4,		// 検索文字列4
	COLORIDX_SEARCH5,		// 検索文字列5
	COLORIDX_COMMENT,		// 行コメント						//Dec. 4, 2000 shifted by MIK
	COLORIDX_SSTRING,		// シングルクォーテーション文字列	//Dec. 4, 2000 shifted by MIK
	COLORIDX_WSTRING,		// ダブルクォーテーション文字列		//Dec. 4, 2000 shifted by MIK
	COLORIDX_URL,			// URL								//Dec. 4, 2000 shifted by MIK
	COLORIDX_KEYWORD1,		// 強調キーワード1 // 2002/03/13 novice
	COLORIDX_KEYWORD2,		// 強調キーワード2 // 2002/03/13 novice  //MIK ADDED
	COLORIDX_KEYWORD3,		// 強調キーワード3 // 2005.01.13 MIK 3-10 added
	COLORIDX_KEYWORD4,		// 強調キーワード4
	COLORIDX_KEYWORD5,		// 強調キーワード5
	COLORIDX_KEYWORD6,		// 強調キーワード6
	COLORIDX_KEYWORD7,		// 強調キーワード7
	COLORIDX_KEYWORD8,		// 強調キーワード8
	COLORIDX_KEYWORD9,		// 強調キーワード9
	COLORIDX_KEYWORD10,		// 強調キーワード10
	COLORIDX_REGEX1,		// 正規表現キーワード1  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX2,		// 正規表現キーワード2  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX3,		// 正規表現キーワード3  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX4,		// 正規表現キーワード4  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX5,		// 正規表現キーワード5  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX6,		// 正規表現キーワード6  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX7,		// 正規表現キーワード7  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX8,		// 正規表現キーワード8  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX9,		// 正規表現キーワード9  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX10,		// 正規表現キーワード10	//@@@ 2001.11.17 add MIK
	COLORIDX_DIFF_APPEND,	// DIFF追加  //@@@ 2002.06.01 MIK
	COLORIDX_DIFF_CHANGE,	// DIFF追加  //@@@ 2002.06.01 MIK
	COLORIDX_DIFF_DELETE,	// DIFF追加  //@@@ 2002.06.01 MIK
	COLORIDX_MARK,			// ブックマーク  // 02/10/16 ai Add

	//カラーの最後
	COLORIDX_LAST,

	//カラー表示制御用
	COLORIDX_BLOCK1,		// ブロックコメント1(文字色と背景色は行コメントと同じ)
	COLORIDX_BLOCK2,		// ブロックコメント2(文字色と背景色は行コメントと同じ)

	//1000- : カラー表示制御用(正規表現キーワード)
	COLORIDX_REGEX_FIRST	= 1000,
	COLORIDX_REGEX_LAST		= COLORIDX_REGEX_FIRST + COLORIDX_LAST - 1,

	// -- -- 別名 -- -- //
	COLORIDX_DEFAULT		= COLORIDX_TEXT,
	COLORIDX_SEARCHTAIL		= COLORIDX_SEARCH5,
};
//	To Here Sept. 18, 2000
#endif
