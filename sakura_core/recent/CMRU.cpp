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

#include "stdafx.h"
#include "CShareData.h"
#include "CMenuDrawer.h"	//	これでいいのか？
#include "global.h"
#include <stdio.h>
#include "CMRU.h"
#include "recent/CRecent.h"	//履歴の管理	//@@@ 2003.04.08 MIK
#include "util/string_ex2.h"

/*!	コンストラクタ
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
CMRU::CMRU()
{
	//	初期化。
	m_pShareData = CShareData::getInstance()->GetShareData();
}

/*	デストラクタ	*/
CMRU::~CMRU()
{
	m_cRecentFile.Terminate();
}

/*!
	ファイル履歴メニューの作成
	
	@param pCMenuDrawer [in] (out?) メニュー作成で用いるMenuDrawer
	
	@author Norio Nakantani
	@return 生成したメニューのハンドル
*/
HMENU CMRU::CreateMenu( CMenuDrawer* pCMenuDrawer )
{
	HMENU	hMenuPopUp;
	TCHAR	szFile2[_MAX_PATH * 2];	//	全部&でも問題ないように。
	TCHAR	szMemu[300];			//	メニューキャプション
	int		i;
	bool	bFavorite;
	EditInfo	*p;

	CShareData::getInstance()->TransformFileName_MakeCache();

	//	空メニューを作る
	hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	for( i = 0; i < m_cRecentFile.GetItemCount(); ++i )
	{
		//	「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if ( i >= m_cRecentFile.GetViewCount() ) break;
		
		/* MRUリストの中にある開かれていないファイル */

		p = m_cRecentFile.GetItem( i );
		
		CShareData::getInstance()->GetTransformFileNameFast( p->m_szPath, szMemu, _MAX_PATH );
		//	&を&&に置換。
		//	Jan. 19, 2002 genta
		dupamp( szMemu, szFile2 );
		
		bFavorite = m_cRecentFile.IsFavorite( i );
		//	j >= 10 + 26 の時の考慮を省いた(に近い)がファイルの履歴MAXを36個にしてあるので事実上OKでしょう
		auto_sprintf(
			szMemu,
			_T("&%tc %ts%ts"),
			(i < 10) ? (_T('0') + i) : (_T('A') + i - 10), 
			(!m_pShareData->m_Common.m_sWindow.m_bMenuIcon && bFavorite) ? _T("★ ") : _T(""),
			szFile2
		);

		//	ファイル名のみ必要。
		//	文字コード表記
		if(IsValidCodeTypeExceptSJIS(p->m_nCharCode)){
			_tcscat( szMemu, CCodeTypeName(p->m_nCharCode).Bracket() );
		}

		//	メニューに追加。
		pCMenuDrawer->MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELMRU + i, szMemu, TRUE,
			bFavorite ? F_FAVORITE : -1 );
	}
	return hMenuPopUp;
}

BOOL CMRU::DestroyMenu( HMENU hMenuPopUp )
{
	return ::DestroyMenu( hMenuPopUp );
}

/*!
	ファイル履歴の一覧を返す
	
	@param ppszMRU [out] 文字列へのポインタリストを格納する．
	最後の要素の次にはNULLが入る．
	予め呼び出す側で最大値+1の領域を確保しておくこと．
*/
std::vector<LPCTSTR> CMRU::GetPathList() const
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
int CMRU::Length(void)
{
	return m_cRecentFile.GetItemCount();
}

/*!
	ファイル履歴のクリア
*/
void CMRU::ClearAll(void)
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
bool CMRU::GetEditInfo( int num, EditInfo* pfi )
{
	EditInfo*	p = m_cRecentFile.GetItem( num );
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
bool CMRU::GetEditInfo( const TCHAR* pszPath, EditInfo* pfi )
{
	EditInfo*	p = m_cRecentFile.GetItem( m_cRecentFile.FindItemByPath( pszPath ) );
	if( NULL == p ) return false;

	*pfi = *p;

	return true;
}

/*!	@brief MRUリストへの登録

	@param pFileInfo [in] 追加するファイルの情報

	該当ファイルがリムーバブルディスク上にある場合にはMRU Listへの登録は行わない。

	@date 2001.03.29 MIK リムーバブルディスク上のファイルを登録しないようにした。
	@date 2001.12.26 YAZAKI CShareData::AddMRUListから移動
*/
void CMRU::Add( EditInfo* pEditInfo )
{
	//	ファイル名が無ければ無視
	if( NULL == pEditInfo || 0 == _tcslen( pEditInfo->m_szPath ) ){
		return;
	}

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

	m_cRecentFile.AppendItem( pEditInfo );
}

/*EOF*/
