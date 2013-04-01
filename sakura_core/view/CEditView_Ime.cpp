#include "StdAfx.h"
#include "CEditView.h"
#include <algorithm>
#include "charset/CShiftJis.h"
#include "doc/CEditDoc.h"
#include "env/DLLSHAREDATA.h"
#include "_main/CAppMode.h"
#include "window/CEditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           IME                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*!	IME ON��

	@date  2006.12.04 ryoji �V�K�쐬�i�֐����j
*/
bool CEditView::IsImeON( void )
{
	bool bRet;
	HIMC	hIme;
	DWORD	conv, sent;

	//	From here Nov. 26, 2006 genta
	hIme = ImmGetContext( m_hwndParent );
	if( ImmGetOpenStatus( hIme ) != FALSE ){
		ImmGetConversionStatus( hIme, &conv, &sent );
		if(( conv & IME_CMODE_NOCONVERSION ) == 0 ){
			bRet = true;
		}
		else {
			bRet = false;
		}
	}
	else {
		bRet = false;
	}
	ImmReleaseContext( m_hwndParent, hIme );
	//	To here Nov. 26, 2006 genta

	return bRet;
}

/* IME�ҏW�G���A�̈ʒu��ύX */
void CEditView::SetIMECompFormPos( void )
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	COMPOSITIONFORM	CompForm;
	HIMC			hIMC = ::ImmGetContext( GetHwnd() );
	POINT			point;
	HWND			hwndFrame;
	hwndFrame = ::GetParent( m_hwndParent );

	::GetCaretPos( &point );
	CompForm.dwStyle = CFS_POINT;
	CompForm.ptCurrentPos.x = (long) point.x;
	CompForm.ptCurrentPos.y = (long) point.y + GetCaret().GetCaretSize().cy - GetTextMetrics().GetHankakuHeight();

	if ( hIMC ){
		::ImmSetCompositionWindow( hIMC, &CompForm );
	}
	::ImmReleaseContext( GetHwnd() , hIMC );
}


/* IME�ҏW�G���A�̕\���t�H���g��ύX */
void CEditView::SetIMECompFormFont( void )
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	HIMC	hIMC = ::ImmGetContext( GetHwnd() );
	if ( hIMC ){
		::ImmSetCompositionFont( hIMC, &(m_pcEditWnd->GetLogfont()) );
	}
	::ImmReleaseContext( GetHwnd() , hIMC );
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �ĕϊ��E�ϊ��⏕
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*!
	@brief IME�̍ĕϊ�/�O��Q�ƂŁA�J�[�\���ʒu����O��200chars�ʂ����o����RECONVERTSTRING�𖄂߂�
	@param  pReconv  [out]  RECONVERTSTRING�\���̂ւ̃|�C���^�BNULL����
	@param  bUnicode        true�Ȃ��UNICODE�ō\���̂𖄂߂�
	@param  bDocumentFeed   true�Ȃ��IMR_DOCUMENTFEED�Ƃ��ď�������
	@return   RECONVERTSTRING�̃T�C�Y�B0�Ȃ�IME�͉������Ȃ�(�͂�)
	@date 2002.04.09 minfu
	@date 2010.03.16 Moca IMR_DOCUMENTFEED�Ή�
*/
LRESULT CEditView::SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode, bool bDocumentFeed)
{
	if( false == bDocumentFeed ){
		m_nLastReconvIndex = -1;
		m_nLastReconvLine  = -1;
	}
	
	//��`�I�𒆂͉������Ȃ�
	if( GetSelectionInfo().IsBoxSelecting() )
		return 0;

	// 2010.04.06 �r���[���[�h�ł͉������Ȃ�
	if( CAppMode::getInstance()->IsViewMode() ){
		return 0;
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �I��͈͂��擾                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//�I��͈͂��擾 -> ptSelect, ptSelectTo, nSelectedLen
	CLogicPoint	ptSelect;
	CLogicPoint	ptSelectTo;
	int			nSelectedLen;
	if( GetSelectionInfo().IsTextSelected() ){
		//�e�L�X�g���I������Ă���Ƃ�
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(GetSelectionInfo().m_sSelect.GetFrom(), &ptSelect);
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(GetSelectionInfo().m_sSelect.GetTo(), &ptSelectTo);
		
		// �I��͈͂������s�̎��A�P���W�b�N�s�ȓ��ɐ���
		if (ptSelectTo.y != ptSelect.y){
			if( bDocumentFeed ){
				// �b��F���I���Ƃ��ĐU����
				// ���P�āF�I��͈͂͒u�������̂ŁA�I��͈͂̑O���IME�ɓn��
				// ptSelectTo.y = ptSelectTo.y;
				ptSelectTo.x = ptSelect.x;
			}else{
				// 2010.04.06 �Ώۂ�ptSelect.y�̍s����J�[�\���s�ɕύX
				const CDocLine* pDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(GetCaret().GetCaretLogicPos().y);
				CLogicInt targetY = GetCaret().GetCaretLogicPos().y;
				// �J�[�\���s���������I���Ȃ�A���O�E����̍s��I��
				if( ptSelect.y == GetCaret().GetCaretLogicPos().y
						&& pDocLine && pDocLine->GetLengthWithoutEOL() == GetCaret().GetCaretLogicPos().x ){
					// �J�[�\�����㑤�s�� => ���̍s�B�s���J�[�\���ł�Shift+Up�Ȃ�
					targetY = t_min(m_pcEditDoc->m_cDocLineMgr.GetLineCount(),
						GetCaret().GetCaretLogicPos().y + 1);
					pDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(targetY);
				}else
				if( ptSelectTo.y == GetCaret().GetCaretLogicPos().y
						&& 0 == GetCaret().GetCaretLogicPos().x ){
					// �J�[�\���������s�� => �O�̍s�B �s����Shift+Down/Shift+End��Right�Ȃ�
					targetY = GetCaret().GetCaretLogicPos().y - 1;
					pDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(targetY);
				}
				// �I��͈͂�x�Ŏw��F������̓J�[�\���ł͂Ȃ��I��͈͊
				if(targetY == ptSelect.y){
					// ptSelect.x; ���ύX
					ptSelectTo.x = pDocLine ? pDocLine->GetLengthWithoutEOL() : 0;
				}else
				if(targetY == ptSelectTo.y){
					ptSelect.x = 0;
					// ptSelectTo.x; ���ύX
				}else{
					ptSelect.x = 0;
					ptSelectTo.x = pDocLine ? pDocLine->GetLengthWithoutEOL() : 0;
				}
				ptSelect.y = targetY;
				// ptSelectTo.y = targetY; �ȉ����g�p
			}
		}
	}
	else{
		//�e�L�X�g���I������Ă��Ȃ��Ƃ�
		m_pcEditDoc->m_cLayoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos(), &ptSelect);
		ptSelectTo = ptSelect;
	}
	nSelectedLen = ptSelectTo.x - ptSelect.x;
	// �ȉ� ptSelect.y ptSelect.x, nSelectedLen ���g�p

	//�h�L�������g�s�擾 -> pcCurDocLine
	CDocLine* pcCurDocLine = m_pcEditDoc->m_cDocLineMgr.GetLine(ptSelect.GetY2());
	if (NULL == pcCurDocLine )
		return 0;

	//�e�L�X�g�擾 -> pLine, nLineLen
	const int nLineLen = pcCurDocLine->GetLengthWithoutEOL();
	if ( 0 == nLineLen )
		return 0;
	const wchar_t* pLine = pcCurDocLine->GetPtr();

	// 2010.04.17 �s�����灩�I�����ƁuSelectTo�����s�̌��̈ʒu�v�ɂ��邽�ߔ͈͂𒲐�����
	// �t���[�J�[�\���I���ł��s�������ɃJ�[�\��������
	if( nLineLen < ptSelect.x ){
		// ���s���O��IME�ɓn���J�[�\���ʒu�Ƃ������Ƃɂ���
		ptSelect.x = CLogicInt(nLineLen);
		nSelectedLen = 0;
	}
	if( nLineLen <  ptSelect.x + nSelectedLen ){
		nSelectedLen = nLineLen - ptSelect.x;
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              �ĕϊ��͈́E�l���������C��                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//�ĕϊ��l��������J�n  //�s�̒��ōĕϊ���API�ɂ킽���Ƃ��镶����̊J�n�ʒu
	int nReconvIndex = 0;
	int nInsertCompLen = 0; // DOCUMENTFEED�p�B�ϊ����̕������dwStr�ɍ�����
	// I�̓J�[�\���@[]���I��͈�=dwTargetStrLen���Ƃ���
	// �s�F���{���I���܂��B
	// IME�F�ɂイ��
	// API�ɓn��������F���{���[�ɂイ��]I���܂��B

	// �I���J�n�ʒu���O��200(or 50)���������l��������ɂ���
	const int nReconvMaxLen = (bDocumentFeed ? 50 : 200); //$$�}�W�b�N�i���o�[����
	while (ptSelect.x - nReconvIndex > nReconvMaxLen) {
		nReconvIndex = t_max<int>(nReconvIndex+1, ::CharNextW_AnyBuild(pLine+nReconvIndex)-pLine);
	}
	
	//�ĕϊ��l��������I��  //�s�̒��ōĕϊ���API�ɂ킽���Ƃ��镶����̒���
	int nReconvLen = nLineLen - nReconvIndex;
	if ( (nReconvLen + nReconvIndex - ptSelect.x) > nReconvMaxLen ){
		const wchar_t*       p = pLine + ptSelect.x;
		const wchar_t* const q = pLine + ptSelect.x + nReconvMaxLen;
		while (p <= q) {
			p = t_max(p+1, const_cast<LPCWSTR>(::CharNextW_AnyBuild(p)));
		}
		nReconvLen = p - pLine - nReconvIndex;
	}
	
	//�Ώە�����̒���
	if ( ptSelect.x + nSelectedLen > nReconvIndex + nReconvLen ){
		// �l��������API�ɓn���Ȃ��̂ŁA�I��͈͂��k��
		nSelectedLen = nReconvLen + nReconvIndex - ptSelect.x;
	}
	
	if( bDocumentFeed ){
		// IMR_DOCUMENTFEED�ł́A�ĕϊ��Ώۂ�IME����擾�������͒�������
		nInsertCompLen = auto_strlen(m_szComposition);
		if( 0 == nInsertCompLen ){
			// 2��Ă΂��̂ŁAm_szComposition�Ɋo���Ă���
			HWND hwnd = GetHwnd();
			HIMC hIMC = ::ImmGetContext( hwnd );
			if( !hIMC ){
				return 0;
			}
			auto_memset(m_szComposition, _T('\0'), _countof(m_szComposition));
			LONG immRet = ::ImmGetCompositionString(hIMC, GCS_COMPSTR, m_szComposition, _countof(m_szComposition));
			if( immRet == IMM_ERROR_NODATA || immRet == IMM_ERROR_GENERAL ){
				m_szComposition[0] = _T('\0');
			}
			::ImmReleaseContext( hwnd, hIMC );
			nInsertCompLen = auto_strlen(m_szComposition);
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �\���̐ݒ�v�f                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//�s�̒��ōĕϊ���API�ɂ킽���Ƃ��镶����̒���
	int         cbReconvLenWithNull; // byte
	DWORD       dwReconvTextLen;    // CHARs
	DWORD       dwReconvTextInsLen; // CHARs
	DWORD       dwCompStrOffset;    // byte
	DWORD       dwCompStrLen;       // CHARs
	DWORD       dwInsByteCount = 0; // byte
	CNativeW    cmemBuf1;
	const void* pszReconv; 
	const void* pszInsBuffer;

	//UNICODE��UNICODE
	if(bUnicode){
		const WCHAR* pszCompInsStr = L"";
		int nCompInsStr   = 0;
		if( nInsertCompLen ){
			pszCompInsStr = to_wchar( m_szComposition );
			nCompInsStr   = wcslen( pszCompInsStr );
		}
		dwInsByteCount      = nCompInsStr * sizeof(wchar_t);
		dwReconvTextLen     = nReconvLen;
		dwReconvTextInsLen  = dwReconvTextLen + nCompInsStr;                 //reconv�����񒷁B�����P�ʁB
		cbReconvLenWithNull = (dwReconvTextInsLen + 1) * sizeof(wchar_t);    //reconv�f�[�^���B�o�C�g�P�ʁB
		dwCompStrOffset     = (Int)(ptSelect.x - nReconvIndex) * sizeof(wchar_t);    //comp�I�t�Z�b�g�B�o�C�g�P�ʁB
		dwCompStrLen        = nSelectedLen + nCompInsStr;                            //comp�����񒷁B�����P�ʁB
		pszReconv           = reinterpret_cast<const void*>(pLine + nReconvIndex);   //reconv������ւ̃|�C���^�B
		pszInsBuffer        = pszCompInsStr;
	}
	//UNICODE��ANSI
	else{
		const wchar_t* pszReconvSrc =  pLine + nReconvIndex;

		//�l��������̊J�n����Ώە�����̊J�n�܂� -> dwCompStrOffset
		if( ptSelect.x - nReconvIndex > 0 ){
			cmemBuf1.SetString(pszReconvSrc, ptSelect.x - nReconvIndex);
			CShiftJis::UnicodeToSJIS(cmemBuf1._GetMemory());
			dwCompStrOffset = cmemBuf1._GetMemory()->GetRawLength();				//comp�I�t�Z�b�g�B�o�C�g�P�ʁB
		}else{
			dwCompStrOffset = 0;
		}
		
		pszInsBuffer = "";
		//�Ώە�����̊J�n����Ώە�����̏I���܂� -> dwCompStrLen
		if (nSelectedLen > 0 ){
			cmemBuf1.SetString(pszReconvSrc + ptSelect.x, nSelectedLen);  
			CShiftJis::UnicodeToSJIS(cmemBuf1._GetMemory());
			dwCompStrLen = cmemBuf1._GetMemory()->GetRawLength();					//comp�����񒷁B�����P�ʁB
		}else if(nInsertCompLen > 0){
			// nSelectedLen �� nInsertCompLen �������w�肳��邱�Ƃ͂Ȃ��͂�
			const ACHAR* pComp = to_achar(m_szComposition);
			pszInsBuffer = pComp;
			dwInsByteCount = strlen( pComp );
			dwCompStrLen = dwInsByteCount;
		}else{
			dwCompStrLen = 0;
		}
		
		//�l�������񂷂ׂ�
		cmemBuf1.SetString(pszReconvSrc , nReconvLen );
		CShiftJis::UnicodeToSJIS(cmemBuf1._GetMemory());
		
		dwReconvTextLen    = cmemBuf1._GetMemory()->GetRawLength();				//reconv�����񒷁B�����P�ʁB
		dwReconvTextInsLen = dwReconvTextLen + dwInsByteCount;						//reconv�����񒷁B�����P�ʁB
		cbReconvLenWithNull = cmemBuf1._GetMemory()->GetRawLength() + dwInsByteCount + sizeof(char);		//reconv�f�[�^���B�o�C�g�P�ʁB
		
		pszReconv = reinterpret_cast<const void*>(cmemBuf1._GetMemory()->GetRawPtr());	//reconv������ւ̃|�C���^
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �\���̐ݒ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if ( NULL != pReconv) {
		//�ĕϊ��\���̂̐ݒ�
		DWORD dwOrgSize = pReconv->dwSize;
		// 2010.03.17 Moca dwSize��pReconv��p�ӂ��鑤(IME��)���ݒ�
		//     �̂͂��Ȃ̂� Win XP+IME2002+TSF �ł� dwSize��0�ő����Ă���
		if( dwOrgSize != 0 && dwOrgSize < sizeof(*pReconv) + cbReconvLenWithNull ){
			// �o�b�t�@�s��
			m_szComposition[0] = _T('\0');
			return 0;
		}
		else if( 0 == dwOrgSize ){
			pReconv->dwSize = sizeof(*pReconv) + cbReconvLenWithNull;
		}
		pReconv->dwVersion         = 0;
		pReconv->dwStrLen          = dwReconvTextInsLen;	//�����P��
		pReconv->dwStrOffset       = sizeof(*pReconv) ;
		pReconv->dwCompStrLen      = dwCompStrLen;		//�����P��
		pReconv->dwCompStrOffset   = dwCompStrOffset;	//�o�C�g�P��
		pReconv->dwTargetStrLen    = dwCompStrLen;		//�����P��
		pReconv->dwTargetStrOffset = dwCompStrOffset;	//�o�C�g�P��
		
		// 2004.01.28 Moca �k���I�[�̏C��
		if( bUnicode ){
			WCHAR* p = (WCHAR*)(pReconv + 1);
			if( dwInsByteCount ){
				// �J�[�\���ʒu�ɁA���͒�IME�f�[�^��}��
				CHAR* pb = (CHAR*)p;
				CopyMemory(pb, pszReconv, dwCompStrOffset);
				pb += dwCompStrOffset;
				CopyMemory(pb, pszInsBuffer, dwInsByteCount);
				pb += dwInsByteCount;
				CopyMemory(pb, ((char*)pszReconv) + dwCompStrOffset,
					dwReconvTextLen*sizeof(wchar_t) - dwCompStrOffset);
			}else{
				CopyMemory(p, pszReconv, cbReconvLenWithNull - sizeof(wchar_t));
			}
			// \0������Ɖ����Ȃ��ɂȂ邱�Ƃ�����
			for( DWORD i = 0; i < dwReconvTextInsLen; i++ ){
				if( p[i] == 0 ){
					p[i] = L' ';
				}
			}
			p[dwReconvTextInsLen] = L'\0';
		}else{
			ACHAR* p = (ACHAR*)(pReconv + 1);
			if( dwInsByteCount ){
				CHAR* pb = p;
				CopyMemory(p, pszReconv, dwCompStrOffset);
				pb += dwCompStrOffset;
				CopyMemory(pb, pszInsBuffer, dwInsByteCount);
				pb += dwInsByteCount;
				CopyMemory(pb, ((char*)pszReconv) + dwCompStrOffset,
					dwReconvTextLen - dwCompStrOffset);
			}else{
				CopyMemory(p, pszReconv, cbReconvLenWithNull - sizeof(char));
			}
			// \0������Ɖ����Ȃ��ɂȂ邱�Ƃ�����
			for( DWORD i = 0; i < dwReconvTextInsLen; i++ ){
				if( p[i] == 0 ){
					p[i] = ' ';
				}
			}
			p[dwReconvTextInsLen]='\0';
		}
	}
	
	if( false == bDocumentFeed ){
		// �ĕϊ����̕ۑ�
		m_nLastReconvIndex = nReconvIndex;
		m_nLastReconvLine  = ptSelect.y;
	}
	if( bDocumentFeed && pReconv ){
		m_szComposition[0] = _T('\0');
	}
	return sizeof(RECONVERTSTRING) + cbReconvLenWithNull;
}

/*�ĕϊ��p �G�f�B�^��̑I��͈͂�ύX���� 2002.04.09 minfu */
LRESULT CEditView::SetSelectionFromReonvert(const PRECONVERTSTRING pReconv, bool bUnicode){
	
	// �ĕϊ���񂪕ۑ�����Ă��邩
	if ( (m_nLastReconvIndex < 0) || (m_nLastReconvLine < 0))
		return 0;

	if ( GetSelectionInfo().IsTextSelected()) 
		GetSelectionInfo().DisableSelectArea( true );

	if( 0 != pReconv->dwVersion ){
		return 0;
	}
	
	DWORD dwOffset, dwLen;

	//UNICODE��UNICODE
	if(bUnicode){
		dwOffset = pReconv->dwCompStrOffset/sizeof(WCHAR);	//0�܂��̓f�[�^���B�o�C�g�P�ʁB�������P��
		dwLen    = pReconv->dwCompStrLen;					//0�܂��͕����񒷁B�����P�ʁB
	}
	//ANSI��UNICODE
	else{
		CNativeA	cmemBuf;

		//�l��������̊J�n����Ώە�����̊J�n�܂�
		if( pReconv->dwCompStrOffset > 0){
			if( pReconv->dwSize < (pReconv->dwStrOffset + pReconv->dwCompStrOffset) ){
				return 0;
			}
			// 2010.03.17 sizeof(pReconv)+1�ł͂Ȃ�dwStrOffset�𗘗p����悤��
			const char* p=((const char*)(pReconv)) + pReconv->dwStrOffset;
			cmemBuf.SetString(p, pReconv->dwCompStrOffset ); 
			CShiftJis::SJISToUnicode(cmemBuf._GetMemory());
			dwOffset = cmemBuf._GetMemory()->GetRawLength()/sizeof(WCHAR);
		}else{
			dwOffset = 0;
		}

		//�Ώە�����̊J�n����Ώە�����̏I���܂�
		if( pReconv->dwCompStrLen > 0 ){
			if( pReconv->dwSize <
					pReconv->dwStrOffset + pReconv->dwCompStrOffset + pReconv->dwCompStrLen*sizeof(char) ){
				return 0;
			}
			// 2010.03.17 sizeof(pReconv)+1�ł͂Ȃ�dwStrOffset�𗘗p����悤��
			const char* p= ((const char*)pReconv) + pReconv->dwStrOffset;
			cmemBuf.SetString(p + pReconv->dwCompStrOffset, pReconv->dwCompStrLen); 
			CShiftJis::SJISToUnicode(cmemBuf._GetMemory());
			dwLen = cmemBuf._GetMemory()->GetRawLength()/sizeof(WCHAR);
		}else{
			dwLen = 0;
		}
	}
	
	//�I���J�n�̈ʒu���擾
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		CLogicPoint(m_nLastReconvIndex + dwOffset, m_nLastReconvLine),
		GetSelectionInfo().m_sSelect.GetFromPointer()
	);

	//�I���I���̈ʒu���擾
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout(
		CLogicPoint(m_nLastReconvIndex + dwOffset + dwLen, m_nLastReconvLine),
		GetSelectionInfo().m_sSelect.GetToPointer()
	);

	// �P��̐擪�ɃJ�[�\�����ړ�
	GetCaret().MoveCursor( GetSelectionInfo().m_sSelect.GetFrom(), true );

	//�I��͈͍ĕ`�� 
	GetSelectionInfo().DrawSelectArea();

	// �ĕϊ����̔j��
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;

	return 1;

}
