/*!	@file
	@brief �C���|�[�g�A�G�N�X�|�[�g�}�l�[�W��

	@author Uchi
	@date 2010/4/22 �V�K�쐬
*/
/*
	Copyright (C) 2010, Uchi

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "CPropTypes.h"
#include "CDataProfile.h"
#include "env/CShareData.h"
#include "dlg/CDlgOpenFile.h"
#include "CImpExpManager.h"

#include "view/CEditView.h"
#include "io/CTextStream.h"
#include "env/CShareData_IO.h"
#include "view/colors/CColorStrategy.h"
#include "typeprop/CDlgTypeAscertain.h"
//#include "types/Ctype.h"
#include "plugin/CPlugin.h"

/*-----------------------------------------------------------------------
�萔
-----------------------------------------------------------------------*/
// ���b�Z�[�W
static const wstring	MSG_NOT_OPEN		= L"�t�@�C�����J���܂���ł����B\n\n";
static const wstring	MSG_NOT_EXPORT		= L"�G�N�X�|�[�g�o���܂���ł����B\n\n";
static const wstring	MSG_OK_EXPORT		= L"�t�@�C�����G�N�X�|�[�g���܂����B\n\n";
static const wstring	MSG_OK_INPORT		= L"�t�@�C�����C���|�[�g���܂����B\n\n";

static const wchar_t	szSecInfo[]				= L"Info";

// �^�C�v�ʐݒ�
static const wchar_t	WSTR_TYPE_HEAD[]		= L" �^�C�v�ʐݒ� Ver1";

static const wchar_t	szSecTypeEx[]			= L"TypeEx";
static const wchar_t	szSecTypes[]			= L"Types";

static const wchar_t	szKeyKeywordTemp[]				= L"szKeyword[%d]";
static const wchar_t	szKeyKeywordFileTemp[]			= L"szKeywordFile[%d]";
static const wchar_t	szKeyKeywordCaseTemp[]			= L"szKeywordCase[%d]";
static const wchar_t	szKeyPluginOutlineName[]		= L"szPluginOutlineName";
static const wchar_t	szKeyPluginOutlineId[]			= L"szPluginOutlineId";
static const wchar_t	szKeyPluginSmartIndentName[]	= L"szPluginSmartIndentName";
static const wchar_t	szKeyPluginSmartIndentId[]		= L"szPluginSmartIndentId";
static const wchar_t	szKeyVersion[]					= L"szVersion";
static const wchar_t	szKeyStructureVersion[]			= L"vStructureVersion";

// �J���[�i CPropTypes.h����R�s�[���� �j
//static const wchar_t	WSTR_COLORDATA_HEAD2[]	=  L" �e�L�X�g�G�f�B�^�F�ݒ� Ver2";
//static const wchar_t	WSTR_COLORDATA_HEAD21[]	=  L" �e�L�X�g�G�f�B�^�F�ݒ� Ver2.1";	//Nov. 2, 2000 JEPRO �ύX [��]. 0.3.9.0:ur3��10�ȍ~�A�ݒ荀�ڂ̔ԍ������ւ�������
static const wchar_t	WSTR_COLORDATA_HEAD3[]	=  L" �e�L�X�g�G�f�B�^�F�ݒ� Ver3";		//Jan. 15, 2001 Stonee  �F�ݒ�Ver3�h���t�g(�ݒ�t�@�C���̃L�[��A�ԁ��������)	//Feb. 11, 2001 JEPRO �L���ɂ���
//static const wchar_t	WSTR_COLORDATA_HEAD4[]	=  L" �e�L�X�g�G�f�B�^�F�ݒ� Ver4";		//2007.10.02 kobake UNICODE���ɍۂ��ăJ���[�t�@�C���d�l���ύX
static const wchar_t	szSecColor[]			=  L"SakuraColor";

// ���K�\���L�[���[�h
static const wchar_t	WSTR_REGEXKW_HEAD[]		= L"// ���K�\���L�[���[�h Ver1\n";

// �L�[���[�h�w���v
static const wchar_t	WSTR_KEYHELP_HEAD[]		= L"// �L�[���[�h�����ݒ� Ver1\n";

// �L�[���蓖��
static const wchar_t	WSTR_KEYBIND_HEAD[]		= L"SakuraKeyBind_Ver3";	//2007.10.05 kobake �t�@�C���`����ini�`���ɕύX
static const wchar_t	WSTR_KEYBIND_HEAD2[]	= L"// �e�L�X�g�G�f�B�^�L�[�ݒ� Ver2";	// (���o�[�W����(ANSI��)�j �ǂݍ��݂̂ݑΉ� 2008/5/3 by Uchi

// �J�X�^�����j���[�t�@�C��
// 2007.10.02 kobake UNICODE���ɍۂ��āA�J�X�^�����j���[�t�@�C���̎d�l��ύX
static       wchar_t	WSTR_CUSTMENU_HEAD_V2[]	= L"SakuraEditorMenu_Ver2";

// �L�[���[�h��`�t�@�C��
static const wchar_t	WSTR_KEYWORD_HEAD[]		= L" �L�[���[�h��`�t�@�C��\n";
static const wchar_t	WSTR_KEYWORD_CASE[]		= L"// CASE=";
static const wchar_t	WSTR_CASE_TRUE[]		= L"// CASE=True";
static const wchar_t	WSTR_CASE_FALSE[]		= L"// CASE=False";


// Export�t�@�C�����̍쐬
//	  �^�C�v���Ȃǃt�@�C���Ƃ��Ĉ������Ƃ��l���Ă��Ȃ������������
//		2010/4/12 Uchi
static wchar_t* MakeExportFileName(wchar_t* res, const wchar_t* trg, const wchar_t* ext)
{
	wchar_t		conv[_MAX_PATH+1];
	wchar_t*	p;

	auto_strcpy( conv, trg );

	p = conv;
	while ( (p = wcspbrk( p, L"\\:*?\"<>|" )) != NULL ) {
		// �t�@�C�����Ɏg���Ȃ�������_ �̒u��������
		*p++ = _T('_');
	}
	p = conv;
	while ( (p = wcspbrk( p, L"/" )) != NULL ) {
		// �t�@�C�����Ɏg���Ȃ�������_ �̒u��������
		*p++ = _T('�^');
	}
	auto_sprintf_s(res, _MAX_PATH, L"%ls.%ls", conv, ext);

	return res;
}

// �C���|�[�g �t�@�C���w��t��
bool CImpExpManager::ImportUI( HINSTANCE hInstance, HWND hwndDlg )
{
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	CDlgOpenFile	cDlgOpenFile;
	cDlgOpenFile.Create(
		hInstance,
		hwndDlg,
		GetDefaultExtension(),
		GetDllShareData().m_sHistory.m_szIMPORTFOLDER, // �C���|�[�g�p�t�H���_
		std::vector<LPCTSTR>(),
		std::vector<LPCTSTR>()
	);
	TCHAR	szPath[_MAX_PATH + 1];
	auto_strcpy( szPath, (TCHAR*)to_tchar( GetFullPath().c_str() ));
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return false;
	}

	wstring	sPath;
	wstring	sErrMsg;

#if _UNICODE
	sPath = szPath;
#else
	sPath = to_wchar(szPath);
#endif
	// �m�F
	if (!ImportAscertain( hInstance, hwndDlg, sPath, sErrMsg )) {
		if (sErrMsg.length() > 0) {
			ErrorMessage( hwndDlg, _T("%ls"), sErrMsg.c_str() );
		}
		return false;
	}

	// Import Folder�̐ݒ�
	SetImportFolder( szPath );

	// Import
	if (!Import( sPath, sErrMsg )) {
		ErrorMessage( hwndDlg, _T("%ls"), sErrMsg.c_str() );
		return false;
	}

	if (sErrMsg.length() > 0) {
		InfoMessage( hwndDlg, _T("%ls"), sErrMsg.c_str() );
	}

	return true;
}

// �G�N�X�|�[�g �t�@�C���w��t��
bool CImpExpManager::ExportUI( HINSTANCE hInstance, HWND hwndDlg )
{
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	CDlgOpenFile	cDlgOpenFile;
	cDlgOpenFile.Create(
		hInstance,
		hwndDlg,
		GetDefaultExtension(),
		GetDllShareData().m_sHistory.m_szIMPORTFOLDER, // �C���|�[�g�p�t�H���_
		std::vector<LPCTSTR>(),
		std::vector<LPCTSTR>()
	);
	TCHAR			szPath[_MAX_PATH + 1];
	auto_strcpy( szPath, (TCHAR*)to_tchar( m_sOriginName.c_str() ));
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return false;
	}

	// Import Folder�̐ݒ�
	SetImportFolder( szPath );

	// Export
	wstring	sPath;
	wstring	sErrMsg;

#if _UNICODE
	sPath = szPath;
#else
	sPath = to_wchar(szPath);
#endif
	if (!Export( sPath, sErrMsg )) {
		ErrorMessage( hwndDlg, to_tchar( sErrMsg.c_str()) );
		return false;
	}

	if (sErrMsg.length() == 0) {
		sErrMsg = MSG_OK_EXPORT + to_wchar( szPath );
	}
	InfoMessage( hwndDlg, to_tchar( sErrMsg.c_str()) );

	return true;
}

// �C���|�[�g�m�F
bool CImpExpManager::ImportAscertain( HINSTANCE hInstance, HWND hwndDlg, const wstring sFileName, wstring& sErrMsg )
{
	return true;
}
// �C���|�[�g
bool CImpExpManager::Import( const wstring sFileName, wstring& sErrMsg )
{
	return false;
}

// �G�N�X�|�[�g
bool CImpExpManager::Export( const wstring sFileName, wstring& sErrMsg )
{
	return false;
}

// �f�t�H���g�g���q�̎擾
const TCHAR* CImpExpManager::GetDefaultExtension()
{
	return _T("");
}
const wchar_t* CImpExpManager::GetOriginExtension()
{
	return L"";
}

// �t�@�C�����̏����l��ݒ�
void CImpExpManager::SetBaseName(wstring sBase)
{
	wchar_t		wbuff[_MAX_PATH + 1];

	m_sBase = sBase;
	m_sOriginName = MakeExportFileName( wbuff, sBase.c_str(), GetOriginExtension() );
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �^�C�v�ʐݒ�                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g�m�F
bool CImpExpType::ImportAscertain( HINSTANCE hInstance, HWND hwndDlg, const wstring sFileName, wstring& sErrMsg )
{
	TCHAR*		szPath = (TCHAR*)to_tchar( sFileName.c_str() );

	CShareData_IO*	MyIO = new CShareData_IO();

	m_cProfile.SetReadingMode();

	if (!m_cProfile.ReadProfile( szPath )) {
		/* �ݒ�t�@�C�������݂��Ȃ� */
		sErrMsg = MSG_NOT_OPEN + sFileName;
		return false;
	}

	// Check Version
	int		nStructureVersion;
	wchar_t	szKeyVersion[64];
	if (!m_cProfile.IOProfileData( szSecInfo, szKeyStructureVersion, nStructureVersion )) {
		sErrMsg = L"�s���Ȍ`���ł��B\n�C���|�[�g�𒆎~���܂�";
		return false;
	}
	if ((unsigned int)nStructureVersion != GetDllShareData().m_vStructureVersion) {
		auto_strcpy( szKeyVersion, L"?" );
		m_cProfile.IOProfileData( szSecInfo, szKeyVersion, MakeStringBufferW( szKeyVersion ) );
		int nRet = ConfirmMessage( hwndDlg,
			_T("�G�N�X�|�[�g���� %ls(%ls/%d) �ƃo�[�W�������قȂ�܂��B\n\n�C���|�[�g���Ă���낵���ł����H"), 
			_APP_NAME_(LTEXT), szKeyVersion, nStructureVersion );
		if ( IDYES != nRet ) {
			return false;
		}
	}

	// �m�F���F�w��
	CDlgTypeAscertain::SAscertainInfo	sAscertainInfo;
	CDlgTypeAscertain	cDlgTypeAscertain;
	wchar_t wszLabel[1024];
	STypeConfig TmpType;

	// �p�����[�^�̐ݒ�
	sAscertainInfo.sImportFile = szPath;
	List_GetText( m_hwndList, m_nIdx, wszLabel );
	sAscertainInfo.sTypeNameTo = wszLabel;
	wszLabel[0] = L'\0';
	m_cProfile.IOProfileData( szSecTypes, L"szTypeName", MakeStringBufferW( wszLabel ));
	sAscertainInfo.sTypeNameFile = wszLabel;

	// �m�F
	if (!cDlgTypeAscertain.DoModal( hInstance, hwndDlg, &sAscertainInfo )) {
		return false;
	}

	m_nColorType = sAscertainInfo.nColorType;
	m_sColorFile = sAscertainInfo.sColorFile;

	return true;
}

// �C���|�[�g
bool CImpExpType::Import( const wstring sFileName, wstring& sErrMsg )
{
	CShareData_IO*	MyIO = new CShareData_IO();

	wstring	files = L"";
	wstring TmpMsg;
	ColorInfo	colorInfoArr[64];				// �F�ݒ�z��(�o�b�N�A�b�v)
	int		i;

	// �F�̕ύX
	if (m_nColorType >= MAX_TYPES) {
		// �F�ݒ�C���|�[�g
		CImpExpColors	cImpExpColors( colorInfoArr );
		if (cImpExpColors.Import( cImpExpColors.MakeFullPath( m_sColorFile ), TmpMsg)) {
			files += wstring(L"\n") + m_sColorFile;
		}
		else {
			// ���s�������{���R�s�[(���b�Z�[�W�͏o���Ȃ�)
			memcpy( &colorInfoArr, &CDocTypeManager().GetTypeSetting(CTypeConfig(0)).m_ColorInfoArr, sizeof(colorInfoArr) );
			files += wstring(L"\n�~ ") + m_sColorFile;	// ���s
		}
	}
	else if (m_nColorType >= 0 ) {
		// �F�w��(����)
		memcpy( &colorInfoArr, &CDocTypeManager().GetTypeSetting(CTypeConfig(m_nColorType)).m_ColorInfoArr, sizeof(colorInfoArr) );
	}

	// �ǂݍ���
	MyIO->ShareData_IO_Type_One( m_cProfile, m_nIdx, szSecTypes);

	m_Types.m_nIdx = m_nIdx;
	if (m_nIdx == 0) {
		// ��{�̏ꍇ�̖��O�Ɗg���q��������
		_tcscpy( m_Types.m_szTypeName, _T("��{") );
		_tcscpy( m_Types.m_szTypeExts, _T("") );
	}

	// �F�̐ݒ�
	if (m_nColorType >= 0 ) {
		// �F�w�肠��
		for (i = 0; i < _countof(colorInfoArr); i++) {
			bool bDisp = m_Types.m_ColorInfoArr[i].m_bDisp;
			m_Types.m_ColorInfoArr[i] = colorInfoArr[i];
			m_Types.m_ColorInfoArr[i].m_bDisp = bDisp;		// �\���t���O�̓t�@�C���̂��̂��g�p����
		}
	}

	// ���ʐݒ�Ƃ̘A����
	wchar_t	szKeyName[64];
	wchar_t	szKeyData[1024];
	int		nIdx;
	wchar_t	szFileName[_MAX_PATH+1];
	bool	bCase;
	wstring	sErrMag;

	// �����L�[���[�h
	CKeyWordSetMgr&	cKeyWordSetMgr = m_pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr;
	for (i=0; i < 10; i++) {
		//types.m_nKeyWordSetIdx[i] = -1;
		auto_sprintf( szKeyName, szKeyKeywordTemp, i+1 );
		if (m_cProfile.IOProfileData( szSecTypeEx, szKeyName, MakeStringBufferW( szKeyData ))) {
			nIdx = cKeyWordSetMgr.SearchKeyWordSet( szKeyData );
			if (nIdx < 0) {
				// �G���g���쐬
				cKeyWordSetMgr.AddKeyWordSet( szKeyData, false );
				nIdx = cKeyWordSetMgr.SearchKeyWordSet( szKeyData );
			}
			if (nIdx >= 0) {
				auto_sprintf( szKeyName, szKeyKeywordCaseTemp, i+1 );
				bCase = false;		// �啶����������ʂ��Ȃ� (Defaule)
				m_cProfile.IOProfileData( szSecTypeEx, szKeyName, bCase );

				// �L�[���[�h��`�t�@�C������
				CImpExpKeyWord	cImpExpKeyWord( m_pShareData->m_Common, nIdx, bCase );

				auto_sprintf( szKeyName, szKeyKeywordFileTemp, i+1 );
				if (m_cProfile.IOProfileData( szSecTypeEx, szKeyName, MakeStringBufferW( szFileName ))
				 && cImpExpKeyWord.Import( cImpExpKeyWord.MakeFullPath( szFileName ), TmpMsg )) {
					files += wstring(L"\n") + szFileName;
				}
				else {
					files += wstring(L"\n�~ ") + szFileName;	// ���s
				}
			}
			m_Types.m_nKeyWordSetIdx[i] = nIdx;
		}
	}

	// Plugin
	//  �A�E�g���C����͕��@
	CommonSetting_Plugin& plugin = GetDllShareData().m_Common.m_sPlugin;
	if (m_cProfile.IOProfileData( szSecTypeEx, szKeyPluginOutlineId, MakeStringBufferW( szKeyData ))) {
		nIdx = -1;
		for (i = 0; i < MAX_PLUGIN; i++) {
			if (auto_strcmp(szKeyData, plugin.m_PluginTable[i].m_szId) == 0) {
				nIdx = i;
				break;
			}
		}
		if (nIdx > 0) {
			m_Types.m_eDefaultOutline = EOutlineType( GetPluginFunctionCode( nIdx, 0 ));
		}
	}
	//  �X�}�[�g�C���f���g
	if (m_cProfile.IOProfileData( szSecTypeEx, szKeyPluginSmartIndentId, MakeStringBufferW( szKeyData ))) {
		nIdx = -1;
		for (i = 0; i < MAX_PLUGIN; i++) {
			if (auto_strcmp(szKeyData, plugin.m_PluginTable[i].m_szId) == 0) {
				nIdx = i;
				break;
			}
		}
		if (nIdx > 0) {
			m_Types.m_eSmartIndent = ESmartIndentType( GetPluginFunctionCode( nIdx, 0 ));
		}
	}

	sErrMsg =  MSG_OK_INPORT + sFileName + files;

	return true;
}


// �G�N�X�|�[�g
bool CImpExpType::Export( const wstring sFileName, wstring& sErrMsg )
{
	CShareData_IO*	MyIO = new CShareData_IO();
	CDataProfile	cProfile;

	cProfile.SetWritingMode();

	MyIO->ShareData_IO_Type_One( cProfile , m_nIdx, szSecTypes);

	// ���ʐݒ�Ƃ̘A����
	int		i;
	wchar_t	szKeyName[64];
	wchar_t buff[64];
	wchar_t	szFileName[_MAX_PATH+1];
	bool	bCase;
	wstring	files = L"";
	wstring	sTmpMsg;
	int		nIdx;

	// �����L�[���[�h
	CKeyWordSetMgr&	cKeyWordSetMgr = m_pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr;
	for (i=0; i < 10; i++) {
		if (m_Types.m_nKeyWordSetIdx[i] >= 0) {
			nIdx = m_Types.m_nKeyWordSetIdx[i];
			auto_sprintf( szKeyName, szKeyKeywordTemp, i+1 );
			auto_strcpy( buff, cKeyWordSetMgr.GetTypeName( nIdx ));
			cProfile.IOProfileData( szSecTypeEx, szKeyName, MakeStringBufferW( buff ));

			// �啶�����������
			bCase = m_pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordCase( nIdx );

			// �L�[���[�h��`�t�@�C���o��
			CImpExpKeyWord	cImpExpKeyWord( m_pShareData->m_Common, m_Types.m_nKeyWordSetIdx[i], bCase );
			cImpExpKeyWord.SetBaseName( m_pShareData->m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( nIdx ));

			if ( cImpExpKeyWord.Export( cImpExpKeyWord.GetFullPath(), sTmpMsg ) ) {
				auto_strcpy( szFileName, cImpExpKeyWord.GetFileName().c_str());
				auto_sprintf( szKeyName, szKeyKeywordFileTemp, i+1 );
				if (cProfile.IOProfileData( szSecTypeEx, szKeyName, MakeStringBufferW( szFileName ))) {
					files += wstring( L"\n" ) + cImpExpKeyWord.GetFileName();
				}
			}

			auto_sprintf( szKeyName, szKeyKeywordCaseTemp, i+1 );
			cProfile.IOProfileData( szSecTypeEx, szKeyName, bCase );
		}
	}

	// Plugin
	//  �A�E�g���C����͕��@
	CommonSetting_Plugin& plugin = GetDllShareData().m_Common.m_sPlugin;
	int		nPIdx;
	if ((nPIdx = GetPluginId( EFunctionCode( m_Types.m_eDefaultOutline ))) >= 0) {
		cProfile.IOProfileData( szSecTypeEx, szKeyPluginOutlineName, MakeStringBufferW(plugin.m_PluginTable[nPIdx].m_szName));
		cProfile.IOProfileData( szSecTypeEx, szKeyPluginOutlineId,   MakeStringBufferW(plugin.m_PluginTable[nPIdx].m_szId));
	}
	//  �X�}�[�g�C���f���g
	if ((nPIdx = GetPluginId( EFunctionCode( m_Types.m_eSmartIndent ))) >= 0) {
		cProfile.IOProfileData( szSecTypeEx, szKeyPluginSmartIndentName, MakeStringBufferW(plugin.m_PluginTable[nPIdx].m_szName));
		cProfile.IOProfileData( szSecTypeEx, szKeyPluginSmartIndentId,   MakeStringBufferW(plugin.m_PluginTable[nPIdx].m_szId));
	}

	// Version
	DLLSHAREDATA* pShare = &GetDllShareData();
	int		nStructureVersion;
	wchar_t	wbuff[_MAX_PATH + 1];
	auto_sprintf( wbuff, L"%d.%d.%d.%d", 
				HIWORD( pShare->m_sVersion.m_dwProductVersionMS ),
				LOWORD( pShare->m_sVersion.m_dwProductVersionMS ),
				HIWORD( pShare->m_sVersion.m_dwProductVersionLS ),
				LOWORD( pShare->m_sVersion.m_dwProductVersionLS ) );
	cProfile.IOProfileData( szSecInfo, szKeyVersion, MakeStringBufferW(wbuff) );
	nStructureVersion = int(pShare->m_vStructureVersion);
	cProfile.IOProfileData( szSecInfo, szKeyStructureVersion, nStructureVersion );

	// ��������
	if (!cProfile.WriteProfile( to_tchar(sFileName.c_str()), WSTR_TYPE_HEAD )) {
		sErrMsg = MSG_NOT_EXPORT + sFileName;
		return false;
	}

	sErrMsg =  MSG_OK_EXPORT + sFileName + files;

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �J���[                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool CImpExpColors::Import( const wstring sFileName, wstring& sErrMsg )
{
	TCHAR*		szPath = (TCHAR*)to_tchar( sFileName.c_str() );

	// �J���邩
	CTextInputStream in( szPath );
	if (!in) {
		sErrMsg = MSG_NOT_OPEN + sFileName;
		return false;
	}

	/* �t�@�C���擪 */
	//�w�b�_�ǎ�
	wstring szHeader = in.ReadLineW();
	if(szHeader.length()>=2) {
		//�R�����g�𔲂�
		szHeader = &szHeader.c_str()[ szHeader.c_str()[0] == _T(';') ? 1 : 2];
	}
	//��r
	if (szHeader != WSTR_COLORDATA_HEAD3) {
		in.Close();
		sErrMsg = wstring(L"�F�ݒ�t�@�C���̌`�����Ⴂ�܂��B�Â��`���̓T�|�[�g����Ȃ��Ȃ�܂����B\n")	// ���o�[�W�����̐����̍폜 2010/4/22 Uchi
			+ sFileName;
		return false;
	}
	in.Close();

	CDataProfile	cProfile;
	cProfile.SetReadingMode();

	/* �F�ݒ�Ver3 */
	if( !cProfile.ReadProfile( szPath ) ){
		return false;
	}

	/* �F�ݒ� I/O */
	CShareData_IO::IO_ColorSet( &cProfile, szSecColor, m_ColorInfoArr );

	return true;
}

// �G�N�X�|�[�g
bool CImpExpColors::Export( const wstring sFileName, wstring& sErrMsg )
{
	/* �F�ݒ� I/O */
	CDataProfile	cProfile;
	cProfile.SetWritingMode();
	CShareData_IO::IO_ColorSet( &cProfile, szSecColor, m_ColorInfoArr );
	if (!cProfile.WriteProfile( to_tchar( sFileName.c_str() ), WSTR_COLORDATA_HEAD3 )) { //Jan. 15, 2001 Stonee
		sErrMsg = MSG_NOT_EXPORT + sFileName;
		return false;
	}

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ���K�\���L�[���[�h                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool CImpExpRegex::Import( const wstring sFileName, wstring& sErrMsg )
{
	CTextInputStream	in( to_tchar( sFileName.c_str() ) );
	if (!in) {
		sErrMsg = MSG_NOT_OPEN + sFileName;
		return false;
	}

	CPropRegex			cPropRegex;		// RegexKakomiCheck�g�p�̂���
	RegexKeywordInfo	pRegexKey[MAX_REGEX_KEYWORD];
	TCHAR				buff[1024];
	int					i, j, k;
	j = 0;
	while(in)
	{
		//1�s�ǂݍ���
		wstring line=in.ReadLineW();
		_wcstotcs(buff,line.c_str(),_countof(buff));

		if(j >= MAX_REGEX_KEYWORD) break;

		//RxKey[999]=ColorName,RegexKeyword
		if( auto_strlen(buff) < 12 ) continue;
		if( auto_memcmp(buff, _T("RxKey["), 6) != 0 ) continue;
		if( auto_memcmp(&buff[9], _T("]="), 2) != 0 ) continue;
		TCHAR	*p;
		p = auto_strstr(&buff[11], _T(","));
		if( p )
		{
			*p = _T('\0');
			p++;
			if( p[0] && cPropRegex.RegexKakomiCheck(to_wchar(p)) )	//�݂͂�����
			{
				//�F�w�薼�ɑΉ�����ԍ���T��
				k = GetColorIndexByName( &buff[11] );	//@@@ 2002.04.30
				if( k != -1 )	/* 3�����J���[������C���f�b�N�X�ԍ��ɕϊ� */
				{
					pRegexKey[j].m_nColorIndex = k;
					_tcstowcs(pRegexKey[j].m_szKeyword, p, _countof(pRegexKey[j].m_szKeyword));
					j++;
				}
				else
				{	/* ���{�ꖼ����C���f�b�N�X�ԍ��ɕϊ����� */
					for(k = 0; k < COLORIDX_LAST; k++)
					{
						if( auto_strcmp(m_Types.m_ColorInfoArr[k].m_szName, &buff[11]) == 0 )
						{
							pRegexKey[j].m_nColorIndex = k;
							_tcstowcs(pRegexKey[j].m_szKeyword, p, _countof(pRegexKey[j].m_szKeyword));
							j++;
							break;
						}
					}
				}
			}
		}
	}

	in.Close();

	ListView_DeleteAllItems( m_hwndList );  /* ���X�g����ɂ��� */
	for(i = 0; i < j; i++)
	{
		LV_ITEM	lvi;
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = const_cast<TCHAR*>(to_tchar(pRegexKey[i].m_szKeyword));
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = 0;
		ListView_InsertItem( m_hwndList, &lvi );

		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = m_Types.m_ColorInfoArr[pRegexKey[i].m_nColorIndex].m_szName;
		ListView_SetItem( m_hwndList, &lvi );
	}
	ListView_SetItemState( m_hwndList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	return true;
}

// �G�N�X�|�[�g
bool CImpExpRegex::Export( const wstring sFileName, wstring& sErrMsg )
{
	CTextOutputStream out( to_tchar( sFileName.c_str() ) );
	if(!out){
		sErrMsg = MSG_NOT_OPEN + sFileName;
		return false;
	}

	out.WriteF( WSTR_REGEXKW_HEAD );

	int j = ListView_GetItemCount(m_hwndList);
	for (int i = 0; i < j; i++)
	{
		TCHAR	szKeyWord[256];
		auto_memset(szKeyWord, 0, _countof(szKeyWord));
		ListView_GetItemText(m_hwndList, i, 0, szKeyWord, _countof(szKeyWord));

		TCHAR	szColorIndex[256];
		auto_memset(szColorIndex, 0, _countof(szColorIndex));
		ListView_GetItemText(m_hwndList, i, 1, szColorIndex, _countof(szColorIndex));

		const TCHAR* p = szColorIndex;
		for (int k = 0; k < COLORIDX_LAST; k++)
		{
			if ( _tcscmp( m_Types.m_ColorInfoArr[k].m_szName, szColorIndex ) == 0 )
			{
				p = GetColorNameByIndex(k);
				break;
			}
		}
		out.WriteF( L"RxKey[%03d]=%ts,%ts\n", i, p, szKeyWord);
	}

	out.Close();

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���[�h�w���v                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool CImpExpKeyHelp::Import( const wstring sFileName, wstring& sErrMsg )
{
	wchar_t			msgBuff[_MAX_PATH + 1];

	CTextInputStream in( to_tchar( sFileName.c_str() ));
	if (!in) {
		sErrMsg = MSG_NOT_OPEN + sFileName;
		return false;
	}

	/* LIST���̃f�[�^�S�폜 */
	ListView_DeleteAllItems(m_hwndList);  /* ���X�g����ɂ��� */
//	GetData_KeyHelp(m_hwndDlg);	// ???

	/* �f�[�^�擾 */
	int invalid_record = 0; // �s���ȍs
	int i=0;
	while (in && i<MAX_KEYHELP_FILE) {
		wstring buff=in.ReadLineW();

		// 2007.02.03 genta �R�����g�݂����ȍs�͖ق��ăX�L�b�v
		// 2007.10.08 kobake ��s���X�L�b�v
		if( buff[0] == LTEXT('\0') ||
			buff[0] == LTEXT('\n') ||
			buff[0] == LTEXT('#') ||
			buff[0] == LTEXT(';') ||
			( buff[0] == LTEXT('/') && buff[1] == LTEXT('/') )){
				//	2007.02.03 genta �������p��
				continue;
		}

		//KDct[99]=ON/OFF,DictAbout,KeyHelpPath
		if( buff.length() < 10 ||
			auto_memcmp(buff.c_str(), LTEXT("KDct["), 5) != 0 ||
			auto_memcmp(&buff[7], LTEXT("]="), 2) != 0 ||
			0 ){
			//	2007.02.03 genta �������p��
			++invalid_record;
			continue;
		}

		WCHAR *p1, *p2, *p3;
		p1 = &buff[9];
		p3 = p1;					//���ʊm�F�p�ɏ�����
		if( NULL != (p2=wcsstr(p1,LTEXT(","))) ){
			*p2 = LTEXT('\0');
			p2 += 1;				//�J���}�̎����A���̗v�f
			if( NULL != (p3=wcsstr(p2,LTEXT(","))) ){
				*p3 = LTEXT('\0');
				p3 += 1;			//�J���}�̎����A���̗v�f
			}
		}/* ���ʂ̊m�F */
		if( (p3==NULL) ||			//�J���}��1����Ȃ�
			(p3==p1) ||				//�J���}��2����Ȃ�
			//	2007.02.03 genta �t�@�C�����ɃJ���}�����邩������Ȃ�
			0 //(NULL!=wcsstr(p3,","))	//�J���}����������
		){
			//	2007.02.03 genta �������p��
			++invalid_record;
			continue;
		}
		/* value�̃`�F�b�N */
		//ON/OFF
		//	2007.02.03 genta 1�łȂ����1�ɂ���
		unsigned int b_enable_flag = (unsigned int)_wtoi(p1);
		if( b_enable_flag > 1){
			b_enable_flag = 1;
		}
		//Path
		FILE* fp2;
		if( (fp2=_tfopen_absini(to_tchar(p3),_T("r"))) == NULL ){	// 2007.02.03 genta ���΃p�X��sakura.exe��ŊJ��	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
			// 2007.02.03 genta ������������Ȃ��ꍇ�̑[�u�D�x�����o������荞��
			p2 = L"�y�����t�@�C����������܂���z";
			b_enable_flag = 0;
		}
		else
			fclose(fp2);

		//About
		if (wcslen(p2) > DICT_ABOUT_LEN) {
			auto_sprintf( msgBuff, L"�����̐�����%d�����ȓ��ɂ��Ă��������B", DICT_ABOUT_LEN );
			sErrMsg = msgBuff;
			++invalid_record;
			continue;
		}

		//�ǂ������Ȃ�
		m_Types.m_KeyHelpArr[i].m_bUse = (b_enable_flag!=0);	// 2007.02.03 genta
		_tcscpy(m_Types.m_KeyHelpArr[i].m_szAbout, to_tchar(p2));
		_tcscpy(m_Types.m_KeyHelpArr[i].m_szPath,  to_tchar(p3));
		i++;
	}
	in.Close();

	/*�f�[�^�̃Z�b�g*/
//	SetData_KeyHelp(m_hwndDlg);	// ???
	// 2007.02.03 genta ���s������x������
	if( invalid_record > 0 ){
		auto_sprintf( msgBuff, L"�ꕔ�̃f�[�^���ǂݍ��߂܂���ł���\n�s���ȍs��: %d", invalid_record );
		sErrMsg = msgBuff;
	}

	return true;
}

// �G�N�X�|�[�g
bool CImpExpKeyHelp::Export( const wstring sFileName, wstring& sErrMsg )
{
	CTextOutputStream out( to_tchar( sFileName.c_str() ) );
	if (!out) {
		sErrMsg = MSG_NOT_OPEN + sFileName;
		return false;
	}

	out.WriteF( WSTR_KEYHELP_HEAD );

	int j = ListView_GetItemCount( m_hwndList );

	for (int i = 0; i < j; i++) {
		out.WriteF(
			L"KDct[%02d]=%d,%ts,%ts\n",
			i,
			m_Types.m_KeyHelpArr[i].m_bUse?1:0,
			m_Types.m_KeyHelpArr[i].m_szAbout,
			m_Types.m_KeyHelpArr[i].m_szPath.c_str()
		);
	}
	out.Close();

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���蓖��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool CImpExpKeybind::Import( const wstring sFileName, wstring& sErrMsg )
{
	TCHAR*		szPath = (TCHAR*)to_tchar( sFileName.c_str() );
	KEYDATA		pKeyNameArr[100];				/* �L�[���蓖�ĕ\ */

	//�I�[�v��
	CDataProfile in;
	in.SetReadingMode();
	if (!in.ReadProfile( to_tchar( sFileName.c_str() ))) {
		sErrMsg = MSG_NOT_OPEN + sFileName;
		return false;
	}

	//�o�[�W�����m�F
	bool	bVer3;			// �V�o�[�W�����̃t�@�C��
	bool	bVer2;
	WCHAR szHeader[256];
	bVer3 = true;
	bVer2 = false;
	in.IOProfileData(szSecInfo, L"KEYBIND_VERSION", MakeStringBufferW(szHeader));
	if(wcscmp(szHeader,WSTR_KEYBIND_HEAD)!=0)	bVer3=false;

	int	nKeyNameArrNum;			// �L�[���蓖�ĕ\�̗L���f�[�^��
	if ( bVer3 ) {
		//Count�擾 -> nKeyNameArrNum
		in.IOProfileData(szSecInfo, L"KEYBIND_COUNT", nKeyNameArrNum);
		if (nKeyNameArrNum<0 || nKeyNameArrNum>100)	bVer3=false; //�͈̓`�F�b�N

		CShareData_IO::ShareData_IO_KeyBind(in, nKeyNameArrNum, pKeyNameArr, true);	// 2008/5/25 Uchi
	}

	if (!bVer3) {
		// �V�o�[�W�����łȂ�
		CTextInputStream in(szPath);
		if (!in) {
			sErrMsg = MSG_NOT_OPEN + sFileName;
			return false;
		}
		// �w�b�_�`�F�b�N
		wstring	szLine = in.ReadLineW();
		bVer2 = true;
		if ( wcscmp(szLine.c_str(), WSTR_KEYBIND_HEAD2) != 0)	bVer2 = false;
		// �J�E���g�`�F�b�N
		int	i, cnt;
		if ( bVer2 ) {
			int	an;
			szLine = in.ReadLineW();
			cnt = swscanf(szLine.c_str(), L"Count=%d", &an);
			if ( cnt != 1 || an < 0 || an > 100 ) {
				bVer2 = false;
			}
			else {
				nKeyNameArrNum = an;
			}
		}
		if ( bVer2 ) {
			//�e�v�f�擾
			for(i = 0; i < 100; i++) {
				int n, kc, nc;
				//�l -> szData
				wchar_t szData[1024];
				auto_strcpy(szData, in.ReadLineW().c_str());

				//��͊J�n
				cnt = swscanf(szData, L"KeyBind[%03d]=%04x,%n",
												&n,   &kc, &nc);
				if( cnt !=2 && cnt !=3 )	{ bVer2= false; break;}
				if( i != n ) break;
				pKeyNameArr[i].m_nKeyCode = kc;
				wchar_t* p = szData + nc;

				//��ɑ����g�[�N��
				for(int j=0;j<8;j++)
				{
					wchar_t* q=auto_strchr(p,L',');
					if(!q)	{ bVer2= false; break;}
					*q=L'\0';

					//�@�\���𐔒l�ɒu��������B(���l�̋@�\�������邩��)
					//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
					WCHAR	szFuncNameJapanese[256];
					EFunctionCode n = CSMacroMgr::GetFuncInfoByName(G_AppInstance(), p, szFuncNameJapanese);
					if( n == F_INVALID )
					{
						if( WCODE::Is09(*p) )
						{
							n = (EFunctionCode)auto_atol(p);
						}
						else
						{
							n = F_DEFAULT;
						}
					}
					pKeyNameArr[i].m_nFuncCodeArr[j] = n;
					p = q + 1;
				}

				auto_strcpy(pKeyNameArr[i].m_szKeyName, to_tchar(p));
			}
		}
	}
	if (!bVer3  && !bVer2) {
		sErrMsg = wstring(L"�L�[�ݒ�t�@�C���̌`�����Ⴂ�܂��B\n\n") + sFileName;
		return false;
	}

	// �f�[�^�̃R�s�[
	m_Common.m_sKeyBind.m_nKeyNameArrNum = nKeyNameArrNum;
	memcpy_raw( m_Common.m_sKeyBind.m_pKeyNameArr, pKeyNameArr, sizeof_raw( pKeyNameArr ) );

	return true;
}

// �G�N�X�|�[�g
bool CImpExpKeybind::Export( const wstring sFileName, wstring& sErrMsg )
{
	TCHAR*		szPath = (TCHAR*)to_tchar( sFileName.c_str() );

	CTextOutputStream out( szPath );
	if (!out) {
		sErrMsg = MSG_NOT_OPEN + sFileName;
		return false;
	}

	out.Close();

	/* �L�[���蓖�ď�� */
	CDataProfile cProfile;

	// �������݃��[�h�ݒ�
	cProfile.SetWritingMode();

	// �w�b�_
	StaticString<wchar_t,256> szKeydataHead = WSTR_KEYBIND_HEAD;
	cProfile.IOProfileData( szSecInfo, L"KEYBIND_VERSION", szKeydataHead );
	cProfile.IOProfileData_WrapInt( szSecInfo, L"KEYBIND_COUNT", m_Common.m_sKeyBind.m_nKeyNameArrNum );

	//���e
	CShareData_IO::ShareData_IO_KeyBind(cProfile, m_Common.m_sKeyBind.m_nKeyNameArrNum, m_Common.m_sKeyBind.m_pKeyNameArr, true);

	// ��������
	if (!cProfile.WriteProfile( szPath, WSTR_KEYBIND_HEAD)) {
		sErrMsg = MSG_NOT_EXPORT + sFileName;
		return false;
	}

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �J�X�^�����j���[                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool CImpExpCustMenu::Import( const wstring sFileName, wstring& sErrMsg )
{
	TCHAR*		szPath = (TCHAR*)to_tchar( sFileName.c_str() );

	//�w�b�_�m�F
	CTextInputStream in(szPath);
	if (!in) {
		sErrMsg = MSG_NOT_OPEN + sFileName;
		return false;
	}

	CDataProfile cProfile;
	cProfile.SetReadingMode();
	cProfile.ReadProfile(szPath);

	//�o�[�W�����m�F
	WCHAR szHeader[256];
	cProfile.IOProfileData(szSecInfo, L"MENU_VERSION", MakeStringBufferW(szHeader));
	if(wcscmp(szHeader, WSTR_CUSTMENU_HEAD_V2)!=0) {
		sErrMsg = wstring( L"�J�X�^�����j���[�ݒ�t�@�C���̌`�����Ⴂ�܂��B\n\n" ) + sFileName;
		return false;
	}

	CShareData_IO::ShareData_IO_CustMenu(cProfile,m_Common.m_sCustomMenu, true);			// 2008/5/24 Uchi

	return true;
}

// �G�N�X�|�[�g
bool CImpExpCustMenu::Export( const wstring sFileName, wstring& sErrMsg )
{
	TCHAR*		szPath = (TCHAR*)to_tchar( sFileName.c_str() );

	// �I�[�v��
	CTextOutputStream out(szPath);
	if (!out) {
		sErrMsg = MSG_NOT_OPEN + sFileName;
		return false;
	}

	out.Close();

	/* �J�X�^�����j���[��� */
	//�w�b�_
	CDataProfile	cProfile;
	CommonSetting_CustomMenu* menu=&m_Common.m_sCustomMenu;

	// �������݃��[�h�ݒ�
	cProfile.SetWritingMode();

	//�w�b�_
	cProfile.IOProfileData( szSecInfo, L"MENU_VERSION", MakeStringBufferW(WSTR_CUSTMENU_HEAD_V2) );
	int iWork = MAX_CUSTOM_MENU;
	cProfile.IOProfileData_WrapInt( szSecInfo, L"MAX_CUSTOM_MENU", iWork );
	
	//���e
	CShareData_IO::ShareData_IO_CustMenu(cProfile, *menu, true);

	// ��������
	if (!cProfile.WriteProfile( szPath, WSTR_CUSTMENU_HEAD_V2)) {
		sErrMsg = MSG_NOT_EXPORT + sFileName;
		return false;
	}

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �����L�[���[�h                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool CImpExpKeyWord::Import( const wstring sFileName, wstring& sErrMsg )
{
	TCHAR*			szPath = (TCHAR*)to_tchar( sFileName.c_str() );

	CDlgOpenFile	cDlgOpenFile;
	bool			bAddError = false;

	CTextInputStream in(szPath);
	if (!in) {
		sErrMsg = MSG_NOT_OPEN + sFileName;
		return false;
	}
	while( in ){
		wstring szLine = in.ReadLineW();

		// �R�����g����
		if (szLine.length() == 0) {
			continue;
		}
		if (2 <= szLine.length() && 0 == auto_memcmp( szLine.c_str(), L"//", 2 )) {
			if (szLine == WSTR_CASE_TRUE) {
				m_bCase = true;
			}
			else if (szLine == WSTR_CASE_FALSE) {
				m_bCase = false;
			}
			continue;
		}
		
		//���
		if( 0 < szLine.length() ){
			/* ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ� */
			int nRetValue = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWord( m_nIdx, szLine.c_str() );
			if( 2 == nRetValue ){
				bAddError = true;
				break;
			}
		}
	}
	in.Close();

	// �啶�����������
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetKeyWordCase( m_nIdx, m_bCase );

	if (bAddError) {
		sErrMsg = L"�L�[���[�h�̐�������ɒB�������߁A�������̃L�[���[�h��ǉ��ł��܂���ł����B";
	}

	return true;
}

// �G�N�X�|�[�g
bool CImpExpKeyWord::Export( const wstring sFileName, wstring& sErrMsg )
{
	TCHAR*		szPath = (TCHAR*)to_tchar( sFileName.c_str() );
	int 		nKeyWordNum;
	int 		i;

	CTextOutputStream out(szPath);
	if (!out) {
		sErrMsg = MSG_NOT_OPEN + sFileName;
		return false;
	}
	out.WriteF( L"// " );
	out.WriteF( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( m_nIdx ) );
	out.WriteF( WSTR_KEYWORD_HEAD );

	out.WriteF( WSTR_KEYWORD_CASE );
	out.WriteF( m_bCase ? L"True" : L"False" );
	out.WriteF( L"\n\n" );

	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SortKeyWord(m_nIdx);	//MIK 2000.12.01 sort keyword

	/* ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ� */
	nKeyWordNum = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordNum( m_nIdx );
	for( i = 0; i < nKeyWordNum; ++i ){
		/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ� */
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( m_nIdx, i );
		out.WriteF( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( m_nIdx, i ) );
		out.WriteF( L"\n" );
	}
	out.Close();

	return true;
}
