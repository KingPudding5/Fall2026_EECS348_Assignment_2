# AGENT.md

Standing rules for this folder. PROMPT.md says what the program must do. This
file says how it must be written. Both apply. If they ever conflict, PROMPT.md
wins.

## Who is reading this code

Me. I have about a year of Python and no C. Write for that reader.

## Comments

Every line gets a comment. Match this style — it is mine, copy it:

```c
#include <iostream> //include the iostream library
template <typename T> //declare the function "template" type T
T maximum(T a, T b) { // define a function named maximum that takes a and b of type T and returns type T
	return (a > b) ? a : b; // return the greater of a and b (a if true b if false)
}
int main() { // define the programs main entry point. Returning an int
	int num1 = 10; //define num1 as as an int 10
	std::cout << "Maximum of " << num1 << " is: " << max_int << std::endl;
	// ^ print output answer in sentence
	return 0; // exit the program and return 0
}// end of main func
```

- Short, lowercase, on the same line, no period on the end.
- Say what the line does in plain words. Do not restate the syntax.
- If the line does something C makes you do by hand that Python does for you,
  say that. `malloc` is "ask the system for memory", not "call malloc".
- The first time a piece of C notation shows up, explain it once: `*`, `&`,
  `->`, `malloc`, `free`, `size_t`, `static`, `NULL`.
- Where there was a real choice, name the option you did not take.
- If the line is too long to comment on the end, put the comment on the next
  line starting with `// ^`.
- Close a function with `} // end of <name>`.
- No name, initials, model tag, or date on any individual comment.
- Write them the way I would. Short and plain beats polished.

## Header

One block comment at the top of the .c file. Once, nowhere else. All nine of
these, each on its own line:

- Program name — EECS 348 Assignment 2
- Brief description of what the program does
- Inputs
- Outputs
- Author — Francis Tryban
- Creation date
- Revision date
- Revisions
- Collaborators and other sources — name the GenAI that wrote this code

## Correctness

- `NEXT` on an empty inbox prints `No emails to read.` Never print nothing.
- Same sender category **and** same date: the email that came first in the file
  is read first.
- Hold the date key in a `long`, not an `int`. `year * 10000` overflows an int.
- More than one command-line argument is an error. Print usage to stderr and
  exit non-zero.
- Every command gets its own function. `main` reads lines and hands off; it does
  not do the work itself.
- Check what every `malloc` and `realloc` hands back before using it.
- Free everything before the program exits.
- It must compile silently under `gcc -Wall -Wextra -O2 file.c -o file`.

## Scope

Write only the file PROMPT.md names. Do not create, rename, or edit anything
else. Do not add features nobody asked for.
