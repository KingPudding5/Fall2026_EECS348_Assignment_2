/* Program name: EECS 348 Assignment 2
Brief description: prioritize a CEO's emails with a max heap
Inputs: email commands from a text file or standard input
Outputs: unread counts and the next email to read
Author: Francis Tryban
Creation date: 09-17-2026
Revision date: 09-17-2026
Revisions: initial version
Collaborators and other sources: OpenAI Codex wrote this code */

#include <stdio.h> // provide file input and printed output
#include <stdlib.h> // provide memory and exit status tools
#include <string.h> // provide text copying and comparison tools

typedef struct { // keep the details of one email together
    char *sender; // the star means this holds the text's address
    char *subject; // point to the subject in the same saved text
    char *date; // point to the date in the same saved text
    int category; // store the sender's priority number
    long date_key; // store a number that puts newer dates first
    unsigned long long arrival; // remember which email came first
} Email; // name this group of email details

typedef struct { // keep the heap's list and its sizes together
    Email *items; // use an array instead of linked nodes
    size_t count; // size_t stores a nonnegative count of items
    size_t capacity; // remember how many items fit in the list
} MaxHeap; // name the priority queue

static int category_priority(const char *sender) { // static keeps this helper inside this file
    if (strcmp(sender, "Boss") == 0) { // check for the highest sender rank
        return 5; // put boss emails ahead of the others
    } // end of boss check
    if (strcmp(sender, "Subordinate") == 0) { // check for the next sender rank
        return 4; // put subordinate emails second
    } // end of subordinate check
    if (strcmp(sender, "Peer") == 0) { // check for the middle sender rank
        return 3; // put peer emails third
    } // end of peer check
    if (strcmp(sender, "ImportantPerson") == 0) { // check for the next sender rank
        return 2; // put important person emails fourth
    } // end of important person check
    return 1; // put other person emails last
} // end of category_priority

static long make_date_key(const char *date) { // turn a date into a sortable number
    int month = 0; // prepare a place for the month
    int day = 0; // prepare a place for the day
    int year = 0; // prepare a place for the year
    sscanf(date, "%d-%d-%d", &month, &day, &year); // ampersands give the parser places to store numbers
    return (long)year * 10000L + month * 100L + day; // use long so the year stays safe when multiplied
} // end of make_date_key

static int higher_priority(const Email *left, const Email *right) { // decide which email comes first
    if (left->category != right->category) { // arrows read fields through the email addresses
        return left->category > right->category; // higher sender rank wins
    } // end of category check
    if (left->date_key != right->date_key) { // compare dates within one sender rank
        return left->date_key > right->date_key; // newer email wins
    } // end of date check
    return left->arrival < right->arrival; // earlier file arrival wins a full tie
} // end of higher_priority

static void swap_emails(Email *first, Email *second) { // exchange two places in the heap
    Email saved = *first; // save one email before overwriting it
    *first = *second; // move the other email into the first place
    *second = saved; // put the saved email into the second place
} // end of swap_emails

static void heap_init(MaxHeap *heap) { // start an empty heap
    heap->items = NULL; // null means there is no list address yet
    heap->count = 0; // begin with no emails
    heap->capacity = 0; // begin with no reserved places
} // end of heap_init

static int heap_push(MaxHeap *heap, Email email) { // add an email in priority order
    size_t index; // remember the new email's place
    if (heap->count == heap->capacity) { // grow the list when it is full
        size_t next_capacity = heap->capacity == 0 ? 8 : heap->capacity * 2; // double it instead of growing one place at a time
        Email *larger; // keep the new list address separate until it works
        if (next_capacity < heap->capacity || next_capacity > (size_t)-1 / sizeof(Email)) { // stop if the requested size cannot fit
            return 0; // report that the list could not grow
        } // end of size check
        larger = realloc(heap->items, next_capacity * sizeof(Email)); // ask the system to grow the list
        if (larger == NULL) { // check whether the system gave us memory
            return 0; // keep the old list for cleanup
        } // end of memory check
        heap->items = larger; // use the larger list
        heap->capacity = next_capacity; // remember its new limit
    } // end of growth check
    index = heap->count; // place the new email at the end
    heap->items[index] = email; // store the email in that place
    heap->count++; // count the added email
    while (index > 0) { // move the email upward while it outranks its parent
        size_t parent = (index - 1) / 2; // find the parent in the list
        if (!higher_priority(&heap->items[index], &heap->items[parent])) { // compare addresses without copying emails
            break; // stop once the heap order is right
        } // end of priority check
        swap_emails(&heap->items[index], &heap->items[parent]); // lift the higher priority email
        index = parent; // continue from the parent's old place
    } // end of upward move
    return 1; // report that the email was added
} // end of heap_push

static const Email *heap_top(const MaxHeap *heap) { // find the next email without removing it
    if (heap->count == 0) { // check for an empty inbox
        return NULL; // report that no email is available
    } // end of empty check
    return &heap->items[0]; // return the address of the highest priority email
} // end of heap_top

static void heap_pop(MaxHeap *heap) { // remove the highest priority email
    size_t index = 0; // start fixing the heap at the top
    if (heap->count == 0) { // allow a read command on an empty inbox
        return; // there is nothing to remove
    } // end of empty check
    free(heap->items[0].sender); // give back the saved text memory by hand
    heap->count--; // leave one fewer email in the heap
    if (heap->count == 0) { // check whether the inbox is now empty
        return; // no other email needs to move
    } // end of last email check
    heap->items[0] = heap->items[heap->count]; // move the last email to the top
    for (;;) { // move it downward until the heap order is right
        size_t left = index * 2 + 1; // find the left child in the list
        size_t right = left + 1; // find the right child in the list
        size_t best = index; // begin with the current email as the best
        if (left < heap->count && higher_priority(&heap->items[left], &heap->items[best])) { // check the left child's rank
            best = left; // choose the left child when it ranks higher
        } // end of left check
        if (right < heap->count && higher_priority(&heap->items[right], &heap->items[best])) { // check the right child's rank
            best = right; // choose the right child when it ranks higher
        } // end of right check
        if (best == index) { // check whether the current place is right
            break; // stop moving the email
        } // end of order check
        swap_emails(&heap->items[index], &heap->items[best]); // move the best child upward
        index = best; // continue from the child's old place
    } // end of downward move
} // end of heap_pop

static void heap_clear(MaxHeap *heap) { // release every email still in the inbox
    size_t index; // walk through the remaining emails
    for (index = 0; index < heap->count; index++) { // visit each stored email once
        free(heap->items[index].sender); // give back this email's saved text
    } // end of email cleanup
    free(heap->items); // give back the heap's list
} // end of heap_clear

static int read_line(FILE *input, char **result) { // read one whole command line
    size_t length = 0; // count the characters read so far
    size_t capacity = 128; // begin with space for a short command
    char *line = malloc(capacity); // ask the system for a growing line buffer
    int character; // hold a character or the end of input
    if (line == NULL) { // check whether the buffer was provided
        return -1; // report a memory problem
    } // end of memory check
    while ((character = fgetc(input)) != EOF && character != '\n') { // collect characters through the line ending
        if (length + 1 >= capacity) { // leave room for the end marker
            char *larger; // keep the old buffer until growth works
            if (capacity > (size_t)-1 / 2) { // check whether doubling would overflow
                free(line); // release the buffer before reporting failure
                return -1; // report that the line cannot grow
            } // end of size check
            capacity *= 2; // make room for a longer subject
            larger = realloc(line, capacity); // ask the system for a larger buffer
            if (larger == NULL) { // check whether the larger buffer was provided
                free(line); // release the old buffer before leaving
                return -1; // report a memory problem
            } // end of memory check
            line = larger; // use the larger buffer
        } // end of growth check
        line[length++] = (char)character; // add this character to the line
    } // end of character reading
    if (character == EOF && length == 0) { // check for the end of all commands
        free(line); // release the unused buffer
        return ferror(input) ? -1 : 0; // report an input error or normal end
    } // end of end of input check
    if (length > 0 && line[length - 1] == '\r') { // accept windows line endings too
        length--; // remove the extra line ending character
    } // end of line ending check
    line[length] = '\0'; // mark the end of the command text
    *result = line; // give the completed line back to the caller
    return 1; // report that a command was read
} // end of read_line

static int command_email(MaxHeap *heap, const char *fields, unsigned long long *arrival) { // save one new email
    Email email; // hold the new email's details
    char *first_comma; // mark the end of the sender
    char *second_comma; // mark the end of the subject
    email.sender = malloc(strlen(fields) + 1); // save one copy instead of three separate text blocks
    if (email.sender == NULL) { // check whether the text memory was provided
        return 0; // report a memory problem
    } // end of memory check
    strcpy(email.sender, fields); // copy the fields before the input line is freed
    first_comma = strchr(email.sender, ','); // find the sender separator
    if (first_comma == NULL) { // guard against a missing separator
        free(email.sender); // release the unused text
        return 1; // ignore a line with missing fields
    } // end of first separator check
    second_comma = strchr(first_comma + 1, ','); // find the subject separator
    if (second_comma == NULL) { // guard against a missing separator
        free(email.sender); // release the unused text
        return 1; // ignore a line with missing fields
    } // end of second separator check
    *first_comma = '\0'; // end the sender text at its comma
    *second_comma = '\0'; // end the subject text at its comma
    email.subject = first_comma + 1; // point to the subject in saved text
    email.date = second_comma + 1; // point to the date in saved text
    email.category = category_priority(email.sender); // set the sender rank
    email.date_key = make_date_key(email.date); // set the sortable date
    email.arrival = (*arrival)++; // save file order and advance the counter
    if (!heap_push(heap, email)) { // check whether the heap accepted the email
        free(email.sender); // release the text if adding failed
        return 0; // report a memory problem
    } // end of heap check
    return 1; // report that the email was saved
} // end of command_email

static void command_next(const MaxHeap *heap) { // show the next email without removing it
    const Email *email = heap_top(heap); // look at the highest priority email
    if (email == NULL) { // check for an empty inbox
        puts("No emails to read."); // tell the ceo there is nothing to read
        return; // finish the command
    } // end of empty check
    puts("Next email:"); // introduce the next email
    printf("Sender: %s\n", email->sender); // show its sender
    printf("Subject: %s\n", email->subject); // show its subject
    printf("Date: %s\n", email->date); // show its original date text
} // end of command_next

static void command_read(MaxHeap *heap) { // deal with the highest priority email
    heap_pop(heap); // remove it without printing it
} // end of command_read

static void command_count(const MaxHeap *heap) { // show how many emails remain
    printf("There are %zu emails to read.\n", heap->count); // print the unread count
} // end of command_count

static int handle_command(MaxHeap *heap, char *line, unsigned long long *arrival) { // send a line to its command function
    if (strncmp(line, "EMAIL ", 6) == 0) { // identify an email command
        return command_email(heap, line + 6, arrival); // add the email to the heap
    } // end of email command check
    if (strcmp(line, "NEXT") == 0) { // identify a next command
        command_next(heap); // show the next email
    } else if (strcmp(line, "READ") == 0) { // identify a read command
        command_read(heap); // remove the next email
    } else if (strcmp(line, "COUNT") == 0) { // identify a count command
        command_count(heap); // show the unread count
    } // end of command checks
    return 1; // report that command handling succeeded
} // end of handle_command

int main(int argc, char *argv[]) { // read commands and hand each one to its function
    FILE *input = stdin; // use standard input when no file was named
    MaxHeap heap; // hold all unread emails
    unsigned long long arrival = 0; // number emails in their file order
    char *line; // hold one command at a time
    int status; // remember whether reading worked
    if (argc > 2) { // allow only one optional filename
        fprintf(stderr, "Usage: %s [input_file]\n", argv[0]); // explain valid use on the error stream
        return EXIT_FAILURE; // stop after an argument error
    } // end of argument check
    if (argc == 2) { // check whether a filename was supplied
        input = fopen(argv[1], "r"); // open that file for reading
        if (input == NULL) { // check whether the file opened
            fprintf(stderr, "Unable to open input file: %s\n", argv[1]); // explain the file problem
            return EXIT_FAILURE; // stop after a file error
        } // end of file check
    } // end of filename check
    heap_init(&heap); // give the heap's address so it can be initialized
    while ((status = read_line(input, &line)) == 1) { // read commands until input ends
        int handled = handle_command(&heap, line, &arrival); // pass the line to its command function
        free(line); // give back each line after it is handled
        if (!handled) { // check whether a command ran out of memory
            status = -1; // remember the failure for cleanup
            break; // stop reading more commands
        } // end of command failure check
    } // end of command reading
    heap_clear(&heap); // release the remaining emails and list
    if (input != stdin) { // check whether this program opened a file
        fclose(input); // close the file that it opened
    } // end of file cleanup
    if (status == -1) { // check whether reading or saving failed
        fputs("Unable to read input or allocate memory.\n", stderr); // explain the failure
        return EXIT_FAILURE; // report an unsuccessful run
    } // end of failure check
    return EXIT_SUCCESS; // report that all commands were handled
} // end of main
