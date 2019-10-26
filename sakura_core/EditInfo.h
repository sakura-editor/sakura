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
#ifndef SAKURA_EDITINFO_38A7E267_160D_4250_9012_AC3FC31993D69_H_
#define SAKURA_EDITINFO_38A7E267_160D_4250_9012_AC3FC31993D69_H_

#include "basis/SakuraBasis.h"
#include "config/maxdata.h"
#include "charset/charcode.h"
#include "mem/CNativeW.h"
#include "types/CType.h"

/*! ファイル情報

	@date 2002.03.07 genta m_szDocType追加
	@date 2003.01.26 aroka m_nWindowSizeX/Y m_nWindowOriginX/Y追加
*/
struct EditInfo {
	//ファイル
	WCHAR		m_szPath[_MAX_PATH];					//!< ファイル名
	ECodeType	m_nCharCode;							//!< 文字コード種別
	bool		m_bBom;									//!< BOM(GetFileInfo)
	WCHAR		m_szDocType[MAX_DOCTYPE_LEN + 1];		//!< 文書タイプ
	int 		m_nTypeId;								//!< 文書タイプ(MRU)

	//表示域
	CLayoutInt	m_nViewTopLine;							//!< 表示域の一番上の行(0開始)
	CLayoutInt	m_nViewLeftCol;							//!< 表示域の一番左の桁(0開始)

	//キャレット
	CLogicPoint m_ptCursor;								//!< キャレット位置

	//各種状態
	bool		m_bIsModified;							//!< 変更フラグ

	//GREPモード
	bool		m_bIsGrep;								//!< Grepのウィンドウか
	wchar_t		m_szGrepKey[1024];

	//デバッグモニタ (アウトプットウィンドウ) モード
	bool		m_bIsDebug;								//!< デバッグモニタモード (アウトプットウィンドウ) か

	//ブックマーク情報
	wchar_t		m_szMarkLines[MAX_MARKLINES_LEN + 1];	//!< ブックマークの物理行リスト

	//ウィンドウ
	int			m_nWindowSizeX;							//!< ウィンドウ  幅(ピクセル数)
	int			m_nWindowSizeY;							//!< ウィンドウ  高さ(ピクセル数)
	int			m_nWindowOriginX;						//!< ウィンドウ  物理位置(ピクセル数・マイナス値も有効)
	int			m_nWindowOriginY;						//!< ウィンドウ  物理位置(ピクセル数・マイナス値も有効)
	
	// Mar. 7, 2002 genta
	// Constructor 確実に初期化するため
	EditInfo() noexcept
		: m_szPath{ 0 }
		, m_nCharCode( CODE_AUTODETECT )
		, m_bBom( false )
		, m_szDocType{ 0 }
		, m_nTypeId( -1 )
		, m_nViewTopLine( -1 )
		, m_nViewLeftCol( -1 )
		, m_ptCursor{ -1, -1 }
		, m_bIsModified( false )
		, m_bIsGrep( false )
		, m_szGrepKey{ 0 }
		, m_bIsDebug( false )
		, m_szMarkLines{ 0 }
		, m_nWindowSizeX( -1 )
		, m_nWindowSizeY( -1 )
		, m_nWindowOriginX( CW_USEDEFAULT )	//	2004.05.13 Moca “指定無し”を-1からCW_USEDEFAULTに変更
		, m_nWindowOriginY( CW_USEDEFAULT )
	{
	}
	bool operator == (const EditInfo& rhs) const noexcept {
		if (this == &rhs) return true;
		return 0 == wcsncmp(m_szPath, rhs.m_szPath, _countof(m_szPath))
			&& m_nCharCode == rhs.m_nCharCode
			&& m_bBom == rhs.m_bBom
			&& 0 == wcsncmp(m_szDocType, rhs.m_szDocType, _countof(m_szDocType))
			&& m_nTypeId == rhs.m_nTypeId
			&& m_nViewTopLine == rhs.m_nViewTopLine
			&& m_nViewLeftCol == rhs.m_nViewLeftCol
			&& m_ptCursor == rhs.m_ptCursor
			&& m_bIsModified == rhs.m_bIsModified
			&& m_bIsGrep == rhs.m_bIsGrep
			&& 0 == wcsncmp(m_szGrepKey, rhs.m_szGrepKey, _countof(m_szGrepKey))
			&& m_bIsDebug == rhs.m_bIsDebug
			&& 0 == wcsncmp(m_szMarkLines, rhs.m_szMarkLines, _countof(m_szMarkLines))
			&& m_nWindowSizeX == rhs.m_nWindowSizeX
			&& m_nWindowSizeY == rhs.m_nWindowSizeY
			&& m_nWindowOriginX == rhs.m_nWindowOriginX
			&& m_nWindowOriginY == rhs.m_nWindowOriginY;
	}
	bool operator != (const EditInfo& rhs) const noexcept { return !(*this == rhs); }
};

#endif /* SAKURA_EDITINFO_38A7E267_160D_4250_9012_AC3FC31993D69_H_ */
/*[EOF]*/
