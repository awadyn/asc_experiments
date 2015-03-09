#include <asc.h>

struct map *parse(int pid, char const *file);

struct map *maps(int pid, char const *file)
{
    return parse(pid, file);
}
