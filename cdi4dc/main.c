/* 
	:: C D I 4 D C ::
	
	By SiZiOUS, http://sbibuilder.shorturl.com/
	5 july 2006, v0.2b
	
	File : 	MAIN.C
	Desc : 	The main file for cd4dc.
			
			Sorry comments are in french. If you need some help you can contact me at sizious[at]dc-france.com.
			
	
	Changes :
		0.3c : 06 january 2024
				- Fixed a bug with the 64 bit platform support
		0.3b :	13 april 2007
				- Added a new image generation method : Data/Data images.
				- Removed the algo that checks if the ISO is a veritable 11702 because it fails.
				
		0.2b : 	6 july 2006
				- If the iso doesn't exists, cdi4dc don't create an empty CDI file. (Thx Fackue)
				- Fixed a problem with CD Labels with spaces (Thx Fackue)
				- Checks if the ISO is a veritable 11702 iso.
		
		0.1b : 	5 july 2006
				Initial release
*/

#include "cdibuild.h"
#include "tools.h"

#define BUILD_DATE "06 january 2024"
#define VERSION "0.3c"

// prototypes
void get_volumename(FILE* iso, char* volume_name);
int write_cdi_audio_track(FILE *cdi);
void write_data_gap_start_track(FILE* cdi);
int write_audio_cdi_header(FILE *cdi, char* cdiname, char* volume_name, int32_t data_sector_count, int32_t total_cdi_space_used);
void write_data_header_boot_track(FILE* cdi, FILE* iso);
int write_data_cdi_header(FILE *cdi, char* cdiname, char* volume_name, int32_t data_sector_count, int32_t total_cdi_space_used);

unsigned int write_method = 0;

void print_help(char* prgname) {	
	printf("%s - %s\n", prgname, VERSION);	
	printf("Usage: ");
	printf("%s <input.iso> <output.cdi> [-d]\n\n", prgname);
	printf("Options:\n");
	printf("  -d        : Generates a Data/Data image from a MSINFO 0 ISO.\n");
	printf("  <nothing> : Generates a Audio/Data image from a MSINFO 11702 ISO.\n\n");	
}

void padding_event(int sector_count) {
	printf("%d block(s) : padding is needed...\n", sector_count);
}

void create_audio_data_image(FILE* infp, FILE* outfp, char* outfilename) {
	char volume_name[256];
	int data_blocks_count;
	float space_used;
	
	/*i = get_iso_msinfo_value(infp);
	if (i != 11702) { // 11702 pour notre MSINFO !
		
		printf("Warning: The ISO's LBA seems to be invalid ! Value : %d\n", i);
		
	}*/
		
	printf("Image method............: ");
	printf("Audio/Data\n");
	
	get_volumename(infp, volume_name);
	printf("Volume name.............: ");
	printf("%s\n", volume_name);
	
	// ecrire piste audio
	printf("Writing audio track.....: ");
	write_cdi_audio_track(outfp);
	printf("OK\n");
	
	// ecrire les tracks GAP
	printf("Writing pregap tracks...: ");
	write_gap_tracks(outfp);
	printf("OK\n");
	
	//ecrire section data
	printf("Writing datas track.....: ");
	
	data_blocks_count = write_data_track(outfp, infp);
	
	int total_space_used = get_total_cdi_space_used(data_blocks_count);
	space_used = (float)(total_space_used * data_sector_size) / 1024 / 1024;
	
	printf("\n%d block(s) written (%5.2fMB used)\n", data_blocks_count, space_used);
	
	// ecrire en t�te (le footer)
	printf("Writing CDI header......: ");
	write_audio_cdi_header(
		outfp, 
		outfilename, 
		volume_name, 
		data_blocks_count, 
		total_space_used
	);
	printf("OK\n");
}

void create_data_data_image(FILE* infp, FILE* outfp, char* outfilename) {
	char volume_name[32];
	int data_blocks_count;
	float space_used;
	
	printf("Image method............: ");
	printf("Data/Data\n");
	
	get_volumename(infp, volume_name);
	printf("Volume name.............: ");
	printf("%s\n", volume_name);
	
	// �crire le d�but du CDI data/data (GAP 1)
	printf("Writing data pregap.....: ");
	write_data_gap_start_track(outfp);
	printf("OK\n");
	
	//ecrire section data
	printf("Writing datas track.....: ");
	
	data_blocks_count = write_data_track(outfp, infp);
	
	space_used = (float)(get_total_cdi_space_used(data_blocks_count) * data_sector_size) / 1024 / 1024;
	
	printf("\n%d block(s) written (%5.2fMB used)\n", data_blocks_count, space_used);
	
	// ecrire les tracks GAP
	printf("Writing pregap tracks...: ");
	write_gap_tracks(outfp);
	printf("OK\n");

	// ecrire la piste 2 head.
	printf("Writing header track....: ");
	write_data_header_boot_track(outfp, infp);
	printf("OK\n");
	
	// ecrire en t�te (le footer)
	printf("Writing CDI header......: ");
	write_data_cdi_header(
		outfp, 
		outfilename, 
		volume_name, 
		data_blocks_count, 
		get_total_cdi_space_used(data_blocks_count)
	);
	printf("OK\n");
}

// let's go my friends
int main(int argc, char *argv[]) {
	char outfilename[256];
	FILE *outfp, *infp;
	
	printf("cdi4dc - %s\n", VERSION);
	
	if (argc < 3) {
		print_help(argv[0]);
		return 1;
	}
	
	infp = fopen(argv[1], "rb");
	if (infp == NULL) {
		perror("Error when opening file");
		return 2;
	}
	
	if (check_iso_is_bootable(infp) == 0) {
		printf("Warning: %s seems not to be bootable\n\n", argv[1]);
	}
	
	strcpy(outfilename, argv[2]);
	outfp = fopen(outfilename, "wb");
	if (outfp == NULL) {
		perror("Error when writing file");
		return 2;
	}
	
	// choose the method
	if ((argc == 4) && (strcmp(argv[3], "-d") == 0)) {
		create_data_data_image(infp, outfp, outfilename);
	} else {
		create_audio_data_image(infp, outfp, outfilename);	
	}
	
	printf("\nAll done OK!\nYou can burn it now\n");
	
	fclose(outfp);
	fclose(infp);
	
	return 0;
}
