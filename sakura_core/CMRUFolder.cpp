/*!	@file
	@brief MRUリストと呼ばれるリストを管理する

	@author YAZAKI
	@date 2001/12/23  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CShareData.h"
#include "CMenuDrawer.h"	//	これでいいのか？
#include "CMRUFolder.h"
#include "etc_uty.h"
#include "my_icmp.h" // 2002/11/30 Moca 追加

/*!	コンストラクタ

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
CMRUFolder::CMRUFolder()
{
	//	初期化。
	m_pShareData = CShareData::getInstance()->GetShareData();
}

/*	デストラクタ	*/
CMRUFolder::~CMRUFolder()
{
}

HMENU CMRUFolder::CreateMenu( CMenuDrawer* pCMenuDrawer )
{
	HMENU	hMenuPopUp;
	char	szFolder2[_MAX_PATH * 2];	//	全部&でも問題ないように :-)
//	char	*p;							//	&をスキャンするときに使う、作業用ポインタ。
	char	szMemu[300];				//	メニューキャプション
	int		createdMenuItem = 0;		//	すでに作成されたメニューの数。
	int		i;

	hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	for( i = 0; i < Length() ; ++i ){
		//	「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if ( m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX <= createdMenuItem ){
			break;
		}

		//	&を&&に置換。
		//	Jan. 19, 2002 genta
		dupamp( m_pShareData->m_szOPENFOLDERArr[i], szFolder2 );

		//	j >= 10 + 26 の時の考慮を省いた(に近い)がフォルダの履歴MAXを36個にしてあるので事実上OKでしょう
		wsprintf( szMemu, "&%c %s", (createdMenuItem < 10) ? ('0' + createdMenuItem) : ('A' + createdMenuItem - 10), szFolder2 );

		//	メニューに追加
		pCMenuDrawer->MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELOPENFOLDER + i, szMemu );
		createdMenuItem++;
	}
	return hMenuPopUp;
}

void CMRUFolder::GetPathList( char** ppszOPENFOLDER )
{
	int i;
	int copiedItem = 0;
	for( i = 0; i < Length(); ++i ){
		//	「共通設定」→「全般」→「フォルダの履歴MAX」を反映
		if ( m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX <= copiedItem ){
			break;
		}
		ppszOPENFOLDER[i] = m_pShareData->m_szOPENFOLDERArr[i];
		copiedItem++;
	}
	ppszOPENFOLDER[i] = NULL;
}

int CMRUFolder::Length()
{
	return m_pShareData->m_nOPENFOLDERArrNum;
}

void CMRUFolder::ClearAll()
{
	int i;
	for( i = 0; i < MAX_OPENFOLDER; ++i ){
		strcpy( m_pShareData->m_szOPENFOLDERArr[i], "" );
	}
	m_pShareData->m_nOPENFOLDERArrNum = 0;
}

/*	@brief 開いたフォルダ リストへの登録

	@date 2001.12.26  CShareData::AddOPENFOLDERListから移動した。（YAZAKI）
*/
void CMRUFolder::Add( const char* pszFolder )
{
	if( 0 == strlen( pszFolder ) ){	//	長さが0なら排除。
		return;
	}

	int i, j;
	for( i = 0; i < Length(); ++i ){
		if( 0 == _stricmp( pszFolder, m_pShareData->m_szOPENFOLDERArr[i] ) ){
			//	もうすでにm_pShareData->m_szOPENFOLDERArr[i]にあった。
			for( j = i; j > 0; j-- ){	//	ここまでのフォルダを繰り下げ。
				strcpy( m_pShareData->m_szOPENFOLDERArr[j], m_pShareData->m_szOPENFOLDERArr[j - 1] );	//	コピー
			}
			strcpy( m_pShareData->m_szOPENFOLDERArr[0], pszFolder );	//	先頭に割り当て。
			//	m_pShareData->m_nOPENFOLDERArrNumは変わらず。
			return;
		}
	}

	//	まだm_szOPENFOLDERArrには無かった。
	for( j = max(Length(), m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX) - 1; j > 0; j-- ){
		strcpy( m_pShareData->m_szOPENFOLDERArr[j], m_pShareData->m_szOPENFOLDERArr[j - 1] );	//	コピー
	}
	strcpy( m_pShareData->m_szOPENFOLDERArr[0], pszFolder );	//	先頭にコピー

	m_pShareData->m_nOPENFOLDERArrNum++;	//	数字を増やす。
	if( m_pShareData->m_nOPENFOLDERArrNum > MAX_OPENFOLDER ){	//	増えすぎたら戻す。
		m_pShareData->m_nOPENFOLDERArrNum = MAX_OPENFOLDER;
	}
}

const char* CMRUFolder::GetPath(int num)
{
	if ( num < Length() ){
		return m_pShareData->m_szOPENFOLDERArr[num];
	}
	return 0;	//	例外を投げたほうがよさそう？
}

