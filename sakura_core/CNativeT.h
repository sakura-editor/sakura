#pragma once

class CNativeW2;
class CNativeA;

#ifdef _UNICODE
typedef CNativeW2 CNativeT;
#else
typedef CNativeA CNativeT;
#endif


#include "CNativeA.h"
#include "CNativeW.h"

