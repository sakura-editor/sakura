#pragma once

bool _IsPosKeywordHead(const CStringRef& cStr, int nPos);

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
	COLORIDX_SEARCH,		// 検索文字列
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
	COLORIDX_COMMENT,		// 行コメント						//Dec. 4, 2000 shifted by MIK
	COLORIDX_SSTRING,		// シングルクォーテーション文字列	//Dec. 4, 2000 shifted by MIK
	COLORIDX_WSTRING,		// ダブルクォーテーション文字列		//Dec. 4, 2000 shifted by MIK
	COLORIDX_URL,			// URL								//Dec. 4, 2000 shifted by MIK
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
	COLORIDX_BRACKET_PAIR,	// 対括弧	  // 02/09/18 ai Add
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
};
//	To Here Sept. 18, 2000

//正規表現キーワードのEColorIndexType値を作る関数
inline EColorIndexType ToColorIndexType_RegularExpression(const int& nRegexColorIndex)
{
	return (EColorIndexType)(COLORIDX_REGEX_FIRST + nRegexColorIndex);
}

//正規表現キーワードのEColorIndexType値かどうか
inline bool IsRegularExpression(const EColorIndexType& eColorIndex)
{
	return (eColorIndex >= COLORIDX_REGEX_FIRST && eColorIndex <= COLORIDX_REGEX_LAST);
}

//正規表現キーワードのEColorIndexType値を色番号に戻す関数
inline int ToColorInfoArrIndex_RegularExpression(const EColorIndexType& eRegexColorIndex)
{
	return eRegexColorIndex - COLORIDX_REGEX_FIRST;
}

//EColorIndexType値を色番号に変換する関数
inline int ToColorInfoArrIndex(const EColorIndexType& eColorIndex)
{
	if(eColorIndex>=0 && eColorIndex<COLORIDX_LAST)
		return eColorIndex;
	else if(eColorIndex==COLORIDX_BLOCK1 || eColorIndex==COLORIDX_BLOCK2)
		return COLORIDX_COMMENT;
	else if( IsRegularExpression(eColorIndex) )
		return ToColorInfoArrIndex_RegularExpression(eColorIndex);

	return -1;
}

// カラー名＜＞インデックス番号の変換	//@@@ 2002.04.30
SAKURA_CORE_API int GetColorIndexByName( const TCHAR *name );
SAKURA_CORE_API const TCHAR* GetColorNameByIndex( int index );


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           基底                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct DispPos;
class CColorStrategy;
#include "view/DispPos.h"
#include <memory> //auto_ptr

struct SColorStrategyInfo{
	SColorStrategyInfo() : sDispPosBegin(0,0), pStrategy(NULL), pStrategyFound(NULL) {}

	//参照
	CEditView*	pcView;
	CGraphics	gr;	//(SColorInfoでは未使用)

	//スキャン位置
	LPCWSTR			pLineOfLogic;
	CLogicInt		nPosInLogic;

	//描画位置
	DispPos*		pDispPos;
	DispPos			sDispPosBegin;

	//色変え
	CColorStrategy*		pStrategy;
	CColorStrategy*		pStrategyFound;

	//! 色の切り替え
	void ChangeColor(EColorIndexType eNewColor)
	{
		this->pcView->SetCurrentColor( this->gr, eNewColor );
	}

	void DoChangeColor(const CStringRef& cLineStr);
	EColorIndexType GetCurrentColor() const;

	//! 現在のスキャン位置
	CLogicInt GetPosInLogic() const
	{
		return nPosInLogic;
	}
	CLogicInt GetPosInLayout() const;
	const CDocLine* GetDocLine() const;
	const CLayout* GetLayout() const;
};

class CColorStrategy{
public:
	virtual ~CColorStrategy(){}
	//! 色定義
	virtual EColorIndexType GetStrategyColor() const = 0;
	//! 色切り替え開始を検出したら、その直前までの描画を行い、さらに色設定を行う。
	virtual void InitStrategyStatus() = 0;
	virtual bool BeginColor(const CStringRef& cStr, int nPos){ return false; }
	virtual bool EndColor(const CStringRef& cStr, int nPos){ return true; }
	//イベント
	virtual void OnStartScanLogic(){}

	//#######ラップ
	EColorIndexType GetStrategyColorSafe() const{ if(this)return GetStrategyColor(); else return COLORIDX_TEXT; }
};

#include "util/design_template.h"
#include <vector>

class CColorStrategyPool : public TSingleton<CColorStrategyPool>{
public:
	//コンストラクタ・デストラクタ
	CColorStrategyPool();
	virtual ~CColorStrategyPool();

	//取得
	CColorStrategy*	GetStrategy(int nIndex) const{ return m_vStrategies[nIndex]; }
	int				GetStrategyCount() const{ return (int)m_vStrategies.size(); }
	CColorStrategy*	GetStrategyByColor(EColorIndexType eColor) const;

	//特定取得
	CColorStrategy* GetFoundStrategy() const{ return m_pcFoundStrategy; }

	//イベント
	void NotifyOnStartScanLogic()
	{
		m_pcFoundStrategy->OnStartScanLogic();
		for(int i=0;i<GetStrategyCount();i++){
			GetStrategy(i)->OnStartScanLogic();
		}
	}
private:
	std::vector<CColorStrategy*>	m_vStrategies;
	CColorStrategy*					m_pcFoundStrategy;
};
