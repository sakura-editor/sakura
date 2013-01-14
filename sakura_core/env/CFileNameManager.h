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
#ifndef SAKURA_CFILENAMEMANAGER_862D56B4_E24F_49AB_AABD_0924391CB6F4_H_
#define SAKURA_CFILENAMEMANAGER_862D56B4_E24F_49AB_AABD_0924391CB6F4_H_

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

	//���j���[�ނ̃t�@�C�����쐬
	bool GetMenuFullLabel_WinList(TCHAR* pszOutput, int nBuffSize, const EditInfo* editInfo, int id, int index){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, editInfo, id, false, index, false);
	}
	bool GetMenuFullLabel_MRU(TCHAR* pszOutput, int nBuffSize, const EditInfo* editInfo, int id, bool bFavorite, int index){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, editInfo, id, bFavorite, index, true);
	}
	bool GetMenuFullLabel_WinListNoEscape(TCHAR* pszOutput, int nBuffSize, const EditInfo* editInfo, int id, int index){
		return GetMenuFullLabel(pszOutput, nBuffSize, false, editInfo, id, false, index, false);
	}
	bool GetMenuFullLabel_File(TCHAR* pszOutput, int nBuffSize, const TCHAR* pszFile, int id, bool bModified = false, ECodeType nCharCode = CODE_NONE){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, pszFile, id, false, nCharCode, false, -1, false);
	}
	bool GetMenuFullLabel_FileNoEscape(TCHAR* pszOutput, int nBuffSize, const TCHAR* pszFile, int id, bool bModified = false, ECodeType nCharCode = CODE_NONE){
		return GetMenuFullLabel(pszOutput, nBuffSize, false, pszFile, id, false, nCharCode, false, -1, false);
	}

	bool GetMenuFullLabel(TCHAR* pszOutput, int nBuffSize, bool bEspaceAmp, const EditInfo* editInfo, int id, bool bFavorite, int index, bool bAccKeyZeroOrigin);
	bool GetMenuFullLabel(TCHAR* pszOutput, int nBuffSize, bool bEspaceAmp, const TCHAR* pszFile, int id, bool bModified, ECodeType nCharCode, bool bFavorite, int index, bool bAccKeyZeroOrigin);
	
	static TCHAR GetAccessKeyByIndex(int index, bool bZeroOrigin);

	static void GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile );	/* �\���ݒ�t�@�C������ini�t�@�C�������擾���� */	// 2007.09.04 ryoji
	void GetIniFileName( LPTSTR pszIniFileName, BOOL bRead = FALSE );	/* ini�t�@�C�����̎擾 */	// 2007.05.19 ryoji

private:
	DLLSHAREDATA* m_pShareData;

	// �t�@�C�����ȈՕ\���p�L���b�V��
	int		m_nTransformFileNameCount; // �L����
	TCHAR	m_szTransformFileNameFromExp[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	int		m_nTransformFileNameOrgId[MAX_TRANSFORM_FILENAME];
};

#endif /* SAKURA_CFILENAMEMANAGER_862D56B4_E24F_49AB_AABD_0924391CB6F4_H_ */
/*[EOF]*/
