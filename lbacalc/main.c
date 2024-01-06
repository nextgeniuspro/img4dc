#include <stdio.h>
#include "tools.h"

#define VERSION "0.1"
#define DATE "26/05/07"

#define AUDIO_BLOCK_SIZE 2352
#define MSINFO_OFFSET 11400
#define MINIMAL_FILE_SIZE 300 * AUDIO_BLOCK_SIZE
#define TAO_OFFSET 150

uint32_t fsize(char* filename) {
	uint32_t curpos, length = 0;	
	FILE* f = fopen(filename, "rb");
	
	if (f != NULL) {
		curpos = ftell(f); // garder la position courante
		fseek(f, 0L, SEEK_END);
		length = ftell(f);
		fseek(f, curpos, SEEK_SET); // restituer la position
		fclose(f);
		if (length < MINIMAL_FILE_SIZE) {
			printf("warning: \"%s\" file is violating the minimal track length !\n", filename);
		}
	} else {
		printf("warning: \"%s\" file doesn't exists !\n", filename);
	}
	
	return length;
}

void print_usage(char* prgname_cmdline) {
	char* prg_name = extract_proggyname(prgname_cmdline);
	
	printf("LBACALC - %s", VERSION);	
	printf("This program was crafted specifically for calculating the\n");
	printf("\"MSINFO\" value to be utilized with mkisofs during the creation\n");
	printf("of your data track, especially when dealing with numerous CDDA tracks.\n");
	printf("However, it's crucial to note that the tracks must be in RAW format, and \n");
	printf("not in WAV format.");

	printf("Usage:");	
	printf(" %s <track_1.raw ... track_n.raw>\n\n", prg_name);
	
	printf("Examples:\n");
	printf("   %s t1.raw : The proggy prints for example 11702 (in case of a minimal\n                    track size of 710304 bytes, corresponding to 4 sec).\n\n", prg_name);
	printf("   %s t1.raw t2.raw t3.raw t4.raw : Example output : ", prg_name);
	
	printf("12608");
	printf(".\n");
	printf("\nNow, you can use the computed value in mkisofs:\n");
	printf("  mkisofs -C 0,");
	printf("12608");
	
	printf(" -V IMAGE -G IP.BIN -joliet -rock -l -o data.iso data\n");
	printf("\nAnd now, you can use your generated iso in your selfboot binary.\n\n");	
	
	free(prg_name);
}

int main(int argc, char* argv[]) {
	int i, msinfo = 0;
	
	if (argc > 1) { 
		for(i = 1 ; i < argc ; i++) {
			msinfo = msinfo + (fsize(argv[i]) / AUDIO_BLOCK_SIZE) + 2; 
			if (i < argc - 1) // pas la derni�re piste
				msinfo = msinfo + TAO_OFFSET;
		}
		msinfo = msinfo + MSINFO_OFFSET;
		
		if (msinfo == 0) {
			printf("error: audio session is empty !");
			return 1;
		} else {
			printf("%d", msinfo);
		}	
	} else {
		print_usage(argv[0]);
	}
		
	return 0;
}

