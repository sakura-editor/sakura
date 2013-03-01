/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�ҏW�n ��{�`)�֐��Q

	2012/12/16	CViewCommander.cpp,CViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, genta
	Copyright (C) 2003, MIK, genta, �����, zenryaku, Moca, ryoji, naoh, KEITA, ���イ��
	Copyright (C) 2005, genta, D.S.Koba, ryoji
	Copyright (C) 2007, ryoji, kobake
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2010, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"
#include "view/CRuler.h"
#include "CWaitCursor.h"
#include "COpeBlk.h"/// 2002/2/3 aroka �ǉ�
#include "window/CEditWnd.h"/// 2002/2/3 aroka �ǉ�
#include "plugin/CJackManager.h"
#include "plugin/CSmartIndentIfObj.h"
#include "debug/CRunningTimer.h"


/* wchar_t1���̕�������� */
void CViewCommander::Command_WCHAR( wchar_t wcChar )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	CLogicInt		nPos;
	CLogicInt		nCharChars;
	CLogicInt		nIdxTo;
	CLayoutInt		nPosX;

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	if( m_pCommanderView->m_bHideMouse && 0 <= m_pCommanderView->m_nMousePouse ){
		m_pCommanderView->m_nMousePouse = -1;
		::SetCursor( NULL );
	}

	/* ���݈ʒu�Ƀf�[�^��}�� */
	nPosX = CLayoutInt(-1);
	CNativeW cmemDataW2;
	cmemDataW2 = wcChar;
	if( WCODE::IsLineDelimiter(wcChar) ){ 
		/* ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾 */
		CEol cWork = GetDocument()->m_cDocEditor.GetNewLineCode();
		cmemDataW2.SetString( cWork.GetValue2(), cWork.GetLen() );

		/* �e�L�X�g���I������Ă��邩 */
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			m_pCommanderView->DeleteData( TRUE );
		}
		if( GetDocument()->m_cDocType.GetDocumentAttribute().m_bAutoIndent ){	/* �I�[�g�C���f���g */
			const CLayout* pCLayout;
			const wchar_t*	pLine;
			CLogicInt		nLineLen;
			pLine = GetDocument()->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pCLayout );
			if( NULL != pCLayout ){
				const CDocLine* pcDocLine;
				pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( pCLayout->GetLogicLineNo() );
				pLine = pcDocLine->GetDocLineStrWithEOL( &nLineLen );
				if( NULL != pLine ){
					/*
					  �J�[�\���ʒu�ϊ�
					  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
					  ��
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					*/
					CLogicPoint ptXY;
					GetDocument()->m_cLayoutMgr.LayoutToLogic(
						GetCaret().GetCaretLayoutPos(),
						&ptXY
					);

					/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
					nIdxTo = m_pCommanderView->LineColmnToIndex( pcDocLine, GetCaret().GetCaretLayoutPos().GetX2() );
					for( nPos = CLogicInt(0); nPos < nLineLen && nPos < ptXY.GetX2(); ){
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );

						/* ���̑��̃C���f���g���� */
						if( 0 < nCharChars
						 && pLine[nPos] != L'\0'	// ���̑��̃C���f���g������ L'\0' �͊܂܂�Ȃ�	// 2009.02.04 ryoji L'\0'���C���f���g����Ă��܂����C��
						 && 0 < (int)wcslen( GetDocument()->m_cDocType.GetDocumentAttribute().m_szIndentChars )
						){
							wchar_t szCurrent[10];
							wmemcpy( szCurrent, &pLine[nPos], nCharChars );
							szCurrent[nCharChars] = L'\0';
							/* ���̑��̃C���f���g�Ώە��� */
							if( NULL != wcsstr(
								GetDocument()->m_cDocType.GetDocumentAttribute().m_szIndentChars,
								szCurrent
							) ){
								goto end_of_for;
							}
						}
						
						{
							bool bZenSpace=GetDocument()->m_cDocType.GetDocumentAttribute().m_bAutoIndent_ZENSPACE;
							if(nCharChars==1 && WCODE::IsIndentChar(pLine[nPos],bZenSpace))
							{
								//���֐i��
							}
							else break;
						}

end_of_for:;
						nPos += nCharChars;
					}
					if( nPos > 0 ){
						nPosX = m_pCommanderView->LineIndexToColmn( pcDocLine, nPos );
					}

					//�C���f���g�擾
					CNativeW cmemIndent;
					cmemIndent.SetString( pLine, nPos );

					//�C���f���g�t��
					cmemDataW2.AppendNativeData(cmemIndent);
				}
			}
		}
	}
	else{
		/* �e�L�X�g���I������Ă��邩 */
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			/* ��`�͈͑I�𒆂� */
			if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
				Command_INDENT( wcChar );
				return;
			}else{
				m_pCommanderView->DeleteData( TRUE );
			}
		}
		else{
			if( ! m_pCommanderView->IsInsMode() /* Oct. 2, 2005 genta */ ){
				DelCharForOverwrite(&wcChar, 1);	// �㏑���p�̈ꕶ���폜	// 2009.04.11 ryoji
			}
		}
	}

	//UNICODE�ɕϊ�����
	CNativeW cmemUnicode;
	cmemUnicode.SetNativeData(cmemDataW2);

	//�{���ɑ}������
	CLayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		cmemUnicode.GetStringPtr(),
		cmemUnicode.GetStringLength(),
		&ptLayoutNew,
		true
	);

	/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
	GetCaret().MoveCursor( ptLayoutNew, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	/* �X�}�[�g�C���f���g */
	ESmartIndentType nSIndentType = GetDocument()->m_cDocType.GetDocumentAttribute().m_eSmartIndent;
	switch( nSIndentType ){	/* �X�}�[�g�C���f���g��� */
	case SMARTINDENT_NONE:
		break;
	case SMARTINDENT_CPP:
		/* C/C++�X�}�[�g�C���f���g���� */
		m_pCommanderView->SmartIndent_CPP( wcChar );
		break;
	default:
		//�v���O�C�����猟������
		{
			CPlug::Array plugs;
			CJackManager::getInstance()->GetUsablePlug( PP_SMARTINDENT, nSIndentType, &plugs );

			if( plugs.size() > 0 ){
				assert_warning( 1 == plugs.size() );
				//�C���^�t�F�[�X�I�u�W�F�N�g����
				CWSHIfObj::List params;
				CSmartIndentIfObj* objIndent = new CSmartIndentIfObj( wcChar );	//�X�}�[�g�C���f���g�I�u�W�F�N�g
				objIndent->AddRef();
				params.push_back( objIndent );

				//�L�[���͂��A���h�D�o�b�t�@�ɔ��f
				m_pCommanderView->SetUndoBuffer();

				//�L�[���͂Ƃ͕ʂ̑���u���b�N�ɂ���i�������v���O�C�����̑���͂܂Ƃ߂�j
				SetOpeBlk(new COpeBlk);
				GetOpeBlk()->AddRef();	// ��Release��HandleCommand�̍Ō�ōs��

				//�v���O�C���Ăяo��
				( *plugs.begin() )->Invoke( m_pCommanderView, params );
				objIndent->Release();
			}
		}
		break;
	}

	/* 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜 */
	if( WCODE::CR == wcChar && GetDocument()->m_cDocType.GetDocumentAttribute().m_bRTrimPrevLine ){	/* ���s���ɖ����̋󔒂��폜 */
		/* �O�̍s�ɂ��閖���̋󔒂��폜���� */
		m_pCommanderView->RTrimPrevLine();
	}

	m_pCommanderView->PostprocessCommand_hokan();	//	Jan. 10, 2005 genta �֐���
}



/*!
	@brief 2�o�C�g��������
	
	WM_IME_CHAR�ő����Ă�����������������D
	�������C�}�����[�h�ł�WM_IME_CHAR�ł͂Ȃ�WM_IME_COMPOSITION�ŕ������
	�擾����̂ł����ɂ͗��Ȃ��D

	@param wChar [in] SJIS�����R�[�h�D��ʂ�1�o�C�g�ځC���ʂ�2�o�C�g�ځD
	
	@date 2002.10.06 genta �����̏㉺�o�C�g�̈Ӗ����t�]�D
		WM_IME_CHAR��wParam�ɍ��킹���D
*/
void CViewCommander::Command_IME_CHAR( WORD wChar )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	CMemory			cmemData;

	//	Oct. 6 ,2002 genta �㉺�t�]
	if( 0 == (wChar & 0xff00) ){
		Command_WCHAR( wChar & 0xff );
		return;
	}
	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

 	if( m_pCommanderView->m_bHideMouse && 0 <= m_pCommanderView->m_nMousePouse ){
		m_pCommanderView->m_nMousePouse = -1;
		::SetCursor( NULL );
	}

	// Oct. 6 ,2002 genta �o�b�t�@�Ɋi�[����
	// Aug. 15, 2007 kobake WCHAR�o�b�t�@�ɕϊ�����
#ifdef _UNICODE
	wchar_t szWord[2]={wChar,0};
#else
	ACHAR szAnsiWord[3]={(wChar >> 8) & 0xff, wChar & 0xff, 0};
	const wchar_t* pUniData = to_wchar(szAnsiWord);
	wchar_t szWord[2]={pUniData[0],0};
#endif
	CLogicInt nWord=CLogicInt(1);

	/* �e�L�X�g���I������Ă��邩 */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* ��`�͈͑I�𒆂� */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			Command_INDENT( szWord, nWord );	//	Oct. 6 ,2002 genta 
			return;
		}else{
			m_pCommanderView->DeleteData( TRUE );
		}
	}
	else{
		if( ! m_pCommanderView->IsInsMode() /* Oct. 2, 2005 genta */ ){
			DelCharForOverwrite(szWord, nWord);	// �㏑���p�̈ꕶ���폜	// 2009.04.11 ryoji
		}
	}

	//	Oct. 6 ,2002 genta 
	CLayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView( GetCaret().GetCaretLayoutPos(), szWord, nWord, &ptLayoutNew, true );

	/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
	GetCaret().MoveCursor( ptLayoutNew, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	m_pCommanderView->PostprocessCommand_hokan();	//	Jan. 10, 2005 genta �֐���
}



//	from CViewCommander_New.cpp
/* Undo ���ɖ߂� */
void CViewCommander::Command_UNDO( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	if( !GetDocument()->m_cDocEditor.IsEnableUndo() ){	/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
		return;
	}

	MY_RUNNINGTIMER( cRunningTimer, "CViewCommander::Command_UNDO()" );

	COpe*		pcOpe = NULL;

	COpeBlk*	pcOpeBlk;
	int			nOpeBlkNum;
	int			i;
	bool		bIsModified;
//	int			nNewLine;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int			nNewPos;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	CLayoutPoint ptCaretPos_Before;

	CLayoutPoint ptCaretPos_After;

	/* �e�탂�[�h�̎����� */
	Command_CANCEL_MODE();

	m_pCommanderView->m_bDoing_UndoRedo = TRUE;	/* �A���h�D�E���h�D�̎��s���� */

	/* ���݂�Undo�Ώۂ̑���u���b�N��Ԃ� */
	if( NULL != ( pcOpeBlk = GetDocument()->m_cDocEditor.m_cOpeBuf.DoUndo( &bIsModified ) ) ){
		m_pCommanderView->SetDrawSwitch(false);	//	hor
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = nOpeBlkNum - 1; i >= 0; i-- ){
			pcOpe = pcOpeBlk->GetOpe( i );
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_After,
				&ptCaretPos_After
			);
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before,
				&ptCaretPos_Before
			);


			/* �J�[�\�����ړ� */
			GetCaret().MoveCursor( ptCaretPos_After, false );

			switch( pcOpe->GetCode() ){
			case OPE_INSERT:
				{
					CInsertOpe* pcInsertOpe = static_cast<CInsertOpe*>(pcOpe);

					/* �I��͈͂̕ύX */
					m_pCommanderView->GetSelectionInfo().m_sSelectBgn.SetFrom(ptCaretPos_Before);
					m_pCommanderView->GetSelectionInfo().m_sSelectBgn.SetTo(m_pCommanderView->GetSelectionInfo().m_sSelectBgn.GetFrom());
					m_pCommanderView->GetSelectionInfo().m_sSelect.SetFrom(ptCaretPos_Before);
					m_pCommanderView->GetSelectionInfo().m_sSelect.SetTo(ptCaretPos_After);

					/* �f�[�^�u�� �폜&�}���ɂ��g���� */
					m_pCommanderView->ReplaceData_CEditView(
						m_pCommanderView->GetSelectionInfo().m_sSelect,				// �폜�͈�
						&pcInsertOpe->m_pcmemData,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
						L"",						// �}������f�[�^
						CLogicInt(0),				// �}������f�[�^�̒���
						false,						// �ĕ`�悷�邩�ۂ�
						m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
					);

					/* �I��͈͂̕ύX */
					m_pCommanderView->GetSelectionInfo().m_sSelectBgn.Clear(-1); //�͈͑I��(���_)
					m_pCommanderView->GetSelectionInfo().m_sSelect.Clear(-1);
				}
				break;
			case OPE_DELETE:
				{
					CDeleteOpe* pcDeleteOpe = static_cast<CDeleteOpe*>(pcOpe);

					//2007.10.17 kobake ���������[�N���Ă܂����B�C���B
					if( 0 < pcDeleteOpe->m_pcmemData.GetStringLength() ){
						/* �f�[�^�u�� �폜&�}���ɂ��g���� */
						CLayoutRange sRange;
						sRange.Set(ptCaretPos_Before);
						m_pCommanderView->ReplaceData_CEditView(
							sRange,
							NULL,										/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
							pcDeleteOpe->m_pcmemData.GetStringPtr(),	/* �}������f�[�^ */
							pcDeleteOpe->m_nDataLen,					/* �}������f�[�^�̒��� */
							false,										/*�ĕ`�悷�邩�ۂ�*/
							m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
						);
					}
					pcDeleteOpe->m_pcmemData.Clear();
				}
				break;
			case OPE_MOVECARET:
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_After, false );
				break;
			}

			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before,
				&ptCaretPos_Before
			);
			if( i == 0 ){
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_Before, true );
			}else{
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_Before, false );
			}
		}
		m_pCommanderView->SetDrawSwitch(true);	//	hor
		m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji

		/* Undo��̕ύX�t���O */
		GetDocument()->m_cDocEditor.SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_pCommanderView->m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

		m_pCommanderView->SetBracketPairPos( true );	// 03/03/07 ai

		/* �ĕ`�� */
		// ���[���[�ĕ`��̕K�v������Ƃ��� DispRuler() �ł͂Ȃ����̕����Ɠ����� Call_OnPaint() �ŕ`�悷��	// 2010.08.20 ryoji
		// �EDispRuler() �̓��[���[�ƃe�L�X�g�̌��ԁi�����͍s�ԍ��̕��ɍ��킹���сj��`�悵�Ă���Ȃ�
		// �E�s�ԍ��\���ɕK�v�ȕ��� OPE_INSERT/OPE_DELETE �������ōX�V����Ă���ύX������΃��[���[�ĕ`��t���O�ɔ��f����Ă���
		// �E�����X�N���[�������[���[�ĕ`��t���O�ɔ��f����Ă���
		const bool bRedrawRuler = m_pCommanderView->GetRuler().GetRedrawFlag();
		m_pCommanderView->Call_OnPaint( PAINT_LINENUMBER | PAINT_BODY | (bRedrawRuler? PAINT_RULER: 0), false );
		if( !bRedrawRuler ){
			// ���[���[�̃L�����b�g�݂̂��ĕ`��
			HDC hdc = m_pCommanderView->GetDC();
			m_pCommanderView->GetRuler().DispRuler( hdc );
			m_pCommanderView->ReleaseDC( hdc );
		}

		GetCaret().ShowCaretPosInfo();	// �L�����b�g�̍s���ʒu��\������	// 2007.10.19 ryoji

		if( !GetEditWindow()->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
			GetEditWindow()->RedrawAllViews( m_pCommanderView );	//	���̃y�C���̕\�����X�V

	}

	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;	// 2007.10.11 ryoji �ǉ�
	m_pCommanderView->m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

	return;
}



//	from CViewCommander_New.cpp
/* Redo ��蒼�� */
void CViewCommander::Command_REDO( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}


	if( !GetDocument()->m_cDocEditor.IsEnableRedo() ){	/* Redo(��蒼��)�\�ȏ�Ԃ��H */
		return;
	}
	MY_RUNNINGTIMER( cRunningTimer, "CViewCommander::Command_REDO()" );

	COpe*		pcOpe = NULL;
	COpeBlk*	pcOpeBlk;
	int			nOpeBlkNum;
	int			i;
//	int			nNewLine;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int			nNewPos;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	bool		bIsModified;
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	CLayoutPoint ptCaretPos_Before;
	CLayoutPoint ptCaretPos_To;
	CLayoutPoint ptCaretPos_After;


	/* �e�탂�[�h�̎����� */
	Command_CANCEL_MODE();

	m_pCommanderView->m_bDoing_UndoRedo = TRUE;	/* �A���h�D�E���h�D�̎��s���� */

	/* ���݂�Redo�Ώۂ̑���u���b�N��Ԃ� */
	if( NULL != ( pcOpeBlk = GetDocument()->m_cDocEditor.m_cOpeBuf.DoRedo( &bIsModified ) ) ){
		m_pCommanderView->SetDrawSwitch(false);	// 2007.07.22 ryoji
		nOpeBlkNum = pcOpeBlk->GetNum();
		for( i = 0; i < nOpeBlkNum; ++i ){
			pcOpe = pcOpeBlk->GetOpe( i );
			/*
			  �J�[�\���ʒu�ϊ�
			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_Before,
				&ptCaretPos_Before
			);

			if( i == 0 ){
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_Before, true );
			}else{
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_Before, false );
			}
			switch( pcOpe->GetCode() ){
			case OPE_INSERT:
				{
					CInsertOpe* pcInsertOpe = static_cast<CInsertOpe*>(pcOpe);

					//2007.10.17 kobake ���������[�N���Ă܂����B�C���B
					if( 0 < pcInsertOpe->m_pcmemData.GetStringLength() ){
						/* �f�[�^�u�� �폜&�}���ɂ��g���� */
						CLayoutRange sRange;
						sRange.Set(ptCaretPos_Before);
						m_pCommanderView->ReplaceData_CEditView(
							sRange,
							NULL,										/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
							pcInsertOpe->m_pcmemData.GetStringPtr(),	/* �}������f�[�^ */
							pcInsertOpe->m_pcmemData.GetStringLength(),	/* �}������f�[�^�̒��� */
							false,										/*�ĕ`�悷�邩�ۂ�*/
							m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
						);

					}
					pcInsertOpe->m_pcmemData.Clear();
				}
				break;
			case OPE_DELETE:
				{
					CDeleteOpe* pcDeleteOpe = static_cast<CDeleteOpe*>(pcOpe);

					GetDocument()->m_cLayoutMgr.LogicToLayout(
						pcDeleteOpe->m_ptCaretPos_PHY_To,
						&ptCaretPos_To
					);

					/* �f�[�^�u�� �폜&�}���ɂ��g���� */
					m_pCommanderView->ReplaceData_CEditView(
						CLayoutRange(ptCaretPos_Before,ptCaretPos_To),
						&pcDeleteOpe->m_pcmemData,	/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
						L"",						/* �}������f�[�^ */
						CLogicInt(0),				/* �}������f�[�^�̒��� */
						false,
						m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
					);
				}
				break;
			case OPE_MOVECARET:
				break;
			}
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				pcOpe->m_ptCaretPos_PHY_After,
				&ptCaretPos_After
			);

			if( i == nOpeBlkNum - 1	){
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_After, true );
			}else{
				/* �J�[�\�����ړ� */
				GetCaret().MoveCursor( ptCaretPos_After, false );
			}
		}
		m_pCommanderView->SetDrawSwitch(true); // 2007.07.22 ryoji
		m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji

		/* Redo��̕ύX�t���O */
		GetDocument()->m_cDocEditor.SetModified(bIsModified,true);	//	Jan. 22, 2002 genta

		m_pCommanderView->m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

		m_pCommanderView->SetBracketPairPos( true );	// 03/03/07 ai

		/* �ĕ`�� */
		// ���[���[�ĕ`��̕K�v������Ƃ��� DispRuler() �ł͂Ȃ����̕����Ɠ����� Call_OnPaint() �ŕ`�悷��	// 2010.08.20 ryoji
		// �EDispRuler() �̓��[���[�ƃe�L�X�g�̌��ԁi�����͍s�ԍ��̕��ɍ��킹���сj��`�悵�Ă���Ȃ�
		// �E�s�ԍ��\���ɕK�v�ȕ��� OPE_INSERT/OPE_DELETE �������ōX�V����Ă���ύX������΃��[���[�ĕ`��t���O�ɔ��f����Ă���
		// �E�����X�N���[�������[���[�ĕ`��t���O�ɔ��f����Ă���
		const bool bRedrawRuler = m_pCommanderView->GetRuler().GetRedrawFlag();
		m_pCommanderView->Call_OnPaint( PAINT_LINENUMBER | PAINT_BODY | (bRedrawRuler? PAINT_RULER: 0), false );
		if( !bRedrawRuler ){
			// ���[���[�̃L�����b�g�݂̂��ĕ`��
			HDC hdc = m_pCommanderView->GetDC();
			m_pCommanderView->GetRuler().DispRuler( hdc );
			m_pCommanderView->ReleaseDC( hdc );
		}

		GetCaret().ShowCaretPosInfo();	// �L�����b�g�̍s���ʒu��\������	// 2007.10.19 ryoji

		if( !GetEditWindow()->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
			GetEditWindow()->RedrawAllViews( m_pCommanderView );	//	���̃y�C���̕\�����X�V
	}

	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;	// 2007.10.11 ryoji �ǉ�
	m_pCommanderView->m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */

	return;
}



//�J�[�\���ʒu�܂��͑I���G���A���폜
void CViewCommander::Command_DELETE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){		/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		// 2008.08.03 nasukoji	�I��͈͂Ȃ���DELETE�����s�����ꍇ�A�J�[�\���ʒu�܂Ŕ��p�X�y�[�X��}����������s���폜���Ď��s�ƘA������
		if( GetDocument()->m_cLayoutMgr.GetLineCount() > GetCaret().GetCaretLayoutPos().GetY2() ){
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
			if( pcLayout ){
				CLayoutInt nLineLen;
				CLogicInt nIndex;
				nIndex = m_pCommanderView->LineColmnToIndex2( pcLayout, GetCaret().GetCaretLayoutPos().GetX2(), &nLineLen );
				if( nLineLen != 0 ){	// �܂�Ԃ�����s�R�[�h���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\������������
					if( EOL_NONE != pcLayout->GetLayoutEol().GetType() ){	// �s�I�[�͉��s�R�[�h��?
						Command_INSTEXT( TRUE, L"", CLogicInt(0), FALSE );	// �J�[�\���ʒu�܂Ŕ��p�X�y�[�X�}��
					}else{	// �s�I�[���܂�Ԃ�
						// �܂�Ԃ��s���ł̓X�y�[�X�}����A���̕������폜����	// 2009.02.19 ryoji

						// �t���[�J�[�\�����̐܂�Ԃ��z���ʒu�ł̍폜�͂ǂ�����̂��Ó����悭�킩��Ȃ���
						// ��t���[�J�[�\�����i���傤�ǃJ�[�\�����܂�Ԃ��ʒu�ɂ���j�ɂ͎��̍s�̐擪�������폜������

						if( nLineLen < GetCaret().GetCaretLayoutPos().GetX2() ){	// �܂�Ԃ��s���ƃJ�[�\���̊ԂɌ��Ԃ�����
							Command_INSTEXT( TRUE, L"", CLogicInt(0), FALSE );	// �J�[�\���ʒu�܂Ŕ��p�X�y�[�X�}��
							pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
							nIndex = m_pCommanderView->LineColmnToIndex2( pcLayout, GetCaret().GetCaretLayoutPos().GetX2(), &nLineLen );
						}
						if( nLineLen != 0 ){	// �i�X�y�[�X�}������j�܂�Ԃ��s���Ȃ玟�������폜���邽�߂Ɏ��s�̐擪�Ɉړ�����K�v������
							if( pcLayout->GetNextLayout() != NULL ){	// �ŏI�s���ł͂Ȃ�
								CLayoutPoint ptLay;
								CLogicPoint ptLog(pcLayout->GetLogicOffset() + nIndex, pcLayout->GetLogicLineNo());
								GetDocument()->m_cLayoutMgr.LogicToLayout( ptLog, &ptLay );
								GetCaret().MoveCursor( ptLay, TRUE );
								GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
							}
						}
					}
				}
			}
		}
	}
	m_pCommanderView->DeleteData( TRUE );
	return;
}



//�J�[�\���O���폜
void CViewCommander::Command_DELETE_BACK( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	//	May 29, 2004 genta ���ۂɍ폜���ꂽ�������Ȃ��Ƃ��̓t���O�����ĂȂ��悤��
	//GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){				/* �e�L�X�g���I������Ă��邩 */
		m_pCommanderView->DeleteData( TRUE );
	}
	else{
		CLayoutPoint	ptLayoutPos_Old = GetCaret().GetCaretLayoutPos();
		CLogicPoint		ptLogicPos_Old = GetCaret().GetCaretLogicPos();
		BOOL	bBool = Command_LEFT( FALSE, FALSE );
		if( bBool ){
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
			if( pcLayout ){
				CLayoutInt nLineLen;
				CLogicInt nIdx = m_pCommanderView->LineColmnToIndex2( pcLayout, GetCaret().GetCaretLayoutPos().GetX2(), &nLineLen );
				if( nLineLen == 0 ){	// �܂�Ԃ�����s�R�[�h���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\������������
					// �E����̈ړ��ł͐܂�Ԃ����������͍폜���邪���s�͍폜���Ȃ�
					// ������i���̍s�̍s������j�̈ړ��ł͉��s���폜����
					if( nIdx < pcLayout->GetLengthWithoutEOL() || GetCaret().GetCaretLayoutPos().GetY2() < ptLayoutPos_Old.GetY2() ){
						if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
							/* ����̒ǉ� */
							GetOpeBlk()->AppendOpe(
								new CMoveCaretOpe(
									ptLogicPos_Old,
									GetCaret().GetCaretLogicPos()
								)
							);
						}
						m_pCommanderView->DeleteData( TRUE );
					}
				}
			}
		}
	}
	m_pCommanderView->PostprocessCommand_hokan();	//	Jan. 10, 2005 genta �֐���
}



/* 	�㏑���p�̈ꕶ���폜	2009.04.11 ryoji */
void CViewCommander::DelCharForOverwrite( const wchar_t* pszInput, int nLen )
{
	bool bEol = false;
	BOOL bDelete = TRUE;
	const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	int nDelLen = CLogicInt(0);
	CLayoutInt nKetaDiff = CLayoutInt(0);
	CLayoutInt nKetaAfterIns = CLayoutInt(0);
	if( NULL != pcLayout ){
		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
		CLogicInt nIdxTo = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
		if( nIdxTo >= pcLayout->GetLengthWithoutEOL() ){
			bEol = true;	// ���݈ʒu�͉��s�܂��͐܂�Ԃ��Ȍ�
			if( pcLayout->GetLayoutEol() != EOL_NONE ){
				if( GetDllShareData().m_Common.m_sEdit.m_bNotOverWriteCRLF ){	/* ���s�͏㏑�����Ȃ� */
					/* ���݈ʒu�����s�Ȃ�΍폜���Ȃ� */
					bDelete = FALSE;
				}
			}
		}else{
			// �������ɍ��킹�ăX�y�[�X���l�߂�
			if( GetDllShareData().m_Common.m_sEdit.m_bOverWriteFixMode ){
				const CStringRef line = pcLayout->GetDocLineRef()->GetStringRefWithEOL();
				CLogicInt nPos = GetCaret().GetCaretLogicPos().GetX();
				if( line.At(nPos) != WCODE::TAB ){
					CLayoutInt nKetaBefore = CNativeW::GetKetaOfChar(line, nPos);
					CLayoutInt nKetaAfter = CNativeW::GetKetaOfChar(pszInput, nLen, 0);
					nKetaDiff = nKetaBefore - nKetaAfter;
					nPos += CNativeW::GetSizeOfChar(line.GetPtr(), line.GetLength(), nPos);
					nDelLen = 1;
					if( nKetaDiff < 0 && nPos < line.GetLength() ){
						wchar_t c = line.At(nPos);
						if( c != WCODE::TAB && !WCODE::IsLineDelimiter(c) ){
							nDelLen = 2;
							CLayoutInt nKetaBefore2 = CNativeW::GetKetaOfChar(line, nPos);
							nKetaAfterIns = nKetaBefore + nKetaBefore2 - nKetaAfter;
						}
					}
				}
			}
		}
	}
	if( bDelete ){
		/* �㏑�����[�h�Ȃ̂ŁA���݈ʒu�̕������P�������� */
		CLayoutPoint posBefore;
		if( bEol ){
			Command_DELETE();	//�s�����ł͍ĕ`�悪�K�v���s���Ȍ�̍폜����������
			posBefore = GetCaret().GetCaretLayoutPos();
		}else{
			// 1�����폜
			m_pCommanderView->DeleteData( FALSE );
			posBefore = GetCaret().GetCaretLayoutPos();
			for(int i = 1; i < nDelLen; i++){
				m_pCommanderView->DeleteData( FALSE );
			}
		}
		CNativeW tmp;
		for(CLayoutInt i = CLayoutInt(0); i < nKetaDiff; i++){
			tmp.AppendString(L" ");
		}
		for(CLayoutInt i = CLayoutInt(0); i < nKetaAfterIns; i++){
			tmp.AppendString(L" ");
		}
		if( 0 < tmp.GetStringLength() ){
			Command_INSTEXT(FALSE, tmp.GetStringPtr(), tmp.GetStringLength(), false, false);
			GetCaret().MoveCursor(posBefore, false);
		}
	}
}
