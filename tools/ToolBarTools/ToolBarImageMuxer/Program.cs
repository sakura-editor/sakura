using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
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

    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length < 2)
            {
                return;
            }
            var inputDir = args[0];
            var outfile = args[1];
            var files = Directory.GetFiles(inputDir, "*.bmp", SearchOption.TopDirectoryOnly);

            var compare = new BmpFileComparer();
            Array.Sort<string>(files, compare);

            BmpMuxer.Mux(files, outfile, 33);
        }
    }
}
