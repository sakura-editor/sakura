#pragma once
#include "basis/SakuraBasis.h"
#include "config/maxdata.h"
#include "types/CType.h"


/*! ファイル情報

	@date 2002.03.07 genta m_szDocType追加
	@date 2003.01.26 aroka m_nWindowSizeX/Y m_nWindowOriginX/Y追加
*/
struct EditInfo {
	//ファイル
	TCHAR		m_szPath[_MAX_PATH];					//!< ファイル名
	ECodeType	m_nCharCode;							//!< 文字コード種別
	TCHAR		m_szDocType[MAX_DOCTYPE_LEN + 1];		//!< 文書タイプ
	CTypeConfig m_nType;								//!< 文書タイプ(MRU)

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
	EditInfo()
	: m_nViewTopLine( -1 )
	, m_nViewLeftCol( -1 )
	, m_nType( -1 )
	, m_ptCursor(CLogicInt(-1), CLogicInt(-1))
	, m_bIsModified( 0 )
	, m_nCharCode( CODE_AUTODETECT )
	, m_bIsGrep( false )
	, m_bIsDebug( false )
	, m_nWindowSizeX( -1 )
	, m_nWindowSizeY( -1 )
	, m_nWindowOriginX( CW_USEDEFAULT )	//	2004.05.13 Moca “指定無し”を-1からCW_USEDEFAULTに変更
	, m_nWindowOriginY( CW_USEDEFAULT )
	{
		m_szPath[0] = '\0';
		m_szMarkLines[0] = L'\0';
		m_szDocType[0] = '\0';
	}
};
