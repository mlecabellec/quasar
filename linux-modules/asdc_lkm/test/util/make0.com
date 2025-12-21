$ set verif
$ cc BINA.C           
$! cc CHERCHE.C        
$! cc CHERCHE1.C       
$! cc COMPR.C          
$! cc DATE.C           
$! cc HEURE.C          
$ cc HEXA.C           
$ cc INDEX.C          
$! cc TEMPS.C          
$ cc TRIM.C           
$ cc TRIM_DRO.C       
$ cc TRIM_GAU.C       
$ lib/create [-]cutil.olb *.obj
$ set noverif
