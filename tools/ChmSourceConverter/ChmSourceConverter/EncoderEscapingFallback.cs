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
using System.Linq;
using System.Security;
using System.Text;
using System.Text.RegularExpressions;

namespace ChmSourceConverter
{
    /// <summary>
    /// Provides a failure handling mechanism, called a fallback,
    /// for an input character that cannot be converted to an output byte sequence.
    /// The fallback uses a user-specified replacement string instead of the original input character.
    /// This class cannot be inherited.
    /// </summary>
    public sealed class EncoderEscapingFallback : EncoderFallback
    {
        /// <summary>
        /// the Escaping Format
        /// </summary>
        public string Format { get; private set; }

        /// <summary>
        /// construct the instance
        /// </summary>
        /// <param name="format">escaping format, should be including "{0}".</param>
        public EncoderEscapingFallback(string format)
            : base()
        {
            // validate format string.
            if (format == null)
                throw new ArgumentNullException("format");
            if (!Regex.IsMatch(format, @".*\{0.*\}.*"))
                throw new ArgumentException("bad format", "format");

            // test formatting
            string.Format(format, 1);

            // set internal member.
            Format = format;
        }

        /// <inheritdoc />
        public override int MaxCharCount => EncoderFallback.ExceptionFallback.MaxCharCount;

        /// <inheritdoc />
        public override EncoderFallbackBuffer CreateFallbackBuffer()
        {
            return new EncoderEscapingFallbackBuffer(this);
        }
    }

    /// <summary>
    /// Represents a substitute input string that is used when the original input character cannot be encoded.
    /// This class cannot be inherited.
    /// </summary>
    internal sealed class EncoderEscapingFallbackBuffer : EncoderFallbackBuffer
    {
        private ArraySegment<char> Buffer;
        private EncoderEscapingFallback FallbackRef;

        /// <inheritdoc />
        public override int Remaining => Buffer.Count();

        /// <summary>
        /// Initializes a new instance of the EncoderEscapingFallbackBuffer class
        /// using the value of a EncoderEscapingFallback object.
        /// </summary>
        /// <param name="fallback">A EncoderEscapingFallback object.</param>
        public EncoderEscapingFallbackBuffer(EncoderEscapingFallback fallback)
        {
            Buffer = new ArraySegment<char>(Array.Empty<char>());
            FallbackRef = fallback;
        }

        /// <summary>
        /// Prepares the escaping fallback buffer to use the current format string.
        /// </summary>
        /// <param name="charUnknown"></param>
        /// <returns></returns>
        private bool Fallback(int charUnknown)
        {
            var escapedChar = string.Format(FallbackRef.Format, charUnknown);
            Buffer = new ArraySegment<char>(escapedChar.ToCharArray());
            return true;
        }

        /// <inheritdoc />
        public override bool Fallback(char charUnknown, int index)
        {
            if (Buffer.Any())
            {
                throw new ArgumentException("This method is called again before the GetNextChar method has read all the replacement string characters.");
            }

            return Fallback((Int32)charUnknown);
        }

        /// <inheritdoc />
        public override bool Fallback(char charUnknownHigh, char charUnknownLow, int index)
        {
            if (Buffer.Any())
            {
                throw new ArgumentException("This method is called again before the GetNextChar method has read all the replacement string characters.");
            }
            if (charUnknownHigh < 0xD800 || charUnknownHigh > 0xD8FF)
            {
                throw new ArgumentOutOfRangeException("charUnknownHigh", charUnknownHigh, "The value of charUnknownHigh is out of range");
            }
            if (charUnknownLow < 0xDC00 || charUnknownLow > 0xDFFF)
            {
                throw new ArgumentOutOfRangeException("charUnknownLow", charUnknownLow, "The value of charUnknownLow is out of range");
            }

            int charUnknown = Char.ConvertToUtf32(charUnknownHigh, charUnknownLow);
            return Fallback(charUnknown);
        }

        /// <inheritdoc />
        public override char GetNextChar()
        {
            char ch = Buffer.FirstOrDefault();
            if (Buffer.Any())
            {
                Buffer = new ArraySegment<char>(Buffer.Array, Buffer.Offset + 1, Buffer.Count - 1);
            }
            return ch;
        }

        /// <inheritdoc />
        public override bool MovePrevious()
        {
            if (Buffer.Offset == 0) return false;
            Buffer = new ArraySegment<char>(Buffer.Array, Buffer.Offset - 1, Buffer.Count + 1);
            return true;
        }

        /// <summary>
        /// Initializes all internal state information and data in this instance of System.Text.EncoderReplacementFallbackBuffer.
        /// </summary>
        [SecuritySafeCritical]
        public override void Reset()
        {
            if (Buffer.Any())
            {
                Buffer = new ArraySegment<char>(Buffer.Array, 0, Buffer.Array.Length);
            }
        }
    }
}
