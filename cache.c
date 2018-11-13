#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    int nk;
    int assoc;
    int bsize;
    char repl[1]; 
    int num_block;
    int set;
    int set_f;
    int offset_f;
    int tag_f;
} Info;

typedef struct {
    char rw[1];
    unsigned long long addr;
} Access;

int counting(int tag, int index, int set, int assoc, char *rw,
        int *rh, int *rm, int *wh, int *wm, int mem[set][assoc+1], int victim[set][assoc+1]) {

    int read_hit=0;
    int read_miss=0;
    int write_hit=0;
    int write_miss=0;

    for (int i=1; i<assoc+1; i++) {
        if ((victim[index][i] < 0) || (victim[index][i] > 64))
            victim[index][i] = 0;
    }
    // read : 114
    // write : 119
    int r = (int)rw[0];
    if ((mem[index][0] < 0) || (mem[index][0] > 64))
        mem[index][0] = 0; //每個set中資料個數

    // read operation
    if (r == 114) {
        for (int i=1; i < assoc+1; i++) {
            if (mem[index][i] == tag) {
                // 有被存取到的index  victim值 +1
                victim[index][i] += 1;
                read_hit++;
                *rh = read_hit;
                return 0;
            }
        }
        // fill in missing block
        int ind = 1;
        while (mem[index][0] <= assoc+1) {
            // 找空block
            if ((mem[index][ind] > 0) && (mem[index][ind] <= 1048064)) {
                mem[index][0] += 1;
                ind++;
                // 沒有空block => LRU
                if (ind >= assoc+1) {
                    int tmp=0;
                    int min=1;
                    //for (int i=1; i<assoc; i++) {
                    //quicksort(victim[index], 1, assoc);
                    for (int i=2; i<assoc+1; i++) {
                        //printf("victim[%d][%d] = %d\n", index,i,victim[index][i]);
                        //printf("victim[index][%d] = %d ",i,victim[index][i]);
                        //printf("victim[index][%d] = %d\n",i+1,victim[index][i+1]);

                        //printf("i : %d\n",i);
                        if (victim[index][min] > victim[index][i]) {
                            //printf("victim[index][%d] = %d ",i,victim[index][i]);
                            //printf("victim[index][%d] = %d\n",i+1,victim[index][i+1]);
                            min = i;
                            //tmp = victim[index][i];
                            //victim[index][i] = victim[index][i+1];
                            //victim[index][i+1] = tmp;
                        }
                    }
                    //printf("min = %d\n",victim[index][min]);
                    //printf("min = %d\n",min);
                    mem[index][victim[index][min]] = tag;
                    // 此block 被換掉 victim值歸零
                    victim[index][min] = 0;
                    read_miss++;
                    *rm = read_miss;
                    return 0;
                }
            }
            else { // 有空block 直接寫入
                //if (ind > assoc)
                    //printf("ind : %d\n",ind);
                mem[index][ind] = tag;
                //printf("ind : %d\n",ind);
                // 有被存取到的index  victim值 +1
                victim[index][ind] += 1;
                read_miss++;
                *rm = read_miss;
                return 0;
            }
        }
    }
    // write operation
    else {
        for (int i=1; i < assoc+1; i++) {
            if (mem[index][i] == tag) {
                // 有被存取到的index  victim值 +1
                victim[index][i] += 1;
                write_hit++;
                *wh = write_hit;
                return 0;
            }
        }
        int ind = 1;      //索引值
        while(mem[index][0] <= assoc+1) {  //set 還未滿
            // 找空block
            if ((mem[index][ind] > 0) && (mem[index][ind] <= 1048064)) {
            //if (mem[index][ind] > 0) {
                mem[index][0] += 1;
                ind++;
                // 沒有空block
                if (ind >= assoc+1) {
                    //printf("mem[%d][0] = %d\n", index,mem[index][0]);
                    // do LRU
                    int tmp=0;
                    int min=1;
                    //quicksort(victim[index], 1, assoc);
                    for (int i=2; i < assoc+1; i++) {
                        if (victim[index][min] > victim[index][i]) {
                            min = i;
                            //tmp = victim[index][i];
                            //victim[index][i] = victim[index][i+1];
                            //victim[index][i+1] = tmp;
                        }
                    }
                    //printf("min : %d\n",min);
                    mem[index][victim[index][min]] = tag;
                    // 此block被換掉 victim值歸零
                    victim[index][min] = 0;
                    //mem[index][ind-1] = tag;
                    write_miss++;
                    *wm = write_miss;
                    return 0;
                }
            }
            else {
                //if (ind > 64)
                    //printf("ind : %d\n",ind);
                // 有空block 直接寫入
                mem[index][ind] = tag; 
                // 有被存取到的index  victim值 +1
                victim[index][ind] += 1;
                write_miss++;
                *wm = write_miss;
                return 0;
            }
        }
    } 
    return 0;
}

int *simulator(Info info, Access as,int rh, int rm, int wh, int wm) {
    int nk = info.nk;            // cache size
    int assoc = info.assoc;      // 關聯度
    int bsize = info.bsize;      // block size
    char *repl = info.repl;      // replacement policy
    int num_block = nk / bsize;   // number of blocks
    int set = num_block / assoc;

//    int index = log2(num_block);  // index
    int set_f = log2(set);  // set 數，每個set中有assoc個block
    int offset_f = log2(bsize);
    int tag_f = 64 - set_f - offset_f;

    char *rw = as.rw;
    unsigned long long addr = as.addr;

    unsigned long long block_addr = addr / bsize;
    int tag = block_addr / set;
    int index = block_addr % set;

    static int count[4]= {0,0,0,0};
    int mem[set][assoc+1];
    int victim[set][assoc+1];   // 紀錄victim block

    counting(tag, index, set, assoc, rw, &rh, &rm, &wh, &wm, mem, victim);
    count[0]+= rh;
    count[1]+= rm;
    count[2]+= wh;
    count[3]+= wm;

    return count;
}

int openfile(char path[], Info info) {
    char rw[1];
    unsigned long long addr;
    FILE *fp;
    
    int rh=0;
    int rm=0;
    int wh=0;
    int wm=0;

    if ((fp = fopen(path,"r")) == NULL) {
        printf("failed to open\n");
        
        exit(1);
    }

    int i=1000000;
    int n = 0;
    int *total;
    int *count;
    Access as;
    while(!!i) {
        fscanf(fp,"%s%llx", rw, &addr);
        strcpy(as.rw, rw);
        as.addr = addr;
        // function
        total = simulator(info, as, rh,rm,wh,wm);
        // op + addr
        //printf("%s %llx\n", rw, addr);
        i--;
    }
    double miss_rate = (double)(total[1] + total[3])/1000000;
    printf("total rh: %d\n",total[0]);
    printf("total rm: %d\n",total[1]);
    printf("total wh: %d\n",total[2]);
    printf("total wm: %d\n",total[3]);
    printf("Miss rate: %f\n", miss_rate);
//    printf("Hit  rate: %f\n", 1 - miss_rate);
    fclose(fp);
    return 0;
}


int main(int argc, char *argv[])
{

    int nk = atoi(argv[1]) * 1024;// cache size
    int assoc = atoi(argv[2]);    // 關聯度
    int bsize = atoi(argv[3]);    // block size
    char *repl = argv[4];         // replacement policy
    int num_block = nk / bsize;   // number of blocks
    int set = num_block / assoc;
//    int index = log2(num_block);  // index
    int set_f = log2(set);  // set 數，每個set中有assoc個block
    int offset_f = log2(bsize);
    int tag_f = 64 - set_f - offset_f;

    Info info;
    info.nk = nk;
    info.assoc = assoc;
    info.bsize = bsize;
    strcpy(info.repl, repl);

    info.num_block = num_block;
    info.set = set;
    info.set_f = set_f;
    info.offset_f = offset_f;
    info.tag_f = tag_f;

//    int num_block = nk / bsize;
    char path[] = "trace/429.mcf-184B.trace.txt/429.mcf-184B.trace.txt";

    if (!!(nk % bsize)) {
        printf("The size should be power of 2");
        exit(1);
    }

    printf("cache size   : %d KB\n",nk/1024);
    printf("associativity: %d-ways\n",assoc);
    printf("block size   : %d B\n",bsize);
    printf("replacement policy : %s\n",repl); 

    // 開檔
    openfile(path, info);
//    printf("Number of blocks : %d\n", index);
    return 0;
} 
