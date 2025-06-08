/*! @file */
//@@@ 2002.05.25 MIK
//2008.02.23 kobake 大整理
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDIFFMANAGER_65AC06D5_0F1A_4E27_94E5_53F786F76D9F_H_
#define SAKURA_CDIFFMANAGER_65AC06D5_0F1A_4E27_94E5_53F786F76D9F_H_
#pragma once

#include "view/colors/EColorIndexType.h"
#include "util/design_template.h" //TSingleton
#include "basis/SakuraBasis.h"
#include "_main/global.h"

class CDocLine;
class CDocLineMgr;
class CGraphics;

//! DIFF情報定数
enum EDiffMark : char {
	MARK_DIFF_NONE		= 0,	//!< 無変更
	MARK_DIFF_APPEND	= 1,	//!< 追加
	MARK_DIFF_CHANGE	= 2,	//!< 変更
	MARK_DIFF_DELETE	= 3,	//!< 削除
	MARK_DIFF_DEL_EX	= 4,	//!< 削除(EOF以降)
};

//! DIFF挙動の管理
class CDiffManager : public TSingleton<CDiffManager>{
	friend class TSingleton<CDiffManager>;
	CDiffManager() = default;

public:
	void SetDiffUse(bool b){ m_bIsDiffUse = b; }
	bool IsDiffUse() const{ return m_bIsDiffUse; }		//!< DIFF使用中

private:
	bool	m_bIsDiffUse;		//!< DIFF差分表示実施中 @@@ 2002.05.25 MIK
};

//! 行に付加するDIFF情報
class CLineDiffed{
public:
	CLineDiffed() : m_nDiffed(MARK_DIFF_NONE) { }
	operator EDiffMark() const{ return m_nDiffed; }
	CLineDiffed& operator = (EDiffMark e){ m_nDiffed = e; return *this; }
private:
	EDiffMark m_nDiffed;
};

//! 行のDIFF情報取得
class CDiffLineGetter{
public:
	CDiffLineGetter(const CDocLine* pcDocLine) : m_pcDocLine(pcDocLine) { }
	EDiffMark GetLineDiffMark() const;
	bool GetDiffColor(EColorIndexType* nColor) const;
	bool DrawDiffMark(CGraphics& gr, int y, int nLineHeight, COLORREF color) const;
private:
	const CDocLine* m_pcDocLine;
};

//! 行のDIFF情報設定
class CDiffLineSetter{
public:
	CDiffLineSetter(CDocLine* pcDocLine) : m_pcDocLine(pcDocLine) { }
	void SetLineDiffMark(EDiffMark mark);
private:
	CDocLine* m_pcDocLine;
};

//! 行全体のDIFF情報管理
class CDiffLineMgr{
public:
	CDiffLineMgr(CDocLineMgr* pcDocLineMgr) : m_pcDocLineMgr(pcDocLineMgr) { }
	void ResetAllDiffMark();															//!< 差分表示の全解除
	bool SearchDiffMark( CLogicInt nLineNum, ESearchDirection bPrevOrNext, CLogicInt* pnLineNum );	//!< 差分検索
	void SetDiffMarkRange( EDiffMark nMode, CLogicInt nStartLine, CLogicInt nEndLine );	//!< 差分範囲の登録
private:
	CDocLineMgr* m_pcDocLineMgr;
};
#endif /* SAKURA_CDIFFMANAGER_65AC06D5_0F1A_4E27_94E5_53F786F76D9F_H_ */
