# EECS 348 Assignment 2 — GenAI Code Comparison

**Working analysis draft.** This is the live document. It is written as decisions
are made, not reconstructed afterward. It becomes the submitted PDF.

- **Author:** Francis (Frankie) Tryban — Student ID 3222774
- **Course:** EECS 348, Software Engineering I, Fall 2026
- **Assignment:** No. 2 — CEO email prioritization via a from-scratch MaxHeap
- **Started:** September 14, 2026
- **Due:** September 17, 2026, 11:59 PM
- **Collaborators / other sources:** see the Collaborator Disclosure section at the
  end. Claude (Anthropic) assisted in drafting this analysis document and in
  building the measurement scripts; that assistance is disclosed there and in the
  prolog of every source file.

---

## What I found, in plain terms

The short version, before any of the detail. Four things matter and the rest is
noise.

**Neither program put my name on it.** No name, no date, no line saying a GenAI
helped write it. That is worth more points than any bug in either file, and the
assignment says leaving off the disclosure is academic misconduct rather than
just a zero. The same thing happened on Assignment 1. Both models will do this
every time unless the folder's `AGENT.md` tells them not to.

**Claude commented its code and Codex barely did.** 53 comment lines against 4.
The rubric asks for a comment on every line. Neither one gets there, but one is
in the neighbourhood and the other is not.

**They disagree about the empty inbox.** Ask for the next email when there are
none: Claude prints `No emails to read.`, Codex prints nothing at all. The
instructions say the grader will test this and never say what should happen. One
of them is wrong and there is no way to know which.

**They disagree about ties.** Two emails from the Boss on the same day. Claude
shows the one that appears later in the file, Codex shows the one that appears
first. Opposite answers from the same input.

**On everything else a person would actually notice, they are the same.** Both
reproduce the assignment's example exactly. Both sort 500 emails in the right
order. Both run at the same speed, use about the same memory, and neither one
crashes or leaks. Run them side by side and you cannot tell them apart.

That is the whole finding. Three of those four items are rules I can write into
`AGENT.md`, which is what section 2g is about.

---

## How to read this document

The rubric asks for seven things (2a–2g). Those seven sections are at the bottom
and are the graded deliverable. Above them is a **decision log**, kept in real
time, that records every choice made in setting up the experiment and why. The
decision log exists because the conclusions in 2e–2g are only as good as the
setup that produced them; a reader who disagrees with a conclusion should be able
to find the exact decision that caused it.

Where a decision later turned out to be wrong, it is corrected in place with the
original reasoning left visible. That is deliberate.

---

## Method (fixed before any code was generated)

The comparison is only meaningful if the two GenAIs receive identical input. The
setup enforces that physically rather than by good intentions:

```
3222774_Tryban_Assignment_2/
├── Claude/
│   ├── AGENT.md        (empty at run 00)
│   ├── PROMPT.md       (assignment program spec — byte-identical to Codex/PROMPT.md)
│   └── iterations/     (00-raw.c lands here, then 01-*.c, ...)
└── Codex/
    ├── AGENT.md        (empty at run 00)
    ├── PROMPT.md       (byte-identical to Claude/PROMPT.md)
    └── iterations/
```

`PROMPT.md` contains the program specification only — the MaxHeap requirements,
the command table, the sample test file, and the byte-exact expected output. It
excludes the submission logistics, the rubric, and the analysis instructions,
because those are addressed to the student, not to the program author. Both files
were written from the same source and verified identical (`diff` clean, 2830
bytes each).

`AGENT.md` is **empty** for run 00. That is the point of run 00: it measures what
each system produces from the specification alone, with no coaching. Rules only
enter `AGENT.md` after the raw results have been analyzed, and the effect of
those rules is measured in run 01.

---

## Decision log

### Decision 1 — Which two GenAIs, and in what configuration
*Logged September 14, 2026*

**The decision.** Claude **Opus 5** (Ultra Code configuration) versus
**5.6 Sol Ultra**, each driven through its own agentic command-line harness,
each with **full computer access enabled**.

**Why.**

1. *Contemporaneous release.* The two models shipped at roughly the same time,
   so neither benefits from an extra generation of training. A comparison
   between a current flagship and a year-old model measures the calendar, not
   the models.
2. *Both are top-of-the-line.* The interesting question is not whether a weak
   model fails — it is whether two of the strongest available systems, given the
   same specification and nothing else, converge on the same program. If they
   diverge here, the divergence is about how each system reads an underspecified
   spec, which is the real subject of this course.
3. *Assignment compliance.* The assignment permits ChatGPT **or** CoPilot as one
   of the two choices, but not both, on the grounds that they are effectively the
   same GenAI. This pairing uses at most one model from that family, so it is
   compliant. (Verify at run time and record the exact provider in 2a.)

**Alternatives considered and rejected.**

- *A flagship against a deliberately weaker or older model.* Rejected: it would
  produce a large, obvious difference that teaches nothing. A result of "the
  better model was better" is not a finding.
- *Two models from the same provider.* Rejected: shared training data and shared
  system prompting would collapse the two outputs toward each other, and it
  risks the ChatGPT/CoPilot restriction depending on the pairing.
- *Chat-window use instead of agentic CLI.* Rejected as the primary method, see
  the caveat below — but note this is what Assignment 1 effectively measured.

**What this configuration buys, and what it costs.**

This is the most important entry in this log, and it must be stated plainly in
the final analysis rather than buried.

With full computer access enabled, each system can compile its own code, run it
against the sample test file printed in `PROMPT.md`, see its own failures, and
fix them before saving. That means `00-raw.c` is **not** a single forward pass
from the model. It is the output of an agentic loop: model plus harness plus
compiler plus filesystem.

So this assignment does not compare two *models*. It compares two *systems*.
That is a defensible and arguably more useful thing to measure — it is how these
tools are actually used in practice — but it is a different measurement from
Assignment 1, and every conclusion in 2e must be phrased accordingly. Saying
"Opus 5 wrote better code than 5.6 Sol Ultra" would be an overclaim. Saying
"the Opus 5 system produced better code than the 5.6 Sol Ultra system under
identical inputs" is what the evidence will actually support.

**Amendment, same day — Decision 2 supersedes part of the above.**

The paragraphs immediately above were written assuming each system would be free
to compile and test. Decision 2 (below) forbids execution during run 00. The
original reasoning is left in place because it is still the correct reasoning for
*run 01 and later*, where the constraint is lifted.

What changes for run 00: full computer access remains enabled at the harness
level — that is the stated configuration and it is not being turned off — but the
run instruction forbids using it to compile, execute, or test. Run 00 is
therefore much closer to a comparison of the two **models** than of the two
systems, which is a cleaner answer to the rubric's question and directly
comparable to Assignment 1.

It also creates a free observable that costs nothing to collect: **each system is
given a capability and told not to use it.** Whether it complies is recorded. A
system that silently compiles anyway has told us something about itself, and the
evidence would be visible as stray object files, binaries, or timestamps in its
folder. This is checked before either `00-raw.c` is read.

**Pre-registered prediction — original (written before Decision 2; superseded, retained for the record).**

Recording this in advance so the analysis cannot be accused of fitting a story to
whatever came out:

The sample test file appears inside `PROMPT.md` together with its byte-exact
expected output. Any system that can compile and run will use it as a self-test.
Therefore **both programs are expected to pass the sample test**, and the sample
test is expected to be useless as a discriminator — a floor, not a measurement.

The discrimination should instead appear in the behaviors the specification
*mentions* but never shows output for:

| Case | Spec says | Spec shows output | Expected to discriminate |
|---|---|---|---|
| `NEXT` / `READ` on an empty inbox | yes — explicitly tested | no | **yes** |
| Two `NEXT`s in a row | yes — same output both times | no | **yes** |
| Two `READ`s in a row | yes — silent delete | no | **yes** |
| `COUNT` when exactly 1 email remains | no | no | **yes** — "1 emails"? |
| How the test file reaches the program | no | no | **yes** — argv vs stdin |
| The printed sample case | yes | yes, byte-exact | no — both should pass |

If that prediction holds, it is itself a finding worth reporting: a specification
that includes its own test case causes capable agentic systems to converge on the
tested path and diverge everywhere else. The untested paths are where the risk
lives, and the grader's test file is explicitly stated to be a different file.

**Pre-registered prediction — revised after Decision 2 (this is the live one).**

Forbidding execution reverses the central claim above. If neither system can run
the sample test, neither can use it as a crutch, and **the sample test is
restored as a real discriminator**. Passing it now requires getting two separate
things right by reasoning alone:

1. **Two-key ordering** — category first, then newest date within a category.
   The sample output only reveals this if both Boss emails are ordered correctly
   (`Never Mind`, 01-03-2025 before `Important`, 12-20-2024).
2. **Date comparison** — `MM-DD-YYYY` compared as a string sorts wrongly
   (`12-01-2024` > `01-03-2025` lexically, which is backwards). A system that
   reaches for `strcmp` on the raw date fails the sample, and cannot discover
   that failure without running it.

Revised expectations for run 00:

| Case | Now expected to discriminate | Why the change |
|---|---|---|
| Does it compile at all, `gcc file.c -o file` | **yes** | no self-check is possible; this was a live failure mode on Assignment 1 |
| The printed sample case, byte-exact | **yes** — was "no" | cannot be self-tested into passing |
| Date comparison method | **yes** | silent wrong answer, invisible without running |
| `NEXT` / `READ` on an empty inbox | **yes**, more strongly | untestable and unshown |
| Two `NEXT`s in a row | **yes**, more strongly | untestable and unshown |
| Two `READ`s in a row | **yes**, more strongly | untestable and unshown |
| `COUNT` with exactly 1 email | **yes** | pluralization never specified |
| How the test file reaches the program | **no** — removed | specified in `PROMPT.md` by Decision 3 |
| Compliance with the no-execution instruction | **yes** | new observable, see amendment above |

The prediction to be judged against results: the two programs will differ more in
run 00 under this protocol than they would have with testing enabled, because
testing is a convergence mechanism — it pulls any system that can run code toward
the one tested path. Removing it should widen the gap and make the comparison
more informative, at the cost of no longer measuring how these tools are actually
used day to day. Run 01 measures that instead.

**Open items this decision creates.**

1. Exact version strings for both systems must be captured at run time
   (CLI `--version`, model banner, date) and pasted into 2a/2b. "Opus 5" and
   "5.6 Sol Ultra" as written here are the names as chosen; the analysis needs
   the precise identifiers the tools report about themselves.
2. Because computer access is on, the Assignment 1 run instruction — "do not
   create or modify any other files" — now conflicts with letting each system
   test its own work. This must be resolved before run 00 and is logged as
   Decision 2.
3. Sample size is n = 1 per system. These models are not deterministic. The
   analysis must state that this is a case study of one run each, not a
   benchmark, and must not generalize from a single sample.

---

### Decision 2 — Run 00 is generation only; no compiling, no execution
*Logged September 14, 2026*

**The decision.** Neither system may compile, run, or test anything during run 00.
The only artifact it produces is `iterations/00-raw.c`. Full computer access stays
enabled in the harness; the instruction, not the configuration, is what forbids
execution.

**Why.**

1. *It isolates the thing being graded.* The rubric asks about the code the GenAI
   generated. If a system compiles, fails, and patches until the sample passes,
   the saved file is the product of a compiler feedback loop, and the compiler
   did part of the work. Forbidding execution keeps `00-raw.c` attributable to
   the model.
2. *It preserves comparability with Assignment 1.* Assignment 1 measured
   single-shot generation. Keeping run 00 single-shot means the two assignments
   sit on the same axis, and any change in quality between them is a change in
   the models rather than a change in the method.
3. *It protects the discriminators.* Self-testing against a sample case that is
   printed inside the prompt is a convergence mechanism. It would have burned the
   single richest test available — see the revised prediction under Decision 1.

**Alternatives considered and rejected.**

- *Test freely, scratch confined to a `scratch/` folder.* Rejected for run 00,
  adopted as the likely protocol for run 01. It produces better code and a
  readable process trace, but it measures the harness as much as the model.
- *Test freely with no constraints.* Rejected: most realistic, least
  controlled. Leftovers would not be comparable between the two folders.

**Cost, stated honestly.** This deliberately does not exercise the agentic
capability that motivated choosing these two systems in the first place. Run 00
answers "how good is the raw generation," which was the stated curiosity. Run 01
answers "how good is the system when it can check its own work." Both questions
get asked; they are just asked in that order.

**Run instruction as planned:**

```
Read AGENT.md and PROMPT.md. Write a C program that satisfies PROMPT.md.
Save it to iterations/00-raw.c. Do not compile, run, or test anything.
Do not create or modify any other files.
```

---

**Amendment, September 15 — Decision 2 was not carried out, and the record says
so rather than pretending otherwise.**

The instruction actually issued to both systems, recovered verbatim from the
session logs, was:

```
Read AGENT.md and PROMPT.md. Write a C program that satisfies PROMPT.md.
Save it to iterations/00-raw.c. Do not create or modify any other files.
```

The sentence "Do not compile, run, or test anything" is absent. Both systems were
therefore free to execute, and both did: Claude compiled with
`-Wall -Wextra -pedantic -Werror -fsanitize=address,undefined`, built sample and
edge-case inputs, diffed against the expected output, and ran a leak check;
Codex compiled with `-Wall -Wextra -Wpedantic -Werror`, piped the sample and
edge cases through the binary, and deleted its temporary executable afterward.

So run 00 is the agentic comparison described in the *original* half of Decision
1, not the generation-only run Decision 2 specified. Three consequences follow,
and all three are load-bearing:

1. **The original pre-registered prediction is the live one again**, and the
   revised one under Decision 2 does not apply to this run. The original said
   both programs would pass the printed sample because both would self-test
   against it, and that discrimination would move to the untested paths. Section
   2e reports how that prediction fared.
2. **Run 00 measures systems, not models** — model plus harness plus compiler.
   Every conclusion is phrased that way.
3. **Both systems violated the instruction they were actually given** — "do not
   create or modify any other files" — by writing scratch files outside the
   project, and **both disclosed it unprompted.** See Decision 5.

This was not caught until after both runs, because nothing in the setup enforced
the instruction text; it was retyped by hand into each session. The fix for run
01 is to keep the instruction in a file and paste from it, so that what was
issued is recoverable without archaeology. It is recorded here rather than
quietly corrected because the difference between the protocol as designed and the
protocol as executed is exactly the sort of thing this document exists to catch.

---

### Decision 3 — The input mechanism is specified rather than left ambiguous
*Logged September 14, 2026*

**The decision.** `PROMPT.md` will state how the test file reaches the program.
The assignment is silent on this; that silence is being closed for both systems
identically.

**Why.** The grader runs a different test file against the submitted executable.
If the two programs disagree about how input arrives — one expecting a filename
argument, the other expecting standard input — then they cannot be driven by the
same test harness, and every downstream measurement (correctness, execution time)
would be comparing programs that were never run the same way. Controlling the
interface is what makes the rest of the comparison valid. It also removes a real
risk of the graded submission simply not accepting the grader's file.

**Cost.** A genuine finding is given up: how each system resolves an unstated
interface would have been interesting, and the grader faces that same ambiguity.
That trade is accepted because a broken interface at grading time is a much
larger loss than a missing observation.

**How it is written, so the professor's text stays intact.** The specification in
`PROMPT.md` is not edited. A short block clearly labeled as a clarification is
appended below it, identical in both folders and byte-verified. Section 2c of
this analysis reproduces `PROMPT.md` in full including that block, so the prompt
actually used is on the record and no reader has to take it on faith that the two
inputs matched.

**The mechanism chosen.** The program accepts the test file **either** as the
first command-line argument **or** on standard input when no argument is given:

```
./program testfile.txt        # filename as argv[1]
./program < testfile.txt      # same commands on stdin
```

Supporting both is a few lines of code and removes the single worst failure mode
at grading time — an executable that cannot read the grader's file at all. It
also means the measurement harness can drive both programs the same way without
knowing which convention each chose, because there is no longer a choice to make.

The clarification block appended to both copies of `PROMPT.md` reads in full:

> ## Input clarification
>
> *The assignment does not state how the test file reaches the program. This
> clarification closes that gap. It is identical in both folders.*
>
> The program's expected input is a text file containing one command per line,
> such as:
>
> ```
> EMAIL Peer,Can you help me on this?,12-01-2024
> EMAIL Boss,Important,12-20-2024
> COUNT
> NEXT
> READ
> ```
>
> The program must accept that file in either of two ways:
>
> - If a filename is given as the first command-line argument, read the commands
>   from that file: `./program testfile.txt`
> - If no argument is given, read the commands from standard input:
>   `./program < testfile.txt`
>
> All output goes to standard output. The program exits with status 0 when it
> completes normally.

**Verification.** Both files were compared after the append: `diff` reports no
difference, both are 3424 bytes, and both hash to
`6c30606e258a4abb57c1936037f0be47` (MD5).

The hash changed once between writing and running. The two-line italic note
explaining *why* the clarification existed was addressed to a reader of this
analysis, not to the program author, so it was deleted from `PROMPT.md` before
the runs — from **both** copies, and the files were re-verified identical
afterward. The figure above is the hash of what the two systems actually
received. Recording the earlier hash and then silently running a different file
would have made the verification decorative; this is the file that was used.

**A second observable this creates.** The clarification says "must accept that
file in either of two ways." That is a compound requirement stated in one
sentence. Whether each system implements both paths, or quietly implements only
the one it would have picked anyway, is recorded under correctness in 2e. It is a
small, fair test of how carefully each reads a requirement that is easy to
half-satisfy.

---

### Decision 4 — Metrics come from the harness log, never from the model
*Logged September 15, 2026*

**The decision.** Token spend, wall-clock time, and the record of what each
system did are read out of the CLI's own session log. The model is never asked to
report them.

**Why.** A model has no reliable introspective access to its own token count or
elapsed time; asked for either, it produces a plausible number rather than a
measured one. Putting invented figures in a report about measurement would
undercut the whole document.

Both systems proved the point themselves. Asked for its token spend, Codex
answered that the number was not available to it and wrote that any figure it
gave would be fabricated — the correct answer, and one that leaves the question
unanswered. Claude, which could reach its own session log, read the numbers out
of it rather than estimating. Neither invented a figure, and neither could have
produced one from introspection alone. The log had Codex's missing number the
whole time.

**What the log gives that nothing else does.** Exact provider-reported token
counts split into fresh and cached input, output, and reasoning; per-event
timestamps, so work time can be separated from the author's typing time; every
command the agent ran, verbatim; and every filesystem path it touched, which is
what makes a sandbox-compliance claim checkable instead of asserted.

**Tooling.** `evidence/session-report.py` reads either a Codex rollout log or a
Claude Code session log and prints the whole picture, including a
`--expect <folder>` check that flags any path outside the folder the system was
supposed to stay in. It is what produced the Codex figures below, and it is run
against every subsequent iteration.

**Where a prompt still helps.** The model is worth asking for a short account of
*why* it did what it did — which ambiguities it noticed, what it chose and what
it rejected. That is reasoning, not telemetry, and the log cannot supply it. It is
collected as a **claim**, cross-checked against the log, and any gap between the
two is itself reported. A model whose account matches its log is evidence about
that system; a model whose account does not is a better finding still.

**Cached tokens are reported separately and deliberately.** Of Codex's 217,439
total tokens, 191,744 were cached input — text the provider had already processed
and did not reprocess. Quoting the headline total as the cost of the work would
overstate it by roughly eight times. The figure reported as the work done is
fresh input plus output: **25,695**.

### Decision 5 — The `00-report.md` self-reports become the source material for `AGENT.md`
*Logged September 15, 2026*

**The decision.** After each system saved `00-raw.c`, it was asked in the same
session for a short account of its own work. Both produced
`iterations/00-report.md`. These are treated as a primary input to `AGENT.md`,
not as a curiosity.

**Why they turn out to be worth more than the code.** Each report contains a
table of every place the specification was silent, what the system chose, and
what it rejected — nineteen entries from Claude, twenty from Codex. That is a
list of decisions that were *already being made invisibly*, on every run, by
whichever model happened to be answering. Reading the two programs side by side
would eventually have surfaced the same list, by inference, at far greater cost
and with no way to tell a deliberate choice from an accident. The reports state
the reasoning directly.

**The filter this creates, and it is the useful part.** Laid against each other,
the two tables sort every ambiguity into two piles:

*Where they independently agreed*, the specification is effectively unambiguous
to a capable system and `AGENT.md` does not need a rule. Both chose a growable
array with children at `2i+1`/`2i+2`. Both converted `MM-DD-YYYY` to a
`YYYYMMDD` integer rather than comparing the raw string — the trap that was
pre-registered as the likeliest silent failure, avoided by both without being
warned. Both kept `There are 1 emails to read.` rather than fixing the grammar,
reasoning that matching the sample mattered more than matching English. Writing
rules for any of these would spend `AGENT.md` on settled questions.

*Where they disagreed*, `AGENT.md` must legislate, because the grader's test file
will hit these and the two programs will print different things:

| Silent point | Claude chose | Codex chose |
|---|---|---|
| `NEXT` on an empty inbox | print `No emails to read.` | print nothing |
| Same category *and* same date | later line in the file wins | earlier line wins (FIFO) |
| More than one command-line argument | ignore the extras | usage error, exit non-zero |
| Malformed `EMAIL` line | skip silently | ignore safely, diagnostics to stderr |
| Unknown sender category | keep it, rank below `OtherPerson` | ignore the line |

That table is the agenda for `AGENT.md`. It was produced in two prompts, and it
is defensible line by line because each entry traces to a system's own written
account of what it did.

**They also function as a compliance confession.** Both systems were told not to
create or modify any other files. Both did — Claude wrote test inputs, outputs
and two binaries into a scratch directory, Codex compiled to `/tmp` and deleted
the binary afterward — and **both volunteered it without being accused.** Claude
went further and flagged a violation it was not certain of, noting that
`git status` may have rewritten `.git/index` and that it had not checked.

**Cross-checked, not taken on trust.** Per Decision 4 a self-report is a claim.
Codex's claim was checked against its rollout log: it listed five files read, one
of them outside its folder
(`~/.codex/skills/.system/openai-docs/SKILL.md`, read while writing the report).
The log's independent path extraction returns exactly that set, and the sandbox
check flags exactly that one outside path. The account matches the telemetry.

**One honesty result worth recording on its own.** Asked for its token spend,
Codex answered that the figure was not available to it and wrote: *"Any numeric
answer would be fabricated."* It declined to guess. Claude, which could reach its
own session log, read the numbers out of it rather than estimating. Neither
invented a number, which is the failure mode Decision 4 was written to avoid —
and the harness log supplied Codex's missing figure anyway, which is the argument
for Decision 4 in one line.

---

## Run 00 — record

*Filesystem evidence, collected September 14, 2026. Times are America/Chicago,
taken from file modification times on the author's machine.*

| Event | Time | Size |
|---|---|---|
| Folders created, `AGENT.md` emptied | 17:29:05 | — |
| `Claude/PROMPT.md` finalized | 18:31:36 | 3424 B |
| `Codex/PROMPT.md` finalized | 18:32:22 | 3424 B |
| `Claude/iterations/00-raw.c` written | 18:35:37 | 10141 B |
| `Codex/iterations/00-raw.c` | never written | — |

**Claude side.** Produced `00-raw.c`, 382 lines, 10141 bytes. From the file
header alone, before any measurement: it opens with a prolog block comment, it
implements the heap over a dynamically grown array, it converts the date to a
`YYYYMMDD` integer key rather than comparing the raw `MM-DD-YYYY` string, and it
carries a third tie-breaker on arrival order that the specification never asked
for. It also states that it reads from `argv[1]` or falls back to standard input,
which is both halves of the Decision 3 requirement. Whether any of this is
*correct* is for 2e and the test bench; this is only what is visible on the face
of the file.

**Compliance with the no-execution instruction.** The `Claude/` folder contains
`AGENT.md`, `PROMPT.md`, and `iterations/` and nothing else — no object files, no
binary, no scratch. That is consistent with having obeyed the instruction, but it
is not proof: a compile in a temporary directory would leave the folder equally
clean. Recorded as consistent-with-compliance, not as compliance.

**Upper bound on generation time, and why it is not the real number.** 241
seconds elapsed between `Claude/PROMPT.md` being finalized and `00-raw.c`
appearing. That interval contains the author opening a terminal, starting a
session, and typing the instruction, so it is an upper bound on generation time
and a loose one. It is recorded because it is the only timing evidence the
filesystem can give; it is not reported as the model's generation time.

**Claude side, restated in light of the above.** Claude Code was run in the
correct folder and produced the right program. The comparison is not yet a
comparison; run 00 currently has one valid half.

---

### A note on what generation cost does and does not measure

Generation time and token consumption describe how expensive it was to *produce*
the program. The rubric's "execution time" in 2e means how long the compiled C
program takes to *run*. These are unrelated numbers, and conflating them would be
a real error — a model could burn ten times the tokens and still emit the faster
program, or the reverse.

Generation cost is still worth recording, as a separate and clearly-labeled
dimension: it is a genuine practical difference between the two systems. It is
reported below under its own heading, never under 2e.

**Provider-reported token counts are not comparable across providers.** Different
tokenizers split the same text into different numbers of tokens, the hidden
system prompt differs in size, and reasoning tokens are accounted for differently
or not exposed at all. Where provider numbers are available they are reported as
provider numbers. In addition, both generated files are measured with a **single
tokenizer applied identically to both**, which is a cruder but genuinely
apples-to-apples figure. Both are shown, with the difference between them
explained rather than averaged away.

---

## 2a. Name of the two GenAI programs

Two of them, from two different companies, released around the same time so
neither has an extra year of development behind it.

- **Claude Opus 5**, made by Anthropic. The exact version string the tool reports
  is `claude-opus-5`.
- **GPT-5.6 Sol**, made by OpenAI, running inside Codex. The exact version string
  is `gpt-5.6-sol`, set to its highest reasoning effort (`xhigh`).

The assignment allows ChatGPT or CoPilot as one of the two choices but not both,
because they are the same underlying model. Codex is the OpenAI one, so it fills
that slot, and Claude fills the other. Only one model from that family is used,
so this pairing is allowed.

Throughout this document I call them **Claude** and **Codex**.

## 2b. How they were accessed

Both on my own Mac, both through a command-line tool rather than a chat window
in a browser. This matters for one reason: a command-line tool can open the
folder it is pointed at, so I could hand each model an identical folder and be
sure that folder was all it saw.

- **Claude** — the Claude Code application, opened in the folder
  `3222774_Tryban_Assignment_2/Claude`.
- **Codex** — the Codex desktop application, version 0.153.4, running through
  VS Code, opened in `3222774_Tryban_Assignment_2/Codex`.

Both ran on 14 September 2026. Both were given permission to use the computer
freely, which means each one was able to compile and test its own program before
saving it, and both did.

Neither model was logged into anything of mine, and neither was given the other's
folder or the other's answer.

## 2c. Prompt used

The assignment requires the same prompt for both, so I made that checkable
instead of just claiming it.

The prompt is in two pieces. The first is a file, `PROMPT.md`, sitting in each
model's folder. It contains the assignment's program description, the table of
commands, the sample test file and its expected output, and nothing else — no
rubric, no submission instructions, none of the material addressed to me rather
than to whoever writes the program. I also added one short paragraph to it,
clearly marked, saying how the test file reaches the program, because the
assignment never says and the two programs would otherwise have been impossible
to test the same way. The full file is in Appendix A.

The two copies are identical, and I checked rather than assumed: both are 3,424
bytes and both produce the same fingerprint (`6c30606e258a4abb57c1936037f0be47`).
A fingerprint, or hash, is a short code calculated from a file's contents — two
files with the same fingerprint have the same contents, down to the byte.

The second piece is the sentence I typed into each program, and I typed nothing
else in either session:

```
Read AGENT.md and PROMPT.md. Write a C program that satisfies PROMPT.md.
Save it to iterations/00-raw.c. Do not create or modify any other files.
```

`AGENT.md` was present and completely empty in both folders for this first run.
That is deliberate. This run measures what each model produces from the
assignment alone, with no coaching from me. The coaching goes into `AGENT.md`
afterwards, in section 2g, and the effect of it is measured separately.

## 2d. Generated code from both GenAIs

Both files exactly as generated. I have not changed a character of either one,
and I will not — the rubric asks for the code the GenAI produced, so these two
files stay frozen for the rest of the assignment and the improved version lives
somewhere else.

Claude wrote 382 lines. Codex wrote 293.

### Program A — Claude Opus 5, `Claude/iterations/00-raw.c`

```c
/*
 * CEO Email Prioritizer
 *
 * Reads commands (EMAIL, NEXT, READ, COUNT) from a file given as the first
 * command-line argument, or from standard input if no argument is given.
 *
 * Emails are stored in a MaxHeap implemented from scratch on top of a
 * dynamically growing array (list-based heap). The heap acts as a priority
 * queue ordered by:
 *   1. Sender category: Boss > Subordinate > Peer > ImportantPerson > OtherPerson
 *   2. Date: newer emails before older emails
 *   3. Arrival order: for identical category and date, the later-arriving
 *      email is treated as newer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ------------------------------------------------------------------------- */
/* Email record                                                              */
/* ------------------------------------------------------------------------- */

typedef struct {
    char *sender;       /* sender category text, e.g. "Boss" */
    char *subject;      /* subject line (may contain spaces) */
    char *date;         /* date text as given, MM-DD-YYYY */
    int   priority;     /* numeric rank of sender category, higher = sooner */
    long  dateKey;      /* YYYYMMDD, so larger means newer */
    unsigned long seq;  /* arrival order, used as a final tie-breaker */
} Email;

static char *dupString(const char *s)
{
    size_t len = strlen(s);
    char *copy = malloc(len + 1);
    if (copy == NULL) {
        fprintf(stderr, "Error: out of memory\n");
        exit(1);
    }
    memcpy(copy, s, len + 1);
    return copy;
}

static void freeEmail(Email *e)
{
    free(e->sender);
    free(e->subject);
    free(e->date);
    e->sender = NULL;
    e->subject = NULL;
    e->date = NULL;
}

/* Map a sender category to its priority. Unknown categories rank lowest. */
static int categoryPriority(const char *category)
{
    if (strcmp(category, "Boss") == 0)            return 5;
    if (strcmp(category, "Subordinate") == 0)     return 4;
    if (strcmp(category, "Peer") == 0)            return 3;
    if (strcmp(category, "ImportantPerson") == 0) return 2;
    if (strcmp(category, "OtherPerson") == 0)     return 1;
    return 0;
}

/* Convert MM-DD-YYYY into a sortable YYYYMMDD number. Returns 0 if malformed. */
static long dateToKey(const char *date)
{
    int month, day, year;
    if (sscanf(date, "%d-%d-%d", &month, &day, &year) != 3) {
        return 0;
    }
    return (long)year * 10000L + (long)month * 100L + (long)day;
}

/* Returns nonzero if email a should be read before email b. */
static int higherPriority(const Email *a, const Email *b)
{
    if (a->priority != b->priority) {
        return a->priority > b->priority;
    }
    if (a->dateKey != b->dateKey) {
        return a->dateKey > b->dateKey;
    }
    return a->seq > b->seq;
}

/* ------------------------------------------------------------------------- */
/* MaxHeap (list-based)                                                      */
/* ------------------------------------------------------------------------- */

typedef struct {
    Email *items;       /* array storage; children of i are 2i+1 and 2i+2 */
    size_t size;
    size_t capacity;
} MaxHeap;

static void heapInit(MaxHeap *heap)
{
    heap->items = NULL;
    heap->size = 0;
    heap->capacity = 0;
}

static void heapFree(MaxHeap *heap)
{
    size_t i;
    for (i = 0; i < heap->size; i++) {
        freeEmail(&heap->items[i]);
    }
    free(heap->items);
    heap->items = NULL;
    heap->size = 0;
    heap->capacity = 0;
}

static size_t heapSize(const MaxHeap *heap)
{
    return heap->size;
}

static int heapIsEmpty(const MaxHeap *heap)
{
    return heap->size == 0;
}

static void swapEmails(Email *a, Email *b)
{
    Email tmp = *a;
    *a = *b;
    *b = tmp;
}

static void siftUp(MaxHeap *heap, size_t index)
{
    while (index > 0) {
        size_t parent = (index - 1) / 2;
        if (!higherPriority(&heap->items[index], &heap->items[parent])) {
            break;
        }
        swapEmails(&heap->items[index], &heap->items[parent]);
        index = parent;
    }
}

static void siftDown(MaxHeap *heap, size_t index)
{
    for (;;) {
        size_t left = 2 * index + 1;
        size_t right = left + 1;
        size_t largest = index;

        if (left < heap->size &&
            higherPriority(&heap->items[left], &heap->items[largest])) {
            largest = left;
        }
        if (right < heap->size &&
            higherPriority(&heap->items[right], &heap->items[largest])) {
            largest = right;
        }
        if (largest == index) {
            break;
        }
        swapEmails(&heap->items[index], &heap->items[largest]);
        index = largest;
    }
}

/* Insert an email; the heap takes ownership of its strings. */
static void heapPush(MaxHeap *heap, Email email)
{
    if (heap->size == heap->capacity) {
        size_t newCapacity = (heap->capacity == 0) ? 16 : heap->capacity * 2;
        Email *grown = realloc(heap->items, newCapacity * sizeof(Email));
        if (grown == NULL) {
            fprintf(stderr, "Error: out of memory\n");
            exit(1);
        }
        heap->items = grown;
        heap->capacity = newCapacity;
    }
    heap->items[heap->size] = email;
    siftUp(heap, heap->size);
    heap->size++;
}

/* Returns the highest-priority email without removing it, or NULL if empty. */
static const Email *heapPeek(const MaxHeap *heap)
{
    if (heapIsEmpty(heap)) {
        return NULL;
    }
    return &heap->items[0];
}

/* Removes and frees the highest-priority email. Returns 0 if heap was empty. */
static int heapPop(MaxHeap *heap)
{
    if (heapIsEmpty(heap)) {
        return 0;
    }
    freeEmail(&heap->items[0]);
    heap->size--;
    if (heap->size > 0) {
        heap->items[0] = heap->items[heap->size];
        siftDown(heap, 0);
    }
    return 1;
}

/* ------------------------------------------------------------------------- */
/* Input handling                                                            */
/* ------------------------------------------------------------------------- */

/*
 * Reads one line of arbitrary length. Returns a malloc'd string without the
 * trailing newline, or NULL at end of input.
 */
static char *readLine(FILE *in)
{
    size_t capacity = 128;
    size_t length = 0;
    char *buffer = malloc(capacity);
    int ch;

    if (buffer == NULL) {
        fprintf(stderr, "Error: out of memory\n");
        exit(1);
    }

    while ((ch = fgetc(in)) != EOF) {
        if (ch == '\n') {
            break;
        }
        if (length + 1 >= capacity) {
            char *grown;
            capacity *= 2;
            grown = realloc(buffer, capacity);
            if (grown == NULL) {
                free(buffer);
                fprintf(stderr, "Error: out of memory\n");
                exit(1);
            }
            buffer = grown;
        }
        buffer[length++] = (char)ch;
    }

    if (ch == EOF && length == 0) {
        free(buffer);
        return NULL;
    }

    buffer[length] = '\0';
    return buffer;
}

/* Trims leading and trailing whitespace (including '\r') in place. */
static char *trim(char *s)
{
    char *end;

    while (*s != '\0' && isspace((unsigned char)*s)) {
        s++;
    }
    if (*s == '\0') {
        return s;
    }
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
    return s;
}

/* ------------------------------------------------------------------------- */
/* Commands                                                                  */
/* ------------------------------------------------------------------------- */

/* Parses "<sender category>,<subject line>,<date>" and inserts the email. */
static void handleEmail(MaxHeap *heap, char *fields, unsigned long seq)
{
    char *firstComma = strchr(fields, ',');
    char *lastComma = strrchr(fields, ',');
    char *sender;
    char *subject;
    char *date;
    Email email;

    if (firstComma == NULL || lastComma == firstComma) {
        return; /* malformed line; ignore */
    }

    *firstComma = '\0';
    *lastComma = '\0';

    sender = trim(fields);
    subject = trim(firstComma + 1);
    date = trim(lastComma + 1);

    email.sender = dupString(sender);
    email.subject = dupString(subject);
    email.date = dupString(date);
    email.priority = categoryPriority(email.sender);
    email.dateKey = dateToKey(email.date);
    email.seq = seq;

    heapPush(heap, email);
}

static void handleNext(const MaxHeap *heap)
{
    const Email *top = heapPeek(heap);
    if (top == NULL) {
        printf("No emails to read.\n");
        return;
    }
    printf("Next email:\n");
    printf("Sender: %s\n", top->sender);
    printf("Subject: %s\n", top->subject);
    printf("Date: %s\n", top->date);
}

static void handleRead(MaxHeap *heap)
{
    /* Reading with an empty inbox is a no-op. */
    heapPop(heap);
}

static void handleCount(const MaxHeap *heap)
{
    printf("There are %lu emails to read.\n", (unsigned long)heapSize(heap));
}

static void processInput(FILE *in, MaxHeap *heap)
{
    char *raw;
    unsigned long seq = 0;

    while ((raw = readLine(in)) != NULL) {
        char *line = trim(raw);

        if (strncmp(line, "EMAIL", 5) == 0 &&
            (line[5] == ' ' || line[5] == '\t')) {
            handleEmail(heap, line + 6, seq++);
        } else if (strcmp(line, "NEXT") == 0) {
            handleNext(heap);
        } else if (strcmp(line, "READ") == 0) {
            handleRead(heap);
        } else if (strcmp(line, "COUNT") == 0) {
            handleCount(heap);
        }
        /* Blank or unrecognized lines are ignored. */

        free(raw);
    }
}

int main(int argc, char *argv[])
{
    FILE *in = stdin;
    MaxHeap heap;

    if (argc > 1) {
        in = fopen(argv[1], "r");
        if (in == NULL) {
            fprintf(stderr, "Error: could not open file '%s'\n", argv[1]);
            return 1;
        }
    }

    heapInit(&heap);
    processInput(in, &heap);
    heapFree(&heap);

    if (in != stdin) {
        fclose(in);
    }

    return 0;
}
```

### Program B — Codex `gpt-5.6-sol`, `Codex/iterations/00-raw.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *sender;
    char *subject;
    char *date;
    int sender_priority;
    int date_value;
    unsigned long insertion_order;
} Email;

typedef struct {
    Email *items;
    size_t size;
    size_t capacity;
} MaxHeap;

static void allocation_error(void)
{
    fprintf(stderr, "Unable to allocate memory.\n");
    exit(EXIT_FAILURE);
}

static char *copy_string(const char *text)
{
    size_t length = strlen(text) + 1;
    char *copy = malloc(length);

    if (copy == NULL) {
        allocation_error();
    }
    memcpy(copy, text, length);
    return copy;
}

static int category_priority(const char *sender)
{
    if (strcmp(sender, "Boss") == 0) {
        return 5;
    }
    if (strcmp(sender, "Subordinate") == 0) {
        return 4;
    }
    if (strcmp(sender, "Peer") == 0) {
        return 3;
    }
    if (strcmp(sender, "ImportantPerson") == 0) {
        return 2;
    }
    return 1;
}

static int date_priority(const char *date)
{
    int month;
    int day;
    int year;

    if (sscanf(date, "%d-%d-%d", &month, &day, &year) != 3) {
        return 0;
    }
    return year * 10000 + month * 100 + day;
}

static int is_higher_priority(const Email *left, const Email *right)
{
    if (left->sender_priority != right->sender_priority) {
        return left->sender_priority > right->sender_priority;
    }
    if (left->date_value != right->date_value) {
        return left->date_value > right->date_value;
    }
    return left->insertion_order < right->insertion_order;
}

static void swap_emails(Email *left, Email *right)
{
    Email temporary = *left;
    *left = *right;
    *right = temporary;
}

static void heap_init(MaxHeap *heap)
{
    heap->items = NULL;
    heap->size = 0;
    heap->capacity = 0;
}

static void heap_insert(MaxHeap *heap, Email email)
{
    size_t index;

    if (heap->size == heap->capacity) {
        size_t new_capacity = heap->capacity == 0 ? 8 : heap->capacity * 2;
        Email *new_items = realloc(heap->items, new_capacity * sizeof(*new_items));

        if (new_items == NULL) {
            allocation_error();
        }
        heap->items = new_items;
        heap->capacity = new_capacity;
    }

    index = heap->size;
    heap->items[index] = email;
    heap->size++;

    while (index > 0) {
        size_t parent = (index - 1) / 2;

        if (!is_higher_priority(&heap->items[index], &heap->items[parent])) {
            break;
        }
        swap_emails(&heap->items[index], &heap->items[parent]);
        index = parent;
    }
}

static const Email *heap_max(const MaxHeap *heap)
{
    if (heap->size == 0) {
        return NULL;
    }
    return &heap->items[0];
}

static void free_email(Email *email)
{
    free(email->sender);
    free(email->subject);
    free(email->date);
}

static void heap_remove_max(MaxHeap *heap)
{
    size_t index = 0;

    if (heap->size == 0) {
        return;
    }

    free_email(&heap->items[0]);
    heap->size--;
    if (heap->size == 0) {
        return;
    }

    heap->items[0] = heap->items[heap->size];

    for (;;) {
        size_t left = index * 2 + 1;
        size_t right = left + 1;
        size_t largest = index;

        if (left < heap->size &&
            is_higher_priority(&heap->items[left], &heap->items[largest])) {
            largest = left;
        }
        if (right < heap->size &&
            is_higher_priority(&heap->items[right], &heap->items[largest])) {
            largest = right;
        }
        if (largest == index) {
            break;
        }
        swap_emails(&heap->items[index], &heap->items[largest]);
        index = largest;
    }
}

static void heap_destroy(MaxHeap *heap)
{
    size_t index;

    for (index = 0; index < heap->size; index++) {
        free_email(&heap->items[index]);
    }
    free(heap->items);
}

static char *read_line(FILE *input)
{
    size_t length = 0;
    size_t capacity = 128;
    char *line = malloc(capacity);
    int character;

    if (line == NULL) {
        allocation_error();
    }

    while ((character = fgetc(input)) != EOF && character != '\n') {
        if (length + 1 >= capacity) {
            char *larger_line;

            capacity *= 2;
            larger_line = realloc(line, capacity);
            if (larger_line == NULL) {
                free(line);
                allocation_error();
            }
            line = larger_line;
        }
        line[length++] = (char) character;
    }

    if (character == EOF && length == 0) {
        free(line);
        return NULL;
    }
    if (length > 0 && line[length - 1] == '\r') {
        length--;
    }
    line[length] = '\0';
    return line;
}

static void add_email_command(MaxHeap *heap, char *fields,
                              unsigned long insertion_order)
{
    char *first_comma = strchr(fields, ',');
    char *second_comma;
    Email email;

    if (first_comma == NULL) {
        return;
    }
    second_comma = strchr(first_comma + 1, ',');
    if (second_comma == NULL) {
        return;
    }

    *first_comma = '\0';
    *second_comma = '\0';

    email.sender = copy_string(fields);
    email.subject = copy_string(first_comma + 1);
    email.date = copy_string(second_comma + 1);
    email.sender_priority = category_priority(email.sender);
    email.date_value = date_priority(email.date);
    email.insertion_order = insertion_order;
    heap_insert(heap, email);
}

int main(int argc, char *argv[])
{
    FILE *input = stdin;
    MaxHeap heap;
    char *line;
    unsigned long insertion_order = 0;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [input_file]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            fprintf(stderr, "Unable to open input file: %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    }

    heap_init(&heap);
    while ((line = read_line(input)) != NULL) {
        if (strncmp(line, "EMAIL ", 6) == 0) {
            add_email_command(&heap, line + 6, insertion_order++);
        } else if (strcmp(line, "NEXT") == 0) {
            const Email *email = heap_max(&heap);

            if (email != NULL) {
                printf("Next email:\n");
                printf("Sender: %s\n", email->sender);
                printf("Subject: %s\n", email->subject);
                printf("Date: %s\n", email->date);
            }
        } else if (strcmp(line, "READ") == 0) {
            heap_remove_max(&heap);
        } else if (strcmp(line, "COUNT") == 0) {
            printf("There are %zu emails to read.\n", heap.size);
        }
        free(line);
    }

    heap_destroy(&heap);
    if (input != stdin) {
        fclose(input);
    }
    return 0;
}
```

## 2e. Analysis: correctness, execution time, space complexity, maintainability

Every number below came from a script I built with Claude's help,
`evidence/run-tests.sh`. It compiles both programs six different ways, feeds them
the same inputs, and prints exactly what each one did. I did not judge any of
this by reading the code. The full raw output is saved in
`evidence/run00-bench-local.txt`.

### What I could tell just by looking

Before running anything: Claude's file is longer, 382 lines against 293, split
into more small functions, and has real comments. Codex's is shorter and has
almost none. Both build the queue the same way and both handle dates the same
way.

That is genuinely all I could tell. I had an opinion about which one looked
nicer and no idea which one was better. The difference that turned out to matter
most is invisible at this level.

### Correctness

**Both match the assignment's example exactly.** Not "looks right" — I had the
computer compare the output to the expected text character by character, and
both are identical. I also tested both ways of feeding in the test file, as a
filename and piped in, and both programs handle both.

This is less impressive than it sounds. The example and its expected output are
printed inside `PROMPT.md`, and both models were allowed to compile and run their
own code, so both of them tested against that example while writing. It was never
going to catch anything. It is a floor, not a measurement.

**Both sort correctly at a size I could not check by hand.** Six emails proves
nothing. I generated 500 random emails, had each program read all of them and
spit them back out in order, and compared that order against a sorting program I
wrote separately in Python. Both got all 500 right.

**Neither program mishandles memory.** C makes you ask the operating system for
memory and give it back yourself, and forgetting either half is the classic C
bug. A tool called valgrind watches a program do this and reports mistakes. It
found zero errors and zero leaked memory in both.

**Where they disagree is exactly where the assignment stops talking.** These are
the cases the instructions promise the grader will test, while never showing what
the output should be:

| What happens | Claude | Codex |
|---|---|---|
| Ask for the next email when there are none | prints `No emails to read.` | prints nothing |
| Delete when there are none | nothing | nothing |
| Ask for the next email twice in a row | same answer twice | same answer twice |
| Delete twice in a row | both gone, silently | both gone, silently |
| Two emails, same sender type, same date | the later one in the file wins | the earlier one wins |
| A sender type that isn't one of the five | kept, sorted last | kept, sorted last |
| A malformed EMAIL line | skipped quietly | skipped quietly |
| An extra word after the filename | ignored | refuses to run |

Three real disagreements. The empty-inbox one is the one that could cost marks,
because the assignment names it specifically as something that gets tested, and
the two programs do visibly different things. The tie one is sneakier — it only
shows up when two emails share both a sender type and a date, which a grader's
file could contain without meaning to.

**One real bug, which will not cost me anything.** Both programs turn a date like
`12-25-2024` into a single number, 20241225, so that comparing dates is just
comparing numbers. Claude stores that number in a type that holds very large
values; Codex stores it in one that tops out around 2.1 billion. Give Codex a
date with a huge year and the number overflows — like an odometer rolling past
999999 back to 000000 — and the newest email in the inbox gets sorted as the
oldest.

I want to be straight about how much this matters: the assignment says dates are
four-digit years, and the biggest four-digit year fits in Codex's type with room
to spare. The grader will not send a six-digit year. This is a robustness
difference, not a points difference, and claiming otherwise would be inventing a
finding.

What it *is* worth is the shape of it. The same category of bug showed up in
Assignment 1, in the other model's file — there it was Claude that had it and
Codex that did not. So I am not willing to conclude that either model writes
safer C. Whichever one is careless is a coin flip per assignment, which is the
argument for running the tests every time instead of trusting a reputation.

### Execution time

This one is a non-finding, and I am reporting it as a non-finding rather than
pretending there is a story.

A test file has maybe a dozen commands in it. At that size neither program's
speed is measurable at all. So I pushed until it would be: 500,000 emails in and
all of them read back out.

| Emails | Claude | Codex |
|---|---|---|
| 5,000 | 0.00 s | 0.005 s |
| 50,000 | 0.039 s | 0.041 s |
| 500,000 | 0.687 s | 0.672 s |

Two percent apart at the largest size, and which one is ahead flips depending on
the run. That is measurement noise, not a difference. Both programs use the same
approach, so they cost the same. There is no execution-time story here and I am
not going to manufacture one.

One thing I did carry over from Assignment 1: I only compared the CPU time spent
in the program itself. The other timing numbers a computer gives you mostly
measure how busy the machine was, not how good the program is.

### Space complexity

Assignment 1 taught me that measuring total memory is useless, because almost all
of it is the C library loading and the number wobbles more between runs of the
same program than it does between two different programs. So I measured the part
that actually belongs to these programs: the memory they request as emails
arrive.

For 10,000 emails, Claude peaks at 1.30 MB and Codex at 1.17 MB. About 11%, and
unlike Assignment 1's memory numbers this one is real and repeats, because it is
not noise — it comes straight from a design choice I can point to in the source.
Codex uses a smaller type for each email's date, so each email costs 40 bytes
instead of 48.

The interesting part is that this is the *same decision* as the overflow bug
above, seen from the other side. Codex is leaner precisely because it picked the
narrower type, and it overflows for precisely the same reason. It bought 130 KB
and paid for it in margin. At the sizes this assignment cares about, neither the
saving nor the cost is worth anything.

Both programs grow their storage as emails arrive and hand it all back at the
end, so memory grows in proportion to the number of emails and nothing is wasted.

### Maintainability

**Comments.** Claude has 53 comment lines, Codex has 4. The rubric pays 14 points
for commenting and asks for a comment on every line. Neither model comes close to
that, but the gap between them is more than tenfold, and it is the single most
visible difference if you open both files.

**Neither one wrote a proper header.** The rubric lists nine things that belong
at the top of the file: program name, description, inputs, outputs, author,
creation date, revision date, collaborators, and any other source used including
GenAI. I checked both files against all nine. Claude has two of them. Codex has
none — its first line is `#include <stdio.h>`.

This is the most expensive thing in the entire comparison and it is a direct
repeat of Assignment 1. Neither model wrote my name. Neither model mentioned that
a GenAI wrote the code. Neither was asked to, and neither volunteered it. That is
the real lesson about working with these tools: they answer the question you
asked and nothing more.

**Older compilers.** Codex uses one piece of newer C notation that an older
compiler complains about; Claude does not. Both still build fine in the grading
environment, so this changes nothing practical. It is worth a line only because
Codex did the same kind of thing in Assignment 1, so it looks like a habit rather
than an accident.

## 2f. Which one was selected, and why

**I am building on Claude's version.**

The honest reason is that it is closer to what the rubric pays for. It has more
than ten times the comments, it has the beginnings of a header block, and it
splits the work into named pieces so that changing one command does not mean
reading the whole file.

I want to be clear about what did *not* decide this, because the measurements
were mostly ties:

- Correctness on everything the assignment actually specifies: tie. Both match
  the example exactly, both sort 500 emails correctly, both are clean under
  valgrind.
- Speed: tie, and not measurable at real sizes.
- Memory: Codex is 11% leaner, on a number too small to care about.
- The overflow bug: real, but only reachable with input the assignment forbids.

So this is not a case where one program is good and the other is broken. Both are
competent. I picked the one that needs less work to reach the standard the
assignment is grading against, and the work it needs less of — writing comments
and a header — is the most tedious work to do by hand.

The thing I keep coming back to is that I could not have made this call by
reading the two files. Almost everything I believed at first glance turned out to
be either wrong or irrelevant, and the two differences that could actually cost
me marks — the empty inbox and the tie order — are invisible until you run the
programs on inputs the assignment never shows you.

## 2g. What was done to improve it

*This section is written before the improved version exists, so that what I
planned is on the record next to what I got. Results are filled in below it once
run 01 is done.*

The comparison produced a short list of things to fix, and every one of them
becomes a standing rule in `AGENT.md` — the file in each model's folder that says
*how* code must be written, as opposed to `PROMPT.md`, which says what it must
do. Writing the rules there instead of fixing the file by hand means the fix
applies to every future run, and means I can measure whether the rules worked.

**Correctness.** Settle the three places the two models disagreed, since the
assignment leaves them open and the grader will hit them: what prints when the
inbox is empty, which email wins when the sender type and the date are both
equal, and what happens with an unexpected command-line argument. Also require
the date to be held in a type that cannot overflow, which is a one-word change
and removes the only genuine defect either program had.

**Maintainability, which is where the points are.** Require the nine-item header
block, by name, including my name and the statement that a GenAI wrote the
original. Require a comment on every line, written for someone who knows a year
of Python and no C — explaining why a line is there rather than restating what it
says. This is the single largest scoring change available and neither model does
it unprompted.

**Execution time and space.** Nothing to do. The measurements say the two
programs are the same, and the sizes involved are far too small for any
optimisation to be worth the risk of breaking working code. Changing them would
be decoration, and I would rather say that plainly than invent an improvement.

**Then measure whether it worked.** Run both models again against the same
`PROMPT.md` with the new `AGENT.md` in place, save the results as iteration 01,
and re-run the same test script. If the rules worked, the disagreements are gone
and the headers are present. If they did not, that is also a result, and a more
interesting one.

## Collaborator disclosure

Everything and everyone that contributed to this assignment, listed because the
rubric requires it and because leaving any of it out is treated as academic
misconduct.

**The two GenAIs being compared.** Claude Opus 5 (Anthropic) and GPT-5.6 Sol
(OpenAI, through Codex) each independently wrote a complete C program from
`PROMPT.md`. Both raw programs are reproduced in section 2d. The final submitted
program is built from Claude's version, as explained in 2f.

**Claude (Anthropic), in a separate session, as an assistant to me.** This is a
third use of a GenAI, separate from the two being compared, and it did the
following: helped draft this analysis document, wrote the test script
`evidence/run-tests.sh` and the session-log reader `evidence/session-report.py`
at my direction, ran the measurements reported in 2e, and helped write
`AGENT.md`. I reviewed everything it produced before using it. No part of the
submitted C program came from this session.

**Lucas**, a friend, who pointed out during Assignment 1 how integer overflow
works — the idea I reused to find the date bug described in 2e.

**The course materials**, for the compile command used throughout
(`gcc file.c -o file`, from the C Basics lecture).

No other person, website, or tool contributed code to this assignment.

## Appendix A — `PROMPT.md` as given to both systems

*To be pasted verbatim.*


---

## Generation cost — measured, not self-reported

*Source: each CLI's own session log, parsed by `evidence/session-report.py`.
Reported separately from rubric 2e, which concerns the runtime of the compiled
program and is unrelated to these numbers.*

### Run 00 — generation prompt only

Both systems were asked for the program, then asked separately for the report.
The figures below cover **only the generation prompt**, so the two are
comparable; the report prompt is excluded. Cache reads are excluded from the
work figure because they are text the provider had already processed.

| | Claude | Codex |
|---|---|---|
| Model | `claude-opus-5` | `gpt-5.6-sol`, effort `xhigh` |
| Harness | Claude Code | Codex Desktop 0.153.4 (VS Code) |
| Wall clock | 1 m 59 s | 1 m 41 s |
| API requests / responses | 7 | 6 |
| Output tokens | 10,282 | 4,874 |
| of which reasoning | (not itemised) | 1,454 |
| **Fresh, non-cache-read tokens** | **39,183** | **19,201** |
| Cache reads (excluded above) | 421,446 | 156,544 |
| Program produced | 382 lines, 10,141 B | 293 lines, 6,780 B |
| Compiled and tested itself? | yes | yes |
| Disclosed rule violation? | yes, unprompted | yes, unprompted |

Claude spent roughly twice the fresh tokens and produced roughly 1.5× the code in
about 18 seconds more wall clock. Whether the extra output bought anything is a
question for 2e, not for this table — length is not quality, and the correctness
bench has not been run yet.

A caution on the Codex figure: its own report put the generation prompt at
1 minute 46.157 seconds from task metadata, while the rollout log's first and
last timestamps for that turn give 101 seconds. The two clocks start at slightly
different moments. The log figure is used for both systems for consistency, and
the discrepancy is recorded rather than averaged away.

