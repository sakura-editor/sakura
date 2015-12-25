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

#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "CColor_Comment.h"
#include "CColor_Quote.h"
#include "CColor_RegexKeyword.h"
#include "CColor_Found.h"
#include "CColor_Url.h"
#include "CColor_Numeric.h"
#include "CColor_KeywordSet.h"
#include "CColor_Found.h"
#include "CColor_Heredoc.h"
#include "doc/layout/CLayout.h"
#include "window/CEditWnd.h"
#include "types/CTypeSupport.h"




bool _IsPosKeywordHead(const CStringRef& cStr, int nPos)
{
	return (nPos==0 || !IS_KEYWORD_CHAR(cStr.At(nPos-1)));
}

/*! �F�̐؂�ւ�����
	@retval true �F�̕ύX����
	@retval false �F�̕ύX�Ȃ�
*/
bool SColorStrategyInfo::CheckChangeColor(const CStringRef& cLineStr)
{
	CColorStrategyPool* pool = CColorStrategyPool::getInstance();
	pool->SetCurrentView(m_pcView);
	CColor_Found*  pcFound  = pool->GetFoundStrategy();
	CColor_Select* pcSelect = pool->GetSelectStrategy();
	bool bChange = false;

	//�I��͈͐F�I��
	if(m_pStrategySelect){
		if(m_pStrategySelect->EndColor(cLineStr,this->GetPosInLogic())){
			m_pStrategySelect = NULL;
			bChange = true;
		}
	}
	//�I��͈͐F�J�n
	if(!m_pStrategySelect){
		if(pcSelect->BeginColorEx(cLineStr,this->GetPosInLogic(), m_pDispPos->GetLayoutLineRef(), this->GetLayout())){
			m_pStrategySelect = pcSelect;
			bChange = true;
		}
	}

	//�����F�I��
	if(m_pStrategyFound){
		if(m_pStrategyFound->EndColor(cLineStr,this->GetPosInLogic())){
			m_pStrategyFound = NULL;
			bChange = true;
		}
	}

	//�����F�J�n
	if(!m_pStrategyFound){
		if(pcFound->BeginColor(cLineStr,this->GetPosInLogic())){
			m_pStrategyFound = pcFound;
			bChange = true;
		}
	}

	//�F�I��
	if(m_pStrategy){
		if(m_pStrategy->EndColor(cLineStr,this->GetPosInLogic())){
			m_pStrategy = NULL;
			bChange = true;
		}
	}

	//�F�J�n
	if(!m_pStrategy){
		int size = pool->GetStrategyCount();
		for(int i = 0; i < size; i++ ){
			if(pool->GetStrategy(i)->BeginColor(cLineStr,this->GetPosInLogic())){
				m_pStrategy = pool->GetStrategy(i);
				bChange = true;
				break;
			}
		}
	}

	//�J�[�\���s�w�i�F
	CTypeSupport cCaretLineBg(m_pcView, COLORIDX_CARETLINEBG);
	if( cCaretLineBg.IsDisp() && !m_pcView->m_bMiniMap ){
		if(m_colorIdxBackLine==COLORIDX_CARETLINEBG){
			if( m_pDispPos->GetLayoutLineRef() != m_pcView->GetCaret().GetCaretLayoutPos().GetY2() ){
				m_colorIdxBackLine = COLORIDX_TEXT;
				bChange = true;
			}
		}else{
			if( m_pDispPos->GetLayoutLineRef() == m_pcView->GetCaret().GetCaretLayoutPos().GetY2() ){
				m_colorIdxBackLine = COLORIDX_CARETLINEBG;
				bChange = true;
			}
		}
	}
	//�����s�̔w�i�F
	CTypeSupport cEvenLineBg(m_pcView, COLORIDX_EVENLINEBG);
	if( cEvenLineBg.IsDisp() && !m_pcView->m_bMiniMap && m_colorIdxBackLine != COLORIDX_CARETLINEBG ){
		if( m_colorIdxBackLine == COLORIDX_EVENLINEBG ){
			if( m_pDispPos->GetLayoutLineRef() % 2 == 0 ){
				m_colorIdxBackLine = COLORIDX_TEXT;
				bChange = true;
			}
		}else{
			if( m_pDispPos->GetLayoutLineRef() % 2 == 1 ){
				m_colorIdxBackLine = COLORIDX_EVENLINEBG;
				bChange = true;
			}
		}
	}
	if( m_pcView->m_bMiniMap ){
		CTypeSupport cPageViewBg(m_pcView, COLORIDX_PAGEVIEW);
		if( cPageViewBg.IsDisp() ){
			CEditView& cActiveView = m_pcView->m_pcEditWnd->GetActiveView();
			CLayoutInt curLine = m_pDispPos->GetLayoutLineRef();
			if( m_colorIdxBackLine == COLORIDX_PAGEVIEW ){
				if( cActiveView.GetTextArea().GetViewTopLine() <= curLine && curLine < cActiveView.GetTextArea().GetBottomLine() ){
				}else{
					m_colorIdxBackLine = COLORIDX_TEXT;
					bChange = true;
				}
			}else if( m_colorIdxBackLine == COLORIDX_TEXT ){
				if( cActiveView.GetTextArea().GetViewTopLine() <= curLine && curLine < cActiveView.GetTextArea().GetBottomLine() ){
					m_colorIdxBackLine = COLORIDX_PAGEVIEW;
					bChange = true;
				}
			}
		}
	}

	return bChange;
}

/*! �F�̐؂�ւ�

	@date 2013.05.11 novice ���ۂ̕ύX�͌Ăяo�����ōs��
*/
void SColorStrategyInfo::DoChangeColor(CColor3Setting *pcColor)
{
	if(m_pStrategySelect){
		m_cIndex.eColorIndex = m_pStrategySelect->GetStrategyColor();
	}else if(m_pStrategyFound){
		m_cIndex.eColorIndex = m_pStrategyFound->GetStrategyColor();
	}else{
		m_cIndex.eColorIndex = m_pStrategy->GetStrategyColorSafe();
	}

	if(m_pStrategyFound){
		m_cIndex.eColorIndex2 = m_pStrategyFound->GetStrategyColor();
	}else{
		m_cIndex.eColorIndex2 = m_pStrategy->GetStrategyColorSafe();
	}

	m_cIndex.eColorIndexBg = m_colorIdxBackLine;

	*pcColor = m_cIndex;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �v�[��                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CColorStrategyPool::CColorStrategyPool()
{
	m_pcView = &(CEditWnd::getInstance()->GetView(0));
	m_pcSelectStrategy = new CColor_Select();
	m_pcFoundStrategy = new CColor_Found();
//	m_vStrategies.push_back(new CColor_Found);				// �}�b�`������
	m_vStrategies.push_back(new CColor_RegexKeyword);		// ���K�\���L�[���[�h
	m_vStrategies.push_back(new CColor_Heredoc);			// �q�A�h�L�������g
	m_vStrategies.push_back(new CColor_BlockComment(COLORIDX_BLOCK1));	// �u���b�N�R�����g
	m_vStrategies.push_back(new CColor_BlockComment(COLORIDX_BLOCK2));	// �u���b�N�R�����g2
	m_vStrategies.push_back(new CColor_LineComment);		// �s�R�����g
	m_vStrategies.push_back(new CColor_SingleQuote);		// �V���O���N�H�[�e�[�V����������
	m_vStrategies.push_back(new CColor_DoubleQuote);		// �_�u���N�H�[�e�[�V����������
	m_vStrategies.push_back(new CColor_Url);				// URL
	m_vStrategies.push_back(new CColor_Numeric);			// ���p����
	m_vStrategies.push_back(new CColor_KeywordSet);			// �L�[���[�h�Z�b�g

	// �ݒ�X�V
	OnChangeSetting();
}

CColorStrategyPool::~CColorStrategyPool()
{
	SAFE_DELETE(m_pcSelectStrategy);
	SAFE_DELETE(m_pcFoundStrategy);
	m_vStrategiesDisp.clear();
	int size = (int)m_vStrategies.size();
	for(int i = 0; i < size; i++ ){
		delete m_vStrategies[i];
	}
	m_vStrategies.clear();
}

CColorStrategy*	CColorStrategyPool::GetStrategyByColor(EColorIndexType eColor) const
{
	if( COLORIDX_SEARCH <= eColor && eColor <= COLORIDX_SEARCHTAIL ){
		return m_pcFoundStrategy;
	}
	int size = (int)m_vStrategiesDisp.size();
	for(int i = 0; i < size; i++ ){
		if(m_vStrategiesDisp[i]->GetStrategyColor()==eColor){
			return m_vStrategiesDisp[i];
		}
	}
	return NULL;
}

void CColorStrategyPool::NotifyOnStartScanLogic()
{
	m_pcSelectStrategy->OnStartScanLogic();
	m_pcFoundStrategy->OnStartScanLogic();
	int size = GetStrategyCount();
	for(int i = 0; i < size; i++ ){
		GetStrategy(i)->OnStartScanLogic();
	}
}


// 2005.11.20 Moca�R�����g�̐F������ON/OFF�֌W�Ȃ��s���Ă����o�O���C��
void CColorStrategyPool::CheckColorMODE(
	CColorStrategy**	ppcColorStrategy,	//!< [in,out]
	int					nPos,
	const CStringRef&	cLineStr
)
{
	//�F�I��
	if(*ppcColorStrategy){
		if((*ppcColorStrategy)->EndColor(cLineStr,nPos)){
			*ppcColorStrategy = NULL;
		}
	}

	//�F�J�n
	if(!*ppcColorStrategy){
		// CheckColorMODE �̓��C�A�E�g�����S�̂̃{�g���l�b�N�ɂȂ邭�炢�p�ɂɌĂяo�����
		// ��{�N���X����̓��I���z�֐��Ăяo�����g�p����Ɩ����ł��Ȃ��قǂ̃I�[�o�w�b�h�ɂȂ�͗l
		// �����̓G���K���g���������\�D��ŌX�̔h���N���X���� BeginColor() ���Ăяo��
		if(m_pcHeredoc && m_pcHeredoc->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcHeredoc; return; }
		if(m_pcBlockComment1 && m_pcBlockComment1->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcBlockComment1; return; }
		if(m_pcBlockComment2 && m_pcBlockComment2->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcBlockComment2; return; }
		if(m_pcLineComment && m_pcLineComment->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcLineComment; return; }
		if(m_pcSingleQuote && m_pcSingleQuote->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcSingleQuote; return; }
		if(m_pcDoubleQuote && m_pcDoubleQuote->BeginColor(cLineStr,nPos)){ *ppcColorStrategy = m_pcDoubleQuote; return; }
	}
}

/*! �ݒ�X�V
*/
void CColorStrategyPool::OnChangeSetting(void)
{
	m_vStrategiesDisp.clear();

	m_pcSelectStrategy->Update();
	m_pcFoundStrategy->Update();
	int size = (int)m_vStrategies.size();
	for(int i = 0; i < size; i++){
		m_vStrategies[i]->Update();

		// �F�����\���Ώۂł���Γo�^
		if( m_vStrategies[i]->Disp() ){
			m_vStrategiesDisp.push_back(m_vStrategies[i]);
		}
	}

	// CheckColorMODE �p
	m_pcHeredoc = static_cast<CColor_Heredoc*>(GetStrategyByColor(COLORIDX_HEREDOC));
	m_pcBlockComment1 = static_cast<CColor_BlockComment*>(GetStrategyByColor(COLORIDX_BLOCK1));	// �u���b�N�R�����g
	m_pcBlockComment2 = static_cast<CColor_BlockComment*>(GetStrategyByColor(COLORIDX_BLOCK2));	// �u���b�N�R�����g2
	m_pcLineComment = static_cast<CColor_LineComment*>(GetStrategyByColor(COLORIDX_COMMENT));	// �s�R�����g
	m_pcSingleQuote = static_cast<CColor_SingleQuote*>(GetStrategyByColor(COLORIDX_SSTRING));	// �V���O���N�H�[�e�[�V����������
	m_pcDoubleQuote = static_cast<CColor_DoubleQuote*>(GetStrategyByColor(COLORIDX_WSTRING));	// �_�u���N�H�[�e�[�V����������

	// �F���������Ȃ��ꍇ�ɁA�������X�L�b�v�ł���悤�Ɋm�F����
	const STypeConfig& type = CEditDoc::GetInstance(0)->m_cDocType.GetDocumentAttribute();
	EColorIndexType bSkipColorTypeTable[] = {
		COLORIDX_DIGIT,
		COLORIDX_COMMENT,
		COLORIDX_SSTRING,
		COLORIDX_WSTRING,
		COLORIDX_HEREDOC,
		COLORIDX_URL,
		COLORIDX_KEYWORD1,
		COLORIDX_KEYWORD2,
		COLORIDX_KEYWORD3,
		COLORIDX_KEYWORD4,
		COLORIDX_KEYWORD5,
		COLORIDX_KEYWORD6,
		COLORIDX_KEYWORD7,
		COLORIDX_KEYWORD8,
		COLORIDX_KEYWORD9,
		COLORIDX_KEYWORD10,
	};
	m_bSkipBeforeLayoutGeneral = true;
	int nKeyword1;
	int bUnuseKeyword = false;
	for(int n = 0; n < _countof(bSkipColorTypeTable); n++ ){
		if( COLORIDX_KEYWORD1 == bSkipColorTypeTable[n] ){
			nKeyword1 = n;
		}
		if( COLORIDX_KEYWORD1 <= bSkipColorTypeTable[n] && bSkipColorTypeTable[n] <= COLORIDX_KEYWORD10 ){
			if( type.m_nKeyWordSetIdx[n - nKeyword1] == -1 ){
				bUnuseKeyword = true; // -1�ȍ~�͖���
			}
			if( !bUnuseKeyword && type.m_ColorInfoArr[bSkipColorTypeTable[n]].m_bDisp ){
				m_bSkipBeforeLayoutGeneral = false;
				break;
			}
		}else if( type.m_ColorInfoArr[bSkipColorTypeTable[n]].m_bDisp ){
			m_bSkipBeforeLayoutGeneral = false;
			break;
		}
	}
	if( m_bSkipBeforeLayoutGeneral ){
		if( type.m_bUseRegexKeyword ){
			m_bSkipBeforeLayoutGeneral = false;
		}
	}
	m_bSkipBeforeLayoutFound = true;
	for(int n = COLORIDX_SEARCH; n <= COLORIDX_SEARCHTAIL; n++ ){
		if( type.m_ColorInfoArr[n].m_bDisp ){
			m_bSkipBeforeLayoutFound = false;
			break;
		}
	}
}

bool CColorStrategyPool::IsSkipBeforeLayout()
{
	if( !m_bSkipBeforeLayoutGeneral ){
		return false;
	}
	if( !m_bSkipBeforeLayoutFound && m_pcView->m_bCurSrchKeyMark ){
		return false;
	}
	return true;
}




/*!
  ini�̐F�ݒ��ԍ��łȂ�������ŏ����o���B(added by Stonee, 2001/01/12, 2001/01/15)
  �z��̏��Ԃ͋��L���������̃f�[�^�̏��Ԃƈ�v���Ă���B

  @note ���l�ɂ������I�Ή��� EColorIndexType EColorIndexType.h
  ���{�ꖼ�Ȃǂ�  ColorInfo_DEFAULT CDocTypeSetting.cpp
  CShareData����global�Ɉړ�
*/
const SColorAttributeData g_ColorAttributeArr[] =
{
	{_T("TXT"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_EFFECTS},
	{_T("RUL"), COLOR_ATTRIB_NO_EFFECTS},
	{_T("CAR"), COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// �L�����b�g		// 2006.12.07 ryoji
	{_T("IME"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// IME�L�����b�g	// 2006.12.07 ryoji
	{_T("CBK"), COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{_T("UND"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{_T("CVL"), COLOR_ATTRIB_NO_BACK | ( COLOR_ATTRIB_NO_EFFECTS & ~COLOR_ATTRIB_NO_BOLD )}, // 2007.09.09 Moca �J�[�\���ʒu�c��
	{_T("NOT"), COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{_T("LNO"), 0},
	{_T("MOD"), 0},
	{_T("EBK"), COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{_T("TAB"), 0},
	{_T("SPC"), 0},	//2002.04.28 Add By KK
	{_T("ZEN"), 0},
	{_T("CTL"), 0},
	{_T("EOL"), 0},
	{_T("RAP"), 0},
	{_T("VER"), 0},  // 2005.11.08 Moca �w�茅�c��
	{_T("EOF"), 0},
	{_T("NUM"), 0},	//@@@ 2001.02.17 by MIK ���p���l�̋���
	{_T("BRC"), 0},	//�Ί���	// 02/09/18 ai Add
	{_T("SEL"), 0},
	{_T("FND"), 0},
	{_T("FN2"), 0},
	{_T("FN3"), 0},
	{_T("FN4"), 0},
	{_T("FN5"), 0},
	{_T("CMT"), 0},
	{_T("SQT"), 0},
	{_T("WQT"), 0},
	{_T("HDC"), 0},
	{_T("URL"), 0},
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
	{_T("MRK"), 0},	//�u�b�N�}�[�N	// 02/10/16 ai Add
	{_T("PGV"), COLOR_ATTRIB_NO_TEXT | COLOR_ATTRIB_NO_EFFECTS},
	{_T("LAST"), 0}	// Not Used
};



/*
 * �J���[������C���f�b�N�X�ԍ��ɕϊ�����
 */
int GetColorIndexByName( const TCHAR *name )
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
const TCHAR* GetColorNameByIndex( int index )
{
	return g_ColorAttributeArr[index].szName;
}
