﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CBOOKMARKMANAGER_982C4B01_0D6B_4F6E_9DD8_5CA15DE5EC17_H_
#define SAKURA_CBOOKMARKMANAGER_982C4B01_0D6B_4F6E_9DD8_5CA15DE5EC17_H_
#pragma once

#include "_main/global.h" // ESearchDirection, SSearchOption

class CDocLine;
class CDocLineMgr;
class CBregexp;

#include "CSearchAgent.h"

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
	void SetBookMarks( wchar_t* pMarkLines );											//!< 物理行番号のリストからまとめて行マーク
	LPCWSTR GetBookMarks();																//!< 行マークされてる物理行番号のリストを作る
	void MarkSearchWord( const CSearchStringPattern& );			//!< 検索条件に該当する行にブックマークをセットする

private:
	CDocLineMgr* m_pcDocLineMgr;
};
#endif /* SAKURA_CBOOKMARKMANAGER_982C4B01_0D6B_4F6E_9DD8_5CA15DE5EC17_H_ */
