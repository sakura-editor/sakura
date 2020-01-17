/*
	Copyright (C) 2018-2020 Sakura Editor Organization

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
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace ChmSourceConverter
{
    /// <summary>
    /// テキストファイルを開いて行データを列挙するクラス
    /// </summary>
    public class FileContents : IEnumerable<string>, IDisposable
    {
        /// <summary>
        /// 行データの列挙オブジェクト
        /// </summary>
        private IEnumerator<string> LineEnumerator;

        /// <summary>
        /// コンストラクタ
        /// </summary>
        /// <param name="filename"></param>
        /// <param name="encoding"></param>
        public FileContents(string filename, Encoding encoding)
        {
            LineEnumerator = new LineEnumerator(filename, encoding);
        }

        /// <inheritdoc />
        public IEnumerator<string> GetEnumerator()
        {
            return LineEnumerator;
        }

        /// <inheritdoc />
        IEnumerator IEnumerable.GetEnumerator()
        {
            return LineEnumerator;
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
                    LineEnumerator.Dispose();
                    LineEnumerator = null;
                }

                disposedValue = true;
            }
        }

        ~FileContents()
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
