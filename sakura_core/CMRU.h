/*!	@file
	@brief MRUリストと呼ばれるリストを管理する

	@author YAZAKI
	@date 2001/12/23  新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, YAZAKI
	Copyright (C) 2003, MIK

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
class CMRU;

#ifndef _CMRU_H_
#define _CMRU_H_

#include <windows.h> /// BOOL,HMENU // 2002/2/10 aroka
#include "CRecent.h"

/*	MRUリストに関係するmaxdata	*/
enum MRU_maxdata{
	MAX_MRU						=  36,	//Sept. 27, 2000 JEPRO 0-9, A-Z で36個になるのでそれに合わせて30→36に変更
};

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
class CMRU{
public:
	//	コンストラクタ
	CMRU();
	~CMRU();

	//	メニューを取得する
	HMENU CreateMenu( class CMenuDrawer* pCMenuDrawer );	//	うーん。pCMenuDrawerが必要なくなるといいなぁ。
	BOOL DestroyMenu( HMENU hMenu );
	
	//	ファイル名の一覧を教えて
	void GetPathList( char** ppszMRU );	//	ppszMRUにはファイル名をコピーしません。変更しないでね☆

	//	アクセス関数
	int Length(void);	//	アイテムの数。
	void ClearAll(void);//	アイテムを削除～。
	BOOL GetFileInfo( int num, struct FileInfo* pfi );				//	番号で指定したFileInfo（情報をまるごと）
	BOOL GetFileInfo( const char* pszPath, FileInfo* pfi );	//	ファイル名で指定したFileInfo（情報をまるごと）
	void Add( FileInfo* pFileInfo );		//	*pFileInfoを追加する。

protected:
	//	共有メモリアクセス用。
	struct DLLSHAREDATA*	m_pShareData;		//	共有メモリを参照するよ。
	bool IsRemovableDrive( const char* pszDrive );

private:
	CRecent	m_cRecent;	//お気に入り	//@@@ 2003.04.08 MIK

};
#endif
