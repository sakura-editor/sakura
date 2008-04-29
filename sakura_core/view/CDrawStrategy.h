#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct DispPos;
#include "DispPos.h"

struct SColorInfo{
	CEditView*				pcView;
	const wchar_t*			pLine;
	CLogicInt				nLineLen;
	int						nBgn;
	CLogicInt				nPos;
	int						nCOMMENTMODE;
	int						nCOMMENTMODE_OLD;
	int						nCOMMENTEND;
	int						nCOMMENTEND_OLD;
	int						nColorIndex;
	BOOL					bSearchStringMode;
	bool					bKeyWordTop;
};

struct SDrawStrategyInfo{
	SDrawStrategyInfo() : sDispPosBegin(0,0) {}

	CEditView*	pcView;
	HDC			hdc;
	CLogicInt	nPos;
	int			nBgn;
	LPCWSTR		pLine;
	CLogicInt	nLineLen;
	DispPos*	pDispPos;

	EColorIndexType	nCOMMENTMODE;
	int				nCOMMENTEND;

	bool			bSearchStringMode;
	bool			bSearchFlg;
	CLogicInt		nSearchStart;
	CLogicInt		nSearchEnd;
	DispPos			sDispPosBegin;
	bool			bKeyWordTop;
	CLogicInt		nCharChars;	//###########����Ȃ����낤�Ȃ�
	int					nCOMMENTEND_OLD;	//#############������
	EColorIndexType		nCOMMENTMODE_OLD;	//#############������

	//! ���݈ʒu�̒��O�܂ŕ`��
	bool DrawToHere()
	{
		bool bRet = false;
		SDrawStrategyInfo* pInfo = this;
		if( pInfo->pDispPos->GetDrawPos().y >= pInfo->pcView->GetTextArea().GetAreaTop() ){
			/* �e�L�X�g�\�� */
			pInfo->pcView->GetTextDrawer().DispText(
				pInfo->hdc,
				pInfo->pDispPos,
				&pInfo->pLine[pInfo->nBgn],
				pInfo->nPos - pInfo->nBgn
			);
			bRet = true;
		}
		pInfo->nBgn = pInfo->nPos;
		return bRet;
	}

	//! �F�̐؂�ւ�
	void ChangeColor(EColorIndexType eNewColor)
	{
		SDrawStrategyInfo* pInfo = this;
		pInfo->nCOMMENTMODE = eNewColor;
		if( !pInfo->bSearchStringMode ){
			pInfo->pcView->SetCurrentColor( pInfo->hdc, pInfo->nCOMMENTMODE );
		}
	}

};

class CDrawStrategy{
public:
	virtual ~CDrawStrategy(){}
	//! �F�؂�ւ��J�n�����o������A���̒��O�܂ł̕`����s���A����ɐF�ݒ���s���B
//	virtual bool BeginColor(SDrawStrategyInfo* pInfo) = 0;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ��̉�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



class CDraw_URL : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool GetColorIndexImp(SColorInfo* pInfo);
};

class CDraw_Numeric : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool GetColorIndexImp(SColorInfo* pInfo);
};

class CDraw_KeywordSet : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool GetColorIndexImp(SColorInfo* pInfo);
};


class CDraw_CtrlCode : public CDrawStrategy{
public:
	virtual bool BeginColor(SDrawStrategyInfo* pInfo);
	virtual bool EndColor(SDrawStrategyInfo* pInfo);
};



//����

class CDraw_ColorEnd : public CDrawStrategy{
public:
	virtual bool EndColor(SDrawStrategyInfo* pInfo);
};

class CDraw_Line : public CDrawStrategy{
public:
	virtual bool EndColor(SDrawStrategyInfo* pInfo);
};
