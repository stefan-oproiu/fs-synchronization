void read_params(char *path)
{
    FILE *f = fopen(path, "r");
    ip = (char *)malloc(20);

    char *s_port = (char *)malloc(20);
    fscanf(f, "ip=%s\nport=%s", ip, s_port);

    if (f)
        fclose(f);

    port = atoi(s_port);
    printf("%s %d\n", ip, port);
}