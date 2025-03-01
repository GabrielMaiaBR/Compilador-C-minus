* TINY Compilation to TM Code
* Standard prelude:
  0:     LD  6,0(0) 	load maxaddress from location 0
  1:     LD  2,0(0) 	load maxaddress from location 0
  2:     ST  0,0(0) 	clear location 0
* End of standard prelude.
* -> Init Function (test)
  4:     ST  0,-1(2) 	store return address from ac
* -> Param
* <- Param
* -> Function Call (output)
* -> Id
  5:     LD  0,-2(2) 	load id value
* <- Id
  6:    OUT  0,0,0 	print value
  7:    LDA  1,0(2) 	save current fp into ac1
  8:     LD  2,0(2) 	make fp = ofp
  9:     LD  7,-1(1) 	return to caller
* <- End Function
* -> Init Function (main)
  3:    LDA  7,6(7) 	jump to main
* -> Function Call (test)
 10:     ST  2,-2(2) 	Guard fp
* -> Const
 11:    LDC  0,42(0) 	load const
* <- Const
 12:     ST  0,-4(2) 	Store value of func argument
 13:    LDA  2,-2(2) 	change fp
 14:    LDC  0,16(0) 	Guard return adress
 15:    LDA  7,-12(7) 	jump to function
* <- Function Call
* <- End Function
* End of execution.
 16:   HALT  0,0,0 	
