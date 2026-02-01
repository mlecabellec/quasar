
### Ouverture du fichier a controler :

# Lecture de tout le fichier dans a
$fichier = $ARGV[0];
open(FICH, $fichier) or die "Impossible d\'ouvrir $fichier !\n";



# Cycle de verification
for ($i=0; $i<10000; $i++)
  { ($a = <FICH>) or goto FINI_TROP_TOT;
    while ($a =! m/- BCRT RT4,7 n=2 err=/)
       { print $a;
         ($a = <FICH>) or goto FINI_TROP_TOT;
       }
    ($a = <FICH>) or goto FINI_TROP_TOT;
    ($a = <FICH>) or goto FINI_TROP_TOT;

    ($a = <FICH>) or goto FINI_TROP_TOT;
    while ($a =! m/- RTBC RT4,8 n=15 err=/)
       { print $a;
         ($a = <FICH>) or goto FINI_TROP_TOT;
       }
    ($a = <FICH>) or goto FINI_TROP_TOT;
    ($a = <FICH>) or goto FINI_TROP_TOT;

    ($a = <FICH>) or goto FINI_TROP_TOT;
    while ($a =! m/- RTBC RT4,9 n=15 err=/)
       { print $a;
         ($a = <FICH>) or goto FINI_TROP_TOT;
       }
    ($a = <FICH>) or goto FINI_TROP_TOT;
    ($a = <FICH>) or goto FINI_TROP_TOT;
  }

exit 0;

FINI_TROP_TOT :
print "Fin du fichier inattendue !\n";
