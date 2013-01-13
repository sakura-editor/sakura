/*
	Copyright (C) 2008, kobake, ryoji
	Copyright (C) 2012, Uchi

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
#include "CSakuraEnvironment.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CFormatManager.h"
#include "env/CFileNameManager.h"
#include "_main/CAppMode.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "CPrintPreview.h"
#include "macro/CSMacroMgr.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "recent/CMru.h"
#include "util/string_ex2.h"
#include "util/module.h" //GetAppVersionInfo
#include "util/shell.h"

CEditWnd* CSakuraEnvironment::GetMainWindow()
{
	return CEditWnd::getInstance();
}

/*!	$x�̓W�J

	���ꕶ���͈ȉ��̒ʂ�
	@li $  $���g
	@li A  �A�v����
	@li F  �J���Ă���t�@�C���̃t���p�X�B���O���Ȃ����(����)�B
	@li f  �J���Ă���t�@�C���̖��O�i�t�@�C����+�g���q�̂݁j
	@li g  �J���Ă���t�@�C���̖��O�i�g���q�����j
	@li /  �J���Ă���t�@�C���̖��O�i�t���p�X�B�p�X�̋�؂肪/�j
	@li N  �J���Ă���t�@�C���̖��O(�ȈՕ\��)
	@li E  �J���Ă���t�@�C���̂���t�H���_�̖��O(�ȈՕ\��)
	@li e  �J���Ă���t�@�C���̂���t�H���_�̖��O
	@li C  ���ݑI�𒆂̃e�L�X�g
	@li x  ���݂̕������ʒu(�擪����̃o�C�g��1�J�n)
	@li y  ���݂̕����s�ʒu(1�J�n)
	@li d  ���݂̓��t(���ʐݒ�̓��t����)
	@li t  ���݂̎���(���ʐݒ�̎�������)
	@li p  ���݂̃y�[�W
	@li P  ���y�[�W
	@li D  �t�@�C���̃^�C���X�^���v(���ʐݒ�̓��t����)
	@li T  �t�@�C���̃^�C���X�^���v(���ʐݒ�̎�������)
	@li V  �G�f�B�^�̃o�[�W����������
	@li h  Grep�����L�[�̐擪32byte
	@li S  �T�N���G�f�B�^�̃t���p�X
	@li I  ini�t�@�C���̃t���p�X
	@li M  ���ݎ��s���Ă���}�N���t�@�C���p�X

	@date 2003.04.03 genta wcsncpy_ex�����ɂ��for���̍팸
	@date 2005.09.15 FILE ���ꕶ��S, M�ǉ�
	@date 2007.09.21 kobake ���ꕶ��A(�A�v����)��ǉ�
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
*/
void CSakuraEnvironment::ExpandParameter(const wchar_t* pszSource, wchar_t* pszBuffer, int nBufferLen)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0); //###

	// Apr. 03, 2003 genta �Œ蕶������܂Ƃ߂�
	static const wchar_t	PRINT_PREVIEW_ONLY[]	= L"(����v���r���[�ł̂ݎg�p�ł��܂�)";
	const int				PRINT_PREVIEW_ONLY_LEN	= _countof( PRINT_PREVIEW_ONLY ) - 1;
	static const wchar_t	NO_TITLE[]				= L"(����)";
	const int				NO_TITLE_LEN			= _countof( NO_TITLE ) - 1;
	static const wchar_t	NOT_SAVED[]				= L"(�ۑ�����Ă��܂���)";
	const int				NOT_SAVED_LEN			= _countof( NOT_SAVED ) - 1;

	const wchar_t *p, *r;	//	p�F�ړI�̃o�b�t�@�Br�F��Ɨp�̃|�C���^�B
	wchar_t *q, *q_max;

	for( p = pszSource, q = pszBuffer, q_max = pszBuffer + nBufferLen; *p != '\0' && q < q_max;){
		if( *p != '$' ){
			*q++ = *p++;
			continue;
		}
		switch( *(++p) ){
		case L'$':	//	 $$ -> $
			*q++ = *p++;
			break;
		case L'A':	//�A�v����
			q = wcs_pushW( q, q_max - q, GSTR_APPNAME_W, wcslen(GSTR_APPNAME_W) );
			++p;
			break;
		case L'F':	//	�J���Ă���t�@�C���̖��O�i�t���p�X�j
			if ( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				r = to_wchar(pcDoc->m_cDocFile.GetFilePath());
				q = wcs_pushW( q, q_max - q, r, wcslen( r ));
				++p;
			}
			break;
		case L'f':	//	�J���Ă���t�@�C���̖��O�i�t�@�C����+�g���q�̂݁j
			// Oct. 28, 2001 genta
			//	�t�@�C�����݂̂�n���o�[�W����
			//	�|�C���^�𖖔���
			if ( ! pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				// 2002.10.13 Moca �t�@�C����(�p�X�Ȃ�)���擾�B���{��Ή�
				//	����\\�������ɂ����Ă����̌��ɂ�\0������̂ŃA�N�Z�X�ᔽ�ɂ͂Ȃ�Ȃ��B
				q = wcs_pushT( q, q_max - q, pcDoc->m_cDocFile.GetFileName());
				++p;
			}
			break;
		case L'g':	//	�J���Ă���t�@�C���̖��O�i�g���q�������t�@�C�����̂݁j
			//	From Here Sep. 16, 2002 genta
			if ( ! pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				//	�|�C���^�𖖔���
				const wchar_t *dot_position, *end_of_path;
				r = to_wchar(pcDoc->m_cDocFile.GetFileName()); // 2002.10.13 Moca �t�@�C����(�p�X�Ȃ�)���擾�B���{��Ή�
				end_of_path = dot_position =
					r + wcslen( r );
				//	��납��.��T��
				for( --dot_position ; dot_position > r && *dot_position != '.'
					; --dot_position )
					;
				//	r�Ɠ����ꏊ�܂ōs���Ă��܂�����.����������
				if( dot_position == r )
					dot_position = end_of_path;

				q = wcs_pushW( q, q_max - q, r, dot_position - r );
				++p;
			}
			break;
			//	To Here Sep. 16, 2002 genta
		case L'/':	//	�J���Ă���t�@�C���̖��O�i�t���p�X�B�p�X�̋�؂肪/�j
			// Oct. 28, 2001 genta
			if ( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				//	�p�X�̋�؂�Ƃ���'/'���g���o�[�W����
				for( r = to_wchar(pcDoc->m_cDocFile.GetFilePath()); *r != L'\0' && q < q_max; ++r, ++q ){
					if( *r == L'\\' )
						*q = L'/';
					else
						*q = *r;
				}
				++p;
			}
			break;
		//	From Here 2003/06/21 Moca
		case L'N':	//	�J���Ă���t�@�C���̖��O(�ȈՕ\��)
			if( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			}
			else {
				TCHAR szText[1024];
				CFileNameManager::getInstance()->GetTransformFileNameFast( pcDoc->m_cDocFile.GetFilePath(), szText, 1023 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			break;
		//	To Here 2003/06/21 Moca
		case L'E':	// �J���Ă���t�@�C���̂���t�H���_�̖��O(�ȈՕ\��)	2012/12/2 Uchi
			if( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
			}
			else {
				WCHAR	buff[_MAX_PATH];		// \�̏����������WCHAR
				WCHAR*	pEnd;
				WCHAR*	p;

				wcscpy_s( buff, _MAX_PATH, to_wchar(pcDoc->m_cDocFile.GetFilePath()) );
				pEnd = NULL;
				for ( p = buff; *p != '\0'; p++) {
					if (*p == L'\\') {
						pEnd = p;
					}
				}
				if (pEnd != NULL) {
					// �Ō��\�̌�ŏI�[
					*(pEnd+1) = '\0';
				}

				// �ȈՕ\���ɕϊ�
				TCHAR szText[1024];
				CFileNameManager::getInstance()->GetTransformFileNameFast( to_tchar(buff), szText, _countof(szText)-1 );
				q = wcs_pushT( q, q_max - q, szText);
			}
			++p;
			break;
		case L'e':	// �J���Ă���t�@�C���̂���t�H���_�̖��O		2012/12/2 Uchi
			if( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
			}
			else {
				const WCHAR*	pStr;
				const WCHAR*	pEnd;
				const WCHAR*	p;

				pStr = to_wchar(pcDoc->m_cDocFile.GetFilePath());
				pEnd = pStr - auto_strlen(pStr) - 1;
				for ( p = pStr; *p != '\0'; p++) {
					if (*p == L'\\') {
						pEnd = p;
					}
				}
				q = wcs_pushW( q, q_max - q, pStr, pEnd - pStr + 1 );
			}
			++p;
			break;
		//	From Here Jan. 15, 2002 hor
		case L'C':	//	���ݑI�𒆂̃e�L�X�g
			{
				CNativeW cmemCurText;
				GetMainWindow()->GetActiveView().GetCurrentTextForSearch( cmemCurText );

				q = wcs_pushW( q, q_max - q, cmemCurText.GetStringPtr(), cmemCurText.GetStringLength());
				++p;
			}
		//	To Here Jan. 15, 2002 hor
			break;
		//	From Here 2002/12/04 Moca
		case L'x':	//	���݂̕������ʒu(�擪����̃o�C�g��1�J�n)
			{
				wchar_t szText[11];
				_itow( GetMainWindow()->GetActiveView().GetCaret().GetCaretLogicPos().x + 1, szText, 10 );
				q = wcs_pushW( q, q_max - q, szText);
				++p;
			}
			break;
		case L'y':	//	���݂̕����s�ʒu(1�J�n)
			{
				wchar_t szText[11];
				_itow( GetMainWindow()->GetActiveView().GetCaret().GetCaretLogicPos().y + 1, szText, 10 );
				q = wcs_pushW( q, q_max - q, szText);
				++p;
			}
			break;
		//	To Here 2002/12/04 Moca
		case L'd':	//	���ʐݒ�̓��t����
			{
				TCHAR szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CFormatManager().MyGetDateFormat( systime, szText, _countof( szText ) - 1 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			break;
		case L't':	//	���ʐݒ�̎�������
			{
				TCHAR szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CFormatManager().MyGetTimeFormat( systime, szText, _countof( szText ) - 1 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			break;
		case L'p':	//	���݂̃y�[�W
			{
				CEditWnd*	pcEditWnd = GetMainWindow();	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					wchar_t szText[1024];
					_itow(pcEditWnd->m_pPrintPreview->GetCurPageNum() + 1, szText, 10);
					q = wcs_pushW( q, q_max - q, szText, wcslen(szText));
					++p;
				}
				else {
					q = wcs_pushW( q, q_max - q, PRINT_PREVIEW_ONLY, PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case L'P':	//	���y�[�W
			{
				CEditWnd*	pcEditWnd = GetMainWindow();	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					wchar_t szText[1024];
					_itow(pcEditWnd->m_pPrintPreview->GetAllPageNum(), szText, 10);
					q = wcs_pushW( q, q_max - q, szText);
					++p;
				}
				else {
					q = wcs_pushW( q, q_max - q, PRINT_PREVIEW_ONLY, PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case L'D':	//	�^�C���X�^���v
			if (pcDoc->m_cDocFile.GetDocFileTime().GetFILETIME().dwLowDateTime){
				TCHAR szText[1024];
				CFormatManager().MyGetDateFormat(
					pcDoc->m_cDocFile.GetDocFileTime().GetSYSTEMTIME(),
					szText,
					_countof( szText ) - 1
				);
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			else {
				q = wcs_pushW( q, q_max - q, NOT_SAVED, NOT_SAVED_LEN );
				++p;
			}
			break;
		case L'T':	//	�^�C���X�^���v
			if (pcDoc->m_cDocFile.GetDocFileTime().GetFILETIME().dwLowDateTime){
				TCHAR szText[1024];
				CFormatManager().MyGetTimeFormat(
					pcDoc->m_cDocFile.GetDocFileTime().GetSYSTEMTIME(),
					szText,
					_countof( szText ) - 1
				);
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			else {
				q = wcs_pushW( q, q_max - q, NOT_SAVED, NOT_SAVED_LEN );
				++p;
			}
			break;
		case L'V':	// Apr. 4, 2003 genta
			// Version number
			{
				wchar_t buf[28]; // 6(�����܂�WORD�̍ő咷) * 4 + 4(�Œ蕔��)
				//	2004.05.13 Moca �o�[�W�����ԍ��́A�v���Z�X���ƂɎ擾����
				DWORD dwVersionMS, dwVersionLS;
				GetAppVersionInfo( NULL, VS_VERSION_INFO, &dwVersionMS, &dwVersionLS );
				int len = auto_sprintf( buf, L"%d.%d.%d.%d",
					HIWORD( dwVersionMS ),
					LOWORD( dwVersionMS ),
					HIWORD( dwVersionLS ),
					LOWORD( dwVersionLS )
				);
				q = wcs_pushW( q, q_max - q, buf, len );
				++p;
			}
			break;
		case L'h':	//	Apr. 4, 2003 genta
			//	Grep Key������ MAX 32����
			//	���g��SetParentCaption()���ڐA
			{
				CNativeW	cmemDes;
				// m_szGrepKey �� cmemDes
				LimitStringLengthW( CAppMode::getInstance()->m_szGrepKey, wcslen( CAppMode::getInstance()->m_szGrepKey ), (q_max - q > 32 ? 32 : q_max - q - 3), cmemDes );
				if( (int)wcslen( CAppMode::getInstance()->m_szGrepKey ) > cmemDes.GetStringLength() ){
					cmemDes.AppendString(L"...");
				}
				q = wcs_pushW( q, q_max - q, cmemDes.GetStringPtr(), cmemDes.GetStringLength());
				++p;
			}
			break;
		case L'S':	//	Sep. 15, 2005 FILE
			//	�T�N���G�f�B�^�̃t���p�X
			{
				SFilePath	szPath;

				::GetModuleFileName( NULL, szPath, _countof2(szPath) );
				q = wcs_pushT( q, q_max - q, szPath );
				++p;
			}
			break;
		case 'I':	//	May. 19, 2007 ryoji
			//	ini�t�@�C���̃t���p�X
			{
				TCHAR	szPath[_MAX_PATH + 1];
				CFileNameManager::getInstance()->GetIniFileName( szPath );
				q = wcs_pushT( q, q_max - q, szPath );
				++p;
			}
			break;
		case 'M':	//	Sep. 15, 2005 FILE
			//	���ݎ��s���Ă���}�N���t�@�C���p�X�̎擾
			{
				// ���s���}�N���̃C���f�b�N�X�ԍ� (INVALID_MACRO_IDX:���� / STAND_KEYMACRO:�W���}�N��)
				switch( CEditApp::getInstance()->m_pcSMacroMgr->GetCurrentIdx() ){
				case INVALID_MACRO_IDX:
					break;
				case TEMP_KEYMACRO:
					q = wcs_pushT( q, q_max - q, CEditApp::getInstance()->m_pcSMacroMgr->GetFile(TEMP_KEYMACRO) );
					break;
				case STAND_KEYMACRO:
					{
						TCHAR* pszMacroFilePath = GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName;
						q = wcs_pushT( q, q_max - q, pszMacroFilePath );
					}
					break;
				default:
					{
						TCHAR szMacroFilePath[_MAX_PATH * 2];
						int n = CShareData::getInstance()->GetMacroFilename( CEditApp::getInstance()->m_pcSMacroMgr->GetCurrentIdx(), szMacroFilePath, _countof(szMacroFilePath) );
						if ( 0 < n ){
							q = wcs_pushT( q, q_max - q, szMacroFilePath );
						}
					}
					break;
				}
				++p;
			}
			break;
		//	Mar. 31, 2003 genta
		//	��������
		//	${cond:string1$:string2$:string3$}
		//	
		case L'{':	// ��������
			{
				int cond;
				cond = _ExParam_Evaluate( p + 1 );
				while( *p != '?' && *p != '\0' )
					++p;
				if( *p == '\0' )
					break;
				p = _ExParam_SkipCond( p + 1, cond );
			}
			break;
		case L':':	// ��������̒���
			//	��������̖����܂�SKIP
			p = _ExParam_SkipCond( p + 1, -1 );
			break;
		case L'}':	// ��������̖���
			//	���ɂ��邱�Ƃ͂Ȃ�
			++p;
			break;
		default:
			*q++ = '$';
			*q++ = *p++;
			break;
		}
	}
	*q = '\0';
}



/*! @brief �����̓ǂݔ�΂�

	��������̍\�� ${cond:A0$:A1$:A2$:..$} �ɂ����āC
	�w�肵���ԍ��ɑΉ�����ʒu�̐擪�ւ̃|�C���^��Ԃ��D
	�w��ԍ��ɑΉ����镨���������$}�̎��̃|�C���^��Ԃ��D

	${���o�ꂵ���ꍇ�ɂ̓l�X�g�ƍl����$}�܂œǂݔ�΂��D

	@param pszSource [in] �X�L�������J�n���镶����̐擪�Dcond:�̎��̃A�h���X��n���D
	@param part [in] �ړ�����ԍ����ǂݔ�΂�$:�̐��D-1��^����ƍŌ�܂œǂݔ�΂��D

	@return �ړ���̃|�C���^�D�Y���̈�̐擪�����邢��$}�̒���D

	@author genta
	@date 2003.03.31 genta �쐬
*/
const wchar_t* CSakuraEnvironment::_ExParam_SkipCond(const wchar_t* pszSource, int part)
{
	if( part == 0 )
		return pszSource;
	
	int nest = 0;	// ����q�̃��x��
	bool next = true;	// �p���t���O
	const wchar_t *p;
	for( p = pszSource; next && *p != L'\0'; ++p ) {
		if( *p == L'$' && p[1] != L'\0' ){ // $�������Ȃ疳��
			switch( *(++p)){
			case L'{':	// ����q�̊J�n
				++nest;
				break;
			case L'}':
				if( nest == 0 ){
					//	�I���|�C���g�ɒB����
					next = false; 
				}
				else {
					//	�l�X�g���x����������
					--nest;
				}
				break;
			case L':':
				if( nest == 0 && --part == 0){ // ����q�łȂ��ꍇ�̂�
					//	�ړI�̃|�C���g
					next = false;
				}
				break;
			}
		}
	}
	return p;
}

/*!	@brief �����̕]��

	@param pCond [in] ������ʐ擪�D'?'�܂ł������ƌ��Ȃ��ĕ]������
	@return �]���̒l

	@note
	�|�C���^�̓ǂݔ�΂���Ƃ͍s��Ȃ��̂ŁC'?'�܂ł̓ǂݔ�΂���
	�Ăяo�����ŕʓr�s���K�v������D

	@author genta
	@date 2003.03.31 genta �쐬

*/
int CSakuraEnvironment::_ExParam_Evaluate( const wchar_t* pCond )
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0); //###

	switch( *pCond ){
	case L'R': // $R �r���[���[�h����ѓǂݎ���p����
		if( CAppMode::getInstance()->IsViewMode() ){
			return 0; // �r���[���[�h
		}
		else if( !CEditDoc::GetInstance(0)->m_cDocLocker.IsDocWritable() ){
			return 1; // �㏑���֎~
		}
		else{
			return 2; // ��L�ȊO
		}
	case L'w': // $w Grep���[�h/Output Mode
		if( CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode ){
			return 0;
		}else if( CAppMode::getInstance()->IsDebugMode() ){
			return 1;
		}else {
			return 2;
		}
	case L'M': // $M �L�[�{�[�h�}�N���̋L�^��
		if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro && GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro==CEditWnd::getInstance()->GetHwnd() ){ /* �E�B���h�E */
			return 0;
		}else {
			return 1;
		}
	case L'U': // $U �X�V
		if( pcDoc->m_cDocEditor.IsModified()){
			return 0;
		}
		else {
			return 1;
		}
	case L'N': // $N �V�K/(����)		2012/12/2 Uchi
		if (!pcDoc->m_cDocFile.GetFilePathClass().IsValidPath()) {
			return 0;
		}
		else {
			return 1;
		}
	case L'I': // $I �A�C�R��������Ă��邩
		if( ::IsIconic( CEditWnd::getInstance()->GetHwnd() )){
			return 0;
		} else {
 			return 1;
 		}
	default:
		return 0;
	}
	return 0;
}



std::tstring CSakuraEnvironment::GetDlgInitialDir()
{
	CEditDoc* pcDoc = CEditDoc::GetInstance(0); //######
	if( pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
		return to_tchar(pcDoc->m_cDocFile.GetFilePathClass().GetDirPath().c_str());
	}
	else if( GetDllShareData().m_Common.m_sEdit.m_eOpenDialogDir == OPENDIALOGDIR_CUR ){
		// 2002.10.25 Moca
		TCHAR pszCurDir[_MAX_PATH];
		int nCurDir = ::GetCurrentDirectory( _countof(pszCurDir), pszCurDir );
		if( 0 == nCurDir || _MAX_PATH < nCurDir ){
			return _T("");
		}
		else{
			return pszCurDir;
		}
	}else if( GetDllShareData().m_Common.m_sEdit.m_eOpenDialogDir == OPENDIALOGDIR_MRU ){
		const CMRU cMRU;
		std::vector<LPCTSTR> vMRU = cMRU.GetPathList();
		if( !vMRU.empty() ){
			return vMRU[0];
		}else{
			TCHAR pszCurDir[_MAX_PATH];
			int nCurDir = ::GetCurrentDirectory( _countof(pszCurDir), pszCurDir );
			if( 0 == nCurDir || _MAX_PATH < nCurDir ){
				return _T("");
			}
			else{
				return pszCurDir;
			}
		}
	}else{
		TCHAR selDir[_MAX_PATH];
		CFileNameManager::ExpandMetaToFolder( GetDllShareData().m_Common.m_sEdit.m_OpenDialogSelDir, selDir, _countof(selDir) );
		return selDir;
	}
}

void CSakuraEnvironment::ResolvePath(TCHAR* pszPath)
{
	// pszPath -> pSrc
	TCHAR* pSrc = pszPath;

	// �V���[�g�J�b�g(.lnk)�̉���: pSrc -> szBuf -> pSrc
	TCHAR szBuf[_MAX_PATH];
	if( ResolveShortcutLink( NULL, pSrc, szBuf ) ){
		pSrc = szBuf;
	}

	// �����O�t�@�C�������擾����: pSrc -> szBuf2 -> pSrc
	TCHAR szBuf2[_MAX_PATH];
	if( ::GetLongFileName( pSrc, szBuf2 ) ){
		pSrc = szBuf2;
	}

	// pSrc -> pszPath
	if(pSrc != pszPath){
		_tcscpy_s(pszPath, _MAX_PATH, pSrc);
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �E�B���h�E�Ǘ�                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/* �w��E�B���h�E���A�ҏW�E�B���h�E�̃t���[���E�B���h�E���ǂ������ׂ� */
BOOL IsSakuraMainWindow( HWND hWnd )
{
	TCHAR	szClassName[64];
	if( hWnd == NULL ){	// 2007.06.20 ryoji �����ǉ�
		return FALSE;
	}
	if( !::IsWindow( hWnd ) ){
		return FALSE;
	}
	if( 0 == ::GetClassName( hWnd, szClassName, _countof(szClassName) - 1 ) ){
		return FALSE;
	}
	if(0 == _tcscmp( GSTR_EDITWINDOWNAME, szClassName ) ){
		return TRUE;
	}else{
		return FALSE;
	}
}


