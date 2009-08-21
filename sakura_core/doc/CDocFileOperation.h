#pragma once

class CEditDoc;

class CDocFileOperation{
public:
	CDocFileOperation(CEditDoc* pcDoc) : m_pcDocRef(pcDoc) { }

	//���b�N
	bool _ToDoLock() const;
	void DoFileLock();
	void DoFileUnlock();
	
	//���[�hUI
	bool OpenFileDialog(
		HWND				hwndParent,
		const TCHAR*		pszOpenFolder,	//!< [in]  NULL�ȊO���w�肷��Ə����t�H���_���w��ł���
		SLoadInfo*			pLoadInfo		//!< [in/out] ���[�h���
	);

	//���[�h�t���[
	bool DoLoadFlow(SLoadInfo* pLoadInfo);
	bool FileLoad(
		SLoadInfo*	pLoadInfo			//!< [in/out]
	);
	bool FileLoadWithoutAutoMacro(
		SLoadInfo*	pLoadInfo			//!< [in/out]
	);
	void ReloadCurrentFile(				//!< ����t�@�C���̍ăI�[�v�� Jul. 26, 2003 ryoji BOM�I�v�V�����ǉ�
		ECodeType	nCharCode			//!< [in] �����R�[�h���
	);

	
	//�Z�[�uUI
	bool SaveFileDialog(SSaveInfo* pSaveInfo);	//!<�u�t�@�C������t���ĕۑ��v�_�C�A���O
	bool SaveFileDialog(LPTSTR szPath);			//!<�u�t�@�C������t���ĕۑ��v�_�C�A���O

	//�Z�[�u�t���[
	bool DoSaveFlow(SSaveInfo* pSaveInfo);
	bool FileSaveAs( const WCHAR* filename = NULL, EEolType eEolType = EOL_NONE );			//!< �_�C�A���O�Ńt�@�C��������͂����A�ۑ��B	// 2006.12.30 ryoji
	bool FileSave();			//!< �㏑���ۑ��B�t�@�C�������w�肳��Ă��Ȃ�������_�C�A���O�œ��͂𑣂��B	// 2006.12.30 ryoji

	//�N���[�Y
	bool FileClose();			//!< ����(����)	// 2006.12.30 ryoji

	//���̑�
	void FileCloseOpen(				//!< ���ĊJ��	// 2006.12.30 ryoji
		const SLoadInfo& sLoadInfo = SLoadInfo(_T(""), CODE_AUTODETECT, false)
	);

private:
	CEditDoc* m_pcDocRef;
};
