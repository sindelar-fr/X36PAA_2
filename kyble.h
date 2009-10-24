/******************************************/
/* KYBLE V1.0       PAA  1999     kyble.h */
/* Reseni zobecneneho problemu dvou kyblu */
/* Copyright (c) 1999 by KP CVUT-FEL      */
/******************************************/

#ifndef kyble_h

#define kyble_h

#define MAXBCKTS 10            /* compile-time max. pocet kyblu */

#define min(x,y) (x<y)?x:y     /* makro */

typedef struct vertex {
   unsigned oper;              /* provadena operace */
   unsigned which1, which2;    /* kyble, se kterymi se pracovalo */
   unsigned bucket[MAXBCKTS];  /* aktualni stav kyblu */
   unsigned backptr;           /* kudy vedla cesta do tohoto uzlu */
   unsigned count;             /* vzdalenost od pocatecniho stavu (pocet operaci) */
   unsigned hits;              /* pocet navstiveni daneho uzlu */
   } vertex;



#endif
