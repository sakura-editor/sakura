//�r���h(�R���p�C��)�ݒ�
//2007.10.18 kobake �쐬

#pragma once

/*!
	���i��int���g�����ǂ����B

	��ɃG�f�B�^�����̍��W�n�P�ʂɊւ���
	�R���p�C�����ɐÓI�Ȍ^�`�F�b�N�������悤�ɂȂ�܂��B
	���������̕��R���p�C�����Ԃ�������܂��B

	���s�������͕ω������B
	���s���I�[�o�[�w�b�h�s���B�R���p�C����������΃I�[�o�[�w�b�h�[���B

	�����[�X�r���h�ł͖����ɂ��Ă����Ɨǂ��B

	@date 2007.10.18 kobake
*/
#ifdef _DEBUG
#define USE_STRICT_INT //��������R�����g�A�E�g����ƌ��i��int�������ɂȂ�܂��B�����[�X�r���h�ł͏�ɖ����B
#endif


//! USE_UNFIXED_FONT ���`����ƁA�t�H���g�I���_�C�A���O�œ����t�H���g�ȊO���I�ׂ�悤�ɂȂ�
//#define USE_UNFIXED_FONT


//UNICODE BOOL�萔
#ifdef _UNICODE
static const bool UNICODE_BOOL=true;
#else
static const bool UNICODE_BOOL=false;
#endif


//DebugMonitorLib(��)���g�����ǂ���
//#define USE_DEBUGMON
