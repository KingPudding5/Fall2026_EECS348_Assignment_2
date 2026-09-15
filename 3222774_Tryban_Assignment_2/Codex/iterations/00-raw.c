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
