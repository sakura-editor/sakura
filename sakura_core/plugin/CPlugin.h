/*!	@file
	@brief �v���O�C����{�N���X

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
#ifndef SAKURA_CPLUGIN_E837BF6E_3F18_4A7E_89FD_F4DAE8DF9CFFD_H_
#define SAKURA_CPLUGIN_E837BF6E_3F18_4A7E_89FD_F4DAE8DF9CFFD_H_

#include <list>
#include <map>
#include "macro/CWSHIfObj.h"


//�v���O�C����`�t�@�C����
#define PII_FILENAME				_T("plugin.def")
//�I�v�V�����t�@�C���g���q�i�I�v�V�����t�@�C�����ʃt�H���_���{�g���q�j
#define PII_OPTFILEEXT				_T(".ini")

//�v���O�C����`�t�@�C���E�L�[������
#define	PII_PLUGIN					L"Plugin"		//���ʏ��
#define	PII_PLUGIN_ID				L"Id"			//ID�F�v���O�C��ID
#define	PII_PLUGIN_NAME				L"Name"			//���O�F�v���O�C����
#define	PII_PLUGIN_DESCRIPTION		L"Description"	//�����F�Ȍ��Ȑ���
#define	PII_PLUGIN_PLUGTYPE			L"Type"			//��ʁFwsh / dll
#define	PII_PLUGIN_AUTHOR			L"Author"		//��ҁF���쌠�Җ�
#define	PII_PLUGIN_VERSION			L"Version"		//�o�[�W�����F�v���O�C���̃o�[�W����
#define	PII_PLUGIN_URL				L"Url"			//�z�zURL�F�z�z��URL

#define PII_PLUG					L"Plug"			//�v���O���

#define PII_COMMAND					L"Command"		//�R�}���h���


class CPlugin;

//�v���O�i�v���O�C�����̏����P�ʁj�N���X
typedef int PlugId;

class CPlug
{
	//�^��`
protected:
	typedef std::wstring wstring;
public:
	typedef std::vector<CPlug*> List;		//�v���O�̃��X�g
	typedef List::const_iterator ListIter;	//���̃C�e���[�^

	//�R���X�g���N�^
public:
	CPlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel )
		: m_id( id )
		, m_sJack( sJack )
		, m_sHandler( sHandler )
		, m_cPlugin( plugin )
		, m_sLabel( sLabel )
	{
	}
	//�f�X�g���N�^
public:
	virtual ~CPlug() {}

	//����
public:
	bool Invoke( CEditView* view, CWSHIfObj::List& params );	//�v���O�����s����

	//����
public:
	EFunctionCode GetFunctionCode() const;

	//�����o�ϐ�
public:
	const PlugId m_id;					//�v���OID
	const wstring m_sJack;				//�֘A�t����W���b�N��
	const wstring m_sHandler;			//�n���h��������i�֐����j
	const wstring m_sLabel;				//���x��������
	wstring m_sIcon;					//�A�C�R���̃t�@�C���p�X
	CPlugin& m_cPlugin;					//�e�v���O�C��
};

//�v���O�C���N���X
typedef int PluginId;

class CPlugin
{
	//�^��`
protected:
	typedef std::wstring wstring;
	typedef std::basic_string<TCHAR> tstring;
public:
	typedef std::list<CPlugin*> List;		//�v���O�C���̃��X�g
	typedef List::const_iterator ListIter;	//���̃C�e���[�^

	//�R���X�g���N�^
public:
	CPlugin( tstring sBaseDir );

	//�f�X�g���N�^
public:
	virtual ~CPlugin(void);

	//����
public:
	tstring GetFilePath( const tstring& sFileName ) const;				//�v���O�C���t�H���_��̑��΃p�X���t���p�X�ɕϊ�
	virtual int AddCommand( const WCHAR* handler, const WCHAR* label, const WCHAR* icon, bool doRegister );//�R�}���h��ǉ�����
protected:
	bool ReadPluginDefCommon( CDataProfile *cProfile );					//�v���O�C����`�t�@�C����Common�Z�N�V������ǂݍ���
	bool ReadPluginDefPlug( CDataProfile *cProfile );					//�v���O�C����`�t�@�C����Plug�Z�N�V������ǂݍ���
	bool ReadPluginDefCommand( CDataProfile *cProfile );				//�v���O�C����`�t�@�C����Command�Z�N�V������ǂݍ���

	//CPlug�C���X�^���X�̍쐬�BReadPluginDefPlug/Command ����Ă΂��B
	virtual CPlug* CreatePlug( CPlugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel )
	{
		return new CPlug( plugin, id, sJack, sHandler, sLabel );
	}

//	void NormalizeExtList( const wstring& sExtList, wstring& sOut );	//�J���}��؂�g���q���X�g�𐳋K������

	//����
public:
	tstring GetPluginDefPath() const{ return GetFilePath( PII_FILENAME ); }	//�v���O�C����`�t�@�C���̃p�X
	tstring GetOptionPath() const{ return m_sBaseDir + PII_OPTFILEEXT; }	//�I�v�V�����t�@�C���̃p�X
	virtual CPlug::List GetPlugs() const = 0;								//�v���O�̈ꗗ

	//�����o�ϐ�
public:
	PluginId m_id;
	wstring m_sId;
	wstring m_sName;
	wstring m_sDescription;
	wstring m_sAuthor;
	wstring m_sVersion;
	wstring m_sUrl;
	tstring m_sBaseDir;
private:
	bool m_bLoaded;
protected:
	CPlug::List m_plugs;
	int m_nCommandCount;

	//�������
public:
	virtual bool InvokePlug( CEditView* view, CPlug& plug, CWSHIfObj::List& param ) =0;	//�v���O�����s����
	virtual bool ReadPluginDef( CDataProfile *cProfile ) =0;		//�v���O�C����`�t�@�C����ǂݍ���
	virtual bool ReadPluginOption( CDataProfile *cProfile ) =0;		//�I�v�V�����t�@�C����ǂݍ���
};

#endif /* SAKURA_CPLUGIN_E837BF6E_3F18_4A7E_89FD_F4DAE8DF9CFFD_H_ */
/*[EOF]*/
