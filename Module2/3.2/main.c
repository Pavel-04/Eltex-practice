#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int get_ip(char* str) {
	uint32_t IP = 0;
	char* token = strtok(str, ".");

	for (int i = 0;i < 4;i++) {
		if (token != NULL) {
			uint8_t tmp = atoi(token);
			IP |= tmp;

			if (i < 3) {
				IP = IP << 8;
			}
		}
		token = strtok(NULL, ".");
	}

	return IP;
}

void genstat_IP(uint32_t gateway, uint32_t mask,uint32_t* ip_gen, int n) {
	srand(time(NULL));
	int count = 0;
	for (int i = 0;i < n;i++) {
		uint8_t octet1 = rand() % 256;
		uint8_t octet2 = rand() % 256;
		uint8_t octet3 = rand() % 256;
		uint8_t octet4 = rand() % 256;
		ip_gen[i] = (octet1 << 24) | (octet2 << 16) | (octet3 << 8) | octet4;

		if ((gateway & mask) == (ip_gen[i] & mask)) {
			count++;
		}
		

	}
	printf("Принято %d, Не принято %d\n", count, n - count);
}




int main(int argc, char* argv[]) {

	uint32_t gateway = get_ip(argv[1]);
	uint32_t mask = get_ip(argv[2]);
	int n = atoi(argv[3]);
	uint32_t* ip_gen;
	ip_gen = (uint32_t*)malloc(n * sizeof(uint32_t));
	genstat_IP(gateway,mask,ip_gen, n);
	free(ip_gen);



	return 0;

}