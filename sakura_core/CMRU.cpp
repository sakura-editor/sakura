/*!	@file
	@brief MRUリストと呼ばれるリストを管理する

	@author YAZAKI
	@date 2001/12/23  新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CShareData.h"
#include "CMenuDrawer.h"	//	これでいいのか？
#include "global.h"
#include "stdio.h"
#include "etc_uty.h"
#include "my_icmp.h" // 2002/11/30 Moca 追加

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
//	HWND	hwndDummy;				//	ダミー（使わないよ）
	char	szFile2[_MAX_PATH * 2];	//	全部&でも問題ないように。
//	char	*p;						//	&をスキャンするときに使う、作業用ポインタ。
	char	szMemu[300];			//	メニューキャプション
	int		createdMenuItem = 0;	//	すでに作成されたメニューの数。
	int		i;
	CShareData::getInstance()->TransformFileName_MakeCash();

	//	空メニューを作る
	hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	for( i = 0; i < Length(); ++i ){
		/* 指定ファイルが開かれているか調べる */
//		開かれててもいいんじゃない？
//		if( m_pcShareData->IsPathOpened( m_pShareData->m_fiMRUArr[i].m_szPath, &hwndDummy ) ){
//			continue;
//		}
		//	「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if ( m_pShareData->m_Common.m_nMRUArrNum_MAX <= createdMenuItem ){
			break;
		}
		
		/* MRUリストの中にある開かれていないファイル */
		
		CShareData::getInstance()->GetTransformFileNameFast( m_pShareData->m_fiMRUArr[i].m_szPath, szMemu, _MAX_PATH );
		//	&を&&に置換。
		//	Jan. 19, 2002 genta
		dupamp( szMemu, szFile2 );
		
		//	j >= 10 + 26 の時の考慮を省いた(に近い)がファイルの履歴MAXを36個にしてあるので事実上OKでしょう
		wsprintf( szMemu, "&%c %s", (createdMenuItem < 10) ? ('0' + createdMenuItem) : ('A' + createdMenuItem - 10), szFile2 );

		//	ファイル名のみ必要。
		//	文字コード表記
		if( 0 <  m_pShareData->m_fiMRUArr[i].m_nCharCode  &&
				 m_pShareData->m_fiMRUArr[i].m_nCharCode  < CODE_CODEMAX ){
			strcat( szMemu, gm_pszCodeNameArr_3[ m_pShareData->m_fiMRUArr[i].m_nCharCode ] );
		}

		//	メニューに追加。
		pCMenuDrawer->MyAppendMenu( hMenuPopUp, MF_BYPOSITION | MF_STRING, IDM_SELMRU + i, szMemu );
		createdMenuItem++;	//	作成したメニュー数+1
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
void CMRU::GetPathList( char** ppszMRU )
{
	int i;
	int copiedItem = 0;
	for( i = 0; i < Length(); ++i ){
		//	「共通設定」→「全般」→「ファイルの履歴MAX」を反映
		if ( m_pShareData->m_Common.m_nMRUArrNum_MAX <= copiedItem ){
			break;
		}
		ppszMRU[i] = m_pShareData->m_fiMRUArr[i].m_szPath;
		copiedItem++;
	}
	ppszMRU[i] = NULL;
}

/*! アイテム数を返す */
int CMRU::Length(void)
{
	return m_pShareData->m_nMRUArrNum;
}

/*!
	ファイル履歴のクリア
*/
void CMRU::ClearAll(void)
{
	int i;
	for( i = 0; i < MAX_MRU; ++i ){
		m_pShareData->m_fiMRUArr[i].m_nViewTopLine = 0;
		m_pShareData->m_fiMRUArr[i].m_nViewLeftCol = 0;
		m_pShareData->m_fiMRUArr[i].m_nX = 0;
		m_pShareData->m_fiMRUArr[i].m_nY = 0;
		m_pShareData->m_fiMRUArr[i].m_bIsModified = 0;
		m_pShareData->m_fiMRUArr[i].m_nCharCode = 0;
		strcpy( m_pShareData->m_fiMRUArr[i].m_szPath, "" );
	}
	m_pShareData->m_nMRUArrNum = 0;
}

/*!
	ファイル情報の取得
	
	@param num [in] 履歴番号(0~)
	@param pfi [out] 構造体へのポインタ格納先
	
	@retval TRUE データが格納された
	@retval FALSE 正しくない番号が指定された．データは格納されなかった．
*/
BOOL CMRU::GetFileInfo( int num, FileInfo* pfi )
{
	if (num < Length()){
		*pfi = m_pShareData->m_fiMRUArr[num];	//	相変わらず無防備。。。
		return TRUE;
	}
	return FALSE;
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
BOOL CMRU::GetFileInfo( const char* pszPath, FileInfo* pfi )
{
	int i;
	for( i = 0; i < Length(); ++i ){
		if( 0 == _stricmp( pszPath, m_pShareData->m_fiMRUArr[i].m_szPath ) ){
			*pfi = m_pShareData->m_fiMRUArr[i];	//	相変わらず無防備。。。
			return TRUE;
		}
	}
	return FALSE;
}

/*!	@brief MRUリストへの登録

	@param pfi [in] 追加するファイルの情報

	該当ファイルがリムーバブルディスク上にある場合にはMRU Listへの登録は行わない。

	@date 2001.03.29 MIK リムーバブルディスク上のファイルを登録しないようにした。
	@date 2001.12.26 YAZAKI CShareData::AddMRUListから移動
*/
void CMRU::Add( FileInfo* pFileInfo )
{
	//	ファイル名が無ければ無視
	if( 0 == strlen( pFileInfo->m_szPath ) ){
		return;
	}
	
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];
	char	szFolder[_MAX_PATH + 1];	//	ドライブ＋フォルダ

	_splitpath( pFileInfo->m_szPath, szDrive, szDir, NULL, NULL );	//	ドライブとフォルダを取り出す。

	//	リムーバブルなら非登録？
	if (/* 「リムーバブルなら登録しない」オン && */ IsRemovableDrive( szDrive ) ){
		return;
	}

	//	szFolder作成
	strcpy( szFolder, szDrive );
	strcat( szFolder, szDir );
	//	Folderを、CMRUFolderに登録
	CMRUFolder cMRUFolder;
	cMRUFolder.Add(szFolder);

	//	MRUに登録。
	int i, j;
	for( i = 0; i < Length(); ++i ){
		if( 0 == _stricmp( pFileInfo->m_szPath, m_pShareData->m_fiMRUArr[i].m_szPath ) ){
			//	もうすでにm_pShareData->m_fiMRUArrにあった。
			for( j = i; j > 0; j-- ){	//	ここまでのファイルを繰り下げ。
				m_pShareData->m_fiMRUArr[j] = m_pShareData->m_fiMRUArr[j - 1];	//	値のコピー
			}
			m_pShareData->m_fiMRUArr[0] = *pFileInfo;	//	先頭に割り当て。
			//	m_pShareData->m_nMRUArrNumは変わらず。
			return;
		}
	}
	
	//	まだm_fiMRUArrには無かった。
	//	ほかに何かがある。
	for( j = max(Length(), m_pShareData->m_Common.m_nMRUArrNum_MAX) - 1; j > 0; j-- ){
		m_pShareData->m_fiMRUArr[j] = m_pShareData->m_fiMRUArr[j - 1];	//	値のコピー
	}
	m_pShareData->m_fiMRUArr[0] = *pFileInfo;	//	先頭に割り当て。

	m_pShareData->m_nMRUArrNum++;	//	数字を増やす。
	if( m_pShareData->m_nMRUArrNum > MAX_MRU ){	//	増えすぎたら戻す。
		m_pShareData->m_nMRUArrNum = MAX_MRU;
	}
}

/*!
	リムーバブルドライブの判定

	@param pszDrive [in] ドライブ名を含むパス名
	
	@retval true リムーバブルドライブ
	@retval false 固定ドライブ．ネットワークドライブ．
	
	@author MIK
	@date 2001.03.29 新規作成
	@date 2001.12.23 YAZAKI MRUの別クラス化に伴う関数化
	@date 2002.01.28 genta 戻り値の型をBOOLからboolに変更．
*/
bool CMRU::IsRemovableDrive( const char* pszDrive )
{
	char	szDriveType[_MAX_DRIVE+1];	// "A:\"登録用
	long	lngRet;
	char	c;

	c = pszDrive[0];
	if( c >= 'a' && c <= 'z' ){
		c = c - ('a' - 'A');
	}
	if( c >= 'A' && c <= 'Z' ){
		sprintf( szDriveType, "%c:\\", c );
		lngRet = GetDriveType( szDriveType );
		if( DRIVE_REMOVABLE	== lngRet
		 || DRIVE_CDROM		== lngRet){
			return true;
		}
	}
	return false;
}

/*EOF*/
