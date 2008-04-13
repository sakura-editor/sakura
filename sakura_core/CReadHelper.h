#pragma once

#include "doc/CDocListener.h"

class CReadHelper : public CDocListenerEx{
public:
	ECallbackResult OnCheckLoad(SLoadInfo* pLoadInfo);
};
