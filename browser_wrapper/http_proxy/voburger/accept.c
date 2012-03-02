/*
 *  Simple alpha PROXY Server
 *  ("Deliverer Program, access control (`check accept or not') routines.")
 *
 *  ORIGINAL by k-chinen@is.aist-nara.ac.jp, 1994
 *  UPDATED & EXTENDED by mike@vorburger.ch, Nov 1998
 *
 *  $Id: accept.c,v 1.1 1994/12/01 11:52:21 k-chinen Exp k-chinen $
 */

#include <stdio.h>
#include <ctype.h>

#include "wcol.h"

#define ACCEPT_TABLE_MAX    20


typedef
    struct {
        unsigned char addr[5];
        unsigned char mask[5];
    } accept_table_rec;

static int accept_last=0;
static accept_table_rec table[ACCEPT_TABLE_MAX];


/*
 * Syntex suger
 */
#define set_addr(ad, a, b, c, d) \
    ad[0] = a; ad[1] = b; ad[2] = c; ad[3] = d; ad[4] = 0

#define show_addr(pre, ad) \
    Trace("%s: %d.%d.%d.%d. (%02x.%02x.%02x.%02x)\n", \
        pre, ad[0], ad[1], ad[2], ad[3], ad[0], ad[1], ad[2], ad[3])



/*
 * Read_Accepttable - Read accept table.
 */
int Read_Accepttable(char *fname)
{
    FILE *fp;
    unsigned char line[BUF_SIZE],tmp[STRING_SIZE];
    unsigned char addr[STRING_SIZE], mask[STRING_SIZE];
    unsigned char *p,*q;
    int i;

    if((fp=fopen(fname, "r"))==NULL) {
        Error("Read_Acceptable: Cannot open '%s'\n", fname);
        Trace("Read_Acceptable: No Access Control\n");
        Log("NO-ACCESS-CONTROL\n", fname);
        return 1;
    }

	Trace("Read accept table from %s ...\n", fname);

    accept_last = 0;
    while(fgets((char*)line, BUF_SIZE, fp)!=NULL) {
        p = line;
        /* Remove whites */
        while(*p && isspace(*p)) {
            p++;
        }

        if(accept_last>=ACCEPT_TABLE_MAX) {
            Error("Read_Acceptable: Too many accept targets.\n");
            break;
        }


        /* Skip comment or empty line */
        if(*p=='\0' || *p=='#')
            continue;

        /* Clear address and mask */
        memset(addr, 0, 5);
        memset(mask, 0, 5);
/*
        Trace("\n> %s", line);
*/

        /*
         * Get Address
         */
        i = 0;
        while(*p && i<4) {
            q = tmp;
            while(*p&&isdigit(*p))
                *q++ = *p++;
            *q = '\0';
            addr[i] = (unsigned char) atoi((char*)tmp);
/*
            Trace("(%d: '%s' %d) ", i, tmp, addr[i]);
*/

            i++;
            if(*p=='.')
                p++;
            else
                break;
        }

        memcpy(table[accept_last].addr, addr, 5);
/*
        show_addr("\tAddress", addr);
*/
        show_addr("Address", addr);


        while(*p&&(*p==' '||*p=='\t')) {
            p++;
        }

        if(!isdigit(*p)) {
                 if(addr[0]<128) {set_addr(mask, 255,   0,   0,   0);} /* A */
            else if(addr[0]<192) {set_addr(mask, 255, 255,   0,   0);} /* B */
            else if(addr[0]<224) {set_addr(mask, 255, 255, 255,   0);} /* C */
#if 0
            else if(addr[0]<240) {set_addr(mask, 255, 255, 255, 255);} /* D */
            else if(addr[0]<248) {set_addr(mask, 255, 255, 255, 255);} /* E */
#endif
            else {
                Trace("Ignore Address. realy ?\n");
            }

            memcpy(table[accept_last++].mask, mask, 5);
/*
            show_addr("\tNetMask", mask);
*/
            show_addr("Default NetMask", mask);

            continue; /* next line */
        }

        /*
         * Get Mask
         */
        i = 0;
        while(*p && i<4) {
            q = tmp;
            while(*p&&isdigit(*p))
                *q++ = *p++;
            *q = '\0';
            mask[i] = (unsigned char) atoi((char*)tmp);
/*
            Trace("(%d: '%s' %d) ", i, tmp, mask[i]);
*/

            i++;
            if(*p=='.')
                p++;
            else
                break;
        }

        memcpy(table[accept_last++].mask, mask, 5);
/*
        show_addr("\tNetMask", mask);
*/
        show_addr("NetMask", mask);
    }

    if(accept_last==0)
        Log("NO-ACCESS-CONTROL\n", fname);

    return 0;
}

/*
 * Acceptable - Check acceptable address or not.
 */
int Acceptable(unsigned char *target)
{
    int i,j;
    int match;

/*
    Trace("Acceptable: ");
    show_addr("Target", target);
*/

    if(accept_last==0)
        return 1;                   /* Because no access control */

    match = 0;
    for(i=0;i<accept_last;i++) {
/*
        Trace("%2d:\n", i);
        show_addr("\tAddress ", table[i].addr);
        show_addr("\tNetmask ", table[i].mask);
        Trace("\t");
*/

        for(j=0;j<4;j++) {
/*
            Trace("%02x.", table[i].mask[j] & target[j]);
*/

            if((table[i].mask[j] & target[j]) != table[i].addr[j])
                break ;
        }
/*
        Trace(" -> %d match.\n", j);
*/

        if(j>=4) {
            match = 1;
#ifndef ACCEPT_TEST
/*
            Trace("\tMatch to pattern #%d.\n",i);
*/
            return 1;
#endif
        }
    }

    return match;
}


#ifdef ACCEPT_TEST

/*
 *  Main routine for test.
 *
 *  If test to a1.a2.a3.a4 address is acceptable, run "$0 a1 a2 a3 a4".
 *
 */


extern char log_file[];
extern int trace_flag;


main(int argc, char **argv)
{
    int i;
    unsigned char target[5];

    trace_flag = 1;
    strcpy(log_file,"/dev/tty");

    Read_Accepttable("./wool-accept.cfg");

    i = 0;
    while(argv[i+1]) {
        target[i] = (unsigned char) atoi(argv[i+1]);
        i++;
    }

    show_addr("Ok, Target ", target);
    Trace(" -> %d\n",Acceptable(target));
}
#endif
