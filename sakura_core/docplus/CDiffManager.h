//@@@ 2002.05.25 MIK
//2008.02.23 kobake �吮��
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
#ifndef SAKURA_CDIFFMANAGER_A4D99FE4_A4AB_468A_96FC_00858B04DEA1_H_
#define SAKURA_CDIFFMANAGER_A4D99FE4_A4AB_468A_96FC_00858B04DEA1_H_

#include "view/colors/EColorIndexType.h"
#include "util/design_template.h" //TSingleton

class CDocLine;
class CDocLineMgr;
class CTypeSupport;
class CGraphics;

//! DIFF���萔
enum EDiffMark{
	MARK_DIFF_NONE		= 0,	//!< ���ύX
	MARK_DIFF_APPEND	= 1,	//!< �ǉ�
	MARK_DIFF_CHANGE	= 2,	//!< �ύX
	MARK_DIFF_DELETE	= 3,	//!< �폜
	MARK_DIFF_DEL_EX	= 4,	//!< �폜(EOF�ȍ~)
};

//! DIFF�����̊Ǘ�
class CDiffManager : public TSingleton<CDiffManager>{
public:
	void SetDiffUse(bool b){ m_bIsDiffUse = b; }
	bool IsDiffUse() const{ return m_bIsDiffUse; }		//!< DIFF�g�p��

private:
	bool	m_bIsDiffUse;		//!< DIFF�����\�����{�� @@@ 2002.05.25 MIK
};

//! �s�ɕt������DIFF���
class CLineDiffed{
public:
	CLineDiffed() : m_nDiffed(MARK_DIFF_NONE) { }
	operator EDiffMark() const{ return m_nDiffed; }
	CLineDiffed& operator = (EDiffMark e){ m_nDiffed = e; return *this; }
private:
	EDiffMark m_nDiffed;
};

//! �s��DIFF���擾
class CDiffLineGetter{
public:
	CDiffLineGetter(const CDocLine* pcDocLine) : m_pcDocLine(pcDocLine) { }
	EDiffMark GetLineDiffMark() const;
	bool GetDiffColor(EColorIndexType* nColor) const;
	bool DrawDiffMark(CGraphics& gr, int y, int nLineHeight, CTypeSupport& cColorType) const;
private:
	const CDocLine* m_pcDocLine;
};

//! �s��DIFF���ݒ�
class CDiffLineSetter{
public:
	CDiffLineSetter(CDocLine* pcDocLine) : m_pcDocLine(pcDocLine) { }
	void SetLineDiffMark(EDiffMark mark);
private:
	CDocLine* m_pcDocLine;
};

//! �s�S�̂�DIFF���Ǘ�
class CDiffLineMgr{
public:
	CDiffLineMgr(CDocLineMgr* pcDocLineMgr) : m_pcDocLineMgr(pcDocLineMgr) { }
	void ResetAllDiffMark();															//!< �����\���̑S����
	bool SearchDiffMark( CLogicInt , ESearchDirection , CLogicInt* );					//!< ��������
	void SetDiffMarkRange( EDiffMark nMode, CLogicInt nStartLine, CLogicInt nEndLine );	//!< �����͈͂̓o�^
private:
	CDocLineMgr* m_pcDocLineMgr;
};

#endif /* SAKURA_CDIFFMANAGER_A4D99FE4_A4AB_468A_96FC_00858B04DEA1_H_ */
/*[EOF]*/
