#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""阶段 1 第 15 步：批量替换字符类型。

把 C++ 源码里的
    L"..."            → LTEXT("...")      （带用户自定义后缀时 L"..."sv → LTEXT("..."sv)）
    LR"(...)"         → LTEXT(R"(...)")
    L'x'              → LCHAR('x')
    wchar_t           → WCHAR
    std::wstring      → sakura::wstring
    std::wstring_view → sakura::wstring_view
注释、字符串、字符字面量里的内容不会改动。数字里的单引号分隔符（1'000）会正确跳过。

Windows 上 LTEXT(x) 展开为 L##x，WCHAR 是 wchar_t，sakura::wstring 是 std::wstring，
所以替换前后 Windows 版的编译结果相同。方案见 macos/docs/phase1-char-type.md。

用法：python3 macos/tools/convert_wchar.py [--check] <文件或目录...>
  --check  只统计，不写回
"""
import os
import re
import sys

EXCLUDE = {
    os.path.normpath('sakura_core/util/wchar16.h'),   # 方案本身
    os.path.normpath('sakura_core/util/std_macro.h'), # LTEXT 的定义
}
EXTS = ('.cpp', '.h', '.hpp', '.inl')
PREFIXES = {'L', 'u', 'U', 'u8', 'R', 'LR', 'uR', 'UR', 'u8R'}
IDENT_START = re.compile(r'[A-Za-z_]')
IDENT = re.compile(r'[A-Za-z0-9_]*')
PPNUM = re.compile(r"(?:\.?[0-9])(?:[eEpP][+-]|'[0-9A-Za-z_]|[0-9A-Za-z_.])*")


def read_quoted(src, i, quote):
    """src[i] 是开头的引号。返回结尾引号之后的位置。"""
    j = i + 1
    n = len(src)
    while j < n:
        c = src[j]
        if c == '\\':
            j += 2
            continue
        if c == quote:
            return j + 1
        if c == '\n':  # 未闭合（不应出现）
            return j
        j += 1
    return j


def read_raw(src, i):
    """src[i] 是 R"delim( 的引号。返回结尾 )delim" 之后的位置。"""
    k = src.index('(', i)
    delim = src[i + 1:k]
    end = src.index(')' + delim + '"', k)
    return end + len(delim) + 2


def convert(src, stats):
    out = []
    i = 0
    n = len(src)
    while i < n:
        c = src[i]
        two = src[i:i + 2]
        if two == '//':
            j = src.find('\n', i)
            j = n if j < 0 else j
            out.append(src[i:j]); i = j; continue
        if two == '/*':
            j = src.find('*/', i + 2)
            j = n if j < 0 else j + 2
            out.append(src[i:j]); i = j; continue
        if c == '"' or c == "'":
            j = read_quoted(src, i, c)
            out.append(src[i:j]); i = j; continue
        if c.isdigit() or (c == '.' and i + 1 < n and src[i + 1].isdigit()):
            m = PPNUM.match(src, i)
            out.append(m.group()); i = m.end(); continue
        if IDENT_START.match(c):
            m = IDENT.match(src, i + 1)
            ident = src[i:m.end()]
            j = m.end()
            nxt = src[j] if j < n else ''
            if ident in PREFIXES and nxt in ('"', "'"):
                if ident.endswith('R') and nxt == '"':
                    k = read_raw(src, j)
                else:
                    k = read_quoted(src, j, nxt)
                body = src[j:k]
                s = IDENT.match(src, k)          # 用户自定义字面量后缀
                suffix = s.group()
                k2 = s.end()
                if ident == 'L':
                    if nxt == '"':
                        out.append('LTEXT(' + body + suffix + ')'); stats['LTEXT'] += 1
                    else:
                        out.append('LCHAR(' + body + suffix + ')'); stats['LCHAR'] += 1
                elif ident == 'LR':
                    out.append('LTEXT(R' + body + suffix + ')'); stats['LTEXT(R)'] += 1
                else:
                    out.append(ident + body + suffix)
                i = k2
                continue
            if ident == 'wchar_t':
                out.append('WCHAR'); stats['wchar_t'] += 1
                i = j; continue
            if ident in ('wstring', 'wstring_view'):
                prev = ''.join(out[-3:])
                if prev.endswith('std::') and not prev.endswith('::std::'):
                    # 去掉已经输出的 "std::"
                    tail = ''.join(out)
                    out = [tail[:-5]]
                    out.append('sakura::' + ident); stats['std::' + ident] += 1
                    i = j; continue
            out.append(ident)
            i = j
            continue
        out.append(c)
        i += 1
    return ''.join(out)


def iter_files(paths):
    for p in paths:
        if os.path.isdir(p):
            for root, _, files in os.walk(p):
                for f in sorted(files):
                    if f.endswith(EXTS):
                        yield os.path.join(root, f)
        else:
            yield p


def main(argv):
    check = '--check' in argv
    paths = [a for a in argv if a != '--check']
    total = {}
    changed = 0
    for path in iter_files(paths):
        if os.path.normpath(path) in EXCLUDE:
            continue
        raw = open(path, 'rb').read()
        bom = raw.startswith(b'\xef\xbb\xbf')
        text = raw[3:].decode('utf-8') if bom else raw.decode('utf-8')
        stats = {k: 0 for k in ('LTEXT', 'LTEXT(R)', 'LCHAR', 'wchar_t', 'std::wstring', 'std::wstring_view')}
        # 改行コードを保つため newline='' 相当で処理する
        new = convert(text, stats)
        if new != text:
            changed += 1
            if not check:
                with open(path, 'wb') as f:
                    f.write((b'\xef\xbb\xbf' if bom else b'') + new.encode('utf-8'))
        for k, v in stats.items():
            total[k] = total.get(k, 0) + v
    print('files changed: %d' % changed)
    for k, v in total.items():
        print('  %-18s %d' % (k, v))


if __name__ == '__main__':
    main(sys.argv[1:])
