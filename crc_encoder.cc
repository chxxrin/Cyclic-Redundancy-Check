#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int wordCount(char *arg,int dataword_size);
void toBinary(char input, char *data);
int toByte(char *codeword_total ,int now);
void modulo2Div(char *data, char *generator,int dataword_size, int generator_size);

int main(int argc, char *argv[]) {
	// 파일 입출력에 필요한 파일 포인터 변수 선언
    FILE* input_file;
    FILE* output_file;

	// (1) 만약 인자의 수가 맞지 않으면 아래와 같은 메시지를 출력하고 종료
    if(argc != 5) {
        fprintf(stderr, "usage: ./crc_encoder input_file output_file generator dataword_size\n");
        exit(1);
    }

	// (2) 만약 input_file을 open하지 못하면 다음과 같은 메시지를 출력하고 종료
    input_file = fopen(argv[1], "rb");
    if (input_file == NULL) {
        fprintf(stderr, "input file open error.\n");
        exit(1);
    }

	// (3) 만약 output_file을 open하지 못하면 다음과 같은 메시지를 출력하고 종료
    output_file = fopen(argv[2], "wb");
    if (output_file == NULL) {
        fprintf(stderr, "output file open error.\n");
        exit(1);
    }

    // (4) dataword_size가 4 또는 8이 아닌 경우, 다음과 같은 메시지를 출력하고 종료
    int dataword_size = *argv[4]-'0';
    if(dataword_size != 4 && dataword_size != 8) {
        fprintf(stderr, "dataword size must be 4 or 8.\n");
        exit(1);
    }

	int word_count = wordCount(argv[1],dataword_size);
	int data_count = dataword_size % 8 ? word_count*2 : word_count;
	int generator_size = strlen(argv[3]);
	char generator[generator_size]; 

    for(int i=0;i<generator_size;i++){
		if(argv[3][i]=='0')
			generator[i]=0;
		else if(argv[3][i]=='1')
			generator[i]=1;
		else {
			printf("Error: invalid generator input\n");
			exit(1);
		}
	}
	
    char *dataword_total= (char*)malloc(sizeof(char)*(data_count*dataword_size));
	char data_binary[8];
	char word;
	int codeword_size = dataword_size+generator_size-1;
    int padding = (8-(data_count*codeword_size%8))%8;
    char *codeword_total = (char*)malloc(sizeof(char) * ((data_count * codeword_size) + padding));
    if (codeword_total == NULL) {
        // 동적 할당 실패 처리
        exit(1);
    }
    memset(codeword_total, 0, (data_count * codeword_size) + padding);

	int now = padding;

	for(int j=0;j<word_count;j++){
		fread(&word, 1, sizeof(char), input_file); 
		toBinary(word,data_binary);
		int v = 0;
		while (v < 8) {
			dataword_total[j*8+v] = data_binary[v];
			v++;
		}

		if(dataword_size == 4){
			for(int i=0;i<2;i++){
				char *codeword = (char*) malloc(codeword_size * sizeof(char));
                memset(codeword, 0, codeword_size * sizeof(char));

				int q = 0;
				while (q < 4) {
					codeword_total[now++] = data_binary[q+4*i]; 
					codeword[q] = data_binary[q+4*i]; 
					q++;
				}

				modulo2Div(codeword,generator, codeword_size, generator_size);

				int w = 4;
				while (w < codeword_size) {
					codeword_total[now++] = codeword[w]; 
					w++;
				}

				free(codeword);
			}
		}
		else{
            char *codeword = (char*)malloc(codeword_size * sizeof(char));
            memset(codeword, 0, codeword_size * sizeof(char));
			
			int k = 0;
			while (k < 8) {
				codeword_total[now++] = data_binary[k];
				codeword[k] = data_binary[k];
				k++;
			}

			modulo2Div(codeword,generator, codeword_size, generator_size);

			int l = 8;
			while (l < codeword_size) {
				codeword_total[now++] = codeword[l];
				l++;
			}

			free(codeword);
		}
	}
	int result_size = (data_count*codeword_size+padding)/8 +1;
	char *result =(char *) malloc(sizeof(char) * result_size);
	unsigned int temp;
	result[0] = padding;
	now = 0;

    int i = 1;
    while (i < result_size) {
        temp = toByte(codeword_total, now);
        result[i] = temp;
        now += 8;
        i++;
    }
	
	fwrite(result, result_size, 1, output_file);
	free(result);

	fclose(input_file);
	fclose(output_file);

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