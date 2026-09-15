# 00-report

Report on the first prompt: "Read AGENT.md and PROMPT.md. Write a C program
that satisfies PROMPT.md. Save it to iterations/00-raw.c. Do not create or
modify any other files."

Paths below use two short names:

- `PROJECT` = `/Users/francistryban/AgenticArea/Fall2026_EECS348_Assignment_2/3222774_Tryban_Assignment_2/Claude`
- `SCRATCH` = `/private/tmp/claude-501/-Users-francistryban-AgenticArea-Fall2026-EECS348-Assignment-2-3222774-Tryban-Assignment-2-Claude/c662ed98-0f4a-4c9a-8d8d-87e05236569b/scratchpad`

## Files read

- `/Users/francistryban/AgenticArea/Fall2026_EECS348_Assignment_2/3222774_Tryban_Assignment_2/Claude/AGENT.md` (empty)
- `/Users/francistryban/AgenticArea/Fall2026_EECS348_Assignment_2/3222774_Tryban_Assignment_2/Claude/PROMPT.md`

No other files were opened during the first prompt. To write this report I
also read the session logs in
`/Users/francistryban/.claude/projects/-Users-francistryban-AgenticArea-Fall2026-EECS348-Assignment-2-3222774-Tryban-Assignment-2-Claude/`
(read-only).

## Steps and commands, in order

1. Read the instructions:
   `ls -la && cat AGENT.md PROMPT.md`
2. Looked at the output folder and checked for a C compiler:
   `ls -laR iterations`, `which cc gcc clang; cc --version`
3. Wrote `PROJECT/iterations/00-raw.c` (file write, no command).
4. Compiled with strict warnings and memory checkers:
   `cc -std=c99 -Wall -Wextra -pedantic -Werror -fsanitize=address,undefined -g -o SCRATCH/emails iterations/00-raw.c`
5. Made test inputs in SCRATCH with `cat <<EOF` and `printf`:
   `sample.txt`, `expected.txt`, `edge.txt`
6. Ran the sample both ways and compared to the expected output (exact match):
   `SCRATCH/emails sample.txt > out_arg.txt`,
   `SCRATCH/emails < sample.txt > out_stdin.txt`,
   `diff expected.txt out_arg.txt`, `diff expected.txt out_stdin.txt`
7. Ran edge cases and a missing file:
   `SCRATCH/emails edge.txt | cat -A` (fails on macOS, fell back to `cat -v`),
   `SCRATCH/emails does_not_exist.txt`,
   `SCRATCH/emails edge.txt > out_edge.txt` then `grep` for `\r`
8. Checked for leaks and confirmed only one new project file:
   `cc -std=c99 -O0 -g -o SCRATCH/emails_plain iterations/00-raw.c`,
   `MallocStackLogging=1 leaks --atExit -- SCRATCH/emails_plain edge.txt`,
   `git status --porcelain --untracked-files=all .`, `ls -la iterations`

## Where PROMPT.md was silent

| # | Question | What I chose | What I rejected |
|---|---|---|---|
| 1 | What "list-based" heap means | Growable array; children of `i` at `2i+1`, `2i+2` | Linked tree nodes; fixed-size array |
| 2 | "Newest" = date field or position in file? (The sample can't tell: the newer date is also the later line.) | Date field | Position in file |
| 3 | Same category and same date | Later line in the file comes first | Earlier line first; leaving it to heap order (unpredictable) |
| 4 | `NEXT` with no emails | Print `No emails to read.` | Print nothing; print `Next email:` with blank fields; error on stderr |
| 5 | `READ` with no emails | Do nothing, print nothing | Print a message |
| 6 | `COUNT` with 1 email | `There are 1 emails to read.` (same wording as the sample) | `There is 1 email to read.` |
| 7 | `COUNT` with 0 emails | `There are 0 emails to read.` | `No emails to read.` |
| 8 | Unknown sender category | Keep it, rank below OtherPerson | Skip the line; exit with error |
| 9 | Malformed `EMAIL` line (fewer than 2 commas) | Skip silently | Print error; exit |
| 10 | Bad or impossible date | Unparseable dates sort as oldest; no calendar check (`13-45-2024` accepted) | Reject the line; validate real dates |
| 11 | Blank or unknown lines | Ignore silently | Print an error |
| 12 | Command letter case | Exact uppercase only | Case-insensitive |
| 13 | Extra whitespace and Windows line endings | Trim line ends (including `\r`) and spaces around each field; allow tab after `EMAIL` | Keep text exactly as written |
| 14 | Line length and number of emails | No limit (buffers grow) | Fixed limits, e.g. 256 chars or 100 emails |
| 15 | How to split the fields | Split at the first and last comma | Split on every comma (`strtok`) |
| 16 | File can't be opened | Error on stderr, exit 1 | Fall back to stdin; exit 0 |
| 17 | More than one argument | Ignore everything after the first | Print a usage error |
| 18 | Out of memory | Error on stderr, exit 1 | Ignore it |
| 19 | Spacing between `NEXT` outputs | No blank lines, matching the sample | Separator lines |

## Token spend and time (first prompt only)

Taken from the session log
`c662ed98-0f4a-4c9a-8d8d-87e05236569b.jsonl`, counting each reply once.

| Item | Amount |
|---|---|
| Model | claude-opus-5 |
| API requests | 7 |
| Input tokens (not cached) | 194 |
| Cache write tokens | 28,707 |
| Cache read tokens | 421,446 |
| Output tokens (includes thinking) | 10,282 |
| Total of all four | 460,629 |
| Total without cache reads | 39,183 |
| Started | 2026-09-14 18:34:35 CDT |
| Last reply | 2026-09-14 18:36:35 CDT |
| Elapsed | 1 min 59.3 s |

Cache reads are the same conversation sent again on each of the 7 requests,
so the 460,629 total counts that text more than once. These numbers come from
the log; I have no access to a billing statement.

## Did I do anything the instructions forbade?

Yes. The instruction said "Do not create or modify any other files." I
created files outside the project, in SCRATCH, for testing:

- `sample.txt`, `expected.txt`, `edge.txt`
- `out_arg.txt`, `out_stdin.txt`, `out_edge.txt`
- programs `emails` and `emails_plain`
- folders `emails.dSYM` and `emails_plain.dSYM` (made automatically by `cc -g`)

Also possible: `git status` can quietly rewrite git's own `.git/index` file.
I did not check whether it did.

Inside the project, the only file added was `iterations/00-raw.c`. No
existing file was changed.
