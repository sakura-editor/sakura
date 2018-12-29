/*!	@file
	@brief ImageListの取り扱い

	@author genta
	@date Oct. 11, 2000 genta
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2000, jepro
	Copyright (C) 2001, GAE, jepro
	Copyright (C) 2003, Moca, genta, wmlhq
	Copyright (C) 2007, ryoji
	Copyright (C) 2010, syat

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CImageListMgr.h"
#include "env/CommonSetting.h"
#include "util/module.h"
#include "debug/CRunningTimer.h"
#include "sakura_rc.h"

//  2010/06/29 syat MAX_X, MAX_Yの値をCommonSettings.hに移動
//	Jul. 21, 2003 genta 他でも使うので関数の外に出した
//	Oct. 21, 2000 JEPRO 設定
const int MAX_X = MAX_TOOLBAR_ICON_X;
const int MAX_Y = MAX_TOOLBAR_ICON_Y;	//2002.01.17

/*! コンストラクタ */
CImageListMgr::CImageListMgr()
	: m_cx( 16 ), m_cy( 16 )
	, m_cTrans( RGB( 0, 0, 0 ))
	, m_hIconBitmap( NULL )
	, m_nIconCount( MAX_TOOLBAR_ICON_COUNT )
{
}

/*!	領域を指定色で塗りつぶす

	@author Nakatani
*/
static void FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr)
{
//	ASSERT_VALID(this);
//	ASSERT(m_hDC != NULL);

	RECT rect;
	::SetBkColor( hdc, clr );
	::SetRect( &rect, x, y, x + cx, y + cy );
	::ExtTextOutW_AnyBuild( hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL );
}

//	Destructor
CImageListMgr::~CImageListMgr()
{
	//	2003.07.21 Image Listの代わりに描画用bitmapを解放
	if( m_hIconBitmap != NULL ){
		DeleteObject( m_hIconBitmap );
	}
}

/*
	@brief Image Listの作成
	
	リソースまたはファイルからbitmapを読み込んで
	描画用に保持する．
	
	@param hInstance [in] bitmapリソースを持つインスタンス
	
	@date 2003.07.21 genta ImageListの構築は行わない．代わりにbitmapをそのまま保持する．
*/
bool CImageListMgr::Create(HINSTANCE hInstance)
{
	MY_RUNNINGTIMER( cRunningTimer, "CImageListMgr::Create" );
	if( m_hIconBitmap != NULL ){	//	既に構築済みなら無視する
		return true;
	}

	HBITMAP	hRscbmp;			//	リソースから読み込んだひとかたまりのBitmap
	HBITMAP	hFOldbmp = NULL;	//	SetObjectで得られた1つ前のハンドルを保持する
	HDC		dcFrom = 0;			//	描画用
	int		nRetPos;			//	後処理用

	nRetPos = 0;
	do {
		//	From Here 2001.7.1 GAE
		//	2001.7.1 GAE リソースをローカルファイル(sakuraディレクトリ) my_icons.bmp から読めるように
		// 2007.05.19 ryoji 設定ファイル優先に変更
		TCHAR szPath[_MAX_PATH];
		GetInidirOrExedir( szPath, FN_TOOL_BMP );
		hRscbmp = (HBITMAP)::LoadImage( NULL, szPath, IMAGE_BITMAP, 0, 0,
			LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS );

		if( hRscbmp == NULL ) {	// ローカルファイルの読み込み失敗時はリソースから取得
			//	このブロック内は従来の処理
			//	リソースからBitmapを読み込む
			//	2003.09.29 wmlhq 環境によってアイコンがつぶれる
			//hRscbmp = ::LoadBitmap( hInstance, MAKEINTRESOURCE( IDB_MYTOOL ) );
			hRscbmp = (HBITMAP)::LoadImage( hInstance, MAKEINTRESOURCE( IDB_MYTOOL ), IMAGE_BITMAP, 0, 0,
				LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS  );
			if( hRscbmp == NULL ){
				//	Oct. 4, 2003 genta エラーコード追加
				//	正常終了と同じコードだとdcFromを不正に解放してしまう
				nRetPos = 2;
				break;
			}
		}
		//	To Here 2001.7.1 GAE

		//	透過色を得るためにDCにマップする
		//	2003.07.21 genta 透過色を得る以外の目的では使わなくなった
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

		m_cTrans = GetPixel( dcFrom, 0, 0 );//	取得した画像の(0,0)の色を背景色として使う
		
		//	2003.07.21 genta
		//	ImageListへの登録部分は当然ばっさり削除
		
		//	もはや処理とは無関係だが，後学のためにコメントのみ残しておこう
		//---------------------------------------------------------
		//	BitmapがMemoryDCにAssignされている間はbitmapハンドルを
		//	使っても正しいbitmapが取得できない．
		//	つまり，DCへの描画命令を発行してもその場でBitmapに
		//	反映されるわけではない．
		//	BitmapをDCから取り外して初めて内容の保証ができる

		//	DCのmap/unmapが速度に大きく影響するため，
		//	横長のBitmapを作って一括登録するように変更
		//	これによって250msecくらい速度が改善される．
		//---------------------------------------------------------

		// アイコンサイズが異なる場合、拡大縮小する
		hRscbmp = ResizeToolIcons(dcFrom, hRscbmp, MAX_X, MAX_Y );
		if ( hRscbmp == NULL ) {
			nRetPos = 4;
			break;
		}

		// クラスメンバに変更を保存する
		m_hIconBitmap = hRscbmp;
		m_cx = ::GetSystemMetrics(SM_CXSMICON);
		m_cy = ::GetSystemMetrics(SM_CYSMICON);

	} while(0);	//	1回しか通らない. breakでここまで飛ぶ

	//	後処理
	switch( nRetPos ){
	case 0:
		//	Oct. 4, 2003 genta hRscBmpをdcFromから切り離しておく必要がある
		//	アイコン描画変更時に過って削除されていた
		SelectObject( dcFrom, hFOldbmp );
	case 4:
		DeleteDC( dcFrom );
	case 2:
	case 1:
		//	2003.07.21 genta hRscbmpは m_hIconBitmap としてオブジェクトと
		//	同じだけ保持されるので解放してはならない
		break;
	}

	return nRetPos == 0;
}

/*! ビットマップの表示 灰色を透明描画

	@author Nakatani
	@date 2003.07.21 genta 以前のCMenuDrawerより移転復活
	@date 2003.08.27 Moca 背景は透過処理に変更し、colBkColorを削除
	@date 2010.01.30 syat 透明にする色を引数に移動
*/
void CImageListMgr::MyBitBlt(
	HDC drawdc, 
	int nXDest, 
	int nYDest, 
	int nWidth, 
	int nHeight, 
	HBITMAP bmp, 
	int nXSrc, 
	int nYSrc,
	COLORREF colToTransParent	/* BMPの中の透明にする色 */
) const
{
	// 仮想DCを生成して指定されたビットマップを展開する
	HDC hdcSrc = ::CreateCompatibleDC( drawdc );
	HGDIOBJ bmpSrcOld = ::SelectObject( hdcSrc, bmp );
	::SetBkColor( hdcSrc, m_cTrans );

	// create a monochrome memory DC
	HDC hdcMask = ::CreateCompatibleDC( drawdc );
	HBITMAP bmpMask = ::CreateCompatibleBitmap( hdcMask, nWidth, nHeight );
	HGDIOBJ bmpMaskOld = ::SelectObject( hdcMask, bmpMask );

	// build a mask
	BitBlt( hdcMask, 0, 0, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, SRCAND );

	// 作業DCを作成
	HDC hdcWork = ::CreateCompatibleDC( drawdc );
	HBITMAP bmpWork = ::CreateCompatibleBitmap( drawdc, nWidth, nHeight );
	HGDIOBJ bmpWorkOld = ::SelectObject( hdcWork, bmpWork );

	// ビットマップ描画(マスクとor描画)
	::BitBlt( hdcWork, 0, 0, nWidth, nHeight, hdcMask, 0, 0, SRCCOPY );
	::BitBlt( hdcWork, 0, 0, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, SRCINVERT );

	// 作業DCの内容を出力DCに転送
	::BitBlt( drawdc, nXDest, nYDest, nWidth, nHeight, hdcWork,  0, 0, SRCPAINT );

	// 後始末
	::SelectObject( hdcWork, bmpWorkOld );
	::DeleteObject( bmpWork );
	::DeleteDC( hdcWork );
	::SelectObject( hdcMask, bmpMaskOld );
	::DeleteObject( bmpMask );
	::DeleteDC( hdcMask );
	::SelectObject( hdcSrc, bmpSrcOld );
	::DeleteDC( hdcSrc );
	return;
}


/*! メニューアイコンの淡色表示

	@author Nakatani
	
	@date 2003.07.21 genta 以前のCMenuDrawerより移転復活
	@date 2003.08.27 Moca 背景色は透過処理する
*/
void CImageListMgr::DitherBlt2( HDC drawdc, int nXDest, int nYDest, int nWidth, 
                        int nHeight, HBITMAP bmp, int nXSrc, int nYSrc) const
{
	// 仮想DCを生成して指定されたビットマップを展開する
	HDC hdcSrc = ::CreateCompatibleDC( drawdc );
	HGDIOBJ bmpSrcOld = ::SelectObject( hdcSrc, bmp );
	::SetBkColor( hdcSrc, m_cTrans );

	// create a monochrome memory DC
	HDC hdcMono = ::CreateCompatibleDC( drawdc );
	HBITMAP bmpMono = ::CreateCompatibleBitmap( hdcMono, nWidth, nHeight );
	HGDIOBJ bmpMonoOld = ::SelectObject( hdcMono, bmpMono );

	// モノクロイメージ作成
	::BitBlt( hdcMono, 0, 0, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, SRCPAINT );

	// 作業DCを作成
	HDC hdcWork = ::CreateCompatibleDC( drawdc );
	HBITMAP bmpWork = ::CreateCompatibleBitmap( drawdc, nWidth, nHeight);
	HGDIOBJ bmpWorkOld = ::SelectObject( hdcWork, bmpWork);

	// Copy the image from the toolbar into the memory DC
	// and draw it (grayed) back into the toolbar.
    //SK: Looks better on the old shell
	COLORREF bkColor = ::GetSysColor( COLOR_BTNFACE );
	COLORREF fgColor = ::GetSysColor( COLOR_GRAYTEXT );
	if ( fgColor == bkColor ) fgColor = ::GetSysColor( COLOR_3DSHADOW );
	if ( fgColor == bkColor ) fgColor = ::GetSysColor( COLOR_BTNSHADOW );
	if ( fgColor == bkColor ) fgColor = ::GetSysColor( COLOR_BTNHILIGHT );

	// ビットマップ描画(背景色・前景色を指定してマスクを描画)
	::SetBkColor( hdcWork, bkColor );
	::SetTextColor( hdcWork, fgColor );
	::BitBlt( hdcWork, 0, 0, nWidth, nHeight, hdcMono, 0, 0, SRCPAINT );

	// 作業DCの内容を出力DCに転送
	::BitBlt( drawdc, nXDest, nYDest, nWidth, nHeight, hdcWork, 0, 0, SRCPAINT );
	
	// 後始末
	::SelectObject( hdcWork, bmpWorkOld );
	::DeleteObject( bmpWork );
	::DeleteDC( hdcWork );
	::SelectObject( hdcMono, bmpMonoOld );
	::DeleteObject( bmpMono );
	::DeleteDC( hdcMono );
	::SelectObject( hdcSrc, bmpSrcOld );
	::DeleteDC( hdcSrc );
	return;
}


/*! @brief アイコンの描画

	指定されたDCの指定された座標にアイコンを描画する．

	@param index [in] 描画するアイコン番号
	@param dc [in] 描画するDevice Context
	@param x [in] 描画するX座標
	@param y [in] 描画するY座標
	@param fstyle [in] 描画スタイル
	@param bgColor [in] 背景色(透明部分の描画用)

	@note 描画スタイルとして有効なのは，ILD_NORMAL, ILD_MASK
	
	@date 2003.07.21 genta 独自描画ルーチンを使う
	@date 2003.08.30 genta 背景色を指定する引数を追加
	@date 2003.09.06 genta Mocaさんの背景色透過処理に伴い，背景色引数削除
	@date 2007.11.02 ryoji アイコン番号が負の場合は描画しない
*/
bool CImageListMgr::Draw(int index, HDC dc, int x, int y, int fstyle ) const
{
	if( m_hIconBitmap == NULL )
		return false;
	if( index < 0 )
		return false;

	if( fstyle == ILD_MASK ){
		DitherBlt2( dc, x, y, cx(), cy(), m_hIconBitmap,
			( index % MAX_X ) * cx(), ( index / MAX_X ) * cy());
	}
	else {
		MyBitBlt( dc, x, y, cx(), cy(), m_hIconBitmap,
			( index % MAX_X ) * cx(), ( index / MAX_X ) * cy(), m_cTrans );
	}
	return true;
}

/*!	アイコン数を返す

	@date 2003.07.21 genta 個数を自分で管理する必要がある．
*/
int CImageListMgr::Count() const
{
	return m_nIconCount;
//	return MAX_X * MAX_Y;
}

/*!
 * @brief アイコンを追加してそのIDを返す
 */
int CImageListMgr::Add( const TCHAR* szPath )
{
	if ( (m_nIconCount % MAX_X) == 0 ) {
		Extend();
	}

	//アイコンを読み込む
	HBITMAP bmpSrc = (HBITMAP)::LoadImage( NULL, szPath, IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION );

	if( bmpSrc == NULL ) {
		return -1;
	}

	int imageNo = m_nIconCount++;

	// 仮想DCを生成して読込んだビットマップを展開する
	HDC hdcSrc = ::CreateCompatibleDC( NULL );
	HGDIOBJ bmpSrcOld = ::SelectObject( hdcSrc, bmpSrc );

	//取得した画像の(0,0)の色を背景色として使う
	::SetBkColor( hdcSrc, ::GetPixel( hdcSrc, 0, 0 ) );

	// DIBセクションからサイズを取得する
	LONG nWidth, nHeight;
	{
		// DIBセクションを取得する
		DIBSECTION di = {};
		if ( !::GetObject( bmpSrc, sizeof( di ), &di ) ) {
			DEBUG_TRACE( _T( "GetObject() failed." ) );
			::SelectObject( hdcSrc, bmpSrcOld );
			::DeleteDC( hdcSrc );
			::DeleteObject( bmpSrc );
			return -1;
		}

		nWidth = di.dsBm.bmWidth;
		nHeight = di.dsBm.bmHeight;
		if ( nWidth != nHeight ) {
			DEBUG_TRACE( _T( "tool bitmap size is unexpected." ) );
			::SelectObject( hdcSrc, bmpSrcOld );
			::DeleteDC( hdcSrc );
			::DeleteObject( bmpSrc );
			return -1;
		}
	}

	// リサイズ不要かどうか
	bool NoResize = nWidth == cx() && nHeight == cy();

	// create a monochrome memory DC
	HDC hdcMask = ::CreateCompatibleDC( NULL );
	HBITMAP bmpMask = ::CreateCompatibleBitmap( hdcMask, nWidth, nHeight );
	HGDIOBJ bmpMaskOld = ::SelectObject( hdcMask, bmpMask );

	// build a mask
	if ( NoResize ) {
		::BitBlt( hdcMask, 0, 0, nWidth, nHeight, hdcSrc, 0, 0, SRCAND );
	} else {
		::StretchBlt( hdcMask, 0, 0, cx(), cy(), hdcSrc, 0, 0, nWidth, nHeight, SRCAND );
	}

	// 作業DCを作成
	HDC hdcWork = ::CreateCompatibleDC( NULL );
	HBITMAP bmpWork = ::CreateCompatibleBitmap( hdcSrc, nWidth, nHeight );
	HGDIOBJ bmpWorkOld = ::SelectObject( hdcWork, bmpWork );

	// ビットマップ描画(マスクとor描画)
	if ( NoResize ) {
		::BitBlt( hdcWork, 0, 0, nWidth, nHeight, hdcSrc, 0, 0, SRCINVERT );
	} else {
		::StretchBlt( hdcWork, 0, 0, cx(), cy(), hdcSrc, 0, 0, nWidth, nHeight, SRCINVERT );
	}

	// 作業DCの内容を出力DCに転送
	HDC hdcDst = ::CreateCompatibleDC( NULL );
	HGDIOBJ hbmDstOld = ::SelectObject( hdcDst, m_hIconBitmap );
	::BitBlt( hdcDst, (imageNo % MAX_X) * cx(), (imageNo / MAX_X) * cy(), cx(), cy(), hdcWork, 0, 0, SRCPAINT );

	// 後始末
	::SelectObject( hdcDst, hbmDstOld );
	::DeleteDC( hdcDst );
	::SelectObject( hdcWork, bmpWorkOld );
	::DeleteObject( bmpWork );
	::DeleteDC( hdcWork );
	::SelectObject( hdcMask, bmpMaskOld );
	::DeleteObject( bmpMask );
	::DeleteDC( hdcMask );
	::SelectObject( hdcSrc, bmpSrcOld );
	::DeleteDC( hdcSrc );

	::DeleteObject( bmpSrc );

	return imageNo;
}

// ツールイメージをリサイズする
HBITMAP CImageListMgr::ResizeToolIcons(
	HDC hDC,
	HBITMAP &hSrcBmp,
	int cols,
	int rows
) const noexcept
{
	// DIBセクションを取得する
	DIBSECTION di = {};
	if (!::GetObject(hSrcBmp, sizeof(di), &di)) {
		DEBUG_TRACE( _T("GetObject() failed.") );
		return NULL;
	}

	// DIBセクションからサイズを取得する
	int cx = di.dsBm.bmWidth / cols;
	int cy = di.dsBm.bmHeight / rows;
	if (cx != cy) {
		DEBUG_TRACE(_T("tool bitmap size is unexpected."));
		return NULL;
	}

	const int cxSmIcon = ::GetSystemMetrics(SM_CXSMICON);
	const int cySmIcon = ::GetSystemMetrics(SM_CYSMICON);

	// アイコンサイズが異なる場合、拡大縮小する
	if (cx != cxSmIcon) {
		// 作業用の仮想DCを作成する
		HDC hAltDC = ::CreateCompatibleDC(hDC);
		// 互換bmpを作る
		HBITMAP hAltBmp = ::CreateCompatibleBitmap(hDC, cxSmIcon * cols, cySmIcon * rows);

		// 仮想DCで互換Bmpを選択
		HGDIOBJ hAltBmpOld = ::SelectObject(hAltDC, hAltBmp);

		// ざっくり拡大縮小すると位置がずれるので1個ずつ変換する
		for (int row = 0; row < rows; ++row) {
			for (int col = 0; col < cols; ++col) {
				// 拡大・縮小する
				::StretchBlt(
					hAltDC,
					col * cxSmIcon,
					row * cySmIcon,
					cxSmIcon,
					cySmIcon,
					hDC,
					col * cx,
					row * cy,
					cx,
					cy,
					SRCCOPY
				);
			}
		}

		// 仮想DCで元Bmpを選択して互換Bmpを解放する
		::SelectObject(hAltDC, hAltBmpOld);

		// ターゲットDCで変換後Bmpを選択する
		::SelectObject(hDC, hAltBmp);

		// 変換前Bmpを破棄して入れ替える
		::DeleteObject(hSrcBmp);

		// 仮想DCを削除する
		::DeleteDC(hAltDC);

		return hAltBmp;
	}

	return hSrcBmp;
}

// ビットマップを一行（MAX_X個）拡張する
void CImageListMgr::Extend(bool bExtend)
{
	int curY = m_nIconCount / MAX_X;
	if( curY < MAX_Y )
		curY = MAX_Y;

	HDC hSrcDC = ::CreateCompatibleDC( 0 );
	HBITMAP hSrcBmpOld = (HBITMAP)::SelectObject( hSrcDC, m_hIconBitmap );

	//1行拡張したビットマップを作成
	HDC hDestDC = ::CreateCompatibleDC( hSrcDC );
	HBITMAP hDestBmp = ::CreateCompatibleBitmap( hSrcDC, MAX_X * cx(), (curY + (bExtend ? 1 : 0)) * cy() );
	HBITMAP hDestBmpOld = (HBITMAP)::SelectObject( hDestDC, hDestBmp );

	::BitBlt( hDestDC, 0, 0, MAX_X * cx(), curY * cy(), hSrcDC, 0, 0, SRCCOPY );

	//拡張した部分は透過色で塗る
	if( bExtend ){
		FillSolidRect( hDestDC, 0, curY * cy(), MAX_X * cx(), cy(), m_cTrans );
	}

	::SelectObject( hSrcDC, hSrcBmpOld );
	::DeleteObject( m_hIconBitmap );
	::DeleteDC( hSrcDC );

	::SelectObject( hDestDC, hDestBmpOld );
	::DeleteDC( hDestDC );

	//ビットマップの差し替え
	m_hIconBitmap = hDestBmp;
}

void CImageListMgr::ResetExtend()
{
	m_nIconCount = MAX_TOOLBAR_ICON_COUNT;
	Extend(false);
}
