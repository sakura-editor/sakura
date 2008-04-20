#include "stdafx.h"
#include "CBase64.h"

/*!
	BASE64 ïœä∑ÉeÅ[ÉuÉã ÇªÇÃÇQ
*/
const uchar_t ucNA = 0xff;
const uchar_t CBase64::BASE64VAL[] = {
	ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, //00-07:
	ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, //08-0f:
	ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, //10-17:
	ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, //18-1f:
	ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, //20-27:
	ucNA, ucNA, ucNA,   62, ucNA, ucNA, ucNA,   63, //28-2f:    +   /
	  52,   53,   54,   55,   56,   57,   58,   59, //30-37: 01234567
	  60,   61, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, //38-3f: 89      
	ucNA,    0,    1,    2,    3,    4,    5,    6, //40-47:  ABCDEFG
	   7,    8,    9,   10,   11,   12,   13,   14, //48-4f: HIJKLMNO
	  15,   16,   17,   18,   19,   20,   21,   22, //50-57: PQRSTUVW
	  23,   24,   25, ucNA, ucNA, ucNA, ucNA, ucNA, //58-5f: XYZ     
	   0,   26,   27,   28,   29,   30,   31,   32, //60-67: `abcdefg
	  33,   34,   35,   36,   37,   38,   39,   40, //68-6f: hijklmno
	  41,   42,   43,   44,   45,   46,   47,   48, //70-77: pqrstuvw
	  49,   50,   51, ucNA, ucNA, ucNA, ucNA, ucNA, //78-7f: xyz
};
