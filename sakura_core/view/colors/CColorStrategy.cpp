#include "stdafx.h"
#include "view/colors/CColorStrategy.h"
#include "CColor_Comment.h"
#include "CColor_Quote.h"
#include "CColor_RegexKeyword.h"
#include "CColor_Found.h"
#include "CColor_Url.h"
#include "CColor_Numeric.h"
#include "CColor_KeywordSet.h"
#include "CColor_Found.h"
#include "doc/CLayout.h"




bool _IsPosKeywordHead(const CStringRef& cStr, int nPos)
{
	return (nPos==0 || !IS_KEYWORD_CHAR(cStr.At(nPos-1)));
}


CLogicInt SColorStrategyInfo::GetPosInLayout() const
{
	return nPosInLogic - pDispPos->GetLayoutRef()->GetLogicOffset();
}

const CDocLine* SColorStrategyInfo::GetDocLine() const
{
	return pDispPos->GetLayoutRef()->GetDocLineRef();
}

const CLayout* SColorStrategyInfo::GetLayout() const
{
	return pDispPos->GetLayoutRef();
}

void SColorStrategyInfo::DoChangeColor(const CStringRef& cLineStr)
{
	CColorStrategyPool* pool = CColorStrategyPool::Instance();
	pool->SetCurrentView(this->pcView);
	CColorStrategy* pcFound = pool->GetFoundStrategy();

	//�����F�I��
	if(this->pStrategyFound){
		if(this->pStrategyFound->EndColor(cLineStr,this->GetPosInLogic())){
			this->pStrategyFound = NULL;
			this->ChangeColor(this->GetCurrentColor());
		}
	}

	//�����F�J�n
	if(!this->pStrategyFound){
		if(pcFound->BeginColor(cLineStr,this->GetPosInLogic())){
			this->pStrategyFound = pcFound;
			this->ChangeColor(this->GetCurrentColor());
		}
	}

	//�F�I��
	if(this->pStrategy){
		if(this->pStrategy->EndColor(cLineStr,this->GetPosInLogic())){
			this->pStrategy = NULL;
			this->ChangeColor(this->GetCurrentColor());
		}
	}

	//�F�J�n
	if(!this->pStrategy){
		for(int i=0;i<pool->GetStrategyCount();i++){
			if(pool->GetStrategy(i)->BeginColor(cLineStr,this->GetPosInLogic())){
				this->pStrategy = pool->GetStrategy(i);
				this->ChangeColor(this->GetCurrentColor());
				break;
			}
		}
	}
}

EColorIndexType SColorStrategyInfo::GetCurrentColor() const
{
	if(pStrategyFound){
		return pStrategyFound->GetStrategyColor();
	}
	else{
		return pStrategy->GetStrategyColorSafe();
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �v�[��                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CColorStrategyPool::CColorStrategyPool()
{
	m_pcView = CEditWnd::Instance()->m_pcEditViewArr[0];
	m_pcFoundStrategy = new CColor_Found;
//	m_vStrategies.push_back(new CColor_Found);				// �}�b�`������
	m_vStrategies.push_back(new CColor_RegexKeyword);		// ���K�\���L�[���[�h
	m_vStrategies.push_back(new CColor_LineComment);		// �s�R�����g
	m_vStrategies.push_back(new CColor_BlockComment(0));	// �u���b�N�R�����g
	m_vStrategies.push_back(new CColor_BlockComment(1));	// �u���b�N�R�����g2
	m_vStrategies.push_back(new CColor_SingleQuote);		// �V���O���N�H�[�e�[�V����������
	m_vStrategies.push_back(new CColor_DoubleQuote);		// �_�u���N�H�[�e�[�V����������
	m_vStrategies.push_back(new CColor_Url);				// URL
	m_vStrategies.push_back(new CColor_Numeric);			// ���p����
	m_vStrategies.push_back(new CColor_KeywordSet);			// �L�[���[�h�Z�b�g

	// CheckColorMODE �p
	m_pcLineComment = (CColor_LineComment*)GetStrategyByColor(COLORIDX_COMMENT);	// �s�R�����g
	m_pcBlockComment1 = (CColor_BlockComment*)GetStrategyByColor(COLORIDX_BLOCK1);	// �u���b�N�R�����g
	m_pcBlockComment2 = (CColor_BlockComment*)GetStrategyByColor(COLORIDX_BLOCK2);	// �u���b�N�R�����g2
	m_pcSingleQuote = (CColor_SingleQuote*)GetStrategyByColor(COLORIDX_SSTRING);	// �V���O���N�H�[�e�[�V����������
	m_pcDoubleQuote = (CColor_DoubleQuote*)GetStrategyByColor(COLORIDX_WSTRING);	// �_�u���N�H�[�e�[�V����������
}

CColorStrategyPool::~CColorStrategyPool()
{
	SAFE_DELETE(m_pcFoundStrategy);
	for(int i=0;i<(int)m_vStrategies.size();i++){
		delete m_vStrategies[i];
	}
	m_vStrategies.clear();
}

CColorStrategy*	CColorStrategyPool::GetStrategyByColor(EColorIndexType eColor) const
{
	for(int i=0;i<(int)m_vStrategies.size();i++){
		if(m_vStrategies[i]->GetStrategyColor()==eColor){
			return m_vStrategies[i];
		}
	}
	return NULL;
}

// 2005.11.20 Moca�R�����g�̐F������ON/OFF�֌W�Ȃ��s���Ă����o�O���C��
bool CColorStrategyPool::CheckColorMODE(
	CColorStrategy**	ppcColorStrategy,	//!< [in/out]
	int					nPos,
	const CStringRef&	cLineStr
)
{
	//�F�I��
	if(*ppcColorStrategy){
		if((*ppcColorStrategy)->EndColor(cLineStr,nPos)){
			*ppcColorStrategy = NULL;
			return true;
		}
	}

	//�F�J�n
	if(!*ppcColorStrategy){
		// CheckColorMODE �̓��C�A�E�g�����S�̂̃{�g���l�b�N�ɂȂ邭�炢�p�ɂɌĂяo�����
		// ��{�N���X����̓��I���z�֐��Ăяo�����g�p����Ɩ����ł��Ȃ��قǂ̃I�[�o�w�b�h�ɂȂ�͗l
		// �����̓G���K���g���������\�D��ŌX�̔h���N���X���� BeginColor() ���Ăяo��
		if(m_pcLineComment->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcLineComment; return false; }
		if(m_pcBlockComment1->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcBlockComment1; return false; }
		if(m_pcBlockComment2->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcBlockComment2; return false; }
		if(m_pcSingleQuote->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcSingleQuote; return false; }
		if(m_pcDoubleQuote->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcDoubleQuote; return false; }
	}

	return false;
}



/*!
  ini�̐F�ݒ��ԍ��łȂ�������ŏ����o���B(added by Stonee, 2001/01/12, 2001/01/15)
  �z��̏��Ԃ͋��L���������̃f�[�^�̏��Ԃƈ�v���Ă���B

  @note ���l�ɂ������I�Ή���global.h�ōs���Ă���̂ŎQ�Ƃ̂��ƁB(Mar. 7, 2001 jepro)
  CShareData����global�Ɉړ�
*/
const SColorAttributeData g_ColorAttributeArr[] =
{
	{_T("TXT"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_EFFECTS},
	{_T("RUL"), COLOR_ATTRIB_NO_EFFECTS},
	{_T("CAR"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// �L�����b�g		// 2006.12.07 ryoji
	{_T("IME"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// IME�L�����b�g	// 2006.12.07 ryoji
	{_T("UND"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{_T("CVL"), COLOR_ATTRIB_NO_BACK | ( COLOR_ATTRIB_NO_EFFECTS & ~COLOR_ATTRIB_NO_BOLD )}, // 2007.09.09 Moca �J�[�\���ʒu�c��
	{_T("LNO"), 0},
	{_T("MOD"), 0},
	{_T("TAB"), 0},
	{_T("SPC"), 0},	//2002.04.28 Add By KK
	{_T("ZEN"), 0},
	{_T("CTL"), 0},
	{_T("EOL"), 0},
	{_T("RAP"), 0},
	{_T("VER"), 0},  // 2005.11.08 Moca �w�茅�c��
	{_T("EOF"), 0},
	{_T("NUM"), 0},	//@@@ 2001.02.17 by MIK ���p���l�̋���
	{_T("FND"), 0},
	{_T("KW1"), 0},
	{_T("KW2"), 0},
	{_T("KW3"), 0},	//@@@ 2003.01.13 by MIK �����L�[���[�h3-10
	{_T("KW4"), 0},
	{_T("KW5"), 0},
	{_T("KW6"), 0},
	{_T("KW7"), 0},
	{_T("KW8"), 0},
	{_T("KW9"), 0},
	{_T("KWA"), 0},
	{_T("CMT"), 0},
	{_T("SQT"), 0},
	{_T("WQT"), 0},
	{_T("URL"), 0},
	{_T("RK1"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK2"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK3"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK4"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK5"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK6"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK7"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK8"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RK9"), 0},	//@@@ 2001.11.17 add MIK
	{_T("RKA"), 0},	//@@@ 2001.11.17 add MIK
	{_T("DFA"), 0},	//DIFF�ǉ�	//@@@ 2002.06.01 MIK
	{_T("DFC"), 0},	//DIFF�ύX	//@@@ 2002.06.01 MIK
	{_T("DFD"), 0},	//DIFF�폜	//@@@ 2002.06.01 MIK
	{_T("BRC"), 0},	//�Ί���	// 02/09/18 ai Add
	{_T("MRK"), 0},	//�u�b�N�}�[�N	// 02/10/16 ai Add
	{_T("LAST"), 0}	// Not Used
};



/*
 * �J���[������C���f�b�N�X�ԍ��ɕϊ�����
 */
SAKURA_CORE_API int GetColorIndexByName( const TCHAR *name )
{
	int	i;
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		if( _tcscmp( name, g_ColorAttributeArr[i].szName ) == 0 ) return i;
	}
	return -1;
}

/*
 * �C���f�b�N�X�ԍ�����J���[���ɕϊ�����
 */
SAKURA_CORE_API const TCHAR* GetColorNameByIndex( int index )
{
	return g_ColorAttributeArr[index].szName;
}
