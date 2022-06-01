﻿/*!	@file
	@brief キーボードマクロ

	CMacroのインスタンスひとつが、1コマンドになる。

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, 鬼
	Copyright (C) 2018-2022, Sakura Editor Organization

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

#ifndef SAKURA_CMACRO_E1ABA5D8_7ACF_417B_A668_A7AD9714118A_H_
#define SAKURA_CMACRO_E1ABA5D8_7ACF_417B_A668_A7AD9714118A_H_
#pragma once

#include <Windows.h>
#include <ObjIdl.h>  // VARIANT等
#include "func/Funccode.h"

class CTextOutputStream;
class CEditView;

enum EMacroParamType{
	EMacroParamTypeNull,
	EMacroParamTypeInt,
	EMacroParamTypeStr,
};
struct CMacroParam{
	WCHAR*			m_pData;
	CMacroParam*	m_pNext;
	int				m_nDataLen;
	EMacroParamType m_eType;

	CMacroParam():m_pData(NULL), m_pNext(NULL), m_nDataLen(0), m_eType(EMacroParamTypeNull){}
	CMacroParam( const CMacroParam& obj ){
		if( obj.m_pData ){
			m_pData = new WCHAR[obj.m_nDataLen + 1];
		}else{
			m_pData = NULL;
		}
		m_pNext = NULL;
		m_nDataLen = obj.m_nDataLen;
		m_eType = obj.m_eType;
	}
	CMacroParam& operator = (const CMacroParam&) = delete;
	CMacroParam(CMacroParam&&) noexcept = delete;
	CMacroParam& operator = (CMacroParam&&) noexcept = delete;
	~CMacroParam(){
		Clear();
	}
	void Clear(){
		delete[] m_pData;
		m_pData = NULL;
		m_nDataLen = 0;
		m_eType = EMacroParamTypeNull;
	}
	void SetStringParam( const WCHAR* szParam, int nLength = -1 );
	void SetIntParam( const int nParam );
};

/*! @brief キーボードマクロの1コマンド

	引数をリスト構造にして、いくつでも持てるようにしてみました。
	スタックにするのが通例なのかもしれません（よくわかりません）。
	
	今後、制御構造が入っても困らないようにしようと思ったのですが、挫折しました。
	
	さて、このクラスは次のような前提で動作している。

	@li 引数のリストを、m_pParamTopからのリスト構造で保持。
	@li 引数を新たに追加するには、AddParam()を使用する。
	  AddParamにどんな値が渡されてもよいように準備するコト。
	  渡された値が数値なのか、文字列へのポインタなのかは、m_nFuncID（機能 ID）によって、このクラス内で判別し、よろしくやること。
	@li 引数は、CMacro内部ではすべて文字列で保持すること（数値97は、"97"として保持）（いまのところ）
*/
class CMacro
{
	using Me = CMacro;

public:
	/*
	||  Constructors
	*/
	CMacro( EFunctionCode nFuncID );	//	機能IDを指定して初期化
	CMacro(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CMacro(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CMacro();
	void ClearMacroParam();

	void SetNext(CMacro* pNext){ m_pNext = pNext; }
	CMacro* GetNext(){ return m_pNext; }
	// 2007.07.20 genta : flags追加
	bool Exec( CEditView* pcEditView, int flags ) const; //2007.09.30 kobake const追加
	void Save( HINSTANCE hInstance, CTextOutputStream& out ) const; //2007.09.30 kobake const追加
	
	void AddLParam( const LPARAM* lParam, const CEditView* pcEditView  );	//@@@ 2002.2.2 YAZAKI pcEditViewも渡す
	void AddStringParam( const WCHAR* szParam, int nLength = -1 );
	void AddIntParam( const int nParam );
	int GetParamCount() const;

	static bool HandleCommand( CEditView *View, EFunctionCode ID, const WCHAR* Argument[], const int ArgLengths[], const int ArgSize );
	static bool HandleFunction( CEditView *View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result);
	//2009.10.29 syat HandleCommandとHandleFunctionの引数を少しそろえた

	/*
	||  Attributes & Operations
	*/
protected:
	static WCHAR* GetParamAt(CMacroParam*, int);

	/*
	||  実装ヘルパ関数
	*/
	EFunctionCode	m_nFuncID;		//	機能ID
	CMacroParam*	m_pParamTop;	//	パラメータ
	CMacroParam*	m_pParamBot;
	CMacro*			m_pNext;		//	次のマクロへのポインタ
};
#endif /* SAKURA_CMACRO_E1ABA5D8_7ACF_417B_A668_A7AD9714118A_H_ */
