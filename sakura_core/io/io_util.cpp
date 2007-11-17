#include "stdafx.h"
#include "io_util.h"

//!�w��t�@�C���ɏ������߂邩�ǂ������ׂ�
bool IsFileWritable(LPCTSTR tszFilePath)
{
	//�������߂邩����
	HANDLE hFile = CreateFile(
		tszFilePath,					//�t�@�C����
		GENERIC_READ | GENERIC_WRITE,	//�ǂݏ������[�h
		0,								//���L����
		NULL,							//����̃Z�L�����e�B�L�q�q
		OPEN_EXISTING,					//�t�@�C�������݂��Ȃ���Ύ��s
		FILE_ATTRIBUTE_NORMAL,			//���ɑ����͎w�肵�Ȃ�
		NULL							//�e���v���[�g����
	);
	if(hFile==INVALID_HANDLE_VALUE){
		return false;
	}
	CloseHandle(hFile);
	return true;
}

//!�w��̋��L���[�h�Ńt�@�C�����J��
HANDLE OpenFile_InShareMode(LPCTSTR tszFilePath, EShareMode eShareMode)
{
	DWORD dwShareMode=0;

	//�t���O���Z�o����
	switch(eShareMode){
	case SHAREMODE_DENY_READWRITE:	// �ǂݏ����֎~
		dwShareMode = 0; //���L����
		break;
	case SHAREMODE_DENY_WRITE:	// �������݋֎~
		dwShareMode = FILE_SHARE_READ; //�ǂݍ��݂̂ݔF�߂�
		break;
	default: //�֎~�����Ȃ�
		dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE; //�ǂݏ������ɔF�߂�
		break;
	}

	//API�Ăяo��
	HANDLE hFile = CreateFile(
		tszFilePath,					//�t�@�C����
		GENERIC_READ | GENERIC_WRITE,	//�ǂݏ����^�C�v
		dwShareMode,					//���L���[�h
		NULL,							//����̃Z�L�����e�B�L�q�q
		OPEN_EXISTING,					//�t�@�C�������݂��Ȃ���Ύ��s
		FILE_ATTRIBUTE_NORMAL,			//���ɑ����͎w�肵�Ȃ�
		NULL							//�e���v���[�g����
	);

	return hFile;
}
