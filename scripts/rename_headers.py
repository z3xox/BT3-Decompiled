#!/usr/bin/env python3
"""rename_headers.py MAP: renames headers (`old new` per line, battle/eft_y battle/eft_quad_2: no include/, no .h)
and updates every #include and the mentions of the file names in comments. Does not build.
rename_headers.py --fold EXTRA TARGET: puts the header EXTRA into TARGET (in front of TARGET's last #endif),
removes EXTRA and points its includers at TARGET. Does not build."""
import sys, os, re, glob, subprocess
R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def sh(*a): subprocess.run(a, cwd=R, check=True)
def files(): return glob.glob(f'{R}/src/**/*.c', recursive=True) + glob.glob(f'{R}/include/**/*.h', recursive=True)
def rd(f): return open(f, errors='surrogateescape').read()
def wr(f, s): open(f, 'w', errors='surrogateescape').write(s)

def rename(M):
    for a, b in M.items():
        assert os.path.exists(f'{R}/include/{a}.h'), a
        assert b in M or not os.path.exists(f'{R}/include/{b}.h'), 'taken: ' + b
    for a in M: sh('git', 'mv', f'include/{a}.h', f'include/{a}.h.renaming')
    for a, b in M.items():
        os.makedirs(os.path.dirname(f'{R}/include/{b}.h'), exist_ok=True)
        sh('git', 'mv', f'include/{a}.h.renaming', f'include/{b}.h')
    base = {os.path.basename(a) + '.h': os.path.basename(b) + '.h' for a, b in M.items()}
    tok = re.compile(r'(?<![\w/])(' + '|'.join(re.escape(k) for k in sorted(base, key=len, reverse=True)) + r')(?![\w])')
    for f in files():
        s = rd(f); t = s
        for a, b in sorted(M.items(), key=lambda x: -len(x[0])):
            t = t.replace(f'"{a}.h"', f'"{b}.h"').replace(f'include/{a}.h', f'include/{b}.h')
        t = tok.sub(lambda m: base[m.group(1)], t)
        if t != s: wr(f, t)

def fold(extra, target):
    e = rd(f'{R}/include/{extra}.h').rstrip('\n').split('\n'); h = rd(f'{R}/include/{target}.h').rstrip('\n').split('\n')
    # the extra header without its include guard and without an include of the target
    gi = next(i for i, l in enumerate(e) if l.startswith('#ifndef'))
    assert e[gi + 1].startswith('#define'), 'no guard'
    ei = max(i for i, l in enumerate(e) if l.startswith('#endif'))
    body = e[:gi] + e[gi + 2:ei]
    body = [l for l in body if l.strip() != f'#include "{target}.h"']
    hi = max(i for i, l in enumerate(h) if l.startswith('#endif'))
    h = h[:hi] + ['', f'/* ======== formerly {os.path.basename(extra)}.h ======== */', ''] + body + [''] + h[hi:]
    wr(f'{R}/include/{target}.h', '\n'.join(h) + '\n')
    sh('git', 'rm', '-q', f'include/{extra}.h')
    for f in files():
        s = rd(f)
        if f'"{extra}.h"' not in s: continue
        L = s.split('\n'); has = any(l.strip() == f'#include "{target}.h"' for l in L) or f.endswith(f'include/{target}.h')
        out = []
        for l in L:
            if l.strip() == f'#include "{extra}.h"':
                if has: continue
                l = f'#include "{target}.h"'; has = True
            out.append(l)
        wr(f, '\n'.join(out))

if __name__ == '__main__':
    if sys.argv[1] == '--fold':
        fold(sys.argv[2], sys.argv[3])
    else:
        rename({a: b for a, b in (l.split() for l in open(sys.argv[1]) if l.strip() and not l.startswith('#'))})
