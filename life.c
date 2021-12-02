#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // sleep()関数を使う
//#include "gol.h"

void my_init_cells(const int height, const int width, int cell[height][width], FILE* fp);
void my_print_cells(FILE* fp, int gen, const int height, const int width, int cell[height][width]);
void my_update_cells(const int height, const int width, int cell[height][width]);
int my_count_adjacent_cells(int h, int w, const int height, const int width, int cell[height][width]);

int main(int argc, char **argv)
{
  FILE *fp = stdout;
  const int height = 40;
  const int width = 70;

  int cell[height][width];
  for(int y = 0 ; y < height ; y++){
    for(int x = 0 ; x < width ; x++){
      cell[y][x] = 0;
    }
  }

  /* ファイルを引数にとるか、ない場合はデフォルトの初期値を使う */
  if ( argc > 2 ) {
    fprintf(stderr, "usage: %s [filename for init]\n", argv[0]);
    return EXIT_FAILURE;
  }
  else if (argc == 2) {
    FILE *lgfile;
    if ( (lgfile = fopen(argv[1],"r")) != NULL ) {
      my_init_cells(height,width,cell,lgfile); // ファイルによる初期化
    }
    else{
      fprintf(stderr,"cannot open file %s\n",argv[1]);
      return EXIT_FAILURE;
    }
    fclose(lgfile);
  }
  else{
    my_init_cells(height, width, cell, NULL); // デフォルトの初期値を使う
  }

  my_print_cells(fp, 0, height, width, cell); // 表示する
  sleep(1); // 1秒休止

  /* 世代を進める*/
  for (int gen = 1 ;; gen++) {
    my_update_cells(height, width, cell); // セルを更新
    my_print_cells(fp, gen, height, width, cell);  // 表示する
    sleep(1); //1秒休止する
    fprintf(fp,"\e[%dA",height+3);//height+3 の分、カーソルを上に戻す(壁2、表示部1)
  }

  return EXIT_SUCCESS;
}

//ファイルによるcellの初期化
void my_init_cells(const int height, const int width, int cell[height][width], FILE* fp){
  if(fp == NULL){
    cell[20][30] = 1;
    cell[22][30] = 1;
    cell[22][31] = 1;
    cell[23][31] = 1;
    cell[20][32] = 1;
  }else{
    //#Life 1.06が先頭かどうか
    char h[30];
    char* header = "#Life 1.06";
    fscanf(fp,"%20[^\n]%*[^\n]",h);
    if(strcmp(h,header)!=0){
      fprintf(stderr,"invalid file format\n");
    }

    //ファイル読み取り
    int x = 0;
    int y = 0;
    int c1 = fscanf(fp,"%d%*C",&x);
    int c2 = fscanf(fp,"%d%*C",&y);
    while(c1+c2 == 2){
      if(x<0 || x>=width || y<0 || y>=height){
        fprintf(stderr,"invalid file format\n");
      }
      printf("%d, %d\n",x,y);
      cell[y][x] = 1;
      c1 = fscanf(fp,"%d%*C",&x);
      c2 = fscanf(fp,"%d%*C",&y);
    }
  }
}

//表示
void my_print_cells(FILE* fp, int gen, const int height, const int width, int cell[height][width]){
  fprintf(fp, "generation = %d\n",gen);
  fprintf(fp,"+");
  for(int i=0 ; i<width ; i++){
    fprintf(fp,"-");
  }
  fprintf(fp,"+\n");
  for(int i=0 ; i<height ; i++){
    fprintf(fp,"|");
    for(int j=0 ; j<width ; j++){
      if(cell[i][j] == 1){
        fprintf(fp, "\x1b[31m");
        fprintf(fp, "#");
        fprintf(fp, "\x1b[39m");
      }else{
        fprintf(fp, " ");
      }
    }
    fprintf(fp, "|\n");
  }
  fprintf(fp,"+");
  for(int i=0 ; i<width ; i++){
    fprintf(fp,"-");
  }
  fprintf(fp,"+\n");
}

//セルの更新
void my_update_cells(const int height, const int width, int cell[height][width]){
  int cell2[height][width];
  for(int i=0 ; i<height ; i++){ //y
    for(int j=0 ; j<width ; j++){ //x
      int ex = my_count_adjacent_cells(i, j, height, width, cell);
      if(cell[i][j]==0){
        if(ex==3){
          cell2[i][j] = 1;
        }else{
          cell2[i][j] = 0;
        }
      }else{
        if(ex==2 || ex==3){
          cell2[i][j] = 1;
        }else{
          cell2[i][j] = 0;
        }
      }
    }
  }
  for(int i=0 ; i<height ; i++){
    for(int j=0 ; j<width ; j++){
      cell[i][j] = cell2[i][j];
    }
  }
}

//周辺の生きたセルをカウント
int my_count_adjacent_cells(int h, int w, const int height, const int width, int cell[height][width]){
  int d[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
  int rx = 0;
  int ry = 0;
  int ex = 0;
  for(int k=0 ; k<8 ; k++){
    rx = w+d[k][1];
    ry = h+d[k][0];
    if(ry>0 && ry<height && rx>0 && rx<width){
      if(cell[ry][rx]==1){
         ex++;
      }
    }
  }
  return ex;
}
