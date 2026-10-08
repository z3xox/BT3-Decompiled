#!/usr/bin/env python3
"""apply_name_list.py FILE...: applies name lists (tab-separated: old, new or KEEP, HIGH|MEDIUM|KEEP, evidence).
Renames whole words in src/, include/ and the symbol lists of config/; a placeholder that has no line in config/
(func_XXXXXXXX, D_XXXXXXXX: the address is in the name) gets one in config/symbols/named_2026_10.txt. MEDIUM names
are marked `// guess` there, as the project does. Does not build."""
import sys, re, glob, os
R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
rows = []
for f in sys.argv[1:]:
    for l in open(f).read().split('\n'):
        p = [x.strip() for x in l.split('\t')]
        if len(p) >= 3 and p[0] and p[2] in ('HIGH', 'MEDIUM') and p[1] != 'KEEP':
            rows.append((p[0], p[1], p[2], p[3] if len(p) > 3 else ''))
M = {a: b for a, b, _, _ in rows}
assert len(set(M.values())) == len(M)
cfgfiles = sorted(glob.glob(f'{R}/config/symbols/*.txt')) + sorted(glob.glob(f'{R}/config/symbol_addrs*.txt'))
cfg = {f: open(f).read() for f in cfgfiles}
alltxt = '\n'.join(cfg.values())
new_lines = []
for a, b, conf, why in rows:
    if re.search(r'^' + re.escape(a) + r'\s*=', alltxt, re.M):
        continue
    m = re.match(r'(func|D)_([0-9A-Fa-f]{6,8})$', a)
    if not m:
        print('no definition and no address in the name, skipped:', a); del M[a]; continue
    note = (' guess:' if conf == 'MEDIUM' else '') + ' ' + why[:110].replace(';', ',')  # (a line of a symbol list may hold one semicolon)
    # the evidence on a line of its own: splat reads `word:` in a trailing comment as an attribute
    new_lines.append(f'//{note}\n{b} = 0x{int(m.group(2), 16):08X};' + (' // type:func' if m.group(1) == 'func' else ''))
tok = re.compile(r'\b(' + '|'.join(re.escape(k) for k in sorted(M, key=len, reverse=True)) + r')\b')
guess = {b for a, b, conf, _ in rows if conf == 'MEDIUM' and a in M}
n = 0
hand = [x for x in glob.glob(f'{R}/src/**/*', recursive=True) if os.path.isfile(x) and not x.endswith(('.c', '.h'))]  # hand-kept assembly (src/vu1)
for f in glob.glob(f'{R}/src/**/*.c', recursive=True) + glob.glob(f'{R}/include/**/*.h', recursive=True) + hand + cfgfiles:
    s = open(f, errors='surrogateescape').read(); t = tok.sub(lambda m: M[m.group(1)], s)
    if f in cfg:  # mark the guesses on their definition lines
        out = []
        for l in t.split('\n'):
            m = re.match(r'(\w+)\s*=\s*0x[0-9A-Fa-f]+;', l)
            if m and m.group(1) in guess and 'guess' not in l:
                l = l + (' // guess' if '//' not in l else ' (guess)')
            out.append(l)
        t = '\n'.join(out)
    t = re.sub(r'^#define (\w+) \1[ \t]*(/\*.*\*/)?[ \t]*\n', '', t, flags=re.M)  # `#define sceMcInit func_...` after the rename
    if t != s:
        open(f, 'w', errors='surrogateescape').write(t); n += 1
if new_lines:
    p = f'{R}/config/symbols/named_2026_10.txt'
    head = '' if os.path.exists(p) else '// Names given on 2026-10-08 to functions and data that had only their address (func_XXXXXXXX, D_XXXXXXXX).\n// The evidence for each is one sentence here; "guess:" marks the ones rated uncertain.\n\n'
    open(p, 'a').write(head + '\n'.join(new_lines) + '\n')
print(f'{len(M)} names applied in {n} files; {len(new_lines)} new definition lines')
