fm *ftd;
int fdc = 0;

void getFilesToDelete(fm *sf, int sfc)
{
    //vrem sa le returnam doar pe cele care se gasesc pe client si nu se gasesc pe server
    //mijlocul de comparatie este campul "path" din structura file_metadata

    ftd = (fm *)malloc(sizeof(fm));
    int i, j;
    for (i = 0; i < paths_count; i++)
    {
        int toDelete = 1;
        for (j = 0; j < sfc; j++)
        {
            if (strcmp(own_files[i].path, sf[j].path) == 0)
            {
                toDelete = 0;
                break;
            }
        }
        if (toDelete)
        {
            ftd = (fm *)realloc(ftd, sizeof(fm) * (++fdc));
            ftd[fdc - 1] = own_files[i];
        }
    }
}

fm *ftu;
int ftuc = 0;

void getFilesToUpdate(fm *sf, int sfc)
{
    //vrem sa returnam file_metadata[] de pe client care difera de cele de pe server
    //la cel putin una din urmatoarele categorii
    //fm->size SAU fm->timestamp SAU fisierul e pe server si NU e pe client

    ftu = (fm *)malloc(sizeof(fm));
    int i, j;
    for (i = 0; i < sfc; i++)
    {
        int different = 0;
        int found = 0;
        for (j = 0; j < ftuc; j++)
        {
            if (!strcmp(sf[i].path, ftu[j].path))
            {
                if (sf[i].size != ftu[j].size || sf[i].timestamp != ftu[j].timestamp || sf[i].is_regular_file != ftu[j].is_regular_file)
                {
                    different = 1;
                    break;
                }
                found = 1;
            }
        }
        if (different || !found)
        {
            ftu = (fm *)realloc(ftu, sizeof(fm) * (++ftuc));
            ftu[ftuc-1] = sf[i];
        } 
    }
}