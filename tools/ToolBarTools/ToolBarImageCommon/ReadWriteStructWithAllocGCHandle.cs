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
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ToolBarImageCommon
{
    // https://smdn.jp/programming/netfx/tips/convert_struct_and_bytearray/
    /// <summary>
    /// ポインタを得るためにGCHandle.Allocでバイト配列のピニングを行い、
    /// Marshal.PtrToStructure・Marshal.StructureToPtrで変換して構造体の読み書きを行う
    /// </summary>
    /// <remarks>参照型のフィールドを持つ構造体は読み書きできない</remarks>
    public class ReadWriteStructWithAllocGCHandle
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
}
