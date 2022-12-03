/*
	Copyright (C) 2020-2022, Sakura Editor Organization

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
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections;
using System.Text.RegularExpressions;

namespace ToolBarImageMuxer
{
    public class BmpFileComparer : IComparer<string>
    {
        static Regex regex = new Regex(@"(\d+)(\D*)\.bmp", RegexOptions.Compiled);
    
        public int Compare(string x, string y)
        {
            var matchX = regex.Match(x);
            var matchY = regex.Match(y);

            if (matchX.Success && matchY.Success)
            {
                var indexX = int.Parse(matchX.Groups[1].Value);
                var indexY = int.Parse(matchY.Groups[1].Value);

				// 数値だけのファイルの場合、数値として比較する
                if (indexX != indexY)
                {
                    return indexX - indexY;
                }
                
                // 数値の部分が同じ場合、文字列として比較する。
            }
            return x.CompareTo(y);
        }
    }
}
