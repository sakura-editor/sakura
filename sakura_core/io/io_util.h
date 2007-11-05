#pragma once

#include "share/CommonSetting.h" //EShareMode

//!指定ファイルに書き込めるかどうか調べる
bool IsFileWritable(LPCTSTR tszFilePath);

//!指定の共有モードでファイルを開く
HANDLE OpenFile_InShareMode(LPCTSTR tszFilePath, EShareMode eShareMode);
