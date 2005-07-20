//	$Id$
/*!	@file
	@brief CEditView�N���X�̃C���N�������^���T�[�`�֘A�R�}���h�����n�֐��Q

	@date	2005/01/10 �쐬
	$Revision$
*/
/*
	Copyright (C) 2004, isearch
	Copyright (C) 2005, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "sakura_rc.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "CDocLine.h"
#include "CMigemo.h"
#include "etc_uty.h"

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
	int&	nCommand,
	BOOL&	bRedraw,
	LPARAM&	lparam1,
	LPARAM&	lparam2,
	LPARAM&	lparam3,
	LPARAM&	lparam4
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
		case F_CHAR:
		case F_IME_CHAR:
			nCommand = F_ISEARCH_ADD_CHAR;
			break;
		case F_INSTEXT:
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
	BOOL	bRedraw,
	LPARAM	lparam1,
	LPARAM	lparam2,
	LPARAM	lparam3,
	LPARAM	lparam4
)
{
	switch( nCommand ){
		//	����������̕ύX����
		case F_ISEARCH_ADD_CHAR:
			ISearchExec((WORD)lparam1);
			return true;
		
		case F_ISEARCH_DEL_BACK:
			ISearchBack();
			return true;

		case F_ISEARCH_ADD_STR:
			ISearchExec((const char*)lparam1);
			return true;

		//	�������[�h�ւ̈ڍs
		case F_ISEARCH_NEXT:
			ISearchEnter(1,1);	//�O���C���N�������^���T�[�` //2004.10.13 isearch
			return true;
		case F_ISEARCH_PREV:
			ISearchEnter(1,0);	//����C���N�������^���T�[�` //2004.10.13 isearch
			return true;
		case F_ISEARCH_REGEXP_NEXT:
			ISearchEnter(2,1);	//�O�����K�\���C���N�������^���T�[�`  //2004.10.13 isearch
			return true;
		case F_ISEARCH_REGEXP_PREV:
			ISearchEnter(2,0);	//������K�\���C���N�������^���T�[�`  //2004.10.13 isearch
			return true;
		case F_ISEARCH_MIGEMO_NEXT:
			ISearchEnter(3,1);	//�O��MIGEMO�C���N�������^���T�[�`    //2004.10.13 isearch
			return true;
		case F_ISEARCH_MIGEMO_PREV:
			ISearchEnter(3,0);	//���MIGEMO�C���N�������^���T�[�`    //2004.10.13 isearch
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
void CEditView::ISearchEnter( int mode  ,int direction)
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
		if(IsTextSelected())	
			DisableSelectArea( TRUE );
		
		if(m_pcmigemo==NULL){
			m_pcmigemo = CMigemo::getInstance();
			m_pcmigemo->Init();
		}
		switch( mode ) {
			case 1: // �ʏ�C���N�������^���T�[�`
				m_bCurSrchRegularExp = FALSE;
				m_pShareData->m_Common.m_bRegularExp = FALSE;
				//SendStatusMessage("I-Search: ");
				break;
			case 2: // ���K�\���C���N�������^���T�[�`
				if (!m_CurRegexp.IsAvailable()){
					MessageBeep(MB_ICONEXCLAMATION);
					SendStatusMessage("BREGREP.DLL���g�p�ł��܂���B");
					return;
				}
				m_bCurSrchRegularExp = TRUE;
				m_pShareData->m_Common.m_bRegularExp = TRUE;
				//SendStatusMessage("[RegExp] I-Search: ");
				break;
			case 3: // MIGEMO�C���N�������^���T�[�`
				if (!m_CurRegexp.IsAvailable()){
					MessageBeep(MB_ICONEXCLAMATION);
					SendStatusMessage("BREGREP.DLL���g�p�ł��܂���B");
					return;
				}
				//migemo dll �`�F�b�N
				//	Jan. 10, 2005 genta �ݒ�ύX�Ŏg����悤�ɂȂ��Ă���
				//	�\��������̂ŁC�g�p�\�łȂ���Έꉞ�����������݂�
				if ( ! m_pcmigemo->IsAvailable() && ! m_pcmigemo->Init() ){
					MessageBeep(MB_ICONEXCLAMATION);
					SendStatusMessage("MIGEMO.DLL���g�p�ł��܂���B");
					return;
				}
				m_pcmigemo->migemo_load_all();
				if (m_pcmigemo->migemo_is_enable()) {
					m_bCurSrchRegularExp = TRUE;
					m_pShareData->m_Common.m_bRegularExp = TRUE;
					//SendStatusMessage("[MIGEMO] I-Search: ");
				}else{
					MessageBeep(MB_ICONEXCLAMATION);
					SendStatusMessage("MIGEMO�͎g�p�ł��܂���B ");
					return;
				}
				break;
		}
		
		//	Feb. 04, 2005 genta	�����J�n�ʒu���L�^
		//	�C���N�������^���T�[�`�ԂŃ��[�h��؂�ւ���ꍇ�ɂ͊J�n�ƌ��Ȃ��Ȃ�
		if( m_nISearchMode == 0 ){
			m_nSrchStartPosX_PHY = m_nCaretPosX_PHY;
			m_nSrchStartPosY_PHY = m_nCaretPosY_PHY;
		}
		
		//m_szCurSrchKey[0] = '\0';
		m_bCurSrchKeyMark = FALSE;
		m_nISearchDirection = direction;
		m_nISearchMode = mode;
		
		m_nISearchHistoryCount = 0;
		m_nISearchX1History[m_nISearchHistoryCount] = m_nCaretPosX;
		m_nISearchY1History[m_nISearchHistoryCount] = m_nCaretPosY;
		m_nISearchX2History[m_nISearchHistoryCount] = m_nCaretPosX;
		m_nISearchY2History[m_nISearchHistoryCount] = m_nCaretPosY;

		Redraw();
		
		CMemory msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetPtr());
		
		m_bISearchWrap = false;
		m_bISearchFirst = true;
	}

	//�}�E�X�J�[�\���ύX
	if (direction == 1){
		::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
	}else{
		::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
	}
}

//!	�C���N�������^���T�[�`���[�h���甲����
void CEditView::ISearchExit()
{
	CShareData::getInstance()->AddToSearchKeyArr( m_szCurSrchKey );
	m_nISearchDirection = 0;
	m_nISearchMode = 0;
	
	if (m_nISearchHistoryCount == 0){
		m_szCurSrchKey[0] = '\0';
	}

	//�}�E�X�J�[�\�������ɖ߂�
	POINT point1;
	GetCursorPos(&point1);
	OnMOUSEMOVE(0,point1.x,point1.y);

	//�X�e�[�^�X�\���G���A���N���A
	SendStatusMessage("");

}

/*!
	@brief �C���N�������^���T�[�`�̎��s(1�����ǉ�)
	
	@param wChar [in] �ǉ����镶�� (1byte or 2byte)
*/
void CEditView::ISearchExec(WORD wChar)
{
	//���ꕶ���͏������Ȃ�
	switch ( wChar){
		case '\r':
		case '\n':
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
		l = strlen(m_szCurSrchKey) ;

	if (wChar <= 255 ) {
		if ( l < sizeof(m_szCurSrchKey) - 1) {
			m_szCurSrchKey[l] =(char)wChar;
			m_szCurSrchKey[l+1] = '\0';				
		}
	}else{
		if ( l < sizeof(m_szCurSrchKey) - 2) {
			m_szCurSrchKey[l]   =(char)(wChar>>8);
			m_szCurSrchKey[l+1] =(char)wChar;
			m_szCurSrchKey[l+2] = '\0';				
		}
	}

	ISearchExec(false);
	return ;
}

/*!
	@brief �C���N�������^���T�[�`�̎��s(������ǉ�)
	
	@param pszText [in] �ǉ����镶����
*/
void CEditView::ISearchExec(const char* pszText)
{
	//�ꕶ�����������Ď��s

	const char* p;
	WORD  c;
	p = pszText;
	
	while(*p!='\0'){
		if (IsDBCSLeadByte(*p)){
			c =( ((WORD)*p) * 256) | (unsigned char)*(p+1);
			p++;
		}else
			c =*p;
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
	int nLineFrom, nColmFrom, nColmTo, nLineTo;
	
	if ( (m_szCurSrchKey[0] == '\0') || (m_nISearchMode == 0)){
		//�X�e�[�^�X�̕\��
		CMemory msg;
		ISearchSetStatusMsg(&msg);
		SendStatusMessage(msg.GetPtr());		
		return ;
	}
	
	ISearchWordMake();
	
	int nLine, nIdx,nIdx1;
	
	if ( bNext && m_bISearchWrap ) {
		switch (m_nISearchDirection)
		{
		case 1:
			nLine = 0;
			nIdx1 = 0;
			break;
		case 0:
			//�Ōォ�猟��
			int nLineP, nIdxP;
			nLineP =  m_pcEditDoc->m_cDocLineMgr.GetLineCount() -1 ;
			CDocLine* pDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( nLineP );
			nIdxP = pDocLine->GetLength() -1;
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(nIdxP,nLineP,&nIdx1,&nLine);
		}
	}else if (IsTextSelected()){
		switch( m_nISearchDirection * 2 + (bNext ? 1: 0)){
			case 2 : //�O�������Ō��݈ʒu���猟���̂Ƃ�
			case 1 : //��������Ŏ��������̂Ƃ�
				//�I��͈͂̐擪�������J�n�ʒu��
				nLine = m_nSelectLineFrom;
				nIdx1 = m_nSelectColmFrom;
				break;
			case 0 : //�O�������Ŏ�������
			case 3 : //��������Ō��݈ʒu���猟��
				//�I��͈͂̌�납��
				nLine = m_nSelectLineTo;
				nIdx1 = m_nSelectColmTo;
				break;
		}
	}else{
		nLine = m_nCaretPosY;
		nIdx1  = m_nCaretPosX;
	}

	//���ʒu����index�ɕϊ�
	CLayout* pCLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLine );
	nIdx = LineColmnToIndex( pCLayout, nIdx1 );

	m_nISearchHistoryCount ++ ;

	CMemory msg;
	ISearchSetStatusMsg(&msg);

	if (m_nISearchHistoryCount >= 256) {
		m_nISearchHistoryCount = 156;
		for(int i = 100 ; i<= 255 ; i++){
			m_bISearchFlagHistory[i-100] = m_bISearchFlagHistory[i];
			m_nISearchX1History[i-100] = m_nISearchX1History[i] ;
			m_nISearchY1History[i-100] = m_nISearchY1History[i] ;
			m_nISearchX2History[i-100] = m_nISearchX2History[i] ;
			m_nISearchY2History[i-100] = m_nISearchY2History[i] ;
		}
	}
	m_bISearchFlagHistory[m_nISearchHistoryCount] = bNext;

	if (m_pcEditDoc->m_cLayoutMgr.SearchWord(
		nLine, 									/* �����J�n�s */
		nIdx, 									/* �����J�n�ʒu */
		m_szCurSrchKey,							/* �������� */
		m_nISearchDirection,					/* 0==�O������ 1==������� */
		m_bCurSrchRegularExp,					/* 1==���K�\�� */
		FALSE,									/* 1==�p�啶���������̋�� */
		FALSE,									/* 1==�P��̂݌��� */
		&nLineFrom,								/* �}�b�`���C�A�E�g�sfrom */
		&nColmFrom, 							/* �}�b�`���C�A�E�g�ʒufrom */
		&nLineTo, 								/* �}�b�`���C�A�E�g�sto */
		&nColmTo, 								/* �}�b�`���C�A�E�g�ʒuto */
		&m_CurRegexp	) == 0 )
	{
		/*�������ʂ��Ȃ�*/
		msg.AppendSz(" (������܂���)");
		SendStatusMessage(msg.GetPtr());
		
		if (bNext) 	m_bISearchWrap = true;
		if (IsTextSelected()){
			m_nISearchX1History[m_nISearchHistoryCount] = m_nSelectColmFrom;
			m_nISearchY1History[m_nISearchHistoryCount] = m_nSelectLineFrom;
			m_nISearchX2History[m_nISearchHistoryCount] = m_nSelectColmTo;
			m_nISearchY2History[m_nISearchHistoryCount] = m_nSelectLineTo;
		}else{
			m_nISearchX1History[m_nISearchHistoryCount] = m_nCaretPosX;
			m_nISearchY1History[m_nISearchHistoryCount] = m_nCaretPosY;
			m_nISearchX2History[m_nISearchHistoryCount] = m_nCaretPosX;
			m_nISearchY2History[m_nISearchHistoryCount] = m_nCaretPosY;
		}
	}else{
		//�������ʂ���
		//�L�����b�g�ړ�
		MoveCursor( nColmFrom, nLineFrom , TRUE, _CARETMARGINRATE / 3 );
		//	2005.06.24 Moca
		SetSelectArea( nLineFrom, nColmFrom, nLineTo, nColmTo );

		m_bISearchWrap = false;
		m_nISearchX1History[m_nISearchHistoryCount] = nColmFrom;
		m_nISearchY1History[m_nISearchHistoryCount] = nLineFrom;
		m_nISearchX2History[m_nISearchHistoryCount] = nColmTo;
		m_nISearchY2History[m_nISearchHistoryCount] = nLineTo;

	}

	m_bCurSrchKeyMark = TRUE;

	Redraw();	
	SendStatusMessage(msg.GetPtr());
	return ;
}

//!	�o�b�N�X�y�[�X�������ꂽ�Ƃ��̏���
void CEditView::ISearchBack(void) {
	if(m_nISearchHistoryCount==0) return;
	
	if(m_nISearchHistoryCount==1){
		m_bCurSrchKeyMark = FALSE;
		m_bISearchFirst = true;
	}else if( m_bISearchFlagHistory[m_nISearchHistoryCount] == false){
		//�����������ւ炷
		long l = strlen(m_szCurSrchKey);
		if (l > 0 ){
			//�Ō�̕����̈�O
			char* p = CharPrev(m_szCurSrchKey,&m_szCurSrchKey[l]);
			*p = '\0';
			//m_szCurSrchKey[l-1] = '\0';

			if ( (p - m_szCurSrchKey) > 0 ) 
				ISearchWordMake();
			else
				m_bCurSrchKeyMark = FALSE;

		}else{
			MessageBeep(MB_ICONEXCLAMATION);
		}
	}
	m_nISearchHistoryCount --;
	int nLineFrom, nColmFrom, nColmTo, nLineTo;
	nColmFrom = m_nISearchX1History[m_nISearchHistoryCount];
	nLineFrom = m_nISearchY1History[m_nISearchHistoryCount];
	nColmTo   = m_nISearchX2History[m_nISearchHistoryCount];
	nLineTo   = m_nISearchY2History[m_nISearchHistoryCount];

	if(m_nISearchHistoryCount == 0){
		DisableSelectArea( TRUE );
		nColmTo = nColmFrom;
	}

	MoveCursor( nColmFrom , nLineFrom , TRUE, _CARETMARGINRATE / 3 );
	if(m_nISearchHistoryCount != 0){
		//	2005.06.24 Moca
		SetSelectArea( nLineFrom, nColmFrom, nLineTo, nColmTo );
	}

	Redraw();

	//�X�e�[�^�X�\��
	CMemory msg;
	ISearchSetStatusMsg(&msg);
	SendStatusMessage(msg.GetPtr());
	
}

//!	���͕�������A���������𐶐�����B
void CEditView::ISearchWordMake(void) {
	int nFlag = 0x00;
	switch ( m_nISearchMode ) {
		case 1: // �ʏ�C���N�������^���T�[�`
			break;
		case 2: // ���K�\���C���N�������^���T�[�`
			if( !InitRegexp( m_hWnd, m_CurRegexp, true ) ){
				return ;
			}
			nFlag |= m_bCurSrchLoHiCase ? 0x01 : 0x00;
			/* �����p�^�[���̃R���p�C�� */
			m_CurRegexp.Compile(m_szCurSrchKey , nFlag );
			break;
		case 3: // MIGEMO�C���N�������^���T�[�`
			if( !InitRegexp( m_hWnd, m_CurRegexp, true ) ){
				return ;
			}
			nFlag |= m_bCurSrchLoHiCase ? 0x01 : 0x00;

			//migemo�ő{��
			m_pszMigemoWord = (char*)m_pcmigemo->migemo_query((unsigned char*)m_szCurSrchKey);
			
			/* �����p�^�[���̃R���p�C�� */
			m_CurRegexp.Compile(m_pszMigemoWord , nFlag );

			m_pcmigemo->migemo_release((unsigned char*)m_pszMigemoWord);
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
void CEditView::ISearchSetStatusMsg(CMemory* msg) const
{

	switch ( m_nISearchMode){
		case 1 :
			msg->SetDataSz("I-Search" );
			break;
		case 2 :
			msg->SetDataSz("[RegExp] I-Search" );
			break;
		case 3 :
			msg->SetDataSz("[Migemo] I-Search" );
			break;
		default:
			msg->SetDataSz("");
			return;
	}
	if (m_nISearchDirection == 0){
		msg->AppendSz(" Backward: ");
	}else{
		msg->AppendSz(": ");
	}
	if(m_nISearchHistoryCount > 0)
		msg->AppendSz(m_szCurSrchKey);
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
/*[EOF]*/
