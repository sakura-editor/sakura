#pragma once

#include "CConvert.h"


//!半角カナ→全角カナ
class CConvert_HankanaToZenkana : public CConvert{
public:
	bool DoConvert(CNativeW* pcData);
};
