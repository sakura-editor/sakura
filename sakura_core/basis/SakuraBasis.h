/*! @file */
/*
	Copyright (C) 2008, kobake

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
#pragma once

#include <Windows.h> //POINT,LONG

#include "config/build_config.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      １次元型の定義                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
#ifdef USE_STRICT_INT
	// -- -- 厳格なintで単位型を定義 -- -- //

	#include "CStrictInteger.h"

	//ロジック単位
	using CLogicInt = CStrictInteger <
		MakeStrictId( StrictCategory::Logic, StrictAttribute::Horizontal),
		true,			//!< 基本型への暗黙の変換を許すかどうか
		ptrdiff_t		//!< 基本型
	>;

	//レイアウト単位
	using CLayoutInt = CStrictInteger <
		MakeStrictId( StrictCategory::Layout, StrictAttribute::Horizontal ),
		false,			//!< 基本型への暗黙の変換を許すかどうか
		ptrdiff_t		//!< 基本型
	>;

	//中間単位
	typedef CLaxInteger<ptrdiff_t> Int;

#else
	// -- -- 通常のintで単位型を定義
	//ロジック単位
	typedef ptrdiff_t CLogicInt;

	//レイアウト単位
	typedef ptrdiff_t CLayoutInt;

	//中間単位
	typedef ptrdiff_t Int;

#endif

typedef CLogicInt  CLogicXInt;
typedef CLogicInt  CLogicYInt;
typedef CLayoutInt CLayoutXInt;
typedef CLayoutInt CLayoutYInt;
typedef int CPixelYInt;
typedef int CPixelXInt;

typedef CLayoutXInt CHabaXInt;
typedef int         CKetaXInt;

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ２次元型の定義                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
#include "CStrictRange.h"
#include "CStrictPoint.h"
#include "CStrictRect.h"

//ロジック単位
struct SLogicPoint{ CLogicXInt x; CLogicYInt y; }; //基底構造体
typedef CStrictPoint<SLogicPoint>	CLogicPoint;
typedef CRangeBase<CLogicPoint>		CLogicRange;
typedef CStrictRect<CLogicPoint>	CLogicRect;

//レイアウト単位
struct SLayoutPoint{ CLayoutXInt x; CLayoutXInt y; }; //基底構造体
typedef CStrictPoint<SLayoutPoint>	CLayoutPoint;
typedef CRangeBase<CLayoutPoint>	CLayoutRange;
typedef CStrictRect<CLayoutPoint>	CLayoutRect;

//ゆるい単位
#include "CMyPoint.h"
typedef CRangeBase<CMyPoint>     SelectionRange;

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ツール                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "CMyRect.h"

/*!
	pt1 - pt2の結果を返す
	Yを優先して比較。Yが同一なら、Xで比較。

	@return <0 pt1 <  pt2
	@return 0  pt1 == pt2
	@return >0 pt1 >  pt2
*/
template <class POINT_T>
inline int PointCompare(const POINT_T& pt1, const POINT_T& pt2)
{
	if (pt1.y != pt2.y)return (Int)(pt1.y - pt2.y);
	return (Int)(pt1.x - pt2.x);
}

//! 2点を対角とする矩形を求める
template <class POINT_T>
inline void TwoPointToRect(
	CStrictRect<POINT_T>*	prcRect,
	POINT_T					pt1,
	POINT_T					pt2
)
{
	if( prcRect != NULL ){
		prcRect->left   = std::min( pt1.x, pt2.x );
		prcRect->top    = std::max( pt1.y, pt2.y );
		prcRect->right  = std::min( pt1.x, pt2.x );
		prcRect->bottom = std::max( pt1.y, pt2.y );
	}
}

//! 2点を対角とする矩形を求める
template <class POINT_T>
inline void TwoPointToRect(
	RECT*	prcRect,
	POINT_T	pt1,
	POINT_T	pt2
)
{
	auto *prc = reinterpret_cast<CStrictRect<POINT>*>( prcRect );
	POINT ptA = { static_cast<LONG>( (Int)pt1.x ), static_cast<LONG>( (Int)pt1.y ) };
	POINT ptB = { static_cast<LONG>( (Int)pt2.x ), static_cast<LONG>( (Int)pt2.y ) };
	TwoPointToRect( prc, ptA, ptB );
}

//変換関数
template <class POINT_T>
inline void TwoPointToRange(
	CRangeBase<POINT_T>* prangeDst,
	POINT_T pt1,
	POINT_T pt2
)
{
	CMyRect rc;
	TwoPointToRect(&rc,pt1,pt2);
	prangeDst->SetFrom(POINT_T(rc.UpperLeft()));
	prangeDst->SetTo(POINT_T(rc.LowerRight()));
}
