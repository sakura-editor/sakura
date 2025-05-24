/*!	@file
	@brief タグジャンプリスト

	@author MIK
	@date 2005.3.31
*/
/*
	Copyright (C) 2005, MIK, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CSORTEDTAGJUMPLIST_54B68DB2_20F2_4F62_BB2C_47629293620F_H_
#define SAKURA_CSORTEDTAGJUMPLIST_54B68DB2_20F2_4F62_BB2C_47629293620F_H_
#pragma once

#include "basis/primitive.h"
#include "util/design_template.h"

#define MAX_TAG_STRING_LENGTH _MAX_PATH	//管理する文字列の最大長

/*!	@brief ダイレクトタグジャンプ用検索結果をソートして保持する．
	@author MIK
*/
class CSortedTagJumpList {
public:
	CSortedTagJumpList(int max);
	~CSortedTagJumpList();

	int AddBaseDir( const WCHAR* baseDir );
	BOOL AddParamA( const ACHAR* keyword, const ACHAR* filename, int no, ACHAR type, const ACHAR* note, int depth, const int baseDirId );
	BOOL GetParam( int index, WCHAR* keyword, WCHAR* filename, int* no, WCHAR* type, WCHAR* note, int* depth, WCHAR* baseDir );
	int GetCount( void ){ return m_nCount; }
	void Empty( void );
	bool IsOverflow( void ){ return m_bOverflow; }

	typedef struct tagjump_info_t {
		struct tagjump_info_t*	next;	//!< 次のリスト
		WCHAR*	keyword;	//!< キーワード
		WCHAR*	filename;	//!< ファイル名
		int		no;			//!< 行番号
		WCHAR	type;		//!< 種類
		WCHAR*	note;		//!< 備考
		int		depth;		//!< (さかのぼる)階層
		int		baseDirId;	//!< ファイル名のベースディレクトリ
	} TagJumpInfo;

	TagJumpInfo* GetPtr( int index );

	/*!	@brief 管理数の最大値を取得する

		@date 2005.04.22 genta 最大値を可変に
	*/
	int GetCapacity(void) const { return m_MAX_TAGJUMPLIST; }

private:
	TagJumpInfo*	m_pTagjump;	//!< タグジャンプ情報
	std::vector<std::wstring> m_baseDirArr;	//!< ベースディレクトリ情報
	int				m_nCount;	//!< 個数
	bool			m_bOverflow;	//!< オーバーフロー
	
	//	2005.04.22 genta 最大値を可変に
	const int		m_MAX_TAGJUMPLIST;	//!< 管理する情報の最大数

	void Free( TagJumpInfo* item );

	DISALLOW_COPY_AND_ASSIGN(CSortedTagJumpList);
};
#endif /* SAKURA_CSORTEDTAGJUMPLIST_54B68DB2_20F2_4F62_BB2C_47629293620F_H_ */
