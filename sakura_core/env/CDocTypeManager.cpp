/*
	2008.05.18 kobake CShareData ���番��
*/

#include "stdafx.h"
#include "CDocTypeManager.h"


/*!
	�t�@�C��������A�h�L�������g�^�C�v�i���l�j���擾����
	
	@param pszFilePath [in] �t�@�C����
	
	�g���q��؂�o���� GetDocumentTypeOfExt �ɓn�������D
*/
CTypeConfig CDocTypeManager::GetDocumentTypeOfPath( const TCHAR* pszFilePath )
{
	TCHAR	szExt[_MAX_EXT];

	if( NULL != pszFilePath && 0 < (int)_tcslen( pszFilePath ) ){
		_tsplitpath( pszFilePath, NULL, NULL, NULL, szExt );
		if( szExt[0] == _T('.') )
			return GetDocumentTypeOfExt( szExt + 1 );
		else
			return GetDocumentTypeOfExt( szExt );
	}
	return CTypeConfig(0);
}


/*!
	�g���q����A�h�L�������g�^�C�v�i���l�j���擾����
	
	@param pszExt [in] �g���q (�擪��,�͊܂܂Ȃ�)
	
	�w�肳�ꂽ�g���q�̑����镶���^�C�v�ԍ���Ԃ��D
	�Ƃ肠�������̂Ƃ���̓^�C�v�͊g���q�݂̂Ɉˑ�����Ɖ��肵�Ă���D
	�t�@�C���S�̂̌`���ɑΉ�������Ƃ��́C�܂��l�������D
*/
CTypeConfig CDocTypeManager::GetDocumentTypeOfExt( const TCHAR* pszExt )
{
	const TCHAR	pszSeps[] = _T(" ;,");	// separator

	int		i;
	TCHAR*	pszToken;
	TCHAR	szText[256];

	for( i = 0; i < MAX_TYPES; ++i ){
		CTypeConfig nType(i);
		_tcscpy( szText, CDocTypeManager().GetTypeSetting(nType).m_szTypeExts );
		pszToken = _tcstok( szText, pszSeps );
		while( NULL != pszToken ){
			if( 0 == _tcsicmp( pszExt, pszToken ) ){
				return CTypeConfig(i);	//	�ԍ�
			}
			pszToken = _tcstok( NULL, pszSeps );
		}
	}
	return CTypeConfig(0);	//	�n�Y��
}
