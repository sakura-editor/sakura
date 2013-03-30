/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�ҏW�n ���x�ȑ���(���P��/�s����))�֐��Q

	2012/12/17	CViewCommander.cpp,CViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta, �݂�
	Copyright (C) 2001, MIK, Stonee, Misaka, asa-o, novice, hor, YAZAKI
	Copyright (C) 2002, hor, YAZAKI, novice, genta, aroka, Azumaiya, minfu, MIK, oak, ���Ȃӂ�, Moca, ai
	Copyright (C) 2003, MIK, genta, �����, zenryaku, Moca, ryoji, naoh, KEITA, ���イ��
	Copyright (C) 2004, isearch, Moca, gis_dur, genta, crayonzen, fotomo, MIK, novice, �݂��΂�, Kazika
	Copyright (C) 2005, genta, novice, �����, MIK, Moca, D.S.Koba, aroka, ryoji, maru
	Copyright (C) 2006, genta, aroka, ryoji, �����, fon, yukihane, Moca
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji
	Copyright (C) 2010, ryoji
	Copyright (C) 2011, ryoji
	Copyright (C) 2012, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"
#include "uiparts/CWaitCursor.h"
#include "COpeBlk.h"/// 2002/2/3 aroka �ǉ�
#include "window/CEditWnd.h"/// 2002/2/3 aroka �ǉ�
#include "mem/CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "_os/COsVersionInfo.h"

using namespace std; // 2002/2/3 aroka to here

#ifndef FID_RECONVERT_VERSION  // 2002.04.10 minfu 
#define FID_RECONVERT_VERSION 0x10000000
#endif
#ifndef SCS_CAP_SETRECONVERTSTRING
#define SCS_CAP_SETRECONVERTSTRING 0x00000004
#define SCS_QUERYRECONVERTSTRING 0x00020000
#define SCS_SETRECONVERTSTRING 0x00010000
#endif


/* �C���f���g ver1 */
void CViewCommander::Command_INDENT( wchar_t wcChar, EIndentType eIndent )
{
	using namespace WCODE;

#if 1	// ���������c���ΑI�𕝃[�����ő�ɂ���i�]���݊������j�B�����Ă� Command_INDENT() ver0 ���K�؂ɓ��삷��悤�ɕύX���ꂽ�̂ŁA�폜���Ă����ɕs�s���ɂ͂Ȃ�Ȃ��B
	// From Here 2001.12.03 hor
	/* SPACEorTAB�C�����f���g�ŋ�`�I�������[���̎��͑I��͈͂��ő�ɂ��� */
	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if( INDENT_NONE != eIndent && m_pCommanderView->GetSelectionInfo().IsBoxSelecting() && GetSelect().GetFrom().x==GetSelect().GetTo().x ){
		GetSelect().SetToX( GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
		m_pCommanderView->RedrawAll();
		return;
	}
	// To Here 2001.12.03 hor
#endif
	Command_INDENT( &wcChar, CLogicInt(1), eIndent );
	return;
}



/* �C���f���g ver0 */
/*
	�I�����ꂽ�e�s�͈̔͂̒��O�ɁA�^����ꂽ������( pData )��}������B
	@param eIndent �C���f���g�̎��
*/
void CViewCommander::Command_INDENT( const wchar_t* const pData, const CLogicInt nDataLen, EIndentType eIndent )
{
	if( nDataLen <= 0 ) return;

	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );
	CLayoutRange sSelectOld;		//�͈͑I��
	CLayoutPoint ptInserted;		//�}����̑}���ʒu
	const struct {
		bool operator()( const wchar_t ch ) const
		{ return ch == WCODE::SPACE || ch == WCODE::TAB; }
	} IsIndentChar;
	struct SSoftTabData {
		SSoftTabData( CLayoutInt nTab ) : m_szTab(NULL), m_nTab((Int)nTab) {}
		~SSoftTabData() { delete []m_szTab; }
		operator const wchar_t* ()
		{
			if( !m_szTab ){
				m_szTab = new wchar_t[m_nTab];
				wmemset( m_szTab, WCODE::SPACE, m_nTab );
			}
			return m_szTab;
		}
		int Len( CLayoutInt nCol ) { return m_nTab - ((Int)nCol % m_nTab); }
		wchar_t* m_szTab;
		int m_nTab;
	} stabData( GetDocument()->m_cLayoutMgr.GetTabSpace() );

	const bool bSoftTab = ( eIndent == INDENT_TAB && GetDocument()->m_cDocType.GetDocumentAttribute().m_bInsSpace );
	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
		if( INDENT_NONE != eIndent && !bSoftTab ){
			// ����`�I���ł͂Ȃ��̂� Command_WCHAR ����Ăі߂������悤�Ȃ��Ƃ͂Ȃ�
			Command_WCHAR( pData[0] );	// 1��������
		}
		else{
			// ����`�I���ł͂Ȃ��̂ł����֗���͎̂��ۂɂ̓\�t�g�^�u�̂Ƃ�����
			if( bSoftTab && !m_pCommanderView->IsInsMode() ){
				DelCharForOverwrite(pData, nDataLen);
			}
			m_pCommanderView->InsertData_CEditView(
				GetCaret().GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(GetCaret().GetCaretLayoutPos().GetX2()),
				&ptInserted,
				true
			);
			GetCaret().MoveCursor( ptInserted, true );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
		return;
	}
	m_pCommanderView->SetDrawSwitch(false);	// 2002.01.25 hor
	/* ��`�͈͑I�𒆂� */
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
// 2012.10.31 Moca �㏑�����[�h�̂Ƃ��̑I��͈͍폜����߂�
#if 0
		// From Here 2001.12.03 hor
		/* �㏑���[�h�̂Ƃ��͑I��͈͍폜 */
		if( ! m_pCommanderView->IsInsMode() /* Oct. 2, 2005 genta */){
			sSelectOld = GetSelect();
			m_pCommanderView->DeleteData( false );
			GetSelect() = sSelectOld;
			m_pCommanderView->GetSelectionInfo().SetBoxSelect(true);
		}
		// To Here 2001.12.03 hor
#endif

		/* 2�_��Ίp�Ƃ����`�����߂� */
		CLayoutRange rcSel;
		TwoPointToRange(
			&rcSel,
			GetSelect().GetFrom(),	// �͈͑I���J�n
			GetSelect().GetTo()		// �͈͑I���I��
		);
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( false/*true 2002.01.25 hor*/ );

		/*
			�����𒼑O�ɑ}�����ꂽ�������A����ɂ�茳�̈ʒu����ǂꂾ�����ɂ��ꂽ���B
			����ɏ]����`�I��͈͂����ɂ��炷�B
		*/
		CLayoutInt minOffset( -1 );
		/*
			���S�p�����̍����̌������ɂ���
			(1) eIndent == INDENT_TAB �̂Ƃ�
				�I��͈͂��^�u���E�ɂ���Ƃ��Ƀ^�u����͂���ƁA�S�p�����̑O�����I��͈͂���
				�͂ݏo���Ă���s�Ƃ����łȂ��s�Ń^�u�̕����A1����ݒ肳�ꂽ�ő�܂łƑ傫���قȂ�A
				�ŏ��ɑI������Ă���������I��͈͓��ɂƂǂ߂Ă������Ƃ��ł��Ȃ��Ȃ�B
				�ŏ��͋�`�I��͈͓��ɂ��ꂢ�Ɏ��܂��Ă���s�ɂ̓^�u��}�������A������Ƃ����͂�
				�o���Ă���s�ɂ����^�u��}�����邱�ƂƂ��A����ł͂ǂ̍s�ɂ��^�u���}������Ȃ�
				�Ƃ킩�����Ƃ��͂�蒼���ă^�u��}������B
			(2) eIndent == INDENT_SPACE �̂Ƃ��i���]���݊��I�ȓ���j
				��1�őI�����Ă���ꍇ�̂ݑS�p�����̍���������������B
				�ŏ��͋�`�I��͈͓��ɂ��ꂢ�Ɏ��܂��Ă���s�ɂ̓X�y�[�X��}�������A������Ƃ����͂�
				�o���Ă���s�ɂ����X�y�[�X��}�����邱�ƂƂ��A����ł͂ǂ̍s�ɂ��X�y�[�X���}������Ȃ�
				�Ƃ킩�����Ƃ��͂�蒼���ăX�y�[�X��}������B
		*/
		bool alignFullWidthChar = eIndent == INDENT_TAB && 0 == rcSel.GetFrom().x % this->GetDocument()->m_cLayoutMgr.GetTabSpace();
#if 1	// ���������c���ΑI��1��SPACE�C���f���g�őS�p�����𑵂���@�\(2)���ǉ������B
		alignFullWidthChar = alignFullWidthChar || (eIndent == INDENT_SPACE && 1 == rcSel.GetTo().x - rcSel.GetFrom().x);
#endif
		for( bool insertionWasDone = false; ; alignFullWidthChar = false ) {
			minOffset = CLayoutInt( -1 );
			for( CLayoutInt nLineNum = rcSel.GetFrom().y; nLineNum <= rcSel.GetTo().y; ++nLineNum ){
				const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
				//	Nov. 6, 2002 genta NULL�`�F�b�N�ǉ�
				//	���ꂪ�Ȃ���EOF�s���܂ދ�`�I�𒆂̕�������͂ŗ�����
				CLogicInt nIdxFrom, nIdxTo;
				CLayoutInt xLayoutFrom, xLayoutTo;
				bool reachEndOfLayout = false;
				if( pcLayout ) {
					/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
					const struct {
						CLayoutInt keta;
						CLogicInt* outLogicX;
						CLayoutInt* outLayoutX;
					} sortedKetas[] = {
						{ rcSel.GetFrom().x, &nIdxFrom, &xLayoutFrom },
						{ rcSel.GetTo().x, &nIdxTo, &xLayoutTo },
						{ CLayoutInt(-1), 0, 0 }
					};
					CMemoryIterator it( pcLayout, this->GetDocument()->m_cLayoutMgr.GetTabSpace() );
					for( int i = 0; 0 <= sortedKetas[i].keta; ++i ) {
						for( ; ! it.end(); it.addDelta() ) {
							if( sortedKetas[i].keta == it.getColumn() ) {
								break;
							}
							it.scanNext();
							if( sortedKetas[i].keta < it.getColumn() + it.getColumnDelta() ) {
								break;
							}
						}
						*sortedKetas[i].outLogicX = it.getIndex();
						*sortedKetas[i].outLayoutX = it.getColumn();
					}
					reachEndOfLayout = it.end();
				}else{
					nIdxFrom = nIdxTo = CLogicInt(0);
					xLayoutFrom = xLayoutTo = CLayoutInt(0);
					reachEndOfLayout = true;
				}
				const bool emptyLine = ! pcLayout || 0 == pcLayout->GetLengthWithoutEOL();
				const bool selectionIsOutOfLine = reachEndOfLayout && (
					(pcLayout && pcLayout->GetLayoutEol() != EOL_NONE) ? xLayoutFrom == xLayoutTo : xLayoutTo < rcSel.GetFrom().x
				);

				// ���͕����̑}���ʒu
				const CLayoutPoint ptInsert( selectionIsOutOfLine ? rcSel.GetFrom().x : xLayoutFrom, nLineNum );

				/* TAB��X�y�[�X�C���f���g�̎� */
				if( INDENT_NONE != eIndent ) {
					if( emptyLine || selectionIsOutOfLine ) {
						continue; // �C���f���g�������C���f���g�Ώۂ����݂��Ȃ�����(���s�����̌����s)�ɑ}�����Ȃ��B
					}
					/*
						���͂��C���f���g�p�̕����̂Ƃ��A��������œ��͕�����}�����Ȃ����Ƃ�
						�C���f���g�𑵂��邱�Ƃ��ł���B
						http://sakura-editor.sourceforge.net/cgi-bin/cyclamen/cyclamen.cgi?log=dev&v=4103
					*/
					if( nIdxFrom == nIdxTo // ��`�I��͈͂̉E�[�܂łɔ͈͂̍��[�ɂ��镶���̖������܂܂�Ă��炸�A
						&& ! selectionIsOutOfLine && pcLayout && IsIndentChar( pcLayout->GetPtr()[nIdxFrom] ) // ���́A�����̊܂܂�Ă��Ȃ��������C���f���g�����ł���A
						&& rcSel.GetFrom().x < rcSel.GetTo().x // ��0��`�I���ł͂Ȃ�(<<�݊����ƃC���f���g�����}���̎g������̂��߂ɏ��O����)�Ƃ��B
					) {
						continue;
					}
					/*
						�S�p�����̍����̌�����
					*/
					if( alignFullWidthChar
						&& (ptInsert.x == rcSel.GetFrom().x || (pcLayout && IsIndentChar( pcLayout->GetPtr()[nIdxFrom] )))
					) {	// �����̍������͈͂ɂ҂�������܂��Ă���
						minOffset = CLayoutInt(0);
						continue;
					}
				}

				/* ���݈ʒu�Ƀf�[�^��}�� */
				m_pCommanderView->InsertData_CEditView(
					ptInsert,
					!bSoftTab? pData: stabData,
					!bSoftTab? nDataLen: stabData.Len(ptInsert.x),
					&ptInserted,
					false
				);
				insertionWasDone = true;
				minOffset = std::min(
					0 <= minOffset ? minOffset : this->GetDocument()->m_cLayoutMgr.GetMaxLineKetas(),
					ptInsert.x <= ptInserted.x ? ptInserted.x - ptInsert.x : std::max( CLayoutInt(0), this->GetDocument()->m_cLayoutMgr.GetMaxLineKetas() - ptInsert.x)
				);

				GetCaret().MoveCursor( ptInserted, false );
				GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			}
			if( insertionWasDone || !alignFullWidthChar ) {
				break; // ���[�v�̕K�v�͂Ȃ��B(1.�����̑}�����s��ꂽ����B2.�����ł͂Ȃ��������̑}�����T���������ł͂Ȃ�����)
			}
		}

		// �}�����ꂽ�����̕������I��͈͂����ɂ��炵�ArcSel�ɃZ�b�g����B
		if( 0 < minOffset ) {
			rcSel.GetFromPointer()->x = std::min( rcSel.GetFrom().x + minOffset, this->GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
			rcSel.GetToPointer()->x = std::min( rcSel.GetTo().x + minOffset, this->GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
		}

		/* �J�[�\�����ړ� */
		GetCaret().MoveCursor( rcSel.GetFrom(), true );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			/* ����̒ǉ� */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
					GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
				)
			);
		}
		GetSelect().SetFrom(rcSel.GetFrom());	//�͈͑I���J�n�ʒu
		GetSelect().SetTo(rcSel.GetTo());		//�͈͑I���I���ʒu
		m_pCommanderView->GetSelectionInfo().SetBoxSelect(true);
	}
	else if( GetSelect().IsLineOne() ){	// �ʏ�I��(1�s��)
		if( INDENT_NONE != eIndent && !bSoftTab ){
			// ����`�I���ł͂Ȃ��̂� Command_WCHAR ����Ăі߂������悤�Ȃ��Ƃ͂Ȃ�
			Command_WCHAR( pData[0] );	// 1��������
		}
		else{
			// ����`�I���ł͂Ȃ��̂ł����֗���͎̂��ۂɂ̓\�t�g�^�u�̂Ƃ�����
			m_pCommanderView->DeleteData( false );
			m_pCommanderView->InsertData_CEditView(
				GetCaret().GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(GetCaret().GetCaretLayoutPos().GetX2()),
				&ptInserted,
				false
			);
			GetCaret().MoveCursor( ptInserted, true );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
	}
	else{	// �ʏ�I��(�����s)
		sSelectOld.SetFrom(CLayoutPoint(CLayoutInt(0),GetSelect().GetFrom().y));
		sSelectOld.SetTo  (CLayoutPoint(CLayoutInt(0),GetSelect().GetTo().y  ));
		if( GetSelect().GetTo().x > 0 ){
			sSelectOld.GetToPointer()->y++;
		}

		// ���݂̑I��͈͂��I����Ԃɖ߂�
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( false );

		for( CLayoutInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().GetY2(); i++ ){
			CLayoutInt nLineCountPrev = GetDocument()->m_cLayoutMgr.GetLineCount();
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( i );
			if( NULL == pcLayout ||						//	�e�L�X�g������EOL�̍s�͖���
				pcLayout->GetLogicOffset() > 0 ||				//	�܂�Ԃ��s�͖���
				pcLayout->GetLengthWithoutEOL() == 0 ){	//	���s�݂̂̍s�͖�������B
				continue;
			}

			/* �J�[�\�����ړ� */
			GetCaret().MoveCursor( CLayoutPoint(CLayoutInt(0), i), false );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			/* ���݈ʒu�Ƀf�[�^��}�� */
			m_pCommanderView->InsertData_CEditView(
				CLayoutPoint(CLayoutInt(0),i),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(CLayoutInt(0)),
				&ptInserted,
				false
			);
			/* �J�[�\�����ړ� */
			GetCaret().MoveCursor( ptInserted, false );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			if ( nLineCountPrev != GetDocument()->m_cLayoutMgr.GetLineCount() ){
				//	�s�����ω�����!!
				sSelectOld.GetToPointer()->y += GetDocument()->m_cLayoutMgr.GetLineCount() - nLineCountPrev;
			}
		}

		GetSelect() = sSelectOld;

		// From Here 2001.12.03 hor
		GetCaret().MoveCursor( GetSelect().GetTo(), true );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
					GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
				)
			);
		}
		// To Here 2001.12.03 hor
	}
	/* �ĕ`�� */
	m_pCommanderView->SetDrawSwitch(true);	// 2002.01.25 hor
	m_pCommanderView->RedrawAll();			// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
	return;
}



/* �t�C���f���g */
void CViewCommander::Command_UNINDENT( wchar_t wcChar )
{
	//	Aug. 9, 2003 genta
	//	�I������Ă��Ȃ��ꍇ�ɋt�C���f���g�����ꍇ��
	//	���Ӄ��b�Z�[�W���o��
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		EIndentType eIndent;
		switch( wcChar ){
		case WCODE::TAB:
			eIndent = INDENT_TAB;	// ��[SPACE�̑}��]�I�v�V������ ON �Ȃ�\�t�g�^�u�ɂ���iWiki BugReport/66�j
			break;
		case WCODE::SPACE:
			eIndent = INDENT_SPACE;
			break;
		default:
			eIndent = INDENT_NONE;
		}
		Command_INDENT( wcChar, eIndent );
		m_pCommanderView->SendStatusMessage(_T("���t�C���f���g�͑I�����̂�"));
		return;
	}

	//�����v
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	CMemory		cmemBuf;

	/* ��`�͈͑I�𒆂� */
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		ErrorBeep();
//**********************************************
//	 ���^�t�C���f���g�ɂ��ẮA�ۗ��Ƃ��� (1998.10.22)
//**********************************************
	}
	else{
		GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

		CLayoutRange sSelectOld;	//�͈͑I��
		sSelectOld.SetFrom(CLayoutPoint(CLayoutInt(0),GetSelect().GetFrom().y));
		sSelectOld.SetTo  (CLayoutPoint(CLayoutInt(0),GetSelect().GetTo().y  ));
		if( GetSelect().GetTo().x > 0 ){
			sSelectOld.GetToPointer()->y++;
		}

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( false );

		CLogicInt		nDelLen;
		for( CLayoutInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().GetY2(); i++ ){
			CLayoutInt nLineCountPrev = GetDocument()->m_cLayoutMgr.GetLineCount();

			const CLayout*	pcLayout;
			CLogicInt		nLineLen;
			const wchar_t*	pLine = GetDocument()->m_cLayoutMgr.GetLineStr( i, &nLineLen, &pcLayout );
			if( NULL == pcLayout || pcLayout->GetLogicOffset() > 0 ){ //�܂�Ԃ��ȍ~�̍s�̓C���f���g�������s��Ȃ�
				continue;
			}

			if( WCODE::TAB == wcChar ){
				if( pLine[0] == wcChar ){
					nDelLen = CLogicInt(1);
				}
				else{
					//����锼�p�X�y�[�X�� (1�`�^�u����) -> nDelLen
					CLogicInt i;
					CLogicInt nTabSpaces = CLogicInt((Int)GetDocument()->m_cLayoutMgr.GetTabSpace());
					for( i = CLogicInt(0); i < nLineLen; i++ ){
						if( WCODE::SPACE != pLine[i] ){
							break;
						}
						//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
						if( i >= nTabSpaces ){
							break;
						}
					}
					if( 0 == i ){
						continue;
					}
					nDelLen = i;
				}
			}
			else{
				if( pLine[0] != wcChar ){
					continue;
				}
				nDelLen = CLogicInt(1);
			}

			/* �J�[�\�����ړ� */
			GetCaret().MoveCursor( CLayoutPoint(CLayoutInt(0), i), false );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			

			CNativeW	pcMemDeleted;
			/* �w��ʒu�̎w�蒷�f�[�^�폜 */
			m_pCommanderView->DeleteData2(
				CLayoutPoint(CLayoutInt(0),i),
				nDelLen,	// 2001.12.03 hor
				&pcMemDeleted
			);
			if ( nLineCountPrev != GetDocument()->m_cLayoutMgr.GetLineCount() ){
				//	�s�����ω�����!!
				sSelectOld.GetToPointer()->y += GetDocument()->m_cLayoutMgr.GetLineCount() - nLineCountPrev;
			}
		}
		GetSelect() = sSelectOld;	//�͈͑I��

		// From Here 2001.12.03 hor
		GetCaret().MoveCursor( GetSelect().GetTo(), true );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
					GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
				)
			);
		}
		// To Here 2001.12.03 hor
	}

	/* �ĕ`�� */
	m_pCommanderView->RedrawAll();	// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
}



//	from CViewCommander_New.cpp
/*! TRIM Step1
	��I�����̓J�����g�s��I������ m_pCommanderView->ConvSelectedArea �� ConvMemory ��
	@author hor
	@date 2001.12.03 hor �V�K�쐬
*/
void CViewCommander::Command_TRIM(
	BOOL bLeft	//!<  [in] FALSE: �ETRIM / ����ȊO: ��TRIM
)
{
	bool bBeDisableSelectArea = false;
	CViewSelect& cViewSelect = m_pCommanderView->GetSelectionInfo();

	if(!cViewSelect.IsTextSelected()){	//	��I�����͍s�I���ɕύX
		cViewSelect.m_sSelect.SetFrom(
			CLayoutPoint(
				CLayoutInt(0),
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		cViewSelect.m_sSelect.SetTo  (
			CLayoutPoint(
				GetDocument()->m_cLayoutMgr.GetMaxLineKetas(),
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		bBeDisableSelectArea = true;
	}

	if(bLeft){
		m_pCommanderView->ConvSelectedArea( F_LTRIM );
	}
	else{
		m_pCommanderView->ConvSelectedArea( F_RTRIM );
	}

	if(bBeDisableSelectArea)
		cViewSelect.DisableSelectArea( true );
}



//	from CViewCommander_New.cpp
/*!	�����s�̃\�[�g�Ɏg���\����*/
typedef struct _SORTTABLE {
	wstring sKey1;
	wstring sKey2;
} SORTDATA, *SORTTABLE;

/*!	�����s�̃\�[�g�Ɏg���֐�(����) */
bool SortByKeyAsc (SORTTABLE pst1, SORTTABLE pst2) {return (pst1->sKey1<pst2->sKey1);}

/*!	�����s�̃\�[�g�Ɏg���֐�(�~��) */
bool SortByKeyDesc(SORTTABLE pst1, SORTTABLE pst2) {return (pst1->sKey1>pst2->sKey1);}

/*!	@brief �����s�̃\�[�g

	��I�����͉������s���Ȃ��D��`�I�����́A���͈̔͂��L�[�ɂ��ĕ����s���\�[�g�D
	
	@note �Ƃ肠�������s�R�[�h���܂ރf�[�^���\�[�g���Ă���̂ŁA
	�t�@�C���̍ŏI�s�̓\�[�g�ΏۊO�ɂ��Ă��܂�
	@author hor
	@date 2001.12.03 hor �V�K�쐬
	@date 2001.12.21 hor �I��͈͂̒������W�b�N�����
*/
void CViewCommander::Command_SORT(BOOL bAsc)	//bAsc:TRUE=����,FALSE=�~��
{
	CLayoutRange sRangeA;
	CLogicRange sSelectOld;

	int			nColmFrom, nColmTo;
	CLayoutInt	nCF(0), nCT(0);
	CLayoutInt	nCaretPosYOLD;
	bool		bBeginBoxSelectOld;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int			j;
	CNativeW	cmemBuf;
	std::vector<SORTTABLE> sta;

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
		return;
	}

	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		sRangeA=m_pCommanderView->GetSelectionInfo().m_sSelect;
		if( m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().x==m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().x ){
			//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
			m_pCommanderView->GetSelectionInfo().m_sSelect.SetToX( GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
		}
		if(m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().x<m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().x){
			nCF=m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().GetX2();
			nCT=m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().GetX2();
		}else{
			nCF=m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().GetX2();
			nCT=m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().GetX2();
		}
	}
	bBeginBoxSelectOld=m_pCommanderView->GetSelectionInfo().IsBoxSelecting();
	nCaretPosYOLD=GetCaret().GetCaretLayoutPos().GetY();
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		m_pCommanderView->GetSelectionInfo().m_sSelect,
		&sSelectOld
	);

	if( bBeginBoxSelectOld ){
		sSelectOld.GetToPointer()->y++;
	}
	else{
		// �J�[�\���ʒu���s������Ȃ� �� �I��͈͂̏I�[�ɉ��s�R�[�h������ꍇ��
		// ���̍s���I��͈͂ɉ�����
		if ( sSelectOld.GetTo().x > 0 ) {
			// 2006.03.31 Moca nSelectLineToOld�́A�����s�Ȃ̂�Layout�n����DocLine�n�ɏC��
			const CDocLine* pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( sSelectOld.GetTo().GetY2() );
			if( NULL != pcDocLine && EOL_NONE != pcDocLine->GetEol() ){
				sSelectOld.GetToPointer()->y++;
			}
		}
	}
	sSelectOld.SetFromX(CLogicInt(0));
	sSelectOld.SetToX(CLogicInt(0));

	//�s�I������ĂȂ�
	if(sSelectOld.IsLineOne()){
		return;
	}

	for( CLogicInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().y; i++ ){
		const CDocLine* pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( i );
		pLine = GetDocument()->m_cDocLineMgr.GetLine(i)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ) continue;
		SORTTABLE pst = new SORTDATA;
		if( bBeginBoxSelectOld ){
			nColmFrom = m_pCommanderView->LineColmnToIndex( pcDocLine, nCF );
			nColmTo   = m_pCommanderView->LineColmnToIndex( pcDocLine, nCT );
			if(nColmTo<nLineLen){	// BOX�I��͈͂̉E�[���s���Ɏ��܂��Ă���ꍇ
				// 2006.03.31 genta std::string::assign���g���Ĉꎞ�ϐ��폜
				pst->sKey1.assign( &pLine[nColmFrom], nColmTo-nColmFrom );
			}
			else if(nColmFrom<nLineLen){	// BOX�I��͈͂̉E�[���s�����E�ɂ͂ݏo���Ă���ꍇ
				pst->sKey1=&pLine[nColmFrom];
			}
			pst->sKey2=pLine;
		}else{
			pst->sKey1=pLine;
		}
		sta.push_back(pst);
	}
	if(bAsc){
		std::stable_sort(sta.begin(), sta.end(), SortByKeyAsc);
	}else{
		std::stable_sort(sta.begin(), sta.end(), SortByKeyDesc);
	}
	cmemBuf.SetString(L"");
	j=(int)sta.size();
	if( bBeginBoxSelectOld ){
		for (int i=0; i<j; i++) cmemBuf.AppendString( sta[i]->sKey2.c_str() ); 
	}else{
		for (int i=0; i<j; i++) cmemBuf.AppendString( sta[i]->sKey1.c_str() );
	}

	//sta.clear(); �����ꂶ�Ⴞ�߂݂���
	for (int i=0; i<j; i++) delete sta[i];

	CLayoutRange sSelectOld_Layout;
	GetDocument()->m_cLayoutMgr.LogicToLayout(sSelectOld, &sSelectOld_Layout);
	m_pCommanderView->ReplaceData_CEditView(
		sSelectOld_Layout,
		NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		false,
		m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
	);

	//	�I���G���A�̕���
	if(bBeginBoxSelectOld){
		m_pCommanderView->GetSelectionInfo().SetBoxSelect(bBeginBoxSelectOld);
		m_pCommanderView->GetSelectionInfo().m_sSelect=sRangeA;
	}else{
		m_pCommanderView->GetSelectionInfo().m_sSelect=sSelectOld_Layout;
	}
	if(nCaretPosYOLD==m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y || m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ) {
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom(), true );
	}else{
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo(), true );
	}
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		m_pCommanderView->m_pcOpeBlk->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}
	m_pCommanderView->RedrawAll();
}



//	from CViewCommander_New.cpp
/*! @brief �����s�̃}�[�W

	�A�����镨���s�œ��e������̕���1�s�ɂ܂Ƃ߂܂��D
	
	��`�I�����͂Ȃɂ����s���܂���D
	
	@note ���s�R�[�h���܂ރf�[�^���r���Ă���̂ŁA
	�t�@�C���̍ŏI�s�̓\�[�g�ΏۊO�ɂ��Ă��܂�
	
	@author hor
	@date 2001.12.03 hor �V�K�쐬
	@date 2001.12.21 hor �I��͈͂̒������W�b�N�����
*/
void CViewCommander::Command_MERGE(void)
{
	CLayoutInt		nCaretPosYOLD;
	const wchar_t*	pLinew;
	CLogicInt		nLineLen;
	int			j;
	CNativeW	cmemBuf;

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
		return;
	}
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		return;
	}

	nCaretPosYOLD=GetCaret().GetCaretLayoutPos().GetY();
	CLogicRange sSelectOld; //�͈͑I��
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		m_pCommanderView->GetSelectionInfo().m_sSelect,
		&sSelectOld
	);

	// 2001.12.21 hor
	// �J�[�\���ʒu���s������Ȃ� �� �I��͈͂̏I�[�ɉ��s�R�[�h������ꍇ��
	// ���̍s���I��͈͂ɉ�����
	if ( sSelectOld.GetTo().x > 0 ) {
		const CLayout* pcLayout=GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo().GetY2()); //2007.10.09 kobake �P�ʍ��݃o�O�C��
		if( NULL != pcLayout && EOL_NONE != pcLayout->GetLayoutEol() ){
			sSelectOld.GetToPointer()->y++;
			//sSelectOld.GetTo().y++;
		}
	}

	sSelectOld.SetFromX(CLogicInt(0));
	sSelectOld.SetToX(CLogicInt(0));

	//�s�I������ĂȂ�
	if(sSelectOld.IsLineOne()){
		return;
	}

	pLinew=NULL;
	cmemBuf.SetString(L"");
	for( CLogicInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().y; i++ ){
		const wchar_t*	pLine = GetDocument()->m_cDocLineMgr.GetLine(i)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ) continue;
		if( NULL == pLinew || wcscmp(pLine,pLinew) ){
			cmemBuf.AppendString( pLine );
		}
		pLinew=pLine;
	}
	j=GetDocument()->m_cDocLineMgr.GetLineCount();

	CLayoutRange sSelectOld_Layout;
	GetDocument()->m_cLayoutMgr.LogicToLayout(sSelectOld, &sSelectOld_Layout);

	m_pCommanderView->ReplaceData_CEditView(
		sSelectOld_Layout,
		NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		false,
		m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
	);
	j-=GetDocument()->m_cDocLineMgr.GetLineCount();

	//	�I���G���A�̕���
	m_pCommanderView->GetSelectionInfo().m_sSelect=sSelectOld_Layout;
	m_pCommanderView->GetSelectionInfo().m_sSelect.GetToPointer()->y -= j;

	if(nCaretPosYOLD==m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom().y){
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom(), true );
	}else{
		GetCaret().MoveCursor( m_pCommanderView->GetSelectionInfo().m_sSelect.GetTo(), true );
	}
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		m_pCommanderView->m_pcOpeBlk->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}
	m_pCommanderView->RedrawAll();

	if(j){
		TopOkMessage( m_pCommanderView->GetHwnd(), _T("%d�s���}�[�W���܂����B"), j);
	}else{
		InfoMessage( m_pCommanderView->GetHwnd(), _T("�}�[�W�\�ȍs���݂���܂���ł����B") );
	}
}



//	from CViewCommander_New.cpp
/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09

	@date 2002.04.11 YAZAKI COsVersionInfo�̃J�v�Z���������܂��傤�B
	@date 2010.03.17 ATOK�p��SCS_SETRECONVERTSTRING => ATRECONVERTSTRING_SET�ɕύX
		2002.11.20 Stonee����̏��
*/
void CViewCommander::Command_Reconvert(void)
{
	const int ATRECONVERTSTRING_SET = 1;

	//�T�C�Y���擾
	int nSize = m_pCommanderView->SetReconvertStruct(NULL,UNICODE_BOOL);
	if( 0 == nSize )  // �T�C�Y�O�̎��͉������Ȃ�
		return ;

	bool bUseUnicodeATOK = false;
	//�o�[�W�����`�F�b�N
	if( !OsSupportReconvert() ){
		
		// MSIME���ǂ���
		HWND hWnd = ImmGetDefaultIMEWnd(m_pCommanderView->GetHwnd());
		if (SendMessage(hWnd, m_pCommanderView->m_uWM_MSIME_RECONVERTREQUEST, FID_RECONVERT_VERSION, 0)){
			SendMessage(hWnd, m_pCommanderView->m_uWM_MSIME_RECONVERTREQUEST, 0, (LPARAM)m_pCommanderView->GetHwnd());
			return ;
		}

		// ATOK���g���邩�ǂ���
		TCHAR sz[256];
		ImmGetDescription(GetKeyboardLayout(0),sz,_countof(sz)); //�����̎擾
		if ( (_tcsncmp(sz,_T("ATOK"),4) == 0) && (NULL != m_pCommanderView->m_AT_ImmSetReconvertString) ){
			bUseUnicodeATOK = true;
		}else{
			//�Ή�IME�Ȃ�
			return;
		}
	}else{
		//���݂�IME���Ή����Ă��邩�ǂ���
		//IME�̃v���p�e�B
		if ( !(ImmGetProperty(GetKeyboardLayout(0),IGP_SETCOMPSTR) & SCS_CAP_SETRECONVERTSTRING) ){
			//�Ή�IME�Ȃ�
			return ;
		}
	}

	//�T�C�Y�擾������
	if (!UNICODE_BOOL && bUseUnicodeATOK) {
		nSize = m_pCommanderView->SetReconvertStruct(NULL,UNICODE_BOOL || bUseUnicodeATOK);
		if( 0 == nSize )  // �T�C�Y�O�̎��͉������Ȃ�
			return ;
	}

	//IME�̃R���e�L�X�g�擾
	HIMC hIMC = ::ImmGetContext( m_pCommanderView->GetHwnd() );
	
	//�̈�m��
	PRECONVERTSTRING pReconv = (PRECONVERTSTRING)::HeapAlloc(
		GetProcessHeap(),
		HEAP_GENERATE_EXCEPTIONS,
		nSize
	);
	
	//�\���̐ݒ�
	// Size�̓o�b�t�@�m�ۑ����ݒ�
	pReconv->dwSize = nSize;
	pReconv->dwVersion = 0;
	m_pCommanderView->SetReconvertStruct( pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	//�ϊ��͈͂̒���
	if(bUseUnicodeATOK){
		(*m_pCommanderView->m_AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize, NULL,0);
	}

	//���������ϊ��͈͂�I������
	m_pCommanderView->SetSelectionFromReonvert(pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	//�ĕϊ����s
	if(bUseUnicodeATOK){
		(*m_pCommanderView->m_AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else{
		::ImmSetCompositionString(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize, NULL, 0);
	}

	//�̈���
	::HeapFree(GetProcessHeap(),0,(LPVOID)pReconv);
	::ImmReleaseContext( m_pCommanderView->GetHwnd(), hIMC);
}
