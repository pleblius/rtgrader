#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include "vector.h"
#include "cvec.h"
#include <stdlib.h>
#include <fcntl.h>

char *rt_grader_name = "rtgrader";
char *expected_path = "rtgrader/expected";
char *new_path;
char *cur_path;

char *abs_path;
char *grader_name = "grader";
char *folder_name;
char *dir_name;
char *file_name;

char buffer1[1024];
char buffer2[1024];

char *rt_args[] = { "./rt/", "-r", "", NULL };
extern char **environ;

Vector *directories;

int file_filter(const char*);
int dir_filter(const char*);
int Chdir(const char *);
pid_t Fork();
void run_compiler(char*, char*);
FILE *Fopen(const char*, const char*);
int Pipe(int[2]);
int Creat(const char*, mode_t);
DIR *Opendir(const char*);
int Mkdir(char*, mode_t);

int main(int argc, char **argv) {
    if (argc != 3) return 1;
    strcpy(buffer1, rt_args[0]);
    strcat(buffer1, argv[1]);
    rt_args[0] = malloc(strlen(buffer1)+1); if (!rt_args[0]) return 1;
    strcpy(rt_args[0], buffer1);
    folder_name = argv[2];
    
    Vector *directories = vector_create();
    if (!directories) return 1;

    struct stat sb;
    if (stat("expected", &sb) == -1)
        Mkdir("expected", 0777);

    Chdir("..");
    abs_path = getcwd(NULL, 0);
    if (!abs_path) {
        fprintf(stderr, "Failed to get directory path: %s\n", strerror(errno));
        return 1;
    }
    if (stat(rt_args[0], &sb) == -1) {
        fprintf(stderr, "Exe file '%s' not found: %s\n", rt_args[0], strerror(errno));
        return 1;
    }

    Chdir("grader");
    
    DIR *hw = Opendir(folder_name);
    Chdir(folder_name);

    cur_path = getcwd(NULL, 0);
    Chdir(abs_path);
    Chdir(expected_path);
    if (stat(folder_name, &sb) == -1)
        Mkdir(folder_name, 0777);
    Chdir(cur_path);
    free(cur_path);

    struct dirent *sub;
    while ((sub = readdir(hw)) != NULL) {
        if (!dir_filter(sub->d_name)) continue;

        vector_append(directories, (void*) sub);
    }

    DIR *sub_dir;
    struct dirent *f_struct;
    Vector *files;
    for (size_t i = 0; i < directories->size; ++i) {
        sub = vector_get(directories, i);
        dir_name = sub->d_name;
        printf("\n%s\n", dir_name);

        sub_dir = Opendir(dir_name);
        Chdir(dir_name);

        cur_path = getcwd(NULL, 0);
        Chdir(abs_path);
        Chdir(expected_path);
        Chdir(folder_name);
        if (stat(dir_name, &sb) == -1)
            Mkdir(dir_name, 0777);
        Chdir(cur_path);
        free(cur_path);

        files = vector_create();
        while ((f_struct = readdir(sub_dir)) != NULL) {
            if (!file_filter(f_struct->d_name)) continue;
            vector_append(files, (void*) f_struct);
        }

        size_t print_count = 0;
        for (size_t j = 0; j < files->size; ++j) {
            f_struct = vector_get(files, j);
            file_name = f_struct->d_name;

            if (print_count++ % 4 == 0)
                printf("\n");
            printf("%s\t", file_name);


            if (snprintf(buffer1, 1024, "%s/%s/%s/%s", grader_name, folder_name, dir_name, file_name) < 0) {
                fprintf(stderr, "Failed to write string to buffer: %s\n", strerror(errno));
                return 1;
            }
            if (snprintf(buffer2, 1024, "%s/%s/%s/%s.expected", expected_path, folder_name, dir_name, file_name) < 0) {
                fprintf(stderr, "Failed to write strign to buffer: %s\n", strerror(errno));
                return 1;
            }

            run_compiler(buffer1, buffer2);
        }

        printf("\n");

        vector_destroy(files);
        closedir(sub_dir);
        Chdir(grader_name);
        Chdir(folder_name);
    }

    vector_destroy(directories);
    closedir(hw);
    return 0;
}

int file_filter(const char *name) {
    if (!strcmp(".", name) || !strcmp("..", name)) return 0;
    
    size_t len = strlen(name);
    size_t i;
    const char *ptr = name;

    for (i = 0; i < len; ++i) {
        if (ptr[i] == '.')
            if (strcmp(".jpl", ptr+i)) return 0;
    }

    return 1;
}

int dir_filter(const char *name) {
    if (!strcmp(".", name) || !strcmp("..", name)) return 0;
    
    size_t len = strlen(name);
    size_t i;
    const char *ptr = name;

    for (i = 0; i < len; ++i) {
        if (ptr[i] == '.')
            return 0;
    }

    return 1;
}

int Chdir(const char *name) {
    if (chdir(name) != 0) {
        fprintf(stderr, "chdir error on %s: %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return 0;
}

pid_t Fork() {
    pid_t ret = fork();
    if (ret < 0) {
        fprintf(stderr, "Fork failure: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return ret;
}

void run_compiler(char *file_name, char *output_name) {
    pid_t pid;
    int status;

    Chdir(abs_path);

    rt_args[2] = file_name;

    if ((pid = Fork()) == 0) {
        int fd = Creat(output_name, 0660);
        int fd2 = open("/dev/null", 0200);

        dup2(fd, fileno(stdout));
        dup2(fd2, fileno(stderr));
        if (execve(rt_args[0], rt_args, environ) < 0) {
            fprintf(stderr, "execve error '%s': %s\n", rt_args[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (waitpid(pid, &status, 0) < 0) {
        fprintf(stderr, "piderror: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

DIR *Opendir(const char *name) {
    DIR *ret = opendir(name);
    if (!ret) {
        fprintf(stderr, "Failed to find directory %s: %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return ret;
}

int Creat(const char *name, mode_t flags) {
    int fd = creat(name, flags);
    if (fd < 0) {
        fprintf(stderr, "File creation error '%s': %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return fd;
}

int Mkdir(char *name, mode_t mode) {
    int ret = mkdir(name, mode);
    if (ret < 0) {
        fprintf(stderr, "Failed to create directory '%s': %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return ret;
}
