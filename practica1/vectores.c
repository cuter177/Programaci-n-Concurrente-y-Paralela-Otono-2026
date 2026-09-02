#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int main(){
  int n = 10;
  int A[n];
  int B[n];
  int C[n];

  FILE *Afile, *Bfile, *Cfile;
  Afile = fopen("A.txt", "r");
  Bfile = fopen("B.txt", "r");

 if(Afile == NULL || Bfile == NULL){
    printf("Error abriendo archivos\n");
    exit(1);
  }

  int var, i = 0;
  while(fscanf(Afile, "%d", &var) == 1){
    A[i] = var;
    i = i + 1;
  }
  fclose(Afile);

  i = 0;
  while(fscanf(Bfile, "%d", &var) == 1){
    B[i] = var;
    i = i + 1;
  }
  fclose(Bfile);

  Cfile = fopen("C.txt", "w+");
  if(Cfile == NULL){
    printf("Error abriendo C.txt\n");
    exit(1);
  }

  for(int j = 0; j < n; j++){
    pid_t pid = fork();
    if(pid == 0){
      C[j] = A[j] + B[j];
      printf("Proceso hijo %d: %d + %d = %d\n", getpid(), A[j], B[j], C[j]);
      fprintf(Cfile, "%d\n", C[j]);
      fflush(Cfile);
      exit(0);
    }

  }
  
  printf("Vector C:\n");
  rewind(Cfile);
  while(fscanf(Cfile, "%d", &var) == 1){
    printf("%d,", var);
  }
  printf("\n");
    
  fclose(Cfile);
  return 0;
}
