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

            BmpMuxer.Mux(files, outfile, 33);
        }
    }
}
