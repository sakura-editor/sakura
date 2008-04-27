#pragma once

#include "util/design_template.h" //TSingleton

class CDocLine;
class CDocLineMgr;

//! Modified管理
class CModifyManager : public TSingleton<CModifyManager>, public CDocListenerEx{
public:
	void OnAfterSave(const SSaveInfo& sSaveInfo);

};

//! 行に付加するModified情報
class CLineModified{
public:
	CLineModified() : m_bModified(true) { }
	operator bool() const{ return m_bModified; }
	CLineModified& operator = (bool b)
	{
		m_bModified = b;
		return *this;
	}
private:
	bool m_bModified;
};

//! 行全体のModified情報アクセサ
class CModifyVisitor{
public:
	//状態
	bool IsLineModified(const CDocLine* pcDocLine) const;
	void SetLineModified(CDocLine* pcDocLine, bool bModified);

	//一括操作
	void ResetAllModifyFlag(CDocLineMgr* pcDocLineMgr);	// 行変更状態をすべてリセット
};
