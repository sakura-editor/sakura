/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CVIEWCALC_1441B90A_43D1_4C24_BC92_B887BD104129_H_
#define SAKURA_CVIEWCALC_1441B90A_43D1_4C24_BC92_B887BD104129_H_
#pragma once

#include "doc/layout/CTsvModeInfo.h"

/*
	X値の単位変換関数群。
*/

class CLayout;
class CDocLine;
class CEditView;

class CViewCalc{
protected:
	//外部依存
	CLayoutInt GetTabSpace() const;
	CPixelXInt GetCharSpacing() const;
	CTsvModeInfo& GetTsvMode() const;

public:
	CViewCalc(const CEditView* pOwner) : m_pOwner(pOwner) { }
	virtual ~CViewCalc(){}

	//単位変換: レイアウト→ロジック
	CLogicInt  LineColumnToIndex ( const CLayout*  pcLayout,  CLayoutInt nColumn ) const;		/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */		// @@@ 2002.09.28 YAZAKI
	CLogicInt  LineColumnToIndex ( const CDocLine* pcDocLine, CLayoutInt nColumn ) const;		/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */		// @@@ 2002.09.28 YAZAKI
	CLogicInt  LineColumnToIndex2( const CLayout*  pcLayout,  CLayoutInt nColumn, CLayoutInt* pnLineAllColLen ) const;	/* 指定された桁に対応する行のデータ内の位置を調べる Ver0 */		// @@@ 2002.09.28 YAZAKI

	//単位変換: ロジック→レイアウト
	CLayoutInt LineIndexToColumn ( const CLayout*  pcLayout,  CLogicInt nIndex ) const;		// 指定された行のデータ内の位置に対応する桁の位置を調べる	// @@@ 2002.09.28 YAZAKI
	CLayoutInt LineIndexToColumn ( const CDocLine* pcLayout,  CLogicInt nIndex ) const;		// 指定された行のデータ内の位置に対応する桁の位置を調べる	// @@@ 2002.09.28 YAZAKI

private:
	const CEditView* m_pOwner;
};
#endif /* SAKURA_CVIEWCALC_1441B90A_43D1_4C24_BC92_B887BD104129_H_ */
