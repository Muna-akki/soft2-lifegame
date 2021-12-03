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
まず、下準備としてcharをintに変換する関数ctoi()を設定する。
```c
int ctoi(char c){
    if(c>='0' && c<='9'){
        return c-'0';
    }
    return -1;
}
```
charが規則正しく割り振られていることを利用して、'0', '9'からのオフセットを元に'3'などのcharを(int)3として返すようにしている。引数のcharが一桁の数字として表せない場合は一律で-1を返すようにする。

RLE形式も読めるようにするために、my_init_cells()内において、引数として与えられたファイルがLife1.06形式でないことが判明した場合(先頭の行が"#Life 1.06"でない場合)、my_rle_cells()を呼び出すように変更。この際、rewind(fp)によりファイル位置を先頭に戻している。
```c
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
```
[RLE形式についての説明](https://www.conwaylife.com/wiki/Run_Length_Encoded)によると、ファイル開始後数行は#で始まる、ファイルについての説明を記した行となりうる。そのため、ここまでの部分で何行目までが#で始まる行なのかを探してcountに記録している。
```c
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
```
一旦ファイル位置を先頭に戻した後、この部分でオブジェクトサイズまでを取得(オブジェクトサイズの形式上、行ごとの読み込みは直前で止め、形式を指定して読み込んでいる)。#で始まる行は統一的な処理が不可能なので無視して、もしオブジェクトサイズが既にある盤面に対して不正な値だった場合盤面の初期化を中断する。ここまで進めると、次に読み込むファイル内容からが盤面の初期状態を表すことになる。
```c
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
```
この部分でRLE形式の本格的な読み込みを始めている。この形式では行の区切りを表す文字が"$"なのでそこで入力を止め、行の状態を表す文字列をsに入れて行ごとに盤面設定に入る。なお、前述のRLE形式についての説明のサイトの例では、盤面を表す文字列の途中に改行が入る例が示されていた(これがかなりのバグの元であった)ため、改行文字を文字列s中から取り除く処理も同時に行っている。ここで、x,yは盤面のうちどこまでをファイルをもとに更新したかを表し、Lはsの長さを表す。
```c
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

```
この部分では、行の状態を表す文字列を解読している。sのi文字目について、iがsの最終文字ならば、sが最終行を表す場合'!'なのでそのままループを抜ける。最終行でない場合それは'b'(死)や'o'(生)など生死を表す文字のはずなのでそのまま盤面の更新に用いる。iがsの最終文字でない場合は、その文字がintとして解釈できるかを判定する。intとして解釈できない場合、それは'b'や'o'のはずなのでそのまま盤面の更新に用いる。intとして解釈できる場合、i+1文字目がintとして解釈できるかどうかを考える。i+1文字目がintとして解釈できない場合、(i+1文字目)が(i個)という意味になるのでそのように盤面を更新する。i+1文字目がintとして解釈できる場合、「行の終わりに数字が来ることはない」という性質からi+2文字目もその行に存在するはずである。また、盤面の設定上widthは2桁なのでi+2文字目は必ず文字。したがって、(i+2文字目)が((i文字目)*10+(i+1文字目))個という意味になるのでそのように盤面を更新する。

この際、何文字分行を読み進めたかに応じてiを更新し、何マス分書き進めたかに応じてxを更新する。ある行についてsを読み終わったらyを加算、xを0に戻してまた次の行について同様の操作を繰り返す。
## 課題4(mylife4.c)