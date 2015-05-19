/* Vectrex Tutorial stabber tool by Joakim Larsson Edström, (c) 2015, GPLv3 */

/*
 * This is a tool to intermix assembler with source code based on the
 * STABS debugformat from the AS6809 .LST, .RST and .S files generated
 * with the -g flag to GCC and -y flag to as6809. Other compilers and
 * assemblers may also be able to produce STABS. The STABS format is 
 * used in COFF output files too but in binary form I think and not 
 * not tested. 
 * 
 * In general STABS is nowadays replaced by the DWARF debug format and
 * others but most 8 bit tools are too old to generate DWARF.  
 *
 * Read more about STABS here: https://docs.freebsd.org/info/stabs/stabs.pdf
 * 
 * Compile with:
 *
 *   gcc -o stabber stabber.c ; 
 *
 * Use example:
 *
 * cat bouncer5.lst | ./stabber
*/

#include <stdio.h>
#include <string.h>

main()
{
  FILE *fp = NULL;
  char buf[200];
  char *p;
  char desc[200], data[200], value[200];
  int  type, other;
  int  dq;   /* Double quote counter */

  printf("; Mixed by STABBER, decoder of debug information\n");
  while (fgets(buf, sizeof(buf), stdin) != NULL)
  {
    if ((p = strstr(buf, "\t.stab")) != NULL || (p = strstr(buf, " .stab")) != NULL)
    {
      char *p1;
      char last = ' ';
      int i, j;


      /* This part of the parser should really be a better hack, sorry */
      p++;
      p1 = &p[6];
      dq = 0;

      for (i = 6; i < 200 && p[i] != '\r' && p[i] != '\n'; i++)
      {
	switch (p[i])
	{
	  case '"': 
	    *p1 = ' ';
	    dq = dq == 0 ? 1 : 0;
	    p1 += last != ' ' ? 1 : 0;
	    last = ' ';
	    break;
	  case ' ':
	    if (dq != 0) // inside double quotes we mark spaces with + to enable sscanf to treat it as one string
	    {
	      *p1 = '+';
	      p1 += 1;
	      break;
	    }
	    *p1 = ' ';
	    p1 += last != ' ' ? 1 : 0;
	    last = ' ';
	    break;
	  case ',':
	    if (dq != 0) // inside double quotes we mark spaces with + to enable sscanf to treat it as one string
	    {
	      *p1 = '|';
	      p1 += 1;
	      break;
	    }
	    *p1 = ' ';
	    p1 += last != ' ' ? 1 : 0;
	    last = ' ';
	    break;
	  default:
	    *p1++ = p[i];
	    last = p[i];
	    break;
	}
      }
      *p1 = '\0';

      /* The following part is a bit better */

      /* .stabs formats */
      if (p[5] == 's')
      {
	char types[200];
	sscanf(&p[6], "%s %d %d %s %s", data, &type, &other, desc, value);
	switch(type)
	{
	case 0x20:
	  strncpy(types,"N_GSYM", sizeof(types)); 
	  printf("; %s Global symbol called %s\n", types, data); 
	  break;
	case 0x24:
	  strncpy(types,"N_FUN", sizeof(types)); 
	  printf("; %s function called %s\n", types, data); 
	  break;
	case 0x26:
	  strncpy(types,"N_STSYM", sizeof(types)); 
	  printf("; %s Data section file-scope static initialized variable %s\n", types, data); 
	  break;
	case 0x28:
	  strncpy(types,"N_LCSYM", sizeof(types)); 
	  printf("; %s BSS section file-scope static un-initialized variable %s\n", types, data); 
	  break;
	case 0x3c:
	  strncpy(types,"N_OPT", sizeof(types)); 
	  printf("; %s The STAB information was generated because the source was %s\n", types, data); 
	  break;
	case 0x40:
	  strncpy(types,"N_RSYM", sizeof(types)); 
	  printf("; %s Register variable %s\n", types, data); 
	  break;
	case 0x64:
	  strncpy(types,"N_SO", sizeof(types)); 
	  printf("; %s source file %s\n", types, data); 
	  fp = fopen(data, "r");
	  break;
	case 0xa0:
	  strncpy(types,"N_PSYM", sizeof(types)); 
	  printf("; %s stack parameter %s at stack offset %s\n", types, data, value); 
	  break;
	case 0x80: /* Symbol definitions */
	  {
	    char name[200], exp[200], *p2;
	    strncpy(types,"N_LSYM", sizeof(types));
	    p1 = data; while (*p1) { *p1 = (*p1 == ':' ? ' ' : *p1); p1++; }
	    sscanf(data, "%s %s", name, exp);
	    printf("; Local symbol %s: %s\n", name, exp);
	  }
	  break;
	case 0x82:
	  strncpy(types,"N_BINCL", sizeof(types)); 
	  printf("; %s beginning of include file: %s\n", types, data); 
	  break;
	default:
	  snprintf(types, sizeof(types), "0x%x", type);
	  printf("; Unknown S record type %d |%s| %s %d |%s| |%s|\n", type, data, types, other, desc, value);
	}
      }
      /* .stabn formats */
      else if (p[5] == 'n')
      {
	int line;
	char types[200];

	sscanf(&p[6], "%d %d %d %s", &type, &other, &line, value);
	switch(type)
	{
	case 0xa2:
	  strncpy(types,"N_EINCL", sizeof(types)); 
	  printf("; %s End of include file: \n", types); 
	  break;
	case 0x44:
	  strncpy(types,"N_SLINE", sizeof(types)); 
	  if (fp != NULL)
	  {
	    char lbf[200];
	    rewind(fp);
	    while (line-- > 0)
	    { 
	      fgets(lbf, sizeof(buf), fp);
	    }
	    printf("; %s", lbf);
	  }
	  break;
	default:
	  snprintf(types, sizeof(types), "0x%x", type);
	  printf("; Unknown N record type %s %d %s %s\n", types, other, desc, value);
	}
      }
      /* .stabd formats */
      else if (p[5] == 'd')
      {
	char types[200];

	sscanf(&p[6], "%d %d %s %s", &type, &other, desc, value);
	switch(type)
	{
	case 0x44:
	  strncpy(types,"N_SLINE", sizeof(types)); 
	  if (fp != NULL)
	  {
	    char lbf[200];
	    signed long line = strtol(desc, NULL , 10);

	    rewind(fp);
	    while (line-- > 0)
	    { 
	      fgets(lbf, sizeof(buf), fp);
	    }
	    printf("; %s", lbf);
	  }
	  break;
	default:
	  snprintf(types, sizeof(types), "0x%x", type);
	  printf("; Unknown D record type %s %d |%s|\n", types, other, desc);
	}
      }
      else
	printf("; ?: Unknown record type: %s\n", p);
    }
    else
      /* Just remove pagination headers and footers */
      if ((strnlen(buf, sizeof(buf)) > 1) && (buf[0] != '\f'))
	printf("%s", buf);
  }
  if (fp != NULL)
    fclose(fp);
}
