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
