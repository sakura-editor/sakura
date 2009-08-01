/*!	@file
	@brief CEditView�N���X�̃C���N�������^���T�[�`�֘A�R�}���h�����n�֐��Q

	@author genta
	@date	2005/01/10 �쐬
*/
/*
	Copyright (C) 2004, isearch
	Copyright (C) 2005, genta, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "stdafx.h"
#include "sakura_rc.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "doc/CDocLine.h"
#include "CMigemo.h"

/*!
	�R�}���h�R�[�h�̕ϊ�(ISearch��)�y��
	�C���N�������^���T�[�`���[�h�𔲂��锻��

	@return true: �R�}���h�����ς� / false: �R�}���h�����p��

	@date 2004.09.14 isearch �V�K�쐬
	@date 2005.01.10 genta �֐���, UNINDENT�ǉ�

	@note UNINDENT��ʏ핶���Ƃ��Ĉ����̂́C
		SHIFT+�����̌��SPACE����͂���悤�ȃP�[�X��
		SHIFT�̉�����x��Ă�����������Ȃ��Ȃ邱�Ƃ�h�����߁D
*/
void CEditView::TranslateCommand_isearch(
	EFunctionCode&	nCommand,
	bool&			bRedraw,
	LPARAM&			lparam1,
	LPARAM&			lparam2,
	LPARAM&			lparam3,
	LPARAM&			lparam4
)
{
	if (m_nISearchMode <= 0 )
		return;

	switch (nCommand){
		//�����̋@�\�̂Ƃ��A�C���N�������^���T�[�`�ɓ���
		case F_ISEARCH_NEXT:
		case F_ISEARCH_PREV:
		case F_ISEARCH_REGEXP_NEXT:
		case F_ISEARCH_REGEXP_PREV:
		case F_ISEARCH_MIGEMO_NEXT:
		case F_ISEARCH_MIGEMO_PREV:
			break;

		//�ȉ��̋@�\�̂Ƃ��A�C���N�������^���T�[�`���͌����������͂Ƃ��ď���
		case F_WCHAR:
		case F_IME_CHAR:
			nCommand = F_ISEARCH_ADD_CHAR;
			break;
		case F_INSTEXT_W:
			nCommand = F_ISEARCH_ADD_STR;
			break;

		case F_INDENT_TAB:	// TAB�̓C���f���g�ł͂Ȃ��P�Ȃ�TAB�����ƌ��Ȃ�
		case F_UNINDENT_TAB:	// genta�ǉ�
			nCommand = F_ISEARCH_ADD_CHAR;
			lparam1 = '\t';
			break;
		case F_INDENT_SPACE:	// �X�y�[�X�̓C���f���g�ł͂Ȃ��P�Ȃ�TAB�����ƌ��Ȃ�
		case F_UNINDENT_SPACE:	// genta�ǉ�
			nCommand = F_ISEARCH_ADD_CHAR;
			lparam1 = ' ';
			break;
		case F_DELETE_BACK:
			nCommand = F_ISEARCH_DEL_BACK;
			break;

		default:
			//��L�ȊO�̃R�}���h�̏ꍇ�̓C���N�������^���T�[�`�𔲂���
			ISearchExit();
	}
}

/*!
	ISearch �R�}���h����

	@date 2005.01.10 genta �e�R�}���h�ɓ����Ă���������1�J���Ɉړ�
*/
bool CEditView::ProcessCommand_isearch(
	int	nCommand,
	bool	bRedraw,
	LPARAM	lparam1,
	LPARAM	lparam2,
	LPARAM	lparam3,
	LPARAM	lparam4
)
{
	switch( nCommand ){
		//	����������̕ύX����
		case F_ISEARCH_ADD_CHAR:
			ISearchExec((DWORD)lparam1);
			return true;
		
		case F_ISEARCH_DEL_BACK:
			ISearchBack();
			return true;

		case F_ISEARCH_ADD_STR:
			ISearchExec((LPCWSTR)lparam1);
			return true;

		//	�������[�h�ւ̈ڍs
		case F_ISEARCH_NEXT:
			ISearchEnter(1,SEARCH_FORWARD);	//�O���C���N�������^���T�[�` //2004.10.13 isearch
			return true;
		case F_ISEARCH_PREV:
			ISearchEnter(1,SEARCH_BACKWARD); //����C���N�������^���T�[�` //2004.10.13 isearch
			return true;
		case F_ISEARCH_REGEXP_NEXT:
			ISearchEnter(2,SEARCH_FORWARD);	//�O�����K�\���C���N�������^���T�[�`  //2004.10.13 isearch
			return true;
		case F_ISEARCH_REGEXP_PREV:
			ISearchEnter(2,SEARCH_BACKWARD);	//������K�\���C���N�������^���T�[�`  //2004.10.13 isearch
			return true;
		case F_ISEARCH_MIGEMO_NEXT:
			ISearchEnter(3,SEARCH_FORWARD);	//�O��MIGEMO�C���N�������^���T�[�`    //2004.10.13 isearch
			return true;
		case F_ISEARCH_MIGEMO_PREV:
			ISearchEnter(3,SEARCH_BACKWARD); //���MIGEMO�C���N�������^���T�[�`    //2004.10.13 isearch
			return true;
	}
	return false;
}

/*!
	�C���N�������^���T�[�`���[�h�ɓ���

	@param mode [in] �������@ 1:�ʏ�, 2:���K�\��, 3:MIGEMO
	@param direction [in] �������� 0:���(���), 1:�O��(����)

	@author isearch
*/
void CEditView::ISearchEnter( int mode, ESearchDirection direction)
{

	if (m_nISearchMode == mode ) {
		//�Ď��s
		m_nISearchDirection =  direction;
		
		if ( m_bISearchFirst ){
			m_bISearchFirst = false;
		}
		//������ƏC��
		ISearchExec(true);

	}else{
		//�C���N�������^���T�[�`���[�h�ɓ��邾��.		
		//�I��͈͂̉���
		if(GetSelectionInfo().IsTextSelected())	
			GetSelectionInfo().DisableSelectArea( TRUE );
		
		if(m_pcmigemo==NULL){
			m_pcmigemo = CMigemo::getInstance();
			m_pcmigemo->InitDll();
		}
		switch( mode ) {
			case 1: // �ʏ�C���N�������^���T�[�`
				m_sCurSearchOption.bRegularExp = FALSE;
				GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp = false;
				//SendStatusMessage(_T("I-Search: "));
				break;
			case 2: // ���K�\���C���N�������^���T�[�`
				if (!m_CurRegexp.IsAvailable()){
					WarningBeep();
					SendStatusMessage(_T("BREGREP.DLL���g�p�ł��܂���B"));
					return;
				}
				m_sCurSearchOption.bRegularExp = TRUE;
				GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp = TRUE;
				//SendStatusMessage(_T("[RegExp] I-Search: "));
				break;
			case 3: // MIGEMO�C���N�������^���T�[�`
				if (!m_CurRegexp.IsAvailable()){
					WarningBeep();
					SendStatusMessage(_T("BREGREP.DLL���g�p�ł��܂���B"));
					return;
				}
				//migemo dll �`�F�b�N
				//	Jan. 10, 2005 genta �ݒ�ύX�Ŏg����悤�ɂȂ��Ă���
				//	�\��������̂ŁC�g�p�\�łȂ���Έꉞ�����������݂�
				if ( !m_pcmigemo->IsAvailable() && DLL_SUCCESS != m_pcmigemo->InitDll() ){
					WarningBeep();
					SendStatusMessage(_T("MIGEMO.DLL���g�p�ł��܂���B"));
					return;
				}
				m_pcmigemo->migemo_load_all();
				if (m_pcmigemo->migemo_is_enable()) {
					m_sCurSearchOption.bRegularExp = TRUE;
					GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp = true;
					//SendStatusMessage(_T("[MIGEMO] I-Search: "));
				}else{
					WarningBeep();
					SendStatusMessage(_T("MIGEMO�͎g�p�ł��܂���B "));
					return;
				}
				break;
		}
		
		//	Feb. 04, 2005 genta	�����J�n�ʒu���L�^
		//	�C���N�������^���T�[�`�ԂŃ��[�h��؂�ւ���ꍇ�ɂ͊J�n�ƌ��Ȃ��Ȃ�
		if( m_nISearchMode == 0 ){
			m_ptSrchStartPos_PHY = GetCaret().GetCaretLogicPos();
		}
		
		m_bCurSrchKeyMark = false;
		m_nISearchDirection = direction;
		m_nISearchMode = mode;
		
		m_nISearchHistoryCount = 0;
		m_sISearchHistory[m_nISearchHistoryCount].Set(GetCaret().GetCaretLayoutPos());

		Redraw();
		
		CNativeT msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetStringPtr());
		
		m_bISearchWrap = false;
		m_bISearchFirst = true;
	}

	//�}�E�X�J�[�\���ύX
	if (direction == 1){
		::SetCursor( ::LoadCursor( G_AppInstance(),MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
	}else{
		::SetCursor( ::LoadCursor( G_AppInstance(),MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
	}
}

//!	�C���N�������^���T�[�`���[�h���甲����
void CEditView::ISearchExit()
{
	CSearchKeywordManager().AddToSearchKeyArr( m_szCurSrchKey );
	m_nISearchDirection = SEARCH_BACKWARD;
	m_nISearchMode = 0;
	
	if (m_nISearchHistoryCount == 0){
		m_szCurSrchKey[0] = '\0';
	}

	//�}�E�X�J�[�\�������ɖ߂�
	POINT point1;
	GetCursorPos(&point1);
	OnMOUSEMOVE(0,point1.x,point1.y);

	//�X�e�[�^�X�\���G���A���N���A
	SendStatusMessage(_T(""));

}

/*!
	@brief �C���N�������^���T�[�`�̎��s(1�����ǉ�)
	
	@param wChar [in] �ǉ����镶�� (1byte or 2byte)
*/
void CEditView::ISearchExec(DWORD wChar)
{
	//���ꕶ���͏������Ȃ�
	switch ( wChar){
		case L'\r':
		case L'\n':
			ISearchExit();
			return;
		//case '\t':
		//	break;
	}
	
	int l;
	if (m_bISearchFirst){
		m_bISearchFirst = false;
		l = 0 ;
	}else	
		l = wcslen(m_szCurSrchKey) ;

	if( wChar <= 0xffff ){
		if( l < _countof(m_szCurSrchKey) - 1 ){
			m_szCurSrchKey[l] = (WCHAR)wChar;
			m_szCurSrchKey[l+1] = L'\0';
		}
	}else{
		if( l < _countof(m_szCurSrchKey) - 2 ){
			m_szCurSrchKey[l]   = (WCHAR)(wChar>>16);
			m_szCurSrchKey[l+1] = (WCHAR)wChar;
			m_szCurSrchKey[l+2] = L'\0';
		}
	}

	ISearchExec(false);
	return ;
}

/*!
	@brief �C���N�������^���T�[�`�̎��s(������ǉ�)
	
	@param pszText [in] �ǉ����镶����
*/
void CEditView::ISearchExec(LPCWSTR pszText)
{
	//�ꕶ�����������Ď��s

	const WCHAR* p;
	DWORD c;
	p = pszText;
	
	while(*p!=L'\0'){
		if( IsUtf16SurrogHi(*p) && IsUtf16SurrogLow(*(p+1)) ){
			c = ( ((WORD)*p)<<16 ) | ( (WORD)*(p+1) );
			p++;
		}else{
			c = *p;
		}
		ISearchExec(c);
		p++;
	}
	return ;
}

/*!
	@brief �C���N�������^���T�[�`�̎��s

	@param bNext [in] true:���̌�������, false:���݂̃J�[�\���ʒu�̂܂܌���
*/
void CEditView::ISearchExec(bool bNext) 
{
	//���������s����.

	if ( (m_szCurSrchKey[0] == L'\0') || (m_nISearchMode == 0)){
		//�X�e�[�^�X�̕\��
		CNativeT msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetStringPtr());
		return ;
	}
	
	ISearchWordMake();
	
	CLayoutInt nLine;
	CLayoutInt nIdx1;
	
	if ( bNext && m_bISearchWrap ) {
		switch (m_nISearchDirection)
		{
		case 1:
			nLine = CLayoutInt(0);
			nIdx1 = CLayoutInt(0);
			break;
		case 0:
			//�Ōォ�猟��
			CLogicInt nLineP;
			int nIdxP;
			nLineP =  m_pcEditDoc->m_cDocLineMgr.GetLineCount() - CLogicInt(1);
			CDocLine* pDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine( nLineP );
			nIdxP = pDocLine->GetLengthWithEOL() -1;
			CLayoutPoint ptTmp;
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout(CLogicPoint(nIdxP,nLineP),&ptTmp);
			nIdx1=ptTmp.GetX2();
			nLine=ptTmp.GetY2();
		}
	}else if (GetSelectionInfo().IsTextSelected()){
		switch( m_nISearchDirection * 2 + (bNext ? 1: 0)){
			case 2 : //�O�������Ō��݈ʒu���猟���̂Ƃ�
			case 1 : //��������Ŏ��������̂Ƃ�
				//�I��͈͂̐擪�������J�n�ʒu��
				nLine = GetSelectionInfo().m_sSelect.GetFrom().GetY2();
				nIdx1 = GetSelectionInfo().m_sSelect.GetFrom().GetX2();
				break;
			case 0 : //�O�������Ŏ�������
			case 3 : //��������Ō��݈ʒu���猟��
				//�I��͈͂̌�납��
				nLine = GetSelectionInfo().m_sSelect.GetTo().GetY2();
				nIdx1 = GetSelectionInfo().m_sSelect.GetTo().GetX2();
				break;
		}
	}else{
		nLine = GetCaret().GetCaretLayoutPos().GetY2();
		nIdx1  = GetCaret().GetCaretLayoutPos().GetX2();
	}

	//���ʒu����index�ɕϊ�
	CLayout* pCLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( nLine );
	CLogicInt nIdx = LineColmnToIndex( pCLayout, nIdx1 );

	m_nISearchHistoryCount ++ ;

	CNativeT msg;
	ISearchSetStatusMsg(&msg);

	if (m_nISearchHistoryCount >= 256) {
		m_nISearchHistoryCount = 156;
		for(int i = 100 ; i<= 255 ; i++){
			m_bISearchFlagHistory[i-100] = m_bISearchFlagHistory[i];
			m_sISearchHistory[i-100] = m_sISearchHistory[i];
		}
	}
	m_bISearchFlagHistory[m_nISearchHistoryCount] = bNext;

	CLayoutRange sMatchRange;

	int nSearchResult = m_pcEditDoc->m_cLayoutMgr.SearchWord(
		nLine,						// �����J�n���C�A�E�g�s
		nIdx,						// �����J�n�f�[�^�ʒu
		m_szCurSrchKey,				// ��������
		m_nISearchDirection,		// 0==�O������ 1==�������
		SSearchOption(
			m_sCurSearchOption.bRegularExp,	//���K�\��
			false,							//�p�啶���������̋��
			false							//�P��̂݌���
		),
		&sMatchRange,				// �}�b�`���C�A�E�g�͈�
		&m_CurRegexp
	);
	if( nSearchResult == 0 ){
		/*�������ʂ��Ȃ�*/
		msg.AppendString(_T(" (������܂���)"));
		SendStatusMessage(msg.GetStringPtr());
		
		if (bNext) 	m_bISearchWrap = true;
		if (GetSelectionInfo().IsTextSelected()){
			m_sISearchHistory[m_nISearchHistoryCount] = GetSelectionInfo().m_sSelect;
		}else{
			m_sISearchHistory[m_nISearchHistoryCount].Set(GetCaret().GetCaretLayoutPos());
		}
	}else{
		//�������ʂ���
		//�L�����b�g�ړ�
		GetCaret().MoveCursor( sMatchRange.GetFrom(), TRUE, _CARETMARGINRATE / 3 );
		
		//	2005.06.24 Moca
		GetSelectionInfo().SetSelectArea( sMatchRange );

		m_bISearchWrap = false;
		m_sISearchHistory[m_nISearchHistoryCount] = sMatchRange;
	}

	m_bCurSrchKeyMark = true;

	Redraw();	
	SendStatusMessage(msg.GetStringPtr());
	return ;
}

//!	�o�b�N�X�y�[�X�������ꂽ�Ƃ��̏���
void CEditView::ISearchBack(void) {
	if(m_nISearchHistoryCount==0) return;
	
	if(m_nISearchHistoryCount==1){
		m_bCurSrchKeyMark = false;
		m_bISearchFirst = true;
	}else if( m_bISearchFlagHistory[m_nISearchHistoryCount] == false){
		//�����������ւ炷
		long l = wcslen(m_szCurSrchKey);
		if (l > 0 ){
			//�Ō�̕����̈�O
			wchar_t* p = (wchar_t*)CNativeW::GetCharPrev( m_szCurSrchKey, l, &m_szCurSrchKey[l] );
			*p = L'\0';
			//m_szCurSrchKey[l-1] = '\0';

			if ( (p - m_szCurSrchKey) > 0 ) 
				ISearchWordMake();
			else
				m_bCurSrchKeyMark = false;

		}else{
			WarningBeep();
		}
	}
	m_nISearchHistoryCount --;

	CLayoutRange sRange = m_sISearchHistory[m_nISearchHistoryCount];

	if(m_nISearchHistoryCount == 0){
		GetSelectionInfo().DisableSelectArea( TRUE );
		sRange.SetToX( sRange.GetFrom().x );
	}

	GetCaret().MoveCursor( sRange.GetFrom(), TRUE, _CARETMARGINRATE / 3 );
	if(m_nISearchHistoryCount != 0){
		//	2005.06.24 Moca
		GetSelectionInfo().SetSelectArea( sRange );
	}

	Redraw();

	//�X�e�[�^�X�\��
	CNativeT msg;
	ISearchSetStatusMsg(&msg);
	SendStatusMessage(msg.GetStringPtr());
	
}

//!	���͕�������A���������𐶐�����B
void CEditView::ISearchWordMake(void)
{
	int nFlag = 0x00;
	switch ( m_nISearchMode ) {
	case 1: // �ʏ�C���N�������^���T�[�`
		break;
	case 2: // ���K�\���C���N�������^���T�[�`
		if( !InitRegexp( this->GetHwnd(), m_CurRegexp, true ) ){
			return ;
		}
		nFlag |= m_sCurSearchOption.bLoHiCase ? 0x01 : 0x00;
		/* �����p�^�[���̃R���p�C�� */
		m_CurRegexp.Compile(m_szCurSrchKey , nFlag );
		break;
	case 3: // MIGEMO�C���N�������^���T�[�`
		if( !InitRegexp( this->GetHwnd(), m_CurRegexp, true ) ){
			return ;
		}
		nFlag |= m_sCurSearchOption.bLoHiCase ? 0x01 : 0x00;

		{
			//migemo�ő{��
			unsigned char* pszMigemoWord = m_pcmigemo->migemo_query((unsigned char*)to_achar(m_szCurSrchKey));
			
			/* �����p�^�[���̃R���p�C�� */
			m_CurRegexp.Compile(to_wchar((char*)pszMigemoWord), nFlag );

			m_pcmigemo->migemo_release(pszMigemoWord);
		}
		break;
	}
}

/*!	@brief ISearch���b�Z�[�W�\�z

	���݂̃T�[�`���[�h�y�ь����������񂩂�
	���b�Z�[�W�G���A�ɕ\�����镶������\�z����
	
	@param msg [out] ���b�Z�[�W�o�b�t�@
	
	@author isearch
	@date 2004/10/13
	@date 2005.01.13 genta ������C��
*/
void CEditView::ISearchSetStatusMsg(CNativeT* msg) const
{

	switch ( m_nISearchMode){
	case 1 :
		msg->SetString(_T("I-Search") );
		break;
	case 2 :
		msg->SetString(_T("[RegExp] I-Search") );
		break;
	case 3 :
		msg->SetString(_T("[Migemo] I-Search") );
		break;
	default:
		msg->SetString(_T(""));
		return;
	}
	if (m_nISearchDirection == 0){
		msg->AppendString(_T(" Backward: "));
	}
	else{
		msg->AppendString(_T(": "));
	}

	if(m_nISearchHistoryCount > 0)
		msg->AppendString(to_tchar(m_szCurSrchKey));
}

/*!
	ISearch��Ԃ��c�[���o�[�ɔ��f������D
	
	@sa CEditWnd::IsFuncChecked()

	@param nCommand [in] ���ׂ����R�}���h��ID
	@return true:�`�F�b�N�L�� / false: �`�F�b�N����
	
	@date 2005.01.10 genta �V�K�쐬
*/
bool CEditView::IsISearchEnabled(int nCommand) const
{
	switch( nCommand )
	{
	case F_ISEARCH_NEXT:
		return m_nISearchMode == 1 && m_nISearchDirection == 1;
	case F_ISEARCH_PREV:
		return m_nISearchMode == 1 && m_nISearchDirection == 0;
	case F_ISEARCH_REGEXP_NEXT:
		return m_nISearchMode == 2 && m_nISearchDirection == 1;
	case F_ISEARCH_REGEXP_PREV:
		return m_nISearchMode == 2 && m_nISearchDirection == 0;
	case F_ISEARCH_MIGEMO_NEXT:
		return m_nISearchMode == 3 && m_nISearchDirection == 1;
	case F_ISEARCH_MIGEMO_PREV:
		return m_nISearchMode == 3 && m_nISearchDirection == 0;
	}
	return false;
}

