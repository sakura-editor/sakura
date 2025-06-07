/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

namespace ChmSourceConverter
{
    class Program
    {
        static void Main(string[] args)
        {
            new ChmSourceConverterApp(Properties.Settings.Default).Start(args);
        }
    }
}
