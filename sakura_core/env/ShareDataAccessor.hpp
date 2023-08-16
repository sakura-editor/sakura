/*! @file */
/*
	Copyright (C) 2023, Sakura Editor Organization

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

#include <memory>

struct DLLSHAREDATA;

/*!
 * 共有メモリへのアクセスを仮想化するクラス
 */
struct ShareDataAccessor
{
	virtual ~ShareDataAccessor() = default;

	virtual DLLSHAREDATA* GetShareData() const;
};

/*!
 * 共有メモリに依存するクラスの基底クラス
 */
class ShareDataAccessorClient
{
	std::shared_ptr<ShareDataAccessor> _ShareDataAccessor;

public:
	explicit ShareDataAccessorClient(std::shared_ptr<ShareDataAccessor> ShareDataAccessor_)
		: _ShareDataAccessor(std::move(ShareDataAccessor_))
	{
	}

protected:
	std::shared_ptr<ShareDataAccessor> GetShareDataAccessor() const
	{
		return _ShareDataAccessor;
	}

	/*!
	 * 共有メモリ構造体への参照を取得します。
	 */
	DLLSHAREDATA& GetDllShareData() const
	{
		return *GetShareData();
	}

	/*!
	 * 共有メモリ構造体のアドレスを取得します。
	 */
	DLLSHAREDATA* GetShareData() const
	{
		return _ShareDataAccessor->GetShareData();
	}
};

/*!
 * 構築時に共有メモリをキャッシュするクラスの基底クラス
 *
 * コンストラクタ内で共有メモリにアクセスしてクラッシュするクラスを
 * テスト可能にするために用意した基底クラスです。
 *
 * @date 2002/02/17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
 */
class ShareDataAccessorClientWithCache : public ShareDataAccessorClient
{
public:
	DLLSHAREDATA* m_pShareData;

	explicit ShareDataAccessorClientWithCache(std::shared_ptr<ShareDataAccessor> ShareDataAccessor_)
		: ShareDataAccessorClient(std::move(ShareDataAccessor_))
	{
		/* 共有データ構造体のアドレスを返す */
		m_pShareData = GetShareData();
	}
};
