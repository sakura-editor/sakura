using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections;
using System.Text.RegularExpressions;

namespace ToolBarImageMuxer
{
    public class BmpFileComparer : IComparer<string>
    {
        static Regex regex = new Regex(@"(\d+)(\D*)\.bmp", RegexOptions.Compiled);
    
        public int Compare(string x, string y)
        {
            var matchX = regex.Match(x);
            var matchY = regex.Match(y);

            if (matchX.Success && matchY.Success)
            {
                var indexX = int.Parse(matchX.Groups[1].Value);
                var indexY = int.Parse(matchY.Groups[1].Value);

				// 数値だけのファイルの場合、数値として比較する
                if (indexX != indexY)
                {
                    return indexX - indexY;
                }
                
                // 数値の部分が同じ場合、文字列として比較する。
            }
            return x.CompareTo(y);
        }
    }
}
