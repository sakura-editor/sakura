/*!	@file
	@brief タグジャンプリスト

	@author MIK
	@date 2005.3.31
*/
/*
	Copyright (C) 2005, MIK, genta

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

#pragma once

#include "util/design_template.h"

#define MAX_TAG_STRING_LENGTH _MAX_PATH //管理する文字列の最大長

/*!	@brief ダイレクトタグジャンプ用検索結果をソートして保持する．
	@author MIK
*/
class CSortedTagJumpList
{
  public:
    CSortedTagJumpList(int max);
    ~CSortedTagJumpList();

    int AddBaseDir(const WCHAR *baseDir);
    BOOL AddParamA(const ACHAR *keyword, const ACHAR *filename, int no, ACHAR type, const ACHAR *note, int depth, const int baseDirId);
    BOOL GetParam(int index, WCHAR *keyword, WCHAR *filename, int *no, WCHAR *type, WCHAR *note, int *depth, WCHAR *baseDir);
    int GetCount(void)
    {
        return m_nCount;
    }
    void Empty(void);
    bool IsOverflow(void)
    {
        return m_bOverflow;
    }

    typedef struct tagjump_info_t
    {
        struct tagjump_info_t *next; //!< 次のリスト
        WCHAR *keyword; //!< キーワード
        WCHAR *filename; //!< ファイル名
        int no; //!< 行番号
        WCHAR type; //!< 種類
        WCHAR *note; //!< 備考
        int depth; //!< (さかのぼる)階層
        int baseDirId; //!< ファイル名のベースディレクトリ
    } TagJumpInfo;

    TagJumpInfo *GetPtr(int index);

    /*!	@brief 管理数の最大値を取得する

		@date 2005.04.22 genta 最大値を可変に
	*/
    int GetCapacity(void) const
    {
        return m_MAX_TAGJUMPLIST;
    }

  private:
    TagJumpInfo *m_pTagjump; //!< タグジャンプ情報
    std::vector<std::wstring> m_baseDirArr; //!< ベースディレクトリ情報
    int m_nCount; //!< 個数
    bool m_bOverflow; //!< オーバーフロー

    //	2005.04.22 genta 最大値を可変に
    const int m_MAX_TAGJUMPLIST; //!< 管理する情報の最大数

    void Free(TagJumpInfo *item);

    DISALLOW_COPY_AND_ASSIGN(CSortedTagJumpList);
};
