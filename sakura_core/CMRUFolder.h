/*!	@file
	@brief MRUリストと呼ばれるリストを管理する。フォルダ版。

	@author YAZAKI
	@date 2001/12/23  新規作成
*/
/*
	Copyright (C) 1998-2001, YAZAKI

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
class CMRUFolder;

#ifndef _CMRUFOLDER_H_
#define _CMRUFOLDER_H_

#include <windows.h> /// BOOL,HMENU // 2002/2/10 aroka

enum MRUFolder_maxdata{
	MAX_OPENFOLDER				=  36,	//Sept. 27, 2000 JEPRO 0-9, A-Z で36個になるのでそれに合わせて30→36に変更
};

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
class CMRUFolder {
public:
	//	コンストラクタ
	CMRUFolder();
	~CMRUFolder();

	//	メニューを取得する
	HMENU CreateMenu( class CMenuDrawer* pCMenuDrawer );	//	うーん。pCMenuDrawerが必要なくなるといいなぁ。
	BOOL DestroyMenu( HMENU hMenu );
	
	//	フォルダ名の一覧を教えて
	void GetPathList( char** ppszMRU );	//	ppszMRUにはフォルダ名をコピーしません。変更しないでね☆

	//	アクセス関数
	int Length();	//	アイテムの数。
	void ClearAll();					//	アイテムを削除～。
	void Add( const char* pszFolder );	//	pszFolderを追加する。
	const char* GetPath(int num);

protected:

	//	共有メモリアクセス用。
	struct DLLSHAREDATA*	m_pShareData;			//	共有メモリを参照するよ。
};

#endif
