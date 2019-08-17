#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SIZE 26		// the number of letters in the english alphabet

int visited[MAX_SIZE];	// Used to keep track of letters

void print_usage(char *program_name) {
	printf("Usage: %s <text_file>\n", program_name);
	printf("\nDescription: \n");
	printf("\tThis script creates a frequency map for every letter from <text_file>.\n");
	printf("\tNote: All letters are converted to uppercase.\n");
	exit(EXIT_SUCCESS);
}

/* This function finds the letter with the maximum number of encounters, then mark it as visited.
 * Each time the function it's called, it will return the next maximum value. For example, if the
 * encounters are: A = 53, B = 79, C = 23, the first call of the function will return the index of
 * B, the second call will return the index of A, the third call will return the index of A and
 * after that, all subsequent calls will return -1, because there will be no more letters.
 */
int find_max(int *buffer) {
	int maxIndex = -1;

	for (int i = 0; i < MAX_SIZE; ++i)
		if (!visited[i] && buffer[i])	// Check if it's not visited and if there are encounters of that letter
			maxIndex = i;

	if (maxIndex == -1)
		return maxIndex;

	for (int i = 0; i < MAX_SIZE; ++i)
		if (buffer[i] > buffer[maxIndex] && !visited[i])
			maxIndex = i;
	visited[maxIndex] = 1;
	return maxIndex;
}

void pretty_print(int *buffer) {
	// Printing the frequency map
	printf("|CHAR|\t : |ENCOUNTERS|\n");

	// The loop ends when the find_max() function will start to return -1, meaning that all letters in the
	// text and their encounters have been printed.
	for (int maxIndex = find_max(buffer); maxIndex != -1; maxIndex = find_max(buffer)) 
		printf("|%c|\t:\t%d\n", (char)(maxIndex + 65), buffer[maxIndex]);
}

void fatal(char *error) {
	printf("[Error] %s", error);
	exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[])
{
	FILE *fptr;
	int frequency[MAX_SIZE] = {0}, current_char;

	if (argc < 2) 
		print_usage(argv[0]);
	
	if ((fptr = fopen(argv[1], "r")) == NULL)
		fatal("when opening the file.");
	
	// Reads from the file one character at a time.
	while ((current_char = fgetc(fptr)) != EOF) {
		if (isupper(current_char))
			frequency[current_char - 65]++;
		else if (islower(current_char))
			frequency[current_char - 97]++;
	}
	fclose(fptr);
	pretty_print(frequency);
	return 0;
}
