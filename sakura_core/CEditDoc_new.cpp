//	$Id$
/*!	@file
	�����֘A���̊Ǘ�

	@author Norio Nakatani
	$Revision$
	
	@date aroka �x���΍�ŕϐ�����
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, Miasaka, hor
	Copyright (C) 2002, hor, genta, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CEditDoc.h"
#include "charcode.h"
#include "CFuncInfoArr.h"// 2002/2/10 aroka
#include "CDocLine.h"// 2002/2/10 aroka
#include "CEditWnd.h"
#include "Debug.h"
#include "etc_uty.h"
#include "my_icmp.h" // Nov. 29, 2002 genta/moca

/* Java�֐����X�g�쐬 */
void CEditDoc::MakeFuncList_Java( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nNestLevel;
	int			nCharChars;
	char		szWordPrev[100];
	char		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	char		szFuncName[80];
	int			nFuncLine = 0;
	int			nFuncId;
	int			nFuncNum;
	char		szClass[1024];

	int			nClassNestArr[16];
	int			nClassNestArrNum;
	int			nNestLevel2Arr[16];

	nNestLevel = 0;
	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = 0;
	nNestLevel2Arr[0] = 0;
	nFuncNum = 0;
	szClass[0] = '\0';
	nClassNestArrNum = 0;
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if(	1 < nCharChars ){
				i += (nCharChars - 1);
				continue;
			}

			/* �G�X�P�[�v�V�[�P���X�͏�Ɏ�菜�� */
			if( '\\' == pLine[i] ){
				++i;
			}else
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �_�u���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 21 == nMode ){
				if( '"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �R�����g�ǂݍ��ݒ� */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �P��ǂݍ��ݒ� */
			if( 1 == nMode ){
				if( '_' == pLine[i] ||
					':' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					'.' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}else{
					/* �N���X�錾������������ */
					//	Oct. 10, 2002 genta interface���Ώۂ�
					if( 0 == strcmp( "class", szWordPrev ) ||
						0 == strcmp( "interface", szWordPrev )
					 ){
						nClassNestArr[nClassNestArrNum] = nNestLevel;
						++nClassNestArrNum;
						if( 0 < nNestLevel	){
							strcat( szClass, "\\" );
						}
						strcat( szClass, szWord );




						nFuncId = 0;
						++nFuncNum;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nLineCount/*nFuncLine - 1*/,
							&nPosX,
							&nPosY
						);
						char szWork[256];
						wsprintf( szWork, "%s::%s", szClass, "��`�ʒu" );
						pcFuncInfoArr->AppendData( nPosY + 1/*nFuncLine*/, nPosY + 1, szWork, nFuncId );

					}


//					strcpy( szWordPrev, szWord );
//					nWordIdx = 0;
//					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* �L����ǂݍ��ݒ� */
			if( 2 == nMode ){
				if( '_' == pLine[i] ||
					':' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					'\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i] ||
					'{' == pLine[i] ||
					'}' == pLine[i] ||
					'(' == pLine[i] ||
					')' == pLine[i] ||
					';' == pLine[i]	||
					'\'' == pLine[i] ||
					'"' == pLine[i] ||
					'/' == pLine[i] ||
					'.' == pLine[i]
				){
					nMode = 0;
					i--;
					continue;
				}else{
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '/' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( '"' == pLine[i] ){
					nMode = 21;
					continue;
				}else
				if( '{' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						//	���\�b�h���ł���Ƀ��\�b�h���`���邱�Ƃ͂Ȃ��̂�
						//	�l�X�g���x������ǉ� class/interface�̒����̏ꍇ�̂ݔ��肷��
						if( nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && 0 != strcmp( "sizeof", szFuncName )
						 && 0 != strcmp( "if", szFuncName )
						 && 0 != strcmp( "for", szFuncName )
						 && 0 != strcmp( "do", szFuncName )
						 && 0 != strcmp( "while", szFuncName )
						 && 0 != strcmp( "catch", szFuncName )
						 && 0 != strcmp( "switch", szFuncName )
						 && 0 != strcmp( "return", szFuncName )
						){
							nFuncId = 2;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.CaretPos_Phys2Log(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							char szWork[256];
							wsprintf( szWork, "%s::%s", szClass, szFuncName );
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szWork, nFuncId );
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					++nNestLevel;
					nMode = 0;
					continue;
				}else
				if( '}' == pLine[i] ){
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}

					nNestLevel--;
					if( 0 < nClassNestArrNum &&
						nClassNestArr[nClassNestArrNum - 1] == nNestLevel
					){
						nClassNestArrNum--;
						int k;
						for( k = lstrlen( szClass ) - 1; k >= 0; k-- ){
							if( '\\' == szClass[k] ){
								break;
							}
						}
						if( 0 > k ){
							k = 0;
						}
						szClass[k] = '\0';
					}
					nMode = 0;
					continue;
				}else
				if( '(' == pLine[i] ){
					if( 0 < nClassNestArrNum /*nNestLevel == 1*/ &&
						0 != strcmp( "new", szWordPrev )
					){
						strcpy( szFuncName, szWord );
						nFuncLine = nLineCount + 1;
						if( 0 < nClassNestArrNum ){
							nNestLevel2Arr[nClassNestArrNum - 1] = 1;
						}
					}
					nMode = 0;
					continue;
				}else
				if( ')' == pLine[i] ){
					int			k;
					const char*	pLine2;
					int			nLineLen2;
					int			nLineCount2;
					nLineCount2 = nLineCount;
					pLine2 = pLine;
					nLineLen2 = nLineLen;
					k = i + 1;
					BOOL		bCommentLoop;
					bCommentLoop = FALSE;
				loop_is_func:;
					for( ; k < nLineLen2; ++k ){
						if( !bCommentLoop ){
							if( pLine2[k] != ' ' && pLine2[k] != TAB && pLine2[k] != CR && pLine2[k] != LF ){
								if( k + 1 < nLineLen2 && pLine2[k] == '/' && pLine2[k + 1] == '*' ){
									bCommentLoop = TRUE;
									++k;
								}else
								if( k + 1 < nLineLen2 && pLine2[k] == '/' && pLine2[k + 1] == '/' ){
									k = nLineLen2 + 1;
									break;
								}else{
									break;
								}
							}
						}else{
							if( k + 1 < nLineLen2 && pLine2[k] == '*' && pLine2[k + 1] == '/' ){
								bCommentLoop = FALSE;
								++k;
							}
						}
					}
					if( k >= nLineLen2 ){
						k = 0;
						++nLineCount2;
						pLine2 = m_cDocLineMgr.GetLineStr( nLineCount2, &nLineLen2 );
						if( NULL != pLine2 ){
							goto loop_is_func;
						}
						if( 0 < nClassNestArrNum ){
							nNestLevel2Arr[nClassNestArrNum - 1] = 0;
						}
					}else{
						//	Oct. 10, 2002 genta
						//	abscract �ɂ��Ή�
						if( pLine2[k] == '{' || pLine2[k] == ';' ||
							__iscsym( pLine2[k] ) ){
							if( 0 < nClassNestArrNum ){
								if( 1 == nNestLevel2Arr[nClassNestArrNum - 1] ){
									nNestLevel2Arr[nClassNestArrNum - 1] = 2;
								}
							}
						}else{
							if( 0 < nClassNestArrNum ){
								nNestLevel2Arr[nClassNestArrNum - 1] = 0;
							}
						}
					}
					nMode = 0;
					continue;
				}else
				if( ';' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						// �֐��̒��ŕʂ̊֐��̐錾�����g�����Ƃ��āCJava�ł���́H
						if( nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && 0 != strcmp( "sizeof", szFuncName )
						 && 0 != strcmp( "if", szFuncName )
						 && 0 != strcmp( "for", szFuncName )
						 && 0 != strcmp( "do", szFuncName )
						 && 0 != strcmp( "while", szFuncName )
						 && 0 != strcmp( "catch", szFuncName )
						 && 0 != strcmp( "switch", szFuncName )
						 && 0 != strcmp( "return", szFuncName )
						){
							nFuncId = 1;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.CaretPos_Phys2Log(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							char szWork[256];
							wsprintf( szWork, "%s::%s", szClass, szFuncName );
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szWork, nFuncId );
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					nMode = 0;
					continue;
				}else{
					if( '_' == pLine[i] ||
						':' == pLine[i] ||
						'~' == pLine[i] ||
						('a' <= pLine[i] &&	pLine[i] <= 'z' )||
						('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
						('0' <= pLine[i] &&	pLine[i] <= '9' )||
						'.' == pLine[i]
					){
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
						nMode = 1;
					}else{
						nMode = 0;
					}
				}
			}
		}
	}
#ifdef _DEBUG
	pcFuncInfoArr->DUMP();
#endif
	return;
}

//	From Here Aug. 14, 2000 genta
//
//	�����������֎~����Ă��邩�ǂ���
//	�߂�l: true: �֎~ / false: ����
//
bool CEditDoc::IsModificationForbidden( int nCommand )
{
	if( m_bReadOnly == FALSE &&	//	�ǂ݂Ƃ��p�ł�
		!( 0 != m_nFileShareModeOld && m_hLockedFile == NULL ) )	//	�㏑���֎~�ł��Ȃ����
		return false;			//	��ɏ�����������

	//	�㏑���֎~���[�h�̏ꍇ
	//	�b��Case��: ���ۂɂ͂����ƌ����̗ǂ����@���g���ׂ�
	switch( nCommand ){
	//	�t�@�C��������������R�}���h�͎g�p�֎~
	case F_CHAR:
	case F_IME_CHAR:
	case F_DELETE:
	case F_DELETE_BACK:
	case F_WordDeleteToEnd:
	case F_WordDeleteToStart:
	case F_WordDelete:
	case F_WordCut:
	case F_LineDeleteToStart:
	case F_LineDeleteToEnd:
	case F_LineCutToStart:
	case F_LineCutToEnd:
	case F_DELETE_LINE:
	case F_CUT_LINE:
	case F_DUPLICATELINE:
	case F_INDENT_TAB:
	case F_UNINDENT_TAB:
	case F_INDENT_SPACE:
	case F_UNINDENT_SPACE:
	case F_CUT:
	case F_PASTE:
	case F_INS_DATE:
	case F_INS_TIME:
	case F_CTRL_CODE_DIALOG:	//@@@ 2002.06.02 MIK
	case F_INSTEXT:
	case F_ADDTAIL:
	case F_PASTEBOX:
	case F_REPLACE_DIALOG:
	case F_REPLACE:
	case F_REPLACE_ALL:
	case F_CODECNV_EMAIL:
	case F_CODECNV_EUC2SJIS:
	case F_CODECNV_UNICODE2SJIS:
	case F_CODECNV_UNICODEBE2SJIS:
	case F_CODECNV_SJIS2JIS:
	case F_CODECNV_SJIS2EUC:
	case F_CODECNV_UTF82SJIS:
	case F_CODECNV_UTF72SJIS:
	case F_CODECNV_SJIS2UTF7:
	case F_CODECNV_SJIS2UTF8:
	case F_CODECNV_AUTO2SJIS:
	case F_TOLOWER:
	case F_TOUPPER:
	case F_TOHANKAKU:
	case F_TOHANKATA:				// 2002/08/29 ai
	case F_TOZENEI:					// 2001/07/30 Miasaka
	case F_TOHANEI:
	case F_TOZENKAKUKATA:
	case F_TOZENKAKUHIRA:
	case F_HANKATATOZENKAKUKATA:
	case F_HANKATATOZENKAKUHIRA:
	case F_TABTOSPACE:
	case F_SPACETOTAB:  //#### Stonee, 2001/05/27
	case F_HOKAN:
	case F_CHGMOD_INS:
	case F_LTRIM:		// 2001.12.03 hor
	case F_RTRIM:		// 2001.12.03 hor
	case F_SORT_ASC:	// 2001.12.11 hor
	case F_SORT_DESC:	// 2001.12.11 hor
	case F_MERGE:		// 2001.12.11 hor
//		::MessageBox( m_hWnd, "Operation is forbidden.", "DEBUG", MB_OK | MB_ICONEXCLAMATION );
		return true;
	}
	return false;	//	�f�t�H���g�ŏ�����������
}
//	To Here Aug. 14, 2000 genta

//	From Here Aug. 21, 2000 genta
//
//	�����ۑ����s�����ǂ����̃`�F�b�N
//
void CEditDoc::CheckAutoSave(void)
{
	if( m_cAutoSave.CheckAction() ){
		//	�㏑���ۑ�

		bool en;
		if( !IsModified() )	//	�ύX�����Ȃ牽�����Ȃ�
			return;				//	�����ł́C�u���ύX�ł��ۑ��v�͖�������

		en = m_cAutoSave.IsEnabled();
		m_cAutoSave.Enable(false);	//	2�d�Ăяo����h������
		SaveFile( GetFilePath() );	//	�ۑ��im_nCharCode, m_cSaveLineCode��ύX���Ȃ��j
		m_cAutoSave.Enable(en);
	}
}

//
//	�ݒ�ύX�������ۑ�����ɔ��f����
//
void CEditDoc::ReloadAutoSaveParam(void)
{
	m_cAutoSave.SetInterval( m_pShareData->m_Common.GetAutoBackupInterval() );
	m_cAutoSave.Enable( m_pShareData->m_Common.IsAutoBackupEnabled() );
}


//	�t�@�C���̕ۑ��@�\��EditView����ړ�
//
bool CEditDoc::SaveFile( const char* pszPath )
{
	if( FileWrite( pszPath, m_cSaveLineCode ) ){
		SetModified(false,true);	//	Jan. 22, 2002 genta

		/* ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm */
		m_cOpeBuf.SetNoModified();
		return true;
	}
	return false;
}

//	To Here Aug. 21, 2000 genta

//	From Here Sep 8, 2000 genta
//
//!	Perl�p�A�E�g���C����͋@�\�i�ȈՔŁj
/*!
	�P���� /^\s*sub\s+(\w+)/ �Ɉ�v������ $1�����o��������s���B
	�l�X�g�Ƃ��͖ʓ|�������̂ōl���Ȃ��B
	package{ }���g��Ȃ���΂���ŏ\���D�������͂܂��B

	@par nMode�̈Ӗ�
	@li 0: �͂���
	@li 2: sub����������
	@li 1: �P��ǂݏo����
*/
void CEditDoc::MakeFuncList_Perl( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nCharChars;
	char		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;

	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		nMode = 0;
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if(	1 < nCharChars ){
				break;
			}

			/* �P��ǂݍ��ݒ� */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					continue;
				}
				if( 's' != pLine[i] )
					break;
				//	sub �̈ꕶ���ڂ�������Ȃ�
				if( nLineLen - i < 4 )
					break;
				if( strncmp( pLine + i, "sub", 3 ) )
					break;
				int c = pLine[ i + 3 ];
				if( c == ' ' || c == '\t' ){
					nMode = 2;	//	����
					i += 3;
				}
				else
					break;
			}
			else if( 2 == nMode ){
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					continue;
				}
				if( '_' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )
				){
					//	�֐����̎n�܂�
					nWordIdx = 0;
					szWord[nWordIdx] = pLine[i];
					szWord[nWordIdx + 1] = '\0';
					nMode = 1;
					continue;
				}
				else
					break;

			}
			else if( 1 == nMode ){
				if( '_' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						break;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}else{
					//	�֐����擾
					/*
					  �J�[�\���ʒu�ϊ�
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					  ��
					  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
					*/
					int		nPosX;
					int		nPosY;
					m_cLayoutMgr.CaretPos_Phys2Log(
						0,
						nLineCount/*nFuncLine - 1*/,
						&nPosX,
						&nPosY
					);
					//	Mar. 9, 2001
					pcFuncInfoArr->AppendData( nLineCount + 1/*nFuncLine*/, nPosY + 1, szWord, 0 );

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








//	From Here June 23, 2001 N.Nakatani
//!	Visual Basic�֐����X�g�쐬�i�ȈՔŁj
/*!
	Visual Basic�̃R�[�h����P���Ƀ��[�U�[��`�̊֐���X�e�[�g�����g�����o��������s���B
*/
void CEditDoc::MakeFuncList_VisualBasic( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen = 0;//: 2002/2/3 aroka �x���΍�F������
	int			nLineCount;
	int			i;
	int			nCharChars;
	char		szWordPrev[100];
	char		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	char		szFuncName[80];
	int			nFuncLine;
	int			nFuncId;
	int			nParseCnt = 0;

	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = 0;
	pLine = NULL;
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		if( NULL != pLine ){
			if( '_' != pLine[nLineLen-1]){
				nParseCnt = 0;
			}
		}
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if(	0 == nCharChars ){
				nCharChars = 1;
			}
			/* �P��ǂݍ��ݒ� */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					'_' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )
					) )
				 || 2 == nCharChars
				){
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);
					}
				}else{
					if( 0 == nParseCnt && 0 == _stricmp( szWord, "Function" )
					 && 0 != _stricmp( szWordPrev, "End" )
					){
						if( 0 == _stricmp( szWordPrev, "Declare" ) ){
							nFuncId = 61;
						}else{
							nFuncId = 63;
						}
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;

					}else
					if( 0 == nParseCnt && 0 == _stricmp( szWord, "Sub" )
					 && 0 != _stricmp( szWordPrev, "End" )
					){
						if( 0 == _stricmp( szWordPrev, "Declare" ) ){
							nFuncId = 60;
						}else{
							nFuncId = 62;
						}
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}else
					if( 1 == nParseCnt ){
						strcpy( szFuncName, szWord );
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  �� ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(	0, nFuncLine - 1, &nPosX, &nPosY );
						pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
						nParseCnt = 0;
					}

					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* �L����ǂݍ��ݒ� */
			if( 2 == nMode ){
				if( '_' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					'\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i] ||
					'{' == pLine[i] ||
					'}' == pLine[i] ||
					'(' == pLine[i] ||
					')' == pLine[i] ||
					';' == pLine[i]	||
					'\'' == pLine[i] ||
					'/' == pLine[i]	||
					'-' == pLine[i]
				){
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}else{
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen && '\'' == pLine[i] ){
					break;
				}else{
					if( (1 == nCharChars && (
						'_' == pLine[i] ||
						'~' == pLine[i] ||
						('a' <= pLine[i] &&	pLine[i] <= 'z' )||
						('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
						('0' <= pLine[i] &&	pLine[i] <= '9' )
						) )
					 || 2 == nCharChars
					){
						nWordIdx = 0;

						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						nWordIdx = 0;
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			}
			i += (nCharChars - 1);
		}
	}
	return;
}
//	To Here June 23, 2001 N.Nakatani


// From Here 2001.12.03 hor
/*! �u�b�N�}�[�N���X�g�쐬�i������I�j

	20020119 aroka
	��s���}�[�N�Ώۂɂ���t���O bMarkUpBlankLineEnable �𓱓����܂����B
*/
void CEditDoc::MakeFuncList_BookMark( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int		nLineLen;
	int		nLineCount;
	int		i,j,nX,nY;
	char*	pszText;
	BOOL	bMarkUpBlankLineEnable = m_pShareData->m_Common.m_bMarkUpBlankLineEnable;	//! ��s���}�[�N�Ώۂɂ���t���O 20020119 aroka
	int		nNewLineLen	= m_cNewLineCode.GetLen();
	int		nLineLast	= m_cDocLineMgr.GetLineCount();

	for( nLineCount = 0; nLineCount <  nLineLast; ++nLineCount ){
		if(!m_cDocLineMgr.GetLineInfo(nLineCount)->IsBookMarked())continue;
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		// Jan, 16, 2002 hor
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if( nLineLen<=nNewLineLen && nLineCount< nLineLast ){
			  continue;
			}
		}// LTrim
		for( i = 0; i < nLineLen; ++i ){
			if( pLine[i] == ' ' ||
				pLine[i] == '\t'){
				continue;
			}else if( (unsigned char)pLine[i] == (unsigned char)0x81 && (unsigned char)pLine[i + 1] == (unsigned char)0x40 ){
				++i;
				continue;
			}
			break;
		}
		
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if(( i >= nLineLen-nNewLineLen && nLineCount< nLineLast )||
				( i >= nLineLen )) {
				continue;
			}
		}// RTrim
		for( j=nLineLen ; j>=i ; --j ){
			if( pLine[j] == CR ||
				pLine[j] == LF ||
				pLine[j] ==' ' ||
				pLine[j] =='\t'||
				pLine[j] =='\0'){
				continue;
			}else if( 1<j && (unsigned char)pLine[j-1] == (unsigned char)0x81 && (unsigned char)pLine[j] == (unsigned char)0x40 ){
				--j;
				continue;
			}else{
				break;
			}
		}
		nLineLen=j-i+1;
		pszText = new char[nLineLen + 1];
		memcpy( pszText, (const char *)&pLine[i], nLineLen );
		pszText[nLineLen] = '\0';
		m_cLayoutMgr.CaretPos_Phys2Log(	0, nLineCount, &nX, &nY );
		pcFuncInfoArr->AppendData( nLineCount+1, nY+1 , (char *)pszText, 0 );
		delete [] pszText;
	}
	return;
}
// To Here 2001.12.03 hor

// From Here Jan. 22, 2002 genta
/*! �ύX�t���O�̐ݒ�

	@param flag [in] �ݒ肷��l�Dtrue: �ύX�L�� / false: �ύX����
	@param redraw [in] true: �^�C�g���̍ĕ`����s�� / false: �s��Ȃ�
	
	@author genta
	@date 2002.01.22 �V�K�쐬
*/
void CEditDoc::SetModified( bool flag, bool redraw)
{
	if( m_bIsModified == flag )	//	�ύX���Ȃ���Ή������Ȃ�
		return;

	m_bIsModified = flag;
	if( redraw )
		SetParentCaption();
}
// From Here Jan. 22, 2002 genta

/*!
	�t�@�C�����̐ݒ�
	
	�t�@�C������ݒ肷��Ɠ����ɁC�E�B���h�E�A�C�R����K�؂ɐݒ肷��D
	
	@param szFile [in] �t�@�C���̃p�X��
	
	@author genta
	@date 2002.09.09
*/
void CEditDoc::SetFilePath(const char* szFile)
{
	strcpy( m_szFilePath, szFile );
	SetDocumentIcon();
}

/*! �t�@�C����(�p�X�Ȃ�)���擾����
	@autor Moca
	@date 2002.10.13
*/
const char * CEditDoc::GetFileName( void ) const
{
	const char *p, *pszName;
	pszName = p = GetFilePath();
	while( *p != '\0'  ){
		if( _IS_SJIS_1( (unsigned char)*p ) && _IS_SJIS_2( (unsigned char)p[1] ) ){
			p+=2;
		}else if( *p == '\\' ){
			pszName = p + 1;
			p++;
		}else{
			p++;
		}
	}
	return pszName;
}

/*!
	�A�C�R���̐ݒ�
	
	�^�C�v�ʐݒ�ɉ����ăE�B���h�E�A�C�R�����t�@�C���Ɋ֘A�Â���ꂽ���C
	�܂��͕W���̂��̂ɐݒ肷��D
	
	@author genta
	@date 2002.09.10
*/
void CEditDoc::SetDocumentIcon(void)
{
	HICON	hIconBig, hIconSmall;
	
	if( m_bGrepMode )	// Grep���[�h�̎��̓A�C�R����ύX���Ȃ�
		return;
	
	if( GetDocumentAttribute().m_bUseDocumentIcon )
		m_pcEditWnd->GetRelatedIcon( GetFilePath(), hIconBig, hIconSmall );
	else
		m_pcEditWnd->GetDefaultIcon( hIconBig, hIconSmall );

	m_pcEditWnd->SetWindowIcon( hIconBig, ICON_BIG );
	m_pcEditWnd->SetWindowIcon( hIconSmall, ICON_SMALL );
}

/*!
	�J�����g�t�@�C����MRU�ɓo�^����B
	�u�b�N�}�[�N���ꏏ�ɓo�^����B

	@date 2003.03.30 genta �쐬

*/
void CEditDoc::AddToMRU(void)
{
	FileInfo	fi;
	CMRU		cMRU;

	SetFileInfo( &fi );
	strcpy( fi.m_szMarkLines, m_cDocLineMgr.GetBookMarks() );

	//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	cMRU.Add( &fi );
}

/*[EOF]*/
