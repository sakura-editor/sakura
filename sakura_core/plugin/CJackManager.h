/*!	@file
	@brief �W���b�N�Ǘ��N���X

*/
/*
	Copyright (C) 2009, syat

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
#ifndef SAKURA_CJACKMANAGER_6CC7B212_130B_46AF_9C88_05F554CDA34BO_H_
#define SAKURA_CJACKMANAGER_6CC7B212_130B_46AF_9C88_05F554CDA34BO_H_

#include "plugin/CPlugin.h"
#include <list>

#define PP_COMMAND_STR	L"Command"

// �W���b�N�i���v���O�C���\�ӏ��j
enum EJack {
	PP_NONE			= -1,
	PP_COMMAND		= 0,
	PP_INSTALL,
	PP_UNINSTALL,
	PP_APP_START,
	PP_APP_END,
	PP_EDITOR_START,
	PP_EDITOR_END,
	PP_OUTLINE,
	PP_SMARTINDENT,

	//���W���b�N��ǉ�����Ƃ��͂��̍s�̏�ɁB
	PP_BUILTIN_JACK_COUNT	//�g�ݍ��݃W���b�N��
};

// �W���b�N��`�\����
typedef struct tagJackDef {
	EJack			ppId;
	const WCHAR*	szName;
	CPlug::Array		plugs;	//�W���b�N�Ɋ֘A�t����ꂽ�v���O
} JackDef;

// �v���O�o�^����
enum ERegisterPlugResult {
	PPMGR_REG_OK,				//�v���O�C���o�^����
	PPMGR_INVALID_NAME,			//�W���b�N�����s��
	PPMGR_CONFLICT				//�w�肵���W���b�N�͕ʂ̃v���O�C�����ڑ����Ă���
};


//�W���b�N�Ǘ��N���X
class CJackManager : public TSingleton<CJackManager>{
	typedef std::wstring wstring;

public:
	friend class TSingleton<CJackManager>;

	//�R���X�g���N�^
protected:
	CJackManager();

	//����
public:
	ERegisterPlugResult RegisterPlug( wstring pszJack, CPlug* plug );	//�v���O���W���b�N�Ɋ֘A�t����
	bool UnRegisterPlug( wstring pszJack, CPlug* plug );	//�v���O�̊֘A�t������������
	bool GetUsablePlug( EJack jack, PlugId plugId, CPlug::Array* plugs );	//���p�\�ȃv���O����������
private:
	EJack GetJackFromName( wstring sName );	//�W���b�N�����W���b�N�ԍ��ɕϊ�����

	//����
public:
	std::vector<JackDef> GetJackDef() const;	//�W���b�N��`�ꗗ��Ԃ�
	EFunctionCode GetCommandCode( int index ) const;		//�v���O�C���R�}���h�̋@�\�R�[�h��Ԃ�
	int GetCommandName( int funccode, WCHAR* buf, int size ) const;	//�v���O�C���R�}���h�̖��O��Ԃ�
	int GetCommandCount() const;	//�v���O�C���R�}���h�̐���Ԃ�
	CPlug* GetCommandById( int id ) const;	//ID�ɍ��v����R�}���h�v���O��Ԃ�
	const CPlug::Array& GetPlugs( EJack jack ) const;	//�v���O��Ԃ�
	//TODO: ��肪��т��ĂȂ��̂Ő������� syat

	//�����o�ϐ�
private:
	DLLSHAREDATA* m_pShareData;
	std::vector<JackDef> m_Jacks;	//�W���b�N��`�\
};

#endif /* SAKURA_CJACKMANAGER_6CC7B212_130B_46AF_9C88_05F554CDA34BO_H_ */
/*[EOF]*/
