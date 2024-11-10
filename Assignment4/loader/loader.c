#include "../loader/loader.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF Executable> \n", argv[0]);
        exit(1);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }

    size_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char* file_data = (char*) malloc(file_size);
    if (file_data == NULL) {
        perror("malloc failed");
        exit(1);
    }

    if (read(fd, file_data, file_size) != file_size) {
        perror("read failed");
        exit(1);
    }

    lseek(fd, 0, SEEK_SET);
    Elf32_Ehdr ehdr;

    ssize_t sizeof_elf = read(fd, &ehdr, sizeof(ehdr));
    if (sizeof_elf != sizeof(ehdr)) {
        printf("The size of loaded elf header doesn't match the standard elf header size");
        exit(1);
    }

    // 1. carry out necessary checks on the input ELF file
    // 2. passing it to the loader for carrying out the loading/execution
    load_and_run_elf(argv);
    // 3. invoke the cleanup routine inside the loader  
    loader_cleanup();
    return 0;
}
