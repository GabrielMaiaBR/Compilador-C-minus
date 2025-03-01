* TINY Compilation to TM Code
* Standard prelude:
  0:     LD  6,0(0) 	load maxaddress from location 0
  1:     LD  2,0(0) 	load maxaddress from location 0
  2:     ST  0,0(0) 	clear location 0
* End of standard prelude.
* -> declare var
* <- declare var
* -> Init Function (main)
  3:    LDA  7,0(7) 	jump to main
* -> assign
* -> Const
  4:    LDC  0,5(0) 	load const
* <- Const
  5:     ST  0,-1023(2) 	assign: store value
* <- assign
* <- End Function
* End of execution.
  6:   HALT  0,0,0 	
