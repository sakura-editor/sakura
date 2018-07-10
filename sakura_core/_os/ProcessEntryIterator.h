/*
  Copyright (C) 2018, Sakura Editor Organization

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Sakura Editor Organization
    https://sakura-editor.github.io/ 

 */

#pragma once

#ifndef _APISETHANDLE_
#error Need to include windows.h before ProcessEntryIterator.h
#endif  /* _APISETHANDLE_ */

#include <iterator>
#include <TlHelp32.h>


//���O���
namespace sakura {
	namespace _os {


/*!
 * @brief ProcessEntryIterator
 *
 * �\�z�����u�Ԃ̃X�i�b�v�V���b�g�����Ƀv���Z�X������������P�����C�e���[�^�B
 *
 * ��u�C���N�������g�͒񋟂��Ȃ��B
 * ��r���Z�q�̓��[�v�̏I������ȊO�̖ړI�Ɏg�p���Ȃ����ƁB
 *
 * @author	berryzplus
 * @date	2018/06/25 berryzplus	�V�K�쐬
 */
class ProcessEntryIterator : public std::iterator<std::input_iterator_tag, PROCESSENTRY32>
{
	typedef std::iterator<std::input_iterator_tag, PROCESSENTRY32>	_Mybase;
	typedef ProcessEntryIterator									_Myt;

private:
	HANDLE				_hSnapshot;
	PROCESSENTRY32		_sFoundData;

public:
	/*!
	 * @brief ��������R���X�g���N�^(�����J�n�p)
	 *
	 * @param [in]		dwProcessId		�v���Z�XID�B
	 */
	ProcessEntryIterator(
		_In_ DWORD dwProcessId
	) noexcept
		: _hSnapshot(INVALID_HANDLE_VALUE)
	{
		// �v���Z�X�̃X�i�b�v�V���b�g���쐬����
		// �����win xp�ȍ~�̂ݎg����֐��B32�ƕt���Ă邪64���p�B
		// https://msdn.microsoft.com/en-us/library/ms682489(v=vs.85).aspx
		_hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, dwProcessId);
		if (_hSnapshot == INVALID_HANDLE_VALUE) {
			return;
		}

		// �ŏ��̌��������s����
		_sFoundData.dwSize = sizeof(decltype(_sFoundData));
		if (!::Process32First(_hSnapshot, &_sFoundData)) {
			Close();
		}
	}
	//�����Ȃ��R���X�g���N�^(end�\�z�p)
	ProcessEntryIterator(void) noexcept
		: _hSnapshot(INVALID_HANDLE_VALUE)
	{
	}
	//�f�X�g���N�^
	virtual ~ProcessEntryIterator(void) noexcept
	{
		Close();
	}

public:
	//�O�u�C���N�������g
	_Myt & operator++(void) noexcept
	{
		//�����n���h�����L���ȏꍇ�̂�
		if (_hSnapshot != INVALID_HANDLE_VALUE)
		{
			//��������
			if (!::Process32Next(_hSnapshot, &_sFoundData))
			{
				Close();
			}
		}
		return (*this);
	}
	pointer   operator->(void) const noexcept { return const_cast< pointer >(&_sFoundData); }
	reference operator* (void) const noexcept { return const_cast< reference >(_sFoundData); }
	bool      operator==(const _Myt& rhs) const noexcept { return _hSnapshot == rhs._hSnapshot; }
	bool      operator!=(const _Myt& rhs) const noexcept { return !(*this == rhs); }

	ProcessEntryIterator(const _Myt& rhs) = delete;
	_Myt& operator = (const _Myt& rhs) = delete;
	_Myt operator++( int ) = delete;

protected:
	//�������I������Ƃ��ƁA�f�X�g���N�^���Ă΂ꂽ�Ƃ��ɌĂ΂��
	void Close(void) noexcept
	{
		//�����n���h���������Ȃ牽�����Ȃ�
		if (_hSnapshot == INVALID_HANDLE_VALUE)
		{
			return;
		}

		//�L���Ȍ����n���h�����c���Ă���Ε���
		::CloseHandle(_hSnapshot);
		_hSnapshot = INVALID_HANDLE_VALUE;
	}
};


	}; // end of namespace _os
}; // end of namespace sakura


   //���O��Ԃ��ӎ������Ɉ�����悤��using���Ă����B
using sakura::_os::ProcessEntryIterator;
