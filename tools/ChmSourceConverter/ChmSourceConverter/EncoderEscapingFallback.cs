/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
