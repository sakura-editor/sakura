#include "StdAfx.h"
#include "DispPos.h"
#include "doc/layout/CLayout.h"

//$$$高速化
void DispPos::ForwardLayoutLineRef(int nOffsetLine)
{
	m_nLineRef += CLayoutInt(nOffsetLine);
	//キャッシュ更新
	int n = nOffsetLine;
	if(m_pcLayoutRef){
		while(n>0 && m_pcLayoutRef){
			m_pcLayoutRef = m_pcLayoutRef->GetNextLayout();
			n--;
		}
		while(n<0 && m_pcLayoutRef){
			m_pcLayoutRef = m_pcLayoutRef->GetPrevLayout();
			n++;
		}
	}
	else{
		m_pcLayoutRef = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( m_nLineRef );
	}
}
