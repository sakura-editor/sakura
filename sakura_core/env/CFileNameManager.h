/*
	2008.05.18 kobake CShareData ���番��
*/

#pragma once

// �v��s��`
// #include "DLLSHAREDATA.h"

#include "util/design_template.h"

//! ini�t�H���_�ݒ�	// 2007.05.31 ryoji
struct IniFolder {
	bool m_bInit;							// �������σt���O
	bool m_bReadPrivate;					// �}���`���[�U�pini����̓ǂݏo���t���O
	bool m_bWritePrivate;					// �}���`���[�U�pini�ւ̏������݃t���O
	TCHAR m_szIniFile[_MAX_PATH];			// EXE���ini�t�@�C���p�X
	TCHAR m_szPrivateIniFile[_MAX_PATH];	// �}���`���[�U�p��ini�t�@�C���p�X
};	/* ini�t�H���_�ݒ� */


//���L���������\����
struct SShare_FileNameManagement{
	IniFolder			m_IniFolder;	/**** ini�t�H���_�ݒ� ****/
};


//!�t�@�C�����Ǘ�
class CFileNameManager : public TSingleton<CFileNameManager>{
public:
	friend class TSingleton<CFileNameManager>;
protected:
	CFileNameManager()
	{
		m_pShareData = &GetDllShareData();
		m_nTransformFileNameCount = -1;
	}
public:
	//�t�@�C�����֘A
	LPTSTR GetTransformFileNameFast( LPCTSTR, LPTSTR, int );	// 2002.11.24 Moca Add
	int TransformFileName_MakeCache( void );
	static LPCTSTR GetFilePathFormat( LPCTSTR, LPTSTR, int, LPCTSTR, LPCTSTR );
	static bool ExpandMetaToFolder( LPCTSTR, LPTSTR, int );
	static void GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile );	/* �\���ݒ�t�@�C������ini�t�@�C�������擾���� */	// 2007.09.04 ryoji
	void GetIniFileName( LPTSTR pszIniFileName, BOOL bRead = FALSE );	/* ini�t�@�C�����̎擾 */	// 2007.05.19 ryoji

private:
	DLLSHAREDATA* m_pShareData;

	// �t�@�C�����ȈՕ\���p�L���b�V��
	int		m_nTransformFileNameCount; // �L����
	TCHAR	m_szTransformFileNameFromExp[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	int		m_nTransformFileNameOrgId[MAX_TRANSFORM_FILENAME];
};
