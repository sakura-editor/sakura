/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace ChmSourceConverter
{
    /// <summary>
    /// 行データを列挙するオブジェクトのクラス
    /// </summary>
    internal class LineEnumerator : IEnumerator<string>
    {
        /// <summary>
        /// ストリームリーダー
        /// </summary>
        private StreamReader Reader;

        /// <summary>
        /// 現在行の行データ
        /// </summary>
        private string CurrentLine;

        /// <inheritdoc />
        public string Current => CurrentLine;

        /// <inheritdoc />
        object IEnumerator.Current => CurrentLine;

        /// <summary>
        /// コンストラクタ
        /// </summary>
        /// <param name="filename"></param>
        /// <param name="encoding"></param>
        public LineEnumerator(string filename, Encoding encoding)
        {
            Reader = new StreamReader(filename, encoding, true);
        }

        /// <inheritdoc />
        public bool MoveNext()
        {
            if (Reader.EndOfStream) return false;
            CurrentLine = Reader.ReadLine();
            return true;
        }

        /// <inheritdoc />
        public void Reset()
        {
            CurrentLine = null;
            Reader.BaseStream.Seek(0, SeekOrigin.Begin);
        }

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    // dispose managed state (managed objects).
                    Reader.Dispose();
                    Reader = null;
                }

                disposedValue = true;
            }
        }

        ~LineEnumerator()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(false);
        }

        // This code added to correctly implement the disposable pattern.
        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(true);
            GC.SuppressFinalize(this);
        }
        #endregion
    }
}
