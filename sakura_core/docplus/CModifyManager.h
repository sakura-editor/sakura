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
#ifndef SAKURA_CMODIFYMANAGER_5129DDF8_A336_4B65_914B_22E626B7B520_H_
#define SAKURA_CMODIFYMANAGER_5129DDF8_A336_4B65_914B_22E626B7B520_H_

#include "util/design_template.h" //TSingleton
#include "doc/CDocListener.h" // CDocListenerEx

class CDocLine;
class CDocLineMgr;

//! Modified管理
class CModifyManager : public TSingleton<CModifyManager>, public CDocListenerEx{
	friend class TSingleton<CModifyManager>;
	CModifyManager(){}

public:
	void OnAfterSave(const SSaveInfo& sSaveInfo);

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

#endif /* SAKURA_CMODIFYMANAGER_5129DDF8_A336_4B65_914B_22E626B7B520_H_ */
/*[EOF]*/
