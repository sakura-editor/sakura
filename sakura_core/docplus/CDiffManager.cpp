#include "StdAfx.h"
#include "docplus/CDiffManager.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "types/CTypeSupport.h"
#include "window/CEditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CDiffLineGetter                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EDiffMark CDiffLineGetter::GetLineDiffMark() const{ return (EDiffMark)m_pcDocLine->m_sMark.m_cDiffmarked; }

/*! �s�̍����}�[�N�ɑΉ������F��Ԃ� -> pnColorIndex
	
	�F�ݒ肪�����ꍇ�� pnColorIndex ��ύX������ false ��Ԃ��B	
*/
bool CDiffLineGetter::GetDiffColor(EColorIndexType* pnColorIndex) const
{
	EDiffMark type = GetLineDiffMark();
	CEditView* pView = &CEditWnd::getInstance()->GetActiveView();

	//DIFF�����}�[�N�\��	//@@@ 2002.05.25 MIK
	if( type ){
		switch( type ){
		case MARK_DIFF_APPEND:	//�ǉ�
			if( CTypeSupport(pView,COLORIDX_DIFF_APPEND).IsDisp() ){
				*pnColorIndex = COLORIDX_DIFF_APPEND;
				return true;
			}
			break;
		case MARK_DIFF_CHANGE:	//�ύX
			if( CTypeSupport(pView,COLORIDX_DIFF_CHANGE).IsDisp() ){
				*pnColorIndex = COLORIDX_DIFF_CHANGE;
				return true;
			}
			break;
		case MARK_DIFF_DELETE:	//�폜
		case MARK_DIFF_DEL_EX:	//�폜
			if( CTypeSupport(pView,COLORIDX_DIFF_DELETE).IsDisp() ){
				*pnColorIndex = COLORIDX_DIFF_DELETE;
				return true;
			}
			break;
		}
	}
	return false;
}


/*! DIFF�}�[�N�`��

	�����͉��B�i���ʂȈ������肻���j
*/
bool CDiffLineGetter::DrawDiffMark(CGraphics& gr, int y, int nLineHeight, CTypeSupport& cColorType) const
{
	EDiffMark type = GetLineDiffMark();

	if( type )	//DIFF�����}�[�N�\��	//@@@ 2002.05.25 MIK
	{
		int	cy = y + nLineHeight / 2;

		gr.PushPen(cColorType.GetTextColor(),0);

		switch( type )
		{
		case MARK_DIFF_APPEND:	//�ǉ�
			::MoveToEx( gr, 3, cy, NULL );
			::LineTo  ( gr, 6, cy );
			::MoveToEx( gr, 4, cy - 2, NULL );
			::LineTo  ( gr, 4, cy + 3 );
			break;

		case MARK_DIFF_CHANGE:	//�ύX
			::MoveToEx( gr, 3, cy - 4, NULL );
			::LineTo  ( gr, 3, cy );
			::MoveToEx( gr, 3, cy + 2, NULL );
			::LineTo  ( gr, 3, cy + 3 );
			break;

		case MARK_DIFF_DELETE:	//�폜
			cy -= 3;
			::MoveToEx( gr, 3, cy, NULL );
			::LineTo  ( gr, 5, cy );
			::LineTo  ( gr, 3, cy + 2 );
			::LineTo  ( gr, 3, cy );
			::LineTo  ( gr, 7, cy + 4 );
			break;
		
		case MARK_DIFF_DEL_EX:	//�폜(EOF)
			cy += 3;
			::MoveToEx( gr, 3, cy, NULL );
			::LineTo  ( gr, 5, cy );
			::LineTo  ( gr, 3, cy - 2 );
			::LineTo  ( gr, 3, cy );
			::LineTo  ( gr, 7, cy - 4 );
			break;
		}

		gr.PopPen();

		return true;
	}
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CDiffLineSetter                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CDiffLineSetter::SetLineDiffMark(EDiffMark mark){ m_pcDocLine->m_sMark.m_cDiffmarked = mark; }


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       CDiffLineMgr                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	�����\���̑S����
	@author	MIK
	@date	2002.05.25
*/
void CDiffLineMgr::ResetAllDiffMark()
{
	CDocLine* pDocLine = m_pcDocLineMgr->GetDocLineTop();
	while( pDocLine )
	{
		pDocLine->m_sMark.m_cDiffmarked = MARK_DIFF_NONE;
		pDocLine = pDocLine->GetNextLine();
	}

	CDiffManager::getInstance()->SetDiffUse(false);
}

/*! ��������
	@author	MIK
	@date	2002.05.25
*/
bool CDiffLineMgr::SearchDiffMark(
	CLogicInt			nLineNum,		//!< �����J�n�s
	ESearchDirection	bPrevOrNext,	//!< 0==�O������ 1==�������
	CLogicInt*			pnLineNum 		//!< �}�b�`�s
)
{
	CLogicInt	nLinePos = nLineNum;

	//�O������
	if( bPrevOrNext == SEARCH_BACKWARD )
	{
		nLinePos--;
		CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		while( pDocLine )
		{
			if( CDiffLineGetter(pDocLine).GetLineDiffMark() != 0 )
			{
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return true;
			}
			nLinePos--;
			pDocLine = pDocLine->GetPrevLine();
		}
	}
	//�������
	else
	{
		nLinePos++;
		CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		while( pDocLine )
		{
			if( CDiffLineGetter(pDocLine).GetLineDiffMark() != 0 )
			{
				*pnLineNum = nLinePos;				/* �}�b�`�s */
				return true;
			}
			nLinePos++;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

/*!	���������s�͈͎w��œo�^����B
	@author	MIK
	@date	2002/05/25
*/
void CDiffLineMgr::SetDiffMarkRange( EDiffMark nMode, CLogicInt nStartLine, CLogicInt nEndLine )
{
	CDiffManager::getInstance()->SetDiffUse(true);

	if( nStartLine < CLogicInt(0) ) nStartLine = CLogicInt(0);

	//�ŏI�s����ɍ폜�s����
	CLogicInt	nLines = m_pcDocLineMgr->GetLineCount();
	if( nLines <= nEndLine )
	{
		nEndLine = nLines - CLogicInt(1);
		CDocLine*	pCDocLine = m_pcDocLineMgr->GetLine( nEndLine );
		if( pCDocLine ) CDiffLineSetter(pCDocLine).SetLineDiffMark(MARK_DIFF_DEL_EX);
	}

	//�s�͈͂Ƀ}�[�N������
	for( CLogicInt i = nStartLine; i <= nEndLine; i++ )
	{
		CDocLine*	pCDocLine = m_pcDocLineMgr->GetLine( i );
		if( pCDocLine ) CDiffLineSetter(pCDocLine).SetLineDiffMark(nMode);
	}

	return;
}
