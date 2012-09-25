#include "StdAfx.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �ϊ��e�[�u��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�S�p�J�i
static const wchar_t tableZenkata_Normal[]=
	L"�A�C�E�G�I�J�L�N�P�R�T�V�X�Z�\�^�`�c�e�g�i�j�k�l�m"	//����
	L"�n�q�t�w�z�}�~��������������������������������"		//      ���u���v�u���v�́u�C�v�u�G�v�ɕϊ��B(ANSI�ł𓥏P) 2012.06.09 syat
	L"�@�B�D�F�H�b������" L"\u30ee\u30f5\u30f6"				//��    ���㔼3�����́u���v�u���v�u���v
;
static const wchar_t tableZenkata_Dakuten[]=
	L"���K�M�O�Q�S�U�W�Y�[�]�_�a�d�f�h"						//���_
	L"�o�r�u�x�{" L"\u30f7\u30f8\u30f9\u30fa"				//���㔼4�����́u���J�v�u���J�v�u���J�v�u���J�v
;
static const wchar_t tableZenkata_HanDakuten[]=
	L"�p�s�v�y�|"											//�����_
;
static const wchar_t tableZenkata_DakuCho[]=
	L"�[�J�K"	L"\u3099\u309A"								//���_�E�����_�E����   ���㔼2�����͌��������̑��_�E�����_
;															//���S�p�J�i�����p�J�i�ϊ��ŁA�O�̕������������ǂ����`�F�b�N����
static const wchar_t tableZenkata_Kigo[]=
	L"�B�A�u�v�E"											//�L��
;

//���p�J�i
static const wchar_t tableHankata_Normal[]=
	L"�������������������������"
	L"������������������ܲ���"
	L"���������ܶ�"
;
static const wchar_t tableHankata_Dakuten[]=
	L"����������������"
	L"�����" L"ܲ��"
;
static const wchar_t tableHankata_HanDakuten[]=
	L"�����"
;
static const wchar_t tableHankata_DakuCho[] =
	L"�����"
;
static const wchar_t tableHankata_Kigo[] =
	L"�����"
;

//�S�p�p�L���B�����̕��тɐ[���Ӗ��͂���܂���B�o�b�N�X���b�V���͖����B
static const wchar_t tableZenKigo[] =
	L"�@�C�D"
	L"�{�|���^�����b��"
	L"�O�����G�F"
	L"�h�e�f�����i�j�o�p�m�n"
	L"�I�H�����P�Q"
;

//���p�p�L��
static const wchar_t tableHanKigo[] =
	L" ,."
	L"+-*/%=|&"
	L"^\\@;:"
	L"\"`'<>(){}[]"
	L"!?#$~_"
;

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	wcschr �̌��ʂ��C���f�b�N�X�Ŏ󂯎��ŁB
	c �����������Ƃ��́A���̈ʒu�� idx �Ɋi�[���Atrue ��Ԃ��B
	������Ȃ��Ƃ��� idx ��ύX������ false ��Ԃ��B
*/
bool wcschr_idx(const wchar_t* str, wchar_t c, int* idx)
{
	const wchar_t* p=wcschr(str,c);
	if(p)*idx=(int)(p-str);
	return p!=NULL;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �ϊ�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



//! �S�p�Ђ炪�ȁ��S�p�J�^�J�i (�������͕s��)
inline wchar_t ZenhiraToZenkata_(wchar_t c){ return ((c>=L'��' && c<=L'\u3096') || (c>=L'�T' && c<=L'�U'))? L'�@'+(c-L'��'): c; }
void Convert_ZenhiraToZenkata(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	while(p<q){
		*p = ZenhiraToZenkata_(*p);
		p++;
	}
}

//! �S�p�J�^�J�i���S�p�Ђ炪�� (�������͕s��)
// 2012.06.17 syat �u���v�u���v���u���v�u���v�ɕϊ����Ȃ�
inline wchar_t ZenkataToZenhira_(wchar_t c){ return ((c>=L'�@' && c<=L'��') || (c>=L'�R' && c<=L'�S'))? L'��'+(c-L'�@'): c; }
void Convert_ZenkataToZenhira(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	while(p<q){
		*p = ZenkataToZenhira_(*p);
		p++;
	}
}


//! �S�p�p�������p�p�� (�������͕s��)
inline wchar_t ZeneisuToHaneisu_(wchar_t c)
{
	int n;

	if(0){}
	else if(c>=L'�`' && c<=L'�y'){ c=L'A'+(c-L'�`'); }
	else if(c>=L'��' && c<=L'��'){ c=L'a'+(c-L'��'); }
	else if(c>=L'�O' && c<=L'�X'){ c=L'0'+(c-L'�O'); }
	//�ꕔ�̋L�����ϊ�����
	else if(wcschr_idx(tableZenKigo,c,&n)){ c=tableHanKigo[n]; }

	return c;
}
void Convert_ZeneisuToHaneisu(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	while(p<q){
		*p = ZeneisuToHaneisu_(*p);
		p++;
	}
}


//! ���p�p�����S�p�p�� (�������͕s��)
inline wchar_t HaneisuToZeneisu_(wchar_t c)
{
	int n;

	if(0){}
	else if(c>=L'A' && c<=L'Z'){ c=L'�`'+(c-L'A'); }
	else if(c>=L'a' && c<=L'z'){ c=L'��'+(c-L'a'); }
	else if(c>=L'0' && c<=L'9'){ c=L'�O'+(c-L'0'); }
	//�ꕔ�̋L�����ϊ�����
	else if(wcschr_idx(tableHanKigo,c,&n)){ c=tableZenKigo[n]; }

	return c;
}
void Convert_HaneisuToZeneisu(wchar_t* pData, int nLength)
{
	wchar_t* p=pData;
	wchar_t* q=p+nLength;
	while(p<q){
		*p = HaneisuToZeneisu_(*p);
		p++;
	}
}


/*!
	�S�p�J�^�J�i�����p�J�^�J�i
	���_�̕������A�������͑�����\��������B�ő��2�{�ɂȂ�B
	pDst�ɂ͂��炩���ߏ\���ȃ��������m�ۂ��Ă������ƁB
*/
void Convert_ZenkataToHankata(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength)
{
	const wchar_t* src = pSrc;
	const wchar_t* src_end = src + nSrcLength;
	wchar_t* dst = pDst;
	int n;
	bool bInHiraKata = false;				// �O�̕������J�^�J�ior�Ђ炪�Ȃ������Ȃ�Atrue�Ƃ��A�����A���_�A�����_�𔼊p�֕ϊ��\�Ƃ���

	while(src<src_end){
		wchar_t c=*src;
		if(0){}
		//�q�b�g���镶��������Εϊ����s��
		else if(wcschr_idx(tableZenkata_Normal    ,c,&n)){ *dst++=tableHankata_Normal[n]; bInHiraKata = true; }
		else if(wcschr_idx(tableZenkata_Dakuten   ,c,&n)){ *dst++=tableHankata_Dakuten[n];    *dst++=L'�'; bInHiraKata = false; }
		else if(wcschr_idx(tableZenkata_HanDakuten,c,&n)){ *dst++=tableHankata_HanDakuten[n]; *dst++=L'�'; bInHiraKata = false; }
		else if(wcschr_idx(tableZenkata_DakuCho   ,c,&n)){ *dst++=(bInHiraKata ? tableHankata_DakuCho[n] : c); bInHiraKata = false; }
		else if(wcschr_idx(tableZenkata_Kigo      ,c,&n)){ *dst++=tableHankata_Kigo[n]; bInHiraKata = false; }
		//���ϊ�
		else { *dst++=c; bInHiraKata = false; }
		src++;
	}
	*dst=L'\0';
	*nDstLength = dst - pDst;
}

/*!
	�S�p�����p
	���_�̕������A�������͑�����\��������B�ő��2�{�ɂȂ�B
	pDst�ɂ͂��炩���ߏ\���ȃ��������m�ۂ��Ă������ƁB
*/
void Convert_ToHankaku(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength)
{
	const wchar_t* src = pSrc;
	const wchar_t* src_end = src + nSrcLength;
	wchar_t* dst = pDst;
	int n;

	while(src<src_end){
		wchar_t c=*src;
		//�S�p�p���𔼊p�p���ɕϊ�����
		wchar_t d = ZeneisuToHaneisu_(c);
		if(d != c){ *dst++ = d; }
		else {
			//�������u�T�v�u�U�v�͑S�p�J�^�J�i�i�u�R�v�u�S�v�j�ɂ͕ϊ��ł��Ă����p�J�^�J�i�܂ł͕ϊ��ł��Ȃ��̂Ŗ��ϊ�
			//�������u���v�u���v�A�u�����J(u3099)�v�u�����K(u309A)�v�u�J(u309B)�v�u�K(u309C)�v�͕ϊ��\  //2012.06.09 syat
			if( (c>=L'\u3097' && c<=L'\u3098') || (c>=L'\u309D' && c<=L'\u309F') ){ *dst++ = c; }
			else{
				//�S�p�Ђ炪�Ȃ�S�p�J�^�J�i�ɂ��Ă��甼�p�J�^�J�i�ɕϊ�����
				c = ZenhiraToZenkata_(c);
				if(0){}
				//�q�b�g���镶��������Εϊ����s��
				else if(wcschr_idx(tableZenkata_Normal    ,c,&n)){ *dst++=tableHankata_Normal[n];                  }
				else if(wcschr_idx(tableZenkata_Dakuten   ,c,&n)){ *dst++=tableHankata_Dakuten[n];    *dst++=L'�'; }
				else if(wcschr_idx(tableZenkata_HanDakuten,c,&n)){ *dst++=tableHankata_HanDakuten[n]; *dst++=L'�'; }
				else if(wcschr_idx(tableZenkata_DakuCho   ,c,&n)){ *dst++=tableHankata_DakuCho[n];                 }
				else if(wcschr_idx(tableZenkata_Kigo      ,c,&n)){ *dst++=tableHankata_Kigo[n];                    }
				//���ϊ�
				else { *dst++=c; }
			}
		}
		src++;
	}
	*dst=L'\0';
	*nDstLength = dst - pDst;
}

/*!
	���p�J�^�J�i���S�p�J�^�J�i
	���_�̕������A�������͌���\��������B�ŏ���2����1�ɂȂ�B
	pDst�ɂ͂��炩���ߏ\���ȃ��������m�ۂ��Ă������ƁB
*/
void Convert_HankataToZenkata(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength)
{
	const wchar_t* src = pSrc;
	const wchar_t* src_end = src + nSrcLength;
	wchar_t* dst = pDst;
	int n;

	while(src<src_end){
		wchar_t c=*src;
		wchar_t next=(src+1<src_end)?*(src+1):0; //����1�������ǂ�
		if(0){}
		//���_�A�����_�̃`�F�b�N���s���čs��
		else if(next==L'�' && wcschr_idx(tableHankata_Dakuten   ,c,&n)){ *dst++=tableZenkata_Dakuten[n];    src++; }
		else if(next==L'�' && wcschr_idx(tableHankata_HanDakuten,c,&n)){ *dst++=tableZenkata_HanDakuten[n]; src++; }
		//����ȊO�̕����`�F�b�N���s��
		else if(              wcschr_idx(tableHankata_Normal    ,c,&n)){ *dst++=tableZenkata_Normal[n];            }
		else if(              wcschr_idx(tableHankata_DakuCho   ,c,&n)){ *dst++=tableZenkata_DakuCho[n];           }
		else if(              wcschr_idx(tableHankata_Kigo      ,c,&n)){ *dst++=tableZenkata_Kigo[n];              }
		//���ϊ�
		else { *dst++=c; }
		src++;
	}
	*dst=L'\0';
	*nDstLength = dst - pDst;
}

/*!
	���p�J�^�J�i���S�p�Ђ炪��
	���_�̕������A�������͌���\��������B�ŏ���2����1�ɂȂ�B
	pDst�ɂ͂��炩���ߏ\���ȃ��������m�ۂ��Ă������ƁB
*/
void Convert_HankataToZenhira(const wchar_t* pSrc, int nSrcLength, wchar_t* pDst, int* nDstLength)
{
	const wchar_t* src = pSrc;
	const wchar_t* src_end = src + nSrcLength;
	wchar_t* dst = pDst;
	int n;

	while(src<src_end){
		wchar_t c=*src;
		wchar_t next=(src+1<src_end)?*(src+1):0; //����1�������ǂ�
		bool hit = true;	//���p�J�^�J�i���S�p�J�^�J�i�ϊ������{�������ǂ����������t���O
		if(0){}
		//���_�A�����_�̃`�F�b�N���s���čs��
		//���u�ށv�u�ށv�͂P���̑S�p�J�^�J�i�ɂ͕ϊ��ł��Ă��S�p�Ђ炪�Ȃ܂ł͕ϊ��ł��Ȃ��̂ő��_��؂藣���ĕϊ�
		else if(              wcschr_idx(L"ܦ"                  ,c,&n)){ *dst++=L"����"[n];            }
		else if(next==L'�' && wcschr_idx(tableHankata_Dakuten   ,c,&n)){ *dst++=tableZenkata_Dakuten[n];    src++; }
		else if(next==L'�' && wcschr_idx(tableHankata_HanDakuten,c,&n)){ *dst++=tableZenkata_HanDakuten[n]; src++; }
		//����ȊO�̕����`�F�b�N���s��
		else if(              wcschr_idx(tableHankata_Normal    ,c,&n)){ *dst++=tableZenkata_Normal[n];            }
		else if(              wcschr_idx(tableHankata_DakuCho   ,c,&n)){ *dst++=tableZenkata_DakuCho[n];           }
		else if(              wcschr_idx(tableHankata_Kigo      ,c,&n)){ *dst++=tableZenkata_Kigo[n];              }
		//���ϊ�
		else { *dst++=c; hit = false; }
		if(hit){ *(dst-1)=ZenkataToZenhira_(*(dst-1)); }	//���p�J�^�J�i����ϊ������S�p�J�^�J�i��S�p�Ђ炪�Ȃɕϊ��i�����Ƃ��ƑS�p�������J�^�J�i�͖��ϊ��j
		src++;
	}
	*dst=L'\0';
	*nDstLength = dst - pDst;
}
