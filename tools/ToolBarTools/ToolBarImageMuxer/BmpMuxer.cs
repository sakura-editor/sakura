using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ToolBarImageMuxer
{

    // https://www.pinvoke.net/default.aspx/Enums/BitmapCompressionMode.html
    public enum BitmapCompressionMode : uint
    {
        BI_RGB = 0,
        BI_RLE8 = 1,
        BI_RLE4 = 2,
        BI_BITFIELDS = 3,
        BI_JPEG = 4,
        BI_PNG = 5
    }

    // https://www.pinvoke.net/default.aspx/Structures.BITMAPFILEHEADER
    [StructLayout(LayoutKind.Sequential, Pack = 2)]
    public struct BITMAPFILEHEADER
    {
        public ushort bfType;
        public uint bfSize;
        public ushort bfReserved1;
        public ushort bfReserved2;
        public uint bfOffBits;
    }

    // https://www.pinvoke.net/default.aspx/Structures.BITMAPINFOHEADER
    [StructLayout(LayoutKind.Sequential)]
    public struct BITMAPINFOHEADER
    {
        public uint biSize;
        public int biWidth;
        public int biHeight;
        public ushort biPlanes;
        public ushort biBitCount;
        public BitmapCompressionMode biCompression;
        public uint biSizeImage;
        public int biXPelsPerMeter;
        public int biYPelsPerMeter;
        public uint biClrUsed;
        public uint biClrImportant;

        public void Init()
        {
            biSize = (uint)Marshal.SizeOf(this);
        }
    }

    // https://www.pinvoke.net/default.aspx/Structures.RGBQUAD
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct RGBQUAD
    {
        public byte rgbBlue;
        public byte rgbGreen;
        public byte rgbRed;
        public byte rgbReserved;
    }

    // https://smdn.jp/programming/netfx/tips/convert_struct_and_bytearray/
    /// <summary>
    /// ポインタを得るためにGCHandle.Allocでバイト配列のピニングを行い、
    /// Marshal.PtrToStructure・Marshal.StructureToPtrで変換して構造体の読み書きを行う
    /// </summary>
    /// <remarks>参照型のフィールドを持つ構造体は読み書きできない</remarks>
    static class ReadWriteStructWithAllocGCHandle
    {
        public static void WriteTo<TStruct>(BinaryWriter writer, TStruct s) where TStruct : struct
        {
            var buffer = new byte[Marshal.SizeOf(typeof(TStruct))];
            var handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);

            try
            {
                Marshal.StructureToPtr(s, handle.AddrOfPinnedObject(), false);
            }
            finally
            {
                handle.Free();
            }

            writer.Write(buffer);
        }

        public static TStruct ReadFrom<TStruct>(BinaryReader reader) where TStruct : struct
        {
            var buffer = reader.ReadBytes(Marshal.SizeOf(typeof(TStruct)));
            var handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);

            try
            {
                return (TStruct)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(TStruct));
            }
            finally
            {
                handle.Free();
            }
        }
    }

    public class Bmp
    {
        public BITMAPINFOHEADER bmih;
        public RGBQUAD[] colorTable;
        public byte[] bitmap;

        public int GetLineStride()
        {
            int bytesPerLine = ((bmih.biWidth * bmih.biBitCount + 31) & (~31)) >> 3;
            if (bmih.biHeight > 0)
                bytesPerLine = -bytesPerLine;
            return bytesPerLine;
        }

        public void Paste(int x, int y, Bmp src)
        {
            int sbc = src.bmih.biBitCount;
            int sw = src.bmih.biWidth;
            int sh = src.bmih.biHeight;
            int dbc = bmih.biBitCount;
            int dw = bmih.biWidth;
            int dh = bmih.biHeight;
            if (sbc > dbc || x < 0 || y < 0 || x + sw > dw || y + sh > dh)
            {
                throw new OutOfMemoryException();
            }
            int sLineStride = src.GetLineStride();
            int dLineStride = GetLineStride();
            if (sbc == 8 && dbc == 8)
            {
                int sidx = Math.Abs(sLineStride) * (sh - 1);
                int didx = Math.Abs(dLineStride) * (dh - 1 - y) + x;
                for (int i=0; i<sh; ++i)
                {
                    for (int j=0; j<sw; ++j)
                    {
                        RGBQUAD sc = src.colorTable[src.bitmap[sidx + j]];
                        for (int k=0; k<colorTable.Count(); ++k)
                        {
                            if (sc.Equals(colorTable[k]))
                            {
                                bitmap[didx + j] = (byte)k;
                                break;
                            }
                        }

                    }
                    sidx += sLineStride;
                    didx += dLineStride;
                }
            }
        }

        public void ToFile(string fileName)
        {
            BITMAPFILEHEADER bmfh;
            bmfh.bfType = 0x4D42;
            int fileSize = Marshal.SizeOf(typeof(BITMAPFILEHEADER));
            fileSize += Marshal.SizeOf(typeof(BITMAPINFOHEADER));
            fileSize += Marshal.SizeOf(typeof(RGBQUAD)) * colorTable.Count();
            bmfh.bfOffBits = (uint)fileSize;
            fileSize += bitmap.Count();
            bmfh.bfSize = (uint)fileSize;
            bmfh.bfReserved1 = 0;
            bmfh.bfReserved2 = 0;
            using (var writer = new BinaryWriter(File.OpenWrite(fileName)))
            {
                ReadWriteStructWithAllocGCHandle.WriteTo<BITMAPFILEHEADER>(writer, bmfh);
                ReadWriteStructWithAllocGCHandle.WriteTo<BITMAPINFOHEADER>(writer, bmih);
                foreach (var c in colorTable)
                {
                    ReadWriteStructWithAllocGCHandle.WriteTo<RGBQUAD>(writer, c);
                }
                writer.BaseStream.Write(bitmap, 0, bitmap.Count());
            }
        }

        public static Bmp FromFile(string fileName)
        {
            Bmp bmp = new Bmp();
            using (BinaryReader reader = new BinaryReader(File.Open(fileName, FileMode.Open)))
            {
                BITMAPFILEHEADER bmfh;
                bmfh = ReadWriteStructWithAllocGCHandle.ReadFrom<BITMAPFILEHEADER>(reader);
                if (bmfh.bfType != 0x4D42)
                {
                    throw new OutOfMemoryException();
                }
                bmp.bmih = ReadWriteStructWithAllocGCHandle.ReadFrom<BITMAPINFOHEADER>(reader);
                uint numQuads;
                if (bmp.bmih.biClrUsed == 0)
                {
                    switch (bmp.bmih.biBitCount)
                    {
                        case 1:
                            numQuads = 2;
                            break;
                        case 4:
                            numQuads = 16;
                            break;
                        case 8:
                            numQuads = 256;
                            break;
                        default:
                            throw new OutOfMemoryException();
                    }
                }
                else
                {
                    numQuads = bmp.bmih.biClrUsed;
                }

                if (bmp.bmih.biHeight < 0)
                {
                    // TODO: TopDown 形式のサポート
                    throw new OutOfMemoryException();
                }

                bmp.colorTable = new RGBQUAD[numQuads];
                for (int i=0; i<numQuads; ++i)
                {
                    bmp.colorTable[i] = ReadWriteStructWithAllocGCHandle.ReadFrom<RGBQUAD>(reader);
                }
                if (reader.BaseStream.Position != bmfh.bfOffBits)
                {
                    throw new OutOfMemoryException();
                }
                // https://stackoverflow.com/a/10038017/4699324
                using (var ms = new MemoryStream())
                {
                    reader.BaseStream.CopyTo(ms);
                    bmp.bitmap = ms.ToArray();
                }
            }
            return bmp;
        }
    }

    public class BmpMuxer
    {
        static private List<RGBQUAD> CombinePalettes(List<Bmp> bmps)
        {
            List<RGBQUAD> colors = new List<RGBQUAD>();
            foreach (var entry in bmps[0].colorTable)
            {
                foreach (var color in colors)
                {
                    colors.Add(entry);
                }
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
                    bmp.colorTable = new RGBQUAD[2 << biBitCount];
                }
                else
                {
                    bmp.bmih.biClrUsed = (uint)(colors.Count);
                    bmp.bmih.biClrImportant = bmp.bmih.biClrUsed;
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
