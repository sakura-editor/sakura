//	$Id$
/*
	CImageListMgr	ImageListを扱うクラス

	Author: genta
	Date:	Oct. 11, 2000
	Copyright (C) 1998-2000, genta

*/
#include "CImageListMgr.h"
#include "sakura_rc.h"

//	Destructor
CImageListMgr::~CImageListMgr()
{
	if( m_hList != NULL ){
		ImageList_Destroy( m_hList );
	}
}

bool CImageListMgr::Create(HINSTANCE hInstance, HWND hWnd)
{
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
#ifdef _DEBUG
	HDC		dcOrg;		//	スクリーンへの描画用(DEBUG)
	char buf[60];	//	お試し用
#endif
	m_cx = m_cy  = 16;
	
//	Oct. 21, 2000 JEPRO 設定
	const int MAX_X = 32;
	const int MAX_Y = 11;
	
	//	暫定
//	const int MAX_X = 128;
//	const int MAX_Y = 1;

	COLORREF cTransparent;	//	取得した画像の(0,0)の色を背景色として使う
	
	nRetPos = 0;
	do {
		//	リソースからBitmapを読み込む
		hRscbmp = ::LoadBitmap( hInstance, MAKEINTRESOURCE( IDB_MYTOOL ));
		if( hRscbmp == NULL ){
			nRetPos = 0;
			break;
		}
#ifdef _DEBUG
		//	スクリーンのdcを得る
		dcOrg = CreateDC( "DISPLAY", NULL, NULL, NULL );
		if( dcFrom == NULL ){
			nRetPos = 50;
			break;
		}
#endif
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
		hTmpbmp = ::CreateCompatibleBitmap(dcFrom, m_cx * MAX_X, m_cy);
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

		int y, py, px;
		for( y = py = 0, px = 0; y < MAX_Y;
			y++, py += m_cy, px += m_cx * MAX_X ){
			::BitBlt( dcTo,	//	Destination
				0, 0,		//	転送先
				m_cx * MAX_X, m_cy,		//	転送サイズ
				dcFrom,		//	Source
				0, py,		//	転送元
				SRCCOPY		//	単純コピー
			);
#ifdef _DEBUG
			::BitBlt( dcOrg,	//	Destination
				px, 0,		//	転送先
				m_cx * MAX_X, m_cy,		//	転送サイズ
				dcFrom,		//	Source
				0, py,		//	転送元
				SRCCOPY		//	単純コピー
			);
			::BitBlt( dcOrg,	//	Destination
				px, m_cy,		//	転送先
				m_cx * MAX_X, m_cy,		//	転送サイズ
				dcTo,		//	Source
				0, 0,		//	転送元
				SRCCOPY		//	単純コピー
			);
#endif
			SelectObject( dcTo, hTOldbmp );	//	超重要！
			//	BitmapがMemoryDCにAssignされている間はbitmapハンドルを
			//	使っても正しいbitmapが取得できない．
			//	つまり，DCへの描画命令を発行してもその場でBitmapに
			//	反映されるわけではない．
			//	BitmapをDCから取り外して初めて内容の保証ができる

			//	Oct, 23, 2000 genta
			//	横長のBitmapにしたらアイコンがおかしくなったので，
			//	1行分を少しずつ追加する方法に変更
			ImageList_AddMasked( m_hList, hTmpbmp, cTransparent);

			//	MAPし直す
			SelectObject( dcTo, hTmpbmp );
		}
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
#ifdef _DEBUG
		DeleteObject( dcOrg );
#endif
	case 50:
		DeleteObject( hRscbmp );
	}

#if 0
	wsprintf( buf, "Ret: %d", nRetPos );
	::MessageBox( NULL, buf, "ImageList", MB_OK );
#endif

	return nRetPos == 0;
}
