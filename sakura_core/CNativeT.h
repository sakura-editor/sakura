#pragma once

class CNativeW;
class CNativeA;

#ifdef _UNICODE
typedef CNativeW CNativeT;
#else
typedef CNativeA CNativeT;
#endif


#include "CNativeA.h"
#include "CNativeW.h"

