﻿/*!	@file
	@brief ImageListの取り扱い

	@author genta
	@date Oct. 11, 2000 genta
*/
/*
	Copyright (C) 2000-2003, genta
	Copyright (C) 2003, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CIMAGELISTMGR_4FA1F1E8_0029_42BD_A346_C51BB0A70B0C_H_
#define SAKURA_CIMAGELISTMGR_4FA1F1E8_0029_42BD_A346_C51BB0A70B0C_H_
#pragma once

#include "_main/global.h"

/*! @brief ImageListの管理

	アイコンイメージを管理する

	@author genta

	@date 2003.07.21 genta イメージリストの使用をやめた．代わりにTool BarのOwner Drawを使う．
		アクセスメソッドの大部分に於いて実装が変更されている．

	@note イメージリストへのビットマップの登録でBitbltを行う部分が
		VAIOと相性が悪くブルースクリーンが発生していた．
		またイメージリストがIE3以前のcommon componentに含まれていないために
		初期Win95でイメージの表示ができなかった．それらを回避するためにImageListの使用をやめて
		当初の独自描画に戻した．
*/
class CImageListMgr {
	using Me = CImageListMgr;

public:

	//	constructor
	CImageListMgr();
	CImageListMgr(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CImageListMgr(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CImageListMgr();

	bool Create(HINSTANCE hInstance);	//	生成
	
	/*! @brief アイコンの描画
	
		指定されたDCの指定された座標にアイコンを描画する．
	
		@param [in] drawdc 描画するDevice Context
		@param [in] x 描画するX座標
		@param [in] y 描画するY座標
		@param [in] imageNo 描画するアイコン番号
		@param [in] fStyle 描画スタイル
		@param [in] cx アイコン幅
		@param [in] cy アイコン高さ
	*/
	bool DrawToolIcon( HDC drawdc, LONG x, LONG y,
		int imageNo, DWORD fStyle, LONG cx, LONG cy ) const;

	//! アイコン数を返す
	int  Count(void) const;	//	アイコン数
	
	//! アイコンの幅
	int  cx(void) const { return m_cx; }
	//! アイコンの高さ
	int  cy(void) const { return m_cy; }
	
	//! アイコンを追加する
	int Add(const WCHAR* szPath);

	//! アイコンの追加を元に戻す
	void ResetExtend();

	/*!
		イメージのToolBarへの登録
	
		@param hToolBar [in] 登録するToolBar
		@param id [in] 登録する先頭アイコン番号

		@date 2003.07.21 genta ここでは何も行わないが，受け皿だけ残しておく
		@date 2003.07.21 genta 戻り型をvoidに変更
	*/
	void  SetToolBarImages(HWND hToolBar, int id = 0) const {}

protected:
	int m_cx;			//!<	width of icon
	int m_cy;			//!<	height of icon
	/*!	@brief 透過色
	
		描画を自前で行うため，透過色を覚えておく必要がある．
		@date 2003.07.21 genta
	*/
	COLORREF m_cTrans;
	
	/*! アイコン用ビットマップを保持する
		@date 2003.07.21 genta
	*/
	HBITMAP m_hIconBitmap;

	int m_nIconCount;	//!<	アイコンの個数

	// アイコン描画関数
	void MyBitBlt( HDC drawdc, int nXDest, int nYDest,
		int nWidth, int nHeight, int nXSrc, int nYSrc ) const;
	void MyDitherBlt( HDC drawdc, int nXDest, int nYDest,
		int nWidth, int nHeight, int nXSrc, int nYSrc ) const;

	//! ツールイメージをリサイズする
	HBITMAP ResizeToolIcons( HBITMAP hRscbmp, COLORREF& clrTransparent ) const noexcept;

	//! ビットマップを一行拡張する
	void Extend(bool = true);
};
#endif /* SAKURA_CIMAGELISTMGR_4FA1F1E8_0029_42BD_A346_C51BB0A70B0C_H_ */
