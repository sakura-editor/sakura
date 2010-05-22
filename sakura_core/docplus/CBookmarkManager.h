#pragma once

class CDocLine;
class CDocLineMgr;
class CBregexp;

#include "global.h" // ESearchDirection, SSearchOption


//! 行に付加するブックマーク情報
class CLineBookmarked{
public:
	CLineBookmarked() : m_bBookmarked(false) { }
	operator bool() const{ return m_bBookmarked; }
	CLineBookmarked& operator=(bool b){ m_bBookmarked = b; return *this; }
private:
	bool m_bBookmarked;
};

//! 行のブックマーク情報の取得
class CBookmarkGetter{
public:
	CBookmarkGetter(const CDocLine* pcDocLine) : m_pcDocLine(pcDocLine) { }
	bool IsBookmarked() const;
private:
	const CDocLine* m_pcDocLine;
};

//! 行のブックマーク情報の取得・設定
class CBookmarkSetter : public CBookmarkGetter{
public:
	CBookmarkSetter(CDocLine* pcDocLine) : CBookmarkGetter(pcDocLine), m_pcDocLine(pcDocLine) { }
	void SetBookmark(bool bFlag);
private:
	CDocLine* m_pcDocLine;
};

//! 行全体のブックマーク情報の管理
class CBookmarkManager{
public:
	CBookmarkManager(CDocLineMgr* pcDocLineMgr) : m_pcDocLineMgr(pcDocLineMgr) { }

	void ResetAllBookMark();															//!< ブックマークの全解除
	bool SearchBookMark( CLogicInt nLineNum, ESearchDirection , CLogicInt* pnLineNum );	//!< ブックマーク検索
	void SetBookMarks( wchar_t* );														//!< 物理行番号のリストからまとめて行マーク
	LPCWSTR GetBookMarks();																//!< 行マークされてる物理行番号のリストを作る
	void MarkSearchWord( const wchar_t* , const SSearchOption& , CBregexp* );			//!< 検索条件に該当する行にブックマークをセットする

private:
	CDocLineMgr* m_pcDocLineMgr;
};

