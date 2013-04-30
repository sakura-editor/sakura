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
#ifndef SAKURA_CFIGURESTRATEGY_ADBE415F_6FA5_4412_9679_B0045ACE4881_H_
#define SAKURA_CFIGURESTRATEGY_ADBE415F_6FA5_4412_9679_B0045ACE4881_H_

#include "view/colors/CColorStrategy.h" //SColorStrategyInfo
#include "util/design_template.h"
#include <vector>


//$$���C�A�E�g�\�z�t���[(DoLayout)�� CFigure �ōs���Ɛ������₷��
class CFigure{
public:
	virtual ~CFigure(){}
	virtual bool DrawImp(SColorStrategyInfo* pInfo) = 0;
	virtual bool Match(const wchar_t* pText) const = 0;

	//! �ݒ�X�V
	virtual void Update(void)
	{
		m_pCEditDoc = CEditDoc::GetInstance(0);
		m_pTypeData = &m_pCEditDoc->m_cDocType.GetDocumentAttribute();
	}
protected:
	const CEditDoc* m_pCEditDoc;
	const STypeConfig* m_pTypeData;
};

//! �e��󔒁i���p�󔒁^�S�p�󔒁^�^�u�^���s�j�`��p�̊�{�N���X
class CFigureSpace : public CFigure{
public:
	virtual bool DrawImp(SColorStrategyInfo* pInfo);
protected:
	virtual void DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const = 0;
	virtual EColorIndexType GetColorIdx(void) const = 0;

	// �����⏕
	bool DrawImp_StyleSelect(SColorStrategyInfo* pInfo);
	void DrawImp_StylePop(SColorStrategyInfo* pInfo);
	void DrawImp_DrawUnderline(SColorStrategyInfo* pInfo, DispPos&);
};



class CFigureManager : public TSingleton<CFigureManager>{
public:
	CFigureManager();
	virtual ~CFigureManager();
	CFigure& GetFigure(const wchar_t* pText);

	// �ݒ�ύX
	void OnChangeSetting(void);

private:
	std::vector<CFigure*>	m_vFigures;
};

#endif /* SAKURA_CFIGURESTRATEGY_ADBE415F_6FA5_4412_9679_B0045ACE4881_H_ */
/*[EOF]*/
