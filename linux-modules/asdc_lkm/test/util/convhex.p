module convhex;

/***************************************************************************/
/*	Fichier : convhex.p						   */
/*      PROCEDURES DE CONVERSION 				           */
/*      DESTINE A ETRE APPELEE PAR UN PROGRAMME PASCAL		           */
/***************************************************************************/
/***************************************************************************/
/*     Quittard/Clement  le : 05/01/1993                                   */
/*     Version     	    : 1.1                                          */
/***************************************************************************/

#include <TLPC/typepas.i>

type
        conversion_2= record
        case integer of 
                1 : (ic  : integer16);
                2 : (ib  : array[1..2] of char);
                3 : (io  : array[1..2] of integer8);
        end;

        conversion_4= record
        case integer of 
                1 : (il  : integer32);
                2 : (ic  : array[1..2] of integer16);
                3 : (ib  : array[1..4] of char);
                4 : (io  : array[1..4] of integer8);
        end;



(* ============================================================ *)
(* Conversion d'un hexadecimal dans string de 80 en entier long *)
(* ============================================================ *)

function convchex(var ch : chaine80;var val :integer32) : integer;
	(* convchex = 0 si OK   -1 si erreur *)
	(* val = 0 si erreur *)
var 
	i,k : integer;

begin
	val := 0;
	convchex := 0;
	for i := 1 to length(ch) do
		begin
	if (ch[i] = ' ') then next;
	k := ord(ch[i]);
	if((k>=48) and (k<=57)) then  val := val * 16 + (k-48)
				else
	 if ((k>=65) and (k<=70)) then  val := val * 16 + (k-55)
				else
	  if ((k>=97) and (k<=102)) then  val := val * 16 + (k-87)
				else begin convchex := -1; exit; end;
 		end;    

end;

(* ============================================================= *)
(* Convertion entier 32 bits en hexa string [10]		 *)
(* ============================================================= *)

procedure conv32hex(n : integer32; var s : chaine10);
var
	x : conversion_4;
	i,l,k : integer32;

begin
	s := '        ';
	l := 1;
	x.il := n;
	for i := 1 to 4 do begin
	k := ord(x.ib[i]) div 16;
	if ((k >= 0) and (k <= 9)) then s[l] := chr(k + 48)
                                 else
	if ((k > 9) and (k <= 15)) then s[l] := chr(k + 55);
	l := l + 1; 
	k :=  ord(x.ib[i]) mod 16;
	if ((k >= 0) and (k <= 9)) then s[l] := chr(k + 48)
                                 else
	if ((k > 9) and (k <= 15)) then s[l] := chr(k + 55);
	l := l + 1;
			   end;
	s[l] := chr(0);
end;

(* ============================================================= *)
(* Convertion entier 16 bits en hexa string [10] 		 *)
(* ============================================================= *)

procedure conv16hex(n : integer16; var s : chaine10);
var
	x : conversion_2;
	i,k,l : integer32;

begin
	s := '    ';
	l := 1;
	x.ic := n;
	for i := 1 to 2 do begin
	k := ord(x.ib[i]) div 16;
	if ((k >= 0) and (k <= 9)) then s[l] := chr(k + 48)
                                 else
	if ((k > 9) and (k <= 15)) then s[l] := chr(k + 55);
	l := l + 1; 
	k :=  ord(x.ib[i]) mod 16;
	if ((k >= 0) and (k <= 9)) then s[l] := chr(k + 48)
                                 else
	if ((k > 9) and (k <= 15)) then s[l] := chr(k + 55);
	l := l + 1;
	s[l] := chr(0);
			   end;
end;

(* ============================================================= *)
(* Convertion entier 8 bits en hexa string [10] 		 *)
(* ============================================================= *)

procedure conv8hex(n : integer8; var s : chaine10);
var
	k,l,m : integer32;

begin
	s := '  ';
	l := 1;
	m := n & 16#00ff;
	k := m div 16;
	if ((k >= 0) and (k <= 9)) then s[l] := chr(k + 48)
                                 else
	if ((k > 9) and (k <= 15)) then s[l] := chr(k + 55);
	l := l + 1; 
	k :=  m mod 16;
	if ((k >= 0) and (k <= 9)) then s[l] := chr(k + 48)
                                 else
	if ((k > 9) and (k <= 15)) then s[l] := chr(k + 55);
	l := l + 1;
	s[l] := chr(0);
end;


