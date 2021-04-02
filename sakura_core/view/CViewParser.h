﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_CVIEWPARSER_440F895C_409D_4ACF_906E_1DD7A0C0C2C0_H_
#define SAKURA_CVIEWPARSER_440F895C_409D_4ACF_906E_1DD7A0C0C2C0_H_
#pragma once

class CEditView;
class CNativeW;

//!品詞解析クラス
class CViewParser{
public:
	CViewParser(const CEditView* pEditView) : m_pEditView(pEditView) { }
	virtual ~CViewParser(){}

	//! カーソル直前の単語を取得
	int GetLeftWord( CNativeW* pcmemWord, int nMaxWordLen ) const;

	//! キャレット位置の単語を取得
	// 2006.03.24 fon
	bool GetCurrentWord( CNativeW* pcmemWord ) const;

private:
	const CEditView* m_pEditView;
};
#endif /* SAKURA_CVIEWPARSER_440F895C_409D_4ACF_906E_1DD7A0C0C2C0_H_ */
