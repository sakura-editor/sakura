/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CEDITVIEW_PAINT_0202B897_3D47_48DD_9279_45594D80F726_H_
#define SAKURA_CEDITVIEW_PAINT_0202B897_3D47_48DD_9279_45594D80F726_H_
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
#endif /* SAKURA_CEDITVIEW_PAINT_0202B897_3D47_48DD_9279_45594D80F726_H_ */
