#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct Block {
    int tag;
    int v;
    int size;
};

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

void swap(struct Block *x, struct Block *y) {
    struct Block tmp = *x;
    *x = *y;
    *y = tmp;
}

void bubble(struct Block index[]) {
    for (int i=0; i<63; i++) {
        for (int j=0; j<63-i; j++) {
            if (index[j].v > index[j+1].v)
                swap(&index[j],&index[j+1]);
        }
    }
}

void counting(int tag, int index, int set, int assoc, char *rw,
        int *rh, int *rm, int *wh, int *wm, struct Block mem[set][assoc], int age) {

    int read_hit=0;
    int read_miss=0;
    int write_hit=0;
    int write_miss=0;

    // ASCII => r=114  w=119
    int r = (int)rw[0];

    int size=0;
    for (int i=0; i<assoc; i++) {
        size += mem[index][i].size;
    }

    // 初始化
    if (size == 0) {
        for (int i=0; i<assoc; i++) {
            mem[index][i].v = 0;
            mem[index][i].tag = -1;
        }
    }

    // read operation
    if (r == 114) {
        for (int i=0; i < assoc; i++) {
            if (mem[index][i].tag == tag) {
                // 有被存取到的index  victim值=age
                mem[index][i].v = age;
                read_hit++;
                *rh = read_hit;
                return;
            }
        }
        // 沒有空block => LRU
        if (size == assoc) {
            bubble(mem[index]);
            mem[index][0].tag = tag;
            mem[index][0].v = age;
            mem[index][0].size = 1;
            read_miss++;
            *rm = read_miss;
            return;
        }
        // fill in missing block
        int ind = 0;
        while (size <= assoc) {
            // 找空block
            if ((mem[index][ind].size) == 1) {
                ind++;
            }
            else { 
                // 有空block 直接寫入
                mem[index][ind].tag = tag;
                mem[index][ind].size = 1;
                mem[index][ind].v = age;
                read_miss++;
                *rm = read_miss;
                return;
            }
        }
    }
    
    // write operation
    else {
        for (int i=0; i < assoc; i++) {
            if (mem[index][i].tag == tag) {
                // 有被存取到的index  victim = age
                mem[index][i].v = age;
                write_hit++;
                *wh = write_hit;
                return;
            }
        }
        // 沒有空block => LRU
        if (size == assoc) {
            bubble(mem[index]);
            mem[index][0].tag = tag;
            mem[index][0].v = age;
            mem[index][0].size = 1;
            write_miss++;
            *wm = write_miss;
            return;
        }
        // fill in missing block
        int ind = 0;
        while (size <= assoc) {
            // 找空block
            if ((mem[index][ind].size) == 1) {
                ind++;
            }
            else { 
                // 有空block 直接寫入
                mem[index][ind].tag = tag;
                mem[index][ind].size = 1;
                mem[index][ind].v = age;
                write_miss++;
                *wm = write_miss;
                return;
            }
        }
    } 
    return;
}

int *simulator(Info info, Access as,int rh, int rm, int wh, int wm, int age) {
    int nk = info.nk;            // cache size
    int assoc = info.assoc;      // 關聯度
    int bsize = info.bsize;      // block size
    char *repl = info.repl;      // replacement policy
    int num_block = nk / bsize;   // number of blocks
    int set = num_block / assoc;

    int set_f = log2(set);  // set 數，每個set中有assoc個block
    int offset_f = log2(bsize);
    int tag_f = 64 - set_f - offset_f;

    char *rw = as.rw;
    unsigned long long addr = as.addr;

    unsigned long long block_addr = addr / bsize;
    int tag = block_addr / set;
    int index = block_addr % set;

    static int count[4]= {0,0,0,0};
    struct Block mem[set][assoc+1];

    counting(tag, index, set, assoc, rw, &rh, &rm, &wh, &wm, mem, age);
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
    int age=1;
    while(!!i) {
        fscanf(fp,"%s%llx", rw, &addr);
        strcpy(as.rw, rw);
        as.addr = addr;
        total = simulator(info, as, rh,rm,wh,wm,age);
        age++;
        i--;
    }
    double miss_rate = (double)(total[1] + total[3])/1000000;
    double read_miss_rate = (double)total[1]/(double)(total[1] + total[0]);
    double write_miss_rate = (double)total[3]/(double)(total[2] + total[3]);
    printf("read   hit: %d\n",total[0]);
    printf("read  miss: %d\n",total[1]);
    printf("write  hit: %d\n",total[2]);
    printf("write miss: %d\n",total[3]);
    printf("Total  Miss rate: %f%%\n", miss_rate*100);
    printf("Read   Miss rate: %f%%\n", read_miss_rate*100);
    printf("write  Miss rate: %f%%\n", write_miss_rate*100);
    
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
    return 0;
} 
