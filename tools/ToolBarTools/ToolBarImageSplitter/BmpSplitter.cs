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
        static public void Split(string fileName, string outDir)
        {
            if (!Directory.Exists(outDir))
            {
                Directory.CreateDirectory(outDir);
            }

            var bmp = (Bitmap)Image.FromFile(fileName);
            var width = bmp.Size.Width;
            var height = bmp.Size.Height;
            var sx = 16;
            var sy = 16;

            var index = 1;
            for (var y = 0; y < height; y += sy)
            {
                for (var x = 0; x < width; x += sx)
                {
                    var cloneRect = new RectangleF(x, y, sx, sy);
                    using (var cloneBitmap = bmp.Clone(cloneRect, System.Drawing.Imaging.PixelFormat.Format4bppIndexed))
                    {
                        var outfile = Path.Combine(outDir, index.ToString() + ".bmp");
                        cloneBitmap.Save(outfile, System.Drawing.Imaging.ImageFormat.Bmp);
                    }
                    index++;
                }
            }
        }
    }
}
