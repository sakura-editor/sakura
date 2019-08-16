/*
	Copyright (C) 2018-2019 Sakura Editor Organization

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
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace ChmSourceConverter
{
    /// <summary>
    /// 変換アプリ本体
    /// </summary>
    internal class ChmSourceConverterApp
    {
        /// <summary>
        /// 設定
        /// </summary>
        private readonly Properties.Settings Settings;

        /// <summary>
        /// 変換対象の拡張子
        /// </summary>
        private readonly IEnumerable<string> TargetExtensions;

        /// <summary>
        /// 入力ファイルのエンコーディング
        /// </summary>
        private readonly Encoding InputEncoding;

        /// <summary>
        /// HTML中に現れる文字セット定義のパターン(正規表現)
        /// </summary>
        private readonly Regex HtmlCharsetPattern;

        /// <summary>
        /// コンストラクタ
        /// </summary>
        public ChmSourceConverterApp(Properties.Settings settings)
        {
            Settings = settings;

            TargetExtensions = Settings.TargetExtensions.Cast<string>().ToList();
            InputEncoding = Encoding.GetEncoding(Settings.InputEncoding);
            HtmlCharsetPattern = new Regex(Settings.ReplacePattern, RegexOptions.IgnoreCase);
        }

        /// <summary>
        /// プログラムを開始します
        /// </summary>
        /// <param name="args"></param>
        public void Start(string[] args)
        {
            if (args.Length < 1 && string.IsNullOrEmpty(args.FirstOrDefault()))
                throw new ArgumentException("argument missing! Pass the target directory.", "args[0]");

            string baseDirectory = args.First();
            if (!Directory.Exists(baseDirectory))
                throw new ArgumentException("invalid argument! the target directory does not exist.", "args[0]");

            var files = Directory.EnumerateFiles(baseDirectory, "*.*", SearchOption.AllDirectories)
                .Where(file => TargetExtensions.Any(ext => file.EndsWith(ext)));

            ParallelOptions options = new ParallelOptions() { MaxDegreeOfParallelism = Settings.MaxDegreeOfParallelism };
            Parallel.ForEach(files, options, (file) => DoConvertFile(file));
        }

        /// <summary>
        /// ファイルを変換する
        /// </summary>
        /// <param name="filename"></param>
        /// <returns></returns>
        public void DoConvertFile(string filename)
        {
            // 作業用ストリームを生成する
            using (var memStream = new MemoryStream())
            {
                // 作業用ストリームにファイルを読み込む
                ReadFileIntoMemory(filename, memStream);

                // 作業用ストリームに蓄積したデータをbyte配列に書き出す
                using (var destStream = new FileStream(filename, FileMode.Truncate, FileAccess.Write, FileShare.None))
                    memStream.WriteTo(destStream);
            }
        }

        /// <summary>
        /// ファイルをメモリに書き込む
        /// </summary>
        /// <param name="filename"></param>
        /// <param name="stream"></param>
        private void ReadFileIntoMemory(string filename, Stream stream)
        {
            // 変換用エンコーディングを生成する
            Encoding outputEncoding = Encoding.GetEncoding(Settings.OutputEncoding,
                new EncoderEscapingFallback(Settings.EscapingFormat),
                new DecoderExceptionFallback());

            // テキストライターを使ってストリームに読み込む
            using (var writer = new StreamWriter(stream, outputEncoding, Settings.OutputBufferSize, true))
                ReadLinesIntoMemory(filename, writer);
        }

        /// <summary>
        /// ファイルから行を読み取ってメモリに書き込む
        /// </summary>
        /// <param name="filename"></param>
        /// <param name="writer"></param>
        private void ReadLinesIntoMemory(string filename, TextWriter writer)
        {
            bool IsHtml = Path.GetExtension(filename) == TargetExtensions.FirstOrDefault();

            // 入力ファイルから行データを読み取る
            using (var contents = new FileContents(filename, InputEncoding))
            {
                foreach (var line in contents)
                {
                    // コンテンツフィルターを適用する
                    if (IsHtml && HtmlCharsetPattern.IsMatch(line))
                    {
                        IsHtml = false;
                        writer.WriteLine(HtmlCharsetPattern.Replace(line, $"$1{Settings.OutputEncoding}$2"));
                        continue;
                    }
                    writer.WriteLine(line);
                }
            }
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            new ChmSourceConverterApp(Properties.Settings.Default).Start(args);
        }
    }
}
