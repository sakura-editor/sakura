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
void DebugOutW( LPCWSTR lpFmt, ...);
void DebugOutA( LPCSTR lpFmt, ...);
#endif	// _DEBUG || USE_RELPRINT

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 �f�o�b�O�p���b�Z�[�W�o��                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*
	MYTRACE�̓����[�X���[�h�ł̓R���p�C���G���[�ƂȂ�悤�ɂ��Ă���̂ŁC
	MYTRACE���g���ꍇ�ɂ͕K��#ifdef _DEBUG �` #endif �ň͂ޕK�v������D
*/
#ifdef _DEBUG
	#ifdef _UNICODE
	#define MYTRACE DebugOutW
	#else
	#define MYTRACE DebugOutA
	#endif
#else
	#define MYTRACE   Do_not_use_the_MYTRACE_function_if_release_mode
#endif

//#ifdef _DEBUG�`#endif�ň͂܂Ȃ��Ă��ǂ���
#ifdef _DEBUG
	#ifdef _UNICODE
	#define DEBUG_TRACE DebugOutW
	#else
	#define DEBUG_TRACE DebugOutA
	#endif
#elif (defined(_MSC_VER) && 1400 <= _MSC_VER) || (defined(__GNUC__) && 3 <= __GNUC__ )
	#define DEBUG_TRACE(...)
#else
	// Not support C99 variable macro
	inline void DEBUG_TRACE( ... ){}
#endif

//RELEASE�łł��o�͂���� (RELEASE�ł̂ݔ�������o�O���Ď�����ړI)
#ifdef USE_RELPRINT
	#ifdef _UNICODE
	#define RELPRINT DebugOutW
	#else
	#define RELPRINT DebugOutA
	#endif
#else
	#define RELPRINT   Do_not_define_USE_RELPRINT
#endif	// USE_RELPRINT


///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_DEBUG1_587B8A50_4B0A_4E5E_A638_40FB1EC301CA_H_ */



