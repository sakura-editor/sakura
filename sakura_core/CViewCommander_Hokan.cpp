#include "StdAfx.h"
#include "CViewCommander.h"
#include "CPropertyManager.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "CEditApp.h"
#include "CPropertyManager.h"

/*!	���͕⊮
	Ctrl+Space�ł����ɓ����B
	CEditView::m_bHokan�F ���ݕ⊮�E�B���h�E���\������Ă��邩��\���t���O�B
	m_Common.m_sHelper.m_bUseHokan�F���ݕ⊮�E�B���h�E���\������Ă���ׂ����ۂ�������킷�t���O�B

    @date 2001/06/19 asa-o �p�啶���������𓯈ꎋ����
                     ��₪1�̂Ƃ��͂���Ɋm�肷��
	@date 2001/06/14 asa-o �Q�ƃf�[�^�ύX
	                 �J���v���p�e�B�V�[�g���^�C�v�ʂɕύX
	@date 2000/09/15 JEPRO [Esc]�L�[��[x]�{�^���ł����~�ł���悤�ɕύX
	@date 2005/01/10 genta CEditView_Command����ړ�
*/
void CViewCommander::Command_HOKAN( void )
{
	if(!GetDllShareData().m_Common.m_sHelper.m_bUseHokan){
		GetDllShareData().m_Common.m_sHelper.m_bUseHokan = TRUE;
	}
retry:;
	/* �⊮���ꗗ�t�@�C�����ݒ肳��Ă��Ȃ��Ƃ��́A�ݒ肷��悤�ɑ����B */
	// 2003.06.22 Moca �t�@�C�������猟������ꍇ�ɂ͕⊮�t�@�C���̐ݒ�͕K�{�ł͂Ȃ�
	if( GetDocument()->m_cDocType.GetDocumentAttribute().m_bUseHokanByFile == FALSE &&
		_T('\0') == GetDocument()->m_cDocType.GetDocumentAttribute().m_szHokanFile[0]
	){
		ConfirmBeep();
		if( IDYES == ::ConfirmMessage( GetMainWindow(),
			_T("�⊮���ꗗ�t�@�C�����ݒ肳��Ă��܂���B\n�������ݒ肵�܂���?")
		) ){
			/* �^�C�v�ʐݒ� �v���p�e�B�V�[�g */
			if( !CEditApp::getInstance()->m_pcPropertyManager->OpenPropertySheetTypes( 2, GetDocument()->m_cDocType.GetDocumentType() ) ){
				return;
			}
			goto retry;
		}
	}

	CNativeW	cmemData;
	/* �J�[�\�����O�̒P����擾 */
	if( 0 < m_pCommanderView->GetParser().GetLeftWord( &cmemData, 100 ) ){
		m_pCommanderView->ShowHokanMgr( cmemData, TRUE );
	}else{
		InfoBeep(); //2010.04.03 Error��Info
		m_pCommanderView->SendStatusMessage(_T("�⊮�Ώۂ�����܂���")); // 2010.05.29 �X�e�[�^�X�ŕ\��
		GetDllShareData().m_Common.m_sHelper.m_bUseHokan = FALSE;	//	���͕⊮�I���̂��m�点
	}
	return;
}
