
### Ouverture du fichier a controler :

$fichier = $ARGV[0];
open(FICH, $fichier) or die "Impossible d\'ouvrir $fichier !\n";


$cycles = 100000;

print "ATTENTION : controle sur $cycles cycles ...\n";


# Definition des motifs a controler 
@rt48 = 
 ("0101 0102 0103 0104 0105 0106 0107 0108 0109 010A 010B 010C 010D 010E 010F",
  "0201 0202 0203 0204 0205 0206 0207 0208 0209 020A 020B 020C 020D 020E 020F",
  "0301 0302 0303 0304 0305 0306 0307 0308 0309 030A 030B 030C 030D 030E 030F",
  "0401 0402 0403 0404 0405 0406 0407 0408 0409 040A 040B 040C 040D 040E 040F",
  "0501 0502 0503 0504 0505 0506 0507 0508 0509 050A 050B 050C 050D 050E 050F",
  "0601 0602 0603 0604 0605 0606 0607 0608 0609 060A 060B 060C 060D 060E 060F",
  "0701 0702 0703 0704 0705 0706 0707 0708 0709 070A 070B 070C 070D 070E 070F",
  "0801 0802 0803 0804 0805 0806 0807 0808 0809 080A 080B 080C 080D 080E 080F",
  "0901 0902 0903 0904 0905 0906 0907 0908 0909 090A 090B 090C 090D 090E 090F",
  "0A01 0A02 0A03 0A04 0A05 0A06 0A07 0A08 0A09 0A0A 0A0B 0A0C 0A0D 0A0E 0A0F",
  "0B01 0B02 0B03 0B04 0B05 0B06 0B07 0B08 0B09 0B0A 0B0B 0B0C 0B0D 0B0E 0B0F",
  "0C01 0C02 0C03 0C04 0C05 0C06 0C07 0C08 0C09 0C0A 0C0B 0C0C 0C0D 0C0E 0C0F",
  "0D01 0D02 0D03 0D04 0D05 0D06 0D07 0D08 0D09 0D0A 0D0B 0D0C 0D0D 0D0E 0D0F",
  "0E01 0E02 0E03 0E04 0E05 0E06 0E07 0E08 0E09 0E0A 0E0B 0E0C 0E0D 0E0E 0E0F",
  "0F01 0F02 0F03 0F04 0F05 0F06 0F07 0F08 0F09 0F0A 0F0B 0F0C 0F0D 0F0E 0F0F",
  "0001 0002 0003 0004 0005 0006 0007 0008 0009 000A 000B 000C 000D 000E 000F");
  
@rt49 =
 ("1111 1112 1113 1114 1115 1116 1117 1118 1119 111A 111B 111C 111D 111E 111F",
  "2221 2222 2223 2224 2225 2226 2227 2228 2229 222A 222B 222C 222D 222E 222F",
  "3331 3332 3333 3334 3335 3336 3337 3338 3339 333A 333B 333C 333D 333E 333F",
  "4441 4442 4443 4444 4445 4446 4447 4448 4449 444A 444B 444C 444D 444E 444F",
  "5551 5552 5553 5554 5555 5556 5557 5558 5559 555A 555B 555C 555D 555E 555F",
  "6661 6662 6663 6664 6665 6666 6667 6668 6669 666A 666B 666C 666D 666E 666F",
  "7771 7772 7773 7774 7775 7776 7777 7778 7779 777A 777B 777C 777D 777E 777F",
  "8881 8882 8883 8884 8885 8886 8887 8888 8889 888A 888B 888C 888D 888E 888F",
  "9991 9992 9993 9994 9995 9996 9997 9998 9999 999A 999B 999C 999D 999E 999F",
  "AAA1 AAA2 AAA3 AAA4 AAA5 AAA6 AAA7 AAA8 AAA9 AAAA AAAB AAAC AAAD AAAE AAAF",
  "BBB1 BBB2 BBB3 BBB4 BBB5 BBB6 BBB7 BBB8 BBB9 BBBA BBBB BBBC BBBD BBBE BBBF",
  "CCC1 CCC2 CCC3 CCC4 CCC5 CCC6 CCC7 CCC8 CCC9 CCCA CCCB CCCC CCCD CCCE CCCF",
  "DDD1 DDD2 DDD3 DDD4 DDD5 DDD6 DDD7 DDD8 DDD9 DDDA DDDB DDDC DDDD DDDE DDDF",
  "EEE1 EEE2 EEE3 EEE4 EEE5 EEE6 EEE7 EEE8 EEE9 EEEA EEEB EEEC EEED EEEE EEEF",
  "FFF1 FFF2 FFF3 FFF4 FFF5 FFF6 FFF7 FFF8 FFF9 FFFA FFFB FFFC FFFD FFFE FFFF",
  "0001 0002 0003 0004 0005 0006 0007 0008 0009 000A 000B 000C 000D 000E 000F");
  
$irt48 = $irt49 = 0;


# Cycle de verification
for ($i=0; $i<$cycles; $i++)
  { $a = <FICH> or goto FINI_TROP_TOT;
    for (; $a !~ m/- BCRT RT4,7 n=2 err=/; )
       { print $a;
         $a = <FICH> or goto FINI_TROP_TOT;
       }
    $b = <FICH> or goto FINI_TROP_TOT;
    if ($i != 0)
      { # Controle des donnees "flux en sortie"
        ($v1, $v2) = split " ", $b;
        $v1 = oct("0x"."$v1");
        $v2 = oct("0x"."$v2");
        $w1 = (($i-1)*2) & 0xFFFF;
        $w2 = ($w1 + 1) & 0xFFFF;
        if (    ($v1 != $w1)
             || ($v2 != $w2))
          {  print "i=$i v1=$v1 v2=$v2 w1=",
                                  $w1, " w2=", $w2, "\n";
            print "*** ".$a;
            print "*** ".$b;
            print "***\n";
          }
      }
    $a = <FICH> or goto FINI_TROP_TOT;

    $a = <FICH> or goto FINI_TROP_TOT;
    for (; $a !~ m/- RTBC RT4,8 n=15 err=/; )
       { print $a;
         $a = <FICH> or goto FINI_TROP_TOT;
       }
    $b = <FICH> or goto FINI_TROP_TOT;
    if ($i != 0)
      { # Controle des donnees "flux en sortie"
        if ($b !~ $rt48[$irt48])
          { print ">>>>>>> i=$i\n";
            print ">>> ".$a;
            print ">>> ".$b;
            print ">>> (".$rt48[$irt48].")\n";
            print ">>>\n";
          }
        if (++$irt48 > $#rt48) { $irt48 = 0; };
      }
    $a = <FICH> or goto FINI_TROP_TOT;

    $a = <FICH> or goto FINI_TROP_TOT;
    for (; $a !~ m/- RTBC RT4,9 n=15 err=/; )
       { print $a;
         $a = <FICH> or goto FINI_TROP_TOT;
       }
    $b = <FICH> or goto FINI_TROP_TOT;
    if ($i != 0)
      { # Controle des donnees "flux en sortie"
        if ($b !~ $rt49[$irt49])
          { print ">>>>>>> i=$i\n";
            print ">>> ".$a;
            print ">>> ".$b;
            print ">>> (".$rt49[$irt49].")\n";
            print ">>>\n";
          }
        if (++$irt49 > $#rt49) { $irt49 = 0; };
      }
    $a = <FICH> or goto FINI_TROP_TOT;
  }

exit 0;

FINI_TROP_TOT :
print "Fin du fichier inattendue !\n";
