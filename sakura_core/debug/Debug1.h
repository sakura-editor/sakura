/*!	@file
	@brief �f�o�b�O�p�֐�

	@author Norio Nakatani

	@date 2013/03/03 Uchi MessageBox�p�֐��𕪗�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_DEBUG1_587B8A50_4B0A_4E5E_A638_40FB1EC301CA_H_
#define SAKURA_DEBUG1_587B8A50_4B0A_4E5E_A638_40FB1EC301CA_H_

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   ���b�Z�[�W�o�́F����                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#if defined(_DEBUG) || defined(USE_RELPRINT)
SAKURA_CORE_API void DebugOutW( LPCWSTR lpFmt, ...);
SAKURA_CORE_API void DebugOutA( LPCSTR lpFmt, ...);
#endif	// _DEBUG || USE_RELPRINT

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 �f�o�b�O�p���b�Z�[�W�o��                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*
	MYTRACE�̓����[�X���[�h�ł̓R���p�C���G���[�ƂȂ�悤�ɂ��Ă���̂ŁC
	MYTRACE���g���ꍇ�ɂ͕K��#ifdef _DEBUG �` #endif �ň͂ޕK�v������D
*/
#ifdef _DEBUG
	#define MYTRACE_A DebugOutA
	#define MYTRACE_W DebugOutW
	#ifdef _UNICODE
	#define MYTRACE DebugOutW
	#else
	#define MYTRACE DebugOutA
	#endif
#else
	#define MYTRACE   Do_not_use_the_MYTRACE_function_if_release_mode
	#define MYTRACE_A Do_not_use_the_MYTRACE_A_function_if_release_mode
	#define MYTRACE_W Do_not_use_the_MYTRACE_W_function_if_release_mode
#endif

//#ifdef _DEBUG�`#endif�ň͂܂Ȃ��Ă��ǂ���
#ifdef _DEBUG
#define DBPRINT_A DebugOutA
#define DBPRINT_W DebugOutW
#else
#if (defined(_MSC_VER) && 1400 <= _MSC_VER) || (defined(__GNUC__) && 3 <= __GNUC__ )
#define DBPRINT_A(...)
#define DBPRINT_W(...)
#else
// Not support C99 variable macro
inline void DBPRINT_A( ... ){}
inline void DBPRINT_W( ... ){}
#endif
#endif

#ifdef _UNICODE
#define DBPRINT DBPRINT_W
#else
#define DBPRINT DBPRINT_A
#endif

#define DEBUG_TRACE DBPRINT


//RELEASE�łł��o�͂���� (RELEASE�ł̂ݔ�������o�O���Ď�����ړI)
#ifdef USE_RELPRINT
#define RELPRINT_A DebugOutA
#define RELPRINT_W DebugOutW

#ifdef _UNICODE
#define RELPRINT DebugOutW
#else
#define RELPRINT DebugOutA
#endif
#endif	// USE_RELPRINT


///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_DEBUG1_587B8A50_4B0A_4E5E_A638_40FB1EC301CA_H_ */



