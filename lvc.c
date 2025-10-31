#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void store(char *file);
void print_help();
void parse_args(int argc, char **argv);
void init(char *path);
void store(char *file);

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    return EXIT_SUCCESS;
}

void print_help() {
    printf(
        "usage: lvc [-h | <command>]\n\n"
        "Commands:\n"
        "init - Initialize version control\n"
        "store - Place a file into version control\n"
    );

    return;
}

void parse_args(int argc, char **argv) {

    // Calls help
    if (argc < 2) {
        return print_help();
    }

    // Calls help
    if (!strcmp(argv[1], "-h")) {
        return print_help();
    }

    // Initializes .lvc repo
    if (!strcmp(argv[1], "init")) {

        if (argc < 3) {
            init(NULL);
        } else {
            init(argv[2]);
        }

    // Places 
    } else if (!strcmp(argv[1], "store")) {

        if (argc < 3) {
            printf("Invalid args, use 'lvc store <filename>'\n");
        }

        store(argv[2]);
    }

    return;
}

/*
 * Initializes version control. Creates a .lvc file to store all versioned
 * files.
 */
void init(char *path) {

    if (path == NULL) {
        int status = mkdir("./.lvc", 0600);

        if (status == -1) {
            perror("Unable to create .lvc directory");
            return;
        }

        return;
    }

    if (path[strlen(path) - 1] == '/') {
        path[strlen(path) - 1] = '\0';
    }

    char *dir_path = malloc(sizeof(char) * (strlen(path) + 1 + 5));

    strcpy(dir_path, path);
    strcat(dir_path, "/.lvc");

    int status = mkdir(dir_path, 0600);

    if (status == -1) {
        perror("Unable to create .lvc directory");
        free(dir_path);
        return;
    }

    free(dir_path);

    return;
}

/*
 * Places a file into VC storage. Largely for testing purposes.
 */
void store(char *file) {
    

    return;
}
