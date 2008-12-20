#include "stdafx.h"
#include "view/figures/CFigureStrategy.h"
#include "CFigure_Tab.h"
#include "CFigure_Eol.h"
#include "CFigure_ZenSpace.h"
#include "CFigure_HanSpace.h"
#include "CFigure_CtrlCode.h"
#include "doc/CLayout.h"
#include "charset/charcode.h"
#include "types/CTypeSupport.h"


//! �ʏ�e�L�X�g�`��
class CFigure_Text : public CFigure{
public:
	bool DrawImp(SColorStrategyInfo* pInfo)
	{
		int nIdx = pInfo->GetPosInLogic();
		int nLength =	CNativeW::GetSizeOfChar(	// �T���Q�[�g�y�A�΍�	2008.10.12 ryoji
							pInfo->pLineOfLogic,
							pInfo->GetDocLine()->GetLengthWithoutEOL(),
							nIdx
						);
		pInfo->pcView->GetTextDrawer().DispText(
			pInfo->gr,
			pInfo->pDispPos,
			&pInfo->pLineOfLogic[nIdx],
			nLength
		);
		pInfo->nPosInLogic += nLength;
		return true;
	}
	bool Match(const wchar_t* pText) const
	{
		return true;
	}
	CLayoutInt GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const
	{
		return CLayoutInt(WCODE::IsZenkaku(pText[0])?2:1);
	}
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�擝��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigureSpace                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool CFigureSpace::DrawImp(SColorStrategyInfo* pInfo)
{
	// ���� DrawImp �� GetSpaceColorType �͊�{�N���X�Ńf�t�H���g������������Ă��邪
	// ���z�֐��Ȃ̂Ŕh���N���X���̃I�[�o�[���C�h�Ŏd�l�ύX�\
	EColorIndexType eCurColor = pInfo->GetCurrentColor();
	int nType = GetSpaceColorType(eCurColor);

	// nType	0:�󔒋L���Ɏw�肳�ꂽ�F���g��, 1:�w�i�F�������݂̔w�i�F�ɍ��킹��
	// ���̒l�ɂ��Ă͕K�v�ɉ����Ďd�l�ύX�E�g�����Ă�������
	// ��j2:�L����\�����Ȃ��i�������F�����݂̔w�i�F�Ɠ����ɂ���j
	CTypeSupport cSupport(pInfo->pcView, pInfo->pcView->GetTextDrawer()._GetColorIdx(GetColorIdx()));
	cSupport.SetGraphicsState_WhileThisObj(pInfo->gr);
	if( nType == 1 )
	{
		CTypeSupport cSupportSpecial(pInfo->pcView, eCurColor);
		pInfo->gr.PushTextBackColor(cSupportSpecial.GetBackColor());
	}
	DispSpace(pInfo->gr, pInfo->pDispPos,pInfo->pcView);
	if( nType == 1 )
		pInfo->gr.PopTextBackColor();
	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigureManager                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
CFigureManager::CFigureManager()
{
	m_vFigures.push_back(new CFigure_Tab());
	m_vFigures.push_back(new CFigure_HanSpace());
	m_vFigures.push_back(new CFigure_ZenSpace());
	m_vFigures.push_back(new CFigure_Eol());
	m_vFigures.push_back(new CFigure_CtrlCode());
	m_vFigures.push_back(new CFigure_Text());
}

CFigureManager::~CFigureManager()
{
	for(int i=0;i<(int)m_vFigures.size();i++){
		SAFE_DELETE(m_vFigures[i]);
	}
	m_vFigures.clear();
}

//$$ �������\
CFigure& CFigureManager::GetFigure(const wchar_t* pText)
{
	for(int i=0;i<(int)m_vFigures.size();i++){
		CFigure* pcFigure = m_vFigures[i];
		if(pcFigure->Match(pText)){
			return *pcFigure;
		}
	}
	assert(0);
	static CFigure_Text cDummy;
	return cDummy;
}

