/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CMODIFYMANAGER_12000875_531F_42DC_A6B0_231385193CB8_H_
#define SAKURA_CMODIFYMANAGER_12000875_531F_42DC_A6B0_231385193CB8_H_
#pragma once

#include "util/design_template.h" //TSingleton
#include "doc/CDocListener.h" // CDocListenerEx

class CDocLine;
class CDocLineMgr;

//! Modified管理
class CModifyManager : public TSingleton<CModifyManager>, public CDocListenerEx{
	friend class TSingleton<CModifyManager>;
	CModifyManager(){}

public:
	void OnAfterSave(const SSaveInfo& sSaveInfo) override;
};

//! 行に付加するModified情報
class CLineModified{
public:
	CLineModified() : m_nModifiedSeq(0) { }
	int GetSeq() const { return m_nModifiedSeq; }
	CLineModified& operator = (int seq)
	{
		m_nModifiedSeq = seq;
		return *this;
	}
private:
	int m_nModifiedSeq;
};

//! 行全体のModified情報アクセサ
class CModifyVisitor{
public:
	//状態
	bool IsLineModified(const CDocLine* pcDocLine, int nSaveSeq) const;
	int GetLineModifiedSeq(const CDocLine* pcDocLine) const;
	void SetLineModified(CDocLine* pcDocLine, int nModifiedSeq);

	//一括操作
	void ResetAllModifyFlag(CDocLineMgr* pcDocLineMgr, int nSeq);	// 行変更状態をすべてリセット
};
#endif /* SAKURA_CMODIFYMANAGER_12000875_531F_42DC_A6B0_231385193CB8_H_ */
