#pragma once

#include "view/colors/CColorStrategy.h"

class CColor_Select : public CColorStrategy{
public:
	virtual EColorIndexType GetStrategyColor() const{ return COLORIDX_SELECT; }
	//�F�ւ�
	virtual void InitStrategyStatus(){ }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);

	virtual bool BeginColorEx(const CStringRef& cStr, int nPos, CLayoutInt, const CLayout*);

	//�C�x���g
	virtual void OnStartScanLogic();

private:
	bool			m_bSelectFlg;
	CLogicInt		m_nSelectStart;
	CLogicInt		m_nSelectEnd;
};

class CColor_Found : public CColorStrategy{
public:
	CColor_Found();
	virtual EColorIndexType GetStrategyColor() const
	{ return this->validColorNum != 0 ? this->highlightColors[ (m_nSearchResult - 1) % this->validColorNum ] : COLORIDX_DEFAULT; }
	//�F�ւ�
	virtual void InitStrategyStatus(){ } //############�v����
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool EndColor(const CStringRef& cStr, int nPos);
	//�C�x���g
	virtual void OnStartScanLogic();

private:
	int				m_nSearchResult;
	CLogicInt		m_nSearchStart;
	CLogicInt		m_nSearchEnd;
	EColorIndexType highlightColors[ COLORIDX_SEARCHTAIL - COLORIDX_SEARCH + 1 ]; ///< �`�F�b�N���t���Ă��錟��������F�̔z��B
	unsigned validColorNum; ///< highlightColors�̉��Ԗڂ̗v�f�܂ł��L�����B
};
