#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "imgwrite.h"
#include "mdfwrt.h"
#include "config.h"
#include "console.h"

unsigned int image_format = UNKNOW_IMAGE_FORMAT;
unsigned int write_method = TAO_WRITE_METHOD_MODE;
int image_creation_okay = 1;

char* proggy_name;

char* output_mds_filename;
char* output_mdf_filename;

char* data_track_iso_filename;

int audio_files_count = 0;
char** audio_files_array = NULL;
	
// position ou on doit placer le curseur avant d'�crire le pourcentage
uint32_t x = 0, y = 0;

void print_help() {

	printf("MDS4DC - Generates Alcohol 120%/50% MDS/MDF images from an ISO and RAW tracks\n\n")
	printf("Usage:\n");
	
	printf("%s <image_format> <output.mds> <data.iso> [audio_1.raw ... audio_n.raw]\n\n", proggy_name);
	
	printf("Image format:\n");
	printf("  -a : Generates a Audio/Data image from a MSINFO 11702 ISO.\n");
	printf("  -c : Generates a Audio/Data image from an ISO with a calculated MSINFO.\n");
	printf("  -d : Generates a Data/Data image from MSINFO 0 ISO.\n");
}

void warning_msg(char* msg) {
	
	printf("%s", msg);
	
}

void info_msg(char* msg) {
	int i;
	unsigned char buf[INFO_MSG_SIZE];
	unsigned int len = (INFO_MSG_SIZE - strlen(msg)) - 1; // -1 pour le ":"
	
	if (strlen(msg) < INFO_MSG_SIZE) {
		sprintf(buf, "%s", msg);
		for(i = 0 ; i < len ; i++)
			sprintf(buf, "%s.", buf);
		sprintf(buf, "%s: ", buf);
		
		printf("%s", buf);
	} else {
		printf(msg);
	}
}

void free_all_stuffs() {
	int i;
	
	// destructions des diff�rents buffers de noms.
	free(output_mds_filename);
	free(output_mdf_filename);
	free(data_track_iso_filename);
	free(proggy_name);
	
	// destruction du tableau contenant les pistes CDDA
	if (image_format == AUDIO_DATA_CUSTOM_CDDA_IMAGE_FORMAT) {
		for(i = 0 ; i < audio_files_count ; i++)
			free(audio_files_array[i]);
		free(audio_files_array);
	}
}

#ifdef APP_CONSOLE
void start_progressbar() {
	x =	whereX();
	y = whereY();
}

void padding_event(int sector_count) {
	gotoXY(x, y);
	
	printf("%d block(s) : padding is needed...", sector_count);
	
}

void writing_track_event(uint32_t current, uint32_t total) {
	gotoXY(x, y);
	
	float p = (float)current / (float)total;	
	uint32_t percent = p * 100;
	printf("[%u/%u] - %u%%\n", current, total, percent);	
}

void writing_track_event_end(uint32_t block_count, uint32_t track_size) {
	gotoXY(x, y);
	char unit[2];
	float res = (float)track_size;
	strcpy(unit, get_friendly_unit(&res));
	printf("%d block(s) written (%1.2f%s used)\n", block_count, res, unit);
}

#endif

/* 
	Ligne de commande :
	mds4dc <image_format> <output.mds> <data.iso> [audio_1.raw ... audio_n.raw]
	<image_format> pourra accepter plus tard un param�tre pour faire des images DAO � la place d'une TAO.
	
	<image_format> :
					-a : audio/data
					-c : audio/data with CDDA
					-d : data/data
*/
int main(int argc, char* argv[]) {
	int i;
	char volume_name[33];
	
	FILE* mdf = NULL; // mdf "Image"
	FILE* mds = NULL; // mds "Media Descriptor"
	FILE* iso = NULL; // data track
	
	proggy_name = extract_proggyname(argv[0]);
	
	printf("MDS4DC - %s", VERSION);
		
	if (argc < 4) {
		print_help();
		return 1;
	}
	
	// d�tecter le type d'image
	if(strcmp(argv[1], "-a") == 0) image_format = AUDIO_DATA_IMAGE_FORMAT;
	if(strcmp(argv[1], "-c") == 0) image_format = AUDIO_DATA_CUSTOM_CDDA_IMAGE_FORMAT;
	if(strcmp(argv[1], "-d") == 0) image_format = DATA_DATA_IMAGE_FORMAT;
	if (image_format == UNKNOW_IMAGE_FORMAT) {
		warning_msg("Unknow option for the image format.\n");
		return 2;
	}		
	
	// r�cup�rer les pistes CDDA
	if (image_format == AUDIO_DATA_CUSTOM_CDDA_IMAGE_FORMAT) {
		audio_files_count = argc - 4; // les 4 premiers arguments sont reserv�s.
		if (audio_files_count == 0) {
			warning_msg("No CDDA tracks were found buddy, check your syntax by reading the help !\n");
			return 3;
		}
		
		// cr�ation d'un tableau de string contenant toutes les pistes CDDA.
		audio_files_array = (char**) malloc(sizeof(char*) * audio_files_count);
		for(i = 0 ; i < audio_files_count ; i++)
			audio_files_array[i] = strdup(argv[i + 4]);
	}
		
	// afficher le nom du volume de l'ISO
	data_track_iso_filename = strdup(argv[3]);
	iso = fopen(data_track_iso_filename, "rb");
	if (iso != NULL) {
		if (!check_iso_is_bootable(iso)) {
			printf("Warning: %s seems not to be bootable !\n\n", data_track_iso_filename);
		}
		get_volumename(iso, volume_name);
	} else { 
		// probl�me avec l'ISO
		
		printf("Error when reading %s !\n", data_track_iso_filename);
		
		free(data_track_iso_filename);
		return 4;
	}
	
	// afficher le format de l'image
	info_msg("Image format");
	switch(image_format) {
		case AUDIO_DATA_IMAGE_FORMAT :
			printf("Audio/Data\n");
			break;
		case AUDIO_DATA_CUSTOM_CDDA_IMAGE_FORMAT :
			printf("Audio/Data with custom CDDA\n");
			break;
		case DATA_DATA_IMAGE_FORMAT :
			printf("Data/Data\n");
	}
	
	// nom de volume
	info_msg("Volume name");
	printf("%s\n", volume_name);
		
	// r�cuperer les noms de fichiers
	output_mds_filename = check_ext(argv[2], "mds");
	output_mdf_filename = check_ext(argv[2], "mdf");
	
	// ouvrir les diff�rents fichiers
	mdf = fopen(output_mdf_filename, "wb");
	mds = fopen(output_mds_filename, "wb");
	
	if (image_format == AUDIO_DATA_CUSTOM_CDDA_IMAGE_FORMAT)
		write_audio_data_image(mds, mdf, iso, audio_files_count, audio_files_array);
	else if (image_format == AUDIO_DATA_IMAGE_FORMAT)
		write_audio_data_image(mds, mdf, iso, 1, NULL);
	else if (image_format == DATA_DATA_IMAGE_FORMAT)
		write_data_data_image(mds, mdf, iso);

	free_all_stuffs();

	int exit_code = 0;
	if (image_creation_okay) {
		printf("\nAll done okay. You can burn it now !");
	} else {
		
		printf("\nThere was an error when creating the image... :(\n");
		exit_code = 5;
	}
	
	
	return exit_code;
}
