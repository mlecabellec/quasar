for ($i=0; $i<16; $i++)
  { print "       DONN = ";
    for ($j=1; $j<16; $j++)
       { printf "0x%04X", 0x100*$i+$j; if ($j<15) { print ", "; }
         if ($j==8) { print "\n              "; }
       }
    print ";\n";
  }

printf("\n\n\n");

for ($i=0; $i<16; $i++)
  { print "       DONN = ";
    for ($j=1; $j<16; $j++)
       { printf "0x%04X", 0x1110*$i+$j; if ($j<15) { print ", "; }
         if ($j==8) { print "\n              "; }
       }
    print ";\n";
  }
