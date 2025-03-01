* TINY Compilation to TM Code
* Standard prelude:
  0:     LD  6,0(0) 	load maxaddress from location 0
  1:     LD  2,0(0) 	load maxaddress from location 0
  2:     ST  0,0(0) 	clear location 0
* End of standard prelude.
* -> declare vector
  3:    LDC  0,10(0) 	load global position to ac
  4:    LDC  5,0(0) 	load 0
  5:     ST  0,10(5) 	store ac in global_position_aux
* <- declare vector
* -> Init Function (main)
  6:    LDA  7,0(7) 	jump to main
* -> assign vector
* -> Vector
* -> Const
  7:    LDC  0,5(0) 	load const
* <- Const
  8:    LDC  5,0(0) 	load 0
  9:     LD  1,10(5) 	get the address of the vector
 10:    LDC  3,0(0) 	get the value of the index
 11:    LDC  4,1(0) 	load 1
 12:    ADD  3,3,4 	sub 3 by 1
 13:    SUB  1,1,3 	get the address
 14:     ST  0,0(1) 	get the value of the vector
* <- End Function
* End of execution.
 15:   HALT  0,0,0 	
