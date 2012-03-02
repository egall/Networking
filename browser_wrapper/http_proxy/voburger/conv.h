/*
 *  Simple alpha PROXY Server
 *  ("Convertor module header file")
 *
 *  ORIGINAL by k-chinen@is.aist-nara.ac.jp, 1994
 *  UPDATED & EXTENDED by mike@vorburger.ch, Nov 1998 
 *
 * $Id: conv.h,v 1.1 1994/12/01 07:26:33 k-chinen Exp k-chinen $
 */

#ifndef WCOL_CONV_H

#define CONV_TBL_NUM        (64)


#define CONV_INNER_FUNCTION	(0x10)

#define CONV_HAS_INPUT      (0x1)
#define CONV_HAS_OUTPUT     (0x2)



typedef struct {
    char target[STRING_SIZE];
    char target_ext[STRING_SIZE];
    char output[STRING_SIZE];
    char program[STRING_SIZE];
} conv_elem;


int check_hook(char *name, char *type);
int conv_program_name(char *dest,char *mask, char *in, char *out);

extern conv_elem conv_tbl[];

#define WCOL_CONV_H
#endif /* WCOL_CONV_H */
