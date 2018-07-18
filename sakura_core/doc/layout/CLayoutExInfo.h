/*
	Copyright (C) 2011, Moca

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
#ifndef SAKURA_CLAYOUTEXINFO_H_
#define SAKURA_CLAYOUTEXINFO_H_

#include "util/design_template.h"

class CLayoutColorInfo{
public:
	CLayoutColorInfo(){}
	virtual ~CLayoutColorInfo(){};
	virtual bool IsEqual(const CLayoutColorInfo*)const = 0;
};


class CLayoutExInfo
{
public:
	CLayoutExInfo() : m_colorInfo(NULL){}
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
		m_colorInfo = NULL;
		return p;
	}
private:
	CLayoutColorInfo* m_colorInfo;

private:
	DISALLOW_COPY_AND_ASSIGN(CLayoutExInfo);
};

#endif
