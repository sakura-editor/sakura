#!/usr/bin/env python3
# Copyright (C) 2026, Sakura Editor Organization
# SPDX-License-Identifier: Zlib
"""从 Windows 资源文件（sakura_rc.rc）的 STRINGTABLE 生成 Mac 版的字符串表（C++ 源文件）。

Windows 版的 sakura.exe 内含日文字符串，CSelectLang::LoadStringW 从中读取；
Mac 版在没有注册其他语言的读取函数时，用这里生成的日文字符串表，结果与 Windows 版相同。

用法: rc_strings.py <sakura_rc.rc> <输出.cpp> <定义 ID 的头文件>...
"""
import re
import sys


def read_defines(paths):
    defines = {}
    for p in paths:
        with open(p, encoding='utf-8-sig', errors='replace') as f:
            for line in f:
                m = re.match(r'\s*#define\s+(\w+)\s+\(?\s*(0x[0-9A-Fa-f]+|\d+)\s*\)?\s*(?://.*)?$', line)
                if m:
                    defines[m.group(1)] = int(m.group(2), 0)
    return defines


def unescape_rc(s):
    """rc 的字符串转义（"" → "，\\n、\\t、\\xhh、八进制等）"""
    out = []
    i = 0
    while i < len(s):
        c = s[i]
        if c == '"' and i + 1 < len(s) and s[i + 1] == '"':
            out.append('"'); i += 2; continue
        if c == '\\' and i + 1 < len(s):
            n = s[i + 1]
            simple = {'n': '\n', 'r': '\r', 't': '\t', 'a': '\a', '\\': '\\', '"': '"'}
            if n in simple:
                out.append(simple[n]); i += 2; continue
            if n in 'xX':
                m = re.match(r'[0-9A-Fa-f]{1,4}', s[i + 2:])
                if m:
                    out.append(chr(int(m.group(0), 16))); i += 2 + len(m.group(0)); continue
            m = re.match(r'[0-7]{1,3}', s[i + 1:])
            if m:
                out.append(chr(int(m.group(0), 8))); i += 1 + len(m.group(0)); continue
            out.append(n); i += 2; continue
        out.append(c); i += 1
    return ''.join(out)


def to_cpp_literal(s):
    out = ['u"']
    for ch in s:
        o = ord(ch)
        if ch == '"': out.append('\\"')
        elif ch == '\\': out.append('\\\\')
        elif o < 0x20 or o == 0x7F: out.append('\\x%02X""' % o)
        elif o > 0xFFFF:
            o -= 0x10000
            out.append('\\x%04X\\x%04X""' % (0xD800 + (o >> 10), 0xDC00 + (o & 0x3FF)))
        else: out.append(ch)
    out.append('"')
    return ''.join(out)


def main():
    if len(sys.argv) < 4:
        print(__doc__, file=sys.stderr)
        return 2
    rc_path, out_path, headers = sys.argv[1], sys.argv[2], sys.argv[3:]
    raw = open(rc_path, 'rb').read()
    text = raw.decode('utf-16') if raw[:2] in (b'\xff\xfe', b'\xfe\xff') else raw.decode('utf-8-sig')
    text = text.replace('\r\n', '\n')
    defines = read_defines(headers)

    entries = {}
    for block in re.findall(r'^STRINGTABLE\b[^\n]*\nBEGIN\n(.*?)^END', text, re.M | re.S):
        for m in re.finditer(r'(\w+)\s*,?\s*"((?:[^"]|"")*)"', block):
            name, value = m.group(1), m.group(2)
            if name.isdigit():
                ident = int(name)
            elif name in defines:
                ident = defines[name]
            else:
                print(f'rc_strings.py: unknown id {name}', file=sys.stderr)
                return 1
            entries[ident] = unescape_rc(value)

    with open(out_path, 'w', encoding='utf-8', newline='\n') as f:
        f.write('// 由 macos/tools/rc_strings.py 根据 sakura_core/sakura_rc.rc 生成，请勿手动修改\n')
        f.write('#include <cstddef>\n\n')
        f.write('struct SakuraResourceString { unsigned int id; const char16_t* text; };\n')
        f.write('extern const SakuraResourceString g_sakuraResourceStrings[] = {\n')
        for ident in sorted(entries):
            f.write(f'\t{{ {ident}u, {to_cpp_literal(entries[ident])} }},\n')
        f.write('};\n')
        f.write(f'extern const std::size_t g_sakuraResourceStringCount = {len(entries)};\n')
    return 0


if __name__ == '__main__':
    sys.exit(main())
