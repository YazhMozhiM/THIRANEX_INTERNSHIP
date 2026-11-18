
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ADD_BUTTON 1
#define DELETE_BUTTON 2
#define COMPLETE_BUTTON 3
#define EDIT_BUTTON 4
#define UPDATE_BUTTON 5
#define CLEAR_BUTTON 6
#define SEARCH_BUTTON 7
#define TASK_INPUT 8
#define TASK_LIST 9
#define PRIORITY_INPUT 10
#define DATE_INPUT 11
#define SEARCH_INPUT 12
#define CLEAR_SEARCH 13

#define MAX_TASKS 500

typedef struct
{
    char name[200];
    char priority[15];
    char date[20];
    int completed;
} Task;

Task tasks[MAX_TASKS];
int taskCount = 0;
int editingIndex = -1;

HWND input, list, priorityBox, dateInput;
HWND searchInput, totalLabel, pendingLabel, completedLabel;

HBRUSH backgroundBrush;

void saveTasks()
{
    FILE *fp = fopen("tasks.txt", "w");

    if (fp == NULL)
        return;

    for (int i = 0; i < taskCount; i++)
    {
        fprintf(fp, "%d\t%s\t%s\t%s\n",
                tasks[i].completed,
                tasks[i].priority,
                tasks[i].date,
                tasks[i].name);
    }

    fclose(fp);
}

void loadTasks()
{
    FILE *fp = fopen("tasks.txt", "r");

    if (fp == NULL)
        return;

    char line[300];

    while (fgets(line, sizeof(line), fp) && taskCount < MAX_TASKS)
    {
        Task t = {0};

        if (sscanf(line, "%d\t%14[^\t]\t%19[^\t]\t%199[^\n]",
                   &t.completed, t.priority, t.date, t.name) == 4)
        {
            tasks[taskCount++] = t;
        }
    }

    fclose(fp);
}

void updateList()
{
    SendMessageA(list, LB_RESETCONTENT, 0, 0);

    char display[300];

    for (int i = 0; i < taskCount; i++)
    {
        sprintf(display, "%s [%s] %s (Due: %s)",
                tasks[i].completed ? "[DONE]" : "[PENDING]",
                tasks[i].priority,
                tasks[i].name,
                tasks[i].date[0] ? tasks[i].date : "Not set");

        SendMessageA(list, LB_ADDSTRING, 0, (LPARAM)display);
    }

    saveTasks();
}

void updateStats()
{
    int pending = 0;
    int completed = 0;

    for (int i = 0; i < taskCount; i++)
    {
        if (tasks[i].completed)
            completed++;
        else
            pending++;
    }

    char text[100];

    sprintf(text, "Total Tasks: %d", taskCount);
    SetWindowTextA(totalLabel, text);

    sprintf(text, "Pending Tasks: %d", pending);
    SetWindowTextA(pendingLabel, text);

    sprintf(text, "Completed Tasks: %d", completed);
    SetWindowTextA(completedLabel, text);
}

int getSelectedTask()
{
    return (int)SendMessageA(list, LB_GETCURSEL, 0, 0);
}

void clearInputs()
{
    SetWindowTextA(input, "");
    SetWindowTextA(dateInput, "");
    SendMessageA(priorityBox, CB_SETCURSEL, 1, 0);
    editingIndex = -1;
}

void addTask()
{
    if (taskCount >= MAX_TASKS)
    {
        MessageBoxA(NULL, "Task limit reached!",
                    "Warning", MB_OK | MB_ICONWARNING);
        return;
    }

    char name[200];
    char date[20];
    char priority[15];

    GetWindowTextA(input, name, sizeof(name));
    GetWindowTextA(dateInput, date, sizeof(date));

    if (strlen(name) == 0)
    {
        MessageBoxA(NULL, "Please enter a task!",
                    "Warning", MB_OK | MB_ICONWARNING);
        return;
    }

    if (strchr(name, '\t') || strchr(name, '\n') ||
        strchr(date, '\t') || strchr(date, '\n'))
    {
        MessageBoxA(NULL, "Tabs and new lines are not allowed!",
                    "Invalid Input", MB_OK | MB_ICONWARNING);
        return;
    }

    int selected = (int)SendMessageA(
        priorityBox, CB_GETCURSEL, 0, 0);

    const char *priorities[] = {"High", "Medium", "Low"};

    strcpy(priority, priorities[selected == CB_ERR ? 1 : selected]);

    strcpy(tasks[taskCount].name, name);
    strcpy(tasks[taskCount].date, date);
    strcpy(tasks[taskCount].priority, priority);

    tasks[taskCount].completed = 0;

    taskCount++;

    updateList();
    updateStats();
    clearInputs();
}

void editTask()
{
    int selected = getSelectedTask();

    if (selected == LB_ERR)
        return;

    editingIndex = selected;

    SetWindowTextA(input, tasks[selected].name);
    SetWindowTextA(dateInput, tasks[selected].date);

    int priority = 1;

    if (strcmp(tasks[selected].priority, "High") == 0)
        priority = 0;
    else if (strcmp(tasks[selected].priority, "Low") == 0)
        priority = 2;

    SendMessageA(priorityBox, CB_SETCURSEL, priority, 0);
}

void updateTask()
{
    if (editingIndex < 0 || editingIndex >= taskCount)
    {
        MessageBoxA(NULL, "Load a task using Edit Task first!",
                    "Warning", MB_OK | MB_ICONWARNING);
        return;
    }

    char name[200];
    char date[20];

    GetWindowTextA(input, name, sizeof(name));
    GetWindowTextA(dateInput, date, sizeof(date));

    if (strlen(name) == 0)
    {
        MessageBoxA(NULL, "Task name cannot be empty!",
                    "Warning", MB_OK | MB_ICONWARNING);
        return;
    }

    if (strchr(name, '\t') || strchr(name, '\n') ||
        strchr(date, '\t') || strchr(date, '\n'))
    {
        MessageBoxA(NULL, "Tabs and new lines are not allowed!",
                    "Invalid Input", MB_OK | MB_ICONWARNING);
        return;
    }

    int selected = (int)SendMessageA(
        priorityBox, CB_GETCURSEL, 0, 0);

    const char *priorities[] = {"High", "Medium", "Low"};

    strcpy(tasks[editingIndex].name, name);
    strcpy(tasks[editingIndex].date, date);

    strcpy(tasks[editingIndex].priority,
           priorities[selected == CB_ERR ? 1 : selected]);

    updateList();
    updateStats();
    clearInputs();
}

void deleteTask(HWND hwnd)
{
    int selected = getSelectedTask();

    if (selected == LB_ERR)
    {
        MessageBoxA(hwnd, "Select a task first!",
                    "Warning", MB_OK);
        return;
    }

    if (MessageBoxA(hwnd, "Are you sure you want to delete this task?",
                    "Confirm Delete",
                    MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        for (int i = selected; i < taskCount - 1; i++)
            tasks[i] = tasks[i + 1];

        taskCount--;

        if (editingIndex == selected)
            editingIndex = -1;
        else if (editingIndex > selected)
            editingIndex--;

        updateList();
        updateStats();
        clearInputs();
    }
}

void toggleComplete(HWND hwnd)
{
    int selected = getSelectedTask();

    if (selected == LB_ERR)
    {
        MessageBoxA(hwnd, "Select a task first!",
                    "Warning", MB_OK);
        return;
    }

    tasks[selected].completed = !tasks[selected].completed;

    updateList();
    updateStats();

    SendMessageA(list, LB_SETCURSEL, selected, 0);
}

void clearAllTasks(HWND hwnd)
{
    if (taskCount == 0)
        return;

    if (MessageBoxA(hwnd, "Delete ALL tasks permanently?",
                    "Confirm Clear All",
                    MB_YESNO | MB_ICONWARNING) == IDYES)
    {
        taskCount = 0;
        editingIndex = -1;

        updateList();
        updateStats();
        clearInputs();
    }
}

int containsIgnoreCase(const char *text, const char *search)
{
    int length = (int)strlen(search);

    if (length == 0)
        return 0;

    for (int i = 0; text[i]; i++)
    {
        if (_strnicmp(text + i, search, length) == 0)
            return 1;
    }

    return 0;
}

void searchTask(HWND hwnd)
{
    char search[200];

    GetWindowTextA(searchInput, search, sizeof(search));

    if (strlen(search) == 0)
    {
        MessageBoxA(hwnd, "Enter a task name to search!",
                    "Search", MB_OK);
        return;
    }

    SendMessageA(list, LB_SETCURSEL, -1, 0);

    for (int i = 0; i < taskCount; i++)
    {
        if (containsIgnoreCase(tasks[i].name, search))
        {
            SendMessageA(list, LB_SETCURSEL, i, 0);
            return;
        }
    }

    MessageBoxA(hwnd, "Task not found!",
                "Search Result", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg,
                                 WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            backgroundBrush = CreateSolidBrush(RGB(240, 244, 250));

            CreateWindowA("STATIC", "MY TO-DO LIST",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                170, 10, 300, 35,
                hwnd, NULL, NULL, NULL);

            CreateWindowA("STATIC", "Task Name:",
                WS_CHILD | WS_VISIBLE,
                30, 65, 100, 25,
                hwnd, NULL, NULL, NULL);

            input = CreateWindowA("EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_BORDER,
                130, 60, 300, 30,
                hwnd, (HMENU)TASK_INPUT, NULL, NULL);

            CreateWindowA("STATIC", "Priority:",
                WS_CHILD | WS_VISIBLE,
                30, 105, 100, 25,
                hwnd, NULL, NULL, NULL);

            priorityBox = CreateWindowA("COMBOBOX", "",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                130, 100, 140, 100,
                hwnd, (HMENU)PRIORITY_INPUT, NULL, NULL);

            SendMessageA(priorityBox, CB_ADDSTRING, 0, (LPARAM)"High");
            SendMessageA(priorityBox, CB_ADDSTRING, 0, (LPARAM)"Medium");
            SendMessageA(priorityBox, CB_ADDSTRING, 0, (LPARAM)"Low");
            SendMessageA(priorityBox, CB_SETCURSEL, 1, 0);

            CreateWindowA("STATIC", "Due Date:",
                WS_CHILD | WS_VISIBLE,
                300, 105, 100, 25,
                hwnd, NULL, NULL, NULL);

            dateInput = CreateWindowA("EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_BORDER,
                380, 100, 180, 30,
                hwnd, (HMENU)DATE_INPUT, NULL, NULL);

            CreateWindowA("BUTTON", "Add Task",
                WS_CHILD | WS_VISIBLE,
                30, 145, 110, 35,
                hwnd, (HMENU)ADD_BUTTON, NULL, NULL);

            CreateWindowA("BUTTON", "Edit Task",
                WS_CHILD | WS_VISIBLE,
                155, 145, 110, 35,
                hwnd, (HMENU)EDIT_BUTTON, NULL, NULL);

            CreateWindowA("BUTTON", "Update Task",
                WS_CHILD | WS_VISIBLE,
                280, 145, 120, 35,
                hwnd, (HMENU)UPDATE_BUTTON, NULL, NULL);

            CreateWindowA("STATIC", "Search:",
                WS_CHILD | WS_VISIBLE,
                30, 195, 80, 25,
                hwnd, NULL, NULL, NULL);

            searchInput = CreateWindowA("EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_BORDER,
                110, 190, 250, 30,
                hwnd, (HMENU)SEARCH_INPUT, NULL, NULL);

            CreateWindowA("BUTTON", "Search",
                WS_CHILD | WS_VISIBLE,
                375, 190, 90, 30,
                hwnd, (HMENU)SEARCH_BUTTON, NULL, NULL);

            CreateWindowA("BUTTON", "Clear Search",
                WS_CHILD | WS_VISIBLE,
                475, 190, 100, 30,
                hwnd, (HMENU)CLEAR_SEARCH, NULL, NULL);

            list = CreateWindowA("LISTBOX", "",
                WS_CHILD | WS_VISIBLE | WS_BORDER |
                WS_VSCROLL | LBS_NOTIFY,
                30, 240, 545, 170,
                hwnd, (HMENU)TASK_LIST, NULL, NULL);

            CreateWindowA("BUTTON", "Delete Task",
                WS_CHILD | WS_VISIBLE,
                30, 425, 115, 35,
                hwnd, (HMENU)DELETE_BUTTON, NULL, NULL);

            CreateWindowA("BUTTON", "Complete / Undo",
                WS_CHILD | WS_VISIBLE,
                160, 425, 145, 35,
                hwnd, (HMENU)COMPLETE_BUTTON, NULL, NULL);

            CreateWindowA("BUTTON", "Clear All",
                WS_CHILD | WS_VISIBLE,
                320, 425, 110, 35,
                hwnd, (HMENU)CLEAR_BUTTON, NULL, NULL);

            totalLabel = CreateWindowA("STATIC", "Total Tasks: 0",
                WS_CHILD | WS_VISIBLE,
                30, 485, 170, 25,
                hwnd, NULL, NULL, NULL);

            pendingLabel = CreateWindowA("STATIC", "Pending Tasks: 0",
                WS_CHILD | WS_VISIBLE,
                210, 485, 170, 25,
                hwnd, NULL, NULL, NULL);

            completedLabel = CreateWindowA("STATIC", "Completed Tasks: 0",
                WS_CHILD | WS_VISIBLE,
                390, 485, 180, 25,
                hwnd, NULL, NULL, NULL);

            loadTasks();
            updateList();
            updateStats();

            break;
        }

        case WM_COMMAND:
        {
            int id = LOWORD(wp);

            switch (id)
            {
                case ADD_BUTTON:
                    addTask();
                    break;

                case EDIT_BUTTON:
                    editTask();
                    break;

                case UPDATE_BUTTON:
                    updateTask();
                    break;

                case DELETE_BUTTON:
                    deleteTask(hwnd);
                    break;

                case COMPLETE_BUTTON:
                    toggleComplete(hwnd);
                    break;

                case CLEAR_BUTTON:
                    clearAllTasks(hwnd);
                    break;

                case SEARCH_BUTTON:
                    searchTask(hwnd);
                    break;

                case CLEAR_SEARCH:
                    SetWindowTextA(searchInput, "");
                    SendMessageA(list, LB_SETCURSEL, -1, 0);
                    break;
            }

            break;
        }

        case WM_CTLCOLORSTATIC:
        {
            HDC hdc = (HDC)wp;

            SetBkColor(hdc, RGB(240, 244, 250));
            SetTextColor(hdc, RGB(35, 50, 75));

            return (LRESULT)backgroundBrush;
        }

        case WM_ERASEBKGND:
        {
            RECT rect;
            GetClientRect(hwnd, &rect);

            FillRect((HDC)wp, &rect, backgroundBrush);
            return 1;
        }

        case WM_DESTROY:
            saveTasks();

            if (backgroundBrush)
                DeleteObject(backgroundBrush);

            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA(hwnd, msg, wp, lp);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev,
                   LPSTR args, int nCmdShow)
{
    WNDCLASSA wc = {0};

    wc.hInstance = hInst;
    wc.lpszClassName = "AdvancedTodoWindow";
    wc.lpfnWndProc = WindowProcedure;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassA(&wc))
        return 0;

    HWND hwnd = CreateWindowA(
        "AdvancedTodoWindow",
        "Advanced To-Do List Management System",
        WS_OVERLAPPEDWINDOW,
        200, 100, 630, 570,
        NULL, NULL, hInst, NULL);

    if (!hwnd)
        return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;

    while (GetMessageA(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}
