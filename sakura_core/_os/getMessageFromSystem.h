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

#ifndef _STRING_
#error Need to include string before getMessageFromSystem.h
#endif  /* _STRING_ */

#if !defined(_WINBASE_) || !defined(_WINNT_)
#error Need to include windows.h before getMessageFromSystem.h
#endif  /* !defined(_WINBASE_) || !defined(_WINNT_) */


namespace _os {


/*!
 * @brief �V�X�e�����b�Z�[�W���擾����
 *
 *		Windows API�ďo���s���̒�^�����𒊏o�������́B
 *
 * @param [in]		dwMessageCode	���b�Z�[�W�R�[�h�B
 *									::GetLastError()�̖߂�l���w�肷��B
 * @param [in,opt]	dwLanguageId	���ꎯ�ʎq�B
 *									�ȗ������ꍇ�A�j���[�g��������B
 *									�ڍׂ�::FormatMessage()�̃}�j���A�����Q�ƁB
 * @retval msg						�V�X�e�����b�Z�[�W�B
 *									�Y�����郁�b�Z�[�W���Ȃ��ꍇ�A�󕶎����Ԃ��B
 * @sa https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-formatmessage
 * @date 2018/06/25 berryzplus		�V�K�쐬
 */
inline
std::wstring getMessageFromSystem(
	_In_ DWORD dwMessageCode,
	_In_opt_ DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
)
{
	HLOCAL pMsg = NULL;
	DWORD length = ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwMessageCode,
		dwLanguageId,
		(LPWSTR)&pMsg,
		0,
		NULL
	);
	std::wstring msg((LPCWSTR)pMsg, length);
	::LocalFree(pMsg);
	return std::move(msg);
}


}; // end of namespace _os
