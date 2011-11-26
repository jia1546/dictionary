#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEN 1024
#define SLOTS 12553
#define CHARACTERS 128

struct hashTableEntry
{
	char key[LEN];
	char value[LEN];
};
typedef struct hashTableEntry hashTableEntry;

hashTableEntry hashTable[SLOTS];

/* string to integer transformation using Hornor's Rule */
int getHashValue(char *key)
{
	unsigned int i;
	int temp = key[0];	
	for(i=1; i<strlen(key); i++)
	{
		temp = (temp % SLOTS) * CHARACTERS;
		temp = temp + key[i];
	}
	temp = temp % SLOTS;
	return temp;
}

/* here we use quadratic probing, but it is easy to chage to linear probing and double hashing */
int function(int i)
{
	i = i * i;
	return i;
}

int getProbeValue(char *key, int i)
{
	return ((getHashValue(key) + function(i)) % SLOTS);
}

/* initialize the hash table with entries read from dict.txt */
void initHashTable()
{
	char key[LEN], value[LEN];
	FILE *fp = fopen("dict.txt", "rt");
	if(fp == NULL)
	{
		printf("open input file error\n");
		return;
	}
	
	int i = 0;
	for(i=0; i<SLOTS; i++)
	{
		strcpy(hashTable[i].key, "NIL");
	}
	
	int addr;
	while(fscanf(fp, "%s %[^\n]", key, value) == 2)
	{
		i=0;
		do
		{
			addr = getProbeValue(key, i);
			if(strcmp(hashTable[addr].key, "NIL") == 0)
			{
				strcpy(hashTable[addr].key, key);
				strcpy(hashTable[addr].value, value);
				break;
			}
			i++;
		}while(i < SLOTS);
		if(i >= SLOTS)
			printf("hash table overflow!\n");			// according to the requirement specification, this case will never happen
	}
	fclose(fp);
}

/* given the key, look up the corresponding value */
int lookup(char *key, char *value)
{
	int i = 0, addr;
	do
	{
		addr = getProbeValue(key, i);
		if(strcmp(hashTable[addr].key, key) == 0)
		{
			strcpy(value, hashTable[addr].value);
			return 1;
		}
		i++;
	}while(strcmp(hashTable[addr].key, "NIL") != 0 && i < SLOTS);
	strcpy(value, "lookup failed");
	return 0;
}

/* compute the number of probes for looking up the word*/
int getProbes(char *key)
{
	int i = 0, addr;
	do
	{
		addr = getProbeValue(key, i);
		if(strcmp(hashTable[addr].key, key) == 0)
		{
			break;
		}
		i++;
	}while(strcmp(hashTable[addr].key, "NIL") != 0 && i < SLOTS);
	return i;	
}

/* given the pair of key and value, insert it into the hash table*/
void insert(char *key, char *value)
{
	char non[LEN];
	if(lookup(key, non) == 1)
	{
		strcpy(value, "failed");
	}
	else
	{
		int i=0, addr;
		do
		{
			addr = getProbeValue(key, i);
			if(strcmp(hashTable[addr].key, "NIL") == 0 || strcmp(hashTable[addr].key, "DELETED") == 0)
			{
				strcpy(hashTable[addr].key, key);
				strcpy(hashTable[addr].value, value);
				strcpy(value, "success");
				break;
			}
			i++;
		}while(i < SLOTS);
		if(i >= SLOTS)
			printf("hash table overflow!\n");
	}
}

/* remove the give key and its corresponding value from the hash table */
void remove(char *key, char *value)
{
	char non[LEN];
	if(lookup(key, non) == 0)
	{
		strcpy(value, "failed");
	}
	else
	{
		int i = 0, addr;
		do
		{
			addr = getProbeValue(key, i);
			if(strcmp(hashTable[addr].key, key) == 0)
			{
				strcpy(hashTable[addr].key, "DELETED");
				strcpy(value, "success");
				break;
			}
			i++;
		}while(strcmp(hashTable[addr].key, "NIL") != 0 && i < SLOTS);
	}
}

/* compute the number of probes for successful search*/
double S()
{
	int s = 0, i, num = 0;
	for(i=0; i<SLOTS; i++)
	{
		if(strcmp(hashTable[i].key, "NIL") != 0 && strcmp(hashTable[i].key, "DELETED") != 0)
		{
			num++;
			s += getProbes(hashTable[i].key);
		}
	}
	return ((double)s / num);
}

/* compute the number of probes for unsuccessful search*/
double F()
{
	int s = 0, index, i;
	for(index=0; index<SLOTS; index++)
	{
		i = 0;
		while(strcmp(hashTable[(index + function(i)) % SLOTS].key, "NIL") != 0)
		{
			++i;
		}
		s += (i+1);
	}
	return ((double)s / SLOTS);
}

void runCommand(char **argv)
{
	FILE *fin = fopen(argv[1], "r");
	FILE *fout = fopen(argv[2], "w");
	char line[LEN], command[LEN], arguments[LEN];
	char key[LEN], value[LEN];
	
	if(fin == NULL)
	{
		printf("open input file error\n");
		return;
	}
	if(fout == NULL)
	{
		printf("open output file error\n");
		return;
	}
	
	while(fgets(line, LEN, fin))
	{
		sscanf(line, "%s %[^\n]", command, arguments);
		if(strcmp(command, "lookup") == 0)
		{
			sscanf(arguments, "%s", key);
			lookup(key, value);
			fprintf(fout, "%s:%s\n", key, value);
		}
		else if(strcmp(command, "insert") == 0)
		{
			if (sscanf(arguments, "%s %[^\n]", key, value) != 2) 
			{
				printf("error format for insert command\n");
				continue;
			}
			insert(key, value);
			fprintf(fout, "%s:insert %s\n", key, value);
		}
		else if(strcmp(command, "remove") == 0)
		{
			sscanf(arguments, "%s", key);
			remove(key, value);
			fprintf(fout, "%s:remove %s\n", key, value);
		}
		else if(strcmp(command, "status") == 0)
		{
			fprintf(fout, "status %.2f %.2f\n", S(), F());
		}
		else
		{
			printf("invalid command\n");
			continue;
		}
	}
	fclose(fin);
	fclose(fout);
}

int main(int argc, char **argv)
{
	initHashTable();
	runCommand(argv);
	return 0;
}
