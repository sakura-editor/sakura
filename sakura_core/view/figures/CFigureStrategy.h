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
		//return ( COLORIDX_SEARCH == eCurColor || IsRegularExpression(eCurColor) )? 1: 0;
		if( COLORIDX_SEARCH == eCurColor ) return 1;	// ��������������̒��ł͌��݂̔w�i�F�ɍ��킹��

		// �ʏ�e�L�X�g�Ɠ���w�i�F���w�肳��Ă���ꍇ�͒ʏ�e�L�X�g�ȊO�̏ꏊ�ł����݂̔w�i�F�ɍ��킹��
		// ��������ƐF�����炵���ݒ�ɂ��邾���Ŏw��w�i�F�ɂł���
		const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
		const ColorInfo* pcColorInfoArr = pcDoc->m_cDocType.GetDocumentAttribute().m_ColorInfoArr;
		if( pcColorInfoArr[GetColorIdx()].m_colBACK == pcColorInfoArr[COLORIDX_TEXT].m_colBACK ) return 1;

		return 0;
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
