#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Found : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_SEARCH; }
	//�F�ւ�
	virtual void InitStrategyStatus(){ } //############�v����
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
	//�C�x���g
	virtual void OnStartScanLogic();

private:
	bool			m_bSearchFlg;
	CLogicInt		m_nSearchStart;
	CLogicInt		m_nSearchEnd;
};
