/*!	@file
	@brief �}�N����ʊǗ�

	@author genta
	@date 2002.4.29
*/
/*
	Copyright (C) 2002, genta
	Copyright (C) 2004, genta

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

#ifndef __CMACROTYPEMGR_H_
#define __CMACROTYPEMGR_H_

#include <map>
#include <list>
#include <string>
#include "util/design_template.h"

class CMacroManagerBase;

/*!
	@brief �}�N��Handler�����N���X
	
	@par ������
	CMacroManagerBase::declare() �ɂ��CMacroEngine��Creater�̓o�^
	RegisterEngine() �y�� �Ή��g���q�̓o�^ RegisterExt() ���Ăяo�����D
	
	@par �Ăяo��
	CMacroFactory::Create()���g���q�������ɂ��ČĂяo���ƑΉ�����
	�}�N���G���W�����Ԃ����D����ꂽEngine�ɑ΂���LoadKeyMacro()�y��
	ExecKeyMacro() ���Ăяo�����ƂŃ}�N���̓ǂݍ��݁E���s���s����D

	Singleton
*/
class CMacroFactory : public TSingleton<CMacroFactory> {
public:
	friend class TSingleton<CMacroFactory>;
	typedef CMacroManagerBase* (*Creator)(const TCHAR*);

	bool RegisterCreator( Creator );
	// Jan. 31, 2004 genta
	// �o�C�i���T�C�Y�팸�̂���m_mMacroExts���폜
	//bool RegisterExt( const char*, Creator );
	bool Unregister( Creator );

	CMacroManagerBase* Create(const TCHAR* ext);

private:
	CMacroFactory();
	std::tstring Ext2Key(const TCHAR *ext);

	// Jan. 31, 2004 genta
	// �o�C�i���T�C�Y�팸�̂��ߊg���q�ێ��pmap���폜
	//	typedef std::map<std::string, Creator> MacroTypeRep;
	typedef std::list<Creator> MacroEngineRep;

	// Jan. 31, 2004 genta
	// �o�C�i���T�C�Y�팸�̂���
	//MacroTypeRep m_mMacroExts;	/*!< �g���q�Ή��\ */
	/*!
		Creator���X�g
		@date 2002.08.25 genta �ǉ�
	*/
	MacroEngineRep m_mMacroCreators;

};
#endif
