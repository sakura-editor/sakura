using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace ToolBarImageMuxer
{
    public class BmpMuxer
    {
        static private List<Color> CombinePalettes(List<ColorPalette> palettes)
        {
            List<Color> colors = new List<Color>();
            foreach (var palette in palettes)
            {
                foreach (var entry in palette.Entries)
                {
                    bool bFound = false;
                    foreach (var color in colors)
                    {
                        if (entry.Equals(color))
                        {
                            bFound = true;
                            break;
                        }
                    }
                    if (!bFound)
                    {
                        colors.Add(entry);
                    }
                }
            }
            return colors;
        }

        static private Bitmap CopyToIndexedPixelFormatBitmap(Bitmap inputImage, List<Color> colors)
        {
            PixelFormat pixfmt;
            if (colors.Count <= 2)
            {
                pixfmt = PixelFormat.Format1bppIndexed;
            }
            if (colors.Count <= 16)
            {
                pixfmt = PixelFormat.Format4bppIndexed;
            }
            else
            {
                pixfmt = PixelFormat.Format8bppIndexed;
            }
            var width = inputImage.Width;
            var height = inputImage.Height;
            var outputImage = new Bitmap(width, height, pixfmt);
            var data = outputImage.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.WriteOnly, pixfmt);
            var bytes = new byte[data.Height * data.Stride];
            switch (pixfmt)
            {
                case PixelFormat.Format1bppIndexed:
                    throw new NotImplementedException();
                case PixelFormat.Format4bppIndexed:
                    for (int y = 0; y < height; ++y)
                    {
                        int w2 = width / 2;
                        for (int i = 0; i < w2; ++i)
                        {
                            Color c0 = inputImage.GetPixel(i * 2 + 0, y);
                            Color c1 = inputImage.GetPixel(i * 2 + 1, y);
                            int idx0 = 0;
                            int idx1 = 0;
                            for (int j = 0; j < 16; ++j)
                            {
                                Color c = colors[j];
                                if (c0 == c)
                                {
                                    idx0 = j;
                                }
                                if (c1 == c)
                                {
                                    idx1 = j;
                                }
                            }
                            bytes[data.Stride * y + i] = (byte)(idx0 * 16 + idx1);
                        }
                        if (width % 2 == 1)
                        {
                            Color c0 = inputImage.GetPixel(width - 1, y);
                            int idx0 = 0;
                            for (int j = 0; j < 16; ++j)
                            {
                                Color c = colors[j];
                                if (c0 == c)
                                {
                                    idx0 = j;
                                }
                            }
                            bytes[data.Stride * y + w2] = (byte)(idx0 * 16 + idx0);
                        }
                    }
                    break;
                case PixelFormat.Format8bppIndexed:
                    for (int y = 0; y < height; ++y)
                    {
                        for (int x = 0; x < width; ++x)
                        {
                            Color c0 = inputImage.GetPixel(x, y);
                            for (int i = 0; i < 256; ++i)
                            {
                                Color c = colors[i];
                                if (c0 == c)
                                {
                                    bytes[data.Stride * y + x] = (byte)i;
                                    break;
                                }
                            }
                        }
                    }
                    break;
            }
            Marshal.Copy(bytes, 0, data.Scan0, bytes.Length);
            outputImage.UnlockBits(data);
            var pal = outputImage.Palette;
            for (int i = 0; i < colors.Count; ++i)
            {
                pal.Entries[i] = colors[i];
            }
            outputImage.Palette = pal;
            return outputImage;
        }

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
            List<ColorPalette> palettes = new List<ColorPalette>();

            bool palettable = true;
            var index = 0;
            for (var y = 0; y < height; y += sy)
            {
                for (var x = 0; x < width; x += sx)
                {
                    using (var bmpTmp = (Bitmap)Image.FromFile(fileNames[index]))
                    {
                        if (palettable)
                        {
	                        switch (bmpTmp.PixelFormat)
	                        {
	                            case PixelFormat.Format1bppIndexed:
	                            case PixelFormat.Format4bppIndexed:
	                            case PixelFormat.Format8bppIndexed:
                                    palettes.Add(bmpTmp.Palette);
	                                break;
	                            default:
	                                // インデックスカラー画像でなくても使われている色数を調べて判断する手もあるが、実装が手間なので行わない
	                                palettable = false;
	                                break;
	                        }
                        }
                        var cloneRect = new RectangleF(x, y, sx, sy);
                        g.DrawImage(bmpTmp, cloneRect);
                    }
                    index++;
                }
            }
            Bitmap imgSave = imgMerge;
            if (palettable)
            {
                // パレット統合
                var colors = CombinePalettes(palettes);
                if (colors.Count <= 256)
                {
                	// インデックスカラー画像にコピー
                    imgSave = CopyToIndexedPixelFormatBitmap(imgMerge, colors);
                }
            }
            imgSave.Save(outFile, System.Drawing.Imaging.ImageFormat.Bmp);
        }
    }
}
