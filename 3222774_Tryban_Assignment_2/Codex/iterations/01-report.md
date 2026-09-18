# Work report: `iterations/01-raw.c`

This report covers the first request to write the C program and the follow-up audit used to prepare this report. The existing `00-report.md` was read and replaced only after the follow-up request.

## Files read

- `/Users/francistryban/AgenticArea/Fall2026_EECS348_Assignment_2/3222774_Tryban_Assignment_2/Codex/AGENT.md`
- `/Users/francistryban/AgenticArea/Fall2026_EECS348_Assignment_2/3222774_Tryban_Assignment_2/Codex/PROMPT.md`
- `/Users/francistryban/AgenticArea/Fall2026_EECS348_Assignment_2/3222774_Tryban_Assignment_2/Codex/iterations/00-raw.c`
- `/Users/francistryban/AgenticArea/Fall2026_EECS348_Assignment_2/3222774_Tryban_Assignment_2/Codex/iterations/01-raw.c` (inspected after writing it)
- `/Users/francistryban/AgenticArea/Fall2026_EECS348_Assignment_2/3222774_Tryban_Assignment_2/Codex/iterations/00-report.md` (its prior contents, during this follow-up)
- `/Users/francistryban/.codex/sessions/2026/09/17/rollout-2026-09-17T18-55-38-01a0b1cc-18d3-70e2-9caf-e22e58d56d1c.jsonl` (first-prompt timing and root token counts)
- `/Users/francistryban/.codex/sessions/2026/09/17/rollout-2026-09-17T18-55-43-01a0b1cc-2e09-7a41-ac8a-62f8f0192cf0.jsonl` (helper token counts)
- `/Users/francistryban/.codex/logs_2.sqlite` (a helper queried its table names only)

The compiler also reads standard headers, and Git reads repository metadata internally. I did not inspect those files individually.

## Order of work and commands

1. Located the instructions and checked the starting workspace with `pwd && rg --files -g 'AGENT.md' -g 'PROMPT.md' -g 'AGENTS.md' -g '!iterations/**'` and `git status --short && rg --files iterations`. An earlier JavaScript orchestration call had a syntax error, so it ran no shell command.
2. Read the instructions and prior source with `cat AGENT.md`, `cat PROMPT.md`, and `cat iterations/00-raw.c`. A read-only helper separately checked the requirements.
3. Designed a dynamic-array MaxHeap with sender rank, a `long` date key, and arrival order. Wrote only `iterations/01-raw.c` with `apply_patch`.
4. Checked syntax and comments with `gcc -Wall -Wextra -O2 -fsyntax-only iterations/01-raw.c`, `git status --short -- iterations/01-raw.c && wc -l iterations/01-raw.c && rg -n '^[^/\n]*[^[:space:]/][[:space:]]*$' iterations/01-raw.c`, and `rg -n '//.*[A-Z]' iterations/01-raw.c`.
5. Checked the full compile with `gcc -Wall -Wextra -O2 iterations/01-raw.c -o /dev/null`; inspected the new file with `git diff --no-index -- /dev/null iterations/01-raw.c`; checked for fileless runtime tools with `command -v tcc || true`, `command -v lli || true`, and `command -v clang || true`.
6. Used `apply_patch` twice to shorten and clarify comments. Ran the full compile again with `gcc -Wall -Wextra -O2 iterations/01-raw.c -o /dev/null` and checked the final workspace with `git status --short && rg -n '//.*[A-Z]' iterations/01-raw.c || true`. A read-only helper reviewed the code. No runtime executable was created.
7. For this follow-up, read the previous report with `cat iterations/00-report.md`; checked file status and dates with `git status --short -- iterations/00-report.md iterations/01-raw.c && stat -f '%N %Sm %z bytes' -t '%Y-%m-%d %H:%M:%S %Z' iterations/00-report.md iterations/01-raw.c`; ran `printenv CODEX_HOME`. A read-only helper used `pwd`, `ls`, `find`, `sqlite3 ... '.tables'`, and five `python3` here-document queries to locate and inspect the two rollout logs.
8. I ran three `python3 -c` queries over those logs to check `task_started`, `task_complete`, and `token_count` records, then `ls -l /Users/francistryban/.codex/logs_2.sqlite`. I replaced this report with `apply_patch` and checked it with `git diff --check -- iterations/00-report.md`, `git status --short -- iterations/00-report.md iterations/01-raw.c`, `sed -n '1,35p' iterations/00-report.md`, and `tail -n 12 iterations/00-report.md`. Finally, `gcc -### -Wall -Wextra -O2 iterations/01-raw.c -o /dev/null` showed a temporary object path. A recursive `rg --files /var/folders/bg/z6pgy6ys3_b4_8pgf6mmc1800000gn/T -g '01-raw-*.o'` hit protected directories, so I used `find /var/folders/bg/z6pgy6ys3_b4_8pgf6mmc1800000gn/T -maxdepth 1 -name '01-raw-*.o' -print` to check the top level.

## Where `PROMPT.md` was silent

Some choices below were set by `AGENT.md` or the direct user request, although `PROMPT.md` itself was silent.

| Silent point | Choice made | Rejected choice |
|---|---|---|
| Output file | Used `iterations/01-raw.c`, as directly requested | Another filename |
| Heap representation | Resizable array used as a list-based binary MaxHeap | Linked nodes or a prebuilt heap module |
| Initial capacity and growth | Start at eight emails and double when full | Fixed capacity or growing one slot at a time |
| Long command lines and subjects | Grow the input buffer as needed | Fixed buffer that truncates text |
| Storage of the three email fields | Copy all fields once, with pointers into that copy | Three separate allocations or references to a freed line |
| Date comparison method | Parse into a `long` `YYYYMMDD` key, following `AGENT.md` | Compare `MM-DD-YYYY` text or use `int` |
| Same category and date | Earlier file arrival wins, following `AGENT.md` | Arbitrary heap order or later arrival first |
| How `NEXT` preserves its email | Peek at the heap root | Remove and reinsert it |
| `NEXT` on an empty inbox | Print `No emails to read.`, following `AGENT.md` | Print nothing |
| `READ` on an empty inbox | Do nothing and print nothing | Treat it as an error |
| Output for `EMAIL` and successful `READ` | Print nothing | Add acknowledgements |
| `COUNT` wording for zero or one | Keep the sample's `There are N emails to read.` form | Change to singular grammar |
| Final line without a newline | Process it as a command | Require a trailing newline |
| Line ending style | Accept LF and CRLF | Accept LF only |
| Command case and extra whitespace | Match the stated format exactly | Normalize case or trim fields |
| Subject whitespace | Preserve it as input | Trim or collapse spaces |
| Unknown commands or missing commas | Ignore them without extra output; valid input was promised | Abort or print invented diagnostics |
| Unknown sender or invalid date | Rely on the promised valid inputs; an unknown rank falls to lowest and date fields start at zero | Add separate validation output |
| More than one filename | Print usage to standard error and fail, following `AGENT.md` | Ignore extra arguments |
| Failure to open a file | Print a diagnostic to standard error and fail | Fall back to standard input |
| Allocation or input failure | Clean up, print a diagnostic to standard error, and fail | Continue with incomplete state |
| C dialect and libraries | Use standard C library calls and write the heap functions from scratch | Compiler-only line reading or a heap library |
| Verification output | Send the final linker output to `/dev/null` | Save a test executable |
| Resource ownership | Free each removed or remaining email, the heap array, each input line, and any opened file | Leave cleanup to process exit |
| Comments and file header | Follow `AGENT.md`'s line comments and nine-field header | Omit those explanations |

## Exact first-prompt usage

The Codex rollout records show the first request started **September 17, 2026 at 18:55:38.646 CDT** and completed at **18:59:54.500 CDT**: **255.854 seconds (4 minutes 15.854 seconds)**. The root session recorded 553,374 input and 10,809 output tokens (564,183 total). Its read-only helper recorded 250,797 input and 2,580 output tokens (253,377 total). Together, the first request spent **804,171 input and 13,389 output tokens, or 817,560 tokens total**. Cached input (756,608) and reasoning output (5,938) are subsets of those totals, not extra tokens. These are the recorded token counts, not a monetary cost.

## Forbidden actions

I violated the first request's literal ban on creating other files: I ran two full compiler checks, and the compiler writes a temporary object file under `/var/folders/bg/z6pgy6ys3_b4_8pgf6mmc1800000gn/T` before sending the final output to `/dev/null`. No matching object remained at the top level of that temporary directory when I checked. I did not save a test executable or knowingly change another workspace file; pre-existing workspace changes were left alone. This follow-up explicitly authorized replacing `iterations/00-report.md`.
