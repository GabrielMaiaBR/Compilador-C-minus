* TINY Compilation to TM Code
* Standard prelude:
  0:     LD  6,0(0) 	load maxaddress from location 0
  1:     LD  2,0(0) 	load maxaddress from location 0
  2:     ST  0,0(0) 	clear location 0
* End of standard prelude.
* -> Init Function (main)
  3:    LDA  7,0(7) 	jump to main
* -> declare var
* <- declare var
* -> assign
* -> Op
* -> Const
  4:    LDC  0,5(0) 	load const
* <- Const
  5:     ST  0,-3(2) 	op: push left
* -> Const
  6:    LDC  0,3(0) 	load const
* <- Const
  7:     LD  1,-3(2) 	op: load left
  8:    ADD  0,1,0 	op +
* <- Op
  9:     ST  0,-2(2) 	assign: store value
* <- assign
* <- End Function
* End of execution.
 10:   HALT  0,0,0 	
