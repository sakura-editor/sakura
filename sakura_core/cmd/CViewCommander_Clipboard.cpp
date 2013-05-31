/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�N���b�v�{�[�h�n)�֐��Q

	2012/12/20	CViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, novice
	Copyright (C) 2002, hor, genta, Azumaiya, ���Ȃӂ�
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2007, ryoji
	Copyright (C) 2010, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"
#include "view/colors/CColorStrategy.h"
#include "view/colors/CColor_Found.h"
#include "uiparts/CWaitCursor.h"
#include "util/os.h"


/** �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)

	@date 2007.11.18 ryoji �u�I���Ȃ��ŃR�s�[���\�ɂ���v�I�v�V���������ǉ�
*/
void CViewCommander::Command_CUT( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	CNativeW	cmemBuf;
	bool	bBeginBoxSelect;
	/* �͈͑I��������Ă��Ȃ� */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* ��I�����́A�J�[�\���s��؂��� */
		if( !GetDllShareData().m_Common.m_sEdit.m_bEnableNoSelectCopy ){	// 2007.11.18 ryoji
			return;	// �������Ȃ��i�����炳�Ȃ��j
		}
		//�s�؂���(�܂�Ԃ��P��)
		Command_CUT_LINE();
		return;
	}
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		bBeginBoxSelect = true;
	}else{
		bBeginBoxSelect = false;
	}

	/* �I��͈͂̃f�[�^���擾 */
	/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
	if( !m_pCommanderView->GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		ErrorBeep();
		return;
	}
	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
	if( !m_pCommanderView->MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), bBeginBoxSelect ) ){
		ErrorBeep();
		return;
	}

	/* �J�[�\���ʒu�܂��͑I���G���A���폜 */
	m_pCommanderView->DeleteData( true );
	return;
}



/**	�I��͈͂��N���b�v�{�[�h�ɃR�s�[

	@date 2007.11.18 ryoji �u�I���Ȃ��ŃR�s�[���\�ɂ���v�I�v�V���������ǉ�
*/
void CViewCommander::Command_COPY(
	bool		bIgnoreLockAndDisable,	//!< [in] �I��͈͂��������邩�H
	bool		bAddCRLFWhenCopy,		//!< [in] �܂�Ԃ��ʒu�ɉ��s�R�[�h��}�����邩�H
	EEolType	neweol					//!< [in] �R�s�[����Ƃ���EOL�B
)
{
	CNativeW	cmemBuf;
	bool		bBeginBoxSelect = false;

	/* �N���b�v�{�[�h�ɓ����ׂ��e�L�X�g�f�[�^���AcmemBuf�Ɋi�[���� */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* ��I�����́A�J�[�\���s���R�s�[���� */
		if( !GetDllShareData().m_Common.m_sEdit.m_bEnableNoSelectCopy ){	// 2007.11.18 ryoji
			return;	// �������Ȃ��i�����炳�Ȃ��j
		}
		m_pCommanderView->CopyCurLine(
			bAddCRLFWhenCopy,
			neweol,
			GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste
		);
	}
	else{
		/* �e�L�X�g���I������Ă���Ƃ��́A�I��͈͂̃f�[�^���擾 */

		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			bBeginBoxSelect = TRUE;
		}
		/* �I��͈͂̃f�[�^���擾 */
		/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
		if( !m_pCommanderView->GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, bAddCRLFWhenCopy, neweol ) ){
			ErrorBeep();
			return;
		}

		/* �N���b�v�{�[�h�Ƀf�[�^cmemBuf�̓��e��ݒ� */
		if( !m_pCommanderView->MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), bBeginBoxSelect, FALSE ) ){
			ErrorBeep();
			return;
		}
	}

	/* �I��͈͂̌�Еt�� */
	if( !bIgnoreLockAndDisable ){
		/* �I����Ԃ̃��b�N */
		if( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = FALSE;
			m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
			if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
				GetCaret().m_cUnderLine.CaretUnderLineON(true, false);
			}
		}
	}
	if( GetDllShareData().m_Common.m_sEdit.m_bCopyAndDisablSelection ){	/* �R�s�[������I������ */
		/* �e�L�X�g���I������Ă��邩 */
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( true );
		}
	}
	return;
}



/** �\��t��(�N���b�v�{�[�h����\��t��)
	@param [in] option �\��t�����̃I�v�V����
	@li 0x01 ���s�R�[�h�ϊ��L��
	@li 0x02 ���s�R�[�h�ϊ�����
	@li 0x04 ���C�����[�h�\��t���L��
	@li 0x08 ���C�����[�h�\��t������
	@li 0x10 ��`�R�s�[�͏�ɋ�`�\��t��
	@li 0x20 ��`�R�s�[�͏�ɒʏ�\��t��

	@date 2007.10.04 ryoji MSDEVLineSelect�`���̍s�R�s�[�Ή�������ǉ��iVS2003/2005�̃G�f�B�^�Ɨގ��̋����Ɂj
*/
void CViewCommander::Command_PASTE( int option )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	//�����v
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	// �N���b�v�{�[�h����f�[�^���擾 -> cmemClip, bColumnSelect
	CNativeW	cmemClip;
	bool		bColumnSelect;
	bool		bLineSelect = false;
	bool		bLineSelectOption = 
		((option & 0x04) == 0x04) ? true :
		((option & 0x08) == 0x08) ? false :
		GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste;

	if( !m_pCommanderView->MyGetClipboardData( cmemClip, &bColumnSelect, bLineSelectOption ? &bLineSelect: NULL ) ){
		ErrorBeep();
		return;
	}

	// �N���b�v�{�[�h�f�[�^�擾 -> pszText, nTextLen
	CLogicInt		nTextLen;
	const wchar_t*	pszText = cmemClip.GetStringPtr(&nTextLen);

	bool bConvertEol = 
		((option & 0x01) == 0x01) ? true :
		((option & 0x02) == 0x02) ? false :
		GetDllShareData().m_Common.m_sEdit.m_bConvertEOLPaste;

	bool bAutoColumnPaste = 
		((option & 0x10) == 0x10) ? true :
		((option & 0x20) == 0x20) ? false :
		GetDllShareData().m_Common.m_sEdit.m_bAutoColumnPaste != FALSE;

	// ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t��
	if( bAutoColumnPaste ){
		// ��`�R�s�[�̃f�[�^�Ȃ��`�\��t��
		if( bColumnSelect ){
			if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){
				ErrorBeep();
				return;
			}
			if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH ){
				return;
			}
			Command_PASTEBOX(pszText, nTextLen);
			m_pCommanderView->AdjustScrollBars();
			m_pCommanderView->Redraw();
			return;
		}
	}

	// 2007.10.04 ryoji
	// �s�R�s�[�iMSDEVLineSelect�`���j�̃e�L�X�g�Ŗ��������s�ɂȂ��Ă��Ȃ���Ή��s��ǉ�����
	// �����C�A�E�g�܂�Ԃ��̍s�R�s�[�������ꍇ�͖��������s�ɂȂ��Ă��Ȃ�
	if( bLineSelect ){
		if( !WCODE::IsLineDelimiter(pszText[nTextLen - 1]) ){
			cmemClip.AppendString(GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2());
			pszText = cmemClip.GetStringPtr( &nTextLen );
		}
	}

	if( bConvertEol ){
		wchar_t	*pszConvertedText = new wchar_t[nTextLen * 2]; // �S����\n��\r\n�ϊ��ōő�̂Q�{�ɂȂ�
		CLogicInt nConvertedTextLen = ConvertEol( pszText, nTextLen, pszConvertedText );
		// �e�L�X�g��\��t��
		Command_INSTEXT( true, pszConvertedText, nConvertedTextLen, true, bLineSelect );	// 2010.09.17 ryoji
		delete [] pszConvertedText;
	}else{
		// �e�L�X�g��\��t��
		Command_INSTEXT( true, pszText, nTextLen, true, bLineSelect );	// 2010.09.17 ryoji
	}

	return;
}



//<< 2002/03/28 Azumaiya
// �������f�[�^����`�\��t���p�̃f�[�^�Ɖ��߂��ď�������B
//  �Ȃ��A���̊֐��� Command_PASTEBOX(void) �ƁA
// 2769 : GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta
// ����A
// 3057 : m_pCommanderView->SetDrawSwitch(true);	// 2002.01.25 hor
// �Ԃ܂ŁA�ꏏ�ł��B
//  �ł����A�R�����g���������A#if 0 �̂Ƃ����������肵�Ă��܂��̂ŁACommand_PASTEBOX(void) ��
// �c���悤�ɂ��܂���(���ɂ��̊֐����g�����g�����o�[�W�������R�����g�ŏ����Ă����܂���)�B
//  �Ȃ��A�ȉ��ɂ�����悤�� Command_PASTEBOX(void) �ƈႤ�Ƃ��낪����̂Œ��ӂ��Ă��������B
// > �Ăяo�������ӔC�������āA
// �E�}�E�X�ɂ��͈͑I�𒆂ł���B
// �E���݂̃t�H���g�͌Œ蕝�t�H���g�ł���B
// �� 2 �_���`�F�b�N����B
// > �ĕ`����s��Ȃ�
// �ł��B
//  �Ȃ��A�������Ăяo�����Ɋ��҂���킯�́A�u���ׂĒu���v�̂悤�ȉ�����A���ŌĂяo��
// �Ƃ��ɁA�ŏ��Ɉ��`�F�b�N����΂悢���̂�������`�F�b�N����͖̂��ʂƔ��f�������߂ł��B
// @note 2004.06.30 ���݁A���ׂĒu���ł͎g�p���Ă��Ȃ�
void CViewCommander::Command_PASTEBOX( const wchar_t *szPaste, int nPasteSize )
{
	/* �����̓���͎c���Ă��������̂����A�Ăяo�����ŐӔC�������Ă���Ă��炤���ƂɕύX�B
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )	// �}�E�X�ɂ��͈͑I��
	{
		ErrorBeep();
		return;
	}
	if( !GetDllShareData().m_Common.m_bFontIs_FIXED_PITCH )	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
	{
		return;
	}
	*/

	int				nBgn;
	int				nPos;
	CLayoutInt		nCount;
	CLayoutPoint	ptLayoutNew;	//�}�����ꂽ�����̎��̈ʒu
	BOOL			bAddLastCR;
	CLayoutInt		nInsPosX;

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	m_pCommanderView->SetDrawSwitch(false);	// 2002.01.25 hor

	// �Ƃ肠�����I��͈͂��폜
	// 2004.06.30 Moca m_pCommanderView->GetSelectionInfo().IsTextSelected()���Ȃ��Ɩ��I�����A�ꕶ�������Ă��܂�
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		m_pCommanderView->DeleteData( false/*true 2002.01.25 hor*/ );
	}

	CLayoutPoint ptCurOld = GetCaret().GetCaretLayoutPos();

	nCount = CLayoutInt(0);

	// Jul. 10, 2005 genta �\��t���f�[�^�̍Ō��CR/LF�������ꍇ�̑΍�
	//	�f�[�^�̍Ō�܂ŏ��� i.e. nBgn��nPasteSize�𒴂�����I��
	//for( nPos = 0; nPos < nPasteSize; )
	for( nBgn = nPos = 0; nBgn < nPasteSize; )
	{
		// Jul. 10, 2005 genta �\��t���f�[�^�̍Ō��CR/LF��������
		//	�ŏI�s��Paste�����������Ȃ��̂ŁC
		//	�f�[�^�̖����ɗ����ꍇ�͋����I�ɏ�������悤�ɂ���
		if( WCODE::IsLineDelimiter(szPaste[nPos]) || nPos == nPasteSize )
		{
			/* ���݈ʒu�Ƀf�[�^��}�� */
			if( nPos - nBgn > 0 ){
				m_pCommanderView->InsertData_CEditView(
					ptCurOld + CLayoutPoint(CLayoutInt(0), nCount),
					&szPaste[nBgn],
					nPos - nBgn,
					&ptLayoutNew,
					false
				);
			}

			/* ���̍s�̑}���ʒu�փJ�[�\�����ړ� */
			GetCaret().MoveCursor( ptCurOld + CLayoutPoint(CLayoutInt(0), nCount), false );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			/* �J�[�\���s���Ō�̍s���s���ɉ��s�������A�}�����ׂ��f�[�^���܂�����ꍇ */
			bAddLastCR = FALSE;
			const CLayout*	pcLayout;
			CLogicInt		nLineLen = CLogicInt(0);
			const wchar_t*	pLine;
			pLine = GetDocument()->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );

			if( NULL != pLine && 1 <= nLineLen )
			{
				if( WCODE::IsLineDelimiter(pLine[nLineLen - 1]) )
				{
				}
				else
				{
					bAddLastCR = TRUE;
				}
			}
			else
			{ // 2001/10/02 novice
				bAddLastCR = TRUE;
			}

			if( bAddLastCR )
			{
//				MYTRACE( _T(" �J�[�\���s���Ō�̍s���s���ɉ��s�������A\n�}�����ׂ��f�[�^���܂�����ꍇ�͍s���ɉ��s��}���B\n") );
				nInsPosX = m_pCommanderView->LineIndexToColumn( pcLayout, nLineLen );

				m_pCommanderView->InsertData_CEditView(
					CLayoutPoint(nInsPosX, GetCaret().GetCaretLayoutPos().GetY2()),
					GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2(),
					GetDocument()->m_cDocEditor.GetNewLineCode().GetLen(),
					&ptLayoutNew,
					false
				);
			}

			if(
				(nPos + 1 < nPasteSize ) &&
				 ( szPaste[nPos] == L'\r' && szPaste[nPos + 1] == L'\n')
			  )
			{
				nBgn = nPos + 2;
			}
			else
			{
				nBgn = nPos + 1;
			}

			nPos = nBgn;
			++nCount;
		}
		else
		{
			++nPos;
		}
	}

	/* �}���f�[�^�̐擪�ʒu�փJ�[�\�����ړ� */
	GetCaret().MoveCursor( ptCurOld, true );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	if( !m_pCommanderView->m_bDoing_UndoRedo )	/* �A���h�D�E���h�D�̎��s���� */
	{
		/* ����̒ǉ� */
		GetOpeBlk()->AppendOpe( 
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}

	m_pCommanderView->SetDrawSwitch(true);	// 2002.01.25 hor
	return;
}



/** ��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
	@param [in] option ���g�p

	@date 2004.06.29 Moca ���g�p���������̂�L���ɂ���
	�I���W�i����Command_PASTEBOX(void)�͂΂�����폜 (genta)
*/
void CViewCommander::Command_PASTEBOX( int option )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )	// �}�E�X�ɂ��͈͑I��
	{
		ErrorBeep();
		return;
	}

	if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH )	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
	{
		return;
	}

	// �N���b�v�{�[�h����f�[�^���擾
	CNativeW	cmemClip;
	if( !m_pCommanderView->MyGetClipboardData( cmemClip, NULL ) ){
		ErrorBeep();
		return;
	}
	// 2004.07.13 Moca \0�R�s�[�΍�
	int nstrlen;
	const wchar_t *lptstr = cmemClip.GetStringPtr( &nstrlen );

	Command_PASTEBOX(lptstr, nstrlen);
	m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
	m_pCommanderView->Redraw();			// 2002.01.25 hor
}



/*! ��`������}��
*/
void CViewCommander::Command_INSBOXTEXT( const wchar_t *pszPaste, int nPasteSize )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )	// �}�E�X�ɂ��͈͑I��
	{
		ErrorBeep();
		return;
	}

	if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH )	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
	{
		return;
	}

	Command_PASTEBOX(pszPaste, nPasteSize);
	m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
	m_pCommanderView->Redraw();			// 2002.01.25 hor
}



/*! �e�L�X�g��\��t��
	@date 2004.05.14 Moca '\\0'���󂯓����悤�ɁA�����ɒ�����ǉ�
	@date 2010.09.17 ryoji ���C�����[�h�\��t���I�v�V������ǉ����ĈȑO�� Command_PASTE() �Ƃ̏d�����𐮗��E����
*/
void CViewCommander::Command_INSTEXT(
	bool			bRedraw,		//!< 
	const wchar_t*	pszText,		//!< [in] �\��t���镶����B
	CLogicInt		nTextLen,		//!< [in] pszText�̒����B-1���w�肷��ƁApszText��NUL�I�[������Ƃ݂Ȃ��Ē����������v�Z����
	bool			bNoWaitCursor,	//!< 
	bool			bLinePaste		//!< [in] ���C�����[�h�\��t��
)
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	CWaitCursor*	pcWaitCursor;
	if( bNoWaitCursor ){
		pcWaitCursor = NULL;
	}else{
		pcWaitCursor = new CWaitCursor( m_pCommanderView->GetHwnd() );
	}

	if( nTextLen < 0 ){
		nTextLen = CLogicInt(wcslen( pszText ));
	}

	GetDocument()->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta

	// �e�L�X�g���I������Ă��邩
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		// ��`�͈͑I�𒆂�
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			//���s�܂ł𔲂��o��
			CLogicInt i;
			for( i = CLogicInt(0); i < nTextLen; i++ ){
				if( WCODE::IsLineDelimiter(pszText[i]) ){
					break;
				}
			}
			Command_INDENT( pszText, i );
			goto end_of_func;
		}
		else{
			//	Jun. 23, 2000 genta
			//	����s�̍s���ȍ~�݂̂��I������Ă���ꍇ�ɂ͑I�𖳂��ƌ��Ȃ�
			CLogicInt		len;
			int pos;
			const wchar_t	*line;
			const CLayout* pcLayout;
			line = GetDocument()->m_cLayoutMgr.GetLineStr( GetSelect().GetFrom().GetY2(), &len, &pcLayout );

			pos = ( line == NULL ) ? 0 : m_pCommanderView->LineColumnToIndex( pcLayout, GetSelect().GetFrom().GetX2() );

			//	�J�n�ʒu���s�������ŁA�I���ʒu������s
			if( pos >= len && GetSelect().IsLineOne()){
				GetCaret().SetCaretLayoutPos(CLayoutPoint(GetSelect().GetFrom().x, GetCaret().GetCaretLayoutPos().y)); //�L�����b�gX�ύX
				m_pCommanderView->GetSelectionInfo().DisableSelectArea(false);
			}
			else{
				// �f�[�^�u�� �폜&�}���ɂ��g����
				// �s�R�s�[�̓\��t���ł͑I��͈͍͂폜�i��ōs���ɓ\��t����j	// 2007.10.04 ryoji
				m_pCommanderView->ReplaceData_CEditView(
					GetSelect(),				// �I��͈�
					NULL,					// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
					bLinePaste? L"": pszText,	// �}������f�[�^
					bLinePaste? CLogicInt(0): nTextLen,	// �}������f�[�^�̒���
					bRedraw,
					m_pCommanderView->m_bDoing_UndoRedo?NULL:GetOpeBlk()
				);
				if( !bLinePaste )	// 2007.10.04 ryoji
					goto end_of_func;
			}
		}
	}

	{	// ��I�����̏��� or ���C�����[�h�\��t�����̎c��̏���
		CLogicInt	nPosX_PHY_Delta(0);
		if( bLinePaste ){	// 2007.10.04 ryoji
			/* �}���|�C���g�i�܂�Ԃ��P�ʍs���j�ɃJ�[�\�����ړ� */
			CLogicPoint ptCaretBefore = GetCaret().GetCaretLogicPos();	// ����O�̃L�����b�g�ʒu
			Command_GOLINETOP( false, 1 );								// �s���Ɉړ�(�܂�Ԃ��P��)
			CLogicPoint ptCaretAfter = GetCaret().GetCaretLogicPos();	// �����̃L�����b�g�ʒu

			// �}���|�C���g�ƌ��̈ʒu�Ƃ̍���������
			nPosX_PHY_Delta = ptCaretBefore.x - ptCaretAfter.x;

			//UNDO�p�L�^
			if( !m_pCommanderView->m_bDoing_UndoRedo ){
				GetOpeBlk()->AppendOpe(
					new CMoveCaretOpe(
						ptCaretBefore,	/* ����O�̃L�����b�g�ʒu */
						ptCaretAfter	/* �����̃L�����b�g�ʒu */
					)
				);
			}
		}

		// ���݈ʒu�Ƀf�[�^��}��
		CLayoutPoint ptLayoutNew; //�}�����ꂽ�����̎��̈ʒu
		m_pCommanderView->InsertData_CEditView(
			GetCaret().GetCaretLayoutPos(),
			pszText,
			nTextLen,
			&ptLayoutNew,
			bRedraw
		);

		// �}���f�[�^�̍Ō�փJ�[�\�����ړ�
		GetCaret().MoveCursor( ptLayoutNew, bRedraw );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if( bLinePaste ){	// 2007.10.04 ryoji
			/* ���̈ʒu�փJ�[�\�����ړ� */
			CLogicPoint ptCaretBefore = GetCaret().GetCaretLogicPos();	//����O�̃L�����b�g�ʒu
			CLayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				ptCaretBefore + CLogicPoint(nPosX_PHY_Delta, CLogicInt(0)),
				&ptLayout
			);
			GetCaret().MoveCursor( ptLayout, bRedraw );					//�J�[�\���ړ�
			CLogicPoint ptCaretAfter = GetCaret().GetCaretLogicPos();	//�����̃L�����b�g�ʒu
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;

			//UNDO�p�L�^
			if( !m_pCommanderView->m_bDoing_UndoRedo ){
				GetOpeBlk()->AppendOpe(
					new CMoveCaretOpe(
						ptCaretBefore,	/* ����O�̃L�����b�g�ʒu�w */
						ptCaretAfter	/* �����̃L�����b�g�ʒu�w */
					)
				);
			}
		}
	}

end_of_func:
	delete pcWaitCursor;

	return;
}



/* �Ō�Ƀe�L�X�g��ǉ� */
void CViewCommander::Command_ADDTAIL(
	const wchar_t*	pszData,	//!< �ǉ�����e�L�X�g
	int				nDataLen	//!< �ǉ�����e�L�X�g�̒����B�����P�ʁB-1���w�肷��ƁA�e�L�X�g�I�[�܂ŁB
)
{
	//�e�L�X�g�������v�Z
	if(nDataLen==-1 && pszData!=NULL)nDataLen=wcslen(pszData);

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	/*�t�@�C���̍Ō�Ɉړ� */
	Command_GOFILEEND( false );

	/* ���݈ʒu�Ƀf�[�^��}�� */
	CLayoutPoint ptLayoutNew;	// �}�����ꂽ�����̎��̈ʒu
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		pszData,
		nDataLen,
		&ptLayoutNew,
		true
	);

	/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
	// Sep. 2, 2002 ���Ȃӂ� �A���_�[���C���̕\�����c���Ă��܂������C��
	GetCaret().MoveCursor( ptLayoutNew, true );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
}



//�I��͈͓��S�s�R�s�[
void CViewCommander::Command_COPYLINES( void )
{
	/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	m_pCommanderView->CopySelectedAllLines(
		NULL,	/* ���p�� */
		FALSE	/* �s�ԍ���t�^���� */
	);
	return;
}



//�I��͈͓��S�s���p���t���R�s�[
void CViewCommander::Command_COPYLINESASPASSAGE( void )
{
	/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	m_pCommanderView->CopySelectedAllLines(
		GetDllShareData().m_Common.m_sFormat.m_szInyouKigou,	/* ���p�� */
		FALSE 									/* �s�ԍ���t�^���� */
	);
	return;
}



//�I��͈͓��S�s�s�ԍ��t���R�s�[
void CViewCommander::Command_COPYLINESWITHLINENUMBER( void )
{
	/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	m_pCommanderView->CopySelectedAllLines(
		NULL,	/* ���p�� */
		TRUE	/* �s�ԍ���t�^���� */
	);
	return;
}



static bool AppendHTMLColor(
	COLORREF cTEXTLast, COLORREF& cTEXTLast2, COLORREF cBACKLast, COLORREF& cBACKLast2,
	bool bBoldLast, bool& bBoldLast2, bool bUnderLineLast, bool& bUnderLineLast2,
	const WCHAR* pAppendStr, int nLen,
	CNativeW& cmemClip)
{
	if( bBoldLast != bBoldLast2 || bUnderLineLast != bUnderLineLast2
	  || cTEXTLast != cTEXTLast2 || cBACKLast != cBACKLast2 ){
		if( bBoldLast2 ){
			cmemClip.AppendString( L"</b>", 4 );
		}
		if( bUnderLineLast2 ){
			if( cTEXTLast != cTEXTLast2 || cBACKLast != cBACKLast2 || bUnderLineLast != bUnderLineLast2 ){
				cmemClip.AppendString( L"</u>", 4 );
			}
		}
		if( cTEXTLast != cTEXTLast2 || cBACKLast != cBACKLast2 ){
			if( cTEXTLast2 != (COLORREF)-1 ){
					cmemClip.AppendString( L"</span>", 7 );
				}
			if( cTEXTLast != (COLORREF)-1 ){
				if( cTEXTLast != cTEXTLast2 || cBACKLast != cBACKLast2 ){
					WCHAR szColor[60];
					DWORD dwTEXTColor = (GetRValue(cTEXTLast) << 16) + (GetGValue(cTEXTLast) << 8) + GetBValue(cTEXTLast);
					DWORD dwBACKColor = (GetRValue(cBACKLast) << 16) + (GetGValue(cBACKLast) << 8) + GetBValue(cBACKLast);
					swprintf( szColor, L"<span style=\"color:#%06x;background-color:#%06x\">", dwTEXTColor, dwBACKColor);
					cmemClip.AppendString( szColor );
				}
			}
		}
		if( bUnderLineLast ){
			if( cTEXTLast != cTEXTLast2 || cBACKLast != cBACKLast2 || bUnderLineLast != bUnderLineLast2 ){
				cmemClip.AppendString( L"<u>", 3 );
			}
		}
		if( bBoldLast ){
			cmemClip.AppendString( L"<b>", 3 );
		}
		cTEXTLast2 = cTEXTLast;
		cBACKLast2 = cBACKLast;
		bUnderLineLast2 = bUnderLineLast;
		bBoldLast2 = bBoldLast;
	}
	CNativeW cmemBuf(pAppendStr, nLen);
	cmemBuf.Replace(L"&", L"&amp;");
	cmemBuf.Replace(L"<", L"&lt;");
	cmemBuf.Replace(L">", L"&gt;");
	cmemClip.AppendNativeData(cmemBuf);
	if( 0 < nLen ){
		return WCODE::IsLineDelimiter(pAppendStr[nLen-1]);
	}
	return false;
}



//!�I��͈͓��F�t��HTML�R�s�[
void CViewCommander::Command_COPY_COLOR_HTML(bool bLineNumber)
{
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected()
	  || GetSelect().GetFrom() == GetSelect().GetTo() ){
		return;
	}
	const STypeConfig& type = GetDocument()->m_cDocType.GetDocumentAttribute();
	bool bLineNumLayout = GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy
		|| m_pCommanderView->GetSelectionInfo().IsBoxSelecting();
	CLayoutRect rcSel;
	TwoPointToRect(
		&rcSel,
		GetSelect().GetFrom(),	// �͈͑I���J�n
		GetSelect().GetTo()		// �͈͑I���I��
	);
	// �C�������������o�b�t�@�̒��������������Ōv�Z
	CLogicRange sSelectLogic;
	sSelectLogic.Clear(-1);
	int nBuffSize = 0;
	const CLayout* pcLayoutTop = NULL;
	{
		const CLayout* pcLayout;
		{
			CLogicInt nLineLenTmp;
			GetDocument()->m_cLayoutMgr.GetLineStr(rcSel.top, &nLineLenTmp, &pcLayout);
		}
		pcLayoutTop = pcLayout;
		CLayoutInt i = rcSel.top;
		for(; pcLayout != NULL && i <= rcSel.bottom; i++, pcLayout = pcLayout->GetNextLayout())
		{
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			CLogicInt nIdxFrom;
			CLogicInt nIdxTo;
			if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
				nIdxFrom = m_pCommanderView->LineColumnToIndex(pcLayout, rcSel.left );
				nIdxTo   = m_pCommanderView->LineColumnToIndex(pcLayout, rcSel.right);
				// ���s�͏���
				if( nIdxTo - nIdxFrom > 0 ){
					const WCHAR* pLine = pcLayout->GetPtr();
					if( pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r' ){
						--nIdxTo;
					}
				}
				if( i == rcSel.top ){
					sSelectLogic.SetFromY(pcLayout->GetLogicLineNo());
					sSelectLogic.SetFromX(nIdxFrom);
				}
				if( i == rcSel.bottom ){
					sSelectLogic.SetToY(pcLayout->GetLogicLineNo());
					sSelectLogic.SetToX(nIdxTo);
				}
			}else{
				if( i == rcSel.top ){
					nIdxFrom = m_pCommanderView->LineColumnToIndex(pcLayout, rcSel.left);
					sSelectLogic.SetFromY(pcLayout->GetLogicLineNo());
					sSelectLogic.SetFromX(nIdxFrom);
				}else{
					nIdxFrom = CLogicInt(0);
				}
				if( i == rcSel.bottom ){
					nIdxTo = m_pCommanderView->LineColumnToIndex(pcLayout, rcSel.right);
					sSelectLogic.SetToY(pcLayout->GetLogicLineNo());
					sSelectLogic.SetToX(nIdxTo);
				}else{
					nIdxTo = pcLayout->GetLengthWithoutEOL();
				}
			}
			nBuffSize += nIdxTo - nIdxFrom;
			if( bLineNumLayout ){
				nBuffSize += 2;
			}else{
				nBuffSize += pcLayout->GetLayoutEol().GetLen();
			}
		}
		if( sSelectLogic.GetTo().x == -1 ){
			sSelectLogic.SetToY(GetDocument()->m_cDocLineMgr.GetLineCount());
			sSelectLogic.SetToX(CLogicInt(0));
		}
	}
	// �s�ԍ��̕����v�Z
	int nLineNumberMaxLen = 0;
	WCHAR szLineFormat[10];
	szLineFormat[0] = L'\0';
	CNativeW cmemNullLine;
	if( bLineNumber ){
		int nLineNumberMax;
		if( type.m_bLineNumIsCRLF ){
			nLineNumberMax = sSelectLogic.GetTo().GetY();
		}else{
			nLineNumberMax = (Int)rcSel.bottom;
		}
		int nWork = 10;
		int i;
		cmemNullLine.AppendString(L" ");
		for( i = 1; i < 12; ++i ){
			if( nWork > nLineNumberMax ){
				break;
			}
			nWork *= 10;
			cmemNullLine.AppendString(L" ");
		}
		nLineNumberMaxLen = i + 1; // "%d:"
		cmemNullLine.AppendString(L":");
		swprintf(szLineFormat, L"%%%dd:", i);
	}
	if( bLineNumLayout ){
		nBuffSize += (Int)(nLineNumberMaxLen * (rcSel.bottom - rcSel.top + 1));
	}else{
		nBuffSize += (Int)(nLineNumberMaxLen * (sSelectLogic.GetTo().y - sSelectLogic.GetFrom().y + 1));
	}
	CNativeW cmemClip;
	cmemClip.AllocStringBuffer(nBuffSize + 11);
	{
		COLORREF cBACK = type.m_ColorInfoArr[COLORIDX_TEXT].m_colBACK;
		DWORD dwBACKColor = (GetRValue(cBACK) << 16) + (GetGValue(cBACK) << 8) + GetBValue(cBACK);
		WCHAR szBuf[50];
		swprintf(szBuf, L"<pre style=\"background-color:#%06x\">", dwBACKColor);
		cmemClip.AppendString( szBuf );
	}
	CLayoutInt nLayoutLineNum = rcSel.top;
	const CLogicInt nLineNumLast = sSelectLogic.GetTo().y;
	const CDocLine* pcDocLine = pcLayoutTop->GetDocLineRef();
	const CLayout* pcLayout = pcLayoutTop;
	while( pcLayout && pcLayout->GetLogicOffset() ){
		pcLayout = pcLayout->GetPrevLayout();
	}
	COLORREF cTEXT = (COLORREF)-1, cBACK = (COLORREF)-1;
	COLORREF cTEXTNext = (COLORREF)-1, cBACKNext = (COLORREF)-1;
	COLORREF cTEXTLast = (COLORREF)-1, cBACKLast = (COLORREF)-1;
	COLORREF cTEXTLast2 = (COLORREF)-1, cBACKLast2 = (COLORREF)-1;
	bool bBold = false, bUnderLine = false;
	bool bBoldNext = false, bUnderLineNext = false;
	bool bBoldLast = false, bUnderLineLast = false;
	bool bBoldLast2 = false, bUnderLineLast2 = false;
	CColorStrategyPool* pool = CColorStrategyPool::getInstance();
	pool->SetCurrentView(m_pCommanderView);
	for(CLogicInt nLineNum = sSelectLogic.GetFrom().y; nLineNum <= nLineNumLast; nLineNum++, pcDocLine = pcDocLine->GetNextLine()){
		if( pcDocLine == NULL ){
			break;
		}
		pool->NotifyOnStartScanLogic();
		CColorStrategy* pStrategyNormal = NULL;
		CColorStrategy* pStrategyFound = NULL;
		CColorStrategy* pStrategy = NULL;
		CColorStrategy*	pStrategyLast = (CColorStrategy*)-1;
		CStringRef cStringLine(pcDocLine->GetPtr(), pcDocLine->GetLengthWithEOL());
		{
			pStrategy = pStrategyNormal = pool->GetStrategyByColor(pcLayout->GetColorTypePrev());
			if( pStrategy ){
				pStrategy->InitStrategyStatus();
			}
			int nColorIdx = ToColorInfoArrIndex(pcLayout->GetColorTypePrev());
			if (-1 != nColorIdx) {
				const ColorInfo& info = type.m_ColorInfoArr[nColorIdx];
				bBold      = info.m_bBoldFont;
				bUnderLine = info.m_bUnderLine;
				cTEXT      = info.m_colTEXT;
				cBACK      = info.m_colBACK;
			}
		}
		const WCHAR* pLine = pcDocLine->GetPtr();
		for(;pcLayout->GetLogicLineNo() == nLineNum; nLayoutLineNum++, pcLayout = pcLayout->GetNextLayout() )
		{
			CLogicInt nIdxFrom;
			CLogicInt nIdxTo;
			const int nLineLen = pcLayout->GetLengthWithoutEOL() + pcLayout->GetLayoutEol().GetLen();
			if( nLayoutLineNum < rcSel.top ){
				nIdxTo = nIdxFrom = CLogicInt(-1);
			}else{
				if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
					nIdxFrom = m_pCommanderView->LineColumnToIndex(pcLayout, rcSel.left );
					nIdxTo   = m_pCommanderView->LineColumnToIndex(pcLayout, rcSel.right);
					// ���s�͏���
					if( nIdxTo - nIdxFrom > 0 ){
						const WCHAR* pLine = pcLayout->GetPtr();
						if( pLine[nIdxTo - 1] == L'\n' || pLine[nIdxTo - 1] == L'\r' ){
							--nIdxTo;
						}
					}
				}else{
					if( nLayoutLineNum == rcSel.top ){
						nIdxFrom = sSelectLogic.GetFrom().x;
					}else{
						nIdxFrom = CLogicInt(0);
					}
					if( nLayoutLineNum == rcSel.bottom ){
						nIdxTo = sSelectLogic.GetTo().x;
					}else{
						nIdxTo = nLineLen;
					}
				}
			}
			// �Ō�̉��s�̎��̍s�ԍ���\�����Ȃ��悤��
			if( nIdxTo == 0 && nLayoutLineNum == rcSel.bottom ){
				break;
			}
			if( bLineNumber ){
				WCHAR szLineNum[14];
				if( type.m_bLineNumIsCRLF ){
					if( pcLayout->GetLogicOffset() != 0 ){
						if( bLineNumLayout ){
							cmemClip.AppendNativeData(cmemNullLine);
						}
					}else{
						int ret = swprintf(szLineNum, szLineFormat, nLineNum + 1);
						cmemClip.AppendString(szLineNum, ret);
					}
				}else{
					if( bLineNumLayout || pcLayout->GetLogicOffset() == 0 ){
						int ret = swprintf(szLineNum, szLineFormat, nLayoutLineNum + 1);
						cmemClip.AppendString(szLineNum, ret);
					}
				}
			}
			const int nLineStart = pcLayout->GetLogicOffset();
			int nBgnLogic = nIdxFrom + nLineStart;
			int iLogic = nLineStart;
			bool bAddCRLF = false;
			for( ; iLogic < nLineStart + nLineLen; ++iLogic ){
				pStrategy = GetColorStrategyHTML(cStringLine, iLogic, pool, &pStrategyNormal, &pStrategyFound);
				if( pStrategy != pStrategyLast ){
					int nColorIdx = ToColorInfoArrIndex( pStrategy ? pStrategy->GetStrategyColor() : COLORIDX_TEXT );
					if (-1 != nColorIdx) {
						const ColorInfo& info = type.m_ColorInfoArr[nColorIdx];
						cTEXTNext      = info.m_colTEXT;
						cBACKNext      = info.m_colBACK;
						bBoldNext      = info.m_bBoldFont;
						bUnderLineNext = info.m_bUnderLine;
					}
					pStrategyLast = pStrategy;
				}
				if( nIdxFrom != -1 && nIdxFrom + nLineStart <= iLogic && iLogic <= nIdxTo + nLineStart ){
					if( nIdxFrom + nLineStart == iLogic ){
						cTEXTLast      = cTEXTNext;
						cBACKLast      = cBACKNext;
						bBoldLast      = bBoldNext;
						bUnderLineLast = bUnderLineNext;
					}else if( nIdxFrom + nLineStart < iLogic
					  && (bBold != bBoldNext || bUnderLine != bUnderLineNext
					  || cTEXT != cTEXTNext || cBACK != cBACKNext) ){
						bAddCRLF = AppendHTMLColor(cTEXTLast, cTEXTLast2, cBACKLast, cBACKLast2, bBoldLast, bBoldLast2,
							bUnderLineLast, bUnderLineLast2, pLine + nBgnLogic, iLogic - nBgnLogic, cmemClip);
						cTEXTLast      = cTEXTNext;
						cBACKLast      = cBACKNext;
						bBoldLast      = bBoldNext;
						bUnderLineLast = bUnderLineNext;
						nBgnLogic = iLogic;
					}else if( nIdxTo + nLineStart == iLogic ){
						bAddCRLF = AppendHTMLColor(cTEXTLast, cTEXTLast2, cBACKLast, cBACKLast2, bBoldLast, bBoldLast2,
							bUnderLineLast, bUnderLineLast2, pLine + nBgnLogic, iLogic - nBgnLogic, cmemClip);
						nBgnLogic = iLogic;
					}
				}
				cTEXT = cTEXTNext;
				cBACK = cBACKNext;
				bBold = bBoldNext;
				bUnderLine = bUnderLineNext;
			}
			if( nIdxFrom != -1 && nIdxTo + nLineStart == iLogic ){
				bAddCRLF = AppendHTMLColor(cTEXTLast, cTEXTLast2, cBACKLast, cBACKLast2, bBoldLast, bBoldLast2,
					bUnderLineLast, bUnderLineLast2, pLine + nBgnLogic, iLogic - nBgnLogic, cmemClip);
			}
			if( bLineNumber ){
				bool bAddLineNum = true;
				const CLayout* pcLayoutNext = pcLayout->GetNextLayout();
				if( pcLayoutNext ){
					if( type.m_bLineNumIsCRLF ){
						if( bLineNumLayout && pcLayoutNext->GetLogicOffset() != 0 ){
							bAddLineNum = true;
						}else{
							bAddLineNum = true;
						}
					}else{
						if( bLineNumLayout || pcLayoutNext->GetLogicOffset() == 0 ){
							bAddLineNum = true;
						}
					}
				}
				if( bAddLineNum ){
					if( bBoldLast2 ){
						cmemClip.AppendString(L"</b>", 4);
					}
					if( bUnderLineLast2 ){
						cmemClip.AppendString(L"</u>", 4);
					}
					if( cTEXTLast2 != (COLORREF)-1 ){
						cmemClip.AppendString(L"</span>", 7);
					}
					bBoldLast = bBoldLast2 = false;
					bUnderLineLast = bUnderLineLast2 = false;
					cTEXTLast = cTEXTLast2 = (COLORREF)-1;
					cBACKLast = cBACKLast2 = (COLORREF)-1;
				}
			}
			if( bLineNumLayout && !bAddCRLF ){
				cmemClip.AppendString(WCODE::CRLF, 2);
			}
		}
	}
	if( bBoldLast2 ){
		cmemClip.AppendString(L"</b>", 4);
	}
	if( bUnderLineLast2 ){
		cmemClip.AppendString(L"</u>", 4);
	}
	if( cTEXTLast2 != (COLORREF)-1 ){
		cmemClip.AppendString(L"</span>", 7);
	}
	cmemClip.AppendString(L"</pre>", 6);

	CClipboard cClipboard(GetDocument()->m_pcEditWnd->GetHwnd());
	if(!cClipboard){
		return;
	}
	cClipboard.Empty();
	cClipboard.SetHtmlText(cmemClip);
	cClipboard.SetText(cmemClip.GetStringPtr(), cmemClip.GetStringLength(), false, false);
}



CColorStrategy* CViewCommander::GetColorStrategyHTML(
	const CStringRef&	cStringLine,
	int					iLogic,
	const CColorStrategyPool*	pool,
	CColorStrategy**	ppStrategy,
	CColorStrategy**	ppStrategyFound		// [in,out]
)
{
	//�����F�I��
	if(*ppStrategyFound){
		if((*ppStrategyFound)->EndColor(cStringLine, iLogic)){
			*ppStrategyFound = NULL;
		}
	}

	//�����F�J�n
	if(!*ppStrategyFound){
		CColor_Found*  pcFound  = pool->GetFoundStrategy();
		if(pcFound->BeginColor(cStringLine, iLogic)){
			*ppStrategyFound = pcFound;
		}
	}

	//�F�I��
	if(*ppStrategy){
		if((*ppStrategy)->EndColor(cStringLine, iLogic)){
			*ppStrategy = NULL;
		}
	}

	//�F�J�n
	if(!*ppStrategy){
		int size = pool->GetStrategyCount();
		for(int i = 0; i < size; i++ ){
			if(pool->GetStrategy(i)->BeginColor(cStringLine, iLogic)){
				*ppStrategy = pool->GetStrategy(i);
				break;
			}
		}
	}
	if( *ppStrategyFound ){
		return *ppStrategyFound;
	}
	return *ppStrategy;
}

//!�I��͈͓��s�ԍ��F�t��HTML�R�s�[
void CViewCommander::Command_COPY_COLOR_HTML_LINENUMBER()
{
	Command_COPY_COLOR_HTML(true);
}



/*!	���ݕҏW���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[
	2002/2/3 aroka
*/
void CViewCommander::Command_COPYFILENAME( void )
{
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		const WCHAR* pszFile = to_wchar(GetDocument()->m_cDocFile.GetFileName());
		m_pCommanderView->MySetClipboardData( pszFile , wcslen( pszFile ), false );
	}
	else{
		ErrorBeep();
	}
}



/* ���ݕҏW���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[ */
void CViewCommander::Command_COPYPATH( void )
{
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		const TCHAR* szPath = GetDocument()->m_cDocFile.GetFilePath();
		m_pCommanderView->MySetClipboardData( szPath, _tcslen(szPath), false );
	}
	else{
		ErrorBeep();
	}
}



//	May 9, 2000 genta
/* ���ݕҏW���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���N���b�v�{�[�h�ɃR�s�[ */
void CViewCommander::Command_COPYTAG( void )
{
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		wchar_t	buf[ MAX_PATH + 20 ];

		CLogicPoint ptColLine;

		//	�_���s�ԍ��𓾂�
		GetDocument()->m_cLayoutMgr.LayoutToLogic( GetCaret().GetCaretLayoutPos(), &ptColLine );

		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		auto_sprintf( buf, L"%ts (%d,%d): ", GetDocument()->m_cDocFile.GetFilePath(), ptColLine.y+1, ptColLine.x+1 );
		m_pCommanderView->MySetClipboardData( buf, wcslen( buf ), false );
	}
	else{
		ErrorBeep();
	}
}



////�L�[���蓖�Ĉꗗ���R�s�[
	//Dec. 26, 2000 JEPRO //Jan. 24, 2001 JEPRO debug version (directed by genta)
void CViewCommander::Command_CREATEKEYBINDLIST( void )
{
	CNativeW		cMemKeyList;

	CKeyBind::CreateKeyBindList(
		G_AppInstance(),
		GetDllShareData().m_Common.m_sKeyBind.m_nKeyNameArrNum,
		GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr,
		cMemKeyList,
		&GetDocument()->m_cFuncLookup,	//	Oct. 31, 2001 genta �ǉ�
		FALSE	// 2007.02.22 ryoji �ǉ�
	);

	// Windows�N���b�v�{�[�h�ɃR�s�[
	//2004.02.17 Moca �֐���
	SetClipboardText( CEditWnd::getInstance()->m_cSplitterWnd.GetHwnd(), cMemKeyList.GetStringPtr(), cMemKeyList.GetStringLength() );
}
