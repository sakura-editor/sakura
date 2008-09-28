#include "stdafx.h"
#include "view/figures/CFigureStrategy.h"
#include "CFigure_Tab.h"
#include "CFigure_Eol.h"
#include "CFigure_ZenSpace.h"
#include "CFigure_HanSpace.h"
#include "doc/CLayout.h"
#include "charset/charcode.h"


//! �ʏ�e�L�X�g�`��
class CFigure_Text : public CFigure{
public:
	bool DrawImp(SColorStrategyInfo* pInfo)
	{
		pInfo->pcView->GetTextDrawer().DispText(
			pInfo->gr,
			pInfo->pDispPos,
			&pInfo->pLineOfLogic[pInfo->GetPosInLogic()],
			1
		);
		pInfo->nPosInLogic++;
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
//                      CFigureManager                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
CFigureManager::CFigureManager()
{
	m_vFigures.push_back(new CFigure_Tab());
	m_vFigures.push_back(new CFigure_HanSpace());
	m_vFigures.push_back(new CFigure_ZenSpace());
	m_vFigures.push_back(new CFigure_Eol());
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

