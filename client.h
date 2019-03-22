fm *files_to_delete;
fm *files_to_update;
fm *dirs_to_create;

int ftu_count;
int ftd_count;
int dtc_count;

char *formatdate(time_t val)
{
    char *str;

    if ((str = (char *)malloc(48)) == NULL)
        displayError("malloc() error.");

    strftime(str, 48, "%Y%m%d%H%M.%S", localtime(&val));
    return str;
}

int adjustTimestamp(char *filepath, char *timestamp)
{
    char cmd[PATH_MAX + 32];
    int status, exitcode;

    snprintf(cmd, sizeof(cmd), "touch -a -m -t %s \"%s\"", timestamp, filepath);

    status = system(cmd);
    exitcode = WEXITSTATUS(status);

    return exitcode;
}

int getIndexFromServerFiles(fm *server_files, int count, char *path)
{
    int i;

    for (i = 0; i < count; i++)
    {
        if (!strcmp(server_files[i].path, path))
            return i;
    }

    return -1;
}

int removedir(char *dirpath)
{
    char cmd[PATH_MAX + 64];
    int status, exitcode;

    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", dirpath);

    status = system(cmd);
    exitcode = WEXITSTATUS(status);

    return exitcode;
}

int removefile(char *path)
{
    char cmd[PATH_MAX + 64];
    int status, exitcode;

    snprintf(cmd, sizeof(cmd), "rm \"%s\"", path);

    status = system(cmd);
    exitcode = WEXITSTATUS(status);

    return exitcode;
}

void getFilesToDelete(fm *server_files, int sf_count)
{
    int i, j;
    files_to_delete = (fm *)malloc(sizeof(fm));

    for (i = 0; i < paths_count; i++)
    {
        int toDelete = 1;

        for (j = 0; j < sf_count; j++)
        {
            if (!strcmp(own_files[i].path, server_files[j].path))
            {
                toDelete = 0;
                break;
            }
        }

        if (toDelete)
        {
            files_to_delete = (fm *)realloc(files_to_delete, sizeof(fm) * (++ftd_count));
            files_to_delete[ftd_count - 1] = own_files[i];
        }
    }
}

void getFilesToUpdate(fm *server_files, int sf_count)
{
    int i, j;
    files_to_update = (fm *)malloc(sizeof(fm));

    for (i = 0; i < sf_count; i++)
    {
        int different = 0;
        int found = 0;

        for (j = 0; j < paths_count; j++)
        {
            if (!strcmp(server_files[i].path, own_files[j].path))
            {
                found = 1;
                if (server_files[i].size != own_files[j].size || server_files[i].timestamp != own_files[j].timestamp || server_files[i].is_regular_file != own_files[j].is_regular_file)
                {
                    different = 1;
                    break;
                }
            }
        }

        if ((different || !found) && server_files[i].is_regular_file)
        {
            files_to_update = (fm *)realloc(files_to_update, sizeof(fm) * (++ftu_count));
            files_to_update[ftu_count - 1] = server_files[i];
        }
    }
}

void getDirectoriesToCreate(fm *server_files, int sf_count, char *root)
{
    dtc_count = 0;
    int i;
    dirs_to_create = (fm *)malloc(sizeof(fm));
    for (i = 0; i < sf_count; i++)
    {
        char newpath[PATH_MAX];
        snprintf(newpath, PATH_MAX, "%s/%s", root, server_files[i].path);
        if (!server_files[i].is_regular_file && isEmptyDirectory(newpath))
        {
            dirs_to_create = (fm *)realloc(dirs_to_create, sizeof(fm) * (++dtc_count));
            dirs_to_create[dtc_count - 1] = server_files[i];
        }
    }
}

void constructPath(char *source)
{
    char *path = (char *)malloc(strlen(source) + 1);
    strcpy(path, source);
    int length = strlen(path);
    int initial_length = length;
    while (1)
    {
        if (access(path, F_OK) == 0) //if file exists
            break;
        while (path[--length] != '/')
            ;
        path[length] = '\0';
    }
    if (length != initial_length)
        length += 1;
    while (length != initial_length)
    {
        strcpy(path, source);
        while ((++length) != initial_length && path[length] != '/')
            ;

        if (length != initial_length)
            path[length] = '\0';
        else
            break;

        printf("\nPATH: %s\n", path);
        if (mkdir(path, ACCESSPERMS) < 0)
            displayError("Error creating path");
    }
    if (path)
        free(path);
}