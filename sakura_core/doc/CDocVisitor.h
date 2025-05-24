/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDOCVISITOR_02993457_F17D_4B64_A914_C954D40568B1_H_
#define SAKURA_CDOCVISITOR_02993457_F17D_4B64_A914_C954D40568B1_H_
#pragma once

#include "CEol.h"

class CEditDoc;

class CDocVisitor{
public:
	CDocVisitor(CEditDoc* pcDoc) : m_pcDocRef(pcDoc) { }

	void SetAllEol(CEol cEol); //!< 改行コードを統一する
private:
	CEditDoc* m_pcDocRef;
};
#endif /* SAKURA_CDOCVISITOR_02993457_F17D_4B64_A914_C954D40568B1_H_ */
