#!/usr/bin/env bash
#
# run-tests.sh -- comparison bench for EECS 348 Assignment 2.
#
# PROVENANCE: written by Claude (Anthropic) at my direction on 2026-09-15 and
# reviewed by me before use. Descended from the Assignment 1 bench; sections 0,
# 1, 3 and 9 are carried over almost unchanged, the rest is new because this
# assignment has a real data structure in it and Assignment 1 did not.
#
# It is a measuring instrument, not a grader. It never decides pass or fail. It
# runs both programs through identical inputs and prints exactly what each did.
# Every claim in my analysis comes from this output.
#
# Usage:  ./run-tests.sh <fileA.c> <fileB.c>

set -u
A_SRC="${1:-claude.c}"; B_SRC="${2:-codex.c}"
A=$(basename "$A_SRC" .c); B=$(basename "$B_SRC" .c)
rule() { printf '\n=== %s ===\n' "$1"; }

# --------------------------------------------------------------------------
# 0. Environment. Every number below is only meaningful against the machine
#    that produced it. Recorded first so a laptop run and a cycle run can be
#    compared honestly.
# --------------------------------------------------------------------------
rule "ENVIRONMENT"
uname -m; gcc --version | head -1; ldd --version | head -1; date

# --------------------------------------------------------------------------
# 1. Compilation under four flag sets. The point is not "does it build". It is
#    that the same file can be silent under one set and complain under another,
#    so the flags you pick decide what you are allowed to find out.
# --------------------------------------------------------------------------
for flags in "" "-Wall -Wextra" "-O2" "-Wall -Wextra -O2" "-std=c89 -pedantic" "-std=c99 -pedantic"; do
    rule "COMPILE: gcc ${flags:-<course command, no flags>}"
    for s in "$A_SRC" "$B_SRC"; do
        echo "--- $s ---"; gcc $flags "$s" -o /dev/null; echo "exit=$?"
    done
done

# --------------------------------------------------------------------------
# 2. Build the working binaries with the exact course command.
# --------------------------------------------------------------------------
gcc "$A_SRC" -o "$A"; gcc "$B_SRC" -o "$B"

# --------------------------------------------------------------------------
# 3. Byte-exact check against the sample printed in PROMPT.md. Reading output
#    and thinking it looks right is not the same as it being right, so diff
#    does the comparing. Run BOTH input paths, because PROMPT.md requires the
#    program to accept a filename argument and standard input.
# --------------------------------------------------------------------------
cat > sample.txt <<'EOT'
EMAIL Peer,Can you help me on this?,12-01-2024
EMAIL OtherPerson,Try our product,12-19-2024
EMAIL Boss,Important,12-20-2024
EMAIL Subordinate,How do I handle this?,12-25-2024
EMAIL ImportantPerson,Health Insurance Enrollment,12-31-2024
EMAIL Boss,Never Mind,01-03-2025
COUNT
NEXT
READ
NEXT
READ
COUNT
EOT
cat > expected.txt <<'EOT'
There are 6 emails to read.
Next email:
Sender: Boss
Subject: Never Mind
Date: 01-03-2025
Next email:
Sender: Boss
Subject: Important
Date: 12-20-2024
There are 4 emails to read.
EOT
rule "SPEC SAMPLE: byte-exact, both input paths"
for prog in "$A" "$B"; do
    for mode in arg stdin; do
        if [ "$mode" = arg ]; then "./$prog" sample.txt > "got_${prog}_${mode}.txt" 2>/dev/null
        else "./$prog" < sample.txt > "got_${prog}_${mode}.txt" 2>/dev/null; fi
        if diff -q expected.txt "got_${prog}_${mode}.txt" >/dev/null; then
            echo "  $prog ($mode): BYTE-IDENTICAL"
        else
            echo "  $prog ($mode): DIFFERS"; diff expected.txt "got_${prog}_${mode}.txt" | sed 's/^/      /'
        fi
    done
done

# --------------------------------------------------------------------------
# 4. The cases the specification names but never shows output for. The
#    instructions say the grader WILL test these, and say nothing about what
#    should be printed, so this is where two programs are free to disagree.
# --------------------------------------------------------------------------
rule "BEHAVIOR: paths the spec mentions but does not illustrate"
CASES=(
  "NEXT on empty inbox:NEXT\n"
  "READ on empty inbox:READ\n"
  "COUNT on empty inbox:COUNT\n"
  "COUNT with exactly one:EMAIL Boss,Solo,01-01-2025\nCOUNT\n"
  "two NEXTs, no READ between:EMAIL Boss,A,01-01-2025\nNEXT\nNEXT\n"
  "two READs, no NEXT between:EMAIL Boss,A,01-01-2025\nEMAIL Boss,B,01-02-2025\nREAD\nREAD\nCOUNT\n"
  "tie: same category AND same date:EMAIL Boss,FIRST,01-01-2025\nEMAIL Boss,SECOND,01-01-2025\nNEXT\n"
  "all five categories, same date:EMAIL OtherPerson,E,01-01-2025\nEMAIL ImportantPerson,D,01-01-2025\nEMAIL Peer,C,01-01-2025\nEMAIL Subordinate,B,01-01-2025\nEMAIL Boss,A,01-01-2025\nNEXT\nREAD\nNEXT\nREAD\nNEXT\nREAD\nNEXT\nREAD\nNEXT\n"
  "date ordering crosses a year:EMAIL Boss,OLD,12-01-2024\nEMAIL Boss,NEW,01-03-2025\nNEXT\n"
  "unknown sender category:EMAIL Alien,Hello,01-01-2025\nCOUNT\nNEXT\n"
  "malformed EMAIL line:EMAIL garbage with no commas\nCOUNT\n"
  "blank lines in the file:\n\nEMAIL Boss,A,01-01-2025\n\nCOUNT\n"
  "subject containing spaces:EMAIL Boss,A subject with many spaces,01-01-2025\nNEXT\n"
  "empty file:"
)
for prog in "$A" "$B"; do
    printf '\n######## %s ########\n' "$prog"
    for case in "${CASES[@]}"; do
        label="${case%%:*}"; input="${case#*:}"
        printf -- '--- %s ---\n' "$label"
        printf "$input" > case.txt
        timeout 5 "./$prog" case.txt; echo "[exit=$?]"
    done
done
rule "BEHAVIOR: command line"
for prog in "$A" "$B"; do
    printf 'COUNT\n' > case.txt
    printf '  %s extra args -> ' "$prog"; "./$prog" case.txt extra 2>&1 | head -1; echo "    [exit=$?]"
    printf '  %s missing file -> ' "$prog"; "./$prog" no_such_file.txt 2>&1 | head -1; echo "    [exit=$?]"
done

# --------------------------------------------------------------------------
# 5. Heap-order oracle. Eyeballing a sorted list proves nothing at size 6. This
#    generates emails with no duplicate (category,date) pairs -- so the two
#    programs' different tie-break rules cannot matter -- drains the queue, and
#    compares the order against an independent sort done in Python. Anything
#    other than a correct max-heap shows up immediately.
# --------------------------------------------------------------------------
rule "HEAP ORDER ORACLE (500 emails, no ties)"
python3 - "$A" "$B" <<'PYEOF'
import random, subprocess, sys
random.seed(348)
CATS=["Boss","Subordinate","Peer","ImportantPerson","OtherPerson"]
RANK={c:5-i for i,c in enumerate(CATS)}
N=500; seen=set(); rows=[]
while len(rows)<N:
    c=random.choice(CATS); m=random.randint(1,12); d=random.randint(1,28); y=random.randint(2000,2030)
    if (c,m,d,y) in seen: continue
    seen.add((c,m,d,y)); rows.append((c,f"Subj{len(rows)}",f"{m:02d}-{d:02d}-{y:04d}",y*10000+m*100+d))
with open('oracle.txt','w') as f:
    for c,s,dt,_ in rows: f.write(f"EMAIL {c},{s},{dt}\n")
    for _ in rows: f.write("NEXT\nREAD\n")
expected=[s for c,s,dt,k in sorted(rows,key=lambda r:(-RANK[r[0]],-r[3]))]
for prog in sys.argv[1:3]:
    out=subprocess.run([f"./{prog}","oracle.txt"],capture_output=True,text=True).stdout
    got=[l.split("Subject: ",1)[1] for l in out.splitlines() if l.startswith("Subject: ")]
    if got==expected: print(f"  {prog}: order CORRECT for all {N}")
    else:
        i=next((i for i,(g,e) in enumerate(zip(got,expected)) if g!=e), min(len(got),len(expected)))
        print(f"  {prog}: WRONG at position {i} -- got {got[i] if i<len(got) else 'EOF'}, expected {expected[i]}")
PYEOF

# --------------------------------------------------------------------------
# 6. Signed-overflow probe on the date key. Both programs turn MM-DD-YYYY into
#    one number, YYYYMMDD. If that number is stored in an int, a year big
#    enough makes year*10000 exceed what an int holds. Signed overflow is
#    undefined behavior -- not "wrong", but "anything may happen". A far-future
#    email can then sort as the oldest one in the queue.
# --------------------------------------------------------------------------
rule "DATE-KEY OVERFLOW PROBE"
printf 'EMAIL Boss,FAR_FUTURE,01-01-429497\nEMAIL Boss,NORMAL_2025,01-01-2025\nNEXT\n' > ovf.txt
for opt in 0 2; do
    for s in "$A_SRC" "$B_SRC"; do
        b=$(basename "$s" .c); gcc -O$opt "$s" -o "${b}_o$opt" 2>/dev/null
        printf '  %-8s -O%s -> %s\n' "$b" "$opt" "$(./${b}_o$opt ovf.txt | grep 'Subject:')"
    done
done
echo "  (correct answer is FAR_FUTURE: year 429497 is newer than 2025)"
for s in "$A_SRC" "$B_SRC"; do
    b=$(basename "$s" .c)
    if gcc -fsanitize=signed-integer-overflow -O0 "$s" -o "${b}_ub" 2>/dev/null; then
        echo "  --- $s under the signed-overflow sanitizer ---"
        ./${b}_ub ovf.txt 2>&1 >/dev/null | head -2 | sed 's/^/     /'
    fi
done

# --------------------------------------------------------------------------
# 7. Execution time. A heap is O(log n) per operation, so unlike Assignment 1
#    there is something real to measure here -- but only at sizes far beyond
#    any test file. Three sizes, best of three runs, user CPU only: on a shared
#    server real and sys measure how busy the machine is, not the program.
# --------------------------------------------------------------------------
rule "EXECUTION TIME: scaling, user CPU, best of 3"
TIMEFORMAT='%3U'
printf '  %-9s %-10s %-10s\n' "N emails" "$A" "$B"
for n in 5000 50000 500000; do
    python3 -c "
import random;random.seed(1)
C=['Boss','Subordinate','Peer','ImportantPerson','OtherPerson']
w=open('scale.txt','w')
for i in range($n): w.write('EMAIL %s,S%d,%02d-%02d-%04d\n'%(random.choice(C),i,random.randint(1,12),random.randint(1,28),random.randint(2000,2030)))
for i in range($n): w.write('NEXT\nREAD\n')"
    printf '  %-9s' "$n"
    for s in "$A_SRC" "$B_SRC"; do
        b=$(basename "$s" .c); gcc -O2 "$s" -o "${b}_t" 2>/dev/null
        best=999
        for r in 1 2 3; do
            t=$( { time ./${b}_t scale.txt > /dev/null; } 2>&1 )
            best=$(python3 -c "print(min($best,$t))")
        done
        printf ' %-10s' "${best}s"
    done
    echo
done

# --------------------------------------------------------------------------
# 8. Space. Two separate questions. The stack frame is what Assignment 1 taught
#    me to measure instead of peak RSS -- RSS is almost all C library and can
#    never show a code difference. But this program also calls malloc, so peak
#    heap is a real number here, and it is where the two designs actually differ.
# --------------------------------------------------------------------------
rule "SPACE: stack frame per function"
for s in "$A_SRC" "$B_SRC"; do
    b=$(basename "$s" .c); gcc -fstack-usage -c "$s" -o /dev/null 2>/dev/null
    [ -f "$b.su" ] || continue
    echo "  --- $b ---"
    sort -t$'\t' -k2 -rn "$b.su" | head -3 | awk -F'\t' '{printf "     %-30s %s bytes\n",$1,$2}'
    awk -F'\t' -v b="$b" '{s+=$2;n++} END{printf "     %d functions, %d bytes total\n",n,s}' "$b.su"
done

rule "SPACE: sizeof(Email) and growth policy"
for s in "$A_SRC" "$B_SRC"; do
    b=$(basename "$s" .c)
    python3 - "$s" "$b" <<'PYEOF'
import sys,re,subprocess
src=open(sys.argv[1]).read(); b=sys.argv[2]
m=re.search(r'typedef struct \{.*?\} Email;', src, re.S)
if m:
    open(f"sz_{b}.c","w").write('#include <stdio.h>\n#include <stdlib.h>\n'+m.group(0)+
        '\nint main(void){printf("     sizeof(Email) = %zu bytes\\n",sizeof(Email));return 0;}\n')
    subprocess.run(["gcc",f"sz_{b}.c","-o",f"sz_{b}"],capture_output=True)
    print(f"  --- {b} ---"); subprocess.run([f"./sz_{b}"])
PYEOF
    grep -oE 'capacity == 0 \? [0-9]+|capacity == 0\) \? [0-9]+' "$s" | head -1 | sed 's/^/     initial capacity: /'
done

rule "SPACE: peak heap and leaks (valgrind; skipped if unavailable)"
if command -v valgrind >/dev/null 2>&1; then
    python3 -c "
import random;random.seed(7)
C=['Boss','Subordinate','Peer','ImportantPerson','OtherPerson']
w=open('mem.txt','w')
for i in range(10000): w.write('EMAIL %s,Subject number %d with some words,%02d-%02d-%04d\n'%(random.choice(C),i,random.randint(1,12),random.randint(1,28),random.randint(2000,2030)))
for i in range(5000): w.write('NEXT\nREAD\n')
w.write('COUNT\n')"
    for s in "$A_SRC" "$B_SRC"; do
        b=$(basename "$s" .c); gcc -g -O0 "$s" -o "${b}_g" 2>/dev/null
        echo "  --- $b ---"
        valgrind --leak-check=full ./${b}_g mem.txt 2>&1 >/dev/null \
          | grep -E 'definitely lost|indirectly lost|ERROR SUMMARY' | sed 's/^==[0-9]*== /     /'
        valgrind --tool=massif --massif-out-file="ms_$b.out" ./${b}_g mem.txt >/dev/null 2>&1
        peak=$(grep mem_heap_B "ms_$b.out" | sed 's/mem_heap_B=//' | sort -rn | head -1)
        echo "     peak heap (10,000 emails): $peak bytes"
    done
else
    echo "  valgrind not installed here -- run this section on the cycle server"
fi

# --------------------------------------------------------------------------
# 9. Maintainability, counted rather than asserted. The rubric pays for
#    comments and for a prolog header, so those get counted, not admired.
# --------------------------------------------------------------------------
rule "MAINTAINABILITY: size, structure, comment density"
printf '  %-10s %8s %8s %8s %8s\n' "file" "lines" "comment" "funcs" "cmt/fn"
for s in "$A_SRC" "$B_SRC"; do
    b=$(basename "$s" .c)
    tot=$(grep -c '' "$s"); cm=$(grep -cE '(^[[:space:]]*\*|//|/\*)' "$s")
    fn=$(grep -cE '^[a-z].*\(.*\)[[:space:]]*$|^static .*\(.*\)$' "$s")
    printf '  %-10s %8s %8s %8s %8s\n' "$b" "$tot" "$cm" "$fn" \
      "$(python3 -c "print(round($cm/max($fn,1),1))")"
done

rule "MAINTAINABILITY: rubric prolog items present in the first 25 lines"
for s in "$A_SRC" "$B_SRC"; do
    echo "  --- $(basename "$s") ---"
    head -25 "$s" > hdr.tmp
    while IFS='|' read -r label pat; do
        if grep -qiE "$pat" hdr.tmp; then r="present"; else r="MISSING"; fi
        printf '     %-22s %s\n' "$label" "$r"
    done <<'EOT'
program name|EECS|348|Assignment
brief description|escription
inputs|[Ii]nput
outputs|[Oo]utput
author full name|uthor|Tryban
creation date|reated|[Dd]ate:
revision date|evision
collaborators|ollaborat
other sources / GenAI|ChatGPT|Codex|Claude|GenAI|OpenAI|Anthropic
EOT
    rm -f hdr.tmp
done

# --------------------------------------------------------------------------
# 10. Everything the two models decided differently, in one place.
# --------------------------------------------------------------------------
rule "SOURCE DIFF"
diff -u "$A_SRC" "$B_SRC" | head -200

echo; echo "=== done ==="
