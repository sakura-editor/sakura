#include "StdAfx.h"
#include "CEditView.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"

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
		PleaseReportToAuthor( NULL, _T("CEditView::KeyWordHelpSearchDict\nnID=%d"), (int)nID );
	}
	/* �I��͈͂̃f�[�^���擾(�����s�I���̏ꍇ�͐擪�̍s�̂�) */
	if( GetSelectedData( &cmemCurText, TRUE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		wchar_t* pszWork = cmemCurText.GetStringPtr();
		int nWorkLength	= cmemCurText.GetStringLength();
		for( i = 0; i < nWorkLength; ++i ){
			if( pszWork[i] == L'\0' ||
				pszWork[i] == WCODE::CR ||
				pszWork[i] == WCODE::LF ){
				break;
			}
		}
		cmemCurText._SetStringLength( i );
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
void CEditView::GetCurrentTextForSearch( CNativeW& cmemCurText, bool bStripMaxPath /* = true */, bool bTrimSpaceTab /* = false */ )
{

	int				i;
	CNativeW		cmemTopic = L"";
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLogicInt		nIdx;
	CLayoutRange	sRange;

	cmemCurText.SetString(L"");
	if( GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* �I��͈͂̃f�[�^���擾 */
		if( GetSelectedData( &cmemCurText, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
			/* ��������������݈ʒu�̒P��ŏ����� */
			if( bStripMaxPath ){
				LimitStringLengthW(cmemCurText.GetStringPtr(), cmemCurText.GetStringLength(), _MAX_PATH - 1, cmemTopic);
			}else{
				cmemTopic = cmemCurText;
			}
		}
	}else{
		const CLayout*	pcLayout;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdx = LineColumnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

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
					if( bStripMaxPath ){
						LimitStringLengthW(cmemCurText.GetStringPtr(), cmemCurText.GetStringLength(), _MAX_PATH - 1, cmemTopic);
					}else{
						cmemTopic = cmemCurText;
					}
				}
				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
				GetSelectionInfo().DisableSelectArea( false );
			}
		}
	}

	wchar_t *pTopic2 = cmemTopic.GetStringPtr();
	if( bTrimSpaceTab ){
		// �O�̃X�y�[�X�E�^�u����菜��
		while( L'\0' != *pTopic2 && ( ' ' == *pTopic2 || '\t' == *pTopic2 ) ){
			pTopic2++;
		}
	}
	int nTopic2Len = (int)wcslen( pTopic2 );
	/* ����������͉��s�܂� */
	for( i = 0; i < nTopic2Len; ++i ){
		if( pTopic2[i] == WCODE::CR || pTopic2[i] == WCODE::LF ){
			break;
		}
	}
	
	if( bTrimSpaceTab ){
		// ���̃X�y�[�X�E�^�u����菜��
		int m = i - 1;
		while( 0 <= m &&
		    ( L' ' == pTopic2[m] || L'\t' == pTopic2[m] ) ){
			m--;
		}
		if( 0 <= m ){
			i = m + 1;
		}
	}
	cmemCurText.SetString( pTopic2, i );
}


/*!	���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾�i�_�C�A���O�p�j
	@date 2006.08.23 ryoji �V�K�쐬
*/
void CEditView::GetCurrentTextForSearchDlg( CNativeW& cmemCurText )
{
	bool bStripMaxPath = false;
	cmemCurText.SetString(L"");

	if( GetSelectionInfo().IsTextSelected() ){	// �e�L�X�g���I������Ă���
		GetCurrentTextForSearch( cmemCurText, bStripMaxPath );
	}
	else{	// �e�L�X�g���I������Ă��Ȃ�
		if( GetDllShareData().m_Common.m_sSearch.m_bCaretTextForSearch ){
			GetCurrentTextForSearch( cmemCurText, bStripMaxPath );	// �J�[�\���ʒu�P����擾
		}
		else if( 0 < GetDllShareData().m_sSearchKeywords.m_aSearchKeys.size()
				&& m_nCurSearchKeySequence < GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence ){
			cmemCurText.SetString( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0] );	// ��������Ƃ��Ă���
		}else{
			cmemCurText.SetString( m_strCurSearchKey.c_str() );
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
/*
	@retval 0
		(�p�^�[��������) �w��ʒu�ȍ~�Ƀ}�b�`�͂Ȃ��B
		(����ȊO) �w��ʒu�͌���������̎n�܂�ł͂Ȃ��B
	@retval 1,2,3,...
		(�p�^�[��������) �w��ʒu�ȍ~�Ƀ}�b�`�����������B
		(�P�ꌟ����) �w��ʒu������������Ɋ܂܂�鉽�Ԗڂ̒P��̎n�܂�ł��邩�B
		(����ȊO) �w��ʒu������������̎n�܂肾�����B
*/
int CEditView::IsSearchString(
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
			return 1;
		}
		else{
			return 0;
		}
	}
	else if( m_sCurSearchOption.bWordOnly ) { // �P�ꌟ��
		/* �w��ʒu�̒P��͈̔͂𒲂ׂ� */
		CLogicInt posWordHead, posWordEnd;
		if( ! CWordParse::WhereCurrentWord_2( cStr.GetPtr(), CLogicInt(cStr.GetLength()), nPos, &posWordHead, &posWordEnd, NULL, NULL ) ) {
			return 0; // �w��ʒu�ɒP�ꂪ������Ȃ������B
 		}
		if( nPos != posWordHead ) {
			return 0; // �w��ʒu�͒P��̎n�܂�ł͂Ȃ������B
		}
		const CLogicInt wordLength = posWordEnd - posWordHead;
		const wchar_t *const pWordHead = cStr.GetPtr() + posWordHead;

		// ��r�֐�
		int (*const fcmp)( const wchar_t*, const wchar_t*, size_t ) = m_sCurSearchOption.bLoHiCase ? wcsncmp : wcsnicmp;

		// �������P��ɕ������Ȃ���w��ʒu�̒P��Əƍ�����B
		int wordIndex = 0;
		const wchar_t* const searchKeyEnd = m_strCurSearchKey.data() + m_strCurSearchKey.size();
		for( const wchar_t* p = m_strCurSearchKey.data(); p < searchKeyEnd; ) {
			CLogicInt begin, end; // ������Ɋ܂܂��P��?�̈ʒu�BWhereCurrentWord_2()�̎d�l�ł͋󔒕�������P��Ɋ܂܂��B
			if( CWordParse::WhereCurrentWord_2( p, CLogicInt(searchKeyEnd - p), CLogicInt(0), &begin, &end, NULL, NULL )
				&& begin == 0 && begin < end
			) {
				if( ! WCODE::IsWordDelimiter( *p ) ) {
					++wordIndex;
					// p...(p + end) ��������Ɋ܂܂�� wordIndex�Ԗڂ̒P��B(wordIndex�̍ŏ��� 1)
					if( wordLength == end && 0 == fcmp( p, pWordHead, wordLength ) ) {
						*pnSearchStart = posWordHead;
						*pnSearchEnd = posWordEnd;
						return wordIndex;
					}
				}
				p += end;
			} else {
				p += CNativeW::GetSizeOfChar( p, searchKeyEnd - p, 0 );
			}
		}
		return 0; // �w��ʒu�̒P��ƌ���������Ɋ܂܂��P��͈�v���Ȃ������B
	}
	else {
		nKeyLength = CLogicInt(m_strCurSearchKey.size());

		//��������������` �܂��� ���������̒�����蒲�ׂ�f�[�^���Z���Ƃ��̓q�b�g���Ȃ�
		if( 0 == nKeyLength || nKeyLength > cStr.GetLength() - nPos ){
			return 0;
		}
		//�p�啶���������̋�ʂ����邩�ǂ���
		if( m_sCurSearchOption.bLoHiCase ){	/* 1==�p�啶���������̋�� */
			if( 0 == auto_memcmp( &cStr.GetPtr()[nPos], m_strCurSearchKey.data(), nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return 1;
			}
		}else{
			if( 0 == auto_memicmp( &cStr.GetPtr()[nPos], m_strCurSearchKey.data(), nKeyLength ) ){
				*pnSearchEnd = nPos + nKeyLength;
				return 1;
			}
		}
	}
	return 0;
}
