
#pragma once

// �v��s��`
// #include "view/CEditView.h"

bool _IsPosKeywordHead(const CStringRef& cStr, int nPos);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �F�萔                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Stonee ���F 2000/01/12
// ������ύX�����Ƃ��́Aglobal.cpp ��g_ColorAttributeArr�̒�`���ύX���ĉ������B
//	From Here Sept. 18, 2000 JEPRO ���Ԃ�啝�ɓ���ւ���
//	2007.09.09 Moca  ���Ԃ̒�`�͂��C���ɕύX
SAKURA_CORE_API enum EColorIndexType {
	COLORIDX_TEXT = 0,		// �e�L�X�g
	COLORIDX_RULER,			// ���[���[
	COLORIDX_CARET,			// �L�����b�g	// 2006.12.07 ryoji
	COLORIDX_CARET_IME,		// IME�L�����b�g // 2006.12.07 ryoji
	COLORIDX_UNDERLINE,		// �J�[�\���s�A���_�[���C��
	COLORIDX_CURSORVLINE,	// �J�[�\���ʒu�c�� // 2006.05.13 Moca
	COLORIDX_GYOU,			// �s�ԍ�
	COLORIDX_GYOU_MOD,		// �s�ԍ�(�ύX�s)
	COLORIDX_TAB,			// TAB�L��
	COLORIDX_SPACE,			// ���p�� //2002.04.28 Add by KK �ȍ~�S��+1
	COLORIDX_ZENSPACE,		// ���{���
	COLORIDX_CTRLCODE,		// �R���g���[���R�[�h
	COLORIDX_EOL,			// ���s�L��
	COLORIDX_WRAP,			// �܂�Ԃ��L��
	COLORIDX_VERTLINE,		// �w�茅�c��	// 2005.11.08 Moca
	COLORIDX_EOF,			// EOF�L��
	COLORIDX_DIGIT,			// ���p���l	 //@@@ 2001.02.17 by MIK //�F�ݒ�Ver.3���烆�[�U�t�@�C���ɑ΂��Ă͕�����ŏ������Ă���̂Ń��i���o�����O���Ă��悢. Mar. 7, 2001 JEPRO noted
	COLORIDX_BRACKET_PAIR,	// �Ί���	  // 02/09/18 ai Add
	COLORIDX_SELECT,		// �I��͈�
	COLORIDX_SEARCH,		// ����������
	COLORIDX_SEARCH2,		// ����������2
	COLORIDX_SEARCH3,		// ����������3
	COLORIDX_SEARCH4,		// ����������4
	COLORIDX_SEARCH5,		// ����������5
	COLORIDX_COMMENT,		// �s�R�����g						//Dec. 4, 2000 shifted by MIK
	COLORIDX_SSTRING,		// �V���O���N�H�[�e�[�V����������	//Dec. 4, 2000 shifted by MIK
	COLORIDX_WSTRING,		// �_�u���N�H�[�e�[�V����������		//Dec. 4, 2000 shifted by MIK
	COLORIDX_URL,			// URL								//Dec. 4, 2000 shifted by MIK
	COLORIDX_KEYWORD1,		// �����L�[���[�h1 // 2002/03/13 novice
	COLORIDX_KEYWORD2,		// �����L�[���[�h2 // 2002/03/13 novice  //MIK ADDED
	COLORIDX_KEYWORD3,		// �����L�[���[�h3 // 2005.01.13 MIK 3-10 added
	COLORIDX_KEYWORD4,		// �����L�[���[�h4
	COLORIDX_KEYWORD5,		// �����L�[���[�h5
	COLORIDX_KEYWORD6,		// �����L�[���[�h6
	COLORIDX_KEYWORD7,		// �����L�[���[�h7
	COLORIDX_KEYWORD8,		// �����L�[���[�h8
	COLORIDX_KEYWORD9,		// �����L�[���[�h9
	COLORIDX_KEYWORD10,		// �����L�[���[�h10
	COLORIDX_REGEX1,		// ���K�\���L�[���[�h1  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX2,		// ���K�\���L�[���[�h2  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX3,		// ���K�\���L�[���[�h3  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX4,		// ���K�\���L�[���[�h4  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX5,		// ���K�\���L�[���[�h5  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX6,		// ���K�\���L�[���[�h6  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX7,		// ���K�\���L�[���[�h7  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX8,		// ���K�\���L�[���[�h8  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX9,		// ���K�\���L�[���[�h9  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX10,		// ���K�\���L�[���[�h10	//@@@ 2001.11.17 add MIK
	COLORIDX_DIFF_APPEND,	// DIFF�ǉ�  //@@@ 2002.06.01 MIK
	COLORIDX_DIFF_CHANGE,	// DIFF�ǉ�  //@@@ 2002.06.01 MIK
	COLORIDX_DIFF_DELETE,	// DIFF�ǉ�  //@@@ 2002.06.01 MIK
	COLORIDX_MARK,			// �u�b�N�}�[�N  // 02/10/16 ai Add

	//�J���[�̍Ō�
	COLORIDX_LAST,

	//�J���[�\������p
	COLORIDX_BLOCK1,		// �u���b�N�R�����g1(�����F�Ɣw�i�F�͍s�R�����g�Ɠ���)
	COLORIDX_BLOCK2,		// �u���b�N�R�����g2(�����F�Ɣw�i�F�͍s�R�����g�Ɠ���)

	//1000- : �J���[�\������p(���K�\���L�[���[�h)
	COLORIDX_REGEX_FIRST	= 1000,
	COLORIDX_REGEX_LAST		= COLORIDX_REGEX_FIRST + COLORIDX_LAST - 1,

	// -- -- �ʖ� -- -- //
	COLORIDX_DEFAULT		= COLORIDX_TEXT,
	COLORIDX_SEARCHTAIL		= COLORIDX_SEARCH5,
};
//	To Here Sept. 18, 2000

//���K�\���L�[���[�h��EColorIndexType�l�����֐�
inline EColorIndexType ToColorIndexType_RegularExpression(const int& nRegexColorIndex)
{
	return (EColorIndexType)(COLORIDX_REGEX_FIRST + nRegexColorIndex);
}

//���K�\���L�[���[�h��EColorIndexType�l���ǂ���
inline bool IsRegularExpression(const EColorIndexType& eColorIndex)
{
	return (eColorIndex >= COLORIDX_REGEX_FIRST && eColorIndex <= COLORIDX_REGEX_LAST);
}

//���K�\���L�[���[�h��EColorIndexType�l��F�ԍ��ɖ߂��֐�
inline int ToColorInfoArrIndex_RegularExpression(const EColorIndexType& eRegexColorIndex)
{
	return eRegexColorIndex - COLORIDX_REGEX_FIRST;
}

//EColorIndexType�l��F�ԍ��ɕϊ�����֐�
inline int ToColorInfoArrIndex(const EColorIndexType& eColorIndex)
{
	if(eColorIndex>=0 && eColorIndex<COLORIDX_LAST)
		return eColorIndex;
	else if(eColorIndex==COLORIDX_BLOCK1 || eColorIndex==COLORIDX_BLOCK2)
		return COLORIDX_COMMENT;
	else if( IsRegularExpression(eColorIndex) )
		return ToColorInfoArrIndex_RegularExpression(eColorIndex);

	return -1;
}

// �J���[�������C���f�b�N�X�ԍ��̕ϊ�	//@@@ 2002.04.30
SAKURA_CORE_API int GetColorIndexByName( const TCHAR *name );
SAKURA_CORE_API const TCHAR* GetColorNameByIndex( int index );


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct DispPos;
class CColorStrategy;
#include "view/DispPos.h"
#include <memory> //auto_ptr

class CColor_Found;
class CColor_Select;

struct SColorStrategyInfo{
	SColorStrategyInfo() : sDispPosBegin(0,0), pStrategy(NULL), pStrategyFound(NULL), pStrategySelect(NULL) {}

	//�Q��
	CEditView*	pcView;
	CGraphics	gr;	//(SColorInfo�ł͖��g�p)

	//�X�L�����ʒu
	LPCWSTR			pLineOfLogic;
	CLogicInt		nPosInLogic;
	CLayoutInt		nLayoutLineNum;

	//�`��ʒu
	DispPos*		pDispPos;
	DispPos			sDispPosBegin;

	//�F�ς�
	CColorStrategy*		pStrategy;
	CColor_Found*		pStrategyFound;
	CColor_Select*		pStrategySelect;

	//! �F�̐؂�ւ�
	void ChangeColor(EColorIndexType eNewColor)
	{
		this->pcView->SetCurrentColor( this->gr, eNewColor );
	}
	void ChangeColor2(EColorIndexType eNewColor, EColorIndexType eNewColor2)
	{
		this->pcView->SetCurrentColor2(this->gr, eNewColor, eNewColor2);
	}

	void DoChangeColor(const CStringRef& cLineStr);
	EColorIndexType GetCurrentColor() const;
	EColorIndexType GetCurrentColor2() const;

	//! ���݂̃X�L�����ʒu
	CLogicInt GetPosInLogic() const
	{
		return nPosInLogic;
	}
	CLogicInt GetPosInLayout() const;
	const CDocLine* GetDocLine() const;
	const CLayout* GetLayout() const;
};

class CColorStrategy{
public:
	virtual ~CColorStrategy(){}
	//! �F��`
	virtual EColorIndexType GetStrategyColor() const = 0;
	//! �F�؂�ւ��J�n�����o������A���̒��O�܂ł̕`����s���A����ɐF�ݒ���s���B
	virtual void InitStrategyStatus() = 0;
	virtual bool BeginColor(const CStringRef& cStr, int nPos){ return false; }
	virtual bool EndColor(const CStringRef& cStr, int nPos){ return true; }
	//�C�x���g
	virtual void OnStartScanLogic(){}

	//#######���b�v
	EColorIndexType GetStrategyColorSafe() const{ if(this)return GetStrategyColor(); else return COLORIDX_TEXT; }
};

#include "util/design_template.h"
#include <vector>
class CColor_LineComment;
class CColor_BlockComment;
class CColor_BlockComment;
class CColor_SingleQuote;
class CColor_DoubleQuote;

class CColorStrategyPool : public TSingleton<CColorStrategyPool>{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CColorStrategyPool();
	virtual ~CColorStrategyPool();

	//�擾
	CColorStrategy*	GetStrategy(int nIndex) const{ return m_vStrategies[nIndex]; }
	int				GetStrategyCount() const{ return (int)m_vStrategies.size(); }
	CColorStrategy*	GetStrategyByColor(EColorIndexType eColor) const;

	//����擾
	CColor_Found*   GetFoundStrategy() const{ return m_pcFoundStrategy; }
	CColor_Select*  GetSelectStrategy() const{ return m_pcSelectStrategy; }

	//�C�x���g
	void NotifyOnStartScanLogic();

	/*
	|| �F����
	*/
	//@@@ 2002.09.22 YAZAKI
	// 2005.11.21 Moca ���p���̐F���������������珜��
	bool CheckColorMODE( CColorStrategy** ppcColorStrategy, int nPos, const CStringRef& cLineStr );

	//�r���[�̐ݒ�E�擾
	CEditView* GetCurrentView(void) const{ return m_pcView; }
	void SetCurrentView(CEditView* pcView) { m_pcView = pcView; }

private:
	std::vector<CColorStrategy*>	m_vStrategies;
	CColor_Found*					m_pcFoundStrategy;
	CColor_Select*					m_pcSelectStrategy;

	CColor_LineComment*				m_pcLineComment;
	CColor_BlockComment*			m_pcBlockComment1;
	CColor_BlockComment*			m_pcBlockComment2;
	CColor_SingleQuote*				m_pcSingleQuote;
	CColor_DoubleQuote*				m_pcDoubleQuote;

	CEditView*						m_pcView;
};
