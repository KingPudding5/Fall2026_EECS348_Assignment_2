#!/usr/bin/env python3
"""
session-report.py -- turn a GenAI CLI session log into hard numbers.

Usage:
    python3 session-report.py <session.jsonl> [--expect <folder>]

Reads a Codex CLI rollout log or a Claude Code session log and prints the
model, the working directory, wall-clock time, exact token spend, and a
numbered list of what the agent actually did.

--expect <folder> flags every file path the agent touched that lies OUTSIDE
that folder. That is the contamination check: it is how you prove an agent
stayed in its own sandbox instead of reading a neighbouring assignment.

Nothing here is self-reported by the model. Every number comes from the
harness's own telemetry, which the model does not write and cannot edit.
"""
import json, sys, os, re, datetime

def load(path):
    rows = []
    with open(path, encoding='utf-8') as fh:
        for line in fh:
            line = line.strip()
            if line:
                try: rows.append(json.loads(line))
                except json.JSONDecodeError: pass
    return rows

def detect(rows):
    for r in rows:
        if r.get('type') == 'session_meta':          return 'codex'
        if r.get('type') in ('assistant','user') and 'message' in r: return 'claude'
    return 'unknown'

def ts(s):
    if not s: return None
    try: return datetime.datetime.fromisoformat(s.replace('Z','+00:00'))
    except ValueError: return None

def fmt_dur(a, b):
    if not a or not b: return "unknown"
    s = int((b-a).total_seconds())
    return f"{s//60}m {s%60}s ({s} s)"

PATH_RE = re.compile(r'(?:/Users|/home)/[^\s"\'`,;)\]}\\]+')

def report_codex(rows, expect):
    meta = next((r['payload'] for r in rows if r.get('type')=='session_meta'), {})
    turn = next((r['payload'] for r in rows if r.get('type')=='turn_context'), {})
    print("SYSTEM        Codex CLI")
    print(f"  model       {turn.get('model') or meta.get('model','?')}"
          f"   (reasoning effort: {(turn.get('collaboration_mode') or {}).get('settings',{}).get('reasoning_effort','?')})")
    print(f"  cli version {meta.get('cli_version','?')}   originator: {meta.get('originator','?')}  source: {meta.get('source','?')}")
    print(f"  provider    {meta.get('model_provider','?')}")
    print(f"  cwd         {meta.get('cwd','?')}")
    sb = (turn.get('sandbox_policy') or {}).get('type','?')
    print(f"  sandbox     {sb}   approval policy: {turn.get('approval_policy','?')}")

    stamps = [ts(r.get('timestamp')) for r in rows if r.get('timestamp')]
    stamps = [s for s in stamps if s]
    start, end = (min(stamps), max(stamps)) if stamps else (None, None)
    print(f"\nWALL CLOCK    {fmt_dur(start,end)}")
    if start: print(f"  start       {start.isoformat()}")
    if end:   print(f"  end         {end.isoformat()}")

    tc = [r['payload'] for r in rows if (r.get('payload') or {}).get('type')=='token_count']
    if tc:
        t = (tc[-1].get('info') or {}).get('total_token_usage', {})
        print("\nTOKENS (provider-reported, whole session)")
        print(f"  input             {t.get('input_tokens',0):>9,}")
        print(f"    of which cached {t.get('cached_input_tokens',0):>9,}")
        print(f"  output            {t.get('output_tokens',0):>9,}")
        print(f"    of which reason {t.get('reasoning_output_tokens',0):>9,}")
        print(f"  TOTAL             {t.get('total_tokens',0):>9,}")
        billable = t.get('input_tokens',0)-t.get('cached_input_tokens',0)+t.get('output_tokens',0)
        print(f"  fresh (uncached)  {billable:>9,}   <- the honest 'work' figure")

    calls = [(r['timestamp'], r['payload']) for r in rows
             if r.get('type')=='response_item' and (r.get('payload') or {}).get('type')=='custom_tool_call']
    print(f"\nACTIONS       {len(calls)} tool call(s)")
    paths = set()
    for i,(t,p) in enumerate(calls,1):
        inp = p.get('input','') or ''
        paths.update(PATH_RE.findall(inp))
        print(f"\n  {i}. [{t}] {p.get('name')}")
        for line in summarize(inp): print(f"       {line}")
    return paths

def report_claude(rows, expect):
    model = cwd = None
    tin=tout=tcache=tcreate=0
    calls=[]; stamps=[]
    for r in rows:
        if r.get('cwd') and not cwd: cwd = r['cwd']
        t = ts(r.get('timestamp'))
        if t: stamps.append(t)
        m = r.get('message') or {}
        if m.get('model'): model = m['model']
        u = m.get('usage') or {}
        if u:
            tin += u.get('input_tokens',0); tout += u.get('output_tokens',0)
            tcache += u.get('cache_read_input_tokens',0)
            tcreate += u.get('cache_creation_input_tokens',0)
        for c in (m.get('content') or []):
            if isinstance(c,dict) and c.get('type')=='tool_use':
                calls.append((r.get('timestamp'), c.get('name'), c.get('input') or {}))
    print("SYSTEM        Claude Code")
    print(f"  model       {model or '?'}")
    print(f"  cwd         {cwd or '?'}")
    start,end = (min(stamps),max(stamps)) if stamps else (None,None)
    print(f"\nWALL CLOCK    {fmt_dur(start,end)}")
    if start: print(f"  start       {start.isoformat()}")
    if end:   print(f"  end         {end.isoformat()}")
    print("\nTOKENS (provider-reported, whole session)")
    print(f"  input             {tin:>9,}")
    print(f"  cache read        {tcache:>9,}")
    print(f"  cache write       {tcreate:>9,}")
    print(f"  output            {tout:>9,}")
    print(f"  TOTAL             {tin+tcache+tcreate+tout:>9,}")
    print(f"  fresh (uncached)  {tin+tcreate+tout:>9,}   <- the honest 'work' figure")
    print(f"\nACTIONS       {len(calls)} tool call(s)")
    paths=set()
    for i,(t,name,inp) in enumerate(calls,1):
        blob = json.dumps(inp)
        paths.update(PATH_RE.findall(blob))
        for k in ('file_path','path','notebook_path'):
            if inp.get(k): paths.add(inp[k])
        print(f"\n  {i}. [{t}] {name}")
        for line in summarize(inp.get('command') or inp.get('file_path') or blob): print(f"       {line}")
    return paths

EXEC_HINT = re.compile(r'\b(gcc|clang|cc|make|\./[\w.-]+|python3?|bash|sh)\b')

def summarize(text, width=100):
    text = text if isinstance(text,str) else json.dumps(text)
    text = ' '.join(text.split())
    out = [text[:width] + (' ...' if len(text)>width else '')]
    if EXEC_HINT.search(text): out.append(">> COMPILE/EXECUTE detected")
    return out

def main():
    if len(sys.argv) < 2:
        print(__doc__); sys.exit(1)
    path = sys.argv[1]
    expect = None
    if '--expect' in sys.argv:
        expect = os.path.abspath(os.path.expanduser(sys.argv[sys.argv.index('--expect')+1]))
    rows = load(path)
    kind = detect(rows)
    print("="*72)
    print(f"SESSION REPORT  --  {os.path.basename(path)}")
    print("="*72)
    if   kind=='codex':  paths = report_codex(rows, expect)
    elif kind=='claude': paths = report_claude(rows, expect)
    else:
        print("Unrecognised log format."); sys.exit(2)

    print("\n" + "-"*72)
    print("FILE PATHS TOUCHED")
    for p in sorted(paths): print(f"  {p}")
    if expect:
        outside = [p for p in sorted(paths) if not os.path.abspath(p).startswith(expect)]
        print(f"\nSANDBOX CHECK  expected folder: {expect}")
        if outside:
            print("  *** PATHS OUTSIDE THE EXPECTED FOLDER ***")
            for p in outside: print(f"    ! {p}")
        else:
            print("  OK - every path seen lies inside the expected folder.")
    print("-"*72)

if __name__ == '__main__':
    main()
