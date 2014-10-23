/*!	@file
	@brief �A�E�g���C�����

	@author genta
	@date	2004.08.08 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta
	Copyright (C) 2002, frozen
	Copyright (C) 2003, zenryaku
	Copyright (C) 2005, genta, D.S.Koba, ���イ��

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <string.h>
#include "doc/CDocOutline.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLine.h"
#include "_main/global.h"
#include "outline/CFuncInfoArr.h"
#include "outline/CFuncInfo.h"
#include "charset/charcode.h"
#include "io/CTextStream.h"
#include "extmodule/CBregexp.h"
#include "util/other_util.h"



/*! ���[���t�@�C����1�s���Ǘ�����\����

	@date 2002.04.01 YAZAKI
	@date 2007.11.29 kobake ���O�ύX: oneRule��SOneRule
*/
struct SOneRule {
	wchar_t szMatch[256];
	int		nLength;
	wchar_t szText[256]; // RegexReplace���̒u���㕶����
	wchar_t szGroupName[256];
	int		nLv;
	int		nRegexOption;
	int		nRegexMode; // 0 ==�uMode=Regex�v, 1 == �uMode=RegexReplace�v
};



/*! ���[���t�@�C����ǂݍ��݁A���[���\���̂̔z����쐬����

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca ����nMaxCount��ǉ��B�o�b�t�@���`�F�b�N������悤�ɕύX
	@date 2013.06.02 _tfopen_absini,fgetws��CTextInputStream_AbsIni�ɕύX�BUTF-8�Ή��BRegex�Ή�
	@date 2014.06.20 RegexReplace ���K�\���u�����[�h�ǉ�
*/
int CDocOutline::ReadRuleFile( const TCHAR* pszFilename, SOneRule* pcOneRule, int nMaxCount, bool& bRegex, std::wstring& title )
{
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	CTextInputStream_AbsIni	file = CTextInputStream_AbsIni( pszFilename );
	if( !file.Good() ){
		return 0;
	}
	std::wstring	strLine;
	wchar_t			szLine[LINEREADBUFSIZE];
	wchar_t			szText[256];
	const wchar_t*	pszDelimit = L" /// ";
	const wchar_t*	pszKeySeps = L",\0";
	const wchar_t*	pszWork;
	wchar_t	cComment = L';';
	int nDelimitLen = wcslen( pszDelimit );
	int nCount = 0;
	bRegex = false;
	bool bRegexReplace = false;
	title = L"";
	int regexOption = CBregexp::optCaseSensitive;

	// �ʏ탂�[�h
	// key1,key2 /// GroupName,Lv=1
	// ���K�\�����[�h
	// RegexMode /// GroupName,Lv=1
	// ���K�\���u�����[�h
	// RegexReplace /// TitleReplace /// GroupName
	while( file.Good() && nCount < nMaxCount ){
		strLine = file.ReadLineW();
		pszWork = wcsstr( strLine.c_str(), pszDelimit );
		if( NULL != pszWork && 0 < strLine.length() && strLine[0] != cComment ){
			int nLen = pszWork - strLine.c_str();
			if( nLen < LINEREADBUFSIZE ){
				// szLine == �ukey1,key2�v
				wmemcpy(szLine, strLine.c_str(), nLen);
				szLine[nLen] = L'\0';
			}else{
				// ���̍s�͒�������
				continue;
			}
			pszWork += nDelimitLen;

			/* �ŏ��̃g�[�N�����擾���܂��B */
			const wchar_t* pszTextReplace = L"";
			wchar_t* pszToken;
			bool bTopDummy = false;
			bool bRegexRep2 = false;
			if( bRegex ){
				// regex�̂Ƃ���,��؂�ɂ��Ȃ�
				pszToken = szLine;
				if( szLine[0] == L'\0' ){
					if( 0 < nCount ){
						// ���Key �͖���
						pszToken = NULL;
					}else{
						// �ŏ��̗v�f�����Key��������_�~�[�v�f
						bTopDummy = true;
					}
				}
				if( bRegexReplace && pszToken ){
					const wchar_t* pszGroupDel = wcsstr( pszWork, pszDelimit );
					if( NULL != pszGroupDel && 0 < pszWork[0] != L'\0' ){
						// pszWork = �utitleRep /// group�v
						// pszGroupDel = �u /// group�v
						int nTitleLen = pszGroupDel - pszWork; // Len == 0 OK
						if( nTitleLen < _countof(szText) ){
							wcsncpy_s(szText, _countof(szText), pszWork, nTitleLen);
						}else{
							wcsncpy_s(szText, _countof(szText), pszWork, _TRUNCATE);
						}
						pszTextReplace = szText;
						bRegexRep2 = true;
						pszWork = pszGroupDel + nDelimitLen; // group
					}
				}
			}else{
				pszToken = wcstok( szLine, pszKeySeps );
				if( nCount == 0 && pszToken == NULL ){
					pszToken = szLine;
					bTopDummy = true;
				}
			}
			const WCHAR* p = wcsstr( pszWork, L",Lv=" );
			int nLv = 0;
			if( p ){
				nLv = _wtoi( p + 4 );
			}
			while( NULL != pszToken ){
				wcsncpy( pcOneRule[nCount].szMatch, pszToken, 255 );
				wcsncpy_s( pcOneRule[nCount].szText, _countof(pcOneRule[0].szText), pszTextReplace, _TRUNCATE );
				wcsncpy( pcOneRule[nCount].szGroupName, pszWork, 255 );
				pcOneRule[nCount].szMatch[255] = L'\0';
				pcOneRule[nCount].szGroupName[255] = L'\0';
				pcOneRule[nCount].nLv = nLv;
				pcOneRule[nCount].nLength = wcslen(pcOneRule[nCount].szMatch);
				pcOneRule[nCount].nRegexOption = regexOption;
				pcOneRule[nCount].nRegexMode = bRegexRep2 ? 1 : 0; // �����񂪐�����������ReplaceMode
				nCount++;
				if( bTopDummy || bRegex ){
					pszToken = NULL;
				}else{
					pszToken = wcstok( NULL, pszKeySeps );
				}
			}
		}else{
			if( 0 < strLine.length() && strLine[0] == cComment ){
				if( 13 <= strLine.length() && strLine.length() <= 14 && 0 == _wcsnicmp( strLine.c_str() + 1, L"CommentChar=", 12 ) ){
					if( 13 == strLine.length() ){
						cComment = L'\0';
					}else{
						cComment = strLine[13];
					}
				}else if( 11 == strLine.length() && 0 == wcsicmp( strLine.c_str() + 1, L"Mode=Regex" ) ){
					bRegex = true;
					bRegexReplace = false;
				}else if( 18 == strLine.length() && 0 == wcsicmp( strLine.c_str() + 1, L"Mode=RegexReplace" ) ){
					bRegex = true;
					bRegexReplace = true;
				}else if( 7 <= strLine.length() && 0 == _wcsnicmp( strLine.c_str() + 1, L"Title=", 6 ) ){
					title = strLine.c_str() + 7;
				}else if( 13 < strLine.length() && 0 == _wcsnicmp( strLine.c_str() + 1, L"RegexOption=", 12 ) ){
					int nCaseFlag = CBregexp::optCaseSensitive;
					regexOption = 0;
					for( int i = 13; i < (int)strLine.length(); i++ ){
						if( strLine[i] == L'i' ){
							nCaseFlag = 0;
						}else if( strLine[i] == L'g' ){
							regexOption |= CBregexp::optGlobal;
						}else if( strLine[i] == L'x' ){
							regexOption |= CBregexp::optExtend;
						}else if( strLine[i] == L'a' ){
							regexOption |= CBregexp::optASCII;
						}else if( strLine[i] == L'u' ){
							regexOption |= CBregexp::optUnicode;
						}else if( strLine[i] == L'd' ){
							regexOption |= CBregexp::optDefault;
						}else if( strLine[i] == L'l' ){
							regexOption |= CBregexp::optLocale;
						}else if( strLine[i] == L'R' ){
							regexOption |= CBregexp::optR;
						}
					}
					regexOption |= nCaseFlag;
				}
			}
		}
	}
	file.Close();
	return nCount;
}

/*! ���[���t�@�C�������ɁA�g�s�b�N���X�g���쐬

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca �l�X�g�̐[�����ő�l�𒴂���ƃo�b�t�@�I�[�o�[��������̂��C��
		�ő�l�ȏ�͒ǉ������ɖ�������
	@date 2007.11.29 kobake SOneRule test[1024] �ŃX�^�b�N�����Ă����̂��C��
*/
void CDocOutline::MakeFuncList_RuleFile( CFuncInfoArr* pcFuncInfoArr, std::tstring& sTitleOverride )
{
	/* ���[���t�@�C���̓��e���o�b�t�@�ɓǂݍ��� */
	auto_array_ptr<SOneRule> test(new SOneRule[1024]);	// 1024���B 2007.11.29 kobake �X�^�b�N�g�������Ȃ̂ŁA�q�[�v�Ɋm�ۂ���悤�ɏC���B
	bool bRegex;
	std::wstring title;
	int nCount = ReadRuleFile(m_pcDocRef->m_cDocType.GetDocumentAttribute().m_szOutlineRuleFilename, test.get(), 1024, bRegex, title );
	if ( nCount < 1 ){
		return;
	}
	if( 0 < title.size() ){
		sTitleOverride = to_tchar(title.c_str());
	}

	/*	�l�X�g�̐[���́A32���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�256�����܂ŋ��
		�i256�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int	nMaxStack = 32;	//	�l�X�g�̍Ő[
	int			nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t		pszStack[nMaxStack][256];
	wchar_t		nLvStack[nMaxStack];
	wchar_t		szTitle[256];			//	�ꎞ�̈�
	CBregexp*	pRegex = NULL;
	if( bRegex ){
		pRegex = new CBregexp[nCount];
		for( int i = 0; i < nCount; i++ ){
			if( 0 == test[i].nLength ){
				continue;
			}
			if( !InitRegexp( NULL, pRegex[i], true ) ){
				delete [] pRegex;
				return;
			}
			if( test[i].nRegexMode == 1 ){
				if( !pRegex[i].Compile(test[i].szMatch, test[i].szText, test[i].nRegexOption) ){
					std::wstring str = test[i].szMatch;
					str += L"\n";
					str += test[i].szText;
					ErrorMessage( NULL, LS(STR_DOCOUTLINE_REGEX),
						str.c_str(),
						pRegex[i].GetLastMessage()
					);
					delete [] pRegex;
					return;
				}
			}else if( !pRegex[i].Compile(test[i].szMatch, test[i].nRegexOption) ){
				ErrorMessage( NULL, LS(STR_DOCOUTLINE_REGEX),
					test[i].szMatch,
					pRegex[i].GetLastMessage()
				);
				delete [] pRegex;
				return;
			}
		}
	}
	// 1�߂���s�������ꍇ�́A���[�g�v�f�Ƃ���
	// ���ږ��̓O���[�v��
	if( test[0].nLength == 0 ){
		const wchar_t* g = test[0].szGroupName;
		wcscpy(pszStack[0], g);
		nLvStack[0] = test[0].nLv;
		const wchar_t *p = wcschr(g, L',');
		int len;
		if( p != NULL ){
			len = p - g;
		}else{
			len = wcslen(g);
		}
		CNativeW mem;
		mem.SetString(g, len);
		pcFuncInfoArr->AppendData( CLogicInt(1), CLayoutInt(1), mem.GetStringPtr(), FUNCINFO_NOCLIPTEXT, nDepth );
		nDepth = 1;
	}
	for( CLogicInt nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//�s�擾
		CLogicInt		nLineLen;
		const wchar_t*	pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			break;
		}

		//�s���̋󔒔�΂�
		int		i = 0;
		if( !bRegex ){
			for( i = 0; i < nLineLen; ++i ){
				if( pLine[i] == L' ' || pLine[i] == L'\t' || pLine[i] == L'�@'){
					continue;
				}
				break;
			}
			if( i >= nLineLen ){
				continue;
			}
		}

		//�擪���������o���L���̂����ꂩ�ł���΁A���֐i��
		wchar_t*		pszText = NULL;
		int		j;
		for( j = 0; j < nCount; j++ ){
			if( bRegex ){
				if( test[j].nRegexMode == 0 ){
					if( 0 < test[j].nLength && pRegex[j].Match( pLine, nLineLen, 0 ) ){
						wcscpy( szTitle, test[j].szGroupName );
						break;
					}
				}else{
					if( 0 < test[j].nLength && 0 < pRegex[j].Replace( pLine, nLineLen, 0 ) ){
						// pLine = "ABC123DEF"
						// test��szMatch = "\d+"
						// test��szText = "$&456"
						// GetString() = "ABC123456DEF"
						// pszText = "123456"
						int nIndex = pRegex[j].GetIndex();
						int nMatchLen = pRegex[j].GetMatchLen();
						int nTextLen = pRegex[j].GetStringLen() - nLineLen + nMatchLen;
						pszText = new wchar_t[nTextLen + 1];
						wmemcpy( pszText, pRegex[j].GetString() + nIndex, nTextLen );
						pszText[nTextLen] = L'\0';
						wcscpy( szTitle, test[j].szGroupName );
						break;
					}
				}
			}else{
				if ( 0 < test[j].nLength && 0 == wcsncmp( &pLine[i], test[j].szMatch, test[j].nLength ) ){
					wcscpy( szTitle, test[j].szGroupName );
					break;
				}
			}
		}
		if( j >= nCount ){
			continue;
		}
		if( 0 == wcscmp( szTitle, L"Except" ) ){
			continue;
		}

		/*	���[���Ƀ}�b�`�����s�́A�A�E�g���C�����ʂɕ\������B
		*/

		//�s�����񂩂���s����菜�� pLine -> pszText
		// ���K�\���u���̂Ƃ��͐ݒ�ς�
		if( NULL == pszText ){
			pszText = new wchar_t[nLineLen + 1];
			wmemcpy( pszText, &pLine[i], nLineLen );
			pszText[nLineLen] = L'\0';
			bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
			for( i = 0; pszText[i] != L'\0'; ++i ){
				if( WCODE::IsLineDelimiter(pszText[i]), bExtEol ){
					pszText[i] = L'\0';
					break;
				}
			}
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
		BOOL bAppend;
		bAppend = TRUE;
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
		else if( nMaxStack> k ){
			//	���܂܂łɓ������o�������݂��Ȃ������B
			//	Lv�������ꍇ�́A��v����܂ł����̂ڂ�
			for ( k = nDepth - 1; 0 <= k ; k-- ){
				if ( nLvStack[k] <= test[j].nLv ){
					k++;
					break;
				}
			}
			if( k < 0 ){
				k = 0;
			}
			wcscpy(pszStack[k], szTitle);
			nLvStack[k] = test[j].nLv;
			nDepth = k;
		}else{
			// 2002.11.03 Moca �ő�l�𒴂���ƃo�b�t�@�I�[�o�[�������邩��K������
			// nDepth = nMaxStack;
			bAppend = FALSE;
		}
		
		if( FALSE != bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	delete [] pRegex;
	return;
}



// From Here 2001.12.03 hor
/*! �u�b�N�}�[�N���X�g�쐬�i������I�j

	@date 2002.01.19 aroka ��s���}�[�N�Ώۂɂ���t���O bMarkUpBlankLineEnable �𓱓����܂����B
	@date 2005.10.11 ryoji "��@" �̉E�Q�o�C�g���S�p�󔒂Ɣ��肳�����̑Ώ�
	@date 2005.11.03 genta �����񒷏C���D�E�[�̃S�~������
*/
void CDocOutline::MakeFuncList_BookMark( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLogicInt		nLineCount;
	int		leftspace, pos_wo_space, k;
	wchar_t*	pszText;
	BOOL	bMarkUpBlankLineEnable = GetDllShareData().m_Common.m_sOutline.m_bMarkUpBlankLineEnable;	//! ��s���}�[�N�Ώۂɂ���t���O 20020119 aroka
	int		nNewLineLen	= m_pcDocRef->m_cDocEditor.m_cNewLineCode.GetLen();
	CLogicInt	nLineLast	= m_pcDocRef->m_cDocLineMgr.GetLineCount();
	int		nCharChars;

	for( nLineCount = CLogicInt(0); nLineCount <  nLineLast; ++nLineCount ){
		if(!CBookmarkGetter(m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)).IsBookmarked())continue;
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			break;
		}
		// Jan, 16, 2002 hor
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if( nLineLen<=nNewLineLen && nLineCount< nLineLast ){
			  continue;
			}
		}// LTrim
		for( leftspace = 0; leftspace < nLineLen; ++leftspace ){
			if( WCODE::IsBlank(pLine[leftspace]) ){
				continue;
			}
			break;
		}
		
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if(( leftspace >= nLineLen-nNewLineLen && nLineCount< nLineLast )||
				( leftspace >= nLineLen )) {
				continue;
			}
		}// RTrim
		// 2005.10.11 ryoji �E����k��̂ł͂Ȃ�������T���悤�ɏC���i"��@" �̉E�Q�o�C�g���S�p�󔒂Ɣ��肳�����̑Ώ��j
		k = pos_wo_space = leftspace;
		bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
		while( k < nLineLen ){
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, k );
			if( 1 == nCharChars ){
				if( !(WCODE::IsLineDelimiter(pLine[k], bExtEol) ||
						pLine[k] == WCODE::SPACE ||
						pLine[k] == WCODE::TAB ||
						WCODE::IsZenkakuSpace(pLine[k]) ||
						pLine[k] == L'\0') )
					pos_wo_space = k + nCharChars;
			}
			k += nCharChars;
		}
		//	Nov. 3, 2005 genta �����񒷌v�Z���̏C��
		{
			int nLen = pos_wo_space - leftspace;
			pszText = new wchar_t[nLen + 1];
			wmemcpy( pszText, &pLine[leftspace], nLen );
			pszText[nLen] = L'\0';
		}
		CLayoutPoint ptXY;
		//int nX,nY
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(	CLogicPoint(CLogicInt(0), nLineCount), &ptXY );
		pcFuncInfoArr->AppendData( nLineCount+CLogicInt(1), ptXY.GetY2()+CLayoutInt(1) , pszText, 0 );
		delete [] pszText;
	}
	return;
}
// To Here 2001.12.03 hor
