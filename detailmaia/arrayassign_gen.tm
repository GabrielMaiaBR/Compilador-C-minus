* TINY Compilation to TM Code
* Standard prelude:
  0:     LD  6,0(0) 	load maxaddress from location 0
  1:     LD  2,0(0) 	load maxaddress from location 0
  2:     ST  0,0(0) 	clear location 0
* End of standard prelude.
* -> Init Function (main)
  3:    LDA  7,0(7) 	jump to main
* -> declare vector
  4:    LDA  0,-2(2) 	guard addr of vector
  5:     ST  0,-2(2) 	store addr of vector
* <- declare vector
* -> assign vector
* -> Vector
* -> Const
  6:    LDC  0,1(0) 	load const
* <- Const
  7:     LD  1,-2(2) 	get the address of the vector
  8:    LDC  3,0(0) 	get the value of the index
  9:    LDC  4,1(0) 	load 1
 10:    ADD  3,3,4 	sub 3 by 1
 11:    SUB  1,1,3 	get the address
 12:     ST  0,0(1) 	get the value of the vector
* <- End Function
* End of execution.
 13:   HALT  0,0,0 	
