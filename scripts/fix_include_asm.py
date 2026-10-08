#!/usr/bin/env python3
"""Every INCLUDE_ASM of a C file names the folder splat writes that file's functions to:
asm/nonmatchings/<dir>/<file>. After a merge or a rename the lines of the part that moved still name the old
folder, which only exists in a tree that was split before. This rewrites them. Prints what it changed."""
import re, glob, os
R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
n = 0
for f in sorted(glob.glob(f'{R}/src/**/*.c', recursive=True)):
    own = 'asm/nonmatchings/' + os.path.relpath(f, f'{R}/src')[:-2]
    s = open(f, errors='surrogateescape').read()
    t, k = re.subn(r'(INCLUDE_ASM\(\s*")asm/nonmatchings/[^"]+(")', lambda m: m.group(1) + own + m.group(2), s)
    if t != s:
        open(f, 'w', errors='surrogateescape').write(t)
        c = sum(1 for a, b in zip(re.findall(r'INCLUDE_ASM\(\s*"([^"]+)"', s), re.findall(r'INCLUDE_ASM\(\s*"([^"]+)"', t)) if a != b)
        print(os.path.relpath(f, R), c); n += c
print(n, 'lines changed')
