//	$Id$
/*!	@file
	@brief タグジャンプリスト

	@author MIK
	$Revision$
*/
/*
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

#include "stdafx.h"
#include "CSortedTagJumpList.h"
#include "my_icmp.h"

/*!
	@date 2005.04.23 genta 管理数の最大値を指定する引数追加
*/
CSortedTagJumpList::CSortedTagJumpList(int max)
	: m_MAX_TAGJUMPLIST( max ),
	  m_pTagjump( NULL ),
	  m_nCount( 0 ),
	  m_bOverflow( false )
{
}

CSortedTagJumpList::~CSortedTagJumpList()
{
	Empty();
}

/*
	指定されたアイテムのメモリを解放する。

	@param[in] item 削除するアイテム
*/
void CSortedTagJumpList::Free( TagJumpInfo* item )
{
	free( item->keyword );
	free( item->filename );
	free( item->note );
	free( item );
	return;
}

/*
	リストをすべて解放する。
*/
void CSortedTagJumpList::Empty( void )
{
	TagJumpInfo*	p;
	TagJumpInfo*	next;
	for( p = m_pTagjump; p; p = next )
	{
		next = p->next;
		Free( p );
	}
	m_pTagjump = NULL;
	m_nCount = 0;
	m_bOverflow = false;
}

/*
	アイテムをソートされた状態でリストに追加する。
	アイテムが最大数を超える場合は、超えるアイテムを削除する。
	
	@param[in] keyword	キーワード
	@param[in] filename	ファイル名
	@param[in] no		行番号
	@param[in] type		種類
	@param[in] note		備考
	@param[in] depth	(さかのぼる)階層
	@return 処理結果
*/
BOOL CSortedTagJumpList::AddParam( TCHAR* keyword, TCHAR* filename, int no, TCHAR type, TCHAR* note, int depth )
{
	TagJumpInfo*	p;
	TagJumpInfo*	prev;
	TagJumpInfo*	item;

	//アイテムを作成する。
	item = (TagJumpInfo*)malloc( sizeof( TagJumpInfo ) );
	if( NULL == item ) return FALSE;
	item->keyword  = _tcsdup( keyword );
	item->filename = _tcsdup( filename );
	item->no       = no;
	item->type     = type;
	item->note     = _tcsdup( note );
	item->depth    = depth;
	item->next     = NULL;

	//文字列長ガード
	if( _tcslen( item->keyword  ) >= MAX_TAG_STRING_LENGTH ) item->keyword[  MAX_TAG_STRING_LENGTH-1 ] = 0;
	if( _tcslen( item->filename ) >= MAX_TAG_STRING_LENGTH ) item->filename[ MAX_TAG_STRING_LENGTH-1 ] = 0;
	if( _tcslen( item->note     ) >= MAX_TAG_STRING_LENGTH ) item->note[     MAX_TAG_STRING_LENGTH-1 ] = 0;

	//アイテムをリストの適当な位置に追加する。
	prev = NULL;
	for( p = m_pTagjump; p; p = p->next )
	{
		if( _tcscmp( p->keyword, item->keyword ) > 0 ) break;
		prev = p;
	}
	item->next = p;
	if( prev ) prev->next = item;
	else       m_pTagjump = item;
	m_nCount++;

	//最大数を超えたら最後のアイテムを削除する。
	if( m_nCount > m_MAX_TAGJUMPLIST )
	{
		prev = NULL;
		for( p = m_pTagjump; p->next; p = p->next ) prev = p;
		if( prev ) prev->next = NULL;
		else       m_pTagjump = NULL;
		Free( p );
		m_nCount--;
		m_bOverflow = true;
	}
	return TRUE;
}

/*
	指定の情報を取得する。

	@param[out] keyword		キーワード
	@param[out] filename	ファイル名
	@param[out] no			行番号
	@param[out] type		種類
	@param[out] note		備考
	@param[out] depth		(さかのぼる)階層
	@return 処理結果

	@note 不要な情報の場合は引数に NULL を指定する。
*/
BOOL CSortedTagJumpList::GetParam( int index, TCHAR* keyword, TCHAR* filename, int* no, TCHAR* type, TCHAR* note, int* depth )
{
	if( keyword  ) _tcscpy( keyword, _T("") );
	if( filename ) _tcscpy( filename, _T("") );
	if( no       ) *no    = 0;
	if( type     ) *type  = 0;
	if( note     ) _tcscpy( note, _T("") );
	if( depth    ) *depth = 0;

	CSortedTagJumpList::TagJumpInfo* p;
	p = GetPtr( index );
	if( NULL != p )
	{
		if( keyword  ) _tcscpy( keyword, p->keyword );
		if( filename ) _tcscpy( filename, p->filename );
		if( no       ) *no    = p->no;
		if( type     ) *type  = p->type;
		if( note     ) _tcscpy( note, p->note );
		if( depth    ) *depth = p->depth;
		return TRUE;
	}
	return FALSE;
}

/*
	指定の情報を構造体ポインタで取得する。
	取得した情報は参照なので解放してはならない。

	@param[in] index 要素番号
	@return タグジャンプ情報
*/
CSortedTagJumpList::TagJumpInfo* CSortedTagJumpList::GetPtr( int index )
{
	TagJumpInfo*	p;
	int	i;
	i = 0;
	for( p = m_pTagjump; p; p = p->next )
	{
		if( index == i ) return p;
		i++;
	}
	return NULL;
}
