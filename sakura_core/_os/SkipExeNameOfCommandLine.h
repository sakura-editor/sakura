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
#error Need to include string before SkipExeNameOfCommandLine.h
#endif  /* _STRING_ */

#ifndef _WINNT_
#error Need to include windows.h before SkipExeNameOfCommandLine.h
#endif  /* _WINNT_ */


namespace _os {


/*!
 * @brief WinMain�̈����u�R�}���h���C��������v���擾����
 *
 *  ����: "\"C:\Program Files (x86)\\sakura\\sakura.exe\" -NOWIN"
 *  �o��: "-NOWIN"
 *  ���|�C���^�ʒu��i�߂Ă��邾���B
 *
 * @param [in] lpCmdLine GetCommandLineW()�̖߂�l�B
 * @retval lpCmdLine wWinMain�`���̃R�}���h���C��������B
 */
inline
_Ret_z_ LPWSTR SkipExeNameOfCommandLine(_In_z_ LPWSTR lpCmdLine) noexcept
{
	// �����萔(�󔒕���)
	const WCHAR whiteSpace[] = L"\t\x20";

	// �����񂪃_�u���N�H�[�e�[�V�����Ŏn�܂��Ă��邩�`�F�b�N
	if (L'\x22' == lpCmdLine[0]) {
		// ������|�C���^��i�߂�
		lpCmdLine++;
		// ���N�H�[�e�[�V������T��(�p�X������Ȃ̂ŃG�X�P�[�v�̍l���͕s�v)
		WCHAR *p = ::wcschr(lpCmdLine, L'\x22');
		if (p) {
			// ������|�C���^��i�߂�
			lpCmdLine = ++p;
		}
	}
	else {
		// �ŏ��̃g�[�N�����X�L�b�v����
		// ��Windows ���Ŏ��s����ꍇ�A���̕����̓f�b�h�R�[�h�ɂȂ�
		//   Wine���ɂ��G�~�����[�^���s���l�����Ď��������͂��Ă���
		size_t nPos = ::wcscspn(lpCmdLine, whiteSpace);
		lpCmdLine = &lpCmdLine[nPos];
	}

	// ���̃g�[�N���܂Ői�߂�
	size_t nPos = ::wcsspn(lpCmdLine, whiteSpace);
	return &lpCmdLine[nPos];
}


}; // end of namespace _os
