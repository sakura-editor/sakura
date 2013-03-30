/*!	@file
	@brief �v���Z�X�����N���X�w�b�_�t�@�C��

	@author aroka
	@date	2002/01/08 �쐬
*/
/*
	Copyright (C) 2002, aroka �V�K�쐬
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CPROCESSFACTORY_H_
#define _CPROCESSFACTORY_H_

#include "global.h"

class CProcess;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �v���Z�X�����N���X

	�^����ꂽ�R�}���h���C���������琶�����ׂ��v���Z�X�̎�ʂ𔻒肵�C
	�Ή�����I�u�W�F�N�g��Ԃ�Factory�N���X�D

	�ʏ�̃G�f�B�^�v���Z�X�̋N�����w�肳�ꂽ�ꍇ�ɂ́C�K�v�ɉ����ăR���g���[���v���Z�X
	�N���̋N�����G�f�B�^�̋N���ɐ旧���čs���D
*/
class CProcessFactory {
public:
	CProcess* Create( HINSTANCE hInstance, LPCTSTR lpCmdLine );
protected:
private:
	bool IsValidVersion();
	bool IsStartingControlProcess();
	bool IsExistControlProcess();
	bool StartControlProcess();
	bool WaitForInitializedControlProcess();	// 2006.04.10 ryoji �R���g���[���v���Z�X�̏����������C�x���g��҂�
	bool TestWriteQuit();	// 2007.09.04 ryoji �u�ݒ��ۑ����ďI������v�I�v�V���������isakuext�A�g�p�j
};

///////////////////////////////////////////////////////////////////////
#endif /* _CPROCESSFACTORY_H_ */


