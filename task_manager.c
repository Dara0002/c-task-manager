#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define tasks_path "./data/tasks.txt"
#define temp_tasks_path "./data/temp_tasks.txt"

#define TASK_NAME_SIZE 201
#define LINE_BUFFER_SIZE 301

struct Task
{
    int id;
    char name[TASK_NAME_SIZE];
    bool done;
};

bool parse_task(const char *line, struct Task *t)
{
    int done_val;
    if (sscanf(line, "%d|%199[^|]|%d", &t->id, t->name, &done_val) == 3)
    {
        t->done = done_val == 1;
        return true;
    }

    return false;
}

void update_task_file(struct Task updated, bool delete)
{
    FILE *f = fopen(tasks_path, "r");
    FILE *temp = fopen(temp_tasks_path, "w");

    if (!f)
    {
        perror("Failed to open task");
        return;
    }
    if (!temp)
    {
        perror("Failed to open temp_tasks");
        return;
    }

    char line[LINE_BUFFER_SIZE];
    struct Task t;

    while (fgets(line, sizeof(line), f))
    {
        if (parse_task(line, &t))
        {
            if (t.id == updated.id)
            {
                if (delete)
                {
                    continue;
                }
                else
                {
                    fprintf(temp, "%d|%s|%d\n", updated.id, updated.name, updated.done ? 1 : 0);
                }
            }
            else
            {
                fputs(line, temp);
            }
        }
    }

    fclose(f);
    fclose(temp);

    remove(tasks_path);
    rename(temp_tasks_path, tasks_path);
}

int make_task_id()
{
    FILE *f = fopen(tasks_path, "r");
    if (!f)
    {
        perror("Failed to open tasks file");
        return 1;
    }

    int id, done;
    char name[TASK_NAME_SIZE];
    int max_id = 0;

    while (fscanf(f, "%d|%199[^|]|%d", &id, name, &done) == 3)
    {
        if (id > max_id)
        {
            max_id = id;
        }
    }

    fclose(f);

    return max_id + 1;
}

int list_tasks()
{
    FILE *f = fopen(tasks_path, "r");
    if (!f)
    {
        perror("Failed to open tasks file");
        return 1;
    }

    char line[LINE_BUFFER_SIZE];
    struct Task t;

    if (fgets(line, sizeof(line), f) == NULL) {
        printf("No tasks found\n");
        return 0;
    }

    while (fgets(line, sizeof(line), f) != NULL)
    {
        if (parse_task(line, &t))
        {
            printf("%d. %s [%s]\n", t.id, t.name, t.done ? "Done" : "Not Done");
        }
    }

    fclose(f);
    return 0;
}

int add_task()
{
    struct Task task;
    printf("Task (max 200 characters): ");
    fgets(task.name, sizeof(task.name), stdin);
    task.name[strcspn(task.name, "\n")] = '\0';

    task.id = make_task_id();
    task.done = false;

    FILE *f = fopen(tasks_path, "a");
    if (!f)
    {
        perror("Failed to open tasks file");
        return 1;
    }

    fprintf(f, "%d|%s|%d\n", task.id, task.name, task.done);
    fclose(f);

    printf("Task added:\n    ID: %d\n    Task: %s\n", task.id, task.name);
    return 0;
}

struct Task get_task(int id)
{
    struct Task t = {0, "", false};
    FILE *f = fopen(tasks_path, "r");
    if (!f)
    {
        perror("Failed to open tasks file");
        return t;
    }

    char line[LINE_BUFFER_SIZE];
    while (fgets(line, sizeof(line), f) != NULL)
    {
        struct Task temp;
        if (parse_task(line, &temp))
        {
            if (temp.id == id)
            {
                fclose(f);
                return temp;
            }
        }
    }

    fclose(f);
    return t;
}

int edit_task()
{
    int task_id;

    printf("Enter task ID: ");
    scanf("%d%*c", &task_id);
    printf("\n");

    struct Task t = get_task(task_id);
    if (strcmp(t.name, "") == 0)
    {
        printf("Could not find task with ID: %d\n", task_id);
        return 0;
    }

    int command;

    do
    {
        printf("1. Edit task [%s]\n2. Toggle status [%s]\n3. Return\n\n>> ", t.name, t.done ? "Done" : "Not Done");
        scanf("%d%*c", &command);
        printf("\n");

        switch (command)
        {
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
            break;
        }
    } while (command != 1 && command != 2 && command != 3);
}

int delete_task()
{
    int task_id;
    printf("Enter task ID: ");
    scanf("%d%*c", &task_id);
    printf("\n");

    struct Task t = get_task(task_id);
    if (strcmp(t.name, "") == 0)
    {
        printf("Could not find task with ID: %d\n", task_id);
        return 0;
    }

    char confirmation;

    do
    {
        printf("Task information:\n    ID: %d\n    Task: %s\n    Status: %s\n\n", t.id, t.name, t.done ? "Done" : "Not Done");
        printf("Are you sure you want to delete this task? [y/n]: ");
        scanf("%c", &confirmation);
        while (getchar() != '\n')
            ;
        printf("\n");

        switch (confirmation)
        {
        case 'y':
            update_task_file(t, true);
            printf("Deleted task with id: %d\n", t.id);
            return 0;
        case 'n':
            printf("Cancelling...\n");
            return 0;
        default:
            printf("Invalid response, please only provide y or n [yes, no]\n\n");
            break;
        }
    } while (confirmation != 'y' && confirmation != 'n');
}

int main(void)
{
    int command;
    int c;

    system("mkdir data");
    system("type nul > data\\tasks.txt");

    while (true)
    {
        printf("\n1. View tasks\n2. Add task\n3. Edit task\n4. Delete task\n5. Quit\n\n>> ");
        scanf("%d%*c", &command);
        printf("\n");

        switch (command)
        {
        case 1:
            list_tasks();
            break;
        case 2:
            add_task();
            break;
        case 3:
            edit_task();
            break;
        case 4:
            delete_task();
            break;
        case 5:
            printf("Quitting...");
            return 0;
        default:
            printf("Invalid option\n\n");
            break;
        }
    }

    return 0;
}