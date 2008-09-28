/*
	2008.05.18 kobake CShareData ���番��
*/

#pragma once

#include "DLLSHAREDATA.h"

//! �h�L�������g�^�C�v�Ǘ�
class CDocTypeManager{
public:
	CDocTypeManager()
	{
		m_pShareData = &GetDllShareData();
	}
	CTypeConfig GetDocumentTypeOfPath( const TCHAR* pszFilePath );	/* �t�@�C���p�X��n���āA�h�L�������g�^�C�v�i���l�j���擾���� */
	CTypeConfig GetDocumentTypeOfExt( const TCHAR* pszExt );		/* �g���q��n���āA�h�L�������g�^�C�v�i���l�j���擾���� */

	STypeConfig& GetTypeSetting(CTypeConfig cDocumentType);
private:
	DLLSHAREDATA* m_pShareData;
};
