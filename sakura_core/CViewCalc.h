/*
	X値の単位変換関数群。
*/

#pragma once

class CLayout;
class CDocLine;
class CEditView;

class CViewCalc{
protected:
	//外部依存
	CLayoutInt GetTabSpace() const;

public:
	CViewCalc(const CEditView* pOwner) : m_pOwner(pOwner) { }
	virtual ~CViewCalc(){}

	//単位変換: レイアウト→ロジック
	CLogicInt  LineColmnToIndex ( const CLayout*  pcLayout,  CLayoutInt nColumn ) const;		/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */		// @@@ 2002.09.28 YAZAKI
	CLogicInt  LineColmnToIndex ( const CDocLine* pcDocLine, CLayoutInt nColumn ) const;		/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */		// @@@ 2002.09.28 YAZAKI
	CLogicInt  LineColmnToIndex2( const CLayout*  pcLayout,  CLayoutInt nColumn, CLayoutInt* pnLineAllColLen ) const;	/* 指定された桁に対応する行のデータ内の位置を調べる Ver0 */		// @@@ 2002.09.28 YAZAKI

	//単位変換: ロジック→レイアウト
	CLayoutInt LineIndexToColmn ( const CLayout*  pcLayout,  CLogicInt nIndex ) const;		// 指定された行のデータ内の位置に対応する桁の位置を調べる	// @@@ 2002.09.28 YAZAKI
	CLayoutInt LineIndexToColmn ( const CDocLine* pcLayout,  CLogicInt nIndex ) const;		// 指定された行のデータ内の位置に対応する桁の位置を調べる	// @@@ 2002.09.28 YAZAKI

private:
	const CEditView* m_pOwner;
};
