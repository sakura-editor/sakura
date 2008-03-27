#pragma once

#include "CDocListener.h"

class CReadHelper : public CDocListenerEx{
public:
	ECallbackResult OnCheckLoad(SLoadInfo* pLoadInfo);
};
