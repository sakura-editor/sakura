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
