//2007.10.18 kobake

#pragma once


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
