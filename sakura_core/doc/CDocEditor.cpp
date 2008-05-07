#include "stdafx.h"
#include "CDocEditor.h"
#include "CClipboard.h"
#include "debug/CRunningTimer.h"

CDocEditor::CDocEditor(CEditDoc* pcDoc)
: m_pcDocRef(pcDoc)
, m_bInsMode( true )	// Oct. 2, 2005 genta
, m_cNewLineCode( EOL_CRLF )		//	New Line Type
, m_bIsDocModified( false )	/* �ύX�t���O */ // Jan. 22, 2002 genta �^�ύX
{
	//	Oct. 2, 2005 genta �}�����[�h
	this->SetInsMode( GetDllShareData().m_Common.m_sGeneral.m_bIsINSMode );
}


/*! �ύX�t���O�̐ݒ�

	@param flag [in] �ݒ肷��l�Dtrue: �ύX�L�� / false: �ύX����
	@param redraw [in] true: �^�C�g���̍ĕ`����s�� / false: �s��Ȃ�
	
	@author genta
	@date 2002.01.22 �V�K�쐬
*/
void CDocEditor::SetModified( bool flag, bool redraw)
{
	if( m_bIsDocModified == flag )	//	�ύX���Ȃ���Ή������Ȃ�
		return;

	m_bIsDocModified = flag;
	if( redraw )
		m_pcDocRef->m_pcEditWnd->UpdateCaption();
}

void CDocEditor::OnBeforeLoad(const SLoadInfo& sLoadInfo)
{
	//�r���[�̃e�L�X�g�I������
	GetListeningDoc()->m_pcEditWnd->Views_DisableSelectArea(true);
}

void CDocEditor::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//	May 12, 2000 genta
	//	�ҏW�p���s�R�[�h�̐ݒ�
	{
		SetNewLineCode( EOL_CRLF );
		CDocLine*	pFirstlineinfo = pcDoc->m_cDocLineMgr.GetLine( CLogicInt(0) );
		if( pFirstlineinfo != NULL ){
			EEolType t = pFirstlineinfo->GetEol();
			if( t != EOL_NONE && t != EOL_UNKNOWN ){
				SetNewLineCode( t );
			}
		}
	}

	//	Nov. 20, 2000 genta
	//	IME��Ԃ̐ݒ�
	this->SetImeMode( pcDoc->m_cDocType.GetDocumentAttribute().m_nImeState );

	// �J�����g�f�B���N�g���̕ύX
	::SetCurrentDirectory( pcDoc->m_cDocFile.GetFilePathClass().GetDirPath().c_str() );

	CAppMode::Instance()->SetViewMode(sLoadInfo.bViewMode);		// �r���[���[�h	##�������A�A������
}

void CDocEditor::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	this->SetModified(false,false);	//	Jan. 22, 2002 genta �֐��� �X�V�t���O�̃N���A

	/* ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm */
	this->m_cOpeBuf.SetNoModified();
}



/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
bool CDocEditor::IsEnableUndo( void )
{
	return m_cOpeBuf.IsEnableUndo();
}


/*! Redo(��蒼��)�\�ȏ�Ԃ��H */
bool CDocEditor::IsEnableRedo( void )
{
	return m_cOpeBuf.IsEnableRedo();
}

/*! �N���b�v�{�[�h����\��t���\���H */
bool CDocEditor::IsEnablePaste( void )
{
	return CClipboard::HasValidData();
}




//	From Here Nov. 20, 2000 genta
/*!	IME��Ԃ̐ݒ�
	
	@param mode [in] IME�̃��[�h
	
	@date Nov 20, 2000 genta
*/
void CDocEditor::SetImeMode( int mode )
{
	DWORD	conv, sent;
	HIMC	hIme;

	hIme = ImmGetContext( CEditWnd::Instance()->GetHwnd() ); //######���v�H

	//	�ŉ��ʃr�b�g��IME���g��On/Off����
	if( ( mode & 3 ) == 2 ){
		ImmSetOpenStatus( hIme, FALSE );
	}
	if( ( mode >> 2 ) > 0 ){
		ImmGetConversionStatus( hIme, &conv, &sent );

		switch( mode >> 2 ){
		case 1:	//	FullShape
			conv |= IME_CMODE_FULLSHAPE;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 2:	//	FullShape & Hiragana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
			conv &= ~( IME_CMODE_KATAKANA | IME_CMODE_NOCONVERSION );
			break;
		case 3:	//	FullShape & Katakana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE | IME_CMODE_KATAKANA;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 4: //	Non-Conversion
			conv |= IME_CMODE_NOCONVERSION;
			break;
		}
		ImmSetConversionStatus( hIme, conv, sent );
	}
	if( ( mode & 3 ) == 1 ){
		ImmSetOpenStatus( hIme, TRUE );
	}
	ImmReleaseContext( CEditWnd::Instance()->GetHwnd(), hIme ); //######���v�H
}
//	To Here Nov. 20, 2000 genta










/*!
	�����ɍs��ǉ�

	@version 1.5

	@param pData    [in] �ǉ����镶����ւ̃|�C���^
	@param nDataLen [in] ������̒����B�����P�ʁB
	@param cEol     [in] �s���R�[�h

*/


void CDocEditAgent::AddLineStrX( const wchar_t* pData, int nDataLen )
{
	//�`�F�[���K�p
	CDocLine* pDocLine = m_pcDocLineMgr->AddNewLine();

	//�C���X�^���X�ݒ�
	pDocLine->SetDocLineString(pData, nDataLen);
}




/* �f�[�^�̍폜 */
/*
|| �w��s���̕��������폜�ł��܂���
|| �f�[�^�ύX�ɂ���ĉe���̂������A�ύX�O�ƕύX��̍s�͈̔͂�Ԃ��܂�
|| ���̏������ƂɁA���C�A�E�g���Ȃǂ��X�V���Ă��������B
||
	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CDocEditAgent::DeleteData_CDocLineMgr(
	CLogicInt	nLine,
	CLogicInt	nDelPos,
	CLogicInt	nDelLen,
	CLogicInt*	pnModLineOldFrom,	//!< �e���̂������ύX�O�̍s(from)
	CLogicInt*	pnModLineOldTo,		//!< �e���̂������ύX�O�̍s(to)
	CLogicInt*	pnDelLineOldFrom,	//!< �폜���ꂽ�ύX�O�_���s(from)
	CLogicInt*	pnDelLineOldNum,	//!< �폜���ꂽ�s��
	CNativeW*	cmemDeleted			//!< [out] �폜���ꂽ�f�[�^
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CDocLineMgr::DeleteData" );
#endif
	CDocLine*	pDocLine;
	CDocLine*	pDocLine2;
	CLogicInt	nDeleteLength;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	const wchar_t*	pLine2;
	CLogicInt		nLineLen2;

	*pnModLineOldFrom = nLine;	/* �e���̂������ύX�O�̍s(from) */
	*pnModLineOldTo = nLine;	/* �e���̂������ύX�O�̍s(to) */
	*pnDelLineOldFrom = CLogicInt(0);		/* �폜���ꂽ�ύX�O�_���s(from) */
	*pnDelLineOldNum = CLogicInt(0);		/* �폜���ꂽ�s�� */
//	cmemDeleted.SetData( "", lstrlen( "" ) );
	cmemDeleted->Clear();

	pDocLine = m_pcDocLineMgr->GetLine( nLine );
	if( NULL == pDocLine ){
		return;
	}

	CModifyVisitor().SetLineModified(pDocLine,true);		/* �ύX�t���O */

	pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );

	if( nDelPos >= nLineLen ){
		return;
	}
	/* �u���s�v���폜����ꍇ�́A���̍s�ƘA������ */
//	if( ( nDelPos == nLineLen -1 && ( pLine[nDelPos] == CR || pLine[nDelPos] == LF ) )
//	 || nDelPos + nDelLen >= nLineLen
	if( ( EOL_NONE != pDocLine->GetEol() && nDelPos == nLineLen - pDocLine->GetEol().GetLen() )
	 || ( EOL_NONE != pDocLine->GetEol() && nDelPos + nDelLen >  nLineLen - pDocLine->GetEol().GetLen() )
	 || ( EOL_NONE == pDocLine->GetEol() && nDelPos + nDelLen >= nLineLen - pDocLine->GetEol().GetLen() )
	){
		/* ���ۂɍ폜����o�C�g�� */
		nDeleteLength = nLineLen - nDelPos;

		/* �폜�����f�[�^ */
		cmemDeleted->SetString( &pLine[nDelPos], nDeleteLength );

		/* ���̍s�̏�� */
		pDocLine2 = pDocLine->GetNextLine();
		if( !pDocLine2 ){
			wchar_t*	pData = new wchar_t[nLineLen + 1];
			if( nDelPos > 0 ){
				wmemcpy( pData, pLine, nDelPos );
			}
			if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
				wmemcpy(
					pData + nDelPos,
					pLine + nDelPos + nDeleteLength,
					nLineLen - ( nDelPos + nDeleteLength )
				);
			}
			pData[ nLineLen - nDeleteLength ] = L'\0';

			if( 0 < nLineLen - nDeleteLength ){
				pDocLine->SetDocLineString( pData, nLineLen - nDeleteLength );
			}else{
				// �s�̍폜
				// 2004.03.18 Moca �֐����g��
				m_pcDocLineMgr->DeleteLine( pDocLine );
				pDocLine = NULL;
				*pnDelLineOldFrom = nLine;	/* �폜���ꂽ�ύX�O�_���s(from) */
				*pnDelLineOldNum = CLogicInt(1);		/* �폜���ꂽ�s�� */
			}
			delete [] pData;
		}
		else{
			*pnModLineOldTo = nLine + CLogicInt(1);	/* �e���̂������ύX�O�̍s(to) */
			pLine2 = pDocLine2->GetDocLineStrWithEOL( &nLineLen2 );
			wchar_t*	pData = new wchar_t[nLineLen + nLineLen2 + 1];
			if( nDelPos > 0 ){
				wmemcpy( pData, pLine, nDelPos );
			}
			if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
				wmemcpy(
					pData + nDelPos,
					pLine + nDelPos + nDeleteLength,
					nLineLen - ( nDelPos + nDeleteLength )
				);
			}
			/* ���̍s�̃f�[�^��A�� */
			wmemcpy( pData + (nLineLen - nDeleteLength), pLine2, nLineLen2 );
			pData[ nLineLen - nDeleteLength + nLineLen2 ] = L'\0';
			pDocLine->SetDocLineString( pData, nLineLen - nDeleteLength + nLineLen2 );

			/* ���̍s���폜 && �����s�Ƃ̃��X�g�̘A��*/
			// 2004.03.18 Moca DeleteLine ���g��
			m_pcDocLineMgr->DeleteLine( pDocLine2 );
			pDocLine2 = NULL;
			*pnDelLineOldFrom = nLine + CLogicInt(1);	/* �폜���ꂽ�ύX�O�_���s(from) */
			*pnDelLineOldNum = CLogicInt(1);			/* �폜���ꂽ�s�� */
			delete [] pData;
		}
	}
	else{
		/* ���ۂɍ폜����o�C�g�� */
		nDeleteLength = nDelLen;

		/* �폜�����f�[�^ */
		cmemDeleted->SetString( &pLine[nDelPos], nDeleteLength );

		wchar_t*	pData = new wchar_t[nLineLen + 1];
		if( nDelPos > 0 ){
			wmemcpy( pData, pLine, nDelPos );
		}
		if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
			wmemcpy(
				pData + nDelPos,
				pLine + nDelPos + nDeleteLength,
				nLineLen - ( nDelPos + nDeleteLength )
			);
		}
		pData[ nLineLen - nDeleteLength ] = L'\0';
		if( 0 < nLineLen - nDeleteLength ){
			pDocLine->SetDocLineString( pData, nLineLen - nDeleteLength );
		}
		delete [] pData;
	}
}





/*!	�f�[�^�̑}��

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CDocEditAgent::InsertData_CDocLineMgr(
	CLogicInt		nLine,
	CLogicInt		nInsPos,
	const wchar_t*	pInsData,
	CLogicInt		nInsDataLen,
	CLogicInt*		pnInsLineNum,	// �}���ɂ���đ������s�̐�
	CLogicPoint*	pptNewPos		// �}�����ꂽ�����̎��̈ʒu
)
{
	CNativeW	cmemPrevLine;
	CNativeW	cmemNextLine;
	CLogicInt	nAllLinesOld = m_pcDocLineMgr->GetLineCount();

	bool		bBookMarkNext;	// 2001.12.03 hor �}���ɂ��}�[�N�s�̐���

	pptNewPos->y = nLine;	/* �}�����ꂽ�����̎��̈ʒu�̍s */

	//	Jan. 25, 2004 genta
	//	�}�������񒷂�0�̏ꍇ�ɍŌ�܂�pnNewPos���ݒ肳��Ȃ��̂�
	//	�����l�Ƃ���0�ł͂Ȃ��J�n�ʒu�Ɠ����l�����Ă����D
	pptNewPos->x  = nInsPos;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */

	/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
	*pnInsLineNum = CLogicInt(0);
	CDocLine*	pDocLine = m_pcDocLineMgr->GetLine( nLine );
	if( !pDocLine ){
		/* ������NULL���A���Ă���Ƃ������Ƃ́A*/
		/* �S�e�L�X�g�̍Ō�̎��̍s��ǉ����悤�Ƃ��Ă��邱�Ƃ����� */
		cmemPrevLine.SetString(L"");
		cmemNextLine.SetString(L"");
		bBookMarkNext=false;	// 2001.12.03 hor
	}
	else{
		CModifyVisitor().SetLineModified(pDocLine,true);		/* �ύX�t���O */

		CLogicInt		nLineLen;
		const wchar_t*	pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
		cmemPrevLine.SetString( pLine, nInsPos );
		cmemNextLine.SetString( &pLine[nInsPos], nLineLen - nInsPos );

		bBookMarkNext= CBookmarkGetter(pDocLine).IsBookmarked();	// 2001.12.03 hor
	}

	int			nCount = 0;
	CLogicInt	nBgn   = CLogicInt(0);
	CLogicInt	nPos   = CLogicInt(0);
	for( nPos = CLogicInt(0); nPos < nInsDataLen; ){
		if( WCODE::IsLineDelimiter(pInsData[nPos]) ){
			/* �s�I�[�q�̎�ނ𒲂ׂ� */
			CEol 	cEOLType;
			cEOLType.SetTypeByString( &pInsData[nPos], nInsDataLen - nPos );

			/* �s�I�[�q���܂߂ăe�L�X�g���o�b�t�@�Ɋi�[ */
			CNativeW	cmemCurLine;
			cmemCurLine.SetString( &pInsData[nBgn], nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + CLogicInt(cEOLType.GetLen());
			nPos = nBgn;
			if( NULL == pDocLine ){
				CDocLine* pDocLineNew = m_pcDocLineMgr->AddNewLine();

				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					pDocLineNew->SetDocLineString(cmemPrevLine + cmemCurLine);
				}
				else{
					pDocLineNew->SetDocLineString(cmemCurLine);
				}
				pDocLine = NULL;
			}
			else{
				/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
				if( 0 == nCount ){
					pDocLine->SetDocLineString( cmemPrevLine + cmemCurLine );

					// 2001.12.13 hor
					// �s���ŉ��s�����猳�̍s�̃}�[�N��V�����s�Ɉړ�����
					// ����ȊO�Ȃ猳�̍s�̃}�[�N���ێ����ĐV�����s�ɂ̓}�[�N��t���Ȃ�
					if(nInsPos==CLogicInt(0)){
						CBookmarkSetter(pDocLine).SetBookmark(false);
					}
					else{
						bBookMarkNext=false;
					}

					pDocLine = pDocLine->GetNextLine();
				}
				else{
					CDocLine* pDocLineNew = m_pcDocLineMgr->InsertNewLine(pDocLine); //pDocLine�̒��O�ɑ}��
					
					pDocLineNew->SetDocLineString( cmemCurLine );
				}
			}

			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			++nCount;
			++pptNewPos->y;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
		}
		else{
			++nPos;
		}
	}

	if( CLogicInt(0) < nPos - nBgn || 0 < cmemNextLine.GetStringLength() ){
		CNativeW	cmemCurLine;
		cmemCurLine.SetString( &pInsData[nBgn], nPos - nBgn );
		cmemCurLine += cmemNextLine;
		if( NULL == pDocLine ){
			CDocLine* pDocLineNew = m_pcDocLineMgr->AddNewLine();
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				pDocLineNew->SetDocLineString( cmemPrevLine + cmemCurLine );
			}
			else{
				pDocLineNew->SetDocLineString( cmemCurLine );
			}
			pDocLine = NULL;
			pptNewPos->x = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
		}
		else{
			/* �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^ */
			if( 0 == nCount ){
				pDocLine->SetDocLineString( cmemPrevLine + cmemCurLine );
				pDocLine = pDocLine->GetNextLine();
				pptNewPos->x = CLogicInt(cmemPrevLine.GetStringLength()) + nPos - nBgn;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}
			else{
				CDocLine* pDocLineNew = m_pcDocLineMgr->InsertNewLine(pDocLine); //pDocLine�̒��O�ɑ}��

				pDocLineNew->SetDocLineString( cmemCurLine );

				CBookmarkSetter(pDocLineNew).SetBookmark(bBookMarkNext);	// 2001.12.03 hor �u�b�N�}�[�N�𕜌�

				pptNewPos->x = nPos - nBgn;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
			}
		}
	}
	*pnInsLineNum = m_pcDocLineMgr->GetLineCount() - nAllLinesOld;
}

