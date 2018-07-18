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
#ifndef SAKURA_CVIEWCALC_F7EB84D2_C716_4183_AF9C_197AEEC5B7A9_H_
#define SAKURA_CVIEWCALC_F7EB84D2_C716_4183_AF9C_197AEEC5B7A9_H_

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

#endif /* SAKURA_CVIEWCALC_F7EB84D2_C716_4183_AF9C_197AEEC5B7A9_H_ */
/*[EOF]*/
