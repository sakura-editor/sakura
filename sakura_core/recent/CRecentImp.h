/*! @file */
// 各CRecent実装クラスのベースクラス
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTIMP_B18E6196_5684_44E4_91E0_ADB1542BF7E1_H_
#define SAKURA_CRECENTIMP_B18E6196_5684_44E4_91E0_ADB1542BF7E1_H_
#pragma once

#include "recent/CRecent.h"

template < class DATA_TYPE, class RECEIVE_TYPE = const DATA_TYPE* >
class CRecentImp : public CRecent{
	using Me = CRecentImp<DATA_TYPE, RECEIVE_TYPE>;

	typedef DATA_TYPE							DataType;
	typedef RECEIVE_TYPE						ReceiveType;

public:
	CRecentImp(){ Terminate(); }
	CRecentImp(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CRecentImp(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CRecentImp(){ Terminate(); }

protected:
	//生成
	bool Create(
		DataType*		pszItemArray,	//!< アイテム配列へのポインタ
		size_t			nTextMaxLength,	//!< 最大テキスト長(終端含む)
		int*			pnItemCount,	//!< アイテム個数へのポインタ
		bool*			pbItemFavorite,	//!< お気に入りへのポインタ(NULL許可)
		int				nArrayCount,	//!< 最大管理可能なアイテム数
		int*			pnViewCount		//!< 表示個数(NULL許可)
	);
public:
	void Terminate();
	bool IsAvailable() const;
	void _Recovery();

	//更新
	bool ChangeViewCount( int nViewCount );	//表示数の変更
	bool UpdateView();

	//プロパティ取得系
	int GetArrayCount() const { return m_nArrayCount; }	//最大要素数
	int GetItemCount() const { return ( IsAvailable() ? *m_pnUserItemCount : 0); }	//登録アイテム数
	int GetViewCount() const { return ( IsAvailable() ? (m_pnUserViewCount ? *m_pnUserViewCount : m_nArrayCount) : 0); }	//表示数

	//お気に入り制御系
	bool SetFavorite( int nIndex, bool bFavorite = true);	//お気に入りに設定
	bool ResetFavorite( int nIndex ) { return SetFavorite( nIndex, false ); }	//お気に入りを解除
	void ResetAllFavorite();			//お気に入りをすべて解除
	bool IsFavorite( int nIndex ) const;			//お気に入りか調べる

	//アイテム制御
	bool AppendItem( ReceiveType pItemData );	//アイテムを先頭に追加
	bool AppendItemText( LPCWSTR pszText );
	bool EditItemText( int nIndex, LPCWSTR pszText );
	bool DeleteItem( int nIndex );				//アイテムをクリア
	bool DeleteItem( ReceiveType pItemData )
	{
		return DeleteItem( FindItem( pItemData ) );
	}
	bool DeleteItemsNoFavorite();			//お気に入り以外のアイテムをクリア
	void DeleteAllItem();					//アイテムをすべてクリア

	//アイテム取得
	const DataType* GetItem( int nIndex ) const;
	DataType* GetItem( int nIndex ){ return const_cast<DataType*>(static_cast<const Me*>(this)->GetItem(nIndex)); }
	int FindItem( ReceiveType pItemData ) const;
	bool MoveItem( int nSrcIndex, int nDstIndex );	//アイテムを移動

	//オーバーライド用インターフェース
	virtual int  CompareItem( const DataType* p1, ReceiveType p2 ) const = 0;
	virtual void CopyItem( DataType* dst, ReceiveType src ) const = 0;
	virtual bool DataToReceiveType( ReceiveType* dst, const DataType* src ) const = 0;
	virtual bool TextToDataType( DataType* dst, LPCWSTR pszText ) const = 0;
	virtual bool ValidateReceiveType( ReceiveType p ) const = 0;

	//実装補助
private:
	const DataType* GetItemPointer(int nIndex) const;
	DataType* GetItemPointer(int nIndex){ return const_cast<DataType*>(static_cast<const Me*>(this)->GetItemPointer(nIndex)); }
	void   ZeroItem( int nIndex );	//アイテムをゼロクリアする
	int    GetOldestItem( int nIndex, bool bFavorite );	//最古のアイテムを探す
	bool   CopyItem( int nSrcIndex, int nDstIndex );

protected:
	//内部フラグ
	bool		m_bCreate;				//!< Create済みか

	//外部参照
	DataType*	m_puUserItemData;		//!< アイテム配列へのポインタ
	int*		m_pnUserItemCount;		//!< アイテム個数へのポインタ
	bool*		m_pbUserItemFavorite;	//!< お気に入りへのポインタ (NULL許可)
	int			m_nArrayCount;			//!< 最大管理可能なアイテム数
	int*		m_pnUserViewCount;		//!< 表示個数 (NULL許可)
	size_t		m_nTextMaxLength;		//!< 最大テキスト長(終端含む)
};

#endif /* SAKURA_CRECENTIMP_B18E6196_5684_44E4_91E0_ADB1542BF7E1_H_ */
