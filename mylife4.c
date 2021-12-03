#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // sleep()関数を使う
#include <time.h>
#include <math.h>
//#include "gol.h"

void my_init_cells(const int height, const int width, int cell[height][width], FILE* fp);
void my_rle_cells(const int height, const int width, int cell[height][width], FILE* fp);
void my_print_cells(FILE* fp, int gen, const int height, const int width, int cell[height][width]);
void my_update_cells(const int height, const int width, int cell[height][width]);
int my_count_adjacent_cells(int h, int w, const int height, const int width, int cell[height][width]);
void make_cells(const int height, const int width, int cell[height][width]);
int count_cells(const int height, const int width, int cell[height][width]);
void make_files(const int height, const int width, int gen, int cell[height][width]);
int ctoi(char c);
void d_and_b(const int height, const int width, int cell[height][width], int random1, int random2, int random3, int random4);
void warp_and_larger(const int height, const int width, int cell[height][width]);
void secret_print_cells(FILE* fp, const int height, const int width, int cell[height][width]);

int main(int argc, char **argv){
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
    }else if (argc == 2) {
        FILE *lgfile;
        if ( (lgfile = fopen(argv[1],"r")) != NULL ) {
            my_init_cells(height,width,cell,lgfile); // ファイルによる初期化
        }else{
            fprintf(stderr,"cannot open file %s\n",argv[1]);
            return EXIT_FAILURE;
        }
        fclose(lgfile);
    }else{
        my_init_cells(height, width, cell, NULL); // デフォルトの初期値を使う
    }
    long stop = 100000;
    my_print_cells(fp, 0, height, width, cell); // 表示する
    usleep(stop); // 休止
    fprintf(fp,"\e[%dA",height+3);//height+3 の分、カーソルを上に戻す(壁2、表示部1)
    
    // ランダムな何かを行う起点
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    srand(start.tv_nsec);

    /* 世代を進める*/
    for (int gen = 1 ;; gen++) {
        my_update_cells(height, width, cell); // セルを更新
        my_print_cells(fp, gen, height, width, cell);  // 表示する
        usleep(stop); //休止する
        fprintf(fp,"\e[%dA",height+3);//height+3 の分、カーソルを上に戻す(壁2、表示部1)

        int check = rand()%1000;
        if(check>=100 && check<=300){
            d_and_b(height, width, cell, rand(), rand(), rand(), rand());
            my_print_cells(fp, gen, height, width, cell);
            usleep(stop);
            fprintf(fp,"\e[%dA",height+3);//height+3 の分、カーソルを上に戻す(壁2、表示部1)
        }else if(check==10 || check==323 || check==457 || check==753 || check==960){
            secret_print_cells(fp, height, width, cell);
            sleep(1);
            fprintf(fp,"\e[%dA",height+3);//height+3 の分、カーソルを上に戻す(壁2、表示部1)
            warp_and_larger(height, width, cell);
            my_print_cells(fp, gen, height, width, cell);
            usleep(stop);
            fprintf(fp,"\e[%dA",height+3);//height+3 の分、カーソルを上に戻す(壁2、表示部1)
        }
        //ファイルへの出力はこのファイルでは邪魔なため止めている。
        /* 
        if(gen%100 == 0 && gen<10000){
            make_files(height, width, gen, cell);
        }*/
    }
    return EXIT_SUCCESS;
}
//特殊表示
void secret_print_cells(FILE* fp, const int height, const int width, int cell[height][width]){
    fprintf(fp, "generation = ???   ????%% of the cells are alive\n");
    fprintf(fp,"+");
    for(int i=0 ; i<width ; i++){
        fprintf(fp,"-");
    }
    fprintf(fp,"+\n");
    for(int i=0 ; i<height ; i++){
        fprintf(fp,"|");
        for(int j=0 ; j<width ; j++){
            fprintf(fp,"\x1b[31m");
            fprintf(fp, "?");
            fprintf(fp,"\x1b[39m");
        }
        fprintf(fp, "|\n");
    }
    fprintf(fp,"+");
    for(int i=0 ; i<width ; i++){
        fprintf(fp,"-");
    }
    fprintf(fp,"+\n");
}

//ワープ
void warp_and_larger(const int height, const int width, int cell[height][width]){
    int e = count_cells(height, width, cell);
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    srand(start.tv_nsec);
    int x = 0;
    int y = 0;
    for(int i=0 ; i<height ; i++){
        for(int j=0 ; j<width ; j++){
            cell[i][j] = 0;
        }
    }
    while(e>0){
        x = rand()%width;
        y = rand()%height;
        if(cell[y][x]==0){
            cell[y][x] = 1;
            e--;
        }
    }
    int d[3][2] = {{-1,0},{-1,-1},{0,-1}};
    int rx,ry;
    for(int i=0 ; i<height ; i++){
        for(int j=0 ; j<width ; j++){
            if(cell[i][j]==0){
                continue;
            }
            for(int k=0 ; k<3 ; k++){
                ry = i+d[k][0];
                rx = j+d[k][1];
                if(ry>=0 && ry<height && rx>=0 && rx<width){
                    cell[ry][rx] = 1;
                }
            }
        }
    }

}

//確率的死亡、誕生
void d_and_b(const int height, const int width, int cell[height][width], int random1, int random2, int random3, int random4){
    int x = random1 % width; //起点x座標
    int y = random2 % height; //起点y座標
    int rmax = random3%5 + 1; //範囲
    int z = 5; //パターン数
    if(random4%z==0){ //円形
        for(int i=y-rmax ; i<=y+rmax ; i++){
            if(i<0 || i>=height){
                continue;
            }
            for(int j=x-rmax ; j<=x+rmax ; j++){
                if(j<0 || j>=width){
                    continue;
                }
                if(sqrt((double)(i-y)*(i-y)+(j-x)*(j-x))<rmax){
                    cell[i][j] = -100;
                }
            }
        } 
    }else if(random4%z==1){ //十字
        for(int i=y-rmax ; i<=y+rmax ; i++){
            if(i<0 || i>=height){
                continue;
            }
            cell[i][x] = -100;
        }
        for(int j=x-rmax ; j<=x+rmax ; j++){
            if(j<0 || j>=width){
                continue;
            }
            cell[y][j] = -100;  
        }
    }else if(random4%z==2){ //X字
        for(int i=0 ; i<=rmax ; i++){
            if(x-i>=0 && y-i>=0){
                cell[y-i][x-i] = -100;
            }
            if(x+i<width && y-i>=0){
                cell[y-i][x+i] = -100;
            }
            if(x-i>=0 && y+i<height){
                cell[y+i][x-i] = -100;
            }
            if(x+i<width && y+i<height){
                cell[y+i][x+i] = -100;
            }
        }
    }else if(random4%z==3){ //円
        for(int i=-rmax ; i<=rmax ; i++){
            if(y+i<0 || y+i>=height){
                continue;
            }
            int j = sqrt(rmax*rmax - i*i);
            if(x+j>=0 && x+j<width){
                cell[y+i][x+j] = 100;
            }
            j *= -1;
            if(x+j>=0 && x+j<width){
                cell[y+i][x+j] = 100;
            }
        }  
    }
    
}

//100世代ごとのファイルへの出力
void make_files(const int height, const int width, int gen, int cell[height][width]){
    FILE* fp2;
    char* name = (char*) malloc(sizeof(char)* 12);
    name[0] = 'g';
    name[1] = 'e';
    name[2] = 'n';
    name[3] = '0';
    name[4] = '0';
    name[5] = '0';
    name[6] = '0';
    name[7] = '.';
    name[8] = 'l';
    name[9] = 'i';
    name[10] = 'f';
    name[11] = '\0';
    int num = gen/100;
    int f = num/10;
    int s = num%10;
    name[3] = '0'+f;
    name[4] = '0'+s;
    
    fp2 = fopen(name, "w");
    if(fp2==NULL){
        printf("Cannot make %s\n",name);
        return;
    }
    free(name);
    fprintf(fp2, "#Life 1.06\n");
    for(int i=0 ; i<height ; i++){
        for(int j=0 ; j<width ; j++){
            if(cell[i][j]==1){
                fprintf(fp2,"%d %d\n",j,i);
            }
        }   
    }
    fclose(fp2);
    
}

//ファイルによるcellの初期化
void my_init_cells(const int height, const int width, int cell[height][width], FILE* fp){
    if(fp == NULL){
        make_cells(height, width, cell);
    }else{
        //#Life 1.06が先頭かどうか
        char h[30];
        char* header = "#Life 1.06";
        fscanf(fp,"%20[^\n]%*[^\n]",h);
        if(strcmp(h,header)!=0){
            rewind(fp);
            my_rle_cells(height, width, cell, fp);
        }else{
            //ファイル読み取り
            int x = 0;
            int y = 0;
            int c1 = fscanf(fp,"%d%*C",&x);
            int c2 = fscanf(fp,"%d%*C",&y);
            while(c1+c2 == 2){
                if(x<0 || x>=width || y<0 || y>=height){
                    fprintf(stderr,"invalid file format\n");
                }
                cell[y][x] = 1;
                c1 = fscanf(fp,"%d%*C",&x);
                c2 = fscanf(fp,"%d%*C",&y);
            }
        }
        
    }
}

//charをintとして返す関数
int ctoi(char c){
    if(c>='0' && c<='9'){
        return c-'0';
    }
    return -1;
}

//RLE形式の読み取り
void my_rle_cells(const int height, const int width, int cell[height][width], FILE* fp){
    int l = height*(width+1);
    char s[l];
    char* p;
    int count = 0;
    //どこまでが#始まりの行かを探す
    while(1){
        p = fgets(s,l,fp);
        if(p==NULL){
            printf("invalid format of file");
            return;
        }
        count++;
        if(s[0]=='#'){
            continue;
        }else{
            break;
        }
    }
    //ファイル先頭へ
    rewind(fp);
    for(int i=0 ; i<count-1 ; i++){
        p = fgets(s,l,fp);
    }
    int x = 0;
    int y = 0;
    fscanf(fp,"x = %d, y = %d", &x, &y); //実質使わない
    if(x<0 || x>width || y<0 || y>height){
        printf("invalid file format");
        return;
    }
    fscanf(fp,"%*[^\n]%*c");
    //ここで目的の内容に到達
    x = 0;
    y = 0;
    while((fscanf(fp, "%[^$]%*c",s))==1){ //行ごと
        x = 0;
        int L = strlen(s);
        while(1){
            int z = 0;
            int w = 0;
            for(int i=0 ; i<L ; i++){
                if(s[i]=='\n'){
                    z = 1;
                    w = i;
                    break;
                }
            }
            if(z==0){
                break;
            }else{
                for(int i=w+1 ; i<L ; i++){
                    s[i-1] = s[i]; 
                }
                L = strlen(s);
                s[L-1] = '\0';
            }
        }
        int num = 0;
        int i = 0;
        L = strlen(s);
        while(i<L){
            num = 1;
            if(i==L-1){
                if(s[i]=='!'){
                    break;
                }else{
                    if(s[i]=='b'){
                        cell[y][x] = 0;
                    }else{
                        cell[y][x] = 1;
                    }
                    x++;
                    i++;
                }
            }else{
                if(ctoi(s[i])==-1){
                    if(s[i]=='b'){
                        cell[y][x] = 0;
                    }else{
                        cell[y][x] = 1;
                    }
                    i++;
                    x++;
                }else{
                    if(ctoi(s[i+1])==-1){
                        num = ctoi(s[i]);
                        for(int j=0 ; j<num ; j++){
                            if(s[i+1]=='b'){
                                cell[y][x] = 0;
                            }else{
                                cell[y][x] = 1;
                            }
                            x++;
                        }
                        i += 2;
                    }else{
                        num = ctoi(s[i])*10+ctoi(s[i+1]);
                        for(int j=0 ; j<num ; j++){
                            if(s[i+2]=='b'){
                                cell[y][x] = 0;
                            }else{
                                cell[y][x] = 1;
                            }
                            x++;
                        }
                        i += 3;
                    }
                }
            }
        }
        y++;
    }

}

//ランダム初期化
void make_cells(const int height, const int width, int cell[height][width]){
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    srand(start.tv_nsec);
    double numd = 0;
    for(int i=0 ; i<height ; i++){
        for(int j=0 ; j<width ; j++){
            numd = (double) rand()/RAND_MAX;
            if(numd<0.1){
                cell[i][j] = 1;
            }
        }
    }
    
}

//生きているcellのカウント
int count_cells(const int height, const int width, int cell[height][width]){
    int ex = 0;
    for(int i=0 ; i<height ; i++){
        for(int j=0 ; j<width ; j++){
            if(cell[i][j]==1){
                ex++;
            }
        }
    }
    return ex;
}

//表示
void my_print_cells(FILE* fp, int gen, const int height, const int width, int cell[height][width]){
    fprintf(fp, "generation = %d",gen);
    int live = count_cells(height,width,cell);
    double per = (double) live /(height*width) * 100;
    fprintf(fp, "   %.2lf%% of the cells are alive\n", per);
    fprintf(fp,"+");
    for(int i=0 ; i<width ; i++){
        fprintf(fp,"-");
    }
    fprintf(fp,"+\n");
    for(int i=0 ; i<height ; i++){
        fprintf(fp,"|");
        for(int j=0 ; j<width ; j++){
            if(cell[i][j] == 1){
                fprintf(fp, "\x1b[32m");
                fprintf(fp, "#");
                fprintf(fp, "\x1b[39m");
            }else if(cell[i][j]==-100){
                fprintf(fp, "\x1b[33m");
                fprintf(fp, "*");
                fprintf(fp, "\x1b[39m");
            }else if(cell[i][j]==100){
                fprintf(fp, "\x1b[36m");
                fprintf(fp, "#");
                fprintf(fp, "\x1b[39m");
                cell[i][j] = 1;
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
        if(ry>=0 && ry<height && rx>=0 && rx<width){
            if(cell[ry][rx]==1){
                ex++;
            }
        }
    }
    return ex;
}

