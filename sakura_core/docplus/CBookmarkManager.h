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
#ifndef SAKURA_CBOOKMARKMANAGER_0BD65312_87D5_4C0F_AA25_7F077D00E8DC_H_
#define SAKURA_CBOOKMARKMANAGER_0BD65312_87D5_4C0F_AA25_7F077D00E8DC_H_

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

#endif /* SAKURA_CBOOKMARKMANAGER_0BD65312_87D5_4C0F_AA25_7F077D00E8DC_H_ */
/*[EOF]*/
