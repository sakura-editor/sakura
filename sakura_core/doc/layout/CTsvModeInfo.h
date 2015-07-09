/*!	@file
	@brief TSV���[�h�Ǘ�
*/
/*
	Copyright (C) 2015, syat

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CTSVMODEINFO_BC141F5B_325B_4C02_9CC7_633B39FE482E1_H_
#define SAKURA_CTSVMODEINFO_BC141F5B_325B_4C02_9CC7_633B39FE482E1_H_

#include <vector>
#include "basis/SakuraBasis.h"

// TSV���[�h
#define TSV_MODE_NONE	0	// TSV���[�h�Ȃ�
#define TSV_MODE_TSV	1	// TSV���[�h
#define TSV_MODE_CSV	2	// CSV���[�h

class CDocLineMgr;

// TSV���[�h���
class CTsvModeInfo {
	
public:
	void CalcTabLength(CDocLineMgr* cDocLineMgr);	// �^�u�ʒu���Čv�Z����
	void CalcTabLength(LPCWSTR pLine);	// �^�u�ʒu���Čv�Z����i��s�j
	CLayoutInt GetActualTabLength(CLayoutInt pos) const;	// �w�肵�����C�A�E�g�ʒu�̃^�u�����擾�i�܂�Ԃ��͍l�����Ȃ��j

	int m_nTsvMode;

private:
	std::vector<int> m_tabLength;
};
#endif /* SAKURA_CTSVMODEINFO_BC141F5B_325B_4C02_9CC7_633B39FE482E1_H_ */
/*[EOF]*/
