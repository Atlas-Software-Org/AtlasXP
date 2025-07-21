#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void EscapeAndWrite(FILE* out, const char* line) {
	while (*line) {
		if (*line == '\"') fputs("\\\"", out);
		else if (*line == '\\') fputs("\\\\", out);
		else if (*line == '\n') fputs("\\n", out);
		else if (*line == '\r') fputs("\\r", out);
		else fputc(*line, out);
		line++;
	}
}

int main(int argc, char** argv) {
	if (argc != 4) {
		fprintf(stderr, "Usage: %s <input.txt> <output.h> <identifier>\n", argv[0]);
		return 1;
	}

	const char* inputPath = argv[1];
	const char* outputPath = argv[2];
	const char* identifier = argv[3];

	FILE* in = fopen(inputPath, "r");
	if (!in) {
		perror("fopen input");
		return 1;
	}

	FILE* out = fopen(outputPath, "w");
	if (!out) {
		perror("fopen output");
		fclose(in);
		return 1;
	}

	fprintf(out, "const char* %s =\n", identifier);

	char buffer[4096];
	while (fgets(buffer, sizeof(buffer), in)) {
		fputc('\"', out);
		EscapeAndWrite(out, buffer);
		fputs("\"\n", out);
	}

	fputs(";\n", out);

	fclose(in);
	fclose(out);

	return 0;
}
