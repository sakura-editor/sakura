#pragma once

#include "share/CommonSetting.h" //EShareMode

//!�w��t�@�C���ɏ������߂邩�ǂ������ׂ�
bool IsFileWritable(LPCTSTR tszFilePath);

//!�w��̋��L���[�h�Ńt�@�C�����J��
HANDLE OpenFile_InShareMode(LPCTSTR tszFilePath, EShareMode eShareMode);
