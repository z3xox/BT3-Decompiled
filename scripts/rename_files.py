#!/usr/bin/env python3
"""rename_files.py MAP: renames C files of the tree (and the header of the same name, where there is one).
MAP is a text file of lines `old new` (battle/eft_y battle/eft_quad_2: no src/, no .c). Updates the build
lists, the INCLUDE_ASM paths, the #include lines and mentions of the file names in comments. Does not build."""
import sys, os, re, glob, subprocess
R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def sh(*a): subprocess.run(a, cwd=R, check=True)
pairs = [l.split() for l in open(sys.argv[1]) if l.strip() and not l.startswith('#')]
M = {a: b for a, b in pairs}
assert len(set(M.values())) == len(M), 'two files would get one name'
for a, b in M.items():
    assert os.path.exists(f'{R}/src/{a}.c'), a
    assert b in M or not os.path.exists(f'{R}/src/{b}.c'), 'taken: ' + b
# 1. the C files (through a temporary name: a new name may be another file's old one)
for a in M:
    sh('git', 'mv', f'src/{a}.c', f'src/{a}.c.renaming')
for a, b in M.items():
    os.makedirs(os.path.dirname(f'{R}/src/{b}.c'), exist_ok=True)
    sh('git', 'mv', f'src/{a}.c.renaming', f'src/{b}.c')
# 2. the header of the same name
H = {}
for a, b in M.items():
    if os.path.exists(f'{R}/include/{a}.h'):
        H[a] = b
for a in H:
    sh('git', 'mv', f'include/{a}.h', f'include/{a}.h.renaming')
left = []
for a, b in list(H.items()):
    if os.path.exists(f'{R}/include/{b}.h'):
        sh('git', 'mv', f'include/{a}.h.renaming', f'include/{a}.h'); left.append(a); del H[a]; continue
    os.makedirs(os.path.dirname(f'{R}/include/{b}.h'), exist_ok=True)
    sh('git', 'mv', f'include/{a}.h.renaming', f'include/{b}.h')
# 3. the text of every source file
base_c = {os.path.basename(a) + '.c': os.path.basename(b) + '.c' for a, b in M.items()}
base_h = {os.path.basename(a) + '.h': os.path.basename(b) + '.h' for a, b in H.items()}
full = {}
for a, b in M.items():
    full[f'src/{a}.c'] = f'src/{b}.c'
    full[f'asm/nonmatchings/{a}"'] = f'asm/nonmatchings/{b}"'
for a, b in H.items():
    full[f'"{a}.h"'] = f'"{b}.h"'
    full[f'include/{a}.h'] = f'include/{b}.h'
tok = re.compile(r'(?<![\w/])(' + '|'.join(re.escape(k) for k in sorted(list(base_c) + list(base_h), key=len, reverse=True)) + r')(?![\w])')
def fix(text):
    for k in sorted(full, key=len, reverse=True):
        text = text.replace(k, full[k])
    return tok.sub(lambda m: base_c.get(m.group(1)) or base_h[m.group(1)], text)
for f in glob.glob(f'{R}/src/**/*.c', recursive=True) + glob.glob(f'{R}/include/**/*.h', recursive=True):
    s = open(f, errors='surrogateescape').read(); t = fix(s)
    if t != s:
        open(f, 'w', errors='surrogateescape').write(t)
# 4. the build lists
for y in ('config/SLUS_216.78.yaml', 'config/DBZP.yaml'):
    out = []
    for l in open(f'{R}/{y}').read().split('\n'):
        m = re.match(r'^(\s*- \[0x[0-9A-Fa-f]+, [.\w]+, )([\w/]+)(\].*)$', l)
        d = re.match(r'^(\s*- \{start: 0x[0-9A-Fa-f]+, type: [.\w]+, name: )([\w/]+)(.*)$', l)
        if m and m.group(2) in M:
            l = m.group(1) + M[m.group(2)] + m.group(3)
        elif d and d.group(2) in M:
            l = d.group(1) + M[d.group(2)] + d.group(3)
        out.append(l)
    open(f'{R}/{y}', 'w').write('\n'.join(out))
print(f'{len(M)} C files renamed, {len(H)} headers with them; headers left under their old name because the new one is taken: {left}')
