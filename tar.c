#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

union record {
	char charptr[512];
	struct header {
		char name[100]; // nume
		char mode[8]; // permisiuni
		char uid[8]; // user id
		char gid[8]; // group id
		char size[12]; // marime
		char mtime[12]; // last modified
		char chksum[8]; // suma octeti header
		char typeflag; // = 0
		char linkname[100]; // = name
		char magic[8]; // = "GNUtar "
		char uname[32]; // nume owner
		char gname[32]; // nume grup
		char devmajor[8]; // = 0
		char devminor[8]; // = 0
	} header;
};

void Convert_to_octal(char *s8, char *s) {
	long long x, x8, p;
	int i;

	x = atol(s); // numarul din sirul dat e pus intr-un long long
	x8 = 0;
	
	for (p = 1; x; x /= 8, p *= 10) { // numarul in baza 8 in long long
		x8 += (x % 8) * p;
	}

	p = p / 10;

	for (i = 0; p; i++, p /= 10) {
		s8[i] = (x8 / p) % 10 + '0';
	}
	s8[i] = '\0';

}

void Find_id(char *username, char *uid, char *gid) {
	FILE *users;
	char s[512], *p;
	int length;
	users = fopen("usermap.txt", "r");
	
	while (fgets(s, 512, users)) {
		length = strlen(s);
		s[length - 1] = '\0';
		p = strstr(s, username);
		if (p) {
			length = strlen(username);
			if (p - s == 0 && p[length] == ':') {
				p = strtok(s, ":"); // username
				p = strtok(NULL, ":"); // 'x'
				p = strtok(NULL, ":"); // uid
				strcpy(uid, p);
				p = strtok(NULL, ":"); // gid
				strcpy(gid, p);
				break;
			}
		}
	}
	
	fclose(users);
}

void Convert_number_to_string(char *string, long long x) {
	int digits = 0, d, i;
	char aux;
	while (x != 0) {
		d = x % 10;
		string[digits++] = d + '0';
		x = x / 10;
	}
	for (i = 0; i < digits / 2; ++i) {
		aux = string[i];
		string[i] = string [digits - i - 1];
		string[digits - i - 1] = aux;
	}
}

void Fill_zero(char *string, int space) {
	int i, length = strlen(string), dif;
	dif = space - length;
	
	for (i = length - 1; i >= 0; --i)
		string[i + dif] = string[i];

	for (i = 0; i < dif; ++i)
		string[i] = '0';
}

void Load(char *archive) {
	FILE *tar, *ls, *file;
	char s[512], *p, *q, date[512], UID[8], GID[8], u, g, o,
		block[512];
	int length, i;

	ls = fopen("file_ls", "r");
	tar = fopen(archive, "ab");
	while (fgets(s, 512, ls)) { // parcurgem fiecare linie din file_ls
		if (s[0] == '-') { // daca e fisier
	
			length = strlen(s); 
			s[length - 1] = '\0'; // eliminam ultimul caracter (\n)			

			union record x;
			memset(x.charptr, 0, 512);
			p = strtok(s, " "); // aici avem permisiunile
			u = g = o = 0; // permisiunile

			if (p[1] == 'r')
				u += 4;
			if (p[2] == 'w')
				u += 2;
			if (p[3] == 'x')
				u += 1;
			if (p[4] == 'r')
				g += 4;
			if (p[5] == 'w')
				g += 2;
			if (p[6] == 'x')
				g += 1;
			if (p[7] == 'r')
				o += 4;
			if (p[8] == 'w')
				o += 2;
			if (p[9] == 'x')
				o += 1;
			x.header.mode[0] = '0' + u;
			x.header.mode[1] = '0' + g;
			x.header.mode[2] = '0' + o;
			Fill_zero(x.header.mode, 7);

			p = strtok(NULL, " "); // numarul de legaturi simbolice

			p = strtok(NULL, " "); // user name
			strcpy(x.header.uname, p);
	
			p = strtok(NULL, " "); // group name
			strcpy(x.header.gname, p);

			p = strtok(NULL, " "); // size
			Convert_to_octal(x.header.size, p);
			Fill_zero(x.header.size, 11);			
			p = strtok(NULL, " "); // data
			strcpy(date, p);
			strcat(date, " ");
			
			p = strtok(NULL, " "); // ora
			strcat(date, p);
			strcat(date, " ");

			p = strtok(NULL, " "); // time zone offset
			strcat(date, p);

			p = strchr(date, '.'); // elimin zecimalele secundelor
			q = strchr(p, ' ');
			strcpy(p, "");
			strcat(p, q); 


			struct tm full_time; // convertim sirul de caractere
			strptime(date, "%Y-%m-%d %H:%M:%S %z", &full_time); 

			time_t unix_timestamp; // aflam unix timestamp
			unix_timestamp = mktime(&full_time);

			// convertim in sir si apoi in octal
			Convert_number_to_string(date, unix_timestamp);
			Convert_to_octal(x.header.mtime, date);
			Fill_zero(x.header.mtime, 11);

			p = strtok(NULL, " "); // nume fisier
			strcpy(x.header.name, p);		

			Find_id(x.header.uname, UID, GID); // aflam uid si gid	

			Convert_to_octal(x.header.uid, UID);
			Convert_to_octal(x.header.gid, GID);
			Fill_zero(x.header.uid, 7);
			Fill_zero(x.header.gid, 7);

			strcpy(x.header.magic, "GNUtar ");
			x.header.typeflag = '0';
			
			long long sum = 32 * 8; // calculam checksum
			char s[10];
			for (i = 0; i < 512; ++i) {
				sum += (long long)x.charptr[i];
			}

			Convert_number_to_string(s, sum);
			Convert_to_octal(x.header.chksum, s);
			Fill_zero(x.header.chksum, 6);
			x.header.chksum[7] = ' ';
			
 			// incepem afisarea 
			fwrite(&x, sizeof(x), 1, tar);
			
			file = fopen(x.header.name, "rb");
			
			memset(block, 0, 512);	
			while (fread(block, sizeof(char), 512, file)) {
				fwrite(block, sizeof(char), 512, tar);
				memset(block, 0, 512);
			}

			fclose(file);
						
		}
	}

	memset(block, 0, 512);
	fwrite(block, sizeof(char), 512, tar);
	fwrite(block, sizeof(char), 512, tar);

	fclose(tar);
	fclose(ls);
}	

void List(char *archive) {
	char zero_block[512];	
	int size, size8, p, i, data_blocks;
	FILE *tar;
	tar = fopen(archive, "rb");
	fseek(tar, 0, SEEK_SET);	

	memset(zero_block, 0, 512); // sir de caractere plin de 0
	union record x;

	fread(x.charptr, 512, 1, tar); // citim cate un block de 512 caractere
	while (strcmp(x.charptr, zero_block) != 0) { // cat timp nu am citit un bloc nul

		printf("%s\n", x.header.name);		

		size8 = atoi(x.header.size);

		size = 0;
		for (p = 1; size8; size8 /= 10, p *= 8) { // convertim in decimal
			size += (size8 % 10) * p;
		}

		
		if (size % 512 == 0) // informatiile sunt stocate in blocuri de 512
			data_blocks = size / 512;
		else
			data_blocks = size / 512 + 1;
		

		for (i = 1; i <= data_blocks; ++i) { // continutul fisierelor
			fread(x.charptr, 512, 1, tar);
		} 
		
		fread(x.charptr, 512, 1, tar);
		if (!strcmp(x.charptr, zero_block)) // ignoram primul bloc nul gasit deoarece
			fread(x.charptr, 512, 1, tar); // sfarsitul arhivei e marcata de 2 blocuri
	}						// nule consecutive

	fclose(tar);	
}

void Get(char *archive, char *file) {
	char zero_block[512];	
	int size, size8, found, p, i, data_blocks;
	FILE *tar;
	tar = fopen(archive, "rb");
	fseek(tar, 0, SEEK_SET); // inceputul fisierului	

	memset(zero_block, 0, 512); // sir de caractere plin de 0
	union record x;

	found = 0; // nu am gasit inca fisierul cautat

	fread(x.charptr, 512, 1, tar); // citim cate un block de 512 caractere
	while (strcmp(x.charptr, zero_block) != 0) { // cat timp nu am citit un bloc nul
		
		size8 = atoi(x.header.size);

		size = 0;
		for (p = 1; size8; size8 /= 10, p *= 8) { // convertim in decimal
			size += (size8 % 10) * p;
		}

		
		if (size % 512 == 0) // informatiile sunt stocate in blocuri de 512
			data_blocks = size / 512;
		else
			data_blocks = size / 512 + 1;

		if (strcmp(file, x.header.name) == 0) {
			found = 1; // urmeaza fisierul cautat
		}

		for (i = 1; i <= data_blocks; ++i) { // continutul
			fread(x.charptr, 512, 1, tar);
			if (found) { // afisam continutul fisierului
				if (i < data_blocks) { 
					fwrite(&x, 512, 1, stdout);
				}
				else { // ultimul bloc poate avea 0 suplimentare
					if (size % 512 == 0) { // daca nu are
						fwrite(&x, 512, 1, stdout);
					}
					else { // daca are afisam doar continutul relevant
						data_blocks--;
						fwrite(&x, size - data_blocks * 512, 1, stdout);
					}
				}
			}
		} 
		
		if (found) {
			break;
		}

		fread(x.charptr, 512, 1, tar);
		if (!strcmp(x.charptr, zero_block)) // ignoram primul bloc nul gasit deoarece
			fread(x.charptr, 512, 1, tar); // sfarsitul arhivei e marcata de 2 blocuri
	}						// nule consecutive

	fclose(tar);	

}

void Read() {
	char s[512], command[10], archive[512], file[512], *p;
	int length;

	fgets(s, 512, stdin); // citim comanda
	length = strlen(s);
	s[length - 1] = '\0'; // eliminam '\n' de la sfarsitul comenzii

	while (strcmp(s, "quit") != 0) { 
		p = strtok(s, " ");
		strcpy(command, p); // tipul comenzii
		p = strtok(NULL, " ");
		strcpy(archive, p); // arhiva de lucru
		p = strtok(NULL, " ");
		if (p != NULL) // daca avem al treilea parametru
			strcpy(file, p); // numele fisierului
			
		if (strcmp(command, "load") == 0) {
			Load(archive);
		}
		else {
			if (strcmp(command, "list") == 0) {
				List(archive);
			}
			else {
				Get(archive, file);
			}
		}
		fgets(s, 512, stdin);
		length = strlen(s);
		s[length - 1] = '\0';
	}
}

int main() {
	Read();
	return 0;
}	
	

	
