/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
using System;
using System.Linq;
using System.Security;
using System.Text;

namespace ChmSourceConverter
{
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
