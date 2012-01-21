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

//! Modified�Ǘ�
class CModifyManager : public TSingleton<CModifyManager>, public CDocListenerEx{
public:
	void OnAfterSave(const SSaveInfo& sSaveInfo);

};

//! �s�ɕt������Modified���
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

//! �s�S�̂�Modified���A�N�Z�T
class CModifyVisitor{
public:
	//���
	bool IsLineModified(const CDocLine* pcDocLine) const;
	void SetLineModified(CDocLine* pcDocLine, bool bModified);

	//�ꊇ����
	void ResetAllModifyFlag(CDocLineMgr* pcDocLineMgr);	// �s�ύX��Ԃ����ׂă��Z�b�g
};

#endif /* SAKURA_CMODIFYMANAGER_5129DDF8_A336_4B65_914B_22E626B7B520_H_ */
/*[EOF]*/
