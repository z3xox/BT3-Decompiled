#!/usr/bin/env python3
"""rename_fields.py LIST...: renames structure fields from lists (tab-separated:
file with the definition, struct name, old field, new field or KEEP, HIGH|MEDIUM|KEEP, evidence).

A field name such as `unk98` exists in many structures, so the uses cannot be found by text. They are found by
the compiler: the field is renamed in the definition, the files that see the header are checked
(`-fsyntax-only`), and every line the compiler rejects with "no member named `unk98'" is a use of THIS field.
One structure at a time. A structure whose uses cannot all be resolved is put back as it was and reported.
Does not build the game: run configure + ninja afterwards."""
import sys, re, os, glob, subprocess, collections
from concurrent.futures import ThreadPoolExecutor
R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CC = f'{R}/tools/ee-gcc2.96/bin/ee-gcc'
def rd(f): return open(f, errors='surrogateescape').read()
def wr(f, s): open(f, 'w', errors='surrogateescape').write(s)

def include_graph():
    inc = {}
    for f in glob.glob(f'{R}/src/**/*.c', recursive=True) + glob.glob(f'{R}/include/**/*.h', recursive=True):
        inc[os.path.relpath(f, R)] = set('include/' + m for m in re.findall(r'#include "([\w/.]+)"', rd(f)))
    return inc
def dependents(inc, target):
    """the C files that see `target` (a header, through any chain of includes), or the C file itself"""
    if target.endswith('.c'):
        return [target]
    seen = {target}; grew = True
    while grew:
        grew = False
        for f, deps in inc.items():
            if f not in seen and deps & seen:
                seen.add(f); grew = True
    return sorted(f for f in seen if f.endswith('.c'))

def check(files):
    def one(f):
        r = subprocess.run([CC, '-fsyntax-only', '-Iinclude', f], cwd=R, capture_output=True, text=True)
        return r.stderr
    with ThreadPoolExecutor(12) as ex:
        return '\n'.join(ex.map(one, files))

def macro_fix(words, old, new, touched):
    """renames `->old` / `.old` in the body of a #define whose name is one of `words` (the tokens of a line the
    compiler rejected); the definition may be in any header or C file and may run over several lines"""
    for f in glob.glob(f'{R}/include/**/*.h', recursive=True) + glob.glob(f'{R}/src/**/*.c', recursive=True):
        t = rd(f)
        if old not in t or '#define' not in t: continue
        L = t.split('\n'); i = 0; changed = False
        while i < len(L):
            m = re.match(r'\s*#\s*define\s+(\w+)', L[i])
            if m:
                j = i
                while L[j].rstrip().endswith('\\') and j + 1 < len(L): j += 1
                if m.group(1) in words:
                    for k in range(i, j + 1):
                        nl = re.sub(r'(->|\.)\s*' + re.escape(old) + r'\b', lambda x: x.group(1) + new, L[k])
                        if nl != L[k]: L[k] = nl; changed = True
                i = j + 1
            else:
                i += 1
        if changed:
            rel = os.path.relpath(f, R)
            if rel not in touched: touched[rel] = t
            wr(f, '\n'.join(L))

def struct_span(text, name):
    """(start, end) of the body of `struct name { ... }` or `typedef struct ... { ... } name;`"""
    for m in re.finditer(r'\b(?:struct|union)\s+(\w+)?\s*\{', text):
        depth = 0; i = m.end() - 1
        while i < len(text):
            if text[i] == '{': depth += 1
            elif text[i] == '}':
                depth -= 1
                if depth == 0: break
            i += 1
        tail = re.match(r'\s*(\w+)?', text[i + 1:i + 80])
        if m.group(1) == name or (tail and tail.group(1) == name):
            return m.end(), i
    return None

def main(lists):
    rows = []
    for l in (x for f in lists for x in rd(f).split('\n')):
        p = [x.strip() for x in l.split('\t')]
        if len(p) >= 5 and p[4] in ('HIGH', 'MEDIUM') and p[3] not in ('KEEP', '') and p[2] != p[3]:
            rows.append(p[:4])
    groups = collections.OrderedDict()
    for f, s, a, b in rows:
        groups.setdefault((f, s), {})[a] = b
    inc = include_graph(); done = failed = fields = 0; report = []
    for (f, s), M in groups.items():
        path = f'{R}/{f}'
        if not os.path.exists(path):
            report.append(f'{f} {s}: no such file'); failed += 1; continue
        text = rd(path); span = struct_span(text, s)
        if not span:
            report.append(f'{f} {s}: structure not found'); failed += 1; continue
        body = text[span[0]:span[1]]
        missing = [a for a in M if not re.search(r'\b' + re.escape(a) + r'\b', body)]
        clash = [b for b in M.values() if re.search(r'\b' + re.escape(b) + r'\b', body)]
        M = {a: b for a, b in M.items() if a not in missing and b not in clash}
        if missing or clash:
            report.append(f'{f} {s}: not in the structure {missing}; name already there {clash}')
        if not M:
            continue
        deps = dependents(inc, f)
        before = {d: rd(f'{R}/{d}') for d in deps}; before[f] = text
        touched = {}
        tok = re.compile(r'\b(' + '|'.join(re.escape(a) for a in sorted(M, key=len, reverse=True)) + r')\b')
        wr(path, text[:span[0]] + tok.sub(lambda m: M[m.group(1)], body) + text[span[1]:])
        ok = False
        for rnd in range(12):
            err = check(deps)
            hits = re.findall(r'^((?:src|include)/[\w/.]+):(\d+): (?:structure|union) has no member named `(\w+)\'', err, re.M)
            hits = [(ff, int(n), a) for ff, n, a in hits]
            mine = [(ff, n, a) for ff, n, a in hits if a in M]
            back = [(ff, n, a) for ff, n, a in hits if a in M.values()]   # a same-named field of another structure was renamed on a shared line
            if not mine and not back:
                ok = not re.search(r'^(?:src|include)/[\w/.]+:\d+: (?!warning)', err, re.M) or all(
                    'no member named' not in l for l in err.split('\n'))
                break
            byfile = collections.defaultdict(set)
            for ff, n, a in mine: byfile[ff].add((n, a, M[a]))
            inv = {v: k for k, v in M.items()}
            for ff, n, a in back: byfile[ff].add((n, a, inv[a]))
            progress = False
            for ff, items in byfile.items():
                macros = []
                for n, a, b in sorted(items):
                    L = rd(f'{R}/{ff}').split('\n'); line = L[n - 1]
                    occ = [m for m in re.finditer(r'(->|\.)\s*' + re.escape(a) + r'\b', line)]
                    if len(occ) == 1:
                        L[n - 1] = line[:occ[0].start()] + occ[0].group(1) + b + line[occ[0].end():]
                        wr(f'{R}/{ff}', '\n'.join(L)); progress = True
                    elif len(occ) > 1:
                        # several fields of this name on the line, perhaps of different structures: the subset the compiler accepts
                        best = None
                        for mask in range(1, 1 << len(occ)):
                            t = line
                            for k in reversed(range(len(occ))):
                                if mask >> k & 1:
                                    t = t[:occ[k].start()] + occ[k].group(1) + b + t[occ[k].end():]
                            L[n - 1] = t; wr(f'{R}/{ff}', '\n'.join(L))
                            e = check([ff])
                            if not re.search(r':%d: (?:structure|union) has no member named `(?:%s|%s)\'' % (n, re.escape(a), re.escape(b)), e):
                                best = t; break
                        L[n - 1] = best if best is not None else line
                        wr(f'{R}/{ff}', '\n'.join(L)); progress = progress or best is not None
                    else:
                        macros.append((set(re.findall(r'[A-Za-z_]\w*', line)), n, a, b))
                for words, n, a, b in macros:
                    before_txt = {x: rd(x) for x in glob.glob(f'{R}/include/**/*.h', recursive=True) + [f'{R}/{ff}']}
                    macro_fix(words, a, b, touched)
                    if all(rd(x) == t for x, t in before_txt.items()):
                        # not in a macro's body: the name itself is handed to a macro (`STG_RELOC(list88)`)
                        L = rd(f'{R}/{ff}').split('\n')
                        t = re.sub(r'\b' + re.escape(a) + r'\b', b, L[n - 1])
                        if t != L[n - 1]:
                            L[n - 1] = t; wr(f'{R}/{ff}', '\n'.join(L)); progress = True
                    else:
                        progress = True
            if not progress:
                break
        else:
            ok = False
        if ok:
            err = check(deps)
            ok = 'no member named' not in err
        if ok:
            done += 1; fields += len(M)
        else:
            why = [l for l in check(deps).split('\n') if re.search(r':\d+: (?!warning)', l)][:3]
            for d, t in before.items(): wr(f'{R}/{d}', t)
            for d, t in touched.items(): wr(f'{R}/{d}', t)
            report.append(f'{f} {s}: put back ({len(M)} fields): ' + ' ;; '.join(w[-110:] for w in why))
            failed += 1
    print(f'{fields} fields renamed in {done} structures; {failed} structures not done')
    for r in report: print('  ' + r)

if __name__ == '__main__':
    main(sys.argv[1:])
