#include "stdafx.h"
#include "types/CType.h"
#include "doc/CDocOutline.h"
#include "CFuncInfoArr.h"

/* Perl */
//Jul. 08, 2001 JEPRO Perl ���[�U�ɑ���
//Jul. 08, 2001 JEPRO �ǉ�
void CType_Perl::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("Perl") );
	_tcscpy( pType->m_szTypeExts, _T("cgi,pl,pm") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"#", -1 );					/* �s�R�����g�f���~�^ */
	pType->m_nDefaultOutline = OUTLINE_PERL;						/* �A�E�g���C����͕��@ */
	pType->m_nKeyWordSetIdx[0]  = 11;								/* �L�[���[�h�Z�b�g */
	pType->m_nKeyWordSetIdx[1] = 12;								/* �L�[���[�h�Z�b�g2 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;			/* ���p���l��F�����\�� */
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;	//�Ί��ʂ̋������f�t�H���gON	//Sep. 21, 2002 genta
}



//	From Here Sep 8, 2000 genta
//
//!	Perl�p�A�E�g���C����͋@�\�i�ȈՔŁj
/*!
	�P���� /^\\s*sub\\s+(\\w+)/ �Ɉ�v������ $1�����o��������s���B
	�l�X�g�Ƃ��͖ʓ|�������̂ōl���Ȃ��B
	package{ }���g��Ȃ���΂���ŏ\���D�������͂܂��B

	@par nMode�̈Ӗ�
	@li 0: �͂���
	@li 2: sub����������
	@li 1: �P��ǂݏo����

	@date 2005.06.18 genta �p�b�P�[�W��؂��\�� ::��'���l������悤��
*/
void CDocOutline::MakeFuncList_Perl( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt			nLineLen;
	int			i;
	int			nCharChars;
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;

	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		nMode = 0;
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if(	1 < nCharChars ){
				break;
			}

			/* �P��ǂݍ��ݒ� */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}
				if( 's' != pLine[i] )
					break;
				//	sub �̈ꕶ���ڂ�������Ȃ�
				if( nLineLen - i < 4 )
					break;
				if( wcsncmp_literal( pLine + i, L"sub" ) )
					break;
				int c = pLine[ i + 3 ];
				if( c == L' ' || c == L'\t' ){
					nMode = 2;	//	����
					i += 3;
				}
				else
					break;
			}
			else if( 2 == nMode ){
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}
				if( L'_' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
				){
					//	�֐����̎n�܂�
					nWordIdx = 0;
					szWord[nWordIdx] = pLine[i];
					szWord[nWordIdx + 1] = L'\0';
					nMode = 1;
					continue;
				}
				else
					break;

			}
			else if( 1 == nMode ){
				if( L'_' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					//	Jun. 18, 2005 genta �p�b�P�[�W�C���q���l��
					//	�R������2�A�����Ȃ��Ƃ����Ȃ��̂����C�����͎蔲��
					L':' == pLine[i] || L'\'' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						break;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
					}
				}else{
					//	�֐����擾
					/*
					  �J�[�\���ʒu�ϊ�
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					  ��
					  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
					*/
					CLayoutPoint ptPosXY;
					m_pcDocRef->m_cLayoutMgr.LogicToLayout(
						CLogicPoint(CLogicInt(0), nLineCount),
						&ptPosXY
					);
					//	Mar. 9, 2001
					pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPosXY.GetY2() + CLayoutInt(1), szWord, 0 );

					break;
				}
			}
		}
	}
#ifdef _DEBUG
	pcFuncInfoArr->DUMP();
#endif
	return;
}
//	To HERE Sep. 8, 2000 genta

