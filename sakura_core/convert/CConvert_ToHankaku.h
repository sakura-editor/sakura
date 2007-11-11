#pragma once

#include "CConvert.h"


//!”¼Šp‚É‚Å‚«‚é‚à‚Ì‚Í‘S•””¼Šp‚É•ÏŠ·
class CConvert_ToHankaku : public CConvert{
public:
	bool DoConvert(CNativeW* pcData);
};


enum EToHankakuMode{
	TO_KATAKANA	= 0x01, //!< ƒJƒ^ƒJƒi‚É‰e‹¿ƒAƒŠ
	TO_HIRAGANA	= 0x02, //!< ‚Ð‚ç‚ª‚È‚É‰e‹¿ƒAƒŠ
	TO_EISU		= 0x04, //!< ‰p”Žš‚É‰e‹¿ƒAƒŠ
};
