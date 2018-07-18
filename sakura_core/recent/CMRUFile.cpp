/*!	@file
	@brief MRUリストと呼ばれるリストを管理する

	@author YAZAKI
	@date 2001/12/23  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, YAZAKI
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, MIK
	Copyright (C) 2006, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <ShlObj.h>
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CFileNameManager.h"
#include "uiparts/CMenuDrawer.h"	//	これでいいのか？
#include "window/CEditWnd.h"
#include "util/string_ex2.h"
#include "util/window.h"

/*!	コンストラクタ
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
CMRUFile::CMRUFile()
{
	//	初期化。
	m_pShareData = &GetDllShareData();
}

/*	デストラクタ	*/
CMRUFile::~CMRUFile()
{
	m_cRecentFile.Terminate();
}

/*!
	ファイル履歴メニューの作成
	
	@param pCMenuDrawer [in] (out?) メニュー作成で用いるMenuDrawer
	
	@author Norio Nakantani
	@return 生成したメニューのハンドル

	2010/5/21 Uchi 組み直し
*/
HMENU CMRUFile::CreateMenu( CMenuDrawer* pCMenuDrawer ) const
{
	HMENU	hMenuPopUp;

	//	空メニューを作る
	hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	return CreateMenu( hMenuPopUp, pCMenuDrawer );
}
/*!
	ファイル履歴メニューの作成
	
	@param 追加するメニューのハンドル
	@param pCMenuDrawer [in] (out?) メニュー作成で用いるMenuDrawer
	
	@author Norio Nakantani
	@return 生成したメニューのハンドル

	2010/5/21 Uchi 組み直し
*/
HMENU CMRUFile::CreateMenu( HMENU	hMenuPopUp, CMenuDrawer* pCMenuDrawer ) const
{
	TCHAR	szMenu[_MAX_PATH * 2 + 10];				//	メニューキャプション
	int		i;
	bool	bFavorite;
	const BOOL bMenuIcon = m_pShareData->m_Common.m_sWindow.m_bMenuIcon;

	CFileNameManager::getInstance()->TransformFileName_MakeCache();
	
	NONCLIENTMETRICS met;
	met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
	CDCFont dcFont(met.lfMenuFont);

	for( i = 0; i < m_cRecentFile.GetItemCount(); ++i )
	{
		//	「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if ( i >= m_cRecentFile.GetViewCount() ) break;
		
		/* MRUリストの中にある開かれていないファイル */

		const EditInfo	*p = m_cRecentFile.GetItem( i );
		bFavorite = m_cRecentFile.IsFavorite( i );
		bool bFavoriteLabel = bFavorite && !bMenuIcon;
		CFileNameManager::getInstance()->GetMenuFullLabel_MRU( szMenu, _countof(szMenu), p, -1, bFavoriteLabel, i, dcFont.GetHDC() );

		//	メニューに追加。
		pCMenuDrawer->MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELMRU + i, szMenu, _T(""), TRUE,
			bFavorite ? F_FAVORITE : -1 );
	}
	return hMenuPopUp;
}

BOOL CMRUFile::DestroyMenu( HMENU hMenuPopUp ) const
{
	return ::DestroyMenu( hMenuPopUp );
}

/*!
	ファイル履歴の一覧を返す
	
	@param ppszMRU [out] 文字列へのポインタリストを格納する．
	最後の要素の次にはNULLが入る．
	予め呼び出す側で最大値+1の領域を確保しておくこと．
*/
std::vector<LPCTSTR> CMRUFile::GetPathList() const
{
	std::vector<LPCTSTR> ret;
	for( int i = 0; i < m_cRecentFile.GetItemCount(); ++i ){
		//	「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if ( i >= m_cRecentFile.GetViewCount() ) break;
		ret.push_back(m_cRecentFile.GetItemText(i));
	}
	return ret;
}

/*! アイテム数を返す */
int CMRUFile::Length(void) const
{
	return m_cRecentFile.GetItemCount();
}

/*!
	ファイル履歴のクリア
*/
void CMRUFile::ClearAll(void)
{
	m_cRecentFile.DeleteAllItem();
}

/*!
	ファイル情報の取得
	
	@param num [in] 履歴番号(0~)
	@param pfi [out] 構造体へのポインタ格納先
	
	@retval TRUE データが格納された
	@retval FALSE 正しくない番号が指定された．データは格納されなかった．
*/
bool CMRUFile::GetEditInfo( int num, EditInfo* pfi ) const
{
	const EditInfo*	p = m_cRecentFile.GetItem( num );
	if( NULL == p ) return false;

	*pfi = *p;

	return true;
}

/*!
	指定された名前のファイルがMRUリストに存在するか調べる。存在するならばファイル情報を返す。

	@param pszPath [in] 検索するファイル名
	@param pfi [out] データが見つかったときにファイル情報を格納する領域。
		呼び出し側で領域をあらかじめ用意する必要がある。
	@retval TRUE  ファイルが見つかった。pfiにファイル情報が格納されている。
	@retval FALSE 指定されたファイルはMRU Listに無い。

	@date 2001.12.26 CShareData::IsExistInMRUListから移動した。（YAZAKI）
*/
bool CMRUFile::GetEditInfo( const TCHAR* pszPath, EditInfo* pfi ) const
{
	const EditInfo*	p = m_cRecentFile.GetItem( m_cRecentFile.FindItemByPath( pszPath ) );
	if( NULL == p ) return false;

	*pfi = *p;

	return true;
}

/*!	@brief MRUリストへの登録

	@param pEditInfo [in] 追加するファイルの情報

	該当ファイルがリムーバブルディスク上にある場合にはMRU Listへの登録は行わない。

	@date 2001.03.29 MIK リムーバブルディスク上のファイルを登録しないようにした。
	@date 2001.12.26 YAZAKI CShareData::AddMRUListから移動
*/
void CMRUFile::Add( EditInfo* pEditInfo )
{
	//	ファイル名が無ければ無視
	if( NULL == pEditInfo || pEditInfo->m_szPath[0] == L'\0' ){
		return;
	}
	
	// すでに登録されている場合は、除外指定を無視する
	if( -1 == m_cRecentFile.FindItemByPath( pEditInfo->m_szPath ) ){
		int nSize = m_pShareData->m_sHistory.m_aExceptMRU.size();
		for( int i = 0 ; i < nSize; i++ ){
			TCHAR szExceptMRU[_MAX_PATH];
			CFileNameManager::ExpandMetaToFolder( m_pShareData->m_sHistory.m_aExceptMRU[i], szExceptMRU, _countof(szExceptMRU) );
			if( NULL != _tcsistr( pEditInfo->m_szPath,  szExceptMRU) ){
				return;
			}
		}
	}
	EditInfo tmpEditInfo = *pEditInfo;
	tmpEditInfo.m_bIsModified = FALSE; // 変更フラグを無効に

	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFolder[_MAX_PATH + 1];	//	ドライブ＋フォルダ

	_tsplitpath( pEditInfo->m_szPath, szDrive, szDir, NULL, NULL );	//	ドライブとフォルダを取り出す。

	//	Jan.  10, 2006 genta USBメモリはRemovable mediaと認識されるようなので，
	//	一応無効化する．
	//	リムーバブルなら非登録？
	//if (/* 「リムーバブルなら登録しない」オン && */ ! IsLocalDrive( szDrive ) ){
	//	return;
	//}

	//	szFolder作成
	_tcscpy( szFolder, szDrive );
	_tcscat( szFolder, szDir );

	//	Folderを、CMRUFolderに登録
	CMRUFolder cMRUFolder;
	cMRUFolder.Add(szFolder);

	m_cRecentFile.AppendItem( &tmpEditInfo );
	
	::SHAddToRecentDocs( SHARD_PATH, to_wchar(pEditInfo->m_szPath) );
}

/*EOF*/
