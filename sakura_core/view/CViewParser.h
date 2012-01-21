/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CVIEWPARSER_7EE1449B_4B78_4C60_BE19_6498574626729_H_
#define SAKURA_CVIEWPARSER_7EE1449B_4B78_4C60_BE19_6498574626729_H_

class CEditView;

//!�i����̓N���X
class CViewParser{
public:
	CViewParser(const CEditView* pEditView) : m_pEditView(pEditView) { }
	virtual ~CViewParser(){}

	//! �J�[�\�����O�̒P����擾
	int GetLeftWord( CNativeW* pcmemWord, int nMaxWordLen ) const;

	//! �L�����b�g�ʒu�̒P����擾
	// 2006.03.24 fon
	BOOL GetCurrentWord( CNativeW* pcmemWord ) const;

private:
	const CEditView* m_pEditView;
};

#endif /* SAKURA_CVIEWPARSER_7EE1449B_4B78_4C60_BE19_6498574626729_H_ */
/*[EOF]*/
