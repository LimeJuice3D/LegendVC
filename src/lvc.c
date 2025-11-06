#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "../third-party/cJSON/cJSON.h"

#define LVC_DIR ".lvc"
#define LVC_DIR_LEN 4
#define BRANCH_DIR ".lvc/branch"
#define BRANCH_DIR_LEN 11
#define SCAFFOLD_FILE ".lvc/scaffold.json"

/*
 * TODO: 
 * branch
 * commit - there's no add in LVC, files to-be-committed are defined at the commit
 * file/commit info commands - log, status, whatever
 * .lvctrack - this is used to manage a record of all files which should automatically added to a commit
 * .lvcignore - This contains files/directories for LVC to ignore. This is processed AFTER .lvctrack
 * .lvcconstruct - This is used to track which files are "constructed"; these are files that are generated from a project, such as asset files. This is processed after .lvctrack but before .lvcignore. 
  * On the clientside: This allows the LVC monitor to automatically commit these files to the /construct directory upon noticing a file change; this allows previous versions of the export to be stored without needing to worry about them in VC while the most recent file is accessible to the user. 
  * On the serverside: when pulling from a server it allows the server to only send the most recent version of the asset without the backlog unless these files are requested. This means that the client saves on space and can generate these "constructed" files using the project files if need be, or request them specifically from the server.
  * Other notes: 
    * The actual implementation of this is an advanced feature; right now I just want to make the structure of it.
 * LVC monitor: a perpetually running process that checks for file changes and commits them automatically. Used for the construct functionality.
  * LVC monitor should also automatically sort stored files based on how regularly
  * they're called upon; regularly accessed files should be separated and
  * uncompressed, occasionally accessed files should be compressed but
  * reasonably segmented, and rarely accessed files should be fully compressed. The LVC monitor regulates these files automatically and transfers them in the background.
  * Alternatively, it could do this on every commit.
 * File compression for VC files.
 *
 * Other notes: 
  * divide architecture into data dumps (which contain the actual files) and
  * another section with pointers to those files that also contain all the
  * metadata
*/

void print_help();
void parse_args(int argc, char **argv);
void init(char *name);
void branch_add(char *branch_name);
void branch_remove(char *branch_name);
void write_scaffold(cJSON *json);
cJSON *read_scaffold();

void branch_remove(char *branch_name) {
    cJSON *json = read_scaffold();

    cJSON *branches = cJSON_GetObjectItemCaseSensitive(json, "branches");

    cJSON *branch = NULL;

    // Search branches array to find branch with same name
    int counter = 0;
    cJSON_ArrayForEach(branch, branches) {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(branch, "name");

        if (!strcmp(name->valuestring, branch_name)) {
            cJSON_DeleteItemFromArray(branches, counter);
            break;
        }

        counter++;
    }

    write_scaffold(json);

    return;
}

cJSON *read_scaffold() {
    struct stat st;

    if (stat(SCAFFOLD_FILE, &st) != 0) {
        perror("Cannot access scaffold.json");
        return NULL;
    }

    FILE *fp = fopen(SCAFFOLD_FILE, "r");

    if (fp == NULL) {
        perror("Cannot access scaffold.json");
        return NULL;
    }

    // Parse file into JSON structure
    char *buffer = malloc(sizeof(char) * (st.st_size + 1));
    fread(buffer, sizeof(char), st.st_size, fp);
    buffer[st.st_size] = '\0';
    fclose(fp);

    cJSON *json = cJSON_Parse(buffer);
    free(buffer);

    if (json == NULL) {
        const char *err = cJSON_GetErrorPtr();
        fprintf(stderr, "Cannot parse scaffold.json: %s\n", err);

        return NULL;
    }

    return json;
}

void write_scaffold(cJSON *json) {
    // print JSON contents to file
    char *json_str = cJSON_Print(json);
    FILE *fp = fopen(SCAFFOLD_FILE, "w");

    if (fp == NULL) {
        printf("Error: Unable to open the file.\n");
    } else {
        fputs(json_str, fp);
    }

    // free memory
    fclose(fp);
    cJSON_free(json_str);
    cJSON_Delete(json);

    return;
}

/*
 * Adds a new branch to the repo.
 */
void branch_add(char *branch_name) {
    cJSON *json = read_scaffold();

    if (json == NULL) {
        return;
    }

    // Branch modifications to add branch
    cJSON *branches = cJSON_GetObjectItemCaseSensitive(json, "branches");

    // check for existing branches with same name
    cJSON *branch = NULL;
    cJSON_ArrayForEach(branch, branches) {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(branch, "name");

        if (!strcmp(name->valuestring, branch_name)) {
            fprintf(stderr, "Branch with same name already exists\n");
            cJSON_Delete(json);
            return;
        }
    }

    cJSON *new_branch = cJSON_CreateObject();

    cJSON_AddStringToObject(new_branch, "name", branch_name);
    cJSON_AddItemToArray(branches, new_branch);

    write_scaffold(json);

    return;
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    return EXIT_SUCCESS;
}

/* 
 * Prints out a helpful help message. It's very helpful.
 */
void print_help() {
    printf(
        "usage: lvc [-h | <command>]\n\n"
        "Commands:\n"
        "init - Initialize version control - Usage: lvc init [name]\n"
    );

    return;
}

/*
 * parses the various commands and checks argument validity
 */
void parse_args(int argc, char **argv) {

    // print help, deliberately
    if (argc < 2 || !strcmp(argv[1], "-h")) {
        print_help();
        return;
    }

    // Initialize
    if (!strcmp(argv[1], "init")) {
        if (argc < 3) {
            init("lvc");
        } else {
            init(argv[2]);
        }

        return;
    }

    // Every command below here must check for a local .lvc directory
    DIR *lvc_dir = opendir(".lvc");

    if (lvc_dir == NULL) {
        fprintf(stderr, "No .lvc directory detected. Use lvc init\n");
        closedir(lvc_dir);

        return;
    }

    closedir(lvc_dir);

    // Branch
    if (!strcmp(argv[1], "branch")) {

        if (argc < 4) {
            fprintf(stderr, "Invalid arguments. Correct usage: lvc branch <add|remove> 'name'\n");
        } else if (!strcmp(argv[2], "add")) {
            branch_add(argv[3]);
        } else if (!strcmp(argv[2], "remove")) {
            branch_remove(argv[3]);
        } else {
            fprintf(stderr, "Invalid arguments. Correct usage: lvc branch <add|remove> 'name'\n");
        }

        return;
    }

    // print help, again
    print_help();

    return;
}

/*
 * Initializes version control. Creates a .lvc file to store all versioned
 * files.
 */
void init(char *repo_name) {

    if (mkdir(LVC_DIR, 0755) == -1) {
        perror("Unable to create .lvc directory");
        return;
    }

    // build JSON object
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "repo_name", repo_name);
    cJSON_AddArrayToObject(json, "branches");

    write_scaffold(json);

    return;
}
