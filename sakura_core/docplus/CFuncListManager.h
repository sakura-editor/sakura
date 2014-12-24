/*
	Copyright (C) 2008, kobake
	Copyright (C) 2014, Moca

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
#ifndef SAKURA_CFUNCLISTMANAGER_H_
#define SAKURA_CFUNCLISTMANAGER_H_

class CDocLine;
class CDocLineMgr;

//! �s�ɕt������Modified���
class CLineFuncList{
public:
	CLineFuncList() : m_bFuncList(false) { }
	bool GetFuncListMark() const { return m_bFuncList; }
	CLineFuncList& operator = (bool bSet)
	{
		m_bFuncList = bSet;
		return *this;
	}
private:
	bool m_bFuncList;
};

//! �s�S�̂�FuncList���A�N�Z�T
class CFuncListManager{
public:
	//���
	bool IsLineFuncList(const CDocLine* pcDocLine, bool bFlag) const;
	bool GetLineFuncList(const CDocLine* pcDocLine) const;
	void SetLineFuncList(CDocLine* pcDocLine, bool bFlag);
	bool SearchFuncListMark(const CDocLineMgr*, CLogicInt, ESearchDirection, CLogicInt* ) const;					//!< �֐����X�g�}�[�N����

	//�ꊇ����
	void ResetAllFucListMark(CDocLineMgr* pcDocLineMgr, bool bFlag);	// �֐����X�g�}�[�N�����ׂă��Z�b�g
};

#endif /* SAKURA_CFUNCLISTMANAGER_H_ */
/*[EOF]*/
