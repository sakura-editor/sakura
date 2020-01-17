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
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using Microsoft.VisualStudio.TestTools.UnitTesting;

using ChmSourceConverter;

namespace ChmSourceConverter
{
    [TestClass]
    public class EncoderEscapingFallbackTest
    {
        [TestMethod]
        public void ConstructNormally()
        {
            var fallback = new EncoderEscapingFallback("&#{0};");
            Assert.IsInstanceOfType(fallback, typeof(EncoderFallback));
        }

        [TestMethod]
        [ExpectedException(typeof(ArgumentNullException))]
        public void ConstructWithNullSimple()
        {
            new EncoderEscapingFallback(null);
        }

        [TestMethod]
        public void ConstructWithNull()
        {
            var ex = Assert.ThrowsException<ArgumentNullException>(() => new EncoderEscapingFallback(null));
            Assert.IsInstanceOfType(ex, typeof(ArgumentException));
        }

        [TestMethod]
        public void ConstructWithEmptyFormat()
        {
            var ex = Assert.ThrowsException<ArgumentException>(() => new EncoderEscapingFallback(string.Empty));
            Assert.IsInstanceOfType(ex, typeof(ArgumentException));
        }

        [TestMethod]
        public void ConstructWithBadFormat()
        {
            var ex = Assert.ThrowsException<ArgumentException>(() => new EncoderEscapingFallback("&{code};"));
            Assert.IsInstanceOfType(ex, typeof(ArgumentException));
        }

        [TestMethod]
        public void EncodeAsciiTest()
        {
            Encoding encoding = Encoding.GetEncoding("Shift_JIS",
                  new EncoderEscapingFallback("&#{0};"),
                  new DecoderExceptionFallback());

            var inputText = "<html lang\"ja\">";
            var inputChars = inputText.ToCharArray();
            var inputBytes = encoding.GetBytes(inputChars, 0, inputChars.Length);
            var escapedText = encoding.GetString(inputBytes);

            Assert.AreEqual(inputText, escapedText);
        }

        [TestMethod]
        public void EncodeOguyTest()
        {
            Encoding encoding = Encoding.GetEncoding("Shift_JIS",
                  new EncoderEscapingFallback("&#{0};"),
                  new DecoderExceptionFallback());

            var inputText = "森鷗外";
            var oguyChars = inputText.ToCharArray();
            var oguyBytes = encoding.GetBytes(oguyChars, 0, oguyChars.Length);
            var escapedOguy = encoding.GetString(oguyBytes);

            Assert.AreNotEqual(inputText, escapedOguy);
            Assert.IsTrue(Regex.IsMatch(escapedOguy, @"森&#\d+;外"));
        }
    }
}
