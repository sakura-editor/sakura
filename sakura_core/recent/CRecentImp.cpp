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

#include "StdAfx.h"
#include "CRecentImp.h"

#include "env/CAppNodeManager.h" // EditNode
#include "EditInfo.h" // EditInfo

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	初期生成処理

	@note
	nCmpType = strcmp, stricmp のときに nCmpSize = 0 を指定すると、AppendItem 
	でのデータが文字列であると認識して strcpy をする。
	他の場合は memcpy で nItemSize 分をコピーする。
	
	pnViewCount = NULL にすると、擬似的に nViewCount == nArrayCount になる。
*/
template <class T, class S>
bool CRecentImp<T, S>::Create( 
	DataType*		pszItemArray,	//!< アイテム配列へのポインタ
	size_t			nTextMaxLength,	//!< 最大テキスト長(終端含む)
	int*			pnItemCount,	//!< アイテム個数へのポインタ
	bool*			pbItemFavorite,	//!< お気に入りへのポインタ(NULL許可)
	int				nArrayCount,	//!< 最大管理可能なアイテム数
	int*			pnViewCount		//!< 表示個数(NULL許可)
)
{
	Terminate();

	//パラメータチェック
	if( NULL == pszItemArray ) return false;
	if( NULL == pnItemCount ) return false;
	if( nArrayCount <= 0 ) return false;
	if( pnViewCount && (*pnViewCount < 0 || nArrayCount < *pnViewCount) ) return false;

	//各パラメータ格納
	m_puUserItemData		= pszItemArray;
	m_nTextMaxLength		= nTextMaxLength;
	m_pnUserItemCount		= pnItemCount;
	m_pbUserItemFavorite	= pbItemFavorite;
	m_nArrayCount			= nArrayCount;
	m_pnUserViewCount		= pnViewCount;
	m_bCreate = true;

	//個別に操作されていたときのための対応
	UpdateView();

	return true;
}

/*
	終了処理
*/
template <class T, class S>
void CRecentImp<T, S>::Terminate()
{
	m_bCreate = false;

	m_puUserItemData     = NULL;
	m_pnUserItemCount    = NULL;
	m_pnUserViewCount    = NULL;
	m_pbUserItemFavorite = NULL;

	m_nArrayCount  = 0;
}


/*
	初期化済みか調べる。
*/
template <class T, class S>
bool CRecentImp<T, S>::IsAvailable() const
{
	if(!m_bCreate)return false;

	//データ破壊時のリカバリをやってみたりする
	const_cast<CRecentImp*>(this)->_Recovery(); 

	return true;
}

//! リカバリ
template <class T, class S>
void CRecentImp<T, S>::_Recovery()
{
	if( *m_pnUserItemCount < 0             ) *m_pnUserItemCount = 0;
	if( *m_pnUserItemCount > m_nArrayCount ) *m_pnUserItemCount = m_nArrayCount;

	if( m_pnUserViewCount )
	{
		if( *m_pnUserViewCount < 0             ) *m_pnUserViewCount = 0;
		if( *m_pnUserViewCount > m_nArrayCount ) *m_pnUserViewCount = m_nArrayCount;
	}
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        お気に入り                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	お気に入り状態を設定する。

	true	設定
	false	解除
*/
template <class T, class S>
bool CRecentImp<T, S>::SetFavorite( int nIndex, bool bFavorite )
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
template <class T, class S>
void CRecentImp<T, S>::ResetAllFavorite()
{
	if( ! IsAvailable() ) return;

	for( int i = 0; i < *m_pnUserItemCount; i++ )
	{
		SetFavorite( i, false );
	}
}

/*
	お気に入り状態かどうか調べる。

	true	お気に入り
	false	通常
*/
template <class T, class S>
bool CRecentImp<T, S>::IsFavorite( int nIndex ) const
{
	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;
	if( NULL == m_pbUserItemFavorite ) return false;

	return m_pbUserItemFavorite[nIndex];
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       アイテム制御                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムを先頭に追加する。

	@note	すでに登録済みの場合は先頭に移動する。
	@note	いっぱいのときは最古のアイテムを削除する。
	@note	お気に入りは削除されない。
*/
template <class T, class S>
bool CRecentImp<T, S>::AppendItem( ReceiveType pItemData )
{
	int		i;

	if( !IsAvailable() ) return false;
	if( !pItemData ) return false;
	if( false == ValidateReceiveType(pItemData) ) return false;

	//登録済みか調べる。
	int	nIndex = FindItem( pItemData );
	if( nIndex >= 0 )
	{
		CopyItem( GetItemPointer(nIndex), pItemData );

		//先頭に持ってくる。
		MoveItem( nIndex, 0 );
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

		DeleteItem( nIndex );
	}

	for( i = *m_pnUserItemCount; i > 0; i-- )
	{
		CopyItem( i - 1, i );
	}

	CopyItem( GetItemPointer(0), pItemData );

	//(void)SetFavorite( 0, true );
	//内部処理しないとだめ。
	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[0] = false;

	*m_pnUserItemCount += 1;


reconfigure:
	//お気に入りを表示内に移動する。
	if( m_pnUserViewCount )
	{
		ChangeViewCount( *m_pnUserViewCount );
	}
	return true;
}


template <class T, class S>
bool CRecentImp<T, S>::AppendItemText( LPCTSTR pText )
{
	DataType data;
	ReceiveType receiveData;
	if( !TextToDataType( &data, pText ) ){
		return false;
	}
	if( !DataToReceiveType( &receiveData, &data ) ){
		return false;
	}
	int findIndex = FindItem( receiveData );
	if( -1 != findIndex ){
		return false;
	}
	return AppendItem( receiveData );
}

template <class T, class S>
bool CRecentImp<T, S>::EditItemText( int nIndex, LPCTSTR pText )
{
	DataType data;
	ReceiveType receiveData;
	memcpy_raw( &data, GetItemPointer( nIndex ), sizeof(data) );
	if( !TextToDataType( &data, pText ) ){
		return false;
	}
	if( !DataToReceiveType( &receiveData, &data ) ){
		return false;
	}
	int findIndex = FindItem( receiveData );
	if( -1 != findIndex && nIndex != findIndex ){
		// 重複不可。ただし同じ場合は大文字小文字の変更かもしれないのでOK
		return false;
	}
	CopyItem( GetItemPointer(nIndex), receiveData );
	return true;
}


/*
	アイテムをゼロクリアする。
*/
template <class T, class S>
void CRecentImp<T, S>::ZeroItem( int nIndex )
{
	if( ! IsAvailable() ) return;
	if( nIndex < 0 || nIndex >= m_nArrayCount ) return;

	memset_raw( GetItemPointer( nIndex ), 0, sizeof(DataType) );

	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[nIndex] = false;

	return;
}

/*
	アイテムを削除する。
*/
template <class T, class S>
bool CRecentImp<T, S>::DeleteItem( int nIndex )
{
	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;

	ZeroItem( nIndex );

	//以降のアイテムを前に詰める。
	int i;
	for( i = nIndex; i < *m_pnUserItemCount - 1; i++ )
	{
		CopyItem( i + 1, i );
	}
	ZeroItem( i );

	*m_pnUserItemCount -= 1;

	return true;
}

/*
	お気に入り以外のアイテムを削除する。
*/
template <class T, class S>
bool CRecentImp<T, S>::DeleteItemsNoFavorite()
{
	if( ! IsAvailable() ) return false;

	bool bDeleted = false;
	int i;
	for( i = *m_pnUserItemCount - 1; 0 <= i; i-- )
	{
		if( false == IsFavorite( i ) )
		{
			if( DeleteItem( i ) )
			{
				bDeleted = true;
			}
		}
	}

	return bDeleted;
}

/*
	すべてのアイテムを削除する。

	@note	ゼロクリアを可能とするため、すべてが対象になる。
*/
template <class T, class S>
void CRecentImp<T, S>::DeleteAllItem()
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
	アイテムを移動する。
*/
template <class T, class S>
bool CRecentImp<T, S>::MoveItem( int nSrcIndex, int nDstIndex )
{
	int	i;
	bool	bFavorite;

	if( ! IsAvailable() ) return false;
	if( nSrcIndex < 0 || nSrcIndex >= *m_pnUserItemCount ) return false;
	if( nDstIndex < 0 || nDstIndex >= *m_pnUserItemCount ) return false;

	if( nSrcIndex == nDstIndex ) return true;

	DataType pri;

	//移動する情報を退避
	memcpy_raw( &pri, GetItemPointer( nSrcIndex ), sizeof(pri) );
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
	memcpy_raw( GetItemPointer( nDstIndex ), &pri, sizeof(pri) );
	SetFavorite( nDstIndex, bFavorite );


	return true;
}

template <class T, class S>
bool CRecentImp<T, S>::CopyItem( int nSrcIndex, int nDstIndex )
{
	if( ! IsAvailable() ) return false;
	if( nSrcIndex < 0 || nSrcIndex >= m_nArrayCount ) return false;
	if( nDstIndex < 0 || nDstIndex >= m_nArrayCount ) return false;

	if( nSrcIndex == nDstIndex ) return true;

	memcpy_raw( GetItemPointer( nDstIndex ), GetItemPointer( nSrcIndex ), sizeof(DataType) );

	//(void)SetFavorite( nDstIndex, IsFavorite( nSrcIndex ) );
	//内部処理しないとだめ。
	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[nDstIndex] = m_pbUserItemFavorite[nSrcIndex];

	return true;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       アイテム取得                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, class S>
const T* CRecentImp<T, S>::GetItem( int nIndex ) const
{
	if(!IsAvailable() || nIndex<0 || nIndex>=*m_pnUserItemCount)return NULL;
	return &m_puUserItemData[nIndex];
}

template <class T, class S>
const T* CRecentImp<T, S>::GetItemPointer(int nIndex) const
{
	if(!IsAvailable() || nIndex<0 || nIndex>=m_nArrayCount)return NULL;
	return &m_puUserItemData[nIndex];
}

/*
	アイテムを検索する。
*/
template <class T, class S>
int CRecentImp<T, S>::FindItem( ReceiveType pItemData ) const
{
	if( !IsAvailable() ) return -1;
	if( !pItemData ) return -1;

	for( int i = 0; i < *m_pnUserItemCount; i++ )
	{
		if( CompareItem(GetItemPointer(i), pItemData) == 0 )return i;
	}

	return -1;
}

/*
	アイテムリストからもっとも古い｛お気に入り・通常｝のアイテムを探す。

	bFavorite=true	お気に入りの中から探す
	bFavorite=false	通常の中から探す
*/
template <class T, class S>
int CRecentImp<T, S>::GetOldestItem( int nIndex, bool bFavorite )
{
	if( ! IsAvailable() ) return -1;
	if( nIndex >= *m_pnUserItemCount ) nIndex = *m_pnUserItemCount - 1;

	for( int i = nIndex; i >= 0; i-- )
	{
		if( IsFavorite( i ) == bFavorite ) return i;
	}

	return -1;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	管理されているアイテムのうちの表示個数を変更する。

	@note	お気に入りは可能な限り表示内に移動させる。
*/
template <class T, class S>
bool CRecentImp<T, S>::ChangeViewCount( int nViewCount )
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
template <class T, class S>
bool CRecentImp<T, S>::UpdateView()
{
	int	nViewCount;

	//範囲外ならエラー
	if( ! IsAvailable() ) return false;

	if( m_pnUserViewCount ) nViewCount = *m_pnUserViewCount;
	else                    nViewCount = m_nArrayCount;

	return ChangeViewCount( nViewCount );
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      インスタンス化                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
template class CRecentImp<CCmdString, LPCTSTR>;
template class CRecentImp<EditNode>;
template class CRecentImp<EditInfo>;
template class CRecentImp<CPathString, LPCTSTR>;
template class CRecentImp<CGrepFileString, LPCTSTR>;
#ifndef __MINGW32__
template class CRecentImp<CMetaPath, LPCTSTR>;
template class CRecentImp<CGrepFolderString, LPCTSTR>;
template class CRecentImp<CSearchString, LPCWSTR>;
template class CRecentImp<CTagjumpKeywordString, LPCWSTR>;
template class CRecentImp<CCurDirString, LPCTSTR>;
#endif
#if !defined(__MINGW32__) || (defined(__MINGW32__) && !defined(UNICODE))
template class CRecentImp<CReplaceString, LPCWSTR>;
#endif
