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

#include <cmath>
#include <array>
#include <list>
#include <functional>

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
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL );
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

	//	From Here 2001.7.1 GAE
	//	2001.7.1 GAE リソースをローカルファイル(sakuraディレクトリ) my_icons.bmp から読めるように
	// 2007.05.19 ryoji 設定ファイル優先に変更
	WCHAR szPath[_MAX_PATH];
	GetInidirOrExedir( szPath, FN_TOOL_BMP );
	hRscbmp = (HBITMAP)::LoadImage( NULL, szPath, IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS );

	if( hRscbmp == NULL ) {	// ローカルファイルの読み込み失敗時はリソースから取得
		//	リソースからBitmapを読み込む
		//	2003.09.29 wmlhq 環境によってアイコンがつぶれる
		hRscbmp = LoadMyToolFromModule( hInstance );
		if( hRscbmp == NULL ){
			return false;
		}
	}
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
	hRscbmp = ResizeToolIcons( hRscbmp, m_cTrans );
	if( hRscbmp == NULL ){
		//	リソースからBitmapを読み込む
		hRscbmp = LoadMyToolFromModule( hInstance );
		if( hRscbmp == NULL ){
			return false;
		}

		// アイコンサイズが異なる場合、拡大縮小する
		hRscbmp = ResizeToolIcons( hRscbmp, m_cTrans );
		if( hRscbmp == NULL ){
			return false;
		}
	}

	// クラスメンバに変更を保存する
	m_hIconBitmap = hRscbmp;

	return true;
}

/*! RGBQUADラッパー
 *  STLコンテナに入れられるよう == 演算子を実装したもの。
 */
struct MyRGBQUAD : tagRGBQUAD
{
	using tagRGBQUAD::rgbRed;
	using tagRGBQUAD::rgbGreen;
	using tagRGBQUAD::rgbBlue;
	using tagRGBQUAD::rgbReserved;

	MyRGBQUAD() noexcept
		: tagRGBQUAD()
	{
		rgbBlue = 0;
		rgbGreen = 0;
		rgbRed = 0;
		rgbReserved = 0;
	}
	MyRGBQUAD( std::initializer_list<BYTE> a ) noexcept
		: tagRGBQUAD()
	{
		assert( a.size() <= 4 );
		decltype(rgbBlue) *p = &rgbBlue;
		for ( auto it = a.begin(); it != a.end(); ++it, ++p ) {
			*p = *it;
		}
	}
	bool operator == ( const RGBQUAD &rhs ) const noexcept
	{
		return rgbBlue == rhs.rgbBlue
			&& rgbGreen == rhs.rgbGreen
			&& rgbRed == rhs.rgbRed
			&& rgbReserved == rhs.rgbReserved;
	}
	bool operator != ( const RGBQUAD &rhs ) const noexcept
	{
		return !(*this == rhs);
	}
	operator COLORREF ( void ) const noexcept
	{
		return RGB( rgbRed, rgbGreen, rgbBlue );
	}
};

// HLS色情報タプル
typedef std::tuple<double, double, double> _HlsTuple;
enum { HLS_H, HLS_S, HLS_L, };

/*!
 * @brief RGB⇒HLS(円柱モデル)変換する
 */
_HlsTuple ToHLS( const COLORREF color )
{
	auto R = (double) GetRValue( color ) / 255.;
	auto G = (double) GetGValue( color ) / 255.;
	auto B = (double) GetBValue( color ) / 255.;
	auto MIN = std::min( { R, G, B } );
	auto MAX = std::max( { R, G, B } );
	auto M = MAX + MIN;
	auto m = MAX - MIN;
	double H;
	if ( MIN == MAX ) {
		H = std::numeric_limits<double>::infinity();
	}
	else if ( MIN == B ) {
		H = 60. * (m == 0 ? 0 : ((G - R) / m)) + 60.;
	}
	else if ( MIN == R ) {
		H = 60. * (m == 0 ? 0 : ((B - G) / m)) + 180.;
	}
	else if ( MIN == G ) {
		H = 60. * (m == 0 ? 0 : ((R - B) / m)) + 300.;
	}
	auto L = M / 2.;
	auto S = M == 0 ? 0 : m / (1 - std::abs( M - 1 ));
	return std::make_tuple( H, S, L );
}

/*!
 * @brief HLS(円柱モデル)⇒RGB変換する
 */
COLORREF FromHLS( const _HlsTuple &hls )
{
	auto H = std::get<HLS_H>( hls );
	auto S = std::get<HLS_S>( hls );
	auto L = std::get<HLS_L>( hls );

	// 彩度の範囲を補正する
	if ( S < 0 ) S = 0;
	if ( 1 < S ) S = 1;

	// 輝度の範囲を補正する
	if ( L < 0 ) L = 0;
	if ( 1 < L ) L = 1;

	// 色相が無効値（＝白黒）の場合
	if ( std::isinf( H ) ) {
		return RGB( L * 255, L * 255, L * 255 );
	}

	// 色相の範囲を補正する
	while ( H < 0 ) H = 360 - H;
	while ( 360 <= H ) H = H - 360;

	double R, G, B;
	double MIN = L + S * (1 - std::abs( 2 * L - 1 )) / 2;
	double MAX = L - S * (1 - std::abs( 2 * L - 1 )) / 2;
	if ( H < 60 ) {
		R = MAX;
		G = MAX + (MAX - MIN) * H / 60;
		B = MIN;
	}
	else if ( H < 120 ) {
		R = MIN + (MAX - MIN) * (120 - H) / 60;
		G = MAX;
		B = MIN;
	}
	else if ( H < 180 ) {
		R = MIN;
		G = MAX;
		B = MIN + (MAX - MIN) * (H - 120) / 60;
	}
	else if ( H < 240 ) {
		R = MIN;
		G = MIN + (MAX - MIN) * (240 - H) / 60;
		B = MAX;
	}
	else if ( H < 300 ) {
		R = MIN + (MAX - MIN) * (H - 240) / 60;
		G = MIN;
		B = MAX;
	}
	else { //if ( H < 360 ) {
		R = MAX;
		G = MIN;
		B = MIN + (MAX - MIN) * (360 - H) / 60;
	}
	return RGB( R * 255, G * 255, B * 255 );
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
	int nXSrc, 
	int nYSrc
) const
{
	// 仮想DCを生成してビットマップを展開する
	const HBITMAP &bmpSrc = m_hIconBitmap;
	HDC hdcSrc = ::CreateCompatibleDC( drawdc );
	HGDIOBJ bmpSrcOld = ::SelectObject( hdcSrc, bmpSrc );

	// 透過色の変数名が分かりづらいので別名定義する
	const COLORREF &cTransparent = m_cTrans;

	// 透過色を考慮して転送
	::TransparentBlt( drawdc, nXDest, nYDest, nWidth, nHeight,
		hdcSrc, nXSrc, nYSrc, cx(), cy(), cTransparent );

	// 後始末
	::SelectObject( hdcSrc, bmpSrcOld );
	::DeleteDC( hdcSrc );
	return;
}


/*! メニューアイコンの淡色表示

	@author Nakatani
	
	@date 2003.07.21 genta 以前のCMenuDrawerより移転復活
	@date 2003.08.27 Moca 背景色は透過処理する
*/
void CImageListMgr::MyDitherBlt( HDC drawdc, int nXDest, int nYDest,
	int nWidth, int nHeight, int nXSrc, int nYSrc ) const
{
	// 仮想DCを生成してビットマップを展開する
	const HBITMAP &bmpSrc = m_hIconBitmap;
	HDC hdcSrc = ::CreateCompatibleDC( drawdc );
	HGDIOBJ bmpSrcOld = ::SelectObject( hdcSrc, bmpSrc );

	// 作業DCを作成
	HDC hdcWork = ::CreateCompatibleDC( drawdc );

	// DIB作成
	BITMAPINFO bmi;
	char* pBits;
	BITMAPINFOHEADER& bmih = bmi.bmiHeader;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = nWidth;
	assert(nHeight > 0);
	bmih.biHeight = -nHeight; // top down
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;
	bmih.biCompression = BI_RGB;
	const int lineStride = ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) / 8);
	bmih.biSizeImage = lineStride * nHeight;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;
	HBITMAP bmpWork = ::CreateDIBSection((HDC)0, &bmi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
	HGDIOBJ bmpWorkOld = ::SelectObject( hdcWork, bmpWork );

	// 作業DCに転送
	::StretchBlt( hdcWork, 0, 0, nWidth, nHeight,
		hdcSrc, nXSrc, nYSrc, cx(), cy(), SRCCOPY );

	// ディザカラーを決める
	// 淡色テキスト色が背景色と同じなら灰色に避ける、違うなら淡色テキストを使う。
	COLORREF grayText = ::GetSysColor( COLOR_GRAYTEXT );
	COLORREF btnFace = ::GetSysColor( COLOR_3DFACE );
	COLORREF textColor = grayText == btnFace ? RGB( 0x80, 0x80, 0x80 ) : grayText;
	auto textColorH = ToHLS( textColor );
	double textColorL;
	{
		auto r = GetRValue( textColor );
		auto g = GetGValue( textColor );
		auto b = GetBValue( textColor );
		textColorL = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0; //[0,1]
	}
	double textColorR = (1.0 - textColorL) / 255.0;

	// ディザカラー256諧調の配列を作る
	std::array<COLORREF, 0x100> ditherColors;
	for ( size_t i = 0; i < ditherColors.size(); ++i ) {
		auto ditherColorH( textColorH );
		std::get<HLS_L>(ditherColorH) = textColorL + i * textColorR;
		ditherColors[i] = FromHLS( ditherColorH );
	}

	// 透過色の変数名が分かりづらいので別名定義する
	const COLORREF cTransparent = m_cTrans;

	// スキャンライン全行を順に取得して処理する
	for (auto n = 0; n < nHeight; ++n) {

		// スキャンラインを1ピクセルずつ処理する
		auto pixels = reinterpret_cast<MyRGBQUAD*>(pBits);
		for ( auto m = 0; m < nWidth; ++m ) {
			MyRGBQUAD& px = pixels[m];

			// 透過色はスキップする
			if ( px == cTransparent ) continue;

			// ピクセル色をディザカラーに変換する
			auto r = px.rgbRed;
			auto g = px.rgbGreen;
			auto b = px.rgbBlue;
			auto mono = (77 * r + 150 * g + 29 * b) >> 8; //[0,255]

			// ディザカラーを書き込む
			px.rgbRed = GetRValue( ditherColors[mono] );
			px.rgbGreen = GetGValue( ditherColors[mono] );
			px.rgbBlue = GetBValue( ditherColors[mono] );
		}

		pBits += lineStride;
	}

	// 背景を透過させつつ転送
	::TransparentBlt( drawdc, nXDest, nYDest, nWidth, nHeight,
		hdcWork, 0, 0, nWidth, nHeight, cTransparent );

	// 後始末
	::SelectObject( hdcWork, bmpWorkOld );
	::DeleteObject( bmpWork );
	::DeleteDC( hdcWork );
	::SelectObject( hdcSrc, bmpSrcOld );
	::DeleteDC( hdcSrc );
	return;
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
	int imageNo, DWORD fStyle, LONG cx, LONG cy ) const
{
	if ( m_hIconBitmap == NULL )
		return false;
	if ( imageNo < 0 || m_nIconCount < imageNo )
		return false;

	if ( (fStyle&ILD_MASK) == ILD_MASK ) {
		MyDitherBlt( drawdc, x, y, cx, cy,
			(imageNo % MAX_X) * m_cx, (imageNo / MAX_X) * m_cy );
	} else {
		MyBitBlt( drawdc, x, y, cx, cy,
			(imageNo % MAX_X) * m_cx, (imageNo / MAX_X) * m_cy );
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
	HDC hdcDst = ::CreateCompatibleDC( NULL );
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
	COLORREF&	clrTransparent		//!< [out] 透過色
) const noexcept
{
	// 引数チェック
	if( bmpSrc == NULL ){
		DEBUG_TRACE( L"tool bitmap is required." );
		return NULL;
	}

	// DIBセクションを取得する
	DIBSECTION di = {};
	if ( !::GetObject( bmpSrc, sizeof( di ), &di ) ) {
		DEBUG_TRACE( L"GetObject() failed." );

		// 変換前Bmpを削除する
		::DeleteObject( bmpSrc );

		return NULL;
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
	HDC hdcSrc = ::CreateCompatibleDC( 0 );	//	転送元用
	if( hdcSrc == NULL ){

		// 変換前Bmpを削除する
		::DeleteObject( bmpSrc );

		return NULL;
	}

	//	まずbitmapをdcにmapする
	//	こうすることでCreateCompatibleBitmapで
	//	bmpSrcと同じ形式のbitmapを作れる．
	//	単にCreateCompatibleDC(0)で取得したdcや
	//	スクリーンのDCに対してCreateCompatibleBitmapを
	//	使うとモノクロBitmapになる．
	HGDIOBJ hFOldbmp = ::SelectObject( hdcSrc, bmpSrc );
	if( hFOldbmp == NULL ){
		DEBUG_TRACE( L"SelectObject() failed." );

		// 変換前Bmpを削除する
		::DeleteObject( bmpSrc );

		// 仮想DCを削除する
		::DeleteDC( hdcSrc );

		return NULL;
	}

	//	仮想DC(=変換前Bmp)の(0,0)の色を背景色として使う
	clrTransparent = ::GetPixel( hdcSrc, 0, 0 );
		
	const int cxSmIcon = m_cx;
	const int cySmIcon = m_cy;

	// アイコンサイズが異なる場合、拡大縮小する
	if ( cx != cxSmIcon ) {
		// 作業DCを作成する
		HDC hdcWork = ::CreateCompatibleDC( hdcSrc );
		HBITMAP bmpWork = ::CreateCompatibleBitmap( hdcSrc, cxSmIcon * cols, cySmIcon * rows );
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
				::TransparentBlt(
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
					m_cTrans
				);
			}
		}

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
