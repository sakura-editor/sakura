# Copyright (C) 2026, Sakura Editor Organization
# SPDX-License-Identifier: Zlib
import re, sys, json
src = open(sys.argv[1], encoding='utf-8-sig').read().splitlines()
zh = {}
for l in open(sys.argv[2], encoding='utf-16'):
    m = re.match(r'\s+(F_\w+)\s+"(.*)"', l)
    if m: zh.setdefault(m.group(1), m.group(2))
sec = None; rows = []
for l in src:
    m = re.match(r'^//\s*(\S.*?系|Main Menu 特殊機能|支援|カスタムメニュー|その他)\s*$', l)
    if m: sec = m.group(1); continue
    if l.startswith('//\tメニューからは直接呼ばれない'): sec = '間接呼び出し'; continue
    m = re.match(r'^(F_\w+)\s*=\s*(\d+)\s*,\s*(?://\s*(.*))?', l)
    if not m: continue
    name, code, cm = m.group(1), int(m.group(2)), (m.group(3) or '').strip()
    if code < 30000 and code not in range(20000,22000): continue
    if name.endswith(('_FIRST','_LAST')) or 'NOT_USED' in name or name.startswith('F_FUNCTION'): continue
    s = sec
    if code >= 40000: s = 'マクロ関数'
    cm = re.split(r'\t+', cm)[0].strip()
    rows.append(dict(name=name, code=code, sec=s, ja=cm, zh=zh.get(name,'')))
json.dump(rows, sys.stdout, ensure_ascii=False)
