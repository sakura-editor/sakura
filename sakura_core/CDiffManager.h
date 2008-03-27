//@@@ 2002.05.25 MIK
//2008.02.23 kobake 大整理

#pragma once

#include "util/design_template.h" //TSingleton
class CDocLine;
class CDocLineMgr;

//! DIFF情報定数
enum EDiffMark{
	MARK_DIFF_NONE		= 0,	//無変更
	MARK_DIFF_APPEND	= 1,	//追加
	MARK_DIFF_CHANGE	= 2,	//変更
	MARK_DIFF_DELETE	= 3,	//削除
	MARK_DIFF_DEL_EX	= 4,	//削除(EOF以降)
};

//! DIFF挙動の管理
class CDiffManager : public TSingleton<CDiffManager>{
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
	operator EDiffMark() const{ return (EDiffMark)m_nDiffed; }
	CLineDiffed& operator = (EDiffMark e){ m_nDiffed = e; return *this; }
private:
	uchar_t m_nDiffed;
};

//! 行のDIFF情報取得
class CDiffLineGetter{
public:
	CDiffLineGetter(const CDocLine* pcDocLine) : m_pcDocLine(pcDocLine) { }
	EDiffMark GetLineDiffMark() const;
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
	bool SearchDiffMark( CLogicInt , ESearchDirection , CLogicInt* );					//!< 差分検索
	void SetDiffMarkRange( EDiffMark nMode, CLogicInt nStartLine, CLogicInt nEndLine );	//!< 差分範囲の登録
private:
	CDocLineMgr* m_pcDocLineMgr;
};

