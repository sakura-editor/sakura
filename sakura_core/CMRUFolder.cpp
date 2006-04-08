/*!	@file
	@brief MRUリストと呼ばれるリストを管理する

	@author YAZAKI
	@date 2001/12/23  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, YAZAKI
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CShareData.h"
#include "CMenuDrawer.h"	//	これでいいのか？
#include "CMRUFolder.h"
#include "CRecent.h"	//お気に入り	//@@@ 2003.04.08 MIK
#include "etc_uty.h"
#include "my_icmp.h" // 2002/11/30 Moca 追加

/*!	コンストラクタ

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
CMRUFolder::CMRUFolder()
{
	//	初期化。
	m_pShareData = CShareData::getInstance()->GetShareData();

	//お気に入り	//@@@ 2003.04.08 MIK
	(void)m_cRecent.EasyCreate( RECENT_FOR_FOLDER );
}

/*	デストラクタ	*/
CMRUFolder::~CMRUFolder()
{
	m_cRecent.Terminate();
}

HMENU CMRUFolder::CreateMenu( CMenuDrawer* pCMenuDrawer )
{
	HMENU	hMenuPopUp;
	char	szFolder2[_MAX_PATH * 2];	//	全部&でも問題ないように :-)
	char	szMemu[300];				//	メニューキャプション
	int		i;
	bool	bFavorite;

	hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	CShareData::getInstance()->TransformFileName_MakeCache();
	for( i = 0; i < m_cRecent.GetItemCount(); ++i )
	{
		//	「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if ( i >= m_cRecent.GetViewCount() ) break;

		CShareData::getInstance()->GetTransformFileNameFast( m_cRecent.GetDataOfItem( i ), szMemu, _MAX_PATH );
		//	&を&&に置換。
		//	Jan. 19, 2002 genta
		dupamp( szMemu, szFolder2 );

		bFavorite = m_cRecent.IsFavorite( i );
		//	j >= 10 + 26 の時の考慮を省いた(に近い)がフォルダの履歴MAXを36個にしてあるので事実上OKでしょう
		wsprintf( szMemu, "&%c %s%s", 
			(i < 10) ? ('0' + i) : ('A' + i - 10), 
			(FALSE == m_pShareData->m_Common.m_bMenuIcon && bFavorite) ? "★ " : "",
			szFolder2 );

		//	メニューに追加
		pCMenuDrawer->MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELOPENFOLDER + i, szMemu, TRUE,
			bFavorite ? F_FAVORITE : -1 );
	}
	return hMenuPopUp;
}

void CMRUFolder::GetPathList( char** ppszOPENFOLDER )
{
	int	i;

	for( i = 0; i < m_cRecent.GetItemCount(); ++i )
	{
		//	「共通設定」→「全般」→「フォルダの履歴MAX」を反映
		if ( i >= m_cRecent.GetViewCount() ) break;
		ppszOPENFOLDER[i] = (char*)m_cRecent.GetDataOfItem( i );
	}
	ppszOPENFOLDER[i] = NULL;
}

int CMRUFolder::Length()
{
	return m_cRecent.GetItemCount();
}

void CMRUFolder::ClearAll()
{
	m_cRecent.DeleteAllItem();
}

/*	@brief 開いたフォルダ リストへの登録

	@date 2001.12.26  CShareData::AddOPENFOLDERListから移動した。（YAZAKI）
*/
void CMRUFolder::Add( const char* pszFolder )
{
	if( NULL == pszFolder
	 || 0 == strlen( pszFolder ) )
	{	//	長さが0なら排除。
		return;
	}

	(void)m_cRecent.AppendItem( pszFolder );
}

const char* CMRUFolder::GetPath(int num)
{
	return m_cRecent.GetDataOfItem( num );
}

