#!/usr/bin/env python3
"""try_merge.py A B [C ...]: in the decomp tree, append the C files B, C, ... to A (in that order), give A their
data subsegments in the build lists, build, and say whether the outputs still equal the disc's. Restores the
tree afterwards (git checkout). Files are given as battle/eft_x_c (no src/, no .c)."""
import sys, subprocess, re, os
R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def run(cmd): return subprocess.run(cmd, cwd=R, shell=True, capture_output=True, text=True)
def main(files, keep=False):
    a, rest = files[0], files[1:]
    src = open(f'{R}/src/{a}.c').read()
    for b in rest:
        src += f'\n\n/* ======== merged from src/{b}.c ======== */\n\n' + open(f'{R}/src/{b}.c').read()
        os.remove(f'{R}/src/{b}.c')
    # the later parts' #include lines go up to the first part's, so that every declaration is seen before any code
    lines = src.split('\n'); mark = [i for i, l in enumerate(lines) if l.startswith('/* ======== merged from')]
    first_inc = max([i for i, l in enumerate(lines[:mark[0]]) if l.startswith('#include')] or [0])
    late = [l for i, l in enumerate(lines) if i > mark[0] and l.startswith('#include')]
    seen = set(l for l in lines[:mark[0]] if l.startswith('#include'))
    late = [l for k, l in enumerate(late) if l not in seen and l not in late[:k]]
    lines = [l for i, l in enumerate(lines) if not (i > mark[0] and l.startswith('#include'))]
    lines[first_inc + 1:first_inc + 1] = late
    open(f'{R}/src/{a}.c', 'w').write('\n'.join(lines))
    for y in ('config/SLUS_216.78.yaml', 'config/DBZP.yaml'):
        out = []
        for l in open(f'{R}/{y}').read().split('\n'):
            m = re.match(r'^(\s*- \[0x[0-9A-Fa-f]+, )([.\w]+)(, )([\w/]+)(\].*)$', l)
            if m and m.group(4) in rest:
                if m.group(2) == 'c':
                    continue  # the second file's code follows the first's in the same object
                l = m.group(1) + m.group(2) + m.group(3) + a + m.group(5)
            out.append(l)
        open(f'{R}/{y}', 'w').write('\n'.join(out))
    run('rm -f build/SLUS_216.78.rom build/DBZP.BIN build/SLUS_216.78.elf build/DBZP.elf')
    r = subprocess.run('.venv/bin/python configure.py', cwd=R, shell=True, capture_output=True, text=True)
    dropped = 0
    for rnd in range(40):  # a function declared twice with different types: the earlier declaration in the C file goes
        c = subprocess.run(f'ninja build/src/{a}.o', cwd=R, shell=True, capture_output=True, text=True)
        if c.returncode == 0:
            break
        out = c.stdout + c.stderr
        # pairs of (place of the conflicting declaration, place of the previous one), by symbol
        D = re.findall(r'^((?:src|include)/[\w/.]+):(\d+): (conflicting types for|previous declaration of|redefinition of|redeclaration of) `(\w+)\'', out, re.M)
        mine = f'src/{a}.c'; by = {}
        for f, n, what, name in D:
            by.setdefault(name, []).append((f, int(n), what))
        Lc = open(f'{R}/src/{a}.c').read().split('\n')
        def is_decl(n):  # a declaration that can go: a prototype, or an `extern` variable; not a definition
            i = n - 1; j = i
            while j < len(Lc) and ';' not in Lc[j] and '{' not in Lc[j] and j - i < 6:
                j += 1
            t = ' '.join(Lc[i:j + 1])
            if '{' in t.split(';')[0]:
                return False
            return '(' in t or t.lstrip().startswith('extern')
        todo = set()
        for name, places in by.items():
            here = sorted(n for f, n, w in places if f == mine)
            other = [1 for f, n, w in places if f != mine]
            decls = [n for n in here if is_decl(n)]
            if other:
                todo.update(decls)                               # a header declares it
            elif len(decls) == len(here):
                todo.update(here[:-1])                           # only declarations: the last stays
            else:
                todo.update(decls)                               # a definition is there: the declarations go
        todo = sorted(todo, reverse=True)
        if not todo:
            break
        L = open(f'{R}/src/{a}.c').read().split('\n')
        for n in todo:
            i = n - 1; j = i
            while j < len(L) and ';' not in L[j] and j - i < 6:
                j += 1
            if '{' in ' '.join(L[i:j + 1]):
                continue  # a definition, not a declaration: left alone
            for k in range(i, j + 1):
                L[k] = '/* (merge: declared in a header) ' + L[k].replace('/*', '(').replace('*/', ')') + ' */'
            dropped += 1
        open(f'{R}/src/{a}.c', 'w').write('\n'.join(L))
    b = subprocess.run('ninja -k 0', cwd=R, shell=True, capture_output=True, text=True)
    text = r.stdout + r.stderr + b.stdout + b.stderr
    have1 = os.path.exists(f'{R}/build/SLUS_216.78.rom'); have2 = os.path.exists(f'{R}/build/DBZP.BIN')
    same1 = have1 and run('cmp build/SLUS_216.78.rom disc/SLUS_216.78.rom').returncode == 0
    same2 = have2 and run('cmp build/DBZP.BIN disc/BIN/DBZP.BIN').returncode == 0
    errs = [l.strip() for l in text.split('\n') if re.search(r'^(src|include)/[\w/.]+:\d+: (?!warning)', l.strip()) or re.search(r'undefined reference|multiple definition|ld:', l)]
    if r.returncode != 0 or not have1 or not have2:
        verdict = 'DOES NOT BUILD (%d error lines): %s' % (len(errs), ' ;; '.join(e[-120:] for e in errs[:3]))
    elif same1 and same2:
        verdict = 'MATCHES'
    else:
        n = 0
        if have1 and not same1:
            n = int(run("cmp -l build/SLUS_216.78.rom disc/SLUS_216.78.rom | wc -l").stdout.strip() or 0)
        elif have2:
            n = int(run("cmp -l build/DBZP.BIN disc/BIN/DBZP.BIN | wc -l").stdout.strip() or 0)
        verdict = 'BUILDS, %d BYTES DIFFER (%s)' % (n, 'main executable' if not same1 else 'overlay')
    print(f'{" + ".join(files)}: {verdict}' + (f'  [{dropped} declarations reconciled]' if dropped else ''), flush=True)
    if not keep:
        run('git checkout -- src config && git clean -fdq src config')
        run('.venv/bin/python configure.py > /dev/null 2>&1; ninja > /dev/null 2>&1')
    return verdict
if __name__ == '__main__':
    main([x for x in sys.argv[1:] if x != '--keep'], '--keep' in sys.argv)
