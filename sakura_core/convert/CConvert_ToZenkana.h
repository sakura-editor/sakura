#pragma once

#include "CConvert.h"


//!できる限り全角カタカナにする
class CConvert_ToZenkana : public CConvert{
public:
	bool DoConvert(CNativeW* pcData);
};
