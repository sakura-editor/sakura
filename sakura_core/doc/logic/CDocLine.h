﻿/*!	@file
	@brief 文書データ1行

	@author Norio Nakatani

	@date 2001/12/03 hor しおり(bookmark)機能追加に伴うメンバー追加
	@date 2001/12/18 hor bookmark, 修正フラグのアクセス関数化
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, MIK
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CDOCLINE_B592082C_24CC_41A6_A931_774BE9675F42_H_
#define SAKURA_CDOCLINE_B592082C_24CC_41A6_A931_774BE9675F42_H_
#pragma once

#include "util/design_template.h"
#include "CEol.h"
#include "mem/CMemory.h"
#include "mem/CNativeW.h"

#include "docplus/CBookmarkManager.h"
#include "docplus/CDiffManager.h"
#include "docplus/CModifyManager.h"
#include "docplus/CFuncListManager.h"

class CDocLine;
class COpeBlk;

#pragma pack(push,1)

//!	文書データ1行
class CDocLine{
protected:
	friend class CDocLineMgr; //######仮
public:
	//コンストラクタ・デストラクタ
	CDocLine();
	~CDocLine();

	//判定
	bool			IsEmptyLine() const;		//	このCDocLineが空行（スペース、タブ、改行記号のみの行）かどうか。

	//データ取得
	CLogicInt		GetLengthWithoutEOL() const			{ return m_cLine.GetStringLength() - m_cEol.GetLen(); } //!< 戻り値は文字単位。
	const wchar_t*	GetPtr() const						{ return m_cLine.GetStringPtr(); }
	CLogicInt		GetLengthWithEOL() const			{ return m_cLine.GetStringLength(); }	//	CMemoryIterator用
#ifdef USE_STRICT_INT
	const wchar_t*	GetDocLineStrWithEOL(int* pnLen) const //###仮の名前、仮の対処
	{
		CLogicInt n;
		const wchar_t* p = GetDocLineStrWithEOL(&n);
		*pnLen = n;
		return p;
	}
#endif
	const wchar_t*	GetDocLineStrWithEOL(CLogicInt* pnLen) const //###仮の名前、仮の対処
	{
		if(this){ // TODO: Remove "this" check
			*pnLen = GetLengthWithEOL(); return GetPtr();
		}
		else{
			*pnLen = 0; return NULL;
		}
	}
	static const wchar_t* GetDocLineStrWithEOL_Safe(const CDocLine* docline, CLogicInt* pnLen) //###仮の名前、仮の対処
	{
		if(docline){
			return docline->GetDocLineStrWithEOL(pnLen);
		}
		else{
			*pnLen = 0; return NULL;
		}
	}
	CStringRef GetStringRefWithEOL() const //###仮の名前、仮の対処
	{
		if(this){ // TODO: Remove "this" check
			return CStringRef(GetPtr(),GetLengthWithEOL());
		}
		else{
			return CStringRef(NULL,0);
		}
	}
	static CStringRef GetStringRefWithEOL_Safe(const CDocLine* docline) //###仮の名前、仮の対処
	{
		if(docline){
			return docline->GetStringRefWithEOL();
		}
		else{
			return CStringRef(NULL, 0);
		}
	}
	const CEol& GetEol() const{ return m_cEol; }
	void SetEol(const CEol& cEol, COpeBlk* pcOpeBlk);
	void SetEol(bool bEnableExtEol); // 現在のバッファから設定

	const CNativeW& _GetDocLineDataWithEOL() const { return m_cLine; } //###仮
	CNativeW& _GetDocLineData() { return m_cLine; }

	//データ設定
	void SetDocLineString(const wchar_t* pData, int nLength, bool bEnableExtEol);
	void SetDocLineString(const CNativeW& cData, bool bEnableExtEol);
	void SetDocLineStringMove(CNativeW* pcData, bool bEnableExtEol);

	//チェーン属性
	CDocLine* GetPrevLine(){ return m_pPrev; }
	const CDocLine* GetPrevLine() const { return m_pPrev; }
	CDocLine* GetNextLine(){ return m_pNext; }
	const CDocLine* GetNextLine() const { return m_pNext; }
	void _SetPrevLine(CDocLine* pcDocLine){ m_pPrev = pcDocLine; }
	void _SetNextLine(CDocLine* pcDocLine){ m_pNext = pcDocLine; }

private: //####
	CDocLine*	m_pPrev;	//!< 一つ前の要素
	CDocLine*	m_pNext;	//!< 一つ後の要素
private:
	CNativeW	m_cLine;	//!< データ  2007.10.11 kobake ポインタではなく、実体を持つように変更
public:
	//拡張情報 $$分離中
	struct MarkType{
		CLineModified	m_cModified;	//変更フラグ
		CLineBookmarked	m_cBookmarked;	//ブックマーク
		CLineFuncList	m_cFuncList;	//関数リストマーク
		CLineDiffed		m_cDiffmarked;	//DIFF差分情報
	};
	MarkType m_sMark;
private:
	CEol		m_cEol;		//!< 行末コード

	DISALLOW_COPY_AND_ASSIGN(CDocLine);
};

#pragma pack(pop)
#endif /* SAKURA_CDOCLINE_B592082C_24CC_41A6_A931_774BE9675F42_H_ */
