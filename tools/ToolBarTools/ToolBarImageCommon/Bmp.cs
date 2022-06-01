﻿/*
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

    public class Bmp
    {
        public BITMAPINFOHEADER bmih;
        public RGBQUAD[] colorTable;
        public byte[] imageData;

        public int GetLineStride()
        {
            int bitsPerLine = ((bmih.biWidth * bmih.biBitCount) + 31) & (~31);
            int bytesPerLine = bitsPerLine / 8;
            if (IsBottomUp())
                bytesPerLine = -bytesPerLine; // ボトムアップ形式の場合は行ストライドは負の値になる
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
                throw new ArgumentOutOfRangeException();
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
                    throw new ArgumentOutOfRangeException();
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
            ret.imageData = new byte[Math.Abs(dLineStride * h)];
            int sidx = GetByteOffset(x, y);
            int didx = ret.GetByteOffset(0, 0);
            int sidx2 = GetByteOffset(x + w, y);
            int copyLen = sidx2 - sidx;
            for (int i=0; i<h; ++i)
            {
                for (int j=0; j< copyLen; ++j)
                {
                    ret.imageData[didx + j] = imageData[sidx + j];
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
            int sLineStride = src.GetLineStride();
            int dLineStride = GetLineStride();
            int sidx = src.GetByteOffset(0, 0);
            int didx = GetByteOffset(x, y);
            int sw2 = sw / 2;
            for (int i = 0; i < sh; ++i)
            {
                for (int j = 0; j < sw2; ++j)
                {
                    byte scidx = src.imageData[sidx + j];
                    RGBQUAD sc0 = src.colorTable[scidx >> 4];
                    RGBQUAD sc1 = src.colorTable[scidx & 0xF];
                    int dcidx0 = Array.IndexOf(colorTable, sc0);
                    int dcidx1 = Array.IndexOf(colorTable, sc1);
                    imageData[didx + j] = (byte)((dcidx0 << 4) | dcidx1);
                }
                sidx += sLineStride;
                didx += dLineStride;
            }
        }

        private void Paste_8BitCount(int x, int y, Bmp src)
        {
            int sw = src.bmih.biWidth;
            int sh = src.bmih.biHeight;
            int sLineStride = src.GetLineStride();
            int dLineStride = GetLineStride();
            int sidx = src.GetByteOffset(0, 0);
            int didx = GetByteOffset(x, y);
            for (int i = 0; i < sh; ++i)
            {
                for (int j = 0; j < sw; ++j)
                {
                    RGBQUAD sc = src.colorTable[src.imageData[sidx + j]];
                    imageData[didx + j] = (byte)Array.IndexOf(colorTable, sc);
                }
                sidx += sLineStride;
                didx += dLineStride;
            }
        }

        public void Paste(int x, int y, Bmp src)
        {
            int sw = src.bmih.biWidth;
            int sh = src.bmih.biHeight;
            int dw = bmih.biWidth;
            int dh = bmih.biHeight;
            if (x < 0 || y < 0 || x + sw > dw || y + sh > dh || (x & 1) == 1 || (sw & 1) == 1)
            {
                throw new NotImplementedException();
            }

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
            fileSize += imageData.Count();
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
                writer.Write(imageData);
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
                    throw new FileFormatException();
                }
                bmp.bmih = ReadWriteStructWithAllocGCHandle.ReadFrom<BITMAPINFOHEADER>(reader);
                uint numQuads;
                if (bmp.bmih.biClrUsed == 0)
                {
                    switch (bmp.bmih.biBitCount)
                    {
                        case 1:
                        case 4:
                        case 8:
                            numQuads = (uint)(1 << bmp.bmih.biBitCount);
                            break;
                        default:
                            throw new FileFormatException();
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
                    throw new FileFormatException();
                }
                // https://stackoverflow.com/a/10038017/4699324
                using (var ms = new MemoryStream())
                {
                    reader.BaseStream.CopyTo(ms);
                    bmp.imageData = ms.ToArray();
                }
            }
            return bmp;
        } // FromFile
    } // class Bmp
}
