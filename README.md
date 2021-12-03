# soft2-lifegame

## 共通部(life.c)
my_init cells()とmy_print_cells()、my_update_cells()、my_count_adjacent_cells()を使用の要求通りに実装。
my_count_adjacent_cells()では以下のように、
```c
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
```
8方向全てを分けて書くのが面倒だったので周囲8マスに対応する座標変化をd[8][2]に格納、そのマスの座標をrx, ryで表すことでforループ内で統一的に盤面内かどうかを判定できるようにした。

## 課題1(mylife1.c)
まず、引数がない場合の初期配置をランダムにするために、my_init_cells()において引数fpがNULLの時make_cells()を呼び出すように変更した。
```c
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
```
この関数内部では現在時刻を取得し、ランダム関数の種に与えている。そして、盤面内全てのマスに対してrand()/RAND_MAXにより0~1の数字を与え、その数字が0.1以下だった時にその細胞が生きている初期状態を与えている。こうすることで初期状態において全体の10％程度の細胞が生きているようにできる。

また、各世代で存在するセルの比率を表示するためにcount_cells()関数を定義し、my_print_cells()内から呼び出すようにした。count_cells()関数は引数の配列の全ての要素をチェックし、生きている細胞の総数を返すだけの関数である。
## 課題2(mylife2.c)
100世代ごとに盤面の状態をファイルに出力するため、main関数内の世代を進めるforループの中からmake_files()関数を100世代ごと(gen%100==0 && gen<10000)に呼び出すように変更した。
```c
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
```
コードを書いている途中で訳のわからないバグが発生した結果、解決策としてファイル名を持つchar* nameとしてmalloc()で確保し、ファイルポインタ生成直後にfree(name)している。numは世代番号の上2桁、fは最上位の桁、sは2桁目であり、これをname内の適切な位置に入れることでファイル名を確定させる。ファイル名が決まった後はFILE* fp2を作り、盤面の状態を指定された形式で書き込んでいる。

## 課題3(mylife3.c)
my_init_cells()内において、引数として与えられたファイルが"Life1.06"形式でないことが判明した場合(先頭の行が"#Life 1.06"でない場合)、my_rle_cells()を呼び出すように変更。

## 課題4(mylife4.c)