#include "StdAfx.h"
#include "types/CType.h"
#include "doc/CDocOutline.h"
#include "doc/CDocLineMgr.h"
#include "doc/CDocLine.h"
#include "doc/CEditDoc.h"
#include "env/DLLSHAREDATA.h"
#include "outline/CFuncInfo.h"
#include "outline/CFuncInfoArr.h"
#include "view/colors/EColorIndexType.h"

/* �e�L�X�g */
//Sep. 20, 2000 JEPRO �e�L�X�g�̋K��l��80��120�ɕύX(�s��ꗗ.txt��������x�ǂ݂₷������)
//Nov. 15, 2000 JEPRO PostScript�t�@�C�����ǂ߂�悤�ɂ���
//Jan. 12, 2001 JEPRO readme.1st ���ǂ߂�悤�ɂ���
//Feb. 12, 2001 JEPRO .err �G���[���b�Z�[�W
//Nov.  6, 2002 genta doc��MS Word�ɏ����Ă�������͊O���i�֘A�Â��h�~�̂��߁j
//Nov.  6, 2002 genta log ��ǉ�
void CType_Text::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("�e�L�X�g") );
	_tcscpy( pType->m_szTypeExts, _T("txt,log,1st,err,ps") );

	//�ݒ�
	pType->m_nMaxLineKetas = CLayoutInt(120);					/* �܂�Ԃ����� */
	pType->m_eDefaultOutline = OUTLINE_TEXT;					/* �A�E�g���C����͕��@ */
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//Oct. 17, 2000 JEPRO	�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//Sept. 4, 2000 JEPRO	�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pType->m_bKinsokuHead = false;								// �s���֑�				//@@@ 2002.04.08 MIK
	pType->m_bKinsokuTail = false;								// �s���֑�				//@@@ 2002.04.08 MIK
	pType->m_bKinsokuRet  = false;								// ���s�������Ԃ牺����	//@@@ 2002.04.13 MIK
	pType->m_bKinsokuKuto = false;								// ��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
	wcscpy( pType->m_szKinsokuHead, L"!%),.:;?]}�����f�h�񁌁����A�B�X�r�t�v�x�z�l�J�K�T�U�E�R�S�I���j�C�D�F�G�H�n�p�����߁�" );		/* �s���֑� */	//@@@ 2002.04.13 MIK 
	wcscpy( pType->m_szKinsokuTail, L"$([{��\\�e�g�q�s�u�w�y�k���i�m�o�����" );		/* �s���֑� */	//@@@ 2002.04.08 MIK 
	// pType->m_szKinsokuKuto�i��Ǔ_�Ԃ牺�������j�͂����ł͂Ȃ��S�^�C�v�Ƀf�t�H���g�ݒ�	// 2009.08.07 ryoji 

	//�������Ȑe�؂Ƃ��āAC:\�`�` �� \\�`�` �Ȃǂ̃t�@�C���p�X���N���b�J�u���ɂ���ݒ���u�e�L�X�g�v�Ɋ���Ŏd����
	//��""�ŋ��܂��ݒ�͋��܂�Ȃ��ݒ������ɖ�����΂Ȃ�Ȃ�
	//��""�ŋ��܂��ݒ�𕡐����Ă�����ƏC������΁A<>��[]�ɋ��܂ꂽ���̂ɂ��Ή��ł���i���[�U�ɔC����j

	//���K�\���L�[���[�h
	int keywordPos = 0;
	wchar_t* pKeyword = pType->m_RegexKeywordList;
	pType->m_bUseRegexKeyword = true;							// ���K�\���L�[���[�h���g����
	pType->m_RegexKeywordArr[0].m_nColorIndex = COLORIDX_URL;	// �F�w��ԍ�
	wcscpyn( &pKeyword[keywordPos],			// ���K�\���L�[���[�h
		L"/(?<=\")(\\b[a-zA-Z]:|\\B\\\\\\\\)[^\"\\r\\n]*/k",			//   ""�ŋ��܂ꂽ C:\�`, \\�` �Ƀ}�b�`����p�^�[��
		_countof(pType->m_RegexKeywordList) - 1 );
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;
	pType->m_RegexKeywordArr[1].m_nColorIndex = COLORIDX_URL;	// �F�w��ԍ�
	wcscpyn( &pKeyword[keywordPos],			// ���K�\���L�[���[�h
		L"/(\\b[a-zA-Z]:\\\\|\\B\\\\\\\\)[\\w\\-_.\\\\\\/$%~]*/k",		//   C:\�`, \\�` �Ƀ}�b�`����p�^�[��
		_countof(pType->m_RegexKeywordList) - keywordPos - 1 );
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;
	pKeyword[keywordPos] = L'\0';
}




/*!	�e�L�X�g�E�g�s�b�N���X�g�쐬
	
	@date 2002.04.01 YAZAKI CDlgFuncList::SetText()���g�p����悤�ɉ����B
	@date 2002.11.03 Moca	�K�w���ő�l�𒴂���ƃo�b�t�@�I�[�o�[��������̂��C��
							�ő�l�ȏ�͒ǉ������ɖ�������
	@date 2007.8��   kobake �@�B�I��UNICODE��
	@date 2007.11.29 kobake UNICODE�Ή��ł��ĂȂ������̂ŏC��
*/
void CDocOutline::MakeTopicList_txt( CFuncInfoArr* pcFuncInfoArr )
{
	using namespace WCODE;

	//���o���L��
	const wchar_t*	pszStarts = GetDllShareData().m_Common.m_sFormat.m_szMidashiKigou;
	int				nStartsLen = wcslen( pszStarts );

	/*	�l�X�g�̐[���́AnMaxStack���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�32�����܂ŋ��
		�i32�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int nMaxStack = 32;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	�ꎞ�̈�
	CLogicInt				nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//�s�擾
		CLogicInt		nLineLen;
		const wchar_t*	pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine )break;

		//�s���̋󔒔�΂�
		int i;
		for( i = 0; i < nLineLen; ++i ){
			if( WCODE::IsBlank(pLine[i]) ){
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}

		//�擪���������o���L���̂����ꂩ�ł���΁A���֐i��
		int j;
		int nCharChars;
		int nCharChars2;
		nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
		for( j = 0; j < nStartsLen; j += nCharChars2 ){
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars2 = CNativeW::GetSizeOfChar( pszStarts, nStartsLen, j );
			if( nCharChars == nCharChars2 ){
				if( 0 == wmemcmp( &pLine[i], &pszStarts[j], nCharChars ) ){
					break;
				}
			}
		}
		if( j >= nStartsLen ){
			continue;
		}

		//���o����ނ̔��� -> szTitle
		if( pLine[i] == L'(' ){
			     if ( IsInRange(pLine[i + 1], L'0', L'9') ) wcscpy( szTitle, L"(0)" ); //����
			else if ( IsInRange(pLine[i + 1], L'A', L'Z') ) wcscpy( szTitle, L"(A)" ); //�p�啶��
			else if ( IsInRange(pLine[i + 1], L'a', L'z') ) wcscpy( szTitle, L"(a)" ); //�p������
			else continue; //���u(�v�̎����p�����Ŗ����ꍇ�A���o���Ƃ݂Ȃ��Ȃ�
		}
		else if( IsInRange(pLine[i], L'�O', L'�X') ) wcscpy( szTitle, L"�O" ); // �S�p����
		else if( IsInRange(pLine[i], L'�@', L'�S') ) wcscpy( szTitle, L"�@" ); // �@�`�S
		else if( IsInRange(pLine[i], L'�T', L'�]') ) wcscpy( szTitle, L"�T" ); // �T�`�]
		else if( wcschr(L"�Z���O�l�ܘZ������\�S����Q��", pLine[i]) ) wcscpy( szTitle, L"��" ); //������
		else{
			wcsncpy( szTitle, &pLine[i], nCharChars );	//	�擪������szTitle�ɕێ��B
			szTitle[nCharChars] = L'\0';
		}

		/*	�u���o���L���v�Ɋ܂܂�镶���Ŏn�܂邩�A
			(0�A(1�A...(9�A(A�A(B�A...(Z�A(a�A(b�A...(z
			�Ŏn�܂�s�́A�A�E�g���C�����ʂɕ\������B
		*/

		//�s�����񂩂���s����菜�� pLine -> pszText
		wchar_t*	pszText = new wchar_t[nLineLen + 1];
		wmemcpy( pszText, &pLine[i], nLineLen );
		pszText[nLineLen] = L'\0';
		for( i = 0; i < (int)wcslen(pszText); ++i ){
			if( pszText[i] == CR || pszText[i] == LF )pszText[i] = L'\0';
		}

		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		CLayoutPoint ptPos;
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0, nLineCount),
			&ptPos
		);

		/* nDepth���v�Z */
		int k;
		bool bAppend = true;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = wcscmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	���[�v�r����break;���Ă����B�����܂łɓ������o�������݂��Ă����B
			//	�̂ŁA�������x���ɍ��킹��AppendData.
			nDepth = k;
		}
		else if( nMaxStack > k ){
			//	���܂܂łɓ������o�������݂��Ȃ������B
			//	�̂ŁApszStack�ɃR�s�[����AppendData.
			wcscpy(pszStack[nDepth], szTitle);
		}
		else{
			// 2002.11.03 Moca �ő�l�𒴂���ƃo�b�t�@�I�[�o�[����
			// nDepth = nMaxStack;
			bAppend = false;
		}
		
		if( bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	return;
}





/*! �K�w�t���e�L�X�g �A�E�g���C�����

	@author zenryaku
	@date 2003.05.20 zenryaku �V�K�쐬
	@date 2003.05.25 genta �������@�ꕔ�C��
	@date 2003.06.21 Moca �K�w��2�i�ȏ�[���Ȃ�ꍇ���l��
*/
void CDocOutline::MakeTopicList_wztxt(CFuncInfoArr* pcFuncInfoArr)
{
	int levelPrev = 0;

	for(CLogicInt nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		const wchar_t*	pLine;
		CLogicInt		nLineLen;

		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if(!pLine)
		{
			break;
		}
		//	May 25, 2003 genta ���菇���ύX
		if( *pLine == L'.' )
		{
			const wchar_t* pPos;	//	May 25, 2003 genta
			int			nLength;
			wchar_t		szTitle[1024];

			//	�s���I�h�̐����K�w�̐[���𐔂���
			for( pPos = pLine + 1 ; *pPos == L'.' ; ++pPos )
				;

			CLayoutPoint ptPos;
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			
			int level = pPos - pLine;

			// 2003.06.27 Moca �K�w��2�i�ʏ�[���Ȃ�Ƃ��́A����̗v�f��ǉ�
			if( levelPrev < level && level != levelPrev + 1  ){
				int dummyLevel;
				// (����)��}��
				//	�������CTAG�ꗗ�ɂ͏o�͂���Ȃ��悤��
				for( dummyLevel = levelPrev + 1; dummyLevel < level; dummyLevel++ ){
					pcFuncInfoArr->AppendData(
						nLineCount+CLogicInt(1),
						ptPos.GetY2()+CLayoutInt(1),
						L"(����)",
						FUNCINFO_NOCLIPTEXT,
						dummyLevel - 1
					);
				}
			}
			levelPrev = level;

			nLength = auto_sprintf(szTitle,L"%d - ", level );
			
			wchar_t *pDest = szTitle + nLength; // �������ݐ�
			wchar_t *pDestEnd = szTitle + _countof(szTitle) - 2;
			
			while( pDest < pDestEnd )
			{
				if( *pPos ==L'\r' || *pPos ==L'\n' || *pPos == L'\0')
				{
					break;
				}
				else {
					*pDest++ = *pPos++;
				}
			}
			*pDest = L'\0';
			pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle, 0, level - 1);
		}
	}
}

