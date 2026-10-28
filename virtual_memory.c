#include <stdio.h>
#include <sys/mman.h>

int main() {
	int size = 10 * 1024 * 1024; // 10 MB
	char *p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	// Citire din fiecare pagină
	for (int i = 0; i < size; i += 4096) {
		volatile char temp = p[i];
	}
	while(1);
}

