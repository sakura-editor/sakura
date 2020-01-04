using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace ToolBarImageMuxer
{
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

            var countPerLine = 32 + 1;
            BmpMuxer.Mux(files, outfile, countPerLine);
        }
    }
}
