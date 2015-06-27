#include <assert.h>
#include <fann.h>

struct fann_train_data *
fann_append_train_data(struct fann_train_data *dst, struct fann_train_data *src)
{
    size_t i, j;

    if (src == 0)
        return dst;

    if (dst == 0)
        return fann_duplicate_train_data(src);

    assert(dst->num_input == src->num_input);
    assert(dst->num_output == src->num_output);

    size_t num_data = dst->num_data + src->num_data;
    size_t num_input = src->num_input;
    size_t num_output = src->num_output;

    dst->input = realloc(dst->input, num_data * sizeof(fann_type *));

    if (dst->input == 0) {
        fann_error((struct fann_error *) dst->input, FANN_E_CANT_ALLOCATE_MEM);
        return 0;
    }

    dst->output = realloc(dst->output, num_data * sizeof(fann_type *));

    if (dst->output == 0) {
        fann_error((struct fann_error *) dst->output, FANN_E_CANT_ALLOCATE_MEM);
        return 0;
    }

    for (i = 0; i < src->num_data; i++) {
        dst->input[i + dst->num_data] = malloc(num_input * sizeof(fann_type));

        for (j = 0; j < src->num_input; j++)
            dst->input[i + dst->num_data][j] = src->input[i][j];

        dst->output[i + dst->num_data] = malloc(num_output * sizeof(fann_type));

        for (j = 0; j < src->num_output; j++)
            dst->output[i + dst->num_data][j] = src->output[i][j];
    }

    dst->num_data = num_data;
    dst->num_input = num_input;
    dst->num_output = num_output;

#if 0
    fprintf(stderr, "%u %u %u\n",
            dst->num_data, dst->num_input, dst->num_output);
#endif

    return dst;
}
