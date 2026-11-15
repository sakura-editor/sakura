/*! @file */
/*
	Copyright (C) 2011, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CLAYOUTEXINFO_735DC1F2_72C6_4B21_934B_7D9F8C234154_H_
#define SAKURA_CLAYOUTEXINFO_735DC1F2_72C6_4B21_934B_7D9F8C234154_H_
#pragma once

#include "util/design_template.h"

#include <memory>

class CLayoutColorInfo{
public:
	CLayoutColorInfo() = default;
	virtual ~CLayoutColorInfo() = default;

	virtual bool IsEqual(const CLayoutColorInfo*)const = 0;
};

class CLayoutExInfo
{
public:
	explicit CLayoutExInfo(CLayoutColorInfo* p)
		: m_colorInfo{ p }
	{
	}

	CLayoutExInfo() : CLayoutExInfo(nullptr) {}

	~CLayoutExInfo(){
		delete m_colorInfo;
	}

	void SetColorInfo(CLayoutColorInfo* p){
		if( m_colorInfo ){
			delete m_colorInfo;
		}
		m_colorInfo = p;
	}

	const CLayoutColorInfo* GetColorInfo() const{
		return m_colorInfo;
	}

	CLayoutColorInfo* DetachColorInfo(){
		CLayoutColorInfo* p = m_colorInfo;
		m_colorInfo = nullptr;
		return p;
	}

private:
	CLayoutColorInfo* m_colorInfo = nullptr;

	DISALLOW_COPY_AND_ASSIGN(CLayoutExInfo);
};

#endif /* SAKURA_CLAYOUTEXINFO_735DC1F2_72C6_4B21_934B_7D9F8C234154_H_ */
