// Write just the MD5 from the header of a set of Pico-encoded files.
#include <pico.h>
#include <string.h>
int main(int argc, char * argv[]) {
    pico_debug = 1;
    // Get longest filename.
    size_t max = 10;
    for (int index = 1; index < argc; ++index) {
        size_t len = strlen(argv[index]);
        if (len > max) max = len;
    } // Find longest filename.
    char format[20];
    sprintf(format, "%%%lds: ", max);
    for (int index = 1; index < argc; ++index) {
        fprintf(stdout, format, argv[index]);
        FILE * file = fopen(argv[index], "r");
        if (file == NULL) {
            fprintf(stdout, "Unable to open file.\n");
            continue;
        }
        PICO * pico = pico_open(file, NULL);
        if (pico == NULL) {
            fprintf(stdout, "Unable to decode file.\n");
            fclose(file);
            continue;
        }
        for (size_t byte = 0; byte < HASH_LEN; ++byte) {
            fprintf(stdout, "%02X", pico->hash[byte]);
        } // Print the hash.
        fprintf(stdout, "\n");
        pico_finish(pico);
        fclose(file);
    } // Write MD5's for the Pico files.
    return 0;
}
