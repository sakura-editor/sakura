#pragma once

class CDocLine;
class CDocLineMgr;
class CBregexp;

#include "global.h" // ESearchDirection, SSearchOption


//! �s�ɕt������u�b�N�}�[�N���
class CLineBookmarked{
public:
	CLineBookmarked() : m_bBookmarked(false) { }
	operator bool() const{ return m_bBookmarked; }
	CLineBookmarked& operator=(bool b){ m_bBookmarked = b; return *this; }
private:
	bool m_bBookmarked;
};

//! �s�̃u�b�N�}�[�N���̎擾
class CBookmarkGetter{
public:
	CBookmarkGetter(const CDocLine* pcDocLine) : m_pcDocLine(pcDocLine) { }
	bool IsBookmarked() const;
private:
	const CDocLine* m_pcDocLine;
};

//! �s�̃u�b�N�}�[�N���̎擾�E�ݒ�
class CBookmarkSetter : public CBookmarkGetter{
public:
	CBookmarkSetter(CDocLine* pcDocLine) : CBookmarkGetter(pcDocLine), m_pcDocLine(pcDocLine) { }
	void SetBookmark(bool bFlag);
private:
	CDocLine* m_pcDocLine;
};

//! �s�S�̂̃u�b�N�}�[�N���̊Ǘ�
class CBookmarkManager{
public:
	CBookmarkManager(CDocLineMgr* pcDocLineMgr) : m_pcDocLineMgr(pcDocLineMgr) { }

	void ResetAllBookMark();															//!< �u�b�N�}�[�N�̑S����
	bool SearchBookMark( CLogicInt nLineNum, ESearchDirection , CLogicInt* pnLineNum );	//!< �u�b�N�}�[�N����
	void SetBookMarks( wchar_t* );														//!< �����s�ԍ��̃��X�g����܂Ƃ߂čs�}�[�N
	LPCWSTR GetBookMarks();																//!< �s�}�[�N����Ă镨���s�ԍ��̃��X�g�����
	void MarkSearchWord( const wchar_t* , const SSearchOption& , CBregexp* );			//!< ���������ɊY������s�Ƀu�b�N�}�[�N���Z�b�g����

private:
	CDocLineMgr* m_pcDocLineMgr;
};

