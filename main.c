#include "main.h"

// Returns the contents of a given file.
char *read_file(char *path)
{
    FILE *fp;
    if (strcmp(path, "-") == 0)
    {
        // By convention, read from stdin if a given filename is "-".
        fp = stdin;
    }
    else
    {
        fp = fopen(path, "r");
        if (!fp)
            error("cannot open %s: %s", path, strerror(errno));
    }

    int filemax = 10 * 1024 * 1024;
    char *buf = malloc(filemax);
    int size = fread(buf, 1, filemax - 2, fp);
    if (!feof(fp))
        error("%s: file too large");

    if (fp != stdin)
        fclose(fp);

    // Make sure that the string ends with "\n\0".
    if (size == 0 || buf[size - 1] != '\n')
        buf[size++] = '\n';
    buf[size] = '\0';
    return buf;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        error("%s: 引数の個数が正しくありません\n", argv[0]);

    filename = argv[1];
    user_input = read_file(argv[1]);
    token = tokenize();
    Program *prog = program();
    add_type(prog);

    codegen(prog);
    return 0;
}