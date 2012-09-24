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
#ifndef SAKURA_CDOCFILEOPERATION_A537A9E9_2400_4A59_9188_3CC3A3966862_H_
#define SAKURA_CDOCFILEOPERATION_A537A9E9_2400_4A59_9188_3CC3A3966862_H_

#include "doc/CDocListener.h" // SLoadInfo
#include "CEol.h"

class CEditDoc;

class CDocFileOperation{
public:
	CDocFileOperation(CEditDoc* pcDoc) : m_pcDocRef(pcDoc) { }

	//���b�N
	bool _ToDoLock() const;
	void DoFileLock(bool bMsg = true);
	void DoFileUnlock();
	
	//���[�hUI
	bool OpenFileDialog(
		HWND				hwndParent,
		const TCHAR*		pszOpenFolder,	//!< [in]  NULL�ȊO���w�肷��Ə����t�H���_���w��ł���
		SLoadInfo*			pLoadInfo,		//!< [in/out] ���[�h���
		std::vector<std::tstring>&	files
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

#endif /* SAKURA_CDOCFILEOPERATION_A537A9E9_2400_4A59_9188_3CC3A3966862_H_ */
/*[EOF]*/
