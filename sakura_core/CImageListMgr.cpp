//	$Id$
/*!	@file
	@brief ImageListの取り扱い

	@author genta
	@date Oct. 11, 2000 genta
	$Revision$

*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, GAE

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "CImageListMgr.h"
#include "sakura_rc.h"
#include "CRunningTimer.h"

//	Destructor
CImageListMgr::~CImageListMgr()
{
	if( m_hList != NULL ){
		ImageList_Destroy( m_hList );
	}
}

/*
	@brief Image Listの作成
	
	リソースまたはファイルからbitmapを読み込み，
	それを元にImage Listを構築する．
	
	@param hInstance [in] bitmapリソースを持つインスタンス
	@param hWnd [in] 未使用
*/
bool CImageListMgr::Create(HINSTANCE hInstance, HWND hWnd)
{
	MY_RUNNINGTIMER( cRunningTimer, "CImageListMgr::Create" );
	if( m_hList != NULL ){	//	既に構築済みなら無視する
		return true;
	}

	HBITMAP	hRscbmp;	//	リソースから読み込んだひとかたまりのBitmap
	HBITMAP	hTmpbmp;	//	1アイコンBitmap用の領域
	HBITMAP	hFOldbmp;	//	SetObjectで得られた1つ前のハンドルを保持する
	HBITMAP	hTOldbmp;	//	SetObjectで得られた1つ前のハンドルを保持する
	HDC		dcFrom;		//	描画用
	HDC		dcTo;		//	描画用
	int		nRetPos;	//	後処理用
	m_cx = m_cy  = 16;

//	Oct. 21, 2000 JEPRO 設定
	const int MAX_X = 32;
	const int MAX_Y = 12;	//2002.01.17

	//	暫定
//	const int MAX_X = 128;
//	const int MAX_Y = 1;

	COLORREF cTransparent;	//	取得した画像の(0,0)の色を背景色として使う

	nRetPos = 0;
	do {
		//	From Here 2001.7.1 GAE
		//	2001.7.1 GAE リソースをローカルファイル(sakuraディレクトリ) my_icons.bmp から読めるように
		char	szPath[_MAX_PATH], szExeDrive[_MAX_DRIVE], szExeDir[_MAX_DIR];

		// sakura.exe のパスを取得
		::GetModuleFileName( ::GetModuleHandle(NULL), szPath, sizeof(szPath) );

		// (sakuraディレクトリ) my_icons.bmp の文字列を作成
		_splitpath( szPath, szExeDrive, szExeDir, NULL, NULL );
		//	@@TEST@@
		wsprintf( szPath, "%s%s%s", szExeDrive, szExeDir, "my_icons.bmp" );	//Jul.04, 2001 JEPRO toolbar.bmp→my_icons.bmp に名前変更
		hRscbmp = (HBITMAP)::LoadImage( NULL, szPath, IMAGE_BITMAP, 0, 0,
			LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS );

		if( hRscbmp == NULL ) {	// ローカルファイルの読み込み失敗時はリソースから取得
			//	このブロック内は従来の処理
			//	リソースからBitmapを読み込む
			hRscbmp = ::LoadBitmap( hInstance, MAKEINTRESOURCE( IDB_MYTOOL ) );
			if( hRscbmp == NULL ){
				nRetPos = 0;
				break;
			}
		}
		//	To Here 2001.7.1 GAE

		//	BitBltを使うためにMemoryDCにマッピングする
		//	MAPした後MemoryDCに対して描画を行うとBitmapも書き換えられているという算段．
		dcFrom = CreateCompatibleDC(0);	//	転送元用
		if( dcFrom == NULL ){
			nRetPos = 1;
			break;
		}

		//	まずbitmapをdcにmapする
		//	こうすることでCreateCompatibleBitmapで
		//	hRscbmpと同じ形式のbitmapを作れる．
		//	単にCreateCompatibleDC(0)で取得したdcや
		//	スクリーンのDCに対してCreateCompatibleBitmapを
		//	使うとモノクロBitmapになる．
		hFOldbmp = (HBITMAP)SelectObject( dcFrom, hRscbmp );
		if( hFOldbmp == NULL ){
			nRetPos = 4;
			break;
		}

		dcTo = CreateCompatibleDC(dcFrom);	//	転送先用
		if( dcTo == NULL ){
			nRetPos = 2;
			break;
		}
		//	Oct, 23, 2000 genta
		//	横長のBitmapにしたらアイコンがおかしくなったので，
		//	1行分を少しずつ追加する方法に変更
		//	1行分のBitmapを構築

		//	Oct. 23, 2002 genta
		//	横長のBitmapでも問題ないので，横長のBitmapを作って一括登録するように変更
		//	DCのmap/unmapが速度に大きく影響するため，
		hTmpbmp = ::CreateCompatibleBitmap(dcFrom, m_cx * MAX_X * MAX_Y, m_cy);
		if( hTmpbmp == NULL ){
			nRetPos = 3;
			break;
		}

		hTOldbmp = (HBITMAP)SelectObject( dcTo, hTmpbmp );
		if( hTOldbmp == NULL ){
			nRetPos = 5;
			break;
		}
		//	透明色の設定
		cTransparent = GetPixel( dcFrom, 0, 0 );

		//	Image List本体を作成
		m_hList = ImageList_Create( m_cx, m_cy, ILC_COLOR |ILC_MASK, MAX_X * MAX_Y, 32 );
		if( m_hList == NULL ){
			nRetPos = 6;
			break;
		}

		MY_TRACETIME( cRunningTimer, "List is created" );
		int y, py, px;
		for( y = py = 0, px = 0; y < MAX_Y;
			y++, py += m_cy, px += m_cx * MAX_X ){
			::BitBlt( dcTo,	//	Destination
				px, 0,		//	転送先
				m_cx * MAX_X, m_cy,		//	転送サイズ
				dcFrom,		//	Source
				0, py,		//	転送元
				SRCCOPY		//	単純コピー
			);
		}
		SelectObject( dcTo, hTOldbmp );	//	超重要！
		//	BitmapがMemoryDCにAssignされている間はbitmapハンドルを
		//	使っても正しいbitmapが取得できない．
		//	つまり，DCへの描画命令を発行してもその場でBitmapに
		//	反映されるわけではない．
		//	BitmapをDCから取り外して初めて内容の保証ができる

		//	Oct, 23, 2000 genta
		//	横長のBitmapにしたらアイコンがおかしくなったので，
		//	1行分を少しずつ追加する方法に変更
		
		//	Oct. 23, 2002 genta
		//	横長のBitmapでも問題ない
		//	DCのmap/unmapが速度に大きく影響するため，
		//	横長のBitmapを作って一括登録するように変更
		//	これによって250msecくらい速度が改善される．
		ImageList_AddMasked( m_hList, hTmpbmp, cTransparent);
	}while(0);	//	1回しか通らない

	//	後処理
	switch( nRetPos ){

	case 6:
		SelectObject( dcTo, hTOldbmp );
	case 0:
	case 5:
		DeleteObject( hTmpbmp );
	case 3:
		//	ウィンドウハンドルに関連づけられたDC(GetDCで取得したもの）はReleaseDCで解放するが，
		//	そうでないものはDeleteDCを使う．
		DeleteDC( dcTo );
	case 2:
		SelectObject( dcFrom, hFOldbmp );
	case 4:
		DeleteDC( dcFrom );
	case 1:
	case 50:
		DeleteObject( hRscbmp );
	}

	return nRetPos == 0;
}


/*[EOF]*/
