using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;

namespace ToolBarImageMuxer
{
    public class BmpMuxer
    {
        static public void Mux(string[] fileNames, string outFile, int countPerLine)
        {
        	// 一番最初の BMP の情報を取得する
            var bmp = (Bitmap)Image.FromFile(fileNames[0]);
            var sx = bmp.Size.Width;
            var sy = bmp.Size.Height;

            var lines = (fileNames.Length + countPerLine - 1) / countPerLine;
            var width = sx * countPerLine;
            var height = sy * lines;

            // フルカラーのビットマップを作成する (Graphics がフルカラーを必要とする)
            var imgMerge = new Bitmap(width, height);
            Graphics g = Graphics.FromImage(imgMerge);

            var index = 0;
            for (var y = 0; y < height; y += sy)
            {
                for (var x = 0; x < width; x += sx)
                {
                    using (var bmpTmp = (Bitmap)Image.FromFile(fileNames[index]))
                    {
                        var cloneRect = new RectangleF(x, y, sx, sy);
                        g.DrawImage(bmpTmp, cloneRect);
                    }
                    index++;
                }
            }

            // 入力のビットマップと同じ形式で保存する (減色処理含む)
            var outRect = new RectangleF(0, 0, width, height);
            var imgSave = imgMerge.Clone(outRect, bmp.PixelFormat);
            imgSave.Save(outFile, System.Drawing.Imaging.ImageFormat.Bmp);
        }
    }
}
