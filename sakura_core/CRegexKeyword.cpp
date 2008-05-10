/*!	@file
	@brief CRegexKeyword Library

	���K�\���L�[���[�h�������B
	BREGEXP.DLL�𗘗p����B

	@author MIK
	@date Nov. 17, 2001
*/
/*
	Copyright (C) 2001, MIK
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

#include "stdafx.h"
#include "global.h"
#include "CRegexKeyword.h"
#include "CBregexp.h"

#if 0
#include <stdio.h>
#define	MYDBGMSG(s) \
{\
	FILE	*fp;\
	fp = fopen("debug.log", "a");\
	fprintf(fp, "%08x: %ls  BMatch(%d)=%d, Use=%d, Idx=%d\n", &m_pTypes, s, &BMatch, BMatch, m_bUseRegexKeyword, m_nTypeIndex);\
	fclose(fp);\
}
#else
#define	MYDBGMSG(a)
#endif

//!	�R���X�g���N�^
/*!	@brief �R���X�g���N�^

	BREGEXP.DLL �������A���K�\���L�[���[�h���������s���B

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 2007.08.12 genta ���K�\��DLL�w��̂��߈����ǉ�
*/
CRegexKeyword::CRegexKeyword(LPCTSTR regexp_dll )
{
	InitDll( regexp_dll );	// 2007.08.12 genta �����ǉ�
	MYDBGMSG("CRegexKeyword")

	m_pTypes    = NULL;
	m_nTypeIndex = -1;

	RegexKeyInit();
}

//!	�f�X�g���N�^
/*!	@brief �f�X�g���N�^

	�R���p�C���ς݃f�[�^�̔j�����s���B
*/
CRegexKeyword::~CRegexKeyword()
{
	int	i;

	MYDBGMSG("~CRegexKeyword")
	//�R���p�C���ς݂̃o�b�t�@���������B
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( m_sInfo[i].pBregexp && IsAvailable() )
			BRegfree(m_sInfo[i].pBregexp);
		m_sInfo[i].pBregexp = NULL;
	}

	RegexKeyInit();

	m_nTypeIndex = -1;
	m_pTypes     = NULL;
//	m_pShareData = NULL;
}

//!	���K�\���L�[���[�h����������
/*!	@brief ���K�\���L�[���[�h������

	 ���K�\���L�[���[�h�Ɋւ���ϐ��ނ�����������B

	@retval TRUE ����
*/
BOOL CRegexKeyword::RegexKeyInit( void )
{
	int	i;

	MYDBGMSG("RegexKeyInit")
	m_nTypeIndex = -1;
	m_nCompiledMagicNumber = 0;
	m_bUseRegexKeyword = FALSE;
	m_nRegexKeyCount = 0;
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		m_sInfo[i].pBregexp = NULL;
#ifdef USE_PARENT
#else
		m_sInfo[i].sRegexKey.m_szKeyword[0] = '\0';
		m_sInfo[i].sRegexKey.m_nColorIndex = COLORIDX_REGEX1;
#endif
	}

	return TRUE;
}

//!	���݃^�C�v�ݒ菈��
/*!	@brief ���݃^�C�v�ݒ�

	���݂̃^�C�v�ݒ��ݒ肷��B

	@param pTypesPtr [in] �^�C�v�ݒ�\���̂ւ̃|�C���^

	@retval TRUE ����
	@retval FALSE ���s

	@note �^�C�v�ݒ肪�ς������ă��[�h���R���p�C������B
*/
BOOL CRegexKeyword::RegexKeySetTypes( STypeConfig *pTypesPtr )
{
	MYDBGMSG("RegexKeySetTypes")
	if( pTypesPtr == NULL ) 
	{
		m_pTypes = NULL;
		m_bUseRegexKeyword = FALSE;
		return FALSE;
	}

	if( pTypesPtr->m_bUseRegexKeyword == FALSE )
	{
		//OFF�ɂȂ����̂ɂ܂�ON�Ȃ�OFF�ɂ���B
		if( m_bUseRegexKeyword )
		{
			m_pTypes = NULL;
			m_bUseRegexKeyword = FALSE;
		}
		return FALSE;
	}

	if( m_pTypes               == pTypesPtr
	 && m_nCompiledMagicNumber == pTypesPtr->m_nRegexKeyMagicNumber
	/* && m_bUseRegexKeyword     == pTypesPtr->m_bUseRegexKeyword */ )
	{
		return TRUE;
	}

	m_pTypes = pTypesPtr;

	RegexKeyCompile();
	
	return TRUE;
}

//!	���K�\���L�[���[�h�R���p�C������
/*!	@brief ���K�\���L�[���[�h�R���p�C��

	���K�\���L�[���[�h���R���p�C������B

	@retval TRUE ����
	@retval FALSE ���s

	@note ���łɃR���p�C���ς݂̏ꍇ�͂����j������B
	�L�[���[�h�̓R���p�C���f�[�^�Ƃ��ē����ϐ��ɃR�s�[����B
	�擪�w��A�F�w�葤�̎g�p�E���g�p���`�F�b�N����B
*/
BOOL CRegexKeyword::RegexKeyCompile( void )
{
	int	i;
	int	matched;
	static const wchar_t dummy[2] = L"\0";
	struct RegexKeywordInfo	*rp;

	MYDBGMSG("RegexKeyCompile")
	//�R���p�C���ς݂̃o�b�t�@���������B
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( m_sInfo[i].pBregexp && IsAvailable() )
			BRegfree(m_sInfo[i].pBregexp);
		m_sInfo[i].pBregexp = NULL;
	}

	//�R���p�C���p�^�[��������ϐ��Ɉڂ��B
	m_nRegexKeyCount = 0;
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( m_pTypes->m_RegexKeywordArr[i].m_szKeyword[0] == '\0' ) break;
#ifdef USE_PARENT
#else
		strcpy(m_sInfo[i].sRegexKey.m_szKeyword, m_pTypes->m_RegexKeywordArr[i].m_szKeyword);
		m_sInfo[i].sRegexKey.m_nColorIndex = m_pTypes->m_RegexKeywordArr[i].m_nColorIndex;
#endif
		m_nRegexKeyCount++;
	}

	m_nTypeIndex = m_pTypes->m_nIdx;
	m_nCompiledMagicNumber = m_pTypes->m_nRegexKeyMagicNumber - 1;	//Not Compiled.
	m_bUseRegexKeyword  = m_pTypes->m_bUseRegexKeyword;
	if( m_bUseRegexKeyword == 0 ) return FALSE;

	if( ! IsAvailable() )
	{
		m_bUseRegexKeyword = 0;
		return FALSE;
	}

	//�p�^�[�����R���p�C������B
	for(i = 0; i < m_nRegexKeyCount; i++)
	{
#ifdef USE_PARENT
		rp = &m_pTypes->m_RegexKeywordArr[i];
#else
		rp = &m_sInfo[i].sRegexKey;
#endif

		if( RegexKeyCheckSyntax( rp->m_szKeyword ) == TRUE )
		{
			m_szMsg[0] = '\0';
			matched = BMatch(rp->m_szKeyword, dummy, dummy+1, &m_sInfo[i].pBregexp, m_szMsg);

			if( m_szMsg[0] == '\0' )	//�G���[���Ȃ����`�F�b�N����
			{
				//�擪�ȊO�͌������Ȃ��Ă悢
				if( wcsncmp( RK_HEAD_STR1, rp->m_szKeyword, RK_HEAD_STR1_LEN ) == 0
				 || wcsncmp( RK_HEAD_STR2, rp->m_szKeyword, RK_HEAD_STR2_LEN ) == 0
				 || wcsncmp( RK_HEAD_STR3, rp->m_szKeyword, RK_HEAD_STR3_LEN ) == 0
				)
				{
					m_sInfo[i].nHead = 1;
				}
				else
				{
					m_sInfo[i].nHead = 0;
				}

				if( COLORIDX_REGEX1  <= rp->m_nColorIndex
				 && COLORIDX_REGEX10 >= rp->m_nColorIndex )
				{
					//�F�w��Ń`�F�b�N�������ĂȂ���Ό������Ȃ��Ă��悢
					if( m_pTypes->m_ColorInfoArr[rp->m_nColorIndex].m_bDisp )
					{
						m_sInfo[i].nFlag = RK_EMPTY;
					}
					else
					{
						//���K�\���ł͐F�w��̃`�F�b�N������B
						m_sInfo[i].nFlag = RK_NOMATCH;
					}
				}
				else
				{
					//���K�\���ȊO�ł́A�F�w��`�F�b�N�͌��Ȃ��B
					//�Ⴆ�΁A���p���l�͐��K�\�����g���A��{�@�\���g��Ȃ��Ƃ����w������蓾�邽��
					m_sInfo[i].nFlag = RK_EMPTY;
				}
			}
			else
			{
				//�R���p�C���G���[�Ȃ̂Ō����Ώۂ���͂���
				m_sInfo[i].nFlag = RK_NOMATCH;
			}
		}
		else
		{
			//�����G���[�Ȃ̂Ō����Ώۂ���͂���
			m_sInfo[i].nFlag = RK_NOMATCH;
		}

	}

	m_nCompiledMagicNumber = m_pTypes->m_nRegexKeyMagicNumber;	//Compiled.

	return TRUE;
}

//!	�s�����J�n����
/*!	@brief �s�����J�n

	�s�������J�n����B

	@retval TRUE ����
	@retval FALSE ���s�܂��͌������Ȃ��w�肠��

	@note ���ꂼ��̍s�����̍ŏ��Ɏ��s����B
	�^�C�v�ݒ蓙���ύX����Ă���ꍇ�̓����[�h����B
*/
BOOL CRegexKeyword::RegexKeyLineStart( void )
{
	int	i;

	MYDBGMSG("RegexKeyLineStart")

	//����ɕK�v�ȃ`�F�b�N������B
	if( ( m_bUseRegexKeyword == FALSE )
	 || ( ! IsAvailable() )
	 || ( m_pTypes == NULL ) )
	{
		return FALSE;
	}

#if 0	//RegexKeySetTypes�Őݒ肳��Ă���͂��Ȃ̂Ŕp�~
	//���s��v�Ȃ�}�X�^����擾���ăR���p�C������B
	if( m_nCompiledMagicNumber != m_pTypes->m_nRegexKeyMagicNumber
	 || m_nTypeIndex           != m_pTypes->m_nIdx )
	{
		RegexKeyCompile();
	}
#endif

	//�����J�n�̂��߂ɃI�t�Z�b�g��񓙂��N���A����B
	for(i = 0; i < m_nRegexKeyCount; i++)
	{
		m_sInfo[i].nOffset = -1;
		//m_sInfo[i].nMatch  = RK_EMPTY;
		m_sInfo[i].nMatch  = m_sInfo[i].nFlag;
		m_sInfo[i].nStatus = RK_EMPTY;
	}

	return TRUE;
}

//!	���K�\����������
/*!	@brief ���K�\������

	���K�\���L�[���[�h����������B

	@param pLine [in] �P�s�̃f�[�^
	@param nPos [in] �����J�n�I�t�Z�b�g
	@param nLineLen [in] �P�s�̒���
	@param nMatchLen [out] �}�b�`��������
	@param nMatchColor [out] �}�b�`�����F�ԍ�

	@retval TRUE ��v
	@retval FALSE �s��v

	@note RegexKeyLineStart�֐��ɂ���ď���������Ă��邱�ƁB
*/
BOOL CRegexKeyword::RegexIsKeyword( const wchar_t *pLine, int nPos, int nLineLen, int *nMatchLen, int *nMatchColor )
{
	int	i, matched;

	MYDBGMSG("RegexIsKeyword")

	//����ɕK�v�ȃ`�F�b�N������B
	if( ( m_bUseRegexKeyword == FALSE )
	 || ( ! IsAvailable() )
#ifdef USE_PARENT
	 || ( m_pTypes == NULL )
#endif
	 /* || ( pLine == NULL ) */ )
	{
		return FALSE;
	}

	for(i = 0; i < m_nRegexKeyCount; i++)
	{
		if( m_sInfo[i].nMatch != RK_NOMATCH )  /* ���̍s�ɃL�[���[�h���Ȃ��ƕ������Ă��Ȃ� */
		{
			if( m_sInfo[i].nOffset == nPos )  /* �ȑO�����������ʂɈ�v���� */
			{
				*nMatchLen   = m_sInfo[i].nLength;
#ifdef USE_PARENT
				*nMatchColor = m_pTypes->m_RegexKeywordArr[i].m_nColorIndex;
#else
				*nMatchColor = m_sInfo[i].sRegexKey.m_nColorIndex;
#endif
				return TRUE;  /* �}�b�`���� */
			}

			/* �ȑO�̌��ʂ͂����Â��̂ōČ������� */
			if( m_sInfo[i].nOffset < nPos )
			{
#ifdef USE_PARENT
				matched = BMatch(m_pTypes->m_RegexKeywordArr[i].m_szKeyword, pLine+nPos, pLine+nLineLen,
					&m_sInfo[i].pBregexp, m_szMsg);
#else
				matched = BMatch(m_sInfo[i].sRegexKey.m_szKeyword, pLine+nPos, pLine+nLineLen,
					&m_sInfo[i].pBregexp, m_szMsg);
#endif
				if( matched )
				{
					m_sInfo[i].nOffset = m_sInfo[i].pBregexp->startp[0] - pLine;
					m_sInfo[i].nLength = m_sInfo[i].pBregexp->endp[0] - m_sInfo[i].pBregexp->startp[0];
					m_sInfo[i].nMatch  = RK_MATCH;
				
					/* �w��̊J�n�ʒu�Ń}�b�`���� */
					if( m_sInfo[i].nOffset == nPos )
					{
						if( m_sInfo[i].nHead != 1 || nPos == 0 )
						{
							*nMatchLen   = m_sInfo[i].nLength;
#ifdef USE_PARENT
							*nMatchColor = m_pTypes->m_RegexKeywordArr[i].m_nColorIndex;
#else
							*nMatchColor = m_sInfo[i].sRegexKey.m_nColorIndex;
#endif
							return TRUE;  /* �}�b�`���� */
						}
					}

					/* �s�擪��v�����鐳�K�\���ł͎��񂩂疳������ */
					if( m_sInfo[i].nHead == 1 )
					{
						m_sInfo[i].nMatch = RK_NOMATCH;
					}
				}
				else
				{
					/* ���̍s�ɂ��̃L�[���[�h�͂Ȃ� */
					m_sInfo[i].nMatch = RK_NOMATCH;
				}
			}
		}
	}  /* for */

	return FALSE;
}

BOOL CRegexKeyword::RegexKeyCheckSyntax(const wchar_t *s)
{
	const wchar_t	*p;
	int	length, i;
	static const wchar_t *kakomi[7 * 2] = {
		L"/",  L"/k",
		L"m/", L"/k",
		L"m#", L"#k",
		L"/",  L"/ki",
		L"m/", L"/ki",
		L"m#", L"#ki",
		NULL, NULL,
	};

	length = wcslen(s);

	for(i = 0; kakomi[i] != NULL; i += 2)
	{
		//���������m���߂�
		if( length > (int)wcslen(kakomi[i]) + (int)wcslen(kakomi[i+1]) )
		{
			//�n�܂���m���߂�
			if( wcsncmp(kakomi[i], s, wcslen(kakomi[i])) == 0 )
			{
				//�I�����m���߂�
				p = &s[length - wcslen(kakomi[i+1])];
				if( wcscmp(p, kakomi[i+1]) == 0 )
				{
					//����
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

//@@@ 2001.11.17 add end MIK


