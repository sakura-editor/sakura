#pragma once

#include "view/colors/CColorStrategy.h" //SColorStrategyInfo
#include "util/design_template.h"
#include <vector>

struct SColorStrategyInfo;

//$$���C�A�E�g�\�z�t���[(DoLayout)�� CFigure �ōs���Ɛ������₷��
class CFigure{
public:
	virtual ~CFigure(){}
	virtual bool DrawImp(SColorStrategyInfo* pInfo) = 0;
	virtual bool Match(const wchar_t* pText) const = 0;
	virtual CLayoutInt GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const = 0;
};

//! �e��󔒁i���p�󔒁^�S�p�󔒁^�^�u�^���s�j�`��p�̊�{�N���X
class CFigureSpace : public CFigure{
public:
	virtual bool DrawImp(SColorStrategyInfo* pInfo);
	virtual int GetSpaceColorType(const EColorIndexType& eCurColor) const
	{
		return ( COLORIDX_SEARCH == eCurColor || IsRegularExpression(eCurColor) )? 1: 0;
	}
	virtual void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView) const = 0;
	virtual EColorIndexType GetColorIdx(void) const = 0;
};



class CFigureManager : public TSingleton<CFigureManager>{
public:
	CFigureManager();
	virtual ~CFigureManager();
	CFigure& GetFigure(const wchar_t* pText);

private:
	std::vector<CFigure*>	m_vFigures;
};
