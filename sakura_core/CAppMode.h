#pragma once

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
	void OnAfterSave(const SSaveInfo& sSaveInfo)
	{
		m_bViewMode = false;	/* �r���[���[�h */
	}

protected:
	void _SetDebugMode(bool bDebugMode){ m_bDebugMode = bDebugMode; }

private:
	bool			m_bViewMode;			//!< �r���[���[�h
	bool			m_bDebugMode;				//!< �f�o�b�O���j�^���[�h
public:
	wchar_t			m_szGrepKey[1024];			//!< Grep���[�h�̏ꍇ�A���̌����L�[
};
