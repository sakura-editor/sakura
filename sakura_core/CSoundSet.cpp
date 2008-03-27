#include "stdafx.h"
#include "CSoundSet.h"



void CSoundSet::NeedlessToSaveBeep()
{
	if( m_nMuteCount>=1 )return;
	ErrorBeep();
}
