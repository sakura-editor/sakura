/*!	@file
	@brief CDicMgr�N���X

	@author Norio Nakatani
	@date	1998/11/05 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka, Moca
	Copyright (C) 2003, Moca
	Copyright (C) 2006, fon
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdio.h>
#include "CDicMgr.h"
#include "CMemory.h" // 2002/2/10 aroka �w�b�_����
#include "CRunningTimer.h"
#include "etc_uty.h"
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�


CDicMgr::CDicMgr()
{
	return;
}




CDicMgr::~CDicMgr()
{
	return;
}




/*!
	�L�[���[�h�̌���
	�ŏ��Ɍ��������L�[���[�h�̈Ӗ���Ԃ�
	
	@param[in] pszKey �����L�[���[�h
	@param[in] nCmpLen �����L�[���[�h�̒���
	@param[out] ppcmemKey ���������L�[���[�h�D�Ăяo�����̐ӔC�ŉ������D
	@param[out] ppcmemMean ���������L�[���[�h�ɑΉ����鎫�����e�D�Ăяo�����̐ӔC�ŉ������D
	@param[in] pszKeyWordHelpFile �L�[���[�h�w���v�t�@�C���̃p�X��
	@param[out] pLine ���������L�[���[�h�̃L�[���[�h�w���v�t�@�C�����ł̍s�ԍ�

	@date 2006.04.10 fon �����q�b�g�s��Ԃ�����pLine��ǉ�
*/
BOOL CDicMgr::Search( const char* pszKey, const int nCmpLen, CMemory** ppcmemKey, CMemory** ppcmemMean, const char* pszKeyWordHelpFile, int *pLine )
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CDicMgr::Search" );
#endif
//	char	szDir[_MAX_PATH];
//	long	lPathLen;
	long	i;
	FILE*	pFile;
	char	szLine[LINEREADBUFSIZE];
	char*	pszDelimit = " /// ";
	char*	pszWork;
	int		nRes;
	char*	pszToken;
	char*	pszKeySeps = ",\0";


	/* �����t�@�C�� */
	if( 0 >= lstrlen( pszKeyWordHelpFile ) ){
		return FALSE;
	}
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	pFile = _tfopen_absini( pszKeyWordHelpFile, "r" );
	if( NULL == pFile ){
		return FALSE;
	}
	for(int line=1 ;NULL != fgets( szLine, sizeof(szLine), pFile ); line++ ){	// 2006.04.10 fon
		pszWork = strstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != ';' ){
			*pszWork = '\0';
			pszWork += lstrlen( pszDelimit );

			/* �ŏ��̃g�[�N�����擾���܂��B */
			pszToken = strtok( szLine, pszKeySeps );
			while( NULL != pszToken ){
				nRes = _strnicmp( pszKey, pszToken, nCmpLen );	// 2006.04.10 fon
				if( 0 == nRes ){
					for( i = 0; i < (int)lstrlen(pszWork); ++i ){
						if( pszWork[i] == '\r' ||
							pszWork[i] == '\n' ){
							pszWork[i] = '\0';
							break;
						}
					}
					//�L�[���[�h�̃Z�b�g
					*ppcmemKey = new CMemory;	// 2006.04.10 fon
					(*ppcmemKey)->SetDataSz( pszToken );
					//�Ӗ��̃Z�b�g
					*ppcmemMean = new CMemory;
//					(*ppcmemMean)->SetData( pszWork, lstrlen(pszWork) );
					(*ppcmemMean)->SetDataSz( pszWork );

					fclose( pFile );
					*pLine = line;	// 2006.04.10 fon
					return TRUE;
				}
				pszToken = strtok( NULL, pszKeySeps );
			}
		}
	}
	fclose( pFile );
	return FALSE;
}





/*
||  ���͕⊮�L�[���[�h�̌���
||
||  �E���������������ׂĕԂ�(���s�ŋ�؂��ĕԂ�)
||  �E�w�肳�ꂽ���̍ő吔�𒴂���Ə����𒆒f����
||  �E������������Ԃ�
||
*/
int CDicMgr::HokanSearch(
			const char* pszKey,
			BOOL		bHokanLoHiCase,	/*�p�啶���������𓯈ꎋ����*/
			CMemory**	ppcmemKouho,
			int			nMaxKouho,	//Max��␔(0==������)
			const char* pszKeyWordFile
)
{
	FILE*	pFile;
	char	szLine[1024];
	int		nKeyLen;
	int		nKouhoNum;
	int		nRet;
	*ppcmemKouho = NULL;
	if( 0 >= lstrlen( pszKeyWordFile ) ){
		return 0;
	}
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	pFile = _tfopen_absini( pszKeyWordFile, "r" );
	if( NULL == pFile ){
		return 0;
	}
	nKouhoNum = 0;
	nKeyLen = lstrlen( pszKey );
	while( NULL != fgets( szLine, sizeof(szLine), pFile ) ){
		if( nKeyLen > (int)lstrlen( szLine ) ){
			continue;
		}
		if( szLine[0] == ';' ){
			continue;
		}
		if( szLine[nKeyLen] == '\r' || szLine[nKeyLen] == '\n' ){
			continue;
		}
		if( bHokanLoHiCase ){	/* �p�啶���������𓯈ꎋ���� */
			nRet = memicmp( pszKey, szLine, nKeyLen );
		}else{
			nRet = memcmp( pszKey, szLine, nKeyLen );
		}
		if( 0 == nRet ){
			if( NULL == *ppcmemKouho ){
				*ppcmemKouho = new CMemory;
				(*ppcmemKouho)->SetDataSz( szLine );
			}else{
				(*ppcmemKouho)->AppendSz( szLine );
			}
			++nKouhoNum;
			if( 0 != nMaxKouho && nMaxKouho <= nKouhoNum ){
				break;
			}
		}
	}
	fclose( pFile );
	return nKouhoNum;
}


/*[EOF]*/
