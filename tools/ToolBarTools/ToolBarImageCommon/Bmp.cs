using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ToolBarImageCommon
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

        public bool IsBottomUp()
        {
            return bmih.biHeight > 0;
        }

        public bool IsTopDown()
        {
            return bmih.biHeight < 0;
        }

        // ビットマップ領域中の指定座標がビットマップ配列の先頭から何バイトずれた位置に記録されているかを返す
        public int GetByteOffset(int x, int y)
        {
            int lineStride = GetLineStride();
            int w = bmih.biWidth;
            int h = Math.Abs(bmih.biHeight);
            if (x < 0 || y < 0 || x > w || y > h)
            {
                throw new OutOfMemoryException();
            }
            int bitCount = bmih.biBitCount;
            int idx = IsBottomUp() ? ((h - 1 - y) * Math.Abs(lineStride)) : (y * lineStride);
            if (bitCount == 8)
            {
                idx += x;
            }
            else if (bitCount == 4)
            {
                if ((x & 1) == 1)
                {
                    throw new OutOfMemoryException();
                }
                idx += x / 2;
            }
            else
            {
                throw new NotImplementedException();
            }
            return idx;
        }

        public Bmp Copy(int x, int y, int w, int h)
        {
            Bmp ret = new Bmp();
            ret.bmih = bmih;
            ret.bmih.biWidth = w;
            ret.bmih.biHeight = h;
            ret.colorTable = colorTable;
            int sLineStride = GetLineStride();
            int dLineStride = ret.GetLineStride();
            int sw = bmih.biWidth;
            int sh = bmih.biHeight;
            if (x < 0 || y < 0 || x + w > sw || y + h > sh)
            {
                throw new NotImplementedException();
            }
            int bc = bmih.biBitCount;
            if (bc == 4)
            {
                if ((x & 1) == 1 || (y & 1) == 1 || (w & 1) == 1 || (h & 1) == 1)
                {
                    throw new NotImplementedException();
                }
            }
            ret.bitmap = new byte[Math.Abs(dLineStride * h)];
            int sidx = GetByteOffset(x, y);
            int didx = ret.GetByteOffset(0, 0);
            int sidx2 = GetByteOffset(x + w, y);
            int copyLen = sidx2 - sidx;
            for (int i=0; i<h; ++i)
            {
                for (int j=0; j< copyLen; ++j)
                {
                    ret.bitmap[didx + j] = bitmap[sidx + j];
                }
                sidx += sLineStride;
                didx += dLineStride;
            }
            return ret;
        }

        private void Paste_4BitCount(int x, int y, Bmp src)
        {
            int sw = src.bmih.biWidth;
            int sh = src.bmih.biHeight;
            int dw = bmih.biWidth;
            int dh = bmih.biHeight;
            if (x < 0 || y < 0 || x + sw > dw || y + sh > dh || (x & 1) == 1 || (sw & 1) == 1)
            {
                throw new NotImplementedException();
            }
            int sLineStride = src.GetLineStride();
            int dLineStride = GetLineStride();
            int sidx = src.GetByteOffset(0, 0);
            int didx = GetByteOffset(x, y);
            int sw2 = sw / 2;
            for (int i = 0; i < sh; ++i)
            {
                for (int j = 0; j < sw2; ++j)
                {
                    byte scidx = src.bitmap[sidx + j];
                    RGBQUAD sc0 = src.colorTable[scidx >> 4];
                    RGBQUAD sc1 = src.colorTable[scidx & 0xF];
                    int dcidx0 = 0;
                    int dcidx1 = 0;
                    for (int k = 0; k < colorTable.Count(); ++k)
                    {
                        RGBQUAD dc = colorTable[k];
                        if (sc0.Equals(dc))
                        {
                            dcidx0 = k;
                        }
                        if (sc1.Equals(dc))
                        {
                            dcidx1 = k;
                        }
                    }
                    bitmap[didx + j] = (byte)((dcidx0 << 4) | dcidx1);
                }
                sidx += sLineStride;
                didx += dLineStride;
            }
        }

        private void Paste_8BitCount(int x, int y, Bmp src)
        {
            int sw = src.bmih.biWidth;
            int sh = src.bmih.biHeight;
            int dw = bmih.biWidth;
            int dh = bmih.biHeight;
            if (x < 0 || y < 0 || x + sw > dw || y + sh > dh || (x & 1) == 1 || (sw & 1) == 1)
            {
                throw new NotImplementedException();
            }
            int sLineStride = src.GetLineStride();
            int dLineStride = GetLineStride();
            int sidx = src.GetByteOffset(0, 0);
            int didx = GetByteOffset(x, y);
            for (int i = 0; i < sh; ++i)
            {
                for (int j = 0; j < sw; ++j)
                {
                    RGBQUAD sc = src.colorTable[src.bitmap[sidx + j]];
                    for (int k = 0; k < colorTable.Count(); ++k)
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

        public void Paste(int x, int y, Bmp src)
        {
            int sbc = src.bmih.biBitCount;
            int dbc = bmih.biBitCount;
            if (sbc == 4 && dbc == 4)
            {
                Paste_4BitCount(x, y, src);
            }
            else if (sbc == 8 && dbc == 8)
            {
                Paste_8BitCount(x, y, src);
            }
            else
            {
                throw new NotImplementedException();
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
            using (var writer = new BinaryWriter(File.Open(fileName, FileMode.Create)))
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
                    throw new NotImplementedException();
                }

                bmp.colorTable = new RGBQUAD[numQuads];
                for (int i = 0; i < numQuads; ++i)
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
        } // FromFile
    } // class Bmp
}
