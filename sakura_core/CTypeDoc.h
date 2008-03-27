#pragma once

#include "CDocListener.h"

class CTypeDoc : public CDocListenerEx{
public:
	void OnBeforeLoad(const SLoadInfo& sLoadInfo);
};
