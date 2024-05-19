#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int wordCount(char *arg,int dataword_size);
void toBinary(char input, char *data);
int toByte(char *codeword_total ,int now);
void modulo2Div(char *data, char *generator,int dataword_size, int generator_size);


int main(int argc, char *argv[]) {
    FILE* input_file = fopen(argv[1], "rb");
    FILE* output_file = fopen(argv[2], "wb");
    FILE* result_file = fopen(argv[3], "w");

	// (1) 만약 인자의 수가 맞지 않으면 아래와 같은 메시지를 출력하고 종료
	if(argc!=6){
		fprintf(stderr, "usage: ./crc_decoder input_file output_file result_file generator dataword_size\n");
		exit(1);
	}

	// (2) 만약 input_file을 open하지 못하면 다음과 같은 메시지를 출력하고 종료
	if (input_file == NULL){
		fprintf(stderr, "input file open error.\n");
		exit(1);
	}
	
	// (3) 만약 output_file을 open하지 못하면 다음과 같은 메시지를 출력하고 종료
	if (output_file == NULL){
		fprintf(stderr, "output file open error.\n");
		exit(1);
	}
    
	// (4) 만약 result_file을 open하지 못하면 다음과 같은 메시지를 출력하고 종료
	if (result_file == NULL){
		fprintf(stderr, "result file open error.\n");
		exit(1);
	}

    
    int generator_size = strlen(argv[4]);
	char generator[generator_size]; 

	int t = 0;
	while (t < generator_size) {
		generator[t] = argv[4][t]-'0';
		t++;
	}

    char dataword_size = *argv[5]-'0';
	if(dataword_size != 4 && dataword_size != 8){
		fprintf(stderr, "dataword size must be 4 or 8.\n");
	 	exit(1);
	}

    int word_count = wordCount(argv[1],dataword_size); 
	int total_length = (word_count-1)*8; 
	char *codeword_total = (char*)malloc(sizeof(char)*(total_length)); 
	char padding, word;
	char data_binary[8];

	fread(&padding, 1, sizeof(char), input_file);

	for(int j=0;j<word_count-1;j++){
		fread(&word, 1, sizeof(char), input_file);
		toBinary(word,data_binary);

		int u = 0;
		while (u < 8) {
			codeword_total[j*8+u] = data_binary[u]; 
			u++;
		}
	}
	
	int codeword_size = dataword_size+generator_size-1;
	int codeword_count= (total_length-padding)/codeword_size ;
	char *dataword_total = (char*)malloc(sizeof(char)*(dataword_size)*codeword_count); 
	int now = padding;
	unsigned char err;
	int count;
	
	for(int i=0;i<codeword_count;i++){
		count=0;
		char *codeword = (char *)malloc(sizeof(char) * codeword_size);
        memset(codeword, 0, codeword_size);

		int j = 0;
		while (j < dataword_size) {
			codeword[j] = codeword_total[now];
			dataword_total[(dataword_size*i)+j] = codeword_total[now++];
			j++;
		}

		while (j < codeword_size) {
			codeword[j] = codeword_total[now++];
			j++;
		}


		modulo2Div(codeword,generator, codeword_size, generator_size);

		int p = 0;
		while (p < codeword_size) {
			count += codeword[p];
			p++;
		}

		if(count!=0){
			err++;
		}
	}
	
	int stream_size = ((codeword_count*dataword_size)+padding)/8;
	char *stream =(char *) malloc(sizeof(char) * stream_size);
	unsigned int temp;
	now = 0;

	int v = 0;
	while (v < stream_size) {
		int temp = toByte(dataword_total, now);
			stream[v] = temp;
			now += 8;
			v++;
		}

	fwrite(stream, stream_size, 1, output_file);
	fprintf(result_file,"%d %d",codeword_count,err);
	
	fclose(input_file);
	fclose(output_file);
	fclose(result_file);

	free(stream);
	free(dataword_total);
	free(codeword_total);

    return 0;
}

int wordCount(char *arg,int dataword_size){
	FILE *inputfile = fopen(arg, "rb");
	fseek(inputfile, 0, SEEK_END);    
	int file_size = ftell(inputfile);         
	fclose(inputfile);
	return file_size;
}

void toBinary(char input, char *data){
    int i = 7;
    while (i >= 0) {
        char temp = input & 1;
        data[i] = temp;
        input = input >> 1;
        i--;
    }
}

int toByte(char *codeword_total, int now) {
    int temp = 0;

    int i = 0;
    while (i < 7) {
        temp += codeword_total[now++];
        temp *= 2;
        i++;
    }
	
    temp += codeword_total[now];
    return temp;
}

void modulo2Div(char *data, char *generator, int data_size, int generator_size) {
	for (int i = 0; i <= data_size - generator_size; i++) {
		if (data[i] == 0) {
			continue;
		}
		for (int j = 0; j < generator_size; j++) {
			data[i + j] = (data[i + j] == generator[j]) ? 0 : 1;
		}
	}
}