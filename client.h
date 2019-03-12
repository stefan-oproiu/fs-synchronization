fm *files_to_delete;
int fdc = 0;

void getFilesToDelete(fm *server_files, int server_files_count)
{
    //vrem sa le returnam doar pe cele care se gasesc pe client si nu se gasesc pe server
    //mijlocul de comparatie este campul "path" din structura file_metadata

    files_to_delete = (fm *)malloc(sizeof(fm) * fdc);
    int i, j;
    for(i = 0; i < paths_count; i++)
    {
        int toDelete = 1;
        for(j = 0; j < server_files_count; j++)
        {
            if(strcmp(own_files[i].path, server_files[j].path) == 0)
            {
                toDelete = 0;
            }
        }
        if(toDelete)
        {
            files_to_delete = (fm *)realloc(files_to_delete, sizeof(fm) * (++fdc));
            files_to_delete[fdc-1] = own_files[i];
        }
    }
}

fm *files_to_update;
int fuc = 0;

void getFilesToUpdate(fm *server_files, int server_files_count)
{
    //vrem sa returnam file_metadata[] de pe client care difera de cele de pe server
    //la cel putin una din urmatoarele categorii
    //fm->size SAU fm->timestamp SAU fisierul e pe server si NU e pe client

    /*
        your code
    */
}