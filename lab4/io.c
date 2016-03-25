/*********************** io.c **************************/

char *table = "0123456789ABCDEF";
u16 *FP;
u16 _base = 10;

#define MAXLEN 128
char *gets(char s[ ]) // caller must provide REAL memory s[MAXLEN]
{
  char c, *t = s; int len=0;
  while( (c=getc()) != '\r' && len < MAXLEN-1)
    {
      *t++ = c; putc(c); len++;
    }
  *t = 0; return s;
}

/*int strlen(char s[])
{
  int i = 0, len = 0;

  while (s[i] != '\0')
    {
      len ++;
      i++;
    }
  return len;
}

int stoi (char s[])
{
  int i = 0, result = 0, len = 0;
  len = strlen(s);

  for(i=0; i<len; i++){
    result = result * 10 + ( s[i] - '0' );
  }
  return result;
}*/

void prints(char *s)
{
  int i = 0;
  for(i = 0; s[i]; i++)
    {
      putc(s[i]);
    }
}

void lrpu(u32 x)
{
  char c;
  if (x)
  {
    c = table[x % _base];
    lrpu(x / _base);
    putc(c);
  }
}

void rpu(u16 x)
{
  char c;
  if (x)
    {
      c = table[x % _base];
      rpu(x / _base);
      putc(c);
    }
} 

void printu(u16 x)
{
  if (x==0)
    putc('0');
  else
    _base = 10;
    rpu(x);
}

void printd(u16 x) //prints an integer
{
  if (x == 0)
    putc('0');
  else
   _base = 10;
   rpu(x);
}

void printl(u32 x) //prints an integer
{
  if (x == 0)
    putc('0');
  else
    _base = 10;
  lrpu(x);
}

int printo(u16 x) //prints x in OCTal
{
  if (x == 0 )
    putc(' ');
  else
    _base = 8;
    rpu(x);
} 

void printx(u32 x) //prints x in hex
{
  if (x == 0 )
    putc('0');
  else
    _base = 16;
    rpu(x);
} 

void printf(char *fmt, ...) // some C compiler requires the three dots
{
  char *cp = fmt; // cp points to the fmt string
  u16 *ip = (u16 *)&fmt + 1; // ip points to first item
  u32 *up; // for accessing long parameters on stack
  while (*cp){ // scan the format string
    if (*cp != '%'){ // spit out ordinary chars
      putc(*cp);
      if (*cp=='\n') // for each ‘\n’
	putc('\r'); // print a ‘\r’
      cp++; continue;
    }
    cp++; // print item by %FORMAT symbol
    switch(*cp){
    case 'c' : putc(*ip); break;
    case 's' : prints((char *)*ip); break;
    case 'u' : printu(*ip); break;
    case 'd' : printd(*ip); break;
    case 'x' : printx(*ip); break;
    case 'l' : printl(*(u32 *)ip++); break;
    case 'X' : printX(*(u32 *)ip++); break;
    }
    cp++; ip++; // advance pointers
  }
}
