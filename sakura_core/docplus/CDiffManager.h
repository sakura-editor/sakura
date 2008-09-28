//@@@ 2002.05.25 MIK
//2008.02.23 kobake �吮��

#pragma once

#include "util/design_template.h" //TSingleton
class CDocLine;
class CDocLineMgr;
enum EColorIndexType;
class CTypeSupport;

//! DIFF���萔
enum EDiffMark{
	MARK_DIFF_NONE		= 0,	//���ύX
	MARK_DIFF_APPEND	= 1,	//�ǉ�
	MARK_DIFF_CHANGE	= 2,	//�ύX
	MARK_DIFF_DELETE	= 3,	//�폜
	MARK_DIFF_DEL_EX	= 4,	//�폜(EOF�ȍ~)
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
	operator EDiffMark() const{ return (EDiffMark)m_nDiffed; }
	CLineDiffed& operator = (EDiffMark e){ m_nDiffed = e; return *this; }
private:
	uchar_t m_nDiffed;
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

