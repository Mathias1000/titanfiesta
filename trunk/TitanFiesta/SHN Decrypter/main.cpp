#include <stdio.h>

int main(int argc, char** argv){
	if(argc < 2){
		return 0;
	}
	FILE* fh;
	fopen_s(&fh, argv[1], "rb");
	if(fh == NULL){
		printf("Could not open file\n");
		return 0;
	}
	fseek(fh, 0, SEEK_END);
	long int filesize = ftell(fh);
	fseek(fh, 0, SEEK_SET);
	unsigned char* data = new unsigned char[filesize];
	fread(data, filesize, 1, fh);
	fclose(fh);

	data += 0x24;
	filesize -= 0x24;
	unsigned char esp10 = data[filesize - 1];
	unsigned char dl;
	unsigned int eax = filesize;
	for(long int i = filesize - 1; i > 0; --i){
		dl = data[i];
		dl ^= (eax & 0xFF);
		eax &= ~0xFF;
		eax |= i & 0xFF;
		data[i] = dl;
		char tmp = eax & 0xff;
		short tmp2 = tmp * 0x0b;
		eax &= ~0xFFFF;
		eax |= *reinterpret_cast<unsigned short*>(&tmp2);
		dl = i & 0xff;
		dl &= 0x0f;
		dl += 0x55;
		dl ^= eax & 0xFF;
		dl ^= esp10;
		dl ^= 0xaa;
		eax &= ~0xFF;
		eax |= dl & 0xFF;
		esp10 = eax & 0xFF;
	}
	data -= 0x24;
	filesize += 0x24;

	{
		char tempPath[256];
		sprintf_s(tempPath, 256, "%s.dec", argv[1]);
		FILE* fh;
		fopen_s(&fh, tempPath, "wb");
		if(fh == NULL){
			printf("Could not open file\n");
			return 0;
		}
		fwrite(data, filesize, 1, fh);
		fclose(fh);
	}

	return 0;
}