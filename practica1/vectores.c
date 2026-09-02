#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int main(){
  int A[10];
  int B[10];
  int C[10];

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

  pid_t pid_par = fork();
 
  if(pid_par == 0 ){
    for(int j = 0; j < 10; j= j + 2){
      int suma = A[j] + B[j];
      C[j] = suma;
      fprintf(Cfile, "%d,\n", C[j]);
      fflush(Cfile);
    }
    exit(0);
  }

  pid_t pid_impar = fork();
  if(pid_impar == 0){
    for(int j = 1; j < 10; j= j + 2){
      int suma = A[j] + B[j];
      C[j] = suma;
      fprintf(Cfile, "%d,\n", C[j]);
      fflush(Cfile);
    }
    exit(0);
  }


  rewind(Cfile);
  var = 0;
  while (fscanf(Cfile, "%d", &var) == 1){
    printf("%d,", var);
  }
  printf("\n");
  for(int j = 0; j < 10; j++){
    printf("%d,", C[j]);
  }

  fclose(Cfile);
  return 0;
}
