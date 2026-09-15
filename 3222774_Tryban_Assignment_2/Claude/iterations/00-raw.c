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
