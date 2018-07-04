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

#ifndef _PROCESSTHREADSAPI_H_
#error Need to include string before ProcessPriority.h
#endif  /* _PROCESSTHREADSAPI_H_ */


//���O���
namespace sakura {
	namespace _os {


/*!
 * @brief �v���Z�X�D��x�N���X
 *   �R���X�g���N�^�Ŏ��s���v���Z�X�̃v���Z�X�D��x��ۑ����A�f�X�g���N�^�Ō��ɖ߂��B
 *
 * @sa https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/nf-processthreadsapi-setpriorityclass
 * @date 2018/07/02 berryzplus		�V�K�쐬
 */
class ProcessPriority
{
private:
	HANDLE _hProcess;			//!< �v���Z�X�n���h��(�L���b�V��)
	DWORD _dwDefaultPriority;	//!< �ύX�O�̃v���Z�X�D��x

public:
	ProcessPriority(_In_ DWORD dwDesiredPriority)
		: _hProcess(::GetCurrentProcess())
		, _dwDefaultPriority(::GetPriorityClass(_hProcess))
	{
		// �v���Z�X�D��x��ݒ肷��
		set(dwDesiredPriority);
	}
	virtual ~ProcessPriority()
	{
		reset();
	}

public:
	//!�v���Z�X�D��x��ݒ肷��
	bool set(_In_ DWORD dwDesiredPriority)
	{
		BOOL bRet = FALSE;
		if (get() != dwDesiredPriority) {
			bRet = ::SetPriorityClass(_hProcess, dwDesiredPriority);
		}
		return bRet;
	}
	//!�v���Z�X�D��x���擾����
	DWORD get() const 
	{
		return ::GetPriorityClass(_hProcess);
	}
	//!�v���Z�X�D��x�����ɖ߂�
	void reset()
	{
		set(_dwDefaultPriority);
	}
};


	}; // end of namespace _os
}; // end of namespace sakura
