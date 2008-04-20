#pragma once

#include <windows.h> //POINT,LONG

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

