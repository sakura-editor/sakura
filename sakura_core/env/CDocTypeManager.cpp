/*
	2008.05.18 kobake CShareData ���番��
*/

#include "StdAfx.h"
#include "CDocTypeManager.h"


/*!
	�t�@�C��������A�h�L�������g�^�C�v�i���l�j���擾����
	
	@param pszFilePath [in] �t�@�C����
	
	�g���q��؂�o���� GetDocumentTypeOfExt �ɓn�������D
*/
CTypeConfig CDocTypeManager::GetDocumentTypeOfPath( const TCHAR* pszFilePath )
{
	TCHAR	szExt[_MAX_EXT];
	TCHAR	szName[_MAX_FNAME];
	TCHAR*	pszExt = szExt;

	if( NULL != pszFilePath && pszFilePath[0] ){
		_tsplitpath( pszFilePath, NULL, NULL, szName, szExt );
		// 2�d�g���q�T��
		TCHAR* pFileExt = _tcschr( szName, '.' );
		if( pFileExt ){
			pFileExt++;
			auto_strcat( pFileExt, pszExt );
		}else{
			if( 0 == pszExt[0] ){
				// �g���q���t�@�C���ɂȂ�
				pFileExt = szName;
			}else{
				pFileExt = pszExt + 1;
			}
		}
		return GetDocumentTypeOfExt( pFileExt );
	}
	return CTypeConfig(0);
}


/*!
	�g���q����A�h�L�������g�^�C�v�i���l�j���擾����
	
	@param pszExt [in] �g���q (�擪��.�͊܂܂Ȃ�)
	
	�w�肳�ꂽ�g���q�̑����镶���^�C�v�ԍ���Ԃ��D
	�Ƃ肠�������̂Ƃ���̓^�C�v�͊g���q�݂̂Ɉˑ�����Ɖ��肵�Ă���D
	�t�@�C���S�̂̌`���ɑΉ�������Ƃ��́C�܂��l�������D
	@date 2012.10.22 Moca �Q�d�g���q, �g���q�Ȃ��ɑΉ�
*/
CTypeConfig CDocTypeManager::GetDocumentTypeOfExt( const TCHAR* pszExt )
{
	const TCHAR	pszSeps[] = _T(" ;,");	// separator

	int		i;
	TCHAR*	pszToken;
	TCHAR	szText[MAX_TYPES_EXTS];

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
	const TCHAR* pFileExt = _tcschr( pszExt, _T('.') );
	if( pFileExt && pFileExt[1] ){
		return GetDocumentTypeOfExt( pFileExt + 1 );
	}
	return CTypeConfig(0);	//	�n�Y��
}
