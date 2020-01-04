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
        static Regex regex = new Regex(@"(\d+)\.bmp", RegexOptions.Compiled);
    
        public int Compare(string x, string y)
        {
            var matchX = regex.Match(x);
            var matchY = regex.Match(y);

            if (matchX.Success && matchY.Success)
            {
                var indexX = int.Parse(matchX.Groups[1].Value);
                var indexY = int.Parse(matchY.Groups[1].Value);

                return indexX.CompareTo(indexY);
            }
            return x.CompareTo(y);
        }
    }
}
