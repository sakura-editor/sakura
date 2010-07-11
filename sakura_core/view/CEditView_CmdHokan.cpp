/*!	@file
	@brief CEditView�N���X�̕⊮�֘A�R�}���h�����n�֐��Q

	@author genta
	@date	2005/01/10 �쐬
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
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "doc/CDocReader.h"
#include "debug/Debug.h"
#include "charset/charcode.h"  // 2006.06.28 rastiv
#include "window/CEditWnd.h"
#include "parse/CWordParse.h"
#include "sakura_rc.h"

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
		 && nCommand != F_WCHAR		//	��������
		 && nCommand != F_IME_CHAR	//	��������
		 ){
			m_pcEditWnd->m_cHokanMgr.Hide();
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
	if( GetDllShareData().m_Common.m_sHelper.m_bUseHokan && !m_bExecutingKeyMacro ){ /* �L�[�{�[�h�}�N���̎��s�� */
		CNativeW	cmemData;

		/* �J�[�\�����O�̒P����擾 */
		if( 0 < GetParser().GetLeftWord( &cmemData, 100 ) ){
			ShowHokanMgr( cmemData, FALSE );
		}else{
			if( m_bHokan ){
				m_pcEditWnd->m_cHokanMgr.Hide();
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
void CEditView::ShowHokanMgr( CNativeW& cmemData, BOOL bAutoDecided )
{
	/* �⊮�Ώۃ��[�h���X�g�𒲂ׂ� */
	CNativeW	cmemHokanWord;
	int			nKouhoNum;
	POINT		poWin;
	/* �⊮�E�B���h�E�̕\���ʒu���Z�o */
	poWin.x = GetTextArea().GetAreaLeft()
			 + (Int)(GetCaret().GetCaretLayoutPos().GetX2() - GetTextArea().GetViewLeftCol())
			  * GetTextMetrics().GetHankakuDx();
	poWin.y = GetTextArea().GetAreaTop()
			 + (Int)(GetCaret().GetCaretLayoutPos().GetY2() - GetTextArea().GetViewTopLine())
			  * GetTextMetrics().GetHankakuDy();
	this->ClientToScreen( &poWin );
	poWin.x -= (
		cmemData.GetStringLength()
		 * GetTextMetrics().GetHankakuDx()
	);

	/*	�⊮�E�B���h�E��\��
		�������AbAutoDecided == TRUE�̏ꍇ�́A�⊮��₪1�̂Ƃ��́A�E�B���h�E��\�����Ȃ��B
		�ڂ����́ASearch()�̐������Q�Ƃ̂��ƁB
	*/
	CNativeW* pcmemHokanWord;
	if ( bAutoDecided ){
		pcmemHokanWord = &cmemHokanWord;
	}
	else {
		pcmemHokanWord = NULL;
	}
	nKouhoNum = m_pcEditWnd->m_cHokanMgr.CHokanMgr::Search(
		&poWin,
		GetTextMetrics().GetHankakuHeight(),
		GetTextMetrics().GetHankakuDx(),
		cmemData.GetStringPtr(),
		m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_szHokanFile,
		m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bHokanLoHiCase,
		m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bUseHokanByFile, // 2003.06.22 Moca
		pcmemHokanWord
	);
	/* �⊮���̐��ɂ���ē����ς��� */
	if (nKouhoNum <= 0) {				//	��△��
		if( m_bHokan ){
			m_pcEditWnd->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
			// 2003.06.25 Moca ���s���Ă���A�r�[�v�����o���ĕ⊮�I���B
			ErrorBeep();
		}
	}
	else if( bAutoDecided && nKouhoNum == 1){ //	���1�̂݁��m��B
		if( m_bHokan ){
			m_pcEditWnd->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
		}
		// 2004.05.14 Moca CHokanMgr::Search���ŉ��s���폜����悤�ɂ��A���ڏ���������̂���߂�

		GetCommander().Command_WordDeleteToStart();
		GetCommander().Command_INSTEXT( TRUE, cmemHokanWord.GetStringPtr(), cmemHokanWord.GetStringLength(), TRUE );
	}
	else {
		m_bHokan = TRUE;
	}
	
	//	�⊮�I���B
	if ( !m_bHokan ){
		GetDllShareData().m_Common.m_sHelper.m_bUseHokan = FALSE;	//	���͕⊮�I���̒m�点
	}
}



/*!
	�ҏW���f�[�^������͕⊮�L�[���[�h�̌���
	CHokanMgr����Ă΂��

	@return ��␔

	@author Moca
	@date 2003.06.25

	@date 2005/01/10 genta  CEditView_Command����ړ�
	@date 2007/10/17 kobake �ǂ݂₷���悤�Ƀl�X�g��󂭂��܂����B
	@date 2008.07.25 nasukoji �啶���������𓯈ꎋ�̏ꍇ�ł����̐U�邢���Ƃ��͊��S��v�Ō���
	@date 2008.10.11 syat ���{��̕⊮
	@date 2010.06.16 Moca �Ђ炪�Ȃő��s����ꍇ�A���O�������ɐ���
*/
int CEditView::HokanSearchByFile(
	const wchar_t*	pszKey,			//!< [in]
	BOOL			bHokanLoHiCase,	//!< [in] �p�啶���������𓯈ꎋ����
	CNativeW**		ppcmemKouho,	//!< [in,out] ���
	int				nKouhoNum,		//!< [in] ppcmemKouho�̂��łɓ����Ă��鐔
	int				nMaxKouho		//!< [in] Max��␔(0==������)
){
	const int nKeyLen = wcslen( pszKey );
	int nLines = m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	int j, nWordLen, nLineLen, nRet, nCharSize, nWordBegin, nWordLenStop;

	const wchar_t* pszLine;
	const wchar_t* word;

	CLogicPoint ptCur = GetCaret().GetCaretLogicPos(); //�����J�[�\���ʒu
	bool bKeyStartWithMark;			//�L�[���L���Ŏn�܂邩
	bool bWordStartWithMark;		//��₪�L���Ŏn�܂邩

	// �L�[�̐擪���L��(#$@\)���ǂ�������
	bKeyStartWithMark = ( wcschr( L"$@#\\", pszKey[0] ) != NULL ? true : false );

	for( CLogicInt i = CLogicInt(0); i < nLines; i++  ){
		pszLine = CDocReader(m_pcEditDoc->m_cDocLineMgr).GetLineStrWithoutEOL( i, &nLineLen );

		for( j = 0; j < nLineLen; j += nCharSize ){
			nCharSize = CNativeW::GetSizeOfChar( pszLine, nLineLen, j );

			// ���p�L���͌��Ɋ܂߂Ȃ�
			if ( pszLine[j] < 0x00C0 && !IS_KEYWORD_CHAR( pszLine[j] ) )continue;

			// �L�[�̐擪���L���ȊO�̏ꍇ�A�L���Ŏn�܂�P��͌�₩��͂���
			if( !bKeyStartWithMark && wcschr( L"$@#\\", pszLine[j] ) != NULL )continue;

			// ������ގ擾
			ECharKind kindPre = CWordParse::WhatKindOfChar( pszLine, nLineLen, j );	// ������ގ擾

			// �S�p�L���͌��Ɋ܂߂Ȃ�
			if ( kindPre == CK_ZEN_SPACE || kindPre == CK_ZEN_NOBASU || kindPre == CK_ZEN_DAKU ||
				 kindPre == CK_ZEN_KIGO  || kindPre == CK_ZEN_SKIGO )continue;

			bWordStartWithMark = ( wcschr( L"$@#\\", pszLine[j] ) != NULL ? true : false );

			nWordBegin = j;
			// ���P��̏I���ʒu�����߂�
			nWordLen = nCharSize;
			nWordLenStop = -1; // ���艼�������p�P��̏I���B-1�͖���
			for( j += nCharSize; j < nLineLen; j += nCharSize ){
				nCharSize = CNativeW::GetSizeOfChar( pszLine, nLineLen, j );

				// ���p�L���͊܂߂Ȃ�
				if ( pszLine[j] < 0x00C0 && !IS_KEYWORD_CHAR( pszLine[j] ) )break;

				// ������ގ擾
				ECharKind kindCur = CWordParse::WhatKindOfChar( pszLine, nLineLen, j );
				// �S�p�L���͌��Ɋ܂߂Ȃ�
				if ( kindCur == CK_ZEN_SPACE || kindCur == CK_ZEN_KIGO || kindCur == CK_ZEN_SKIGO ){
					break;
				}

				// ������ނ��ς������P��̐؂�ڂƂ���
				ECharKind kindMerge = CWordParse::WhatKindOfTwoChars( kindPre, kindCur );
				if ( kindMerge == CK_NULL ) {	// kindPre��kindCur���ʎ�
					if( kindCur == CK_HIRA ) {
						kindMerge = kindCur;		// �Ђ炪�ȂȂ瑱�s
						// 2010.06.16 Moca �����̂ݑ��艼�������Ɋ܂߂�
						if( kindPre != CK_ZEN_ETC ) {
							nWordLenStop = nWordLen;
						}
					}else if( bKeyStartWithMark && bWordStartWithMark && kindPre == CK_ETC ){
						kindMerge = kindCur;		// �L���Ŏn�܂�P��͐������ɂ߂�
					}else{
						j -= nCharSize;
						break;						// ����ȊO�͒P��̐؂��
					}
				}

				kindPre = kindMerge;
				nWordLen += nCharSize;				// ���̕�����
			}

			if( 0 < nWordLenStop ){
				nWordLen  = nWordLenStop;
			}


			// CDicMgr���̐����ɂ�蒷������P��͖�������
			if( nWordLen > 1020 ){
				continue;
			}
			if( nKeyLen > nWordLen ) continue;

			// ���P��̊J�n�ʒu�����߂�
			word = pszLine + nWordBegin;

			// �L�[�Ɣ�r����
			if( bHokanLoHiCase ){
				nRet = auto_memicmp( pszKey, word, nKeyLen );
			}else{
				nRet = auto_memcmp( pszKey, word, nKeyLen );
			}
			if( nRet!=0 )continue;

			// �J�[�\���ʒu�̒P��͌�₩��͂���
			if( ptCur.y == i && nWordBegin <= ptCur.x && ptCur.x <= nWordBegin + nWordLen ){	// 2010.02.20 syat �C��// 2008.11.09 syat �C��
				continue;
			}

			if( NULL == *ppcmemKouho ){
				*ppcmemKouho = new CNativeW;
				(*ppcmemKouho)->SetString( word, nWordLen );
				(*ppcmemKouho)->AppendString( L"\n" );
				++nKouhoNum;
			}
			else{
				// �d�����Ă�����ǉ����Ȃ�
				int nLen;
				const wchar_t* ptr = (*ppcmemKouho)->GetStringPtr( &nLen );
				int nPosKouho;
				nRet = 1;
				// 2008.07.25 nasukoji	�啶���������𓯈ꎋ�̏ꍇ�ł����̐U�邢���Ƃ��͊��S��v�Ō���
				if( nWordLen < nLen ){
					if( L'\n' == ptr[nWordLen] && 0 == auto_memcmp( ptr, word, nWordLen )  ){
						nRet = 0;
					}else{
						const int nPosKouhoMax = nLen - nWordLen - 1;
						for( nPosKouho = 1; nPosKouho < nPosKouhoMax; nPosKouho++ ){
							if( ptr[nPosKouho] == L'\n' ){
								if( ptr[nPosKouho + nWordLen + 1] == L'\n' ){
									if( 0 == auto_memcmp( &ptr[nPosKouho + 1], word, nWordLen) ){
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
				if( 0 == nRet ){
					continue;
				}
				//2007.10.17 kobake ���������[�N���Ă܂����B�C���B
				(*ppcmemKouho)->AppendString( word, nWordLen );
				(*ppcmemKouho)->AppendString( L"\n" );
				++nKouhoNum;
			}
			if( 0 != nMaxKouho && nMaxKouho <= nKouhoNum ){
				return nKouhoNum;
			}
			
		}
	}
	return nKouhoNum;
}

