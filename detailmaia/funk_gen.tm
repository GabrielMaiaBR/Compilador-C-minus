* TINY Compilation to TM Code
* Standard prelude:
  0:     LD  6,0(0) 	load maxaddress from location 0
  1:     LD  2,0(0) 	load maxaddress from location 0
  2:     ST  0,0(0) 	clear location 0
* End of standard prelude.
* -> Init Function (test)
  4:     ST  0,-1(2) 	store return address from ac
* -> return
* -> Const
  5:    LDC  0,0(0) 	load const
* <- Const
  6:    LDA  1,0(2) 	save current fp into ac1
  7:     LD  2,0(2) 	make fp = ofp
  8:     LD  7,-1(1) 	return to caller
* <- return
* <- End Function
* -> Init Function (main)
  3:    LDA  7,5(7) 	jump to main
* <- End Function
* End of execution.
  9:   HALT  0,0,0 	
