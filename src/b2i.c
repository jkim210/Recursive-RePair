#include <stdlib.h>
#include <stdio.h>


// create an output file containing the same bytes as the input file
// but replacing each byte by a int

int main (int argc, char **argv)
{ 
  char foname[1024];
  FILE *fi,*fo;
  int c,i;

  if(argc!=2) {
     printf("Usage: %s <filename>\n\n", argv[0]);
     exit(1);
  }

  puts("==== Command line:");
  for(i=0;i<argc;i++)
    printf(" %s",argv[i]);
  puts("\n");

  // open input file
  fi = fopen(argv[1],"r");
  if (fi == NULL) { 
    fprintf(stderr,"Cannot open file %s\n",argv[1]);
    exit(1);
  }
  // open integer output file
  sprintf(foname,"%s.int",argv[1]);
  fo = fopen(foname,"w");
  if (fo == NULL) { 
    fprintf(stderr,"Cannot create file %s\n",foname);
    exit(1);
  }
  
  // main loop 
  int n=0;
  while (1) {
      int e = fread(&c,sizeof(char),1,fi);
      if(e!=1) break;
      e = fwrite (&c,sizeof(int),1,fo); // write int to output file 
      if(e!=1) {perror("Error writing to .int file"); exit(1); }
      n++;
  }
  fclose(fo);
  fclose(fi);
  fprintf(stderr, "Done! %d ints written\n",n);
  return 0;
}

