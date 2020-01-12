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
            List<RGBQUAD> colors = new List<RGBQUAD>();
            foreach (var entry in bmps[0].colorTable)
            {
                colors.Add(entry);
            }
            for (var i=1; i<bmps.Count; ++i)
            {
                var bmp = bmps[i];
                foreach (var entry in bmp.colorTable)
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
            var width = bmps[0].bmih.biWidth;
            var height = Math.Abs(bmps[0].bmih.biHeight);
            bool palettable = true;
            for (var i=1; i<bmps.Count; ++i)
            {
                var bmp = bmps[i];
                var width2 = bmp.bmih.biWidth;
                var height2 = Math.Abs(bmp.bmih.biHeight);
                if (width != width2 || height != height2)
                {
                    throw new Exception();
                }
                if (palettable)
                {
                    var biBitCount = bmp.bmih.biBitCount;
                    if (biBitCount != 1 && biBitCount != 4 && biBitCount != 8)
                    {
                        // インデックスカラー画像でなくても使われている色数を調べて判断する手もあるが、実装が手間なので行わない
                        palettable = false;
                    }
                }
            }
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
                bmp.bitmap = new byte[Math.Abs(lineStride * bmp.bmih.biHeight)];
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
