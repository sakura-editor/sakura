/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_CONVERT_UTIL_0B07D622_2EDB_4687_A655_830B20F2BD22_H_
#define SAKURA_CONVERT_UTIL_0B07D622_2EDB_4687_A655_830B20F2BD22_H_

//2007.10.18 kobake

//! �S�p�Ђ炪�ȁ��S�p�J�^�J�i (�������͕s��)
void Convert_ZenhiraToZenkata(wchar_t* pData, int nLength);

//! �S�p�J�^�J�i���S�p�Ђ炪�� (�������͕s��)
void Convert_ZenkataToZenhira(wchar_t* pData, int nLength);

//! �S�p�p�������p�p�� (�������͕s��)
void Convert_ZeneisuToHaneisu(wchar_t* pData, int nLength);

/*!
	�S�p�J�^�J�i�����p�J�^�J�i
	���_�̕������A�������͑�����\��������B�ő��2�{�ɂȂ�B
	pDst�ɂ͂��炩���ߏ\���ȃ��������m�ۂ��Ă������ƁB
*/
void Convert_ZenkataToHankata(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength);

/*!
	�S�p�����p
	���_�̕������A�������͑�����\��������B�ő��2�{�ɂȂ�B
	pDst�ɂ͂��炩���ߏ\���ȃ��������m�ۂ��Ă������ƁB
*/
void Convert_ToHankaku(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength);


//! ���p�p�����S�p�p�� (�������͕s��)
void Convert_HaneisuToZeneisu(wchar_t* pData, int nLength);

/*!
	���p�J�^�J�i���S�p�J�^�J�i
	���_�̕������A�������͌���\��������B�ŏ���2����1�ɂȂ�B
	pDst�ɂ͂��炩���ߏ\���ȃ��������m�ۂ��Ă������ƁB
*/
void Convert_HankataToZenkata(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength);

/*!
	���p�J�^�J�i���S�p�Ђ炪��
	���_�̕������A�������͌���\��������B�ŏ���2����1�ɂȂ�B
	pDst�ɂ͂��炩���ߏ\���ȃ��������m�ۂ��Ă������ƁB
*/
void Convert_HankataToZenhira(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength);

#endif /* SAKURA_CONVERT_UTIL_0B07D622_2EDB_4687_A655_830B20F2BD22_H_ */
/*[EOF]*/
