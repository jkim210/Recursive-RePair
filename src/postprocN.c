// postproc.c
//
// final tool for bigRepair for chars  
// combines a IRePair compression for the dictionary and the parse into
// a RePair compression for the original input

// file.dicz.int.R contains rules for dictionary words, first int is # of terminals
// file.dicz.int.C contains the sequences, separated by terminals >= 256
// for each sequence (=original dictionary word), create balanced rules 
// to convert it into one nonterminal
// the terminals in file.parse must be renamed as these nonterminals
// and the nonterminals must be shifted accordingly
// the rules of .parse.R and .dicz.int.R must be appended

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>

//  exit program with error msg if test is true
void die(int test, const char* msg)
{
	if (test) {
		perror(msg);
		exit(1);
	}
}


static int bits(size_t x)

{
	int l = 0;
	while (x) { x >>= 1; l++; }
	return l;
}

int main(int argc, char** argv)

{
	FILE* diczR, * diczC, * parseR, * parseC, * R, * C;
	char fname[1024];
	char nname[1024];
	int terms; // how many terminals in diczR
	size_t rules; // how many rules in diczR
	size_t prules = 0; // how many rules in parseR
	size_t sizeC; // size of diczC
	size_t psizeC = 0; // size of parseC
	int phrases; // how many phrases in dicz = terminals in parseR
	struct stat s;
	int v256 = 256; // upper bound on the size of the input alphabet
	size_t i, p, e;
	int val[2];
	int* AdiczC; // to read diczC in memory
	int* transl; // translation table for phrases of dicz to their nonterms

	fprintf(stderr, "==== Command line:\n");
	for (i = 0; i < argc; i++)
		fprintf(stderr, " %s", argv[i]);
	fputs("\n", stderr);

	if (argc != 2)
	{
		fprintf(stderr,
			"Usage: %s <file> makes <file>.[RC] from <file>.[dicz.int+parse].[RC]\n",
			argv[0]);
		exit(1);
	}

	//rename files
	sprintf(nname, "%s.R", argv[1]);
	sprintf(fname, "%s.parse.R", argv[1]);
	if (rename(fname, nname) == -1) {
		fprintf(stderr, "Cannot rename .parse.R to %s\n", nname);
		exit(1);
	}

	sprintf(nname, "%s.C", argv[1]);
	sprintf(fname, "%s.parse.C", argv[1]);
	if (rename(fname, nname) == -1) {
		fprintf(stderr, "Cannot rename .parse.C to %s\n", nname);
		exit(1);
	}

	// open all *R files and read basic data from them

	sprintf(fname, "%s.R", argv[1]);
	R = fopen(fname, "r+"); // output .R file
	if (R == NULL)
	{
		fprintf(stderr, "Cannot open %s\n", fname);
		exit(1);
	}

	sprintf(fname, "%s.dicz.int.R", argv[1]);
	diczR = fopen(fname, "r");
	if (diczR == NULL)
	{
		fprintf(stderr, "Cannot open %s\n", fname);
		exit(1);
	}
	e = fread(&terms, sizeof(int), 1, diczR);
	die(e != 1, "Read error");
	stat(fname, &s);
	rules = (s.st_size - sizeof(int)) / (2 * sizeof(int));


	int alpha;
	int recursiveDiczRClen;
	int Rlen;
	e = fread(&alpha, sizeof(int), 1, R); //# of terminals in P_1 | number of words/phrases in D_1 | alpha in ipostprocDbg
	die(e != 1, "Read error");
	e = fread(&recursiveDiczRClen, sizeof(int), 1, R);
	die(e != 1, "Read error");
	e = fread(&Rlen, sizeof(int), 1, R);
	die(e != 1, "Read error");

	/*fprintf(stderr, "terms: %d\n", terms);
	fprintf(stderr, "rules: %d\n", rules);
	fprintf(stderr, "old alpha: %d\n", alpha);
	fprintf(stderr, "v256: %d\n", v256);
	fprintf(stderr, "terms-v256: %d\n", terms - 256);*/


	// shift all rules of diczR, removing terminals in 256..terms-1
	// leaves cursor at the end of R, ready to add more rules

	transl = (int*)malloc((alpha + 1) * sizeof(int));
	if ((transl == NULL) && (alpha > 0))
	{
		fprintf(stderr, "Cannot allocate %li bytes for the phrases of %s.dicz.int.R\n",
			alpha * sizeof(int), argv[1]);
		exit(1);
	}

	if (fseek(R, 0, SEEK_END) != 0) {
		fprintf(stderr, "fseek failed");
		exit(1);
	}
	//fprintf(stderr, "start: %ld\n", ftell(R) / 4);

	//fwrite(&v256, sizeof(int), 1, R); // could be better, we assume 256 terminals 0..255
	for (i = 0; i < rules; i++)
	{
		e = fread(val, sizeof(int), 2, diczR);
		die(e != 2, "Read error");
		if (val[0] >= 256) val[0] -= terms - 256 - 2 * Rlen;			//terms - 256 = phrases = # of phrases in dicz = terminals in parseR
		if (val[1] >= 256) val[1] -= terms - 256 - 2 * Rlen;
		fwrite(val, sizeof(int), 2, R);
	}
	fclose(diczR);

	// read diczC in memory, identify the terms and create balanced rules on them
	// adding those to R and remembering the mapping phrase -> top nonterm in transl

	sprintf(fname, "%s.dicz.int.C", argv[1]);
	diczC = fopen(fname, "r");
	if (diczC == NULL)
	{
		fprintf(stderr, "Cannot open %s\n", fname);
		exit(1);
	}
	stat(fname, &s);
	sizeC = s.st_size / sizeof(int);
	AdiczC = (int*)malloc(s.st_size);
	//fprintf(stderr, "sizeC: %d\n", sizeC);
	if (AdiczC == NULL)
	{
		fprintf(stderr, "Cannot allocate %li bytes for %s.dicz.int.C\n", s.st_size, argv[1]);
		exit(1);
	}
	e = fread(AdiczC, sizeof(int), sizeC, diczC);
	die(e != sizeC, "Read error");

	fclose(diczC);

	//fprintf(stderr, "diczC start: %d\n", ftell(R) / 4);
	i = 0; p = 1; int diczCexpLen = 0; int start = ftell(R);
	while (i < sizeC)
	{
		size_t j = i;
		size_t ni;
		while ((AdiczC[j] < 256) || (AdiczC[j] >= terms))	//moves the cursor up to the end of a dicz phrase = terminator bc dicz.int
		{
			if (AdiczC[j] >= 256) AdiczC[j] -= terms - 256;	//shift rule nonterminals 
			j++;
		}
		ni = j + 1;											//ni = next phrase start position, skip terminator
		while (j - i > 1)
		{
			size_t k = i;
			size_t ko = i;
			while (k + 1 < j)
			{
				val[0] = AdiczC[k++]; val[1] = AdiczC[k++];
				if (val[0] >= 256)
					val[0] += 2 * Rlen;
				if (val[1] >= 256)
					val[1] += 2 * Rlen;
				fwrite(val, sizeof(int), 2, R);
				diczCexpLen += 2;
				AdiczC[ko++] = 256 + rules++;
			}
			if (k < j) AdiczC[ko++] = AdiczC[k];
			j = ko;
		}
		transl[p++] = AdiczC[i];
		i = ni;
	}
	/*fprintf(stderr, "end: %d\n", ftell(R) / 4);
	fprintf(stderr, "rules: %d\n", rules);
	fprintf(stderr, "p: %d\n", p);*/
	fseek(R, start, SEEK_SET);
	/*fprintf(stderr, "diczC start: %d\n", ftell(R) / 4);
	fprintf(stderr, "diczCexpLen: %d\n", diczCexpLen);
	fprintf(stderr, "diczC start + diczCexpLen: %d\n", ftell(R) / 4 + diczCexpLen);*/

	//fprintf(stderr, "end: %d\n", ftell(R) / 4);
	for (i = 0; i < p; i++) {
		/*if (transl[i] < 256)
			fprintf(stderr, "%d\n", transl[i]);*/
		if (transl[i] >= 256)
			transl[i] += 2 * Rlen;
	}
	free(AdiczC);

	// translate the rules in .parse.R according to table transl

	fseek(R, 3 * sizeof(int), SEEK_SET);
	//fprintf(stderr, "position: %d\n", ftell(R) / 4);
	//fprintf(stderr, "recursiveDiczRClen: %d\n", recursiveDiczRClen);
	for (i = 0; i < recursiveDiczRClen; i++)
	{
		e = fread(val, sizeof(int), 2, R);
		die(e != 2, "Read error");
		if (val[0] < alpha) val[0] = transl[val[0]];   // terminal in the parse
		if (val[1] < alpha) val[1] = transl[val[1]];
		fseek(R, -2 * sizeof(int), SEEK_CUR);
		fwrite(val, sizeof(int), 2, R);
	}
	//fprintf(stderr, "position = 122111?: %d\n", ftell(R) / 4);
	assert(ftell(R) == (3 + 2 * (recursiveDiczRClen)) * sizeof(int));

	for (i = 0; i < Rlen - recursiveDiczRClen; i++) {
		e = fread(val, sizeof(int), 2, R);
		die(e != 2, "Read error");
		if (val[0] < alpha) val[0] = transl[val[0]];   // terminal in the parse
		if (val[1] < alpha) val[1] = transl[val[1]];
		fseek(R, -2 * sizeof(int), SEEK_CUR);
		fwrite(val, sizeof(int), 2, R);
	}
	assert(ftell(R) == (3 + 2 * (Rlen)) * sizeof(int));

	/*int max = 0;
	int min = 99999999;
	for (i = 0; i < rules; i++) {
		e = fread(val, sizeof(int), 2, R);
		die(e != 2, "Read error");
		if (val[0] > max) max = val[0];
		if (val[1] > max) max = val[1];
		if (val[0] > 255) {
			if (val[0] < min) { min = val[0]; }
		}
		if (val[1] > 255) {
			if (val[1] < min) { min = val[1]; }
		}
	}
	fprintf(stderr, "max: %d\n", max);
	fprintf(stderr, "min: %d\n", min);

	fseek(R, 0, SEEK_END);
	fprintf(stderr, "end?: %d\n", ftell(R) / 4);*/
	if (fclose(R) != 0) {
		fprintf(stderr, "Cannot close %s.R\n", argv[1]); exit(1);
	}

	sprintf(fname, "%s.C", argv[1]);
	C = fopen(fname, "r+");
	stat(fname, &s);
	int lenC = (s.st_size / sizeof(int));
	//fprintf(stderr, "lenC: %d\n", lenC);

	//readable outputs commented out
	/*sprintf(fname, "%s.Creadable", argv[1]);
	FILE* Creadable = fopen(fname, "w");
	int upto10 = 0;*/
	//int count = 0;

	for (i = 0; i < lenC; i++)
	{
		e = fread(val, sizeof(int), 1, C);
		die(e != 1, "Read error");
		//fprintf(Creadable, "%d | ", val[0]);
		if (val[0] < alpha) {
			//fprintf(stderr, "%d| ", val[0]);
			//count++;
			val[0] = transl[val[0]];   // terminal in parse replace with grammar (pfp dict)
			//fprintf(stderr, "%d\n", val[0]);
		}
		fseek(R, -1 * sizeof(int), SEEK_CUR);
		fwrite(val, sizeof(int), 1, C);

		//make readable C file
		/*fprintf(Creadable, "%d, ", val[0]);
		if (upto10 >= 5) {
			fprintf(Creadable, "\n");
			upto10 = 0;
		}
		upto10++;*/
	}
	//fprintf(stderr, "<alpha: %d\n", count);
	free(transl);

	fprintf(stderr, "Prefix-Free + Repair succeeded\n");

	// get orginal size by measuring number of bytes in the original input
	/*if (stat(argv[1], &s) != 0) {
		fprintf(stderr, "Cannot stat original input file %s\n", argv[1]);
		fprintf(stderr, "Compression ratio estimate not available\n");
		fprintf(stdout, "  Estimated output size (stdout): NaN\n"); // don't change this: est_size must be the the last printed item 
		exit(2);
	}*/

	// estimate compression
	// the estimate of the output size is done assuming we use
	// a complete binary tree with 2r nodes to encode r rules.
	// So we have:
	//  1 bit per node (total 2r bits) to describe the shape of the binary tree
	//     we do not explicitly represent non terminal (internal nodes)
	//     we identify them in C with their preorder rank  
	//  log(alpha+r) bits for each symbol in C and each leaf in the
	//               binary tree, total log(alpha+r)(C+r), here alpha=256 
	//               actually we could use just r*log(alpha) bits for the leaves
	//               since they are non terminal  
	rules += prules; // final number of rules
	long est_size = (long)((2.0 * rules + ((double)bits((size_t)256 + rules)) * (rules + psizeC)) / 8) + 1;
	//fprintf(stderr, "  Original file size: %li (bytes)\n", s.st_size);
	fprintf(stderr, "  Number of rules: %li\n", rules);
	fprintf(stderr, "  Final sequence length: %li (integers)\n", psizeC);
	fprintf(stderr, "  Estimated output size (bytes): %ld\n", est_size);
	//fprintf(stderr, "  Compression ratio: %0.2f%%\n", (100.0 * est_size) / s.st_size);
	fprintf(stdout, "  Estimated output size (stdout): %ld\n", est_size); // don't change this: est_size must be the the last printed item 
	fprintf(stderr, "=== postprocessing completed!\n");
	return 0;
}
