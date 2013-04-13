/*
	2008.05.18 kobake CShareData ���番��
*/
/*
	Copyright (C) 2008, kobake

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
#include "DLLSHAREDATA.h"

#include "CHelpManager.h"
#include "env/CDocTypeManager.h"


/*!	�O��Win�w���v���ݒ肳��Ă��邩�m�F�B
*/
bool CHelpManager::ExtWinHelpIsSet( CTypeConfig nTypeNo )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHelp[0] != L'\0'){
		return true;	//	���ʐݒ�ɐݒ肳��Ă���
	}
	if (!nTypeNo.IsValid()) {
		return false;	//	���ʐݒ�ɐݒ肳��Ă��Ȃ���nTypeNo���͈͊O�B
	}
	if (CDocTypeManager().GetTypeSetting(nTypeNo).m_szExtHelp[0] != L'\0'){
		return true;	//	�^�C�v�ʐݒ�ɐݒ肳��Ă���B
	}
	return false;
}

/*!	�ݒ肳��Ă���O��Win�w���v�̃t�@�C������Ԃ��B
	�^�C�v�ʐݒ�Ƀt�@�C�������ݒ肳��Ă���΁A���̃t�@�C������Ԃ��܂��B
	�����łȂ���΁A���ʐݒ�̃t�@�C������Ԃ��܂��B
*/
const TCHAR* CHelpManager::GetExtWinHelp( CTypeConfig nTypeNo )
{
	if (nTypeNo.IsValid() && CDocTypeManager().GetTypeSetting(nTypeNo).m_szExtHelp[0] != _T('\0')){
		return CDocTypeManager().GetTypeSetting(nTypeNo).m_szExtHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHelp;
}

/*!	�O��HTML�w���v���ݒ肳��Ă��邩�m�F�B
*/
bool CHelpManager::ExtHTMLHelpIsSet( CTypeConfig nTypeNo )
{
	if (m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp[0] != L'\0'){
		return true;	//	���ʐݒ�ɐݒ肳��Ă���
	}
	if (!nTypeNo.IsValid()){
		return false;	//	���ʐݒ�ɐݒ肳��Ă��Ȃ���nTypeNo���͈͊O�B
	}
	if (nTypeNo->m_szExtHtmlHelp[0] != L'\0'){
		return true;	//	�^�C�v�ʐݒ�ɐݒ肳��Ă���B
	}
	return false;
}

/*!	�ݒ肳��Ă���O��Win�w���v�̃t�@�C������Ԃ��B
	�^�C�v�ʐݒ�Ƀt�@�C�������ݒ肳��Ă���΁A���̃t�@�C������Ԃ��܂��B
	�����łȂ���΁A���ʐݒ�̃t�@�C������Ԃ��܂��B
*/
const TCHAR* CHelpManager::GetExtHTMLHelp( CTypeConfig nTypeNo )
{
	if (nTypeNo.IsValid() && CDocTypeManager().GetTypeSetting(nTypeNo).m_szExtHtmlHelp[0] != _T('\0')){
		return CDocTypeManager().GetTypeSetting(nTypeNo).m_szExtHtmlHelp;
	}
	
	return m_pShareData->m_Common.m_sHelper.m_szExtHtmlHelp;
}

/*!	�r���[�A�𕡐��N�����Ȃ���ON����Ԃ��B
*/
bool CHelpManager::HTMLHelpIsSingle( CTypeConfig nTypeNo )
{
	if (nTypeNo.IsValid() && CDocTypeManager().GetTypeSetting(nTypeNo).m_szExtHtmlHelp[0] != L'\0'){
		return CDocTypeManager().GetTypeSetting(nTypeNo).m_bHtmlHelpIsSingle;
	}

	return m_pShareData->m_Common.m_sHelper.m_bHtmlHelpIsSingle;
}
