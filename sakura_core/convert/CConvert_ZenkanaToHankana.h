#pragma once

#include "CConvert.h"


//!全角カナ→半角カナ
class CConvert_ZenkanaToHankana : public CConvert{
public:
	bool DoConvert(CNativeW* pcData);
};
