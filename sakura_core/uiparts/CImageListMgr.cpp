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
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CImageListMgr.h"

#include <cmath>
#include <array>
#include <list>
#include <functional>

#include "env/CommonSetting.h"
#include "util/module.h"
#include "debug/CRunningTimer.h"
#include "sakura_rc.h"
#include "config/system_constants.h"

//  2010/06/29 syat MAX_X, MAX_Yの値をCommonSettings.hに移動
//	Jul. 21, 2003 genta 他でも使うので関数の外に出した
//	Oct. 21, 2000 JEPRO 設定
const int MAX_X = MAX_TOOLBAR_ICON_X;
const int MAX_Y = MAX_TOOLBAR_ICON_Y;	//2002.01.17

/*! コンストラクタ */
CImageListMgr::CImageListMgr()
	: m_cx( 16 ), m_cy( 16 )
	, m_cTrans( RGB( 0, 0, 0 ))
	, m_hIconBitmap( nullptr )
	, m_hDC( nullptr )
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
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rect, nullptr, 0, nullptr );
}

/*! リソースに埋め込まれたmytool.bmpを読み込む
 */
static inline
HBITMAP LoadMyToolFromModule( HINSTANCE hInstance )
{
	//	リソースからBitmapを読み込む
	HANDLE hRscbmp = ::LoadImageW(
		hInstance,
		MAKEINTRESOURCE( IDB_MYTOOL ),
		IMAGE_BITMAP,
		0,
		0,
		LR_CREATEDIBSECTION
	);

	return (HBITMAP)hRscbmp;
}

//	Destructor
CImageListMgr::~CImageListMgr()
{
	//	2003.07.21 Image Listの代わりに描画用bitmapを解放
	if( m_hIconBitmap != nullptr ){
		DeleteObject( m_hIconBitmap );
	}
	if (m_hDC != nullptr) {
		::DeleteDC(m_hDC);
	}
}

static
HBITMAP ConvertTo32bppBMP(HBITMAP hbmpSrc, uint32_t*& pBits, LONG& bmpWidth, LONG& bmpHeight)
{
	BITMAP bmp;
	if (0 == GetObject(hbmpSrc, sizeof(BITMAP), &bmp )) {
		return hbmpSrc;
	}
	if (bmp.bmBitsPixel == 32) {
		return hbmpSrc;
	}
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = bmp.bmWidth;
	bmi.bmiHeader.biHeight = -bmp.bmHeight;
	bmi.bmiHeader.biPlanes = bmp.bmPlanes;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;
	HBITMAP hdib = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, (void**)&pBits, nullptr, 0);
	if (hdib == nullptr) {
		return hbmpSrc;
	}
	HDC hdcSrc = CreateCompatibleDC(nullptr);
	if (!hdcSrc) {
		DeleteObject(hdib);
		return hbmpSrc;
	}
	HDC hdcDst = CreateCompatibleDC(nullptr);
	if (!hdcDst) {
		DeleteDC(hdcSrc);
		DeleteObject(hdib);
		return hbmpSrc;
	}
	HGDIOBJ hbmpSrcOld = SelectObject(hdcSrc, hbmpSrc);
	HGDIOBJ hbmpDstOld = SelectObject(hdcDst, hdib);
	BitBlt(hdcDst, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcSrc, 0, 0, SRCCOPY);
	SelectObject(hdcSrc, hbmpSrcOld);
	SelectObject(hdcDst, hbmpDstOld);
	DeleteDC(hdcSrc);
	DeleteDC(hdcDst);
	DeleteObject(hbmpSrc);
	bmpWidth = bmp.bmWidth;
	bmpHeight = bmp.bmHeight;
	return hdib;
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
	MY_RUNNINGTIMER( cRunningTimer, L"CImageListMgr::Create" );
	if( m_hIconBitmap != nullptr ){	//	既に構築済みなら無視する
		return true;
	}

	HBITMAP	hRscbmp;			//	リソースから読み込んだひとかたまりのBitmap

	//	From Here 2001.7.1 GAE
	//	2001.7.1 GAE リソースをローカルファイル(sakuraディレクトリ) my_icons.bmp から読めるように
	// 2007.05.19 ryoji 設定ファイル優先に変更
	WCHAR szPath[_MAX_PATH];
	GetInidirOrExedir( szPath, FN_TOOL_BMP );
	hRscbmp = (HBITMAP)::LoadImage( nullptr, szPath, IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS );

	if( hRscbmp == nullptr ) {	// ローカルファイルの読み込み失敗時はリソースから取得
		//	リソースからBitmapを読み込む
		//	2003.09.29 wmlhq 環境によってアイコンがつぶれる
		hRscbmp = LoadMyToolFromModule( hInstance );
		if( hRscbmp == nullptr ){
			return false;
		}
	}

	hRscbmp = ConvertTo32bppBMP(hRscbmp, m_pBits, m_bmpWidth, m_bmpHeight);

	//	To Here 2001.7.1 GAE

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

	// システムのスモールアイコンサイズを取得する
	m_cx = ::GetSystemMetrics( SM_CXSMICON );
	m_cy = ::GetSystemMetrics( SM_CYSMICON );

	// アイコンサイズが異なる場合、拡大縮小する
	hRscbmp = ResizeToolIcons( hRscbmp, m_pBits, m_bmpWidth, m_bmpHeight, m_cTrans );
	if( hRscbmp == nullptr ){
		//	リソースからBitmapを読み込む
		hRscbmp = LoadMyToolFromModule( hInstance );
		if( hRscbmp == nullptr ){
			return false;
		}

		hRscbmp = ConvertTo32bppBMP(hRscbmp, m_pBits, m_bmpWidth, m_bmpHeight);

		// アイコンサイズが異なる場合、拡大縮小する
		hRscbmp = ResizeToolIcons( hRscbmp, m_pBits, m_bmpWidth, m_bmpHeight, m_cTrans );
		if( hRscbmp == nullptr ){
			return false;
		}
	}

	// クラスメンバに変更を保存する
	m_hIconBitmap = hRscbmp;

	m_hDC = ::CreateCompatibleDC(nullptr);

	::SelectObject(m_hDC, m_hIconBitmap);

	return true;
}

/*!
 * @brief アイコンの描画
 *
 * 指定されたDCの指定された座標にアイコンを描画する．
 *
 * @param [in] drawdc 描画するDevice Context
 * @param [in] x 描画するX座標
 * @param [in] y 描画するY座標
 * @param [in] imageNo 描画するアイコン番号
 * @param [in] fStyle 描画スタイル
 * @param [in] cx アイコン幅
 * @param [in] cy アイコン高さ
 * @note 描画スタイルとして有効なのは，ILD_NORMAL, ILD_MASK
 * 
 * @date 2003.07.21 genta 独自描画ルーチンを使う
 * @date 2003.08.30 genta 背景色を指定する引数を追加
 * @date 2003.09.06 genta Mocaさんの背景色透過処理に伴い，背景色引数削除
 * @date 2007.11.02 ryoji アイコン番号が負の場合は描画しない
 */
bool CImageListMgr::DrawToolIcon( HDC drawdc, LONG x, LONG y,
	int imageNo, bool enabled, LONG cx, LONG cy ) const
{
	if ( m_hIconBitmap == nullptr )
		return false;
	if ( imageNo < 0 || m_nIconCount < imageNo )
		return false;

	BLENDFUNCTION bf = { 0 };
	bf.BlendOp = AC_SRC_OVER;
	bf.SourceConstantAlpha = enabled ? 255 : 127; // 0-255, 全体の不透明度
	bf.AlphaFormat = AC_SRC_ALPHA; // ピクセル単位の α を使う
	::AlphaBlend(drawdc, x, y, cx, cy, m_hDC, (imageNo % MAX_X) * m_cx, (imageNo / MAX_X) * m_cy, cx, cy, bf);
	return true;
}

bool CImageListMgr::DrawToolIcon(uint32_t* pixels, int imageNo, bool enabled, LONG cx, LONG cy) const
{
	if (m_hIconBitmap == nullptr)
		return false;
	if (imageNo < 0 || m_nIconCount < imageNo)
		return false;

	auto sy = (imageNo / MAX_X) * m_cy;
	auto sx = (imageNo % MAX_X) * m_cx;
	for (LONG y = 0; y < cy; ++y) {
		const auto* srcLine = &m_pBits[m_bmpWidth * (sy + y) + sx];
		auto* dstLine = &pixels[cx * y];
		for (LONG x = 0; x < cx; ++x) {
			auto srcPixel = srcLine[x];
			if (!enabled && (srcPixel & 0xFF000000)) {
				srcPixel = 0x80000000 | (0x00FFFFFF & srcPixel);
			}
			dstLine[x] = srcPixel;
		}
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
int CImageListMgr::Add( const WCHAR* szPath )
{
	if ( (m_nIconCount % MAX_X) == 0 ) {
		Extend();
	}

	//アイコンを読み込む
	HBITMAP bmpSrc = (HBITMAP)::LoadImage( nullptr, szPath, IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION );

	if( bmpSrc == nullptr ) {
		return -1;
	}

	int imageNo = m_nIconCount++;

	// 仮想DCを生成して読込んだビットマップを展開する
	HDC hdcSrc = ::CreateCompatibleDC( nullptr );
	HGDIOBJ bmpSrcOld = ::SelectObject( hdcSrc, bmpSrc );

	//取得した画像の(0,0)の色を背景色として使う
	COLORREF cTransParent = ::GetPixel( hdcSrc, 0, 0 );

	// DIBセクションからサイズを取得する
	LONG nWidth, nHeight;
	{
		// DIBセクションを取得する
		DIBSECTION di = {};
		if ( !::GetObject( bmpSrc, sizeof( di ), &di ) ) {
			DEBUG_TRACE( L"GetObject() failed." );
			::SelectObject( hdcSrc, bmpSrcOld );
			::DeleteDC( hdcSrc );
			::DeleteObject( bmpSrc );
			return -1;
		}

		nWidth = di.dsBm.bmWidth;
		nHeight = di.dsBm.bmHeight;
		if ( nWidth != nHeight ) {
			DEBUG_TRACE( L"tool bitmap size is unexpected." );
			::SelectObject( hdcSrc, bmpSrcOld );
			::DeleteDC( hdcSrc );
			::DeleteObject( bmpSrc );
			return -1;
		}
	}

	// 作業DCの内容を出力DCに転送
	HDC hdcDst = ::CreateCompatibleDC( nullptr );
	HGDIOBJ hbmDstOld = ::SelectObject( hdcDst, m_hIconBitmap );
	::TransparentBlt( hdcDst, (imageNo % MAX_X) * cx(), (imageNo / MAX_X) * cy(), cx(), cy(),
		hdcSrc, 0, 0, nWidth, nHeight, cTransParent );

	// 後始末
	::SelectObject( hdcDst, hbmDstOld );
	::DeleteDC( hdcDst );
	::SelectObject( hdcSrc, bmpSrcOld );
	::DeleteDC( hdcSrc );

	::DeleteObject( bmpSrc );

	return imageNo;
}

// ツールイメージをリサイズする
HBITMAP CImageListMgr::ResizeToolIcons(
	HBITMAP		bmpSrc,				//!< [in] 変換前Bmpのハンドル
	uint32_t*&  pBits,
	LONG&		bmpWidth,
	LONG&		bmpHeight,
	COLORREF&	clrTransparent		//!< [out] 透過色
) const noexcept
{
	// 引数チェック
	if( bmpSrc == nullptr ){
		DEBUG_TRACE( L"tool bitmap is required." );
		return nullptr;
	}

	// DIBセクションを取得する
	DIBSECTION di = {};
	if ( !::GetObject( bmpSrc, sizeof( di ), &di ) ) {
		DEBUG_TRACE( L"GetObject() failed." );

		// 変換前Bmpを削除する
		::DeleteObject( bmpSrc );

		return nullptr;
	}

	// DIBセクションからサイズを取得する
	const auto bmWidth = di.dsBm.bmWidth;
	const auto bmHeight = di.dsBm.bmHeight;

	// 内部ビットマップの列数/段数は固定。
	const int cols = MAX_X;
	const int rows = MAX_Y;

	// アイコンサイズは固定。
	const int cx = 16;
	const int cy = cx;

	// 仮想DCを作成
	HDC hdcSrc = ::CreateCompatibleDC( nullptr );	//	転送元用
	if( hdcSrc == nullptr ){

		// 変換前Bmpを削除する
		::DeleteObject( bmpSrc );

		return nullptr;
	}

	//	まずbitmapをdcにmapする
	//	こうすることでCreateCompatibleBitmapで
	//	bmpSrcと同じ形式のbitmapを作れる．
	//	単にCreateCompatibleDC(0)で取得したdcや
	//	スクリーンのDCに対してCreateCompatibleBitmapを
	//	使うとモノクロBitmapになる．
	HGDIOBJ hFOldbmp = ::SelectObject( hdcSrc, bmpSrc );
	if( hFOldbmp == nullptr ){
		DEBUG_TRACE( L"SelectObject() failed." );

		// 変換前Bmpを削除する
		::DeleteObject( bmpSrc );

		// 仮想DCを削除する
		::DeleteDC( hdcSrc );

		return nullptr;
	}

	//	仮想DC(=変換前Bmp)の(0,0)の色を背景色として使う
	clrTransparent = ::GetPixel( hdcSrc, 0, 0 );
		
	const int cxSmIcon = m_cx;
	const int cySmIcon = m_cy;
	auto setAlpha = [&]() {
		uint32_t* pixels = (uint32_t*)pBits;
		auto clrTransparent = pixels[0];
		for (int j = 0; j < cxSmIcon * cols * cySmIcon * rows; ++j) {
			uint32_t& pixel = pixels[j];
			if (pixel == clrTransparent) {
				pixel = 0;
			}
			else {
				pixel |= 0xFF000000;
			}
		}
	};

	// アイコンサイズが異なる場合、拡大縮小する
	if ( cx != cxSmIcon ) {
		// 作業DCを作成する
		HDC hdcWork = ::CreateCompatibleDC( hdcSrc );

		BITMAPINFO bmi = {};
		bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
		bmi.bmiHeader.biWidth = cxSmIcon * cols;
		bmi.bmiHeader.biHeight = -cySmIcon * rows;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		bmpWidth = cxSmIcon * cols;
		bmpHeight = cySmIcon * rows;
		HBITMAP bmpWork = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, (void**)&pBits, nullptr, 0);
		HGDIOBJ bmpWorkOld = ::SelectObject( hdcWork, bmpWork );

		// 作業DCを透過色で塗りつぶす
		{
			HBRUSH hBrush = ::CreateSolidBrush( clrTransparent );
			HGDIOBJ hBrushOld = ::SelectObject( hdcWork, hBrush );
			::PatBlt( hdcWork, 0, 0, cxSmIcon * cols, cySmIcon * rows, PATCOPY );
			::SelectObject( hdcWork, hBrushOld );
			::DeleteObject( hBrush );
		}

		// ざっくり拡大縮小すると位置がずれるので1個ずつ変換する
		for ( int row = 0; row < rows; ++row ) {
			for ( int col = 0; col < cols; ++col ) {
				// 拡大・縮小する
				::StretchBlt(
					hdcWork,
					col * cxSmIcon,
					row * cySmIcon,
					cxSmIcon,
					cySmIcon,
					hdcSrc,
					col * cx,
					row * cy,
					cx,
					cy,
					SRCCOPY
				);
			}
		}

		setAlpha();

		// 作業DCで元Bmpを選択して変換後Bmpを解放する
		::SelectObject( hdcWork, bmpWorkOld );

		// 作業DCを削除する
		::DeleteDC( hdcWork );

		// 仮想DCで元Bmpを選択して変換前Bmpを解放する
		::SelectObject( hdcSrc, hFOldbmp );

		// 仮想DCを削除する
		::DeleteDC( hdcSrc );

		// 変換前Bmpを削除する
		::DeleteObject( bmpSrc );

		return bmpWork;
	}
	else {
		setAlpha();
	}

	// 仮想DCで元Bmpを選択して変換前Bmpを解放する
	::SelectObject( hdcSrc, hFOldbmp );

	// 仮想DCを削除する
	::DeleteDC( hdcSrc );

	return bmpSrc;
}

// ビットマップを一行（MAX_X個）拡張する
void CImageListMgr::Extend(bool bExtend)
{
	int curY = m_nIconCount / MAX_X;
	if( curY < MAX_Y )
		curY = MAX_Y;

	::SelectObject( m_hDC, m_hIconBitmap );

	//1行拡張したビットマップを作成
	HDC hDestDC = ::CreateCompatibleDC( nullptr );
	HBITMAP hDestBmp = ::CreateCompatibleBitmap( hDestDC, MAX_X * cx(), (curY + (bExtend ? 1 : 0)) * cy() );
	::SelectObject( hDestDC, hDestBmp );

	::BitBlt( hDestDC, 0, 0, MAX_X * cx(), curY * cy(), m_hDC, 0, 0, SRCCOPY );

	//拡張した部分は透過色で塗る
	if( bExtend ){
		FillSolidRect( hDestDC, 0, curY * cy(), MAX_X * cx(), cy(), m_cTrans );
	}

	::DeleteObject( m_hIconBitmap );
	::DeleteDC( m_hDC );

	//ビットマップの差し替え
	m_hIconBitmap = hDestBmp;
	m_hDC = hDestDC;
}

void CImageListMgr::ResetExtend()
{
	m_nIconCount = MAX_TOOLBAR_ICON_COUNT;
	Extend(false);
}
