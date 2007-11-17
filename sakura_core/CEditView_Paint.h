#pragma once

class CEditView;


//! クリッピング領域を計算する際のフラグ
enum EPaintArea{
	PAINT_LINENUMBER = (1<<0), //!< 行番号
	PAINT_RULER      = (1<<1), //!< ルーラー
	PAINT_BODY       = (1<<2), //!< 本文

	//特殊
	PAINT_ALL        = PAINT_LINENUMBER | PAINT_RULER | PAINT_BODY, //!< ぜんぶ
};

class CEditView_Paint{
public:
	virtual CEditView* GetEditView()=0;

public:
	virtual ~CEditView_Paint(){}
	void Call_OnPaint(
		int nPaintFlag,   //!< 描画する領域を選択する
		bool bUseMemoryDC //!< メモリDCを使用する
	);
};
