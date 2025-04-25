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

#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define RESET "\x1B[0m"

#define ADD_LESS(cv) (cvec_append_array(cv, "< ", 2))
#define ADD_MORE(cv) (cvec_append_array(cv, "> ", 2))

char *abs_path;
char *cur_path;

char *jplc_args[] = { "./", "-r", "", NULL };
char *diff_args[] = { "/bin/diff", "", "", NULL };
extern char** environ;

char buffer1[1024];
char buffer2[1024];
char buffer3[1024];
char *grader_name = "grader";
char *folder_name;
char *dir_name;
char *file_name;

int pass_count;
int fail_count;

CVec *your_output;
CVec *rt_output;
CVec *diff_output;

int dir_written = 0;

char *success = "Compilation succeeded";
char *fail = "Compilation failed";

int compare(const void*, const void*);
int file_filter(const char*);
int dir_filter(const char*);
int Chdir(const char *);
pid_t Fork();
int run_comparison(char*);
int compare_files(char*, char*);
FILE *Fopen(const char*, const char*);
int Pipe(int[2]);
int Creat(const char*, mode_t);
DIR *Opendir(const char*);
char *open_file(char*);
size_t string_line(char*);

int main(int argc, char **argv) {
    if (argc != 3) return 1;
    strcpy(buffer1, jplc_args[0]);
    strcat(buffer1, argv[1]);
    jplc_args[0] = malloc(strlen(buffer1)+1); if (!jplc_args[0]) return 1;
    strcpy(jplc_args[0], buffer1);
    folder_name = argv[2];
    folder_name = argv[2];

    remove("diff.txt");

    struct stat sb;
    Chdir("..");
    if (stat(jplc_args[0], &sb) == -1) {
        fprintf(stderr, "Exe file '%s' not found: %s\n", jplc_args[0], strerror(errno));
        return 1;
    }
    abs_path = getcwd(NULL, 0);
    if (!abs_path) {
        fprintf(stderr, "Failed to get directory path: %s\n", strerror(errno));
        return 1;
    }

    Vector *directories = vector_create();
    if (!directories) return 1;

    your_output = cvec_create();
    rt_output = cvec_create();
    diff_output = cvec_create();
    if (!your_output || !rt_output || !diff_output) {
        fprintf(stderr, "malloc failure: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    Chdir("grader");

    cvec_append_array_line(diff_output, folder_name, strlen(folder_name));
    
    DIR *hw = Opendir(folder_name);
    Chdir(folder_name);

    struct dirent *sub;
    while ((sub = readdir(hw)) != NULL) {
        if (!dir_filter(sub->d_name)) continue;

        vector_append(directories, (void*) sub);
    }

    qsort(directories->array, directories->size, sizeof(void*), compare);

    DIR *sub_dir;
    struct dirent *f_struct;
    Vector *files;
    for (size_t i = 0; i < directories->size; ++i) {
        sub = vector_get(directories, i);
        dir_name = sub->d_name;
        dir_written = 0;
        printf("\n%s", dir_name);

        cvec_append(diff_output, '\n');
        cvec_append_array_line(diff_output, dir_name, strlen(dir_name));

        sub_dir = Opendir(dir_name);
        Chdir(dir_name);

        files = vector_create();
        while ((f_struct = readdir(sub_dir)) != NULL) {
            if (!file_filter(f_struct->d_name)) continue;
            vector_append(files, (void*) f_struct);
        }

        qsort(files->array, files->size, sizeof(void*), compare);

        size_t print_count = 0;
        pass_count = 0;
        fail_count = 0;
        for (size_t j = 0; j < files->size; ++j) {
            f_struct = vector_get(files, j);
            file_name = f_struct->d_name;

            if (print_count % 6 == 0)
                printf("\n");

            printf("%s", file_name);
            cvec_append(diff_output, '\n');
            cvec_append_array_line(diff_output, file_name, strlen(file_name));

            if (snprintf(buffer1, 1024, "%s/%s/%s/%s", grader_name, folder_name, dir_name, file_name) < 0) {
                fprintf(stderr, "Failed to write string to buffer: %s\n", strerror(errno));
                return 1;
            }
            if (snprintf(buffer2, 1024, "%s/%s/%s/%s/%s.expected", "rtgrader", "expected", folder_name, dir_name, file_name) < 0) {
                fprintf(stderr, "Failed to write string to buffer: %s\n", strerror(errno));
                return 1;
            }

            if (!run_comparison(buffer1)) {
                printf(":" RED " failed\t\t" RESET);
                ++fail_count;
            }
            else {
                printf(":" GRN " passed\t\t" RESET);
                ++pass_count;
            }
            ++print_count;
        }
        printf("\nPassed: %d/%d = %5.2f%%\n", pass_count, (pass_count+fail_count), ((float) 100. * pass_count) / ((float) pass_count + fail_count));

        vector_destroy(files);
        closedir(sub_dir);
        Chdir(folder_name);
    }

    Chdir(abs_path);
    cvec_printf(diff_output, Fopen("rtgrader/diff.txt", "w"));

    closedir(hw);
    return 0;
}


int compare(const void *a, const void *b) {
    struct dirent *a_struct = *(struct dirent**) a;
    struct dirent *b_struct = *(struct dirent**) b;
    
    char *a_name = a_struct->d_name;
    char *b_name = b_struct->d_name;

    return strcmp(a_name, b_name);
}

int file_filter(const char *name) {
    if (!strcmp(".", name) || !strcmp("..", name)) return 0;
    
    size_t len = strlen(name);
    size_t i;
    const char *ptr = name;

    for (i = 0; i < len; ++i) {
        if (ptr[i] == '.')
            if (!strcmp(".expected", ptr+i)) return 0;
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

int run_comparison(char *file_name) {
    pid_t pid;
    int status;

    Chdir(abs_path);
    jplc_args[2] = file_name;

    if ((pid = Fork()) == 0) {
        int fd = Creat("rtgrader/_temp1", 0660);
        int fd2 = open("/dev/null", 0200);
        // stdout->file
        dup2(fd, fileno(stdout));
        // stderr->null
        dup2(fd2, fileno(stderr));

        if (execve(jplc_args[0], jplc_args, environ) < 0) {
            fprintf(stderr, "Execve error '%s': %s\n", jplc_args[0], strerror(errno));
            exit(0);
        }
    }

    if (waitpid(pid, &status, 0) < 0) {
        fprintf(stderr, "piderror: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return compare_files("rtgrader/_temp1", buffer2);
}

int compare_files(char *filename1, char *filename2) {
    if (!filename1 || !filename2) return 0;
    int return_status = 1;

    char *file_string1 = open_file(filename1);
    char *file_string2 = open_file(filename2);
    if (!file_string1 || !file_string2) {
        fprintf(stderr, "Failed to open temporary file string\n");
        exit(EXIT_FAILURE);
    }
    size_t len1;
    size_t len2;

    size_t i1 = 0;
    size_t i2 = 0;
    while (1) {
        while (file_string1[i1] == '\n' || file_string1[i1] == '\t') ++i1;
        while (file_string2[i2] == '\n' || file_string2[i2] == '\t') ++i2;

        if (file_string1[i1] == '\0') {
            if (file_string2[i2] == '\0') break;

            return_status = 0;
            while (file_string2[i2] != '\0') {
                if (file_string2[i2] == '\n') {
                    ++i2;
                    continue;
                }

                len2 = string_line(file_string2 + i2);
                ADD_MORE(rt_output);
                cvec_append_array_line(rt_output, file_string2+i2, len2);
                i2 += len2;
            }

            break;
        }
        else if (file_string2[i2] == '\0') {
            if (file_string1[i1] == '\0') break;

            return_status = 0;
            while (file_string1[i1] != '\0') {
                if (file_string1[i1] == '\n') {
                    ++i1;
                    continue;
                }

                len1 = string_line(file_string1 + i1);
                ADD_MORE(rt_output);
                cvec_append_array_line(rt_output, file_string1+i1, len1);
                i1 += len1;
            }

            break;
        }

        len1 = strlen(success);
        if (!strncmp(file_string1+i1, success, len1) && !strncmp(file_string2+i2, success, len1))
            break;

        len1 = strlen(fail);
        if (!strncmp(file_string1+i1, fail, len1) && !strncmp(file_string2+i2, fail, len1))
            break;

        len1 = string_line(file_string1 + i1);
        len2 = string_line(file_string2 + i2);

        if (len1 != len2 || strncmp(file_string1+i1, file_string2+i2, len1)) {
            return_status = 0;
            ADD_LESS(your_output);
            cvec_append_array_line(your_output, file_string1+i1, len1);
            ADD_MORE(rt_output);
            cvec_append_array_line(rt_output, file_string2+i2, len2);
        }
        i1 += len1;
        i2 += len2;
    }

    if (!return_status) {
        cvec_append_array(diff_output, rt_output->array, rt_output->size);
        cvec_append_array_line(diff_output, "************************************************************", 60);
        cvec_append_array(diff_output, your_output->array, your_output->size);
    }

    free(file_string1);
    free(file_string2);

    cvec_clear(your_output);
    cvec_clear(rt_output);

    Chdir(grader_name);
    return return_status;
}

FILE *Fopen(const char *name, const char *mode) {
    FILE *ret = fopen(name, mode);
    if (!ret) {
        fprintf(stderr, "File error on '%s': %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return ret;
}

int Pipe(int fds[2]) {
    if (pipe(fds) == -1) {
        fprintf(stderr, "Pipe error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return 0;
}

int Creat(const char *name, mode_t flags) {
    int fd = creat(name, flags);
    if (fd < 0) {
        fprintf(stderr, "File creation error '%s': %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return fd;
}

DIR *Opendir(const char *name) {
    DIR *ret = opendir(name);
    if (!ret) {
        fprintf(stderr, "Failed to find directory %s: %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    return ret;
}

char *open_file(char *file_name) {
    if (!file_name) return NULL;
    size_t file_size;
    FILE *f_ptr = Fopen(file_name, "r");

    if (fseek(f_ptr, 0, SEEK_END) != 0) return NULL;
    file_size = ftell(f_ptr);
    if (fseek(f_ptr, 0, SEEK_SET) != 0) return NULL;

    char *file_string = malloc(file_size + 1); if (!file_string) return NULL;
    file_string[file_size] = '\0';

    if (fread(file_string, 1, file_size, f_ptr) != file_size)
        if (ferror(f_ptr)) {
            printf("Error occurred while reading file %s\n", file_name);
            return NULL;
        }

    fclose(f_ptr);
    return file_string;
}

size_t string_line(char *string) {
    if (!string) return 0;

    size_t i = 0;
    while (1) {
        if (string[i] == '\n' || string[i] == '\0') return i;
        ++i;
    }

    return 0;
}
