#pragma once

#include "CConvert.h"


//!���p�ɂł�����̂͑S�����p�ɕϊ�
class CConvert_ToHankaku : public CConvert{
public:
	bool DoConvert(CNativeW* pcData);
};


enum EToHankakuMode{
	TO_KATAKANA	= 0x01, //!< �J�^�J�i�ɉe���A��
	TO_HIRAGANA	= 0x02, //!< �Ђ炪�Ȃɉe���A��
	TO_EISU		= 0x04, //!< �p�����ɉe���A��
};
