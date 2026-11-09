/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_EDITINFO_27D29614_33E6_4D60_B4B1_05115049CD16_H_
#define SAKURA_EDITINFO_27D29614_33E6_4D60_B4B1_05115049CD16_H_
#pragma once

#include "basis/SakuraBasis.h"
#include "basis/CMyString.h"
#include "config/maxdata.h"
#include "charset/charset.h"
#include "util/StaticType.h"

/*!
 * ファイル情報
 *
 * @date 2002.03.07 genta m_szDocType追加
 * @date 2003.01.26 aroka m_nWindowSizeX/Y m_nWindowOriginX/Y追加
 *
 * @note この構造体は DLLSHAREDATA に含まれるため、
 *   プロセス間共有できない型のメンバを追加してはならない。
 */
struct EditInfo {
	using SDocType = StaticString<MAX_DOCTYPE_LEN + 1>;
	using SGrepKey = StaticString<1024>;
	using SMarkLines = StaticString<MAX_MARKLINES_LEN + 1>;

	//ファイル
	SFilePath	m_szPath = {};							//!< ファイル名
	ECodeType	m_nCharCode = CODE_AUTODETECT;			//!< 文字コード種別
	bool		m_bBom = false;							//!< BOM(GetFileInfo)
	SDocType	m_szDocType = {};						//!< 文書タイプ
	int 		m_nTypeId = -1;							//!< 文書タイプ(MRU)

	//表示域
	CLayoutInt	m_nViewTopLine = CLayoutInt(-1);		//!< 表示域の一番上の行(0開始)
	CLayoutInt	m_nViewLeftCol = CLayoutInt(-1);		//!< 表示域の一番左の桁(0開始)

	//キャレット
	CLogicPoint m_ptCursor = {-1, -1};					//!< キャレット位置

	//各種状態
	bool		m_bIsModified = false;					//!< 変更フラグ

	//GREPモード
	bool		m_bIsGrep = false;						//!< Grepのウィンドウか
	SGrepKey	m_szGrepKey = {};

	//デバッグモニタ (アウトプットウィンドウ) モード
	bool		m_bIsDebug = false;						//!< デバッグモニタモード (アウトプットウィンドウ) か

	//ブックマーク情報
	SMarkLines	m_szMarkLines = {};						//!< ブックマークの物理行リスト

	//ウィンドウ
	int			m_nWindowSizeX = -1;					//!< ウィンドウ  幅(ピクセル数)
	int			m_nWindowSizeY = -1;					//!< ウィンドウ  高さ(ピクセル数)
	int			m_nWindowOriginX = CW_USEDEFAULT;		//!< ウィンドウ  物理位置(ピクセル数・マイナス値も有効)
	int			m_nWindowOriginY = CW_USEDEFAULT;		//!< ウィンドウ  物理位置(ピクセル数・マイナス値も有効)

	// コンストラクタ
	EditInfo() noexcept;
};

#endif /* SAKURA_EDITINFO_27D29614_33E6_4D60_B4B1_05115049CD16_H_ */
