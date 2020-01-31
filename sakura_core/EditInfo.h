/*! @file */
/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

#include "basis/SakuraBasis.h"
#include "config/maxdata.h"
#include "charset/charcode.h"
#include "mem/CNativeW.h"
#include "types/CType.h"

/*!
 * ファイル情報
 *
 * @date 2002.03.07 genta m_szDocType追加
 * @date 2003.01.26 aroka m_nWindowSizeX/Y m_nWindowOriginX/Y追加
 *
 * @note この構造体は DLLSHAREDATA に含まれるため、
 *   プロセス間共有できない型のメンバを追加してはならない。
 */
struct EditInfo
{
    //ファイル
    WCHAR m_szPath[_MAX_PATH]; //!< ファイル名
    ECodeType m_nCharCode; //!< 文字コード種別
    bool m_bBom; //!< BOM(GetFileInfo)
    WCHAR m_szDocType[MAX_DOCTYPE_LEN + 1]; //!< 文書タイプ
    int m_nTypeId; //!< 文書タイプ(MRU)

    //表示域
    CLayoutInt m_nViewTopLine; //!< 表示域の一番上の行(0開始)
    CLayoutInt m_nViewLeftCol; //!< 表示域の一番左の桁(0開始)

    //キャレット
    CLogicPoint m_ptCursor; //!< キャレット位置

    //各種状態
    bool m_bIsModified; //!< 変更フラグ

    //GREPモード
    bool m_bIsGrep; //!< Grepのウィンドウか
    wchar_t m_szGrepKey[1024];

    //デバッグモニタ (アウトプットウィンドウ) モード
    bool m_bIsDebug; //!< デバッグモニタモード (アウトプットウィンドウ) か

    //ブックマーク情報
    wchar_t m_szMarkLines[MAX_MARKLINES_LEN + 1]; //!< ブックマークの物理行リスト

    //ウィンドウ
    int m_nWindowSizeX; //!< ウィンドウ  幅(ピクセル数)
    int m_nWindowSizeY; //!< ウィンドウ  高さ(ピクセル数)
    int m_nWindowOriginX; //!< ウィンドウ  物理位置(ピクセル数・マイナス値も有効)
    int m_nWindowOriginY; //!< ウィンドウ  物理位置(ピクセル数・マイナス値も有効)

    // コンストラクタ
    EditInfo() noexcept;
};
