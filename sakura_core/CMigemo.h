/*!	@file
	@brief Migemo�֘A

	C/Migemo�A�N�Z�X�֐�

	@author isearch
	@date 2004.09.14 �V�K�쐬
*/
/*
	Copyright (C) 2004, isearch
	Copyright (C) 2005, aroka
	Copyright (C) 2009, miau

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
/*
Migemo �̓��[�}���̂܂ܓ��{����C���N�������^���������邽�߂̃c�[���ł��B
*/

#ifndef _SAKURA_MIGEMO_H_
#define _SAKURA_MIGEMO_H_

#define MIGEMO_VERSION "1.1"

/* for migemo_load() */
#define MIGEMO_DICTID_INVALID		0
#define MIGEMO_DICTID_MIGEMO		1
#define MIGEMO_DICTID_ROMA2HIRA		2
#define MIGEMO_DICTID_HIRA2KATA		3
#define MIGEMO_DICTID_HAN2ZEN		4

/* for migemo_set_operator()/migemo_get_operator().  see: rxgen.h */
#define MIGEMO_OPINDEX_OR		0
#define MIGEMO_OPINDEX_NEST_IN		1
#define MIGEMO_OPINDEX_NEST_OUT		2
#define MIGEMO_OPINDEX_SELECT_IN	3
#define MIGEMO_OPINDEX_SELECT_OUT	4
#define MIGEMO_OPINDEX_NEWLINE		5

/* see: rxgen.h */
typedef int (*MIGEMO_PROC_CHAR2INT)(unsigned char*, unsigned int*);
typedef int (*MIGEMO_PROC_INT2CHAR)(unsigned int, unsigned char*);

/**
 * Migemo�I�u�W�F�N�g�Bmigemo_open()�ō쐬����Amigemo_close�Ŕj�������B
 */
typedef struct _migemo migemo;


#include "CDllHandler.h"

class SAKURA_CORE_API CMigemo : public CDllImp {


public:
	CMigemo() ;
	virtual ~CMigemo();
	static CMigemo* _instance;
	static CMigemo* getInstance();

	//	Entry Point
protected:
	//	Aug. 20, 2005 Aroka : �œK���I�v�V�����Ńf�t�H���g��__fastcall�ɕύX���Ă�
	//	�e�����󂯂Ȃ��悤�ɂ���D
	typedef migemo*        (__cdecl *Proc_migemo_open)            (char* dict);
	typedef void           (__cdecl *Proc_migemo_close)           (migemo* object);
	typedef unsigned char* (__cdecl *Proc_migemo_query)           (migemo* object, unsigned char* query);
	typedef void           (__cdecl *Proc_migemo_release)         (migemo* object, unsigned char* string);
	typedef int            (__cdecl *Proc_migemo_set_operator)    (migemo* object, int index, unsigned char* op);
	typedef const uchar_t* (__cdecl *Proc_migemo_get_operator)    (migemo* object, int index);
	typedef void           (__cdecl *Proc_migemo_setproc_char2int)(migemo* object, MIGEMO_PROC_CHAR2INT proc);
	typedef void           (__cdecl *Proc_migemo_setproc_int2char)(migemo* object, MIGEMO_PROC_INT2CHAR proc);
	typedef int            (__cdecl *Proc_migemo_load)            (migemo* obj, int dict_id, const char* dict_file);
	typedef int            (__cdecl *Proc_migemo_is_enable)       (migemo* obj);
	
	Proc_migemo_open                  m_migemo_open                ;
	Proc_migemo_close                 m_migemo_close               ;
	Proc_migemo_query                 m_migemo_query               ;
	Proc_migemo_release               m_migemo_release             ;
	Proc_migemo_set_operator          m_migemo_set_operator        ;
	Proc_migemo_get_operator          m_migemo_get_operator        ;
	Proc_migemo_setproc_char2int      m_migemo_setproc_char2int    ;
	Proc_migemo_setproc_int2char      m_migemo_setproc_int2char    ;
	Proc_migemo_load                  m_migemo_load                ;
	Proc_migemo_is_enable             m_migemo_is_enable           ;

	migemo* m_migemo;
	
	LPCTSTR GetDllNameImp(int nIndex);
	bool InitDllImp();
	int DeInitDll(void);
	
public:
	long migemo_open(char* dict);
	void migemo_close();
	unsigned char* migemo_query(unsigned char* query);
	void migemo_release( unsigned char* string);
	int migemo_set_operator(int index, unsigned char* op);
	const unsigned char* migemo_get_operator(int index);
	void migemo_setproc_char2int(MIGEMO_PROC_CHAR2INT proc);
	void migemo_setproc_int2char(MIGEMO_PROC_INT2CHAR proc);
	int migemo_load_a(int dict_id, const char* dict_file);
	int migemo_load_w(int dict_id, const wchar_t* dict_file);
#ifdef _UNICODE
	#define migemo_load_t migemo_load_w
#else
	#define migemo_load_t migemo_load_a
#endif
	int migemo_is_enable();
	int migemo_load_all();

};

/*! @brief PCRE ���^�L�����N�^�̃G�X�P�[�v�������s���B
 �iCMigemo::migemo_setproc_int2char �̈����Ƃ��Ďg�p�j
 @param[in] in ���͕����R�[�h(unsigned int)
 @param[out] out �o�̓o�C�g��(unsigned char*)
 @return �o�͂��ꂽ������̃o�C�g���B
  0��Ԃ��΃f�t�H���g�̃v���V�[�W�������s�����B
*/
static int __cdecl pcre_int2char(unsigned int in, unsigned char* out);	// 2009.04.30 miau

#endif



