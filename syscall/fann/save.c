#include <assert.h>
#include <error.h>
#include <libgen.h>
#include <errno.h>
#include <fann.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

int fann_save_safe(char const *path, struct fann const *net)
{
    char *str, temp[PATH_MAX];
    int fd;

    if ((str = strndup(path, PATH_MAX)) == 0) {
        assert(0);
        return -EINVAL;
    }

    snprintf(temp, PATH_MAX, ".%s.XXXXXX", basename(str));

    free(str);

#if 0
    error(1, 0, "name = %s", temp);
#endif

    if (strlen(temp) > PATH_MAX - 2) {
        assert(0);
        return -ENAMETOOLONG;
    }

    if ((fd = mkstemp(temp)) < 0) {
        assert(0);
        return -errno;
    }

    if (close(fd)) {
        assert(0);
        return -errno;
    }

    if (fann_save((struct fann *) net, temp)) {
        assert(0);
        return -EIO;
    }

#if 0
    error(0, errno, "renaming `%s' to `%s' ...", temp, path);
#endif

    if (rename(temp, path)) {
        assert(0);
        return -errno;
    }

    assert(errno == 0);

    return 0;
}

struct fann_train_data *
fann_append_train_data(struct fann_train_data *, struct fann_train_data *);

int fann_save_train_safe(char const *path, struct fann_train_data *data)
{
    struct fann_train_data *S;

    if (access(path, R_OK) == 0) {
        if ((S = fann_read_train_from_file(path)) == 0)
            return -1;

        if (S->num_input == data->num_input) {
            fann_append_train_data(data, S);
        }

        fann_destroy_train(S);
    }

    fann_shuffle_train_data(data);

    if (fann_save_train(data, path))
        return -1;

    return 0;
}
