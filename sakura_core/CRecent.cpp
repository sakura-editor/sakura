/*!	@file
	@brief 最近使ったリスト

	お気に入りを含む最近使ったリストを管理する。

	@author MIK
	@date Apr. 05, 2003
	@date Apr. 03, 2005

*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK

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


#include "StdAfx.h"
#include "global.h"
#include "CShareData.h"
#include "CRecent.h"
#include <string.h>
#include "my_icmp.h"
#include <stddef.h>

CRecent::CRecent()
{
	//	初期化。
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_bCreate = false;

	m_nArrayCount  = 0;
	m_nItemSize    = 0;
	m_nOffset      = 0;
	m_nCmpSize     = 0;
	m_nCmpType     = 0;
	m_puUserItemData     = NULL;
	m_pnUserItemCount    = NULL;
	m_pnUserViewCount    = NULL;
	m_pbUserItemFavorite = NULL;
}

CRecent::~CRecent()
{
	Terminate();
}

/*
	初期化済みか調べる。
*/
bool CRecent::IsAvailable( void ) const
{
	if( ! m_bCreate ) return false;

	if( NULL == m_puUserItemData
	 || NULL == m_pnUserItemCount )
	{
		const_cast<CRecent*>(this)->Terminate();
		return false;
	}

	//データ破壊時のリカバリをやってみたりする
	if( *m_pnUserItemCount < 0             ) *m_pnUserItemCount = 0;
	if( *m_pnUserItemCount > m_nArrayCount ) *m_pnUserItemCount = m_nArrayCount;

	if( m_pnUserViewCount )
	{
		if( *m_pnUserViewCount < 0             ) *m_pnUserViewCount = 0;
		if( *m_pnUserViewCount > m_nArrayCount ) *m_pnUserViewCount = m_nArrayCount;
	}

	return true;
}

/*
	初期生成処理

	@param	pszItemArray	アイテム配列へのポインタ
	@param	pnItemCount		アイテム個数へのポインタ
	@param	pbItemFavorite	お気に入りへのポインタ(NULL許可)
	@param	nArrayCount		最大管理可能なアイテム数
	@param	*pnViewCount	表示個数(NULL許可)
	@param	nItemSize		ユーザデータのバイトサイズ
	@param	nOffset			比較位置
	@param	nCmpSize		比較サイズ(strn*cmp, mem*cmp で必須)
	@param	nCmpType		比較タイプ

	@note
	nCmpType = strcmp, stricmp のときに nCmpSize = 0 を指定すると、AppendItem 
	でのデータが文字列であると認識して strcpy をする。
	他の場合は memcpy で nItemSize 分をコピーする。
	
	pnViewCount = NULL にすると、擬似的に nViewCount == nArrayCount になる。
*/
bool CRecent::Create( 
	char	*pszItemArray,	//アイテム配列へのポインタ
	int		*pnItemCount,	//アイテム個数へのポインタ
	bool	*pbItemFavorite,	//お気に入りへのポインタ
	int		nArrayCount, 
	int		*pnViewCount, 
	int		nItemSize, 
	int		nOffset, 
	int		nCmpSize
)
{
	Terminate();

	//パラメータチェック
	if( NULL == pszItemArray ) return false;
	if( NULL == pnItemCount ) return false;
	//if( NULL == pnViewCount ) return false;
	//if( NULL == pbItemFavorite ) return false;

	if( nArrayCount <= 0 ) return false;
	if( pnViewCount )
	{
		if( *pnViewCount < 0 || nArrayCount < *pnViewCount ) return false;
	}
	if( nItemSize <= 0 || nItemSize < nOffset ) return false;
	if( nCmpSize < 0 || nItemSize < nOffset + nCmpSize ) return false;

	m_puUserItemData     = pszItemArray;
	m_pnUserItemCount    = pnItemCount;
	m_pnUserViewCount    = pnViewCount;
	m_pbUserItemFavorite = pbItemFavorite;

	m_nArrayCount = nArrayCount;
	m_nItemSize   = nItemSize;
	m_nOffset     = nOffset;
	m_nCmpSize    = nCmpSize;

	m_bCreate = true;

	//個別に操作されていたときのための対応
	UpdateView();

	return true;
}

/*
	終了処理
*/
void CRecent::Terminate( void )
{
	m_bCreate = false;

	m_puUserItemData     = NULL;
	m_pnUserItemCount    = NULL;
	m_pnUserViewCount    = NULL;
	m_pbUserItemFavorite = NULL;

	m_nArrayCount  = 0;
	m_nItemSize    = 0;
	m_nOffset      = 0;
	m_nCmpSize     = 0;
	m_nCmpType     = 0;

	return;
}

/*
	管理されているアイテムのうちの表示個数を変更する。

	@note	お気に入りは可能な限り表示内に移動させる。
*/
bool CRecent::ChangeViewCount( int nViewCount )
{
	int	i;
	int	nIndex;

	//範囲外ならエラー
	if( ! IsAvailable() ) return false;
	if( nViewCount < 0 || nViewCount > m_nArrayCount ) return false;

	//表示個数を更新する。
	if( m_pnUserViewCount )
	{
		*m_pnUserViewCount = nViewCount;
	}

	//範囲内にすべて収まっているので何もしなくてよい。
	if( nViewCount >= *m_pnUserItemCount ) return true;

	//最も古いお気に入りを探す。
	i = GetOldestItem( *m_pnUserItemCount - 1, true );
	if( -1 == i ) return true;	//ないので何もしないで終了

	//表示外アイテムを表示内に移動する。
	for( ; i >= nViewCount; i-- )
	{
		if( IsFavorite( i ) )
		{
			//カレント位置から上に通常アイテムを探す
			nIndex = GetOldestItem( i - 1, false );
			if( -1 == nIndex ) break;	//もう1個もない

			//見つかったアイテムをカレント位置に移動する
			MoveItem( nIndex, i );
		}
	}

	return true;
}

/*
	リストを更新する。
*/
bool CRecent::UpdateView( void )
{
	int	nViewCount;

	//範囲外ならエラー
	if( ! IsAvailable() ) return false;

	if( m_pnUserViewCount ) nViewCount = *m_pnUserViewCount;
	else                    nViewCount = m_nArrayCount;

	return ChangeViewCount( nViewCount );
}

/*
	お気に入り状態を設定する。

	true	設定
	false	解除
*/
bool CRecent::SetFavorite( int nIndex, bool bFavorite )
{
	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;
	if( NULL == m_pbUserItemFavorite ) return false;

	m_pbUserItemFavorite[nIndex] = bFavorite;

	return true;
}

/*
	すべてのお気に入り状態を解除する。
*/
void CRecent::ResetAllFavorite( void )
{
	int	i;

	if( ! IsAvailable() ) return;

	for( i = 0; i < *m_pnUserItemCount; i++ )
	{
		(void)SetFavorite( i, false );
	}

	return;
}

/*
	お気に入り状態かどうか調べる。

	true	お気に入り
	false	通常
*/
bool CRecent::IsFavorite( int nIndex ) const
{
	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;
	if( NULL == m_pbUserItemFavorite ) return false;

	return m_pbUserItemFavorite[nIndex];
}

/*
	アイテムリストからもっとも古い｛お気に入り・通常｝のアイテムを探す。

	bFavorite=true	お気に入りの中から探す
	bFavorite=false	通常の中から探す
*/
int CRecent::GetOldestItem( int nIndex, bool bFavorite )
{
	int	i;

	if( ! IsAvailable() ) return -1;
	if( nIndex >= *m_pnUserItemCount ) nIndex = *m_pnUserItemCount - 1;

	for( i = nIndex; i >= 0; i-- )
	{
		if( IsFavorite( i ) == bFavorite ) return i;
	}

	return -1;
}

/*
	アイテムを移動する。
*/
bool CRecent::MoveItem( int nSrcIndex, int nDstIndex )
{
	char	*pri;
	int	i;
	bool	bFavorite;

	if( ! IsAvailable() ) return false;
	if( nSrcIndex < 0 || nSrcIndex >= *m_pnUserItemCount ) return false;
	if( nDstIndex < 0 || nDstIndex >= *m_pnUserItemCount ) return false;

	if( nSrcIndex == nDstIndex ) return true;

	pri = new char[ m_nItemSize ];
	if( NULL == pri ) return false;

	//移動する情報を退避
	memcpy( pri, GetArrayOffset( nSrcIndex ), m_nItemSize );
	bFavorite = IsFavorite( nSrcIndex );

	if( nSrcIndex < nDstIndex )
	{
		for( i = nSrcIndex; i < nDstIndex; i++ )
		{
			CopyItem( i + 1, i );
		}
	}
	else
	{
		for( i = nSrcIndex; i > nDstIndex; i-- )
		{
			CopyItem( i - 1, i );
		}
	}

	//新しい位置に格納
	memcpy( GetArrayOffset( nDstIndex ), pri, m_nItemSize );
	(void)SetFavorite( nDstIndex, bFavorite );

	delete [] pri;

	return true;
}

bool CRecent::CopyItem( int nSrcIndex, int nDstIndex )
{
	if( ! IsAvailable() ) return false;
	if( nSrcIndex < 0 || nSrcIndex >= m_nArrayCount ) return false;
	if( nDstIndex < 0 || nDstIndex >= m_nArrayCount ) return false;

	if( nSrcIndex == nDstIndex ) return true;

	memcpy( GetArrayOffset( nDstIndex ), GetArrayOffset( nSrcIndex ), m_nItemSize );

	//(void)SetFavorite( nDstIndex, IsFavorite( nSrcIndex ) );
	//内部処理しないとだめ。
	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[nDstIndex] = m_pbUserItemFavorite[nSrcIndex];

	return true;
}

/*
	アイテムをゼロクリアする。
*/
void CRecent::ZeroItem( int nIndex )
{
	if( ! IsAvailable() ) return;
	if( nIndex < 0 || nIndex >= m_nArrayCount ) return;

	memset( GetArrayOffset( nIndex ), 0, m_nItemSize );

	//(void)SetFavorite( nIndex, false );
	//内部処理しないとだめ。
	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[nIndex] = false;

	return;
}

/*
	アイテムを削除する。
*/
bool CRecent::DeleteItem( int nIndex )
{
	int	i;

	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;

	ZeroItem( nIndex );

	//以降のアイテムを前に詰める。
	for( i = nIndex; i < *m_pnUserItemCount - 1; i++ )
	{
		CopyItem( i + 1, i );
	}
	ZeroItem( i );

	*m_pnUserItemCount -= 1;

	return true;
}

/*
	すべてのアイテムを削除する。

	@note	ゼロクリアを可能とするため、すべてが対象になる。
*/
void CRecent::DeleteAllItem( void )
{
	int	i;

	if( ! IsAvailable() ) return;

	for( i = 0; i < m_nArrayCount; i++ )
	{
		ZeroItem( i );
	}

	*m_pnUserItemCount = 0;

	return;
}

/*
	アイテムを取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const char *CRecent::GetItem( int nIndex ) const
{
	if( ! IsAvailable() ) return NULL;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return NULL;

	return GetArrayOffset( nIndex );
}

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const char *CRecent::GetDataOfItem( int nIndex ) const
{
	const char	*p;

	if( ! IsAvailable() ) return NULL;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return NULL;

	p = GetItem( nIndex );
	if( NULL == p ) return NULL;

	return &p[ m_nOffset ];
}

/*
	アイテムを先頭に追加する。

	@note	すでに登録済みの場合は先頭に移動する。
	@note	いっぱいのときは最古のアイテムを削除する。
	@note	お気に入りは削除されない。
*/
bool CRecent::AppendItem( const char *pszItemData )
{
	int		nIndex;
	int		i;
	char	*p;

	if( ! IsAvailable() ) return false;
	if( NULL == pszItemData ) return false;

	//登録済みか調べる。
	nIndex = FindItem( &pszItemData[ m_nOffset ] );
	if( nIndex >= 0 )
	{
		//最新の情報に更新する。
		if( m_nCmpSize > 0 )
			memcpy( GetArrayOffset( nIndex ), pszItemData, m_nItemSize );
		else	//文字列として受け取る。
		{
			p = GetArrayOffset( nIndex );
			strncpy( p, pszItemData, m_nItemSize );
			p[ m_nItemSize - 1 ] = '\0';
			
		}

		//先頭に持ってくる。
		(void)MoveItem( nIndex, 0 );
		goto reconfigure;
	}

	//いっぱいのときは最古の通常アイテムを削除する。
	if( m_nArrayCount <= *m_pnUserItemCount )
	{
		nIndex = GetOldestItem( *m_pnUserItemCount - 1, false );
		if( -1 == nIndex )
		{
			return false;
		}

		(void)DeleteItem( nIndex );
	}

	for( i = *m_pnUserItemCount; i > 0; i-- )
	{
		CopyItem( i - 1, i );
	}

	if( m_nCmpSize > 0 )
		memcpy( GetArrayOffset( 0 ), pszItemData, m_nItemSize );
	else	//文字列として受け取る。
	{
		p = GetArrayOffset( 0 );
		strncpy( p, pszItemData, m_nItemSize );
		p[ m_nItemSize - 1 ] = '\0';
	}

	//(void)SetFavorite( 0, true );
	//内部処理しないとだめ。
	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[0] = false;

	*m_pnUserItemCount += 1;


reconfigure:
	//お気に入りを表示内に移動する。
	if( m_pnUserViewCount )
	{
		(void)ChangeViewCount( *m_pnUserViewCount );
	}
	return true;
}

