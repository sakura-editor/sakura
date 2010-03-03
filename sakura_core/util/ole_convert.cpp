/*!	@file
	@brief OLE�^�iVARIANT, BSTR�Ȃǁj�̕ϊ��֐�

*/
#include "stdafx.h"
#include "ole_convert.h"

// VARIANT�ϐ���BSTR�Ƃ݂Ȃ��Awstring�ɕϊ�����
// CMacro::HandleFunction���Q�l�Ƃ����B
bool variant_to_wstr( VARIANT v, std::wstring& wstr )
{
	Variant varCopy;	// VT_BYREF���ƍ���̂ŃR�s�[�p
	if(VariantChangeType(&varCopy.Data, &v, 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���

	wchar_t *Source;
	int SourceLength;
	Wrap(&varCopy.Data.bstrVal)->GetW(&Source, &SourceLength);

	wstr.assign( Source, SourceLength );
	delete[] Source;

	return true;
}

// VARIANT�ϐ��𐮐��Ƃ݂Ȃ��Aint�ɕϊ�����
// CMacro::HandleFunction���Q�l�Ƃ����B
bool variant_to_int( VARIANT v, int& n )
{
	Variant varCopy;	// VT_BYREF���ƍ���̂ŃR�s�[�p
	if(VariantChangeType(&varCopy.Data, &v, 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���

	n = varCopy.Data.lVal;
	return true;
}
