char *heure()
{
  struct XREGS {short ax, bx, cx, dx, si, di;};
  struct HREGS {char  al, ah, bl, bh, cl, ch, dl, dh;};
  union REGS { struct XREGS x;
               struct HREGS h;
             } rin, rout;

  static char vheure[35];

  rin.h.ah = 0x2C;
  int86(0x21, &rin, &rout);
  sprintf(vheure,"%2d:%2d:%2d",rout.h.ch,rout.h.cl,rout.h.dh);

  return(&vheure);
}
