#include "stdafx.h"
#include "CEditView.h"
#include "parse/CWordParse.h"

const int STRNCMP_MAX = 100;	/* MAX�L�[���[�h���Fstrnicmp�������r�ő�l(CEditView::KeySearchCore) */	// 2006.04.10 fon

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! �L�[���[�h���������̑O������`�F�b�N�ƁA����

	@date 2006.04.10 fon OnTimer, CreatePopUpMenu_R���番��
*/
BOOL CEditView::KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc )
{
	CNativeW	cmemCurText;
	int			i;

	/* �L�[���[�h�w���v���g�p���邩�H */
	if( !m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyWordHelp )	/* �L�[���[�h�w���v�@�\���g�p���� */	// 2006.04.10 fon
		goto end_of_search;
	/* �t�H�[�J�X�����邩�H */
	if( !GetCaret().ExistCaretFocus() ) 
		goto end_of_search;
	/* �E�B���h�E���Ƀ}�E�X�J�[�\�������邩�H */
	GetCursorPos( po );
	GetWindowRect( GetHwnd(), rc );
	if( !PtInRect( rc, *po ) )
		goto end_of_search;
	switch(nID){
	case LID_SKH_ONTIMER:
		/* �E�R�����g�̂P�`�R�łȂ��ꍇ */
		if(!( m_bInMenuLoop == FALSE	&&			/* �P�D���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ� */
			0 != m_dwTipTimer			&&			/* �Q�D����Tip��\�����Ă��Ȃ� */
			300 < ::GetTickCount() - m_dwTipTimer	/* �R�D��莞�Ԉȏ�A�}�E�X���Œ肳��Ă��� */
		) )	goto end_of_search;
		break;
	case LID_SKH_POPUPMENU_R:
		if(!( m_bInMenuLoop == FALSE	//&&			/* �P�D���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ� */
		//	0 != m_dwTipTimer			&&			/* �Q�D����Tip��\�����Ă��Ȃ� */
		//	1000 < ::GetTickCount() - m_dwTipTimer	/* �R�D��莞�Ԉȏ�A�}�E�X���Œ肳��Ă��� */
		) )	goto end_of_search;
		break;
	default:
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, _T("��҂ɋ����ė~�����G���["),
		_T("CEditView::KeyWordHelpSearchDict\nnID=%d"), (int)nID );
	}
	/* �I��͈͂̃f�[�^���擾(�����s�I���̏ꍇ�͐擪�̍s�̂�) */
	if( GetSelectedData( &cmemCurText, TRUE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		wchar_t* pszWork = cmemCurText.GetStringPtr();
		int nWorkLength	= wcslen( pszWork );
		for( i = 0; i < nWorkLength; ++i ){
			if( pszWork[i] == L'\0' ||
				pszWork[i] == WCODE::CR ||
				pszWork[i] == WCODE::LF ){
				break;
			}
		}
		wchar_t* pszBuf = new wchar_t[i + 1];
		wmemcpy( pszBuf, pszWork, i );
		pszBuf[i] = L'\0';
		cmemCurText.SetString( pszBuf, i );
		delete [] pszBuf;
	}
	/* �L�����b�g�ʒu�̒P����擾���鏈�� */	// 2006.03.24 fon
	else if(GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord){
		if(!GetParser().GetCurrentWord(&cmemCurText))
			goto end_of_search;
	}
	else
		goto end_of_search;

	if( CNativeW::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) &&	/* ���Ɍ����ς݂� */
		(!m_cTipWnd.m_KeyWasHit) )								/* �Y������L�[���Ȃ����� */
		goto end_of_search;
	m_cTipWnd.m_cKey = cmemCurText;

	/* �������s */
	if( !KeySearchCore(&m_cTipWnd.m_cKey) )
		goto end_of_search;
	m_dwTipTimer = 0;		/* ����Tip��\�����Ă��� */
	m_poTipCurPos = *po;	/* ���݂̃}�E�X�J�[�\���ʒu */
	return TRUE;			/* �����܂ŗ��Ă���΃q�b�g�E���[�h */

	/* �L�[���[�h�w���v�\�������I�� */
	end_of_search:
	return FALSE;
}

/*! �L�[���[�h���������������C��

	@date 2006.04.10 fon KeyWordHelpSearchDict���番��
*/
BOOL CEditView::KeySearchCore( const CNativeW* pcmemCurText )
{
	CNativeW*	pcmemRefKey;
	int			nCmpLen = STRNCMP_MAX; // 2006.04.10 fon
	int			nLine; // 2006.04.10 fon


	CTypeConfig nTypeNo = m_pcEditDoc->m_cDocType.GetDocumentType();
	m_cTipWnd.m_cInfo.SetString( _T("") );	/* tooltip�o�b�t�@������ */
	/* 1�s�ڂɃL�[���[�h�\���̏ꍇ */
	if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpKeyDisp){	/* �L�[���[�h���\������ */	// 2006.04.10 fon
		m_cTipWnd.m_cInfo.AppendString( _T("[ ") );
		m_cTipWnd.m_cInfo.AppendString( pcmemCurText->GetStringT() );
		m_cTipWnd.m_cInfo.AppendString( _T(" ]") );
	}
	/* �r���܂ň�v���g���ꍇ */
	if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpPrefix)
		nCmpLen = wcslen( pcmemCurText->GetStringPtr() );	// 2006.04.10 fon
	m_cTipWnd.m_KeyWasHit = FALSE;
	for(int i=0;i<CDocTypeManager().GetTypeSetting(nTypeNo).m_nKeyHelpNum;i++){	//�ő吔�FMAX_KEYHELP_FILE
		if( CDocTypeManager().GetTypeSetting(nTypeNo).m_KeyHelpArr[i].m_bUse ){
			// 2006.04.10 fon (nCmpLen,pcmemRefKey,nSearchLine)������ǉ�
			CNativeW*	pcmemRefText;
			int nSearchResult=m_cDicMgr.CDicMgr::Search(
				pcmemCurText->GetStringPtr(),
				nCmpLen,
				&pcmemRefKey,
				&pcmemRefText,
				CDocTypeManager().GetTypeSetting(nTypeNo).m_KeyHelpArr[i].m_szPath,
				&nLine
			);
			if(nSearchResult){
				/* �Y������L�[������ */
				LPWSTR		pszWork;
				pszWork = pcmemRefText->GetStringPtr();
				/* �L���ɂȂ��Ă��鎫����S���Ȃ߂āA�q�b�g�̓s�x�����̌p������ */
				if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpAllSearch){	/* �q�b�g�������̎��������� */	// 2006.04.10 fon
					/* �o�b�t�@�ɑO�̃f�[�^���l�܂��Ă�����separator�}�� */
					if(m_cTipWnd.m_cInfo.GetStringLength() != 0)
						m_cTipWnd.m_cInfo.AppendString( _T("\n--------------------\n��") );
					else
						m_cTipWnd.m_cInfo.AppendString( _T("��") );	/* �擪�̏ꍇ */
					/* �����̃p�X�}�� */
					m_cTipWnd.m_cInfo.AppendString( CDocTypeManager().GetTypeSetting(nTypeNo).m_KeyHelpArr[i].m_szPath );
					m_cTipWnd.m_cInfo.AppendString( _T("\n") );
					/* �O����v�Ńq�b�g�����P���}�� */
					if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpPrefix){	/* �I��͈͂őO����v���� */
						m_cTipWnd.m_cInfo.AppendString( pcmemRefKey->GetStringT() );
						m_cTipWnd.m_cInfo.AppendString( _T(" >>\n") );
					}/* ���������u�Ӗ��v��}�� */
					m_cTipWnd.m_cInfo.AppendStringW( pszWork );
					delete pcmemRefText;
					delete pcmemRefKey;	// 2006.07.02 genta
					/* �^�O�W�����v�p�̏����c�� */
					if(!m_cTipWnd.m_KeyWasHit){
						m_cTipWnd.m_nSearchDict=i;	/* �������J���Ƃ��ŏ��Ƀq�b�g�����������J�� */
						m_cTipWnd.m_nSearchLine=nLine;
						m_cTipWnd.m_KeyWasHit = TRUE;
					}
				}
				else{	/* �ŏ��̃q�b�g���ڂ̂ݕԂ��ꍇ */
					/* �L�[���[�h�������Ă�����separator�}�� */
					if(m_cTipWnd.m_cInfo.GetStringLength() != 0)
						m_cTipWnd.m_cInfo.AppendString( _T("\n--------------------\n") );
					
					/* �O����v�Ńq�b�g�����P���}�� */
					if(m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseKeyHelpPrefix){	/* �I��͈͂őO����v���� */
						m_cTipWnd.m_cInfo.AppendString( pcmemRefKey->GetStringT() );
						m_cTipWnd.m_cInfo.AppendString( _T(" >>\n") );
					}
					
					/* ���������u�Ӗ��v��}�� */
					m_cTipWnd.m_cInfo.AppendStringW( pszWork );
					delete pcmemRefText;
					delete pcmemRefKey;	// 2006.07.02 genta
					/* �^�O�W�����v�p�̏����c�� */
					m_cTipWnd.m_nSearchDict=i;
					m_cTipWnd.m_nSearchLine=nLine;
					m_cTipWnd.m_KeyWasHit = TRUE;
					return TRUE;
				}
			}
		}
	}
	if(m_cTipWnd.m_KeyWasHit == TRUE){
			return TRUE;
	}
	/* �Y������L�[���Ȃ������ꍇ */
	return FALSE;
}

/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
void CEditView::GetCurrentTextForSearch( CNativeW& cmemCurText )
{

	int				i;
	wchar_t			szTopic[_MAX_PATH];
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLogicInt		nIdx;
	CLayoutRange	sRange;

	cmemCurText.SetString(L"");
	szTopic[0] = L'\0';
	if( GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* �I��͈͂̃f�[�^���擾 */
		if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
			/* ��������������݈ʒu�̒P��ŏ����� */
			wcsncpy( szTopic, cmemCurText.GetStringPtr(), _MAX_PATH - 1 );
			szTopic[_MAX_PATH - 1] = L'\0';
		}
	}else{
		const CLayout*	pcLayout;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdx = LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

			/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
			bool bWhere = m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
				GetCaret().GetCaretLayoutPos().GetY2(),
				nIdx,
				&sRange,
				NULL,
				NULL
			);
			if( bWhere ){
				/* �I��͈͂̕ύX */
				GetSelectionInfo().m_sSelectBgn = sRange;
				GetSelectionInfo().m_sSelect    = sRange;

				/* �I��͈͂̃f�[�^���擾 */
				if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
					/* ��������������݈ʒu�̒P��ŏ����� */
					wcsncpy( szTopic, cmemCurText.GetStringPtr(), MAX_PATH - 1 );
					szTopic[MAX_PATH - 1] = L'\0';
				}
				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
				GetSelectionInfo().DisableSelectArea( FALSE );
			}
		}
	}

	/* ����������͉��s�܂� */
	for( i = 0; i < (int)wcslen( szTopic ); ++i ){
		if( szTopic[i] == WCODE::CR || szTopic[i] == WCODE::LF ){
			szTopic[i] = L'\0';
			break;
		}
	}
	cmemCurText.SetString( szTopic );
}


/*!	���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾�i�_�C�A���O�p�j
	@date 2006.08.23 ryoji �V�K�쐬
*/
void CEditView::GetCurrentTextForSearchDlg( CNativeW& cmemCurText )
{
	cmemCurText.SetString(L"");

	if( GetSelectionInfo().IsTextSelected() ){	// �e�L�X�g���I������Ă���
		GetCurrentTextForSearch( cmemCurText );
	}
	else{	// �e�L�X�g���I������Ă��Ȃ�
		if( GetDllShareData().m_Common.m_sSearch.m_bCaretTextForSearch ){
			GetCurrentTextForSearch( cmemCurText );	// �J�[�\���ʒu�P����擾
		}
		else{
			cmemCurText.SetString( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0] );	// ��������Ƃ��Ă���
		}
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �`��p����                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* ���݈ʒu������������ɊY�����邩 */
//2002.02.08 hor
//���K�\���Ō��������Ƃ��̑��x���P�̂��߁A�}�b�`�擪�ʒu�������ɒǉ�
//Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
bool CEditView::IsSearchString(
	const CStringRef&	cStr,
	/*
	const wchar_t*	pszData,
	CLogicInt		nDataLen,
	*/
	CLogicInt		nPos,
	CLogicInt*		pnSearchStart,
	CLogicInt*		pnSearchEnd
) const
{
	CLogicInt		nKeyLength;

	*pnSearchStart = nPos;	// 2002.02.08 hor

	if( m_sCurSearchOption.bRegularExp ){
		/* �s���ł͂Ȃ�? */
		/* �s�������`�F�b�N�́ACBregexp�N���X�����Ŏ��{����̂ŕs�v 2003.11.01 ����� */

		/* �ʒu��0��MatchInfo�Ăяo���ƁA�s�������������ɁA�S�� true�@�ƂȂ�A
		** ��ʑS�̂����������񈵂��ɂȂ�s��C��
		** �΍�Ƃ��āA�s���� MacthInfo�ɋ����Ȃ��Ƃ����Ȃ��̂ŁA������̒����E�ʒu����^����`�ɕύX
		** 2003.05.04 �����
		*/
		if( m_CurRegexp.Match( cStr.GetPtr(), cStr.GetLength(), nPos ) ){
			*pnSearchStart = m_CurRegexp.GetIndex();	// 2002.02.08 hor
			*pnSearchEnd = m_CurRegexp.GetLastIndex();
			return true;
		}
		else{
			return false;
		}
	}
	else{
		nKeyLength = CLogicInt(wcslen( m_szCurSrchKey ));		/* �������� */

		// 2001/06/23 �P��P�ʂ̌����̂��߂ɒǉ�
		// 2010.06.30 Moca GetDllShareData() �ɂȂ��Ă����̂��C��
		if( m_sCurSearchOption.bWordOnly ){	/* �����^�u��  1==�P��̂݌��� */
		
			/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
			/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
			CLogicInt nIdxFrom, nIdxTo;
			if( !CWordParse::WhereCurrentWord_2( cStr.GetPtr(), CLogicInt(cStr.GetLength()), nPos, &nIdxFrom, &nIdxTo, NULL, NULL ) ){
				return false;
			}
			if( nPos != nIdxFrom || nKeyLength != nIdxTo - nIdxFrom ){
				return false;
			}
		}

		//��������������` �܂��� ���������̒�����蒲�ׂ�f�[�^���Z���Ƃ��̓q�b�g���Ȃ�
		if( 0 == nKeyLength || nKeyLength > cStr.GetLength() - nPos ){
			return false;
		}
		//�p�啶���������̋�ʂ����邩�ǂ���
		if( m_sCurSearchOption.bLoHiCase ){	/* 1==�p�啶���������̋�� */
			if( 0 == auto_memcmp( &cStr.GetPtr()[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return true;
			}
		}else{
			if( 0 == auto_memicmp( &cStr.GetPtr()[nPos], m_szCurSrchKey, nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return true;
			}
		}
	}
	return false;
}
