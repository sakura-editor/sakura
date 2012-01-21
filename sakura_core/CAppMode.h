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
#ifndef SAKURA_CAPPMODE_22018B3D_FC0F_485B_A72E_DA3DA3B9DDAAR_H_
#define SAKURA_CAPPMODE_22018B3D_FC0F_485B_A72E_DA3DA3B9DDAAR_H_

#include "util/design_template.h"
#include "doc/CDocListener.h"

class CAppMode : public TSingleton<CAppMode>, public CDocListenerEx{ //###��
public:
	CAppMode()
	: m_bViewMode( false )	// �r���[���[�h
	, m_bDebugMode( false )		// �f�o�b�O���j�^���[�h
	{
		wcscpy( m_szGrepKey, L"" );
	}

	//�C���^�[�t�F�[�X
	bool	IsViewMode() const				{ return m_bViewMode; }			//!< �r���[���[�h���擾
	void	SetViewMode(bool bViewMode)		{ m_bViewMode = bViewMode; }	//!< �r���[���[�h��ݒ�
	bool	IsDebugMode() const				{ return m_bDebugMode; }
	void	SetDebugModeON();	//!< �f�o�b�O���j�^���[�h�ݒ�
	void	SetDebugModeOFF();	//!< �f�o�b�O���j�^���[�h����

	//�C�x���g
	void OnAfterSave(const SSaveInfo& sSaveInfo);

protected:
	void _SetDebugMode(bool bDebugMode){ m_bDebugMode = bDebugMode; }

private:
	bool			m_bViewMode;			//!< �r���[���[�h
	bool			m_bDebugMode;				//!< �f�o�b�O���j�^���[�h
public:
	wchar_t			m_szGrepKey[1024];			//!< Grep���[�h�̏ꍇ�A���̌����L�[
};

#endif /* SAKURA_CAPPMODE_22018B3D_FC0F_485B_A72E_DA3DA3B9DDAAR_H_ */
/*[EOF]*/
