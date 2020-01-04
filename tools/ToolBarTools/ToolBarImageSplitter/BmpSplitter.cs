using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Drawing;

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

            var bmp = (Bitmap)Image.FromFile(fileName);
            var width = bmp.Size.Width;
            var height = bmp.Size.Height;
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

                    var cloneRect = new RectangleF(x, y, sx, sy);
                    using (var cloneBitmap = bmp.Clone(cloneRect, bmp.PixelFormat))
                    {
                        cloneBitmap.Save(outfile, System.Drawing.Imaging.ImageFormat.Bmp);
                        Console.WriteLine("wrote {0} x={1} y={2}", outfile, x, y);
                    }
                }
            }
        }
    }
}
