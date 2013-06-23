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
#ifndef SAKURA_SAKURABASIS_DA3203FF_3FD7_4AD1_BC64_E85C8171335E9_H_
#define SAKURA_SAKURABASIS_DA3203FF_3FD7_4AD1_BC64_E85C8171335E9_H_

#include <Windows.h> //POINT,LONG

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �P�����^�̒�`                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
#ifdef USE_STRICT_INT
	// -- -- ���i��int�ŒP�ʌ^���` -- -- //

	#include "CStrictInteger.h"

	//���W�b�N�P��
	typedef CStrictInteger <
		0,		//!< �^�𕪂��邽�߂̐��l�B
		true,	//!< int�Ƃ̔�r���������ǂ���
		true,	//!< int�Ƃ̉����Z���������ǂ���
		true,	//!< int�ւ̈Öق̕ϊ����������ǂ���
		true	//!< int�̑�����������ǂ���
	>
	CLogicInt;

	//���C�A�E�g�P��
	typedef CStrictInteger <
		1,		//!< �^�𕪂��邽�߂̐��l�B
		true,	//!< int�Ƃ̔�r���������ǂ���
		true,	//!< int�Ƃ̉����Z���������ǂ���
		false,	//!< int�ւ̈Öق̕ϊ����������ǂ���
		true	//!< int�̑�����������ǂ���
	>
	CLayoutInt;

#else
	// -- -- �ʏ��int�ŒP�ʌ^���`

	//���W�b�N�P��
	typedef int CLogicInt;

	//���C�A�E�g�P��
	typedef int CLayoutInt;

#endif

typedef CLogicInt  CLogicXInt;
typedef CLogicInt  CLogicYInt;
typedef CLayoutInt CLayoutXInt;
typedef CLayoutInt CLayoutYInt;
typedef int CPixelYInt;
typedef int CPixelXInt;

#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
typedef CLayoutXInt CHabaXInt;
typedef int         CKetaXInt;
#else
typedef CPixelXInt  CHabaXInt;
typedef CLayoutXInt CKetaXInt;
#endif


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �Q�����^�̒�`                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
#include "CStrictRange.h"
#include "CStrictPoint.h"
#include "CStrictRect.h"

//���W�b�N�P��
struct SLogicPoint{ CLogicInt x; CLogicInt y; }; //���\����
typedef CStrictPoint<SLogicPoint, CLogicInt>	CLogicPoint;
typedef CRangeBase<CLogicPoint>					CLogicRange;
typedef CStrictRect<CLogicInt, CLogicPoint>		CLogicRect;

//���C�A�E�g�P��
struct SLayoutPoint{ CLayoutInt x; CLayoutInt y; }; //���\����
typedef CStrictPoint<SLayoutPoint, CLayoutInt>	CLayoutPoint;
typedef CRangeBase<CLayoutPoint>				CLayoutRange;
typedef CStrictRect<CLayoutInt, CLayoutPoint>	CLayoutRect;

//��邢�P��
#include "CMyPoint.h"
typedef CRangeBase<CMyPoint>     SelectionRange;



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �c�[��                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "CMyRect.h"

//�ϊ��֐�
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


//! 2�_��Ίp�Ƃ����`�����߂�
template <class T, class INT_TYPE>
inline void TwoPointToRect(
	CStrictRect<INT_TYPE, CStrictPoint<T,INT_TYPE> >*	prcRect,
	CStrictPoint<T,INT_TYPE>							pt1,
	CStrictPoint<T,INT_TYPE>							pt2
)
{
	if( pt1.y < pt2.y ){
		prcRect->top	= pt1.GetY2();
		prcRect->bottom	= pt2.GetY2();
	}else{
		prcRect->top	= pt2.GetY2();
		prcRect->bottom	= pt1.GetY2();
	}
	if( pt1.x < pt2.x ){
		prcRect->left	= pt1.GetX2();
		prcRect->right	= pt2.GetX2();
	}else{
		prcRect->left	= pt2.GetX2();
		prcRect->right	= pt1.GetX2();
	}
}

#endif /* SAKURA_SAKURABASIS_DA3203FF_3FD7_4AD1_BC64_E85C8171335E9_H_ */
/*[EOF]*/
