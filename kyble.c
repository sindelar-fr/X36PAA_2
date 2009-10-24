/******************************************/
/* KYBLE V1.0       PAA  1999     kyble.c */
/* Reseni zobecneneho problemu dvou kyblu */
/* Copyright (c) 1999 by KP CVUT-FEL      */
/******************************************/

#include "kyble.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*#define DEBUG            /* Control prints */
/* #define DEBUGO           /* Control prints - operations */
/* #define DEBUGQ           /* Control prints - queue */
#define HARDERR          /* Pri preteceni fronty program skonci */

#define MAXVERTEX 100000  /* max. pocet uzlu grafu stav. prostoru */
#define MAXQUEUE  30000   /* max. pocet rozpracovanych uzlu */
#define MAXBUCKET 5       /* run-time max. pocet kyblu */

unsigned full_buckets[MAXBCKTS]; /* objem jednotl. kyblu */
unsigned final_buckets[MAXBCKTS]; /* koncovy stav */
vertex vertices[MAXVERTEX]; /* souvisly seznam uzlu */
unsigned ffree = 1; /* ukazatel na prvni volne misto v poli uzlu */
unsigned queue[MAXQUEUE]; /* fronta rozpracovanych uzlu */
unsigned qcnt = 0; /* pocet prvku ve fronte */
unsigned first = 0, last = 0; /* prvni, posledni prvek fronty */
FILE* trace; /* ulozeni prubehu vypoctu */
FILE* fv; /* ulozeni reseni */
// struct time t;                     /* cas vypoctu */
struct tm * t_m;
time_t t;

/* Konvence: prvni prvek v poli vertices (tj. 0) musi zustat prazdny */

void set_final_buckets(void) {
    final_buckets[0] = 14;
    final_buckets[1] = 4;
    final_buckets[2] = 5;
    final_buckets[3] = 0;
    final_buckets[4] = 4;
    final_buckets[5] = 3;
    final_buckets[6] = 3;
    final_buckets[7] = 3;
    final_buckets[8] = 3;
    final_buckets[9] = 3; /* */
}

void set_full_buckets(void) {
    full_buckets[0] = 14;
    full_buckets[1] = 10;
    full_buckets[2] = 6;
    full_buckets[3] = 2;
    full_buckets[4] = 8;
    full_buckets[5] = 3;
    full_buckets[6] = 3;
    full_buckets[7] = 3;
    full_buckets[8] = 3;
    full_buckets[9] = 3; /* */
}

void putvertex(unsigned b) {
    /* ulozi novy stav do grafu */
    (vertices + ffree)->backptr = b;
    if (b) (vertices + ffree)->count = (vertices + b)->count + 1;
    else (vertices + ffree)->count = 0;
    (vertices + ffree)->hits = 1;
    if ((ffree++) > MAXVERTEX) {
        printf("PUTVERTEX: MAXIMUM NUMBER OF VERTICES EXCEEDED \n");
        fprintf(trace, "PUTVERTEX: MAXIMUM NUMBER OF VERTICES EXCEEDED \n");
        exit(1);
    }
}

unsigned set_initial_buckets(void) {
    unsigned b = 1;
    ffree = 1; /* pro jistotu */
    (vertices + b)->backptr = 0; /* pocatecni stav nema predchudce */
    (vertices + b)->bucket[0] = 0;
    (vertices + b)->bucket[1] = 0;
    (vertices + b)->bucket[2] = 1;
    (vertices + b)->bucket[3] = 0;
    (vertices + b)->bucket[4] = 0;
    (vertices + b)->bucket[5] = 0;
    (vertices + b)->bucket[6] = 0;
    (vertices + b)->bucket[7] = 0;
    (vertices + b)->bucket[8] = 0;
    (vertices + b)->bucket[9] = 0; /* */
    putvertex(0);
    return b;
}

unsigned isempty(unsigned b, unsigned which) {
    /* zjisti, jestli dany kybl je prazdny */
    return (!((vertices + b)->bucket[which]));
}

unsigned isfull(unsigned b, unsigned which) {
    /* zjisti, jestli dany kybl je plny */
    return (((vertices + b)->bucket[which]) == full_buckets[which]);
}

unsigned empty(unsigned bk, unsigned which) {
    /* Vyleje kybl, jehoz poradi je urceno cislem which */
    unsigned a = 0;
    vertex *vert, *buckets;
    if (which > MAXBUCKET) {
        printf("EMPTY: INVALID BUCKET NUMBER\n");
        fprintf(trace, "EMPTY: INVALID BUCKET NUMBER\n");
        exit(1);
    }
    if (bk > MAXVERTEX) {
        printf("EMPTY: MAXIMUM NUMBER OF VERTICES EXCEEDED\n");
        fprintf(trace, "EMPTY: MAXIMUM NUMBER OF VERTICES EXCEEDED\n");
        exit(1);
    }
    buckets = (vertices + bk);
#ifdef DEBUGO
    printf("EMPTY: ENTRY B %2i V %2i \n", which, buckets->bucket[which]);
    fprintf(trace, "EMPTY: ENTRY B %2i V %2i \n", which, buckets->bucket[which]);
#endif
    vert = (vertices + ffree);
    vert->backptr = bk;
    vert->oper = 0;
    vert->count = 0;
    vert->which1 = which;
    vert->which2 = 0;
    for (a = 0; a < MAXBUCKET; a++) vert->bucket[a] = buckets->bucket[a];
    vert->bucket[which] = 0;
#ifdef DEBUGO
    buckets = (vertices + ffree);
    printf("EMPTY: EXIT  B %2i V %2i \n", which, buckets->bucket[which]);
    fprintf(trace, "EMPTY: EXIT  B %2i V %2i \n", which, buckets->bucket[which]);
#endif
    return ffree;
}

unsigned fill(unsigned bk, unsigned which) {
    /* Naplni kybl, jehoz poradi je urceno cislem which */
    unsigned a = 0;
    vertex *vert, *buckets;
    if (which > MAXBUCKET) {
        printf("FILL: INVALID BUCKET NUMBER\n");
        fprintf(trace, "FILL: INVALID BUCKET NUMBER\n");
        exit(1);
    }
    if (bk > MAXVERTEX) {
        printf("FILL: MAXIMUM NUMBER OF VERTICES EXCEEDED\n");
        fprintf(trace, "FILL: MAXIMUM NUMBER OF VERTICES EXCEEDED\n");
        exit(1);
    }
    buckets = (vertices + bk);
#ifdef DEBUGO
    printf("FILL:  ENTRY B %2i V %2i \n", which, buckets->bucket[which]);
    fprintf(trace, "FILL:  ENTRY B %2i V %2i \n", which, buckets->bucket[which]);
#endif
    vert = (vertices + ffree);
    vert->backptr = bk;
    vert->oper = 1;
    vert->count = 0;
    vert->which1 = which;
    vert->which2 = 0;
    for (a = 0; a < MAXBUCKET; a++) vert->bucket[a] = buckets->bucket[a];
    vert->bucket[which] = full_buckets[which];
#ifdef DEBUGO
    buckets = (vertices + ffree);
    printf("FILL:  EXIT  B %2i V %2i \n", which, buckets->bucket[which]);
    fprintf(trace, "FILL:  EXIT  B %2i V %2i \n", which, buckets->bucket[which]);
#endif
    return ffree;
}

unsigned pour(unsigned bk, unsigned which1, unsigned which2) {
    /* Preleje obsah kyblu do druheho */
    /* which2=which1; which1=0;*/
    unsigned a = 0, pom1 = 0, pom2 = 0;
    vertex *vert, *buckets;
    if (which1 > MAXBUCKET) {
        printf("POUR: INVALID BUCKET NUMBER\n");
        fprintf(trace, "POUR: INVALID BUCKET NUMBER\n");
        exit(1);
    }
    if (bk > MAXVERTEX) {
        printf("POUR: MAXIMUM NUMBER OF VERTICES EXCEEDED\n");
        fprintf(trace, "POUR: MAXIMUM NUMBER OF VERTICES EXCEEDED\n");
        exit(1);
    }

    buckets = (vertices + bk);
#ifdef DEBUGO
    printf("POUR:  ENTRY B %2i V %2i B %2i V %2i \n", which1, buckets->bucket[which1], which2, buckets->bucket[which2]);
    fprintf(trace, "POUR:  ENTRY B %2i V %2i B %2i V %2i \n", which1, buckets->bucket[which1], which2, buckets->bucket[which2]);
#endif
    vert = (vertices + ffree);
    vert->backptr = bk;
    vert->oper = 2;
    vert->count = 0;
    vert->which1 = which1;
    vert->which2 = which2;
    for (a = 0; a < MAXBUCKET; a++) vert->bucket[a] = buckets->bucket[a];
    /* vert->bucket[which2]=(vert->bucket[which1]>vert->bucket[which2])?full_buckets[which2]:vert->bucket[which1]; /* Old version */
    pom1 = full_buckets[which2] - vert->bucket[which2]; /* volne misto v druhem kyblu */
    pom2 = min(vert->bucket[which1], pom1); /* bud se omezuje volnym mistem v druhem kyblu, nebo mnozstvim vody v prvnim kyblu */
    vert->bucket[which1] = vert->bucket[which1] - pom2; /* odlit */
    vert->bucket[which2] = vert->bucket[which2] + pom2; /* nalit */
#ifdef DEBUGO
    buckets = (vertices + ffree);
    printf("POUR:  EXIT  B %2i V %2i B %2i V %2i \n", which1, buckets->bucket[which1], which2, buckets->bucket[which2]);
    fprintf(trace, "POUR:  EXIT  B %2i V %2i B %2i V %2i \n", which1, buckets->bucket[which1], which2, buckets->bucket[which2]);
#endif
    return ffree;
}

unsigned check(unsigned b) {
    /* Zkontroluje, jestli dany stav je pozadovane reseni */
    unsigned a = 0, flag = 1;
    vertex *buckets;
    buckets = (vertices + b);
    for (a = 0; a < MAXBUCKET; a++) if (buckets->bucket[a] != final_buckets[a]) flag = 0;
    return flag;
}

unsigned initcheck(unsigned b) {
    /* Zkontroluje, jestli dany stav je pripustny stav stavoveho prostoru */
    unsigned a = 0, flag = 1;
    vertex *buckets;
    buckets = (vertices + b);
    for (a = 0; a < MAXBUCKET; a++) if (buckets->bucket[a] > full_buckets[a]) flag = 0;
    return flag; /* 1..OK, 0..FAIL */
}

unsigned compare(unsigned b, unsigned bk) {
    /* Zkontroluje, jestli jsou uzly shodne */
    unsigned a = 0, flag = 1;
    vertex *buckets1, *buckets2;
    buckets1 = (vertices + b);
    buckets2 = (vertices + bk);
    if (b == bk) return 1; /* New version */
    for (a = 0; a < MAXBUCKET; a++) if (buckets1->bucket[a] != buckets2->bucket[a]) flag = 0;
    return flag;
}

void assign_rewrite(FILE **F, const char *Name) {
    /* otevre soubor na vystup */
    if ((*F = fopen(Name, "wt")) == NULL) {
        printf("ASSIGN_REWRITE: UNABLE TO OPEN FILE %s\n", Name);
        exit(1);
    }
}

void enqueue(unsigned b) {
    /* vlozi dany uzel do fronty k dalsimu zpracovani */
    static unsigned cnt = 0;
#ifdef DEBUGB
    unsigned a = 0;
#endif
    cnt++;
#ifdef DEBUGQ
    printf("ENQUEUE: PASS %i MEMBERS %i\n", cnt, qcnt);
    fprintf(trace, "ENQUEUE: PASS %i MEMBERS %i\n", cnt, qcnt);
#endif
#ifdef DEBUGB
    printf("ENQUEUE: BUCKETS  ");
    fprintf(trace, "ENQUEUE: BUCKETS  ");
    for (a = 0; a < MAXBUCKET; a++) {
        printf("%i ", (vertices + b)->bucket[a]);
        fprintf(trace, "%i ", (vertices + b)->bucket[a]);
    }
    printf("\n");
    fprintf(trace, "\n");
#endif
    if (((last + 1) % MAXQUEUE) == first) {
        printf("ENQUEUE: QUEUE IS FULL   PASS %i\n", cnt);
        fprintf(trace, "ENQUEUE: QUEUE IS FULL    PASS %i\n", cnt);
#ifdef HARDERR
        exit(1);
#else
        return;
#endif
    } else qcnt++;
    queue[last] = b;
    last = (last + 1) % MAXQUEUE;
}

unsigned dequeue(void) {
    /* vybere uzel z fronty k dalsimu zpracovani */
    static unsigned cnt = 0;
    unsigned b = 0;
    cnt++;
    if (qcnt) qcnt--; /* osetreni podteceni */
#ifdef DEBUGQ
    printf("DEQUEUE: PASS %i MEMBERS %i\n", cnt, qcnt);
    fprintf(trace, "DEQUEUE: PASS %i MEMBERS %i\n", cnt, qcnt);
#endif
    if (first == last) {
        printf("DEQUEUE: QUEUE IS EMPTY    PASS %i\n", cnt);
        fprintf(trace, "DEQUEUE: QUEUE IS EMPTY    PASS %i\n", cnt);
        return (0);
    }
    b = queue[first];
    first = (first + 1) % MAXQUEUE;
    if (b == 0) {
        printf("DEQUEUE: FATAL ERROR    PASS %i\n", cnt);
        fprintf(trace, "DEQUEUE: FATAL ERROR    PASS %i\n", cnt);
        exit(1);
    }
    return (b);
}

void save(unsigned temp, unsigned prev) {
    /* zapise postup operaci pro nalezeni reseni do souboru */
    /* prev je primy predchudce uzlu temp pro prave provadenou operaci */
    /* je to kvuli osetreni vice zpusobu dosazeni reseni */
    unsigned b = temp, c = 0;
    unsigned bb = 0;
    static unsigned cnt = 0;
    cnt++;
    if ((temp != prev) && (prev != (vertices + temp)->backptr)) {
        printf("SAVE: DUPLICATE SOLUTION   #%i\n", cnt);
        fprintf(fv, "SAVE: DUPLICATE SOLUTION   #%i\n", cnt);
    }

    for (bb = 0; bb < MAXBUCKET; bb++) {
        printf("%2i ", (vertices + temp)->bucket[bb]);
        fprintf(fv, "%2i ", (vertices + temp)->bucket[bb]);
    }
    printf("\n");
    fprintf(fv, "\n");

    /* b=(vertices+b)->backptr; /* Old version */
    /* if (!(b=(vertices+b)->backptr)) b=prev; /* asi to neni ono */

    while (b != 0) {
        c++;
        if ((vertices + b)->backptr != 0) { /* prvni uzel je pocatecni stav a neni pro nej definovana operace */
            printf("#%2i %2i OPER %i  WHICH1 %i  ", cnt, c, (vertices + b)->oper, (vertices + b)->which1);
            if ((vertices + b)->oper == 2) printf("WHICH2 %i   #%i ", (vertices + b)->which2, cnt);
            else printf("           #%i ", cnt);
            fprintf(fv, "#%2i %2i OPER %i  WHICH1 %i  ", cnt, c, (vertices + b)->oper, (vertices + b)->which1);
            if ((vertices + b)->oper == 2) fprintf(fv, "WHICH2 %i   #%i ", (vertices + b)->which2, cnt);
            else fprintf(fv, "           #%i ", cnt);
            for (bb = 0; bb < MAXBUCKET; bb++) {
                printf("%2i ", (vertices + b)->bucket[bb]);
                fprintf(fv, "%2i ", (vertices + b)->bucket[bb]);
            }
            printf("\n");
            fprintf(fv, "\n");
        }
        b = (vertices + b)->backptr;
    }
}

void insert(unsigned state) {
    unsigned i;
    for (i = 0; i < ffree; i++) {
        if (compare(ffree, i)) return;
    }
    putvertex(state);
    enqueue(ffree - 1);
}

void search(void) {
    /* Reseni problemu kyblu Vasi heuristikou */
    int i, j;
    unsigned state;
    enqueue(1);

    while (state = dequeue()) {
        if (check(state)) {
            /*printf("a");*/
            printf("Nalezeno reseni: %d\n", state);
            fprintf(trace, "Nalezeno reseni: %d\n", state);
            return;
        }
        /*nalit*/
        for (i = 0; i < MAXBUCKET; i++) {
            if (!isfull(state, i)) {
                fill(state, i);
                insert(state);
            }
        }
        /*vylit*/
        for (i = 0; i < MAXBUCKET; i++) {
            if (!isempty(state, i)) {
                empty(state, i);
                insert(state);
            }
        }

        /*prelit*/
        for (i = 0; i < MAXBUCKET; i++) {
            for (j = 0; j < MAXBUCKET; j++) {
                if (!isempty(state, i) && (!isfull(state, j))) {
                    pour(state, i, j);
                    insert(state);
                }
            }
        }

    }


    printf("SEARCH: DONE\n");
    fprintf(trace, "SEARCH: DONE\n");
}

void dump_vertices(void) {
    unsigned a = 0, b = 0;
    for (a = 1; a < ffree; a++) {
        printf("#%2i BUCKETS: ", a);
        fprintf(trace, "#%2i BUCKETS: ", a);
        for (b = 0; b < MAXBUCKET; b++) {
            printf("%2i ", (vertices + a)->bucket[b]);
            fprintf(trace, "%2i ", (vertices + a)->bucket[b]);
        }
        printf(" DIST: %2i  HITS: %2i\n", (vertices + a)->count, (vertices + a)->hits);
        fprintf(trace, " DIST: %2i  HITS: %2i\n", (vertices + a)->count, (vertices + a)->hits);
    }
}

void save_vertices(void) {
    FILE *mdl;
    unsigned b = 0;

    if ((mdl = fopen("source.bin", "wb")) == NULL) {
        printf("SAVE_VERTICES: ERROR OPENING FILE");
        exit(1);
    }
    printf("Writing space \n");
    fprintf(trace, "Writing space \n");

    if (fwrite(&ffree, sizeof (int), 1, mdl) == -1) {
        printf("SAVE_VERTICES: ERROR WRITING VERTEX COUNT");
        exit(1);
    }

    for (b = 0; b < ffree; b++) {
        if (fwrite((vertices + b), sizeof (vertex), 1, mdl) == -1) {
            printf("SAVE_VERTICES: ERROR WRITING VERTEX");
            exit(1);
        }
    }


    fclose(mdl);
}

int main(void) {
    /* Reseni zobecneneho problemu dvou kyblu */
    unsigned a, b, c;
    a = b = c = 0;

    assign_rewrite(&trace, "trace.txt");
    assign_rewrite(&fv, "solution.txt");


    set_full_buckets();
    b = set_initial_buckets();
    set_final_buckets();
    if (!initcheck(b)) {
        printf("MAIN: INITIAL STATE IS INVALID \n");
        fprintf(trace, "MAIN: INITIAL STATE IS INVALID \n");
        exit(1);
    }

    t = time(&t);
    t_m = localtime(&t);

    printf("%2i:%02i  MAIN: Starting search...\n", t_m->tm_hour, t_m->tm_min);
    fprintf(trace, "%2i:%02i  MAIN: Starting search...\n", t_m->tm_hour, t_m->tm_min);


    search(); /* uplne prochazeni stav. prostoru */

    t = time(&t);
    t_m = localtime(&t);

    printf("%2i:%02i  MAIN: DONE AFTER EXPLORING %i VERTICES \n\n", t_m->tm_hour, t_m->tm_min, ffree - 1);
    fprintf(trace, "%2i:%02i  MAIN: DONE AFTER EXPLORING %i VERTICES \n", t_m->tm_hour, t_m->tm_min, ffree - 1);
    printf("\n\n\n\n");
    dump_vertices();
    save_vertices(); /* ulozeni stav. prostoru */

    fclose(trace);
    fclose(fv);

}
