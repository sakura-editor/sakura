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
}
