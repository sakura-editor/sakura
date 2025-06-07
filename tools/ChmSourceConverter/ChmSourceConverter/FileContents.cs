/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
using System;
using System.Collections;
using System.Collections.Generic;
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
}
