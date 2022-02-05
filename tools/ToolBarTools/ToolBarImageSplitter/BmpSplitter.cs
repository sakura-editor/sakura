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
using System.IO;
using System.Diagnostics;
using ToolBarImageCommon;

namespace ToolBarImageSplitter
{
    public class BmpSplitter
    {
        static public void Split(string fileName, string outDir, int countPerLine)
        {
            if (!Directory.Exists(outDir))
            {
                Directory.CreateDirectory(outDir);
            }

            var bmp = Bmp.FromFile(fileName);
            var width = bmp.bmih.biWidth;
            var height = Math.Abs(bmp.bmih.biHeight);
            // 縦横ピクセル数同じ限定
            var sx = width / countPerLine;
            var sy = sx;

            var index = 0;
            for (var y = 0; y < height; y += sy)
            {
                for (var x = 0; x < width; x += sx)
                {
                    var outfile = string.Empty;
                    if (x + sx == width)
                    {
	                    // 右端の画像はインデックス用
                        outfile = Path.Combine(outDir, index.ToString() + "-index.bmp");
                    }
                    else
                    {
                        index++;
                        outfile = Path.Combine(outDir, index.ToString() + ".bmp");
                    }
                    var copiedBitmap = bmp.Copy(x, y, sx, sy);
                    copiedBitmap.ToFile(outfile);
                    Console.WriteLine("wrote {0} x={1} y={2}", outfile, x, y);
                }
            }
        }
    }
}
