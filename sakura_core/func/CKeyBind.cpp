/*!	@file
	@brief �L�[���蓖�ĂɊւ���N���X

	@author Norio Nakatani
	@date 1998/03/25 �V�K�쐬
	@date 1998/05/16 �N���X���Ƀf�[�^�������Ȃ��悤�ɕύX
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, genta
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "func/CKeyBind.h"
#include "env/CShareData.h"
#include "macro/CSMacroMgr.h"// 2002/2/10 aroka


//�����⏕
/* KEYDATA�z��Ƀf�[�^���Z�b�g */
static void SetKeyNameArrVal(
	DLLSHAREDATA*	pShareData,
	int				nIdx,
	short			nKeyCode,
	const TCHAR*	pszKeyName,
	EFunctionCode	nFuncCode_0,
	EFunctionCode	nFuncCode_1,
	EFunctionCode	nFuncCode_2,
	EFunctionCode	nFuncCode_3,
	EFunctionCode	nFuncCode_4,
	EFunctionCode	nFuncCode_5,
	EFunctionCode	nFuncCode_6,
	EFunctionCode	nFuncCode_7
);


CKeyBind::CKeyBind()
{
}


CKeyBind::~CKeyBind()
{
}




/*! Windows �A�N�Z�����[�^�̍쐬
	@date 2007.02.22 ryoji �f�t�H���g�@�\���蓖�ĂɊւ��鏈����ǉ�
*/
HACCEL CKeyBind::CreateAccerelator(
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr
)
{
	ACCEL*	pAccelArr;
	HACCEL	hAccel;
	int		j, k;

	// �@�\�����蓖�Ă��Ă���L�[�̐����J�E���g -> nAccelArrNum
	int nAccelArrNum = 0;
	for( int i = 0; i < nKeyNameArrNum; ++i ){
		if( 0 != pKeyNameArr[i].m_nKeyCode ){
			for( j = 0; j < 8; ++j ){
				if( 0 != GetFuncCodeAt( pKeyNameArr[i], j ) ){
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
	for( int i = 0; i < nKeyNameArrNum; ++i ){
		if( 0 != pKeyNameArr[i].m_nKeyCode ){
			for( j = 0; j < 8; ++j ){
				if( 0 != GetFuncCodeAt( pKeyNameArr[i], j ) ){
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

	@date 2007.02.22 ryoji �f�t�H���g�@�\���蓖�ĂɊւ��鏈����ǉ�
*/
EFunctionCode CKeyBind::GetFuncCode(
		WORD		nAccelCmd,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	int nCmd = (int)LOBYTE(nAccelCmd);
	int nSts = (int)HIBYTE(nAccelCmd);
	if( nCmd == 0 ){ // mouse command
		for( int i = 0; i < nKeyNameArrNum; ++i ){
			if( nCmd == pKeyNameArr[i].m_nKeyCode ){
				return GetFuncCodeAt( pKeyNameArr[i], nSts, bGetDefFuncCode );
			}
		}
	}else{
		// 2012.12.10 aroka �L�[�R�[�h�������̃��[�v������
		DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();
		return GetFuncCodeAt( pKeyNameArr[pShareData->m_Common.m_sKeyBind.m_VKeyToKeyNameArr[nCmd]], nSts, bGetDefFuncCode );
	}
	return F_DEFAULT;
}






/*!
	@return �@�\�����蓖�Ă��Ă���L�[�X�g���[�N�̐�
	
	@date Oct. 31, 2001 genta ���I�ȋ@�\���ɑΉ����邽�߈����ǉ�
	@date 2007.02.22 ryoji �f�t�H���g�@�\���蓖�ĂɊւ��鏈����ǉ�
*/
int CKeyBind::CreateKeyBindList(
	HINSTANCE		hInstance,		//!< [in] �C���X�^���X�n���h��
	int				nKeyNameArrNum,	//!< [in]
	KEYDATA*		pKeyNameArr,	//!< [out]
	CNativeW&		cMemList,		//!<
	CFuncLookup*	pcFuncLookup,	//!< [in] �@�\�ԍ������O�̑Ή������
	BOOL			bGetDefFuncCode //!< [in] ON:�f�t�H���g�@�\���蓖�Ă��g��/OFF:�g��Ȃ� �f�t�H���g:TRUE
)
{
	int		i;
	int		j;
	int		nValidKeys;
	WCHAR	pszStr[256];
	WCHAR	szFuncName[256];
	WCHAR	szFuncNameJapanese[256];

	nValidKeys = 0;
//	cMemList = "";
//	cMemList.SetData( "", strlen( "" ) );
	cMemList.SetString(LTEXT(""));
	const WCHAR*	pszSHIFT = LTEXT("Shift+");
	const WCHAR*	pszCTRL  = LTEXT("Ctrl+");
	const WCHAR*	pszALT   = LTEXT("Alt+");
//	char*	pszEQUAL = " = ";
	const WCHAR*	pszTAB   = LTEXT("\t");

//	char*	pszCR = "\n";	//Feb. 17, 2001 JEPRO \n=0x0a=LF���s���R�[�h�ɂȂ��Ă��܂��̂�
	const WCHAR*	pszCR    = LTEXT("\r\n");	//\r=0x0d=CR��ǉ�


	cMemList.AppendString( LTEXT("�L�[\t�@�\��\t�֐���\t�@�\�ԍ�\t�L�[�}�N���L�^��/�s��") );
	cMemList.AppendString( pszCR );
	cMemList.AppendString( LTEXT("-----\t-----\t-----\t-----\t-----") );
	cMemList.AppendString( pszCR );

	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; ++i ){
			int iFunc = GetFuncCodeAt( pKeyNameArr[i], j, bGetDefFuncCode );

			if( 0 != iFunc ){
				nValidKeys++;
				if( j & _SHIFT ){
					cMemList.AppendString( pszSHIFT );
				}
				if( j & _CTRL ){
					cMemList.AppendString( pszCTRL );
				}
				if( j & _ALT ){
					cMemList.AppendString( pszALT );
				}
				cMemList.AppendString( to_wchar(pKeyNameArr[i].m_szKeyName) );
//				cMemList.AppendString( pszEQUAL );
//				cMemList.AppendString( pszTAB );
				//	Oct. 31, 2001 genta 
				if( !pcFuncLookup->Funccode2Name(
					iFunc,
					szFuncNameJapanese, 255 )){
					auto_strcpy( szFuncNameJapanese, LTEXT("---���O����`����Ă��Ȃ�-----") );
				}
				auto_strcpy( szFuncName, LTEXT("")/*"---unknown()--"*/ );

//				/* �@�\�����{�� */
//				::LoadString(
//					hInstance,
//					pKeyNameArr[i].m_nFuncCodeArr[j],
//					 szFuncNameJapanese, 255
//				);
				cMemList.AppendString( pszTAB );
				cMemList.AppendString( szFuncNameJapanese );

				/* �@�\ID���֐����C�@�\�����{�� */
				//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
//				CMacro::GetFuncInfoByID(
				CSMacroMgr::GetFuncInfoByID(
					hInstance,
					iFunc,
					szFuncName,
					szFuncNameJapanese
				);

				/* �֐��� */
				cMemList.AppendString( pszTAB );
				cMemList.AppendString( szFuncName );

				/* �@�\�ԍ� */
				cMemList.AppendString( pszTAB );
				auto_sprintf( pszStr, LTEXT("%d"), iFunc );
				cMemList.AppendString( pszStr );

				/* �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ� */
				cMemList.AppendString( pszTAB );
				//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
//				if( CMacro::CanFuncIsKeyMacro( pKeyNameArr[i].m_nFuncCodeArr[j] ) ){
				if( CSMacroMgr::CanFuncIsKeyMacro( iFunc ) ){
					cMemList.AppendString( LTEXT("��") );
				}else{
					cMemList.AppendString( LTEXT("�~") );
				}



				cMemList.AppendString( pszCR );
			}
		}
	}
//	delete [] pszStr;
	return nValidKeys;
}

/** �@�\�ɑΉ�����L�[���̃T�[�`(�⏕�֐�)

	�^����ꂽ�V�t�g��Ԃɑ΂��āC�w�肳�ꂽ�͈͂̃L�[�G���A����
	���Y�@�\�ɑΉ�����L�[�����邩�𒲂ׁC����������
	�Ή�����L�[��������Z�b�g����D
	
	�֐�����o��Ƃ��ɂ͌����J�n�ʒu(nKeyNameArrBegin)��
	���ɏ�������index��ݒ肷��D

	@param[in,out] nKeyNameArrBegin �����J�nINDEX (�I�����ɂ͎���̊J�nINDEX�ɏ�����������)
	@param[in] nKeyNameArrBegin �����I��INDEX + 1
	@param[in] pKeyNameArr �L�[�z��
	@param[in] nShiftState �V�t�g���
	@param[out] cMemList �L�[������ݒ��
	@param[in]	nFuncId �����Ώۋ@�\ID
	@param[in]	bGetDefFuncCode �W���@�\���擾���邩�ǂ���
*/
bool CKeyBind::GetKeyStrSub(
		int&		nKeyNameArrBegin,
		int			nKeyNameArrEnd,
		KEYDATA*	pKeyNameArr,
		int			nShiftState,
		CNativeT&	cMemList,
		int			nFuncId,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	const TCHAR*	pszSHIFT = _T("Shift+");
	const TCHAR*	pszCTRL  = _T("Ctrl+");
	const TCHAR*	pszALT   = _T("Alt+");

	int i;
	for( i = nKeyNameArrBegin; i < nKeyNameArrEnd; ++i ){
		if( nFuncId == GetFuncCodeAt( pKeyNameArr[i], nShiftState, bGetDefFuncCode ) ){
			if( nShiftState & _SHIFT ){
				cMemList.AppendString( pszSHIFT );
			}
			if( nShiftState & _CTRL ){
				cMemList.AppendString( pszCTRL );
			}
			if( nShiftState & _ALT ){
				cMemList.AppendString( pszALT );
			}
			cMemList.AppendString( pKeyNameArr[i].m_szKeyName );
			nKeyNameArrBegin = i + 1;
			return true;
		}
	}
	nKeyNameArrBegin = i;
	return false;
}


/** �@�\�ɑΉ�����L�[���̎擾
	@date 2007.02.22 ryoji �f�t�H���g�@�\���蓖�ĂɊւ��鏈����ǉ�
	@date 2007.11.04 genta �}�E�X�N���b�N���L�[���蓖�Ă̗D��x���グ��
	@date 2007.11.04 genta ���ʋ@�\�̃T�u���[�`����
*/
int CKeyBind::GetKeyStr(
		HINSTANCE	hInstance,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		CNativeT&	cMemList,
		int			nFuncId,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	int		i;
	int		j;
	cMemList.SetString(_T(""));

	//	��ɃL�[�����𒲍�����
	for( j = 0; j < 8; ++j ){
		for( i = MOUSEFUNCTION_KEYBEGIN; i < nKeyNameArrNum; /* 1�������Ă͂����Ȃ� */ ){
			if( GetKeyStrSub( i, nKeyNameArrNum, pKeyNameArr, j, cMemList, nFuncId, bGetDefFuncCode )){
				return 1;
			}
		}
	}

	//	��Ƀ}�E�X�����𒲍�����
	for( j = 0; j < 8; ++j ){
		for( i = 0; i < MOUSEFUNCTION_KEYBEGIN; /* 1�������Ă͂����Ȃ� */ ){
			if( GetKeyStrSub( i, nKeyNameArrNum, pKeyNameArr, j, cMemList, nFuncId, bGetDefFuncCode )){
				return 1;
			}
		}
	}
	return 0;
}


/** �@�\�ɑΉ�����L�[���̎擾(����)
	@date 2007.02.22 ryoji �f�t�H���g�@�\���蓖�ĂɊւ��鏈����ǉ�
	@date 2007.11.04 genta ���ʋ@�\�̃T�u���[�`����
*/
int CKeyBind::GetKeyStrList(
	HINSTANCE	hInstance,
	int			nKeyNameArrNum,
	KEYDATA*	pKeyNameArr,
	CNativeT***	pppcMemList,
	int			nFuncId,
	BOOL		bGetDefFuncCode /* = TRUE */
)
{
	int		i;
	int		j;
	const TCHAR*	pszSHIFT = _T("Shift+");
	const TCHAR*	pszCTRL  = _T("Ctrl+");
	const TCHAR*	pszALT   = _T("Alt+");
	int		nAssignedKeysNum;

	nAssignedKeysNum = 0;
	if( 0 == nFuncId ){
		return 0;
	}
	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; ++i ){
			if( nFuncId == GetFuncCodeAt( pKeyNameArr[i], j, bGetDefFuncCode ) ){
				nAssignedKeysNum++;
			}
		}
	}
	if( 0 == nAssignedKeysNum ){
		return 0;
	}
	(*pppcMemList) = new CNativeT*[nAssignedKeysNum + 1];
	for( i = 0; i < nAssignedKeysNum; ++i ){
		(*pppcMemList)[i] = new CNativeT;
	}
	(*pppcMemList)[i] = NULL;


	nAssignedKeysNum = 0;
	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; /* 1�������Ă͂����Ȃ� */ ){
			//	2007.11.04 genta ���ʋ@�\�̃T�u���[�`����
			if( GetKeyStrSub( i, nKeyNameArrNum, pKeyNameArr, j,
					*((*pppcMemList)[nAssignedKeysNum]), nFuncId, bGetDefFuncCode )){
				nAssignedKeysNum++;
			}
		}
	}
	return nAssignedKeysNum;
}




// �A�N�Z�X�L�[�t���̕�����̍쐬
TCHAR*	CKeyBind::MakeMenuLabel(const TCHAR* sName, const TCHAR* sKey)
{
	static	TCHAR	sLabel[300];
	const	TCHAR*	p;

	if (sKey == NULL || sKey[0] == L'\0') {
		return const_cast<TCHAR*>( to_tchar(sName) );
	}
	else {
		if (!CShareData::getInstance()->GetShareData()->m_Common.m_sMainMenu.m_bMainMenuKeyParentheses
			  && (p = auto_strchr( sName, sKey[0] )) != NULL) {
			// �������A�g�p���Ă��镶�����A�N�Z�X�L�[��
			auto_strcpy_s( sLabel, _countof(sLabel), sName );
			sLabel[p-sName] = _T('&');
			auto_strcpy_s( sLabel + (p-sName) + 1, _countof(sLabel), p );
		}
		else if( (p = auto_strchr( sName, _T('(') )) != NULL
			  && (p = auto_strchr( p, sKey[0] )) != NULL) {
			// (�t���̌�ɃA�N�Z�X�L�[
			auto_strcpy_s( sLabel, _countof(sLabel), sName );
			sLabel[p-sName] = _T('&');
			auto_strcpy_s( sLabel + (p-sName) + 1, _countof(sLabel), p );
		}
		else if (_tcscmp( sName + _tcslen(sName) - 3, _T("...") ) == 0) {
			// ����...
			auto_strcpy_s( sLabel, _countof(sLabel), sName );
			sLabel[_tcslen(sName) - 3] = '\0';						// ������...�����
			auto_strcat_s( sLabel, _countof(sLabel), _T("(&") );
			auto_strcat_s( sLabel, _countof(sLabel), sKey );
			auto_strcat_s( sLabel, _countof(sLabel), _T(")...") );
		}
		else {
			auto_sprintf_s( sLabel, _countof(sLabel), _T("%ts(&%ts)"), sName, sKey );
		}

		return sLabel;
	}
}

/*! ���j���[���x���̍쐬
	@date 2007.02.22 ryoji �f�t�H���g�@�\���蓖�ĂɊւ��鏈����ǉ�
	2010/5/17	�A�N�Z�X�L�[�̒ǉ�
*/
TCHAR* CKeyBind::GetMenuLabel(
		HINSTANCE	hInstance,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		int			nFuncId,
		TCHAR*      pszLabel,   //!< [in,out] �o�b�t�@��256�ȏ�Ɖ���
		const TCHAR*	pszKey,
		BOOL		bKeyStr,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	const int LABEL_MAX = 256;

	if( _T('\0') == pszLabel[0] ){
		::LoadString( hInstance, nFuncId, pszLabel, LABEL_MAX );
	}
	if( _T('\0') == pszLabel[0] ){
		_tcscpy( pszLabel, _T("-- undefined name --") );
	}
	// �A�N�Z�X�L�[�̒ǉ�	2010/5/17 Uchi
	_tcsncpy_s( pszLabel, LABEL_MAX, MakeMenuLabel( pszLabel, pszKey ), _TRUNCATE );

	/* �@�\�ɑΉ�����L�[����ǉ����邩 */
	if( bKeyStr ){
		CNativeT    cMemAccessKey;
		// 2010.07.11 Moca ���j���[���x���́u\t�v�̕t�������ύX
		// [�t�@�C��/�t�H���_/�E�B���h�E�ꗗ�ȊO]����[�A�N�Z�X�L�[������Ƃ��̂�]�ɕt������悤�ɕύX
		/* �@�\�ɑΉ�����L�[���̎擾 */
		if( GetKeyStr( hInstance, nKeyNameArrNum, pKeyNameArr, cMemAccessKey, nFuncId, bGetDefFuncCode ) ){
			// �o�b�t�@������Ȃ��Ƃ��͓���Ȃ�
			if( _tcslen( pszLabel ) + cMemAccessKey.GetStringLength() + 1 < LABEL_MAX ){
				_tcscat( pszLabel, _T("\t") );
				_tcscat( pszLabel, cMemAccessKey.GetStringPtr() );
			}
		}
	}
	return pszLabel;
}


/*! �L�[�̃f�t�H���g�@�\���擾����

	@param nKeyCode [in] �L�[�R�[�h
	@param nState [in] Shift,Ctrl,Alt�L�[���

	@return �@�\�ԍ�

	@date 2007.02.22 ryoji �V�K�쐬
*/
EFunctionCode CKeyBind::GetDefFuncCode( int nKeyCode, int nState )
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();
	if( pShareData == NULL )
		return F_DEFAULT;

	EFunctionCode nDefFuncCode = F_DEFAULT;
	if( nKeyCode == VK_F4 ){
		if( nState == _CTRL ){
			nDefFuncCode = F_FILECLOSE;	// ����(����)
			if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
				nDefFuncCode = F_WINCLOSE;	// ����
			}
		}
		else if( nState == _ALT ){
			nDefFuncCode = F_WINCLOSE;	// ����
			if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
				if( !pShareData->m_Common.m_sTabBar.m_bTab_CloseOneWin ){
					nDefFuncCode = F_GROUPCLOSE;	// �O���[�v�����	// 2007.06.20 ryoji
				}
			}
		}
	}
	return nDefFuncCode;
}


/*! ����̃L�[��񂩂�@�\�R�[�h���擾����

	@param KeyData [in] �L�[���
	@param nState [in] Shift,Ctrl,Alt�L�[���
	@param bGetDefFuncCode [in] �f�t�H���g�@�\���擾���邩�ǂ���

	@return �@�\�ԍ�

	@date 2007.03.07 ryoji �C�����C���֐�����ʏ�̊֐��ɕύX�iBCC�̍œK���o�O�΍�j
*/
EFunctionCode CKeyBind::GetFuncCodeAt( KEYDATA& KeyData, int nState, BOOL bGetDefFuncCode )
{
	if( 0 != KeyData.m_nFuncCodeArr[nState] )
		return KeyData.m_nFuncCodeArr[nState];
	if( bGetDefFuncCode )
		return GetDefFuncCode( KeyData.m_nKeyCode, nState );
	return F_DEFAULT;
}















//	Sep. 14, 2000 JEPRO
//	Shift+F1 �Ɂu�R�}���h�ꗗ�v, Alt+F1 �Ɂu�w���v�ڎ��v, Shift+Alt+F1 �Ɂu�L�[���[�h�����v��ǉ�	//Nov. 25, 2000 JEPRO �E���Ă����̂��C���E����
//Dec. 25, 2000 JEPRO Shift+Ctrl+F1 �Ɂu�o�[�W�������v��ǉ�
// 2001.12.03 hor F2�Ƀu�b�N�}�[�N�֘A������
//Sept. 21, 2000 JEPRO	Ctrl+F3 �Ɂu�����}�[�N�̃N���A�v��ǉ�
//Aug. 12, 2002 ai	Ctrl+Shift+F3 �Ɂu�����J�n�ʒu�֖߂�v��ǉ�
//Oct. 7, 2000 JEPRO	Alt+F4 �Ɂu�E�B���h�E�����v, Shift+Alt+F4 �Ɂu���ׂẴE�B���h�E�����v��ǉ�
//	Ctrl+F4�Ɋ��蓖�Ă��Ă����u�c���ɕ����v���u����(����)�v�ɕύX�� Shift+Ctrl+F4 �Ɂu���ĊJ���v��ǉ�
//Jan. 14, 2001 Ctrl+Alt+F4 �Ɂu�e�L�X�g�G�f�B�^�̑S�I���v��ǉ�
//Jun. 2001�u�T�N���G�f�B�^�̑S�I���v�ɉ���
//2006.10.21 ryoji Alt+F4 �ɂ͉������蓖�ĂȂ��i�f�t�H���g�̃V�X�e���R�}���h�u����v�����s�����悤�Ɂj
//2007.02.13 ryoji Shift+Ctrl+F4��F_WIN_CLOSEALL����F_EXITALLEDITORS�ɕύX
//2007.02.22 ryoji Ctrl+F4 �ւ̊��蓖�Ă��폜�i�f�t�H���g�̃R�}���h�����s�j
//	Sep. 20, 2000 JEPRO Ctrl+F5 �Ɂu�O���R�}���h���s�v��ǉ�  �Ȃ��}�N������CMMAND ����COMMAND �ɕύX�ς�
//Oct. 28, 2000 F5 �́u�ĕ`��v�ɕύX	//Jan. 14, 2001 Alt+F5 �Ɂuuudecode���ĕۑ��v, Ctrl+ Alt+F5 �ɁuTAB���󔒁v��ǉ�
//	May 28, 2001 genta	S-C-A-F5��SPACE-to-TAB��ǉ�
//Jan. 14, 2001 JEPRO	Ctrl+F6 �Ɂu�������v, Alt+F6 �ɁuBase64�f�R�[�h���ĕۑ��v��ǉ�
// 2007.11.15 nasukoji	�g���v���N���b�N�E�N�A�h���v���N���b�N�Ή�
//Jan. 14, 2001 JEPRO	Ctrl+F7 �Ɂu�啶���v, Alt+F7 �ɁuUTF-7��SJIS�R�[�h�ϊ��v, Shift+Alt+F7 �ɁuSJIS��UTF-7�R�[�h�ϊ��v, Ctrl+Alt+F7 �ɁuUTF-7�ŊJ�������v��ǉ�
//Nov. 9, 2000 JEPRO	Shift+F8 �ɁuCRLF���s�ŃR�s�[�v��ǉ�
//Jan. 14, 2001 JEPRO	Ctrl+F8 �Ɂu�S�p�����p�v, Alt+F8 �ɁuUTF-8��SJIS�R�[�h�ϊ��v, Shift+Alt+F8 �ɁuSJIS��UTF-8�R�[�h�ϊ��v, Ctrl+Alt+F8 �ɁuUTF-8�ŊJ�������v��ǉ�
//Jan. 14, 2001 JEPRO	Ctrl+F9 �Ɂu���p�{�S�Ђ灨�S�p�E�J�^�J�i�v, Alt+F9 �ɁuUnicode��SJIS�R�[�h�ϊ��v, Ctrl+Alt+F9 �ɁuUnicode�ŊJ�������v��ǉ�
//Oct. 28, 2000 JEPRO F10 �ɁuSQL*Plus�Ŏ��s�v��ǉ�(F5����̈ړ�)
//Jan. 14, 2001 JEPRO	Ctrl+F10 �Ɂu���p�{�S�J�^���S�p�E�Ђ炪�ȁv, Alt+F10 �ɁuEUC��SJIS�R�[�h�ϊ��v, Shift+Alt+F10 �ɁuSJIS��EUC�R�[�h�ϊ��v, Ctrl+Alt+F10 �ɁuEUC�ŊJ�������v��ǉ�
//Jan. 14, 2001 JEPRO	Shift+F11 �ɁuSQL*Plus���A�N�e�B�u�\���v, Ctrl+F11 �Ɂu���p�J�^�J�i���S�p�J�^�J�i�v, Alt+F11 �ɁuE-Mail(JIS��SJIS)�R�[�h�ϊ��v, Shift+Alt+F11 �ɁuSJIS��JIS�R�[�h�ϊ��v, Ctrl+Alt+F11 �ɁuJIS�ŊJ�������v��ǉ�
//Jan. 14, 2001 JEPRO	Ctrl+F12 �Ɂu���p�J�^�J�i���S�p�Ђ炪�ȁv, Alt+F12 �Ɂu�������ʁ�SJIS�R�[�h�ϊ��v, Ctrl+Alt+F11 �ɁuSJIS�ŊJ�������v��ǉ�
//Sept. 1, 2000 JEPRO	Alt+Enter �Ɂu�t�@�C���̃v���p�e�B�v��ǉ�	//Oct. 15, 2000 JEPRO Ctrl+Enter �Ɂu�t�@�C�����e��r�v��ǉ�
//Oct. 7, 2000 JEPRO �����̂Ŗ��̂��ȗ��`�ɕύX(BackSpace��BkSp)
//Oct. 7, 2000 JEPRO ���̂�VC++�ɍ��킹�ȗ��`�ɕύX(Insert��Ins)
//Oct. 7, 2000 JEPRO ���̂�VC++�ɍ��킹�ȗ��`�ɕύX(Delete��Del)
//Jun. 26, 2001 JEPRO	Shift+Del �Ɂu�؂���v��ǉ�
//Oct. 7, 2000 JEPRO	Shift+Ctrl+Alt+���Ɂu�c�����ɍő剻�v��ǉ�
//Jun. 27, 2001 JEPRO
//	Ctrl+���Ɋ��蓖�Ă��Ă����u�J�[�\����ړ�(�Q�s����)�v���u�e�L�X�g���P�s���փX�N���[���v�ɕύX
//2001.02.10 by MIK Shift+Ctrl+Alt+���Ɂu�������ɍő剻�v��ǉ�
//Sept. 14, 2000 JEPRO
//	Ctrl+���Ɋ��蓖�Ă��Ă����u�E�N���b�N���j���[�v���u�J�[�\�����ړ�(�Q�s����)�v�ɕύX
//	����ɕt�����Ă���Ɂu�E�N���b�N���j���[�v��Ctrl�{Alt�{���ɕύX
//Jun. 27, 2001 JEPRO
//	Ctrl+���Ɋ��蓖�Ă��Ă����u�J�[�\�����ړ�(�Q�s����)�v���u�e�L�X�g���P�s��փX�N���[���v�ɕύX
//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn �ɂ��ꂼ��u�P�y�[�W�_�E���v, �u(�I��)�P�y�[�W�_�E���v��ǉ�
//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�Ɍ���(RollUp��PgDn) //Oct. 10, 2000 JEPRO ���̕ύX
//2001.12.03 hor 1Page/HalfPage ���ւ�
//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn �ɂ��ꂼ��u�P�y�[�W�A�b�v�v, �u(�I��)�P�y�[�W�A�b�v�v��ǉ�
//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�Ɍ���(RollDown��PgUp) //Oct. 10, 2000 JEPRO ���̕ύX
//2001.12.03 hor 1Page/HalfPage ���ւ�
//Oct. 7, 2000 JEPRO ���̂�VC++�ɍ��킹�ȗ��`�ɕύX(SpaceBar��Space)
//Oct. 7, 2000 JEPRO	Ctrl+0 ���u�^�C�v�ʐݒ�ꗗ�v���u����`�v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+0 �Ɂu�J�X�^�����j���[10�v, Shift+Alt+0 �Ɂu�J�X�^�����j���[20�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+1 ���u�^�C�v�ʐݒ�v���u�c�[���o�[�̕\���v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+1 �Ɂu�J�X�^�����j���[1�v, Shift+Alt+1 �Ɂu�J�X�^�����j���[11�v��ǉ�
//Jan. 19, 2001 JEPRO	Shift+Ctrl+1 �Ɂu�J�X�^�����j���[21�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+2 ���u���ʐݒ�v���u�t�@���N�V�����L�[�̕\���v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+2 ���u�A�E�g�v�b�g�v���u�J�X�^�����j���[2�v�ɕύX���u�A�E�g�v�b�g�v�� Alt+O �Ɉړ�, Shift+Alt+2 �Ɂu�J�X�^�����j���[12�v��ǉ�
//Jan. 19, 2001 JEPRO	Shift+Ctrl+2 �Ɂu�J�X�^�����j���[22�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+3 ���u�t�H���g�ݒ�v���u�X�e�[�^�X�o�[�̕\���v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+3 �Ɂu�J�X�^�����j���[3�v, Shift+Alt+3 �Ɂu�J�X�^�����j���[13�v��ǉ�
//Jan. 19, 2001 JEPRO	Shift+Ctrl+3 �Ɂu�J�X�^�����j���[23�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+4 ���u�c�[���o�[�̕\���v���u�^�C�v�ʐݒ�ꗗ�v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+4 �Ɂu�J�X�^�����j���[4�v, Shift+Alt+4 �Ɂu�J�X�^�����j���[14�v��ǉ�
//Jan. 19, 2001 JEPRO	Shift+Ctrl+4 �Ɂu�J�X�^�����j���[24�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+5 ���u�t�@���N�V�����L�[�̕\���v���u�^�C�v�ʐݒ�v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+5 �Ɂu�J�X�^�����j���[5�v, Shift+Alt+5 �Ɂu�J�X�^�����j���[15�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+6 ���u�X�e�[�^�X�o�[�̕\���v���u���ʐݒ�v�ɕύX
//Jan. 13, 2001 JEPRO	Alt+6 �Ɂu�J�X�^�����j���[6�v, Shift+Alt+6 �Ɂu�J�X�^�����j���[16�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+7 �Ɂu�t�H���g�ݒ�v��ǉ�
//Jan. 13, 2001 JEPRO	Alt+7 �Ɂu�J�X�^�����j���[7�v, Shift+Alt+7 �Ɂu�J�X�^�����j���[17�v��ǉ�
//Jan. 13, 2001 JEPRO	Alt+8 �Ɂu�J�X�^�����j���[8�v, Shift+Alt+8 �Ɂu�J�X�^�����j���[18�v��ǉ�
//Jan. 13, 2001 JEPRO	Alt+9 �Ɂu�J�X�^�����j���[9�v, Shift+Alt+9 �Ɂu�J�X�^�����j���[19�v��ǉ�
//2001.12.06 hor Alt+A ���uSORT_ASC�v�Ɋ���
//Jan. 13, 2001 JEPRO	Ctrl+B �Ɂu�u���E�Y�v��ǉ�
//Jan. 16, 2001 JEPRO	SHift+Ctrl+C �Ɂu.h�Ɠ�����.c(�Ȃ����.cpp)���J���v��ǉ�
//Feb. 07, 2001 JEPRO	SHift+Ctrl+C ���u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v���u������C/C++�w�b�_(�\�[�X)���J���v�ɕύX
//Jan. 16, 2001 JEPRO	Ctrl+D �Ɂu�P��؂���v, Shift+Ctrl+D �Ɂu�P��폜�v��ǉ�
//2001.12.06 hor Alt+D ���uSORT_DESC�v�Ɋ���
//Oct. 7, 2000 JEPRO	Ctrl+Alt+E �Ɂu�d�˂ĕ\���v��ǉ�
//Jan. 16, 2001	JEPRO	Ctrl+E �Ɂu�s�؂���(�܂�Ԃ��P��)�v, Shift+Ctrl+E �Ɂu�s�폜(�܂�Ԃ��P��)�v��ǉ�
//Oct. 07, 2000 JEPRO	Ctrl+Alt+H �Ɂu�㉺�ɕ��ׂĕ\���v��ǉ�
//Jan. 16, 2001 JEPRO	Ctrl+H ���u�J�[�\���O���폜�v���u�J�[�\���s���E�B���h�E�����ցv�ɕύX��	Shift+Ctrl+H �Ɂu.c�܂���.cpp�Ɠ�����.h���J���v��ǉ�
//Feb. 07, 2001 JEPRO	SHift+Ctrl+H ���u.c�܂���.cpp�Ɠ�����.h���J���v���u������C/C++�w�b�_(�\�[�X)���J���v�ɕύX
//Jan. 21, 2001	JEPRO	Ctrl+I �Ɂu�s�̓�d���v��ǉ�
//Jan. 16, 2001	JEPRO	Ctrl+K �Ɂu�s���܂Ő؂���(���s�P��)�v, Shift+Ctrl+E �Ɂu�s���܂ō폜(���s�P��)�v��ǉ�
//Jan. 14, 2001 JEPRO	Ctrl+Alt+L �Ɂu�������v, Shift+Ctrl+Alt+L �Ɂu�啶���v��ǉ�
//Jan. 16, 2001 Ctrl+L ���u�J�[�\���s���E�B���h�E�����ցv���u�L�[�}�N���̓ǂݍ��݁v�ɕύX���u�J�[�\���s���E�B���h�E�����ցv�� Ctrl+H �Ɉړ�
//2001.12.03 hor Alt+L ���uLTRIM�v�Ɋ���
//Jan. 16, 2001 JEPRO	Ctrl+M �Ɂu�L�[�}�N���̕ۑ��v��ǉ�
//2001.12.06 hor Alt+M ���uMERGE�v�Ɋ���
//Oct. 20, 2000 JEPRO	Alt+N �Ɂu�ړ�����: ���ցv��ǉ�
//Jan. 13, 2001 JEPRO	Alt+O �Ɂu�A�E�g�v�b�g�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+P �Ɂu����v, Shift+Ctrl+P �Ɂu����v���r���[�v, Ctrl+Alt+P �Ɂu�y�[�W�ݒ�v��ǉ�
//Oct. 20, 2000 JEPRO	Alt+P �Ɂu�ړ�����: �O�ցv��ǉ�
//Jan. 24, 2001	JEPRO	Ctrl+Q �Ɂu�L�[���蓖�Ĉꗗ���R�s�[�v��ǉ�
//2001.12.03 hor Alt+R ���uRTRIM�v�Ɋ���
//Oct. 7, 2000 JEPRO	Shift+Ctrl+S �Ɂu���O��t���ĕۑ��v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+Alt+T �Ɂu���E�ɕ��ׂĕ\���v��ǉ�
//Jan. 21, 2001	JEPRO	Ctrl+T �Ɂu�^�O�W�����v�v, Shift+Ctrl+T �Ɂu�^�O�W�����v�o�b�N�v��ǉ�
//Oct. 7, 2000 JEPRO	Ctrl+Alt+U �Ɂu���݂̃E�B���h�E���Ő܂�Ԃ��v��ǉ�
//Jan. 16, 2001	JEPRO	Ctrl+U �Ɂu�s���܂Ő؂���(���s�P��)�v, Shift+Ctrl+U �Ɂu�s���܂ō폜(���s�P��)�v��ǉ�
//Jan. 13, 2001 JEPRO	Alt+X ���u�J�X�^�����j���[1�v���u����`�v�ɕύX���u�J�X�^�����j���[1�v�� Alt+1 �Ɉړ�
//Oct. 7, 2000 JEPRO	Shift+Ctrl+- �Ɂu�㉺�ɕ����v��ǉ�
// 2002.02.08 hor Ctrl+-�Ƀt�@�C�������R�s�[��ǉ�
//Oct. 7, 2000 JEPRO	Shift+Ctrl+\ �Ɂu���E�ɕ����v��ǉ�
//Sept. 20, 2000 JEPRO	Ctrl+@ �Ɂu�t�@�C�����e��r�v��ǉ�  //Oct. 15, 2000 JEPRO�u�I��͈͓��S�s�R�s�[�v�ɕύX
//	Aug. 16, 2000 genta
//	���Ό����̊��ʂɂ����ʌ�����ǉ�
//Oct. 7, 2000 JEPRO	Shift+Ctrl+; �Ɂu�c���ɕ����v��ǉ�	//Jan. 16, 2001	Alt+; �Ɂu���t�}���v��ǉ�
//Sept. 14, 2000 JEPRO	Ctrl+: �Ɂu�I��͈͓��S�s�s�ԍ��t���R�s�[�v��ǉ�	//Jan. 16, 2001	Alt+: �Ɂu�����}���v��ǉ�
//Sept. 14, 2000 JEPRO	Ctrl+. �Ɂu�I��͈͓��S�s���p���t���R�s�[�v��ǉ�
//	Nov. 15, 2000 genta PC/AT�L�[�{�[�h�ɍ��킹�ăL�[�R�[�h��ύX
//	PC98�~�ς̂��߁C�]���̃L�[�R�[�h�ɑΉ����鍀�ڂ�ǉ��D
//Oct. 7, 2000 JEPRO	�����ĕ\��������Ȃ������łĂ��Ă��܂��̂ŃA�v���P�[�V�����L�[���A�v���L�[�ɒZ�k
//2008.05.03 kobake �ǐ����������ቺ���Ă����̂ŁA�����𐮗��B
// 2008.05.30 nasukoji	Ctrl+Alt+S �Ɂu�w�茅�Ő܂�Ԃ��v��ǉ�
// 2008.05.30 nasukoji	Ctrl+Alt+W �Ɂu�E�[�Ő܂�Ԃ��v��ǉ�
// 2008.05.30 nasukoji	Ctrl+Alt+X �Ɂu�܂�Ԃ��Ȃ��v��ǉ�

/*!	@brief ���L������������/�L�[���蓖��

	�f�t�H���g�L�[���蓖�Ċ֘A�̏���������

	@date 2005.01.30 genta CShareData::Init()���番��
	@date 2007.11.04 genta �L�[�ݒ萔��DLLSHARE�̗̈�𒴂�����N���ł��Ȃ��悤��
*/
bool CShareData::InitKeyAssign(DLLSHAREDATA* pShareData)
{
	/********************/
	/* ���ʐݒ�̋K��l */
	/********************/
	struct KEYDATAINIT {
		short			nKeyCode;		//!< Key Code (0 for non-keybord button)
		const TCHAR*	pszKeyName;		//!< Key Name (for display)
		EFunctionCode	nFuncCode_0;	//!<                      Key
		EFunctionCode	nFuncCode_1;	//!< Shift +              Key
		EFunctionCode	nFuncCode_2;	//!<         Ctrl +       Key
		EFunctionCode	nFuncCode_3;	//!< Shift + Ctrl +       Key
		EFunctionCode	nFuncCode_4;	//!<                Alt + Key
		EFunctionCode	nFuncCode_5;	//!< Shift +        Alt + Key
		EFunctionCode	nFuncCode_6;	//!<         Ctrl + Alt + Key
		EFunctionCode	nFuncCode_7;	//!< Shift + Ctrl + Alt + Key
	};
#define _SQL_RUN	F_PLSQL_COMPILE_ON_SQLPLUS
#define _COPYWITHLINENUM	F_COPYLINESWITHLINENUMBER
	static KEYDATAINIT	KeyDataInit[] = {
	//Sept. 1, 2000 Jepro note: key binding
	//Feb. 17, 2001 jepro note 2: ���Ԃ�2�i�ŉ���3�r�b�g[Alt][Ctrl][Shift]�̑g�����̏�(�����2���������l)
	//		0,		1,		 2(000), 3(001),4(010),	5(011),		6(100),	7(101),		8(110),		9(111)

		/* �}�E�X�{�^�� */
		//keycode,	keyname,			�Ȃ�,			Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 0,_T("�_�u���N���b�N"),		F_SELECTWORD,	F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD }, //Feb. 19, 2001 JEPRO Alt�ƉE�N���b�N�̑g�����͌����Ȃ��̂ŉE�N���b�N���j���[�̃L�[���蓖�Ă��͂�����
		{ 0,_T("�E�N���b�N"),			F_MENU_RBUTTON,	F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_0,					F_0,				F_0,					F_0 },
		{ 0,_T("���N���b�N"),			F_AUTOSCROLL,	F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, // novice 2004/10/11 �}�E�X���{�^���Ή�
		{ 0,_T("���T�C�h�N���b�N"),		F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, // novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
		{ 0,_T("�E�T�C�h�N���b�N"),		F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0,_T("�g���v���N���b�N"),		F_SELECTLINE,	F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE },
		{ 0,_T("�N�A�h���v���N���b�N"),	F_SELECTALL,	F_SELECTALL,		F_SELECTALL,			F_SELECTALL,		F_SELECTALL,			F_SELECTALL,		F_SELECTALL,			F_SELECTALL },

		/* �t�@���N�V�����L�[ */
		//keycode,	keyname,			�Ȃ�,			Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ VK_F1,	_T("F1" ),			F_EXTHTMLHELP,	F_MENU_ALLFUNC,		F_EXTHELP1,				F_ABOUT,			F_HELP_CONTENTS,		F_HELP_SEARCH,		F_0,					F_0 },
		{ VK_F2,	_T("F2" ),			F_BOOKMARK_NEXT,F_BOOKMARK_PREV,	F_BOOKMARK_SET,			F_BOOKMARK_RESET,	F_BOOKMARK_VIEW,		F_0,				F_0,					F_0 },
		{ VK_F3,	_T("F3" ),			F_SEARCH_NEXT,	F_SEARCH_PREV,		F_SEARCH_CLEARMARK,		F_JUMP_SRCHSTARTPOS,F_0,					F_0,				F_0,					F_0 },
		{ VK_F4,	_T("F4" ),			F_SPLIT_V,		F_SPLIT_H,			F_0,					F_FILECLOSE_OPEN,	F_0,					F_EXITALLEDITORS,	F_EXITALL,				F_0 },
		{ VK_F5,	_T("F5" ),			F_REDRAW,		F_0,				F_EXECMD_DIALOG,		F_0,				F_UUDECODE,				F_0,				F_TABTOSPACE,			F_SPACETOTAB },
		{ VK_F6,	_T("F6" ),			F_BEGIN_SEL,	F_BEGIN_BOX,		F_TOLOWER,				F_0,				F_BASE64DECODE,			F_0,				F_0,					F_0 },
		{ VK_F7,	_T("F7" ),			F_CUT,			F_0,				F_TOUPPER,				F_0,				F_CODECNV_UTF72SJIS,	F_CODECNV_SJIS2UTF7,F_FILE_REOPEN_UTF7,		F_0 },
		{ VK_F8,	_T("F8" ),			F_COPY,			F_COPY_CRLF,		F_TOHANKAKU,			F_0,				F_CODECNV_UTF82SJIS,	F_CODECNV_SJIS2UTF8,F_FILE_REOPEN_UTF8,		F_0 },
		{ VK_F9,	_T("F9" ),			F_PASTE,		F_PASTEBOX,			F_TOZENKAKUKATA,		F_0,				F_CODECNV_UNICODE2SJIS,	F_0,				F_FILE_REOPEN_UNICODE,	F_0 },
		{ VK_F10,	_T("F10"),			_SQL_RUN,		F_DUPLICATELINE,	F_TOZENKAKUHIRA,		F_0,				F_CODECNV_EUC2SJIS,		F_CODECNV_SJIS2EUC,	F_FILE_REOPEN_EUC,		F_0 },
		{ VK_F11,	_T("F11"),			F_OUTLINE,		F_ACTIVATE_SQLPLUS,	F_HANKATATOZENKATA,		F_0,				F_CODECNV_EMAIL,		F_CODECNV_SJIS2JIS,	F_FILE_REOPEN_JIS,		F_0 },
		{ VK_F12,	_T("F12"),			F_TAGJUMP,		F_TAGJUMPBACK,		F_HANKATATOZENHIRA,		F_0,				F_CODECNV_AUTO2SJIS,	F_0,				F_FILE_REOPEN_SJIS,		F_0 },
		{ VK_F13,	_T("F13"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F14,	_T("F14"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F15,	_T("F15"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F16,	_T("F16"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F17,	_T("F17"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F18,	_T("F18"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F19,	_T("F19"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F20,	_T("F20"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F21,	_T("F21"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F22,	_T("F22"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F23,	_T("F23"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F24,	_T("F24"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },

		/* ����L�[ */
		//keycode,	keyname,			�Ȃ�,			Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ VK_TAB,	_T("Tab"),			F_INDENT_TAB,	F_UNINDENT_TAB,		F_NEXTWINDOW,			F_PREVWINDOW,		F_0,					F_0,				F_0,					F_0 },
		{ VK_RETURN,_T("Enter"),		F_0,			F_0,				F_COMPARE,				F_0,				F_PROPERTY_FILE,		F_0,				F_0,					F_0 },
		{ VK_ESCAPE,_T("Esc"),			F_CANCEL_MODE,	F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_BACK,	_T("BkSp"),			F_DELETE_BACK,	F_0,				F_WordDeleteToStart,	F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_INSERT,_T("Ins"),			F_CHGMOD_INS,	F_PASTE,			F_COPY,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_DELETE,_T("Del"),			F_DELETE,		F_CUT,				F_WordDeleteToEnd,		F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_HOME,	_T("Home"),			F_GOLINETOP,	F_GOLINETOP_SEL,	F_GOFILETOP,			F_GOFILETOP_SEL,	F_0,					F_0,				F_0,					F_0 },
		{ VK_END,	_T("End(Help)"),	F_GOLINEEND,	F_GOLINEEND_SEL,	F_GOFILEEND,			F_GOFILEEND_SEL,	F_0,					F_0,				F_0,					F_0 },
		{ VK_LEFT,	_T("��"),			F_LEFT,			F_LEFT_SEL,			F_WORDLEFT,				F_WORDLEFT_SEL,		F_BEGIN_BOX,			F_0,				F_0,					F_0 },
		{ VK_UP,	_T("��"),			F_UP,			F_UP_SEL,			F_WndScrollDown,		F_UP2_SEL,			F_BEGIN_BOX,			F_0,				F_0,					F_MAXIMIZE_V },
		{ VK_RIGHT,	_T("��"),			F_RIGHT,		F_RIGHT_SEL,		F_WORDRIGHT,			F_WORDRIGHT_SEL,	F_BEGIN_BOX,			F_0,				F_0,					F_MAXIMIZE_H },
		{ VK_DOWN,	_T("��"),			F_DOWN,			F_DOWN_SEL,			F_WndScrollUp,			F_DOWN2_SEL,		F_BEGIN_BOX,			F_0,				F_MENU_RBUTTON,			F_MINIMIZE_ALL },
		{ VK_NEXT,	_T("PgDn(RollUp)"),	F_1PageDown,	F_1PageDown_Sel,	F_HalfPageDown,			F_HalfPageDown_Sel,	F_0,					F_0,				F_0,					F_0 },
		{ VK_PRIOR,	_T("PgUp(RollDn)"),	F_1PageUp,		F_1PageUp_Sel,		F_HalfPageUp,			F_HalfPageUp_Sel,	F_0,					F_0,				F_0,					F_0 },
		{ VK_SPACE,	_T("Space"),		F_INDENT_SPACE,	F_UNINDENT_SPACE,	F_HOKAN,				F_0,				F_0,					F_0,				F_0,					F_0 },

		/* ���� */
		//keycode,	keyname,			�Ȃ�,			Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ '0',		_T("0"),			F_0,			F_0,				F_0,					F_0,				F_CUSTMENU_10,			F_CUSTMENU_20,		F_0,					F_0 },
		{ '1',		_T("1"),			F_0,			F_0,				F_SHOWTOOLBAR,			F_CUSTMENU_21,		F_CUSTMENU_1,			F_CUSTMENU_11,		F_0,					F_0 },
		{ '2',		_T("2"),			F_0,			F_0,				F_SHOWFUNCKEY,			F_CUSTMENU_22,		F_CUSTMENU_2,			F_CUSTMENU_12,		F_0,					F_0 },
		{ '3',		_T("3"),			F_0,			F_0,				F_SHOWSTATUSBAR,		F_CUSTMENU_23,		F_CUSTMENU_3,			F_CUSTMENU_13,		F_0,					F_0 },
		{ '4',		_T("4"),			F_0,			F_0,				F_TYPE_LIST,			F_CUSTMENU_24,		F_CUSTMENU_4,			F_CUSTMENU_14,		F_0,					F_0 },
		{ '5',		_T("5"),			F_0,			F_0,				F_OPTION_TYPE,			F_0,				F_CUSTMENU_5,			F_CUSTMENU_15,		F_0,					F_0 },
		{ '6',		_T("6"),			F_0,			F_0,				F_OPTION,				F_0,				F_CUSTMENU_6,			F_CUSTMENU_16,		F_0,					F_0 },
		{ '7',		_T("7"),			F_0,			F_0,				F_FONT,					F_0,				F_CUSTMENU_7,			F_CUSTMENU_17,		F_0,					F_0 },
		{ '8',		_T("8"),			F_0,			F_0,				F_0,					F_0,				F_CUSTMENU_8,			F_CUSTMENU_18,		F_0,					F_0 },
		{ '9',		_T("9"),			F_0,			F_0,				F_0,					F_0,				F_CUSTMENU_9,			F_CUSTMENU_19,		F_0,					F_0 },

		/* �A���t�@�x�b�g */
		//keycode,	keyname,			�Ȃ�,			Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 'A',		_T("A"),			F_0,			F_0,				F_SELECTALL,			F_0,				F_SORT_ASC,				F_0,				F_0,					F_0 },
		{ 'B',		_T("B"),			F_0,			F_0,				F_BROWSE,				F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'C',		_T("C"),			F_0,			F_0,				F_COPY,					F_OPEN_HfromtoC,	F_0,					F_0,				F_0,					F_0 },
		{ 'D',		_T("D"),			F_0,			F_0,				F_WordCut,				F_WordDelete,		F_SORT_DESC,			F_0,				F_0,					F_0 },
		{ 'E',		_T("E"),			F_0,			F_0,				F_CUT_LINE,				F_DELETE_LINE,		F_0,					F_0,				F_CASCADE,				F_0 },
		{ 'F',		_T("F"),			F_0,			F_0,				F_SEARCH_DIALOG,		F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'G',		_T("G"),			F_0,			F_0,				F_GREP_DIALOG,			F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'H',		_T("H"),			F_0,			F_0,				F_CURLINECENTER,		F_OPEN_HfromtoC,	F_0,					F_0,				F_TILE_V,				F_0 },
		{ 'I',		_T("I"),			F_0,			F_0,				F_DUPLICATELINE,		F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'J',		_T("J"),			F_0,			F_0,				F_JUMP_DIALOG,			F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'K',		_T("K"),			F_0,			F_0,				F_LineCutToEnd,			F_LineDeleteToEnd,	F_0,					F_0,				F_0,					F_0 },
		{ 'L',		_T("L"),			F_0,			F_0,				F_LOADKEYMACRO,			F_EXECKEYMACRO,		F_LTRIM,				F_0,				F_TOLOWER,				F_TOUPPER },
		{ 'M',		_T("M"),			F_0,			F_0,				F_SAVEKEYMACRO,			F_RECKEYMACRO,		F_MERGE,				F_0,				F_0,					F_0 },
		{ 'N',		_T("N"),			F_0,			F_0,				F_FILENEW,				F_0,				F_JUMPHIST_NEXT,		F_0,				F_0,					F_0 },
		{ 'O',		_T("O"),			F_0,			F_0,				F_FILEOPEN,				F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'P',		_T("P"),			F_0,			F_0,				F_PRINT,				F_PRINT_PREVIEW,	F_JUMPHIST_PREV,		F_0,				F_PRINT_PAGESETUP,		F_0 },
		{ 'Q',		_T("Q"),			F_0,			F_0,				F_CREATEKEYBINDLIST,	F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'R',		_T("R"),			F_0,			F_0,				F_REPLACE_DIALOG,		F_0,				F_RTRIM,				F_0,				F_0,					F_0 },
		{ 'S',		_T("S"),			F_0,			F_0,				F_FILESAVE,				F_FILESAVEAS_DIALOG,F_0,					F_0,				F_TMPWRAPSETTING,		F_0 },
		{ 'T',		_T("T"),			F_0,			F_0,				F_TAGJUMP,				F_TAGJUMPBACK,		F_0,					F_0,				F_TILE_H,				F_0 },
		{ 'U',		_T("U"),			F_0,			F_0,				F_LineCutToStart,		F_LineDeleteToStart,F_0,					F_0,				F_WRAPWINDOWWIDTH,		F_0 },
		{ 'V',		_T("V"),			F_0,			F_0,				F_PASTE,				F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'W',		_T("W"),			F_0,			F_0,				F_SELECTWORD,			F_0,				F_0,					F_0,				F_TMPWRAPWINDOW,		F_0 },
		{ 'X',		_T("X"),			F_0,			F_0,				F_CUT,					F_0,				F_0,					F_0,				F_TMPWRAPNOWRAP,		F_0 },
		{ 'Y',		_T("Y"),			F_0,			F_0,				F_REDO,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'Z',		_T("Z"),			F_0,			F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 },

		/* �L�� */
		//keycode,	keyname,			�Ȃ�,			Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 0x00bd,	_T("-"),			F_0,			F_0,				F_COPYFNAME,			F_SPLIT_V,			F_0,					F_0,				F_0,					F_0 },
		{ 0x00de,	_T("^(�p��')"),		F_0,			F_0,				F_COPYTAG,				F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00dc,	_T("\\"),			F_0,			F_0,				F_COPYPATH,				F_SPLIT_H,			F_0,					F_0,				F_0,					F_0 },
		{ 0x00c0,	_T("@(�p��`)"),		F_0,			F_0,				F_COPYLINES,			F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00db,	_T("["),			F_0,			F_0,				F_BRACKETPAIR,			F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00bb,	_T(";"),			F_0,			F_0,				F_0,					F_SPLIT_VH,			F_INS_DATE,				F_0,				F_0,					F_0 },
		{ 0x00ba,	_T(":"),			F_0,			F_0,				_COPYWITHLINENUM,		F_0,				F_INS_TIME,				F_0,				F_0,					F_0 },
		{ 0x00dd,	_T("]"),			F_0,			F_0,				F_BRACKETPAIR,			F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00bc,	_T(","),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00be,	_T("."),			F_0,			F_0,				F_COPYLINESASPASSAGE,	F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00bf,	_T("/"),			F_0,			F_0,				F_HOKAN,				F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00e2,	_T("_"),			F_0,			F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00df,	_T("_(PC-98)"),		F_0,			F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_APPS,	_T("�A�v���L�["),	F_MENU_RBUTTON,	F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON }
	};
	const int	nKeyDataInitNum = _countof( KeyDataInit );
	const int	KEYNAME_SIZE = _countof( pShareData->m_Common.m_sKeyBind.m_pKeyNameArr ) -1;// �Ō�̂P�v�f�̓_�~�[�p�ɗ\�� 2012.11.25 aroka
	//	From Here 2007.11.04 genta �o�b�t�@�I�[�o�[�����h�~
	if( nKeyDataInitNum > KEYNAME_SIZE ) {
		PleaseReportToAuthor( NULL, _T("�L�[�ݒ萔�ɑ΂���DLLSHARE::m_nKeyNameArr[]�̃T�C�Y���s�����Ă��܂�") );
		return false;
	}
	//	To Here 2007.11.04 genta �o�b�t�@�I�[�o�[�����h�~

	// �}�E�X�R�[�h�̌Œ�Əd���r�� 2012.11.25 aroka
	SetKeyNameArrVal( // �C���f�b�N�X�p�_�~�[�쐬
		pShareData,
		KEYNAME_SIZE,
		0,
		_T(""),
		F_0,		F_0,		F_0,		F_0,
		F_0,		F_0,		F_0,		F_0
	);
	// �C���f�b�N�X�쐬 �d�������ꍇ�͐擪�ɂ�����̂�D��
	for( int ii = 0; ii< 256; ii++ ){
		pShareData->m_Common.m_sKeyBind.m_VKeyToKeyNameArr[ii] = KEYNAME_SIZE;
	}
	for( int i=nKeyDataInitNum-1; i>=0; i-- ){
		pShareData->m_Common.m_sKeyBind.m_VKeyToKeyNameArr[KeyDataInit[i].nKeyCode] = i;
	}

	for( int i = 0; i < nKeyDataInitNum; ++i ){
		SetKeyNameArrVal(
			pShareData,
			i,
			KeyDataInit[i].nKeyCode,
			KeyDataInit[i].pszKeyName,
			KeyDataInit[i].nFuncCode_0,
			KeyDataInit[i].nFuncCode_1,
			KeyDataInit[i].nFuncCode_2,
			KeyDataInit[i].nFuncCode_3,
			KeyDataInit[i].nFuncCode_4,
			KeyDataInit[i].nFuncCode_5,
			KeyDataInit[i].nFuncCode_6,
			KeyDataInit[i].nFuncCode_7
		);
	}
	pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum = nKeyDataInitNum;
	return true;
}





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* KEYDATA�z��Ƀf�[�^���Z�b�g */
static void SetKeyNameArrVal(
	DLLSHAREDATA*	pShareData,
	int				nIdx,
	short			nKeyCode,
	const TCHAR*			pszKeyName,
	EFunctionCode	nFuncCode_0,
	EFunctionCode	nFuncCode_1,
	EFunctionCode	nFuncCode_2,
	EFunctionCode	nFuncCode_3,
	EFunctionCode	nFuncCode_4,
	EFunctionCode	nFuncCode_5,
	EFunctionCode	nFuncCode_6,
	EFunctionCode	nFuncCode_7
)
{
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nKeyCode = nKeyCode;
	_tcscpy( pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_szKeyName, pszKeyName );
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[0] = nFuncCode_0;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[1] = nFuncCode_1;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[2] = nFuncCode_2;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[3] = nFuncCode_3;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[4] = nFuncCode_4;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[5] = nFuncCode_5;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[6] = nFuncCode_6;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[7] = nFuncCode_7;
}

