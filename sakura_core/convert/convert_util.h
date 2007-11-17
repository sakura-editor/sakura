//2007.10.18 kobake

#pragma once


//! �S�p�Ђ炪�ȁ��S�p�J�^�J�i (�������͕s��)
void Convert_ZenhiraToZenkana(wchar_t* pData, int nLength);

//! �S�p�J�^�J�i���S�p�Ђ炪�� (�������͕s��)
void Convert_ZenkanaToZenhira(wchar_t* pData, int nLength);

//! �S�p�p�������p�p�� (�������͕s��)
void Convert_ZenEisuu_To_HanEisuu(wchar_t* pData, int nLength);

/*!
	�S�p�J�^�J�i�����p�J�^�J�i
	���_�̕������A�������͑�����\��������B�ő��2�{�ɂȂ�B
	pDst�ɂ͂��炩���ߏ\���ȃ��������m�ۂ��Ă������ƁB
*/
void Convert_ZenKana_To_HanKana(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst);


//! ���p�p�����S�p�p�� (�������͕s��)
void Convert_HanEisuu_To_ZenEisuu(wchar_t* pData, int nLength);

/*!
	���p�J�^�J�i���S�p�J�^�J�i
	���_�̕������A�������͌���\��������B�ŏ���2����1�ɂȂ�B
	pDst�ɂ͂��炩���ߏ\���ȃ��������m�ۂ��Ă������ƁB
*/
void Convert_HankanaToZenkana(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst);
