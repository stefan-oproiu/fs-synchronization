fm *files_to_delete;
fm *files_to_update;

int ftu_count;
int ftd_count;

void getFilesToDelete(fm *server_files, int sf_count)
{
    int i, j;
    files_to_delete = (fm *) malloc(sizeof(fm));
    
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
            files_to_delete = (fm *) realloc(files_to_delete, sizeof(fm) * (++ftd_count));
            files_to_delete[ftd_count - 1] = own_files[i];
        }
    }
}

void getFilesToUpdate(fm *server_files, int sf_count)
{
    int i, j;
    files_to_update = (fm *) malloc(sizeof(fm));

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

        if (different || !found)
        {
            files_to_update = (fm *) realloc(files_to_update, sizeof(fm) * (++ftu_count));
            files_to_update[ftu_count - 1] = server_files[i];
        } 
    }
}