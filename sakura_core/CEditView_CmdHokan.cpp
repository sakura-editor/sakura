//	$Id$
/*!	@file
	@brief CEditView�N���X�̕⊮�֘A�R�}���h�����n�֐��Q

	@author genta
	@date	2005/01/10 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, asa-o
	Copyright (C) 2003, Moca
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "stdafx.h"
#include "sakura_rc.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "debug.h"
#include "etc_uty.h"

/*!
	@brief �R�}���h��M�O�⊮����
	
	�⊮�E�B���h�E�̔�\��

	@date 2005.01.10 genta �֐���
*/
void CEditView::PreprocessCommand_hokan( int nCommand )
{
	/* �⊮�E�B���h�E���\������Ă���Ƃ��A���ʂȏꍇ�������ăE�B���h�E���\���ɂ��� */
	if( m_bHokan ){
		if( nCommand != F_HOKAN		//	�⊮�J�n�E�I���R�}���h
		 && nCommand != F_CHAR		//	��������
		 && nCommand != F_IME_CHAR	//	��������
		 ){
			m_pcEditDoc->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
		}
	}
}

/*!
	�R�}���h���s��⊮����

	@author Moca
	@date 2005.01.10 genta �֐���
*/
void CEditView::PostprocessCommand_hokan(void)
{
	if( m_pShareData->m_Common.m_bUseHokan
 	 && FALSE == m_bExecutingKeyMacro	/* �L�[�{�[�h�}�N���̎��s�� */
	){
		CMemory	cmemData;

		/* �J�[�\�����O�̒P����擾 */
		if( 0 < GetLeftWord( &cmemData, 100 ) ){
			ShowHokanMgr( cmemData, FALSE );
		}else{
			if( m_bHokan ){
				m_pcEditDoc->m_cHokanMgr.Hide();
				m_bHokan = FALSE;
			}
		}
	}
}

/*!	�⊮�E�B���h�E��\������
	�E�B���h�E��\��������́AHokanMgr�ɔC����̂ŁAShowHokanMgr�̒m��Ƃ���ł͂Ȃ��B
	
	@param cmemData [in] �⊮���錳�̃e�L�X�g �uAb�v�Ȃǂ�����B
	@param bAutoDecided [in] ��₪1��������m�肷��

	@date 2005.01.10 genta CEditView_Command����ړ�
*/
void CEditView::ShowHokanMgr( CMemory& cmemData, BOOL bAutoDecided )
{
	/* �⊮�Ώۃ��[�h���X�g�𒲂ׂ� */
	CMemory		cmemHokanWord;
	int			nKouhoNum;
	POINT		poWin;
	/* �⊮�E�B���h�E�̕\���ʒu���Z�o */
	poWin.x = m_nViewAlignLeft
			 + (m_nCaretPosX - m_nViewLeftCol)
			  * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	poWin.y = m_nViewAlignTop
			 + (m_nCaretPosY - m_nViewTopLine)
			  * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
	::ClientToScreen( m_hWnd, &poWin );
	poWin.x -= (
		cmemData.GetLength()
		 * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )
	);

	/*	�⊮�E�B���h�E��\��
		�������AbAutoDecided == TRUE�̏ꍇ�́A�⊮��₪1�̂Ƃ��́A�E�B���h�E��\�����Ȃ��B
		�ڂ����́ASearch()�̐������Q�Ƃ̂��ƁB
	*/
	CMemory* pcmemHokanWord;
	if ( bAutoDecided ){
		pcmemHokanWord = &cmemHokanWord;
	}
	else {
		pcmemHokanWord = NULL;
	}
	nKouhoNum = m_pcEditDoc->m_cHokanMgr.Search(
		&poWin,
		m_nCharHeight,
		m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace,
		cmemData.GetPtr(),
		m_pcEditDoc->GetDocumentAttribute().m_szHokanFile,
		m_pcEditDoc->GetDocumentAttribute().m_bHokanLoHiCase,
		m_pcEditDoc->GetDocumentAttribute().m_bUseHokanByFile, // 2003.06.22 Moca
		pcmemHokanWord
	);
	/* �⊮���̐��ɂ���ē����ς��� */
	if (nKouhoNum <= 0) {				//	��△��
		if( m_bHokan ){
			m_pcEditDoc->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
			// 2003.06.25 Moca ���s���Ă���A�r�[�v�����o���ĕ⊮�I���B
			::MessageBeep( MB_ICONHAND );
		}
	}
	else if( bAutoDecided && nKouhoNum == 1){ //	���1�̂݁��m��B
		if( m_bHokan ){
			m_pcEditDoc->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
		}
		// 2004.05.14 Moca CHokanMgr::Search���ŉ��s���폜����悤�ɂ��A���ڏ���������̂���߂�
//		pszKouhoWord = cmemHokanWord.GetPtr( &nKouhoWordLen );
//		pszKouhoWord[nKouhoWordLen] = '\0';
		Command_WordDeleteToStart();
		Command_INSTEXT( TRUE, cmemHokanWord.GetPtr(), cmemHokanWord.GetLength(), TRUE );
	}
	else {
		m_bHokan = TRUE;
	}
	
	//	�⊮�I���B
	if ( !m_bHokan ){
		m_pShareData->m_Common.m_bUseHokan = FALSE;	//	���͕⊮�I���̒m�点
	}
}


/*!	���͕⊮
	Ctrl+Space�ł����ɓ����B
	CEditView::m_bHokan�F ���ݕ⊮�E�B���h�E���\������Ă��邩��\���t���O�B
	m_Common.m_bUseHokan�F���ݕ⊮�E�B���h�E���\������Ă���ׂ����ۂ�������킷�t���O�B

    @date 2001/06/19 asa-o �p�啶���������𓯈ꎋ����
                     ��₪1�̂Ƃ��͂���Ɋm�肷��
	@date 2001/06/14 asa-o �Q�ƃf�[�^�ύX
	                 �J���v���p�e�B�V�[�g���^�C�v�ʂɕύX
	@date 2000/09/15 JEPRO [Esc]�L�[��[x]�{�^���ł����~�ł���悤�ɕύX
	@date 2005/01/10 genta CEditView_Command����ړ�
*/
void CEditView::Command_HOKAN( void )
{
	if (m_pShareData->m_Common.m_bUseHokan == FALSE){
		m_pShareData->m_Common.m_bUseHokan = TRUE;
	}
retry:;
	/* �⊮���ꗗ�t�@�C�����ݒ肳��Ă��Ȃ��Ƃ��́A�ݒ肷��悤�ɑ����B */
	// 2003.06.22 Moca �t�@�C�������猟������ꍇ�ɂ͕⊮�t�@�C���̐ݒ�͕K�{�ł͂Ȃ�
	if( m_pcEditDoc->GetDocumentAttribute().m_bUseHokanByFile == FALSE &&
		0 == lstrlen( m_pcEditDoc->GetDocumentAttribute().m_szHokanFile 
	) ){
		::MessageBeep( MB_ICONHAND );
		if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
			"�⊮���ꗗ�t�@�C�����ݒ肳��Ă��܂���B\n�������ݒ肵�܂���?"
		) ){
			/* �^�C�v�ʐݒ� �v���p�e�B�V�[�g */
			if( !m_pcEditDoc->OpenPropertySheetTypes( 2, m_pcEditDoc->GetDocumentType() ) ){
				return;
			}
			goto retry;
		}
	}

	CMemory		cmemData;
	/* �J�[�\�����O�̒P����擾 */
	if( 0 < GetLeftWord( &cmemData, 100 ) ){
		ShowHokanMgr( cmemData, TRUE );
	}else{
		::MessageBeep( MB_ICONHAND );
		m_pShareData->m_Common.m_bUseHokan = FALSE;	//	���͕⊮�I���̂��m�点
	}
	return;
}

/*!
	�ҏW���f�[�^������͕⊮�L�[���[�h�̌���
	CHokanMgr����Ă΂��

	@return ��␔

	@author Moca
	@date 2003.06.25

	@date 2005/01/10 genta CEditView_Command����ړ�
*/
int CEditView::HokanSearchByFile(
		const char* pszKey,
		BOOL		bHokanLoHiCase,	//!< �p�啶���������𓯈ꎋ����
		CMemory**	ppcmemKouho,	//!< [IN/OUT] ���
		int			nKouhoNum,		//!< ppcmemKouho�̂��łɓ����Ă��鐔
		int			nMaxKouho		//!< Max��␔(0==������)
){
	const int nKeyLen = lstrlen( pszKey );
	int nLines = m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	int i, j, nWordLen, nLineLen, nRet;
	int nCurX, nCurY; // �����J�[�\���ʒu
	const char* pszLine;
	const char* word;
	char *pszWork;
	nCurX = m_nCaretPosX_PHY;
	nCurY = m_nCaretPosY_PHY;
	
	for( i = 0; i < nLines; i++  ){
		pszLine = m_pcEditDoc->m_cDocLineMgr.GetLineStrWithoutEOL( i, &nLineLen );
		for( j = 0; j < nLineLen; j++ ){
			if( IS_KEYWORD_CHAR( (unsigned char)(pszLine[j]) ) ){
				word = pszLine + j;
				for( j++, nWordLen = 1;j < nLineLen && IS_KEYWORD_CHAR( (unsigned char)(pszLine[j]) ); j++ ){
					nWordLen++;
				}
				if( nWordLen > 1020 ){ // CDicMgr���̐����ɂ�蒷������P��͖�������
					continue;
				}
				if( nKeyLen <= nWordLen ){
					if( bHokanLoHiCase ){
						nRet = memicmp( pszKey, word, nKeyLen );
					}else{
						nRet = memcmp( pszKey, word, nKeyLen );
					}
					if( 0 == nRet ){
						// �J�[�\���ʒu�̒P��͌�₩��͂���
						if( nCurY == i && nCurX <= j && j - nWordLen <= nCurX ){
							continue;
						}
						if( NULL == *ppcmemKouho ){
							*ppcmemKouho = new CMemory;
							(*ppcmemKouho)->SetData( word, nWordLen );
							(*ppcmemKouho)->AppendSz( "\n" );
							++nKouhoNum;
						}else{
							// �d�����Ă�����ǉ����Ȃ�
							int nLen;
							const char* ptr = (*ppcmemKouho)->GetPtr( &nLen );
							int nPosKouho;
							nRet = 1;
							if( bHokanLoHiCase ){
								if( nWordLen < nLen ){
									if( '\n' == ptr[nWordLen] && 0 == memicmp( ptr, word, nWordLen )  ){
										nRet = 0;
									}else{
										int nPosKouhoMax = nLen - nWordLen - 1;
										for( nPosKouho = 1; nPosKouho < nPosKouhoMax; nPosKouho++ ){
											if( ptr[nPosKouho] == '\n' ){
												if( ptr[nPosKouho + nWordLen + 1] == '\n' ){
													if( 0 == memicmp( &ptr[nPosKouho + 1], word, nWordLen) ){
														nRet = 0;
														break;
													}else{
														nPosKouho += nWordLen;
													}
												}
											}
										}
									}
								}
							}else{
								if( nWordLen < nLen ){
									if( '\n' == ptr[nWordLen] && 0 == memcmp( ptr, word, nWordLen )  ){
										nRet = 0;
									}else{
										int nPosKouhoMax = nLen - nWordLen - 1;
										for( nPosKouho = 1; nPosKouho < nPosKouhoMax; nPosKouho++ ){
											if( ptr[nPosKouho] == '\n' ){
												if( ptr[nPosKouho + nWordLen + 1] == '\n' ){
													if( 0 == memcmp( &ptr[nPosKouho + 1], word, nWordLen) ){
														nRet = 0;
														break;
													}else{
														nPosKouho += nWordLen;
													}
												}
											}
										}
									}
								}
							}
							if( 0 == nRet ){
								continue;
							}
							pszWork = new char[nWordLen + 1];
							memcpy( pszWork, word, nWordLen );
							pszWork[nWordLen] = '\n';
							(*ppcmemKouho)->Append( pszWork, nWordLen + 1 );
							++nKouhoNum;
						}
						if( 0 != nMaxKouho && nMaxKouho <= nKouhoNum ){
							return nKouhoNum;
						}
					}
				}
			}else if( _IS_SJIS_1( (unsigned char)pszLine[j] ) ){
				j++;
				continue;
			}
		}
	}
	return nKouhoNum;
}
/*[EOF]*/
