/*!	@file
	@brief Migemo関連

	C/Migemoアクセス関数

	@author isearch
	@date 2004.09.14 新規作成
*/
/*
	Copyright (C) 2004, isearch
	Copyright (C) 2005, aroka
	Copyright (C) 2009, miau

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
/*
Migemo はローマ字のまま日本語をインクリメンタル検索するためのツールです。
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
#define MIGEMO_DICTID_ZEN2HAN		5

/* for migemo_set_operator()/migemo_get_operator().  see: rxgen.h */
#define MIGEMO_OPINDEX_OR		0
#define MIGEMO_OPINDEX_NEST_IN		1
#define MIGEMO_OPINDEX_NEST_OUT		2
#define MIGEMO_OPINDEX_SELECT_IN	3
#define MIGEMO_OPINDEX_SELECT_OUT	4
#define MIGEMO_OPINDEX_NEWLINE		5

/* see: rxgen.h */
typedef int (__cdecl *MIGEMO_PROC_CHAR2INT)(const unsigned char*, unsigned int*);
typedef int (__cdecl *MIGEMO_PROC_INT2CHAR)(unsigned int, unsigned char*);

/**
 * Migemoオブジェクト。migemo_open()で作成され、migemo_closeで破棄される。
 */
typedef struct _migemo migemo;


#include "CDllHandler.h"
#include "util/design_template.h"

class CMigemo : public TSingleton<CMigemo>, public CDllImp {
	friend class TSingleton<CMigemo>;
	CMigemo(){}

public:
	virtual ~CMigemo();

	//	Entry Point
protected:
	//	Aug. 20, 2005 Aroka : 最適化オプションでデフォルトを__fastcallに変更しても
	//	影響を受けないようにする．
	typedef migemo*        (__cdecl *Proc_migemo_open)            (char* dict);
	typedef void           (__cdecl *Proc_migemo_close)           (migemo* object);
	typedef unsigned char* (__cdecl *Proc_migemo_query)           (migemo* object, unsigned char* query);
	typedef void           (__cdecl *Proc_migemo_release)         (migemo* object, unsigned char* str);
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

	typedef migemo*        (__stdcall *Proc_migemo_open_s)            (char* dict);
	typedef void           (__stdcall *Proc_migemo_close_s)           (migemo* object);
	typedef unsigned char* (__stdcall *Proc_migemo_query_s)           (migemo* object, unsigned char* query);
	typedef void           (__stdcall *Proc_migemo_release_s)         (migemo* object, unsigned char* str);
	typedef int            (__stdcall *Proc_migemo_set_operator_s)    (migemo* object, int index, unsigned char* op);
	typedef const uchar_t* (__stdcall *Proc_migemo_get_operator_s)    (migemo* object, int index);
	typedef void           (__stdcall *Proc_migemo_setproc_char2int_s)(migemo* object, MIGEMO_PROC_CHAR2INT proc);
	typedef void           (__stdcall *Proc_migemo_setproc_int2char_s)(migemo* object, MIGEMO_PROC_INT2CHAR proc);
	typedef int            (__stdcall *Proc_migemo_load_s)            (migemo* obj, int dict_id, const char* dict_file);
	typedef int            (__stdcall *Proc_migemo_is_enable_s)       (migemo* obj);

	Proc_migemo_open_s                m_migemo_open_s;
	Proc_migemo_close_s               m_migemo_close_s;
	Proc_migemo_query_s               m_migemo_query_s;
	Proc_migemo_release_s             m_migemo_release_s;
	Proc_migemo_set_operator_s        m_migemo_set_operator_s;
	Proc_migemo_get_operator_s        m_migemo_get_operator_s;
	Proc_migemo_setproc_char2int_s    m_migemo_setproc_char2int_s;
	Proc_migemo_setproc_int2char_s    m_migemo_setproc_int2char_s;
	Proc_migemo_load_s                m_migemo_load_s;
	Proc_migemo_is_enable_s           m_migemo_is_enable_s;

	migemo* m_migemo;
	// IA64/x64は対応不要
#ifdef _WIN64
	static const bool	m_bStdcall = true;
#else
	bool	m_bStdcall;
#endif
	bool	m_bUtf8;

	LPCTSTR GetDllNameImp(int nIndex);
	bool InitDllImp();
	bool DeinitDllImp(void);

public:
	long migemo_open(char* dict);
	void migemo_close();
	unsigned char* migemo_query(unsigned char* query);
	std::wstring migemo_query_w(const wchar_t* query);
	void migemo_release( unsigned char* str);
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


#endif



