#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

#define DEBUG_MODE

typedef struct file_metadata
{
    char path[PATH_MAX];
    unsigned int size;
    unsigned long timestamp;
    short is_regular_file; // 1 if regular file; 0 if directory
} fm;

int paths_count = 0;
char paths[1024][PATH_MAX];

fm *own_files;

char *ip;
int port;

void getAllFilesMetadata(char *root);
void getAllFilesPaths(char *dirPath);
void read_params(char *path);
void displayError(char *msg);
int isEmptyDirectory(char *dirPath);
void printPaths();
void getFilesPaths(char *dirPath, int length);
int getPathIndex(char *path);

int getPathIndex(char *path)
{
    int i;

    for (i = 0; i < paths_count; i++)
    {
        if (!strcmp(own_files[i].path, path))
            return i;
    }

    return -1;
}

void read_params(char *path)
{
    FILE *f = fopen(path, "r");
    ip = (char *) malloc(20);

    char *s_port = (char *) malloc(20);
    fscanf(f, "ip=%s\nport=%s", ip, s_port);

    if (f)
        fclose(f);

    port = atoi(s_port);
    printf("[loaded] ip: %s, port: %d\n", ip, port);
}

void displayError(char *msg)
{
    fprintf(stderr, "%s:%s\n", msg, strerror(errno));
    exit(errno);
}

int isEmptyDirectory(char *dirPath)
{
    char cmd[1024];
    int status, exitcode;
    snprintf(cmd, 1024, "test $(ls -A \"%s\" 2>/dev/null | wc -l) -ne 0", dirPath);

    status = system(cmd);
    exitcode = WEXITSTATUS(status);

    return exitcode;
}

void printPaths()
{
	int i;

	for (i = 0; i < paths_count; i++)
		printf("%s\n", paths[i]);
}

void getFilesPaths(char *dirPath, int length)
{
    DIR *dir;
    struct dirent *in;
    char *name;
    struct stat info;
    char cale[PATH_MAX], cale_link[PATH_MAX + 1];
    int n;

    char errorMessage[PATH_MAX + 100];

    if (!(dir = opendir(dirPath)))
    {
        snprintf(errorMessage, sizeof(errorMessage), "Error while openning directory: %s\n", dirPath);
        displayError(errorMessage);
    }

    while ((in = readdir(dir)) > 0)
    {
        name = in->d_name;

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        snprintf(cale, sizeof(cale), "%s/%s", dirPath, name);

        if (lstat(cale, &info) < 0)
        {
            snprintf(errorMessage, sizeof(errorMessage), "Error at lstat: %s\n", cale);
            displayError(errorMessage);
        }

        strcpy(paths[paths_count++], cale + length);
        
        if (S_ISDIR(info.st_mode) && !isEmptyDirectory(cale)) {
            getFilesPaths(cale, length);
        }
    }

    if (closedir(dir))
    {
        snprintf(errorMessage, sizeof(errorMessage), "Error closing directory: %s\n", dirPath);
        displayError(errorMessage);
    }
}

void getAllFilesPaths(char *dirPath)
{
    getFilesPaths(dirPath, strlen(dirPath) + 1);
    strcpy(paths[paths_count], "");
}

void getAllFilesMetadata(char *root)
{
    own_files = (fm *) malloc(paths_count * sizeof(fm));
    //pentru fiecare path din paths, creaza un nou obiect file_metadata,
    //populeaza-l cu date apeland lstat SI adauga-l la own_files_metadata

    int i;
    struct stat s;

    for (i = 0; i < paths_count; i++)
    {
        char *fullpath = (char *) malloc(strlen(root) + strlen(paths[i]) + 10);

        snprintf(fullpath, PATH_MAX, "%s/%s", root, paths[i]);
        strcpy(own_files[i].path, paths[i]);
        
        stat(fullpath, &s);
        own_files[i].size = s.st_size;
        own_files[i].timestamp = s.st_mtime;
		own_files[i].is_regular_file = S_ISDIR(s.st_mode) ? 0 : 1;

        if (fullpath)
        	free(fullpath);
    }
}