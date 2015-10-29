// Write the header of a Pico-encoded file specified by the command line
// argument as a Python dictionary.
#include <pico.h>
int main(int argc, char * argv[]) {
    pico_debug = 1;
    if (argc < 2) return 0;
    FILE * file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "Unable to open file.\n");
        return 1;
    }
    PICO * pico = pico_open(file, NULL);
    if (pico == NULL) {
        fprintf(stderr, "Unable to decode file.\n");
        return 2;
    }
    pico_dump_header(pico, PYTHON_DICT, stdout);
    return 0;
}
