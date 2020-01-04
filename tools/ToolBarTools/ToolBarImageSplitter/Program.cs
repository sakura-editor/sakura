using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ToolBarImageSplitter
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length < 2)
            {
                return;
            }
            var input = args[0];
            var outdir = args[1];

            BmpSplitter.Split(input, outdir);
        }
    }
}
