#include "stdafx.h"
#include "CReadHelper.h"
#include "util/file.h"

bool IsDirectory(LPCTSTR pszPath)
{
	HANDLE			hFind;
	WIN32_FIND_DATA	w32fd;
	hFind = ::FindFirstFile( pszPath, &w32fd );
	if(hFind!=INVALID_HANDLE_VALUE){
		::FindClose( hFind );
		return (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	else{
		return false;
	}
}


ECallbackResult CReadHelper::OnCheckLoad(SLoadInfo* pLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//�t�H���_���w�肳�ꂽ�ꍇ�́u�t�@�C�����J���v�_�C�A���O��\�����A���ۂ̃t�@�C�����͂𑣂�
	if( IsDirectory(pLoadInfo->cFilePath) ){
		pLoadInfo->eCharCode = CODE_AUTODETECT;	// �����R�[�h��������
		pLoadInfo->bViewMode = false;			// �r���[���[�h�I�t
		bool bDlgResult = pcDoc->m_cDocFileOperation.OpenFileDialog(
			CEditWnd::Instance()->GetHwnd(),
			pLoadInfo->cFilePath.GetDirPath().c_str(),	//�w�肳�ꂽ�t�H���_
			pLoadInfo
		);
		if( !bDlgResult ){
			return CALLBACK_INTERRUPT; //�L�����Z�����ꂽ�ꍇ�͒��f
		}
	}
	return CALLBACK_CONTINUE;
}
