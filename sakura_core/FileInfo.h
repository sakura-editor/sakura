#pragma once
#include "basis/SakuraBasis.h"
#include "config/maxdata.h"


/*! ファイル情報

	@date 2002.03.07 genta m_szDocType追加
	@date 2003.01.26 aroka m_nWindowSizeX/Y m_nWindowOriginX/Y追加
*/
struct FileInfo {
	CLayoutInt	m_nViewTopLine;			/*!< 表示域の一番上の行(0開始) */
	CLayoutInt	m_nViewLeftCol;			/*!< 表示域の一番左の桁(0開始) */
	CLogicPoint m_ptCursor;				//!< カーソル  物理位置(行頭からのバイト数, 折り返し無し行位置)
	int			m_bIsModified;			/*!< 変更フラグ */
	ECodeType	m_nCharCode;			/*!< 文字コード種別 */
	TCHAR		m_szPath[_MAX_PATH];	/*!< ファイル名 */
	BOOL		m_bIsGrep;				/*!< Grepのウィンドウか */
	BOOL		m_bIsDebug;				/*!< デバッグモードか(アウトプットウインドウ) */
	wchar_t		m_szGrepKey[1024];
	wchar_t		m_szMarkLines[MAX_MARKLINES_LEN + 1];	/*!< ブックマークの物理行リスト */
	TCHAR		m_szDocType[MAX_DOCTYPE_LEN + 1];	/*!< 文書タイプ */
	int			m_nWindowSizeX;			/*!< ウィンドウ  幅(ピクセル数) */
	int			m_nWindowSizeY;			/*!< ウィンドウ  高さ(ピクセル数) */
	int			m_nWindowOriginX;		/*!< ウィンドウ  物理位置(ピクセル数・マイナス値も有効) */
	int			m_nWindowOriginY;		/*!< ウィンドウ  物理位置(ピクセル数・マイナス値も有効) */
	
	// Mar. 7, 2002 genta
	// Constructor 確実に初期化するため
	FileInfo()
		: m_nViewTopLine( -1 )
		, m_nViewLeftCol( -1 )
		, m_ptCursor(CLogicInt(-1), CLogicInt(-1))
		, m_bIsModified( 0 )
		, m_nCharCode( CODE_AUTODETECT )
		, m_bIsGrep( FALSE )
		, m_bIsDebug( FALSE )
		, m_nWindowSizeX( -1 )
		, m_nWindowSizeY( -1 )
		//	2004.05.13 Moca “指定無し”を-1からCW_USEDEFAULTに変更
		, m_nWindowOriginX( CW_USEDEFAULT )
		, m_nWindowOriginY( CW_USEDEFAULT )
	{
		m_szPath[0] = '\0';
		m_szMarkLines[0] = L'\0';
		m_szDocType[0] = '\0';
	}
};
