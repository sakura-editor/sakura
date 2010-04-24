// printf�n���b�v�֐��Q
// 2007.09.20 kobake �쐬�B
//
// �d�v�ȓ����Ƃ��āA�Ǝ��̃t�B�[���h "%ts" ����� "%tc" ��F�����ď�������A�Ƃ����_������܂��B
// UNICODE�r���h�ł� "%ts", "%tc" �͂��ꂼ�� "%ls", %lc" �Ƃ��ĔF������A
// ANSI�r���h�ł�    "%ts", "%tc" �͂��ꂼ�� "%hs", %hc" �Ƃ��ĔF������܂��B
//
// "%s", "%c" �͎g�p�֐��ɂ��^���ς��Achar, wchar_t �����݂���R�[�f�B���O���ł̓o�O�̌��ƂȂ�₷���̂ŁA
// �ł��邾���A��ɋL�����悤�Ȗ����I�Ȍ^�w��������t�B�[���h��p���Ă��������B
//
// ���ӁF%10ts %.12ts �̂悤�Ȃ��͖̂��T�|�[�g
//
// ++ ++ ���P�� ++ ++
//
// �����܂ł��W�����C�u����������u���b�v�v���Ă��邾���Ȃ̂ŁA
// ���̃��b�v�������A�p�t�H�[�}���X�͈����ł��B
// �W�����C�u�����ɗ��炸�ɑS�Ď��O�Ŏ�������΁A�W�����C�u�������݂̃p�t�H�[�}���X��������͂��ł��B
//
// ������Ɗ֐�����������ɂ����̂ŁA�����Ɨǂ����O��W�B
// ���̂܂܂��ƁA��L������ǂ܂Ȃ���΁A_tsprintf �Ƃ��Ɖ����Ⴄ�́H�Ǝv��ꂿ�Ⴂ�����B�B�B
//
// �v���W�F�N�g�S�̂�TCHAR�ɗ���Ȃ��̂ł���΁A�����̊֐��Q�͕s�v�B
//

#pragma once

// vsprintf_s���b�v
int tchar_vsprintf_s (ACHAR* buf, size_t nBufCount, const ACHAR* format, va_list& v);
int tchar_vswprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, va_list& v);

// vsprintf���b�v
int tchar_vsprintf (ACHAR* buf, const ACHAR* format, va_list& v);
int tchar_vswprintf(WCHAR* buf, const WCHAR* format, va_list& v);

// vsnprintf_s���b�v
int tchar_vsnprintf_s (ACHAR* buf, size_t nBufCount, const ACHAR* format, va_list& v);
int tchar_vsnwprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, va_list& v);

// sprintf_s���b�v
int tchar_sprintf_s (ACHAR* buf, size_t nBufCount, const ACHAR* format, ...);
int tchar_swprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, ...);

// sprintf���b�v
int tchar_sprintf(ACHAR* buf, const ACHAR* format, ...);
int tchar_swprintf(WCHAR* buf, const WCHAR* format, ...);

// _snprintf_s���b�v
int tchar_snprintf_s (ACHAR* buf, size_t count, const ACHAR* format, ...);
int tchar_snwprintf_s(WCHAR* buf, size_t count, const WCHAR* format, ...);
