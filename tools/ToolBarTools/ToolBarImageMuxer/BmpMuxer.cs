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
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using ToolBarImageCommon;

namespace ToolBarImageMuxer
{
    public class BmpMuxer
    {
        static private List<RGBQUAD> CombinePalettes(List<Bmp> bmps)
        {
            List<RGBQUAD> colors = new List<RGBQUAD>(bmps[0].colorTable);
            for (var i=1; i<bmps.Count; ++i)
            {
                var bmp = bmps[i];
                foreach (var entry in bmp.colorTable)
                {
                    if (!colors.Contains(entry))
                    {
                        colors.Add(entry);
                    }
                }
            }
            return colors;
        }

        static protected bool IsPalettable(List<Bmp> bmps)
        {
            var width = bmps[0].bmih.biWidth;
            var height = Math.Abs(bmps[0].bmih.biHeight);
            for (var i = 1; i < bmps.Count; ++i)
            {
                var bmp = bmps[i];
                var width2 = bmp.bmih.biWidth;
                var height2 = Math.Abs(bmp.bmih.biHeight);
                if (width != width2 || height != height2)
                {
                    throw new Exception();
                }
                var biBitCount = bmp.bmih.biBitCount;
                if (biBitCount != 1 && biBitCount != 4 && biBitCount != 8)
                {
                    // インデックスカラー画像でなくても使われている色数を調べて判断する手もあるが、実装が手間なので行わない
                    return false;
                }
            }
            return true;
        }

        static public void Mux(string[] fileNames, string outFile, int countPerLine)
        {
            List<Bmp> bmps = new List<Bmp>();
            var lines = (fileNames.Length + countPerLine - 1) / countPerLine;
            var index = 0;
            for (var y = 0; y < lines; ++y)
            {
                for (var x = 0; x < countPerLine; ++x)
                {
                    bmps.Add(Bmp.FromFile(fileNames[index]));
                    index++;
                }
            }
            bool palettable = IsPalettable(bmps);
            List<RGBQUAD> colors = null;
            if (palettable)
            {
                // パレット統合
                colors = CombinePalettes(bmps);
                if (colors.Count > 256)
                {
                    palettable = false;
                }
            }
            if (palettable)
            {
                ushort biBitCount = 8;
                if (colors.Count <= 2)
                {
                    biBitCount = 1;
                }
                if (colors.Count <= 16)
                {
                    biBitCount = 4;
                }
                else
                {
                    biBitCount = 8;
                }
                Bmp bmp = new Bmp();
                bmp.bmih = bmps[0].bmih;
                var width = bmps[0].bmih.biWidth;
                var height = Math.Abs(bmps[0].bmih.biHeight);
                bmp.bmih.biWidth = width * countPerLine;
                bmp.bmih.biHeight = height * lines;
                bmp.bmih.biBitCount = biBitCount;
                if (bmp.bmih.biClrUsed == 0)
                {
                    bmp.colorTable = new RGBQUAD[1 << biBitCount];
                }
                else
                {
                    bmp.bmih.biClrUsed = (uint)(colors.Count);
                    bmp.colorTable = new RGBQUAD[bmp.bmih.biClrUsed];
                }
                for (int i=0; i<colors.Count; ++i)
                {
                    bmp.colorTable[i] = colors[i];
                }

                int lineStride = bmp.GetLineStride();
                bmp.imageData = new byte[Math.Abs(lineStride * bmp.bmih.biHeight)];
                index = 0;
                for (var y = 0; y < lines; ++y)
                {
                    for (var x = 0; x < countPerLine; ++x)
                    {
                        bmp.Paste(x * width, y * height, bmps[index]);
                        index++;
                    }
                }
                bmp.ToFile(outFile);
            }
            else
            {
                throw new NotImplementedException();
            }
        }
    }
}
