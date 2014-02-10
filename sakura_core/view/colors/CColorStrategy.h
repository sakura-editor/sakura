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
#ifndef SAKURA_CCOLORSTRATEGY_BC7B5956_A0AF_4C9C_9C0E_07FE658028AC9_H_
#define SAKURA_CCOLORSTRATEGY_BC7B5956_A0AF_4C9C_9C0E_07FE658028AC9_H_

// �v��s��`
// #include "view/CEditView.h"
#include "EColorIndexType.h"
#include "uiparts/CGraphics.h"

class	CEditView;

bool _IsPosKeywordHead(const CStringRef& cStr, int nPos);


//! ���K�\���L�[���[�h��EColorIndexType�l�����֐�
inline EColorIndexType ToColorIndexType_RegularExpression(const int nRegexColorIndex)
{
	return (EColorIndexType)(COLORIDX_REGEX_FIRST + nRegexColorIndex);
}

//! ���K�\���L�[���[�h��EColorIndexType�l��F�ԍ��ɖ߂��֐�
inline int ToColorInfoArrIndex_RegularExpression(const EColorIndexType eRegexColorIndex)
{
	return eRegexColorIndex - COLORIDX_REGEX_FIRST;
}

/*! �F�萔��F�ԍ��ɕϊ�����֐�

	@date 2013.05.08 novice �͈͊O�̂Ƃ��̓e�L�X�g��I������
*/
inline int ToColorInfoArrIndex(const EColorIndexType eColorIndex)
{
	if( eColorIndex>=0 && eColorIndex<COLORIDX_LAST )
		return eColorIndex;
	else if( eColorIndex & COLORIDX_BLOCK_BIT )
		return COLORIDX_COMMENT;
	else if( eColorIndex & COLORIDX_REGEX_BIT )
		return ToColorInfoArrIndex_RegularExpression( eColorIndex );

	assert(0); // �����ɂ͗��Ȃ�
	return COLORIDX_TEXT;
}

// �J���[�������C���f�b�N�X�ԍ��̕ϊ�	//@@@ 2002.04.30
int GetColorIndexByName( const TCHAR *name );
const TCHAR* GetColorNameByIndex( int index );


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct DispPos;
class CColorStrategy;
#include "view/DispPos.h"

class CColor_Found;
class CColor_Select;

//! �F�ݒ�
struct CColor3Setting {
	EColorIndexType eColorIndex;    //!< �I�����܂ތ��݂̐F
	EColorIndexType eColorIndex2;   //!< �I���ȊO�̌��݂̐F
	EColorIndexType eColorIndexBg;  //!< �w�i�F
};

struct SColorStrategyInfo{
	SColorStrategyInfo() : m_sDispPosBegin(0,0), m_pStrategy(NULL), m_pStrategyFound(NULL), m_pStrategySelect(NULL), m_colorIdxBackLine(COLORIDX_TEXT) {
		m_cIndex.eColorIndex = COLORIDX_TEXT;
		m_cIndex.eColorIndex2 = COLORIDX_TEXT;
		m_cIndex.eColorIndexBg = COLORIDX_TEXT;
	}

	//�Q��
	CEditView*	m_pcView;
	CGraphics	m_gr;	//(SColorInfo�ł͖��g�p)

	//�X�L�����ʒu
	LPCWSTR			m_pLineOfLogic;
	CLogicInt		m_nPosInLogic;

	//�`��ʒu
	DispPos*		m_pDispPos;
	DispPos			m_sDispPosBegin;

	//�F�ς�
	CColorStrategy*		m_pStrategy;
	CColor_Found*		m_pStrategyFound;
	CColor_Select*		m_pStrategySelect;
	EColorIndexType		m_colorIdxBackLine;
	CColor3Setting		m_cIndex;

	//! �F�̐؂�ւ�
	bool CheckChangeColor(const CStringRef& cLineStr);
	void DoChangeColor(CColor3Setting *pcColor);
	EColorIndexType GetCurrentColor() const { return m_cIndex.eColorIndex; }
	EColorIndexType GetCurrentColor2() const { return m_cIndex.eColorIndex2; }
	EColorIndexType GetCurrentColorBg() const{ return m_cIndex.eColorIndexBg; }

	//! ���݂̃X�L�����ʒu
	CLogicInt GetPosInLogic() const
	{
		return m_nPosInLogic;
	}
	const CDocLine* GetDocLine() const
	{
		return m_pDispPos->GetLayoutRef()->GetDocLineRef();
	}
	const CLayout* GetLayout() const
	{
		return m_pDispPos->GetLayoutRef();
	}
};

class CColorStrategy{
public:
	virtual ~CColorStrategy(){}
	//! �F��`
	virtual EColorIndexType GetStrategyColor() const = 0;
	virtual CLayoutColorInfo* GetStrategyColorInfo() const{
		return NULL;
	}
	//! �F�؂�ւ��J�n�����o������A���̒��O�܂ł̕`����s���A����ɐF�ݒ���s���B
	virtual void InitStrategyStatus() = 0;
	virtual void SetStrategyColorInfo(const CLayoutColorInfo* = NULL){};
	virtual bool BeginColor(const CStringRef& cStr, int nPos){ return false; }
	virtual bool EndColor(const CStringRef& cStr, int nPos){ return true; }
	virtual bool Disp() const = 0;
	//�C�x���g
	virtual void OnStartScanLogic(){}

	//! �ݒ�X�V
	virtual void Update(void)
	{
		const CEditDoc* pCEditDoc = CEditDoc::GetInstance(0);
		m_pTypeData = &pCEditDoc->m_cDocType.GetDocumentAttribute();
	}

	//#######���b�v
	EColorIndexType GetStrategyColorSafe() const{ if(this)return GetStrategyColor(); else return COLORIDX_TEXT; }
	CLayoutColorInfo* GetStrategyColorInfoSafe() const{
		if(this){
			return GetStrategyColorInfo();
		}
		return NULL;
	}

protected:
	const STypeConfig* m_pTypeData;
};

#include "util/design_template.h"
#include <vector>
class CColor_LineComment;
class CColor_BlockComment;
class CColor_BlockComment;
class CColor_SingleQuote;
class CColor_DoubleQuote;
class CColor_Heredoc;

class CColorStrategyPool : public TSingleton<CColorStrategyPool>{
	friend class TSingleton<CColorStrategyPool>;
	CColorStrategyPool();
	virtual ~CColorStrategyPool();

public:

	//�擾
	CColorStrategy*	GetStrategy(int nIndex) const{ return m_vStrategiesDisp[nIndex]; }
	int				GetStrategyCount() const{ return (int)m_vStrategiesDisp.size(); }
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
	void CheckColorMODE( CColorStrategy** ppcColorStrategy, int nPos, const CStringRef& cLineStr );
	bool IsSkipBeforeLayout();	// ���C�A�E�g���s������`�F�b�N���Ȃ��Ă���������

	//�ݒ�ύX
	void OnChangeSetting(void);

	//�r���[�̐ݒ�E�擾
	CEditView* GetCurrentView(void) const{ return m_pcView; }
	void SetCurrentView(CEditView* pcView) { m_pcView = pcView; }

private:
	std::vector<CColorStrategy*>	m_vStrategies;
	std::vector<CColorStrategy*>	m_vStrategiesDisp;	//!< �F�����\���Ώ�
	CColor_Found*					m_pcFoundStrategy;
	CColor_Select*					m_pcSelectStrategy;

	CColor_LineComment*				m_pcLineComment;
	CColor_BlockComment*			m_pcBlockComment1;
	CColor_BlockComment*			m_pcBlockComment2;
	CColor_SingleQuote*				m_pcSingleQuote;
	CColor_DoubleQuote*				m_pcDoubleQuote;
	CColor_Heredoc*					m_pcHeredoc;

	CEditView*						m_pcView;

	bool	m_bSkipBeforeLayoutGeneral;
	bool	m_bSkipBeforeLayoutFound;
};

#endif /* SAKURA_CCOLORSTRATEGY_BC7B5956_A0AF_4C9C_9C0E_07FE658028AC9_H_ */
/*[EOF]*/
