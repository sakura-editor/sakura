//2007.10.19 kobake

#pragma once

enum maxdata{
	MAX_EDITWINDOWS				= 256,
	MAX_SEARCHKEY				=  30,
	MAX_REPLACEKEY				=  30,
	MAX_GREPFILE				=  30,
	MAX_GREPFOLDER				=  30,
	MAX_TYPES					=  30,	//Jul. 12, 2001 JEPRO �^�C�v�ʐݒ�̍ő�ݒ萔��16���瑝�₵��
	MAX_PRINTSETTINGARR			=   8,

	//	From Here Sep. 14, 2001 genta
	MACRONAME_MAX				= 64,
	MAX_EXTCMDLEN				= 1024,
	MAX_EXTCMDMRUNUM			= 32,

	MAX_CMDLEN					= 1024,
	MAX_CMDARR					= 32,
	MAX_REGEX_KEYWORD			= 100,	//@@@ 2001.11.17 add MIK

	MAX_KEYHELP_FILE			= 20,	//@@@ 2006.04.10 fon

	MAX_MARKLINES_LEN			= 1023,	// 2002.01.18 hor
	MAX_DOCTYPE_LEN				= 7,
	MAX_TRANSFORM_FILENAME		= 16,	/// 2002.11.24 Moca

	/*! �o�^�ł���}�N���̐�
		@date 2005.01.30 genta 50�ɑ��₵��
	*/
	MAX_CUSTMACRO				= 50,

	// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
	MAX_TAGJUMPNUM				= 100,	// �^�u�W�����v���ő�l
	MAX_TAGJUMP_KEYWORD			= 30,	//�^�O�W�����v�p�L�[���[�h�ő�o�^��2005.04.04 MIK
	MAX_KEYWORDSET_PER_TYPE		= 10,	// 2004.01.23 genta (for MIK) �^�C�v�ʐݒ薈�̃L�[���[�h�Z�b�g��
	MAX_VERTLINES = 10,	// 2005.11.08 Moca �w�茅�c��

	//	MRU���X�g�Ɋ֌W����maxdata
	MAX_MRU						=  36,	//Sept. 27, 2000 JEPRO 0-9, A-Z ��36�ɂȂ�̂ł���ɍ��킹��30��36�ɕύX�B2007.10.23 kobake maxdata�Ɉړ��B
	MAX_OPENFOLDER				=  36,	//Sept. 27, 2000 JEPRO 0-9, A-Z ��36�ɂȂ�̂ł���ɍ��킹��30��36�ɕύX

	MAX_PLUGIN					= 20,	// �o�^�ł���v���O�C���̐�
	MAX_PLUGIN_ID				= 63+1,	// �v���O�C��ID�̍ő咷��
	MAX_PLUGIN_NAME				= 63+1,	// �v���O�C�����̍ő咷��
};




