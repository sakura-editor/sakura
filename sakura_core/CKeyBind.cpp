//	$Id$
/*!	@file
	@brief �L�[���蓖�ĂɊւ���N���X

	@author Norio Nakatani
	@date 1998/03/25 �V�K�쐬
	@date 1998/05/16 �N���X���Ƀf�[�^�������Ȃ��悤�ɕύX
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CKeyBind.h"
#include "debug.h"
#include "CSMacroMgr.h"// 2002/2/10 aroka
#include "CFuncLookup.h"
#include "KeyCode.h"// 2002/2/10 aroka
#include "CMemory.h"// 2002/2/10 aroka


CKeyBind::CKeyBind()
{
	return;
}


CKeyBind::~CKeyBind()
{
	return;
}




/*! Windows �A�N�Z�����[�^�̍쐬 */
HACCEL CKeyBind::CreateAccerelator(
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr
)
{
	ACCEL*	pAccelArr;
	int		nAccelArrNum;
	HACCEL	hAccel;
	int		i, j, k;

	/* �@�\�����蓖�Ă��Ă���L�[�̐����J�E���g */
	nAccelArrNum = 0;
	for( i = 0; i < nKeyNameArrNum; ++i ){
		if( 0 != pKeyNameArr[i].m_nKeyCode ){
			for( j = 0; j < 8; ++j ){
				if( 0 != pKeyNameArr[i].m_nFuncCodeArr[j] ){
					nAccelArrNum++;
				}
			}
		}
	}
//	nAccelArrNum = nKeyNameArrNum * 8;


	if( nAccelArrNum <= 0 ){
		/* �@�\���蓖�Ă��[�� */
		return NULL;
	}
	pAccelArr = new ACCEL[nAccelArrNum];
	k = 0;
	for( i = 0; i < nKeyNameArrNum; ++i ){
		if( 0 != pKeyNameArr[i].m_nKeyCode ){
			for( j = 0; j < 8; ++j ){
				if( 0 != pKeyNameArr[i].m_nFuncCodeArr[j] ){
					pAccelArr[k].fVirt = FNOINVERT | FVIRTKEY;;
					pAccelArr[k].key = pKeyNameArr[i].m_nKeyCode;
					pAccelArr[k].cmd = pKeyNameArr[i].m_nKeyCode | (((WORD)j)<<8) ;
					if( j & _SHIFT ){
						pAccelArr[k].fVirt |= FSHIFT;
					}
					if( j & _CTRL ){
						pAccelArr[k].fVirt |= FCONTROL;
					}
					if( j & _ALT ){
						pAccelArr[k].fVirt |= FALT;
					}
					k++;
				}
			}
		}
	}
	hAccel = ::CreateAcceleratorTable( pAccelArr, nAccelArrNum );
	delete [] pAccelArr;
	return hAccel;
}






/*! �A�N���Z���[�^���ʎq�ɑΉ�����R�}���h���ʎq��Ԃ��D
	�Ή�����A�N���Z���[�^���ʎq���Ȃ��ꍇ�܂��͋@�\�����蓖�Ă̏ꍇ��0��Ԃ��D
*/
int CKeyBind::GetFuncCode(
		WORD		nAccelCmd,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr
)
{
	int i;
	int nCmd = (int)( nAccelCmd & 0x00ff );
	int nSts = (int)( ( nAccelCmd & 0xff00 ) >> 8 );
	for( i = 0; i < nKeyNameArrNum; ++i ){
		if( nCmd == pKeyNameArr[i].m_nKeyCode ){
			return pKeyNameArr[i].m_nFuncCodeArr[nSts];
		}
	}
	return 0;
}






/*!
	@param hInstance [in] �C���X�^���X�n���h��
	@param nKeyNameArrNum [in] 
	@param nKeyNameArr [out] 
	@param cMemList
	@param pcFuncLookup [in] �@�\�ԍ������O�̑Ή������

	@return �@�\�����蓖�Ă��Ă���L�[�X�g���[�N�̐�
	
	@date Oct. 31, 2001 genta ���I�ȋ@�\���ɑΉ����邽�߈����ǉ�
*/
int CKeyBind::CreateKeyBindList(
		HINSTANCE	hInstance,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		CMemory&	cMemList,
		CFuncLookup* pcFuncLookup
)
{
	int		i;
	int		j;
	int		nValidKeys;
	char	pszStr[256];
	char	szFuncName[256];
	char	szFuncNameJapanese[256];

	nValidKeys = 0;
//	cMemList = "";
//	cMemList.SetData( "", strlen( "" ) );
	cMemList.SetDataSz( "" );
	char*	pszSHIFT = "Shift+";
	char*	pszCTRL = "Ctrl+";
	char*	pszALT = "Alt+";
//	char*	pszEQUAL = " = ";
	char*	pszTAB = "\t";

//	char*	pszCR = "\n";	//Feb. 17, 2001 JEPRO \n=0x0a=LF���s���R�[�h�ɂȂ��Ă��܂��̂�
	char*	pszCR = "\r\n";	//\r=0x0d=CR��ǉ�


	cMemList.AppendSz( "�L�[\t�@�\��\t�֐���\t�@�\�ԍ�\t�L�[�}�N���L�^��/�s��" );
	cMemList.AppendSz( pszCR );
	cMemList.AppendSz( "-----\t-----\t-----\t-----\t-----" );
	cMemList.AppendSz( pszCR );

	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; ++i ){
			if( 0 != pKeyNameArr[i].m_nFuncCodeArr[j] ){
				nValidKeys++;
				if( j & _SHIFT ){
					cMemList.AppendSz( pszSHIFT );
				}
				if( j & _CTRL ){
					cMemList.AppendSz( pszCTRL );
				}
				if( j & _ALT ){
					cMemList.AppendSz( pszALT );
				}
				cMemList.AppendSz( pKeyNameArr[i].m_szKeyName );
//				cMemList.AppendSz( pszEQUAL );
//				cMemList.AppendSz( pszTAB );
				//	Oct. 31, 2001 genta 
				if( !pcFuncLookup->Funccode2Name(
					pKeyNameArr[i].m_nFuncCodeArr[j],
					szFuncNameJapanese, 255 )){
					strcpy( szFuncNameJapanese, "---���O����`����Ă��Ȃ�-----" );
				}
				strcpy( szFuncName, ""/*"---unknown()--"*/ );

//				/* �@�\�����{�� */
//				::LoadString(
//					hInstance,
//					pKeyNameArr[i].m_nFuncCodeArr[j],
//					 szFuncNameJapanese, 255
//				);
				cMemList.AppendSz( pszTAB );
				cMemList.AppendSz( szFuncNameJapanese );

				/* �@�\ID���֐����C�@�\�����{�� */
				//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
//				CMacro::GetFuncInfoByID(
				CSMacroMgr::GetFuncInfoByID(
					hInstance,
					pKeyNameArr[i].m_nFuncCodeArr[j],
					szFuncName,
					szFuncNameJapanese
				);

				/* �֐��� */
				cMemList.AppendSz( pszTAB );
				cMemList.AppendSz( szFuncName );

				/* �@�\�ԍ� */
				cMemList.AppendSz( pszTAB );
				wsprintf( pszStr, "%d", pKeyNameArr[i].m_nFuncCodeArr[j] );
				cMemList.AppendSz( pszStr );

				/* �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ� */
				cMemList.AppendSz( pszTAB );
				//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
//				if( CMacro::CanFuncIsKeyMacro( pKeyNameArr[i].m_nFuncCodeArr[j] ) ){
				if( CSMacroMgr::CanFuncIsKeyMacro( pKeyNameArr[i].m_nFuncCodeArr[j] ) ){
					cMemList.AppendSz( "��" );
				}else{
					cMemList.AppendSz( "�~" );
				}



				cMemList.AppendSz( pszCR );
			}
		}
	}
//	delete [] pszStr;
	return nValidKeys;
}



/*! �@�\�ɑΉ�����L�[���̎擾 */
int CKeyBind::GetKeyStr(
		HINSTANCE	hInstance,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		CMemory&	cMemList,
		int			nFuncId
)
{
	int		i;
	int		j;
	char*	pszSHIFT = "Shift+";
	char*	pszCTRL = "Ctrl+";
	char*	pszALT = "Alt+";
//	cMemList.SetData( "", strlen( "" ) );
	cMemList.SetDataSz( "" );
	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; ++i ){
			if( nFuncId == pKeyNameArr[i].m_nFuncCodeArr[j] ){
				if( j & _SHIFT ){
//					cMemList.Append( pszSHIFT, strlen( pszSHIFT ) );
					cMemList.AppendSz( pszSHIFT );
				}
				if( j & _CTRL ){
//					cMemList.Append( pszCTRL, strlen( pszCTRL ) );
					cMemList.AppendSz( pszCTRL );
				}
				if( j & _ALT ){
//					cMemList.Append( pszALT, strlen( pszALT ) );
					cMemList.AppendSz( pszALT );
				}
//				cMemList.Append( pKeyNameArr[i].m_szKeyName, strlen( pKeyNameArr[i].m_szKeyName ) );
				cMemList.AppendSz( pKeyNameArr[i].m_szKeyName );
				return 1;
			}
		}
	}
	return 0;
}


/*! �@�\�ɑΉ�����L�[���̎擾(����) */
int CKeyBind::GetKeyStrList(
		HINSTANCE	hInstance,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		CMemory***	pppcMemList,
		int			nFuncId
)
{
	int		i;
	int		j;
	char*	pszSHIFT = "Shift+";
	char*	pszCTRL = "Ctrl+";
	char*	pszALT = "Alt+";
	int		nAssignedKeysNum;

	nAssignedKeysNum = 0;
	if( 0 == nFuncId ){
		return 0;
	}
	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; ++i ){
			if( nFuncId == pKeyNameArr[i].m_nFuncCodeArr[j] ){
				nAssignedKeysNum++;
			}
		}
	}
	if( 0 == nAssignedKeysNum ){
		return 0;
	}
	(*pppcMemList) = new CMemory*[nAssignedKeysNum + 1];
	for( i = 0; i < nAssignedKeysNum; ++i ){
		(*pppcMemList)[i] = new CMemory;
	}
	(*pppcMemList)[i] = NULL;


	nAssignedKeysNum = 0;
	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; ++i ){
			if( nFuncId == pKeyNameArr[i].m_nFuncCodeArr[j] ){
				if( j & _SHIFT ){
//					(*pppcMemList)[nAssignedKeysNum]->Append( pszSHIFT, strlen( pszSHIFT ) );
					(*pppcMemList)[nAssignedKeysNum]->AppendSz( pszSHIFT );
				}
				if( j & _CTRL ){
//					(*pppcMemList)[nAssignedKeysNum]->Append( pszCTRL, strlen( pszCTRL ) );
					(*pppcMemList)[nAssignedKeysNum]->AppendSz( pszCTRL );
				}
				if( j & _ALT ){
//					(*pppcMemList)[nAssignedKeysNum]->Append( pszALT, strlen( pszALT ) );
					(*pppcMemList)[nAssignedKeysNum]->AppendSz( pszALT );
				}
//				(*pppcMemList)[nAssignedKeysNum]->Append( pKeyNameArr[i].m_szKeyName, strlen( pKeyNameArr[i].m_szKeyName ) );
				(*pppcMemList)[nAssignedKeysNum]->AppendSz( pKeyNameArr[i].m_szKeyName );
				nAssignedKeysNum++;
			}
		}
	}
	return nAssignedKeysNum;
}


/*! ���j���[���x���̍쐬 */
char* CKeyBind::GetMenuLabel(
		HINSTANCE	hInstance,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		int			nFuncId,
		char*		pszLabel,
		BOOL		bKeyStr
)
{
	CMemory		cMemList;
//	int			i;


	if( 0 == strlen( pszLabel ) ){
		strcpy( pszLabel, "-- undefined name --" );
		::LoadString( hInstance, nFuncId, pszLabel, 255 );
	}


	/* �@�\�ɑΉ�����L�[����ǉ����邩 */
	if( bKeyStr ){
		/* �@�\�ɑΉ�����L�[���̎擾 */
		if( ( IDM_SELWINDOW <= nFuncId && nFuncId <= IDM_SELWINDOW + 999 )
		 || ( IDM_SELMRU <= nFuncId && nFuncId <= IDM_SELMRU + 999 )
		 || ( IDM_SELOPENFOLDER <= nFuncId && nFuncId <= IDM_SELOPENFOLDER + 999 )
		 ){
		}else{
			strcat( pszLabel, "\t" );
		}
		if( GetKeyStr( hInstance, nKeyNameArrNum, pKeyNameArr, cMemList, nFuncId ) ){
			strcat( pszLabel, cMemList.GetPtr( NULL ) );
		}
	}
	return pszLabel;
}


/*[EOF]*/
