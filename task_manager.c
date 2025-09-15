#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TASKS_PATH "./data/tasks.txt"
#define TEMP_TASKS_PATH "./data/temp_tasks.txt"

#define TASK_NAME_SIZE 201
#define LINE_BUFFER_SIZE 301

struct Task {
    int id;
    char name[TASK_NAME_SIZE];
    bool done;
};

bool parse_task(const char *line, struct Task *t) {
    int done_val;
    if (sscanf(line, "%d|%200[^|]|%d", &t->id, t->name, &done_val) == 3) {
        t->done = done_val == 1;
        return true;
    }
    return false;
}

void ensure_data_file() {
    system("mkdir -p data"); 
    FILE *f = fopen(TASKS_PATH, "a");
    if (f) fclose(f);
}

void update_task_file(struct Task updated, bool delete) {
    FILE *f = fopen(TASKS_PATH, "r");
    FILE *temp = fopen(TEMP_TASKS_PATH, "w");

    if (!f || !temp) {
        perror("Failed to open files");
        return;
    }

    char line[LINE_BUFFER_SIZE];
    struct Task t;

    while (fgets(line, sizeof(line), f)) {
        if (parse_task(line, &t)) {
            if (t.id == updated.id) {
                if (!delete) {
                    fprintf(temp, "%d|%s|%d\n", updated.id, updated.name, updated.done ? 1 : 0);
                }
                // else skip line to delete
            } else {
                fputs(line, temp);
            }
        }
    }

    fclose(f);
    fclose(temp);

    if (remove(TASKS_PATH) != 0) perror("Failed to remove old tasks file");
    if (rename(TEMP_TASKS_PATH, TASKS_PATH) != 0) perror("Failed to rename temp tasks file");
}

int make_task_id() {
    FILE *f = fopen(TASKS_PATH, "r");
    if (!f) return 1;

    int id, done;
    char name[TASK_NAME_SIZE];
    int max_id = 0;

    while (fscanf(f, "%d|%200[^|]|%d", &id, name, &done) == 3) {
        if (id > max_id) max_id = id;
    }

    fclose(f);
    return max_id + 1;
}

int list_tasks() {
    FILE *f = fopen(TASKS_PATH, "r");
    if (!f) {
        perror("Failed to open tasks file");
        return 1;
    }

    char line[LINE_BUFFER_SIZE];
    struct Task t;
    bool empty = true;

    while (fgets(line, sizeof(line), f) != NULL) {
        if (parse_task(line, &t)) {
            empty = false;
            printf("%d. %s [%s]\n", t.id, t.name, t.done ? "Done" : "Not Done");
        }
    }

    if (empty) printf("No tasks found\n");

    fclose(f);
    return 0;
}

int add_task() {
    struct Task task;
    printf("Task (max 200 characters): ");
    fgets(task.name, sizeof(task.name), stdin);
    task.name[strcspn(task.name, "\n")] = '\0';

    task.id = make_task_id();
    task.done = false;

    FILE *f = fopen(TASKS_PATH, "a");
    if (!f) {
        perror("Failed to open tasks file");
        return 1;
    }

    fprintf(f, "%d|%s|%d\n", task.id, task.name, task.done);
    fclose(f);

    printf("Task added:\n    ID: %d\n    Task: %s\n", task.id, task.name);
    return 0;
}

struct Task get_task(int id) {
    struct Task t = {0, "", false};
    FILE *f = fopen(TASKS_PATH, "r");
    if (!f) {
        perror("Failed to open tasks file");
        return t;
    }

    char line[LINE_BUFFER_SIZE];
    while (fgets(line, sizeof(line), f) != NULL) {
        struct Task temp;
        if (parse_task(line, &temp) && temp.id == id) {
            fclose(f);
            return temp;
        }
    }

    fclose(f);
    return t;
}

int edit_task() {
    char buffer[20];
    printf("Enter task ID: ");
    fgets(buffer, sizeof(buffer), stdin);
    int task_id = atoi(buffer);

    struct Task t = get_task(task_id);
    if (strcmp(t.name, "") == 0) {
        printf("Could not find task with ID: %d\n", task_id);
        return 0;
    }

    while (1) {
        printf("1. Edit task [%s]\n2. Toggle status [%s]\n3. Return\n\n>> ", t.name, t.done ? "Done" : "Not Done");
        fgets(buffer, sizeof(buffer), stdin);
        int command = atoi(buffer);
        printf("\n");

        switch (command) {
            case 1:
                printf("New task: ");
                fgets(t.name, sizeof(t.name), stdin);
                t.name[strcspn(t.name, "\n")] = '\0';
                update_task_file(t, false);
                printf("Updated successfully.\n");
                return 0;
            case 2:
                t.done = !t.done;
                update_task_file(t, false);
                printf("Changed task status to: %s\n", t.done ? "Done" : "Not Done");
                return 0;
            case 3:
                printf("Returning...\n");
                return 0;
            default:
                printf("Invalid option\n\n");
        }
    }
}

int delete_task() {
    char buffer[20];
    printf("Enter task ID: ");
    fgets(buffer, sizeof(buffer), stdin);
    int task_id = atoi(buffer);

    struct Task t = get_task(task_id);
    if (strcmp(t.name, "") == 0) {
        printf("Could not find task with ID: %d\n", task_id);
        return 0;
    }

    while (1) {
        printf("Task information:\n    ID: %d\n    Task: %s\n    Status: %s\n\n", t.id, t.name, t.done ? "Done" : "Not Done");
        printf("Are you sure you want to delete this task? [y/n]: ");
        fgets(buffer, sizeof(buffer), stdin);
        char confirmation = buffer[0];

        switch (confirmation) {
            case 'y':
            case 'Y':
                update_task_file(t, true);
                printf("Deleted task with id: %d\n", t.id);
                return 0;
            case 'n':
            case 'N':
                printf("Cancelling...\n");
                return 0;
            default:
                printf("Invalid response, please only provide y or n [yes, no]\n\n");
        }
    }
}

int main(void) {
    ensure_data_file();

    char buffer[10];
    while (1) {
        printf("\n1. View tasks\n2. Add task\n3. Edit task\n4. Delete task\n5. Quit\n\n>> ");
        fgets(buffer, sizeof(buffer), stdin);
        int command = atoi(buffer);
        printf("\n");

        switch (command) {
            case 1: list_tasks(); break;
            case 2: add_task(); break;
            case 3: edit_task(); break;
            case 4: delete_task(); break;
            case 5: printf("Quitting...\n"); return 0;
            default: printf("Invalid option\n\n");
        }
    }
}