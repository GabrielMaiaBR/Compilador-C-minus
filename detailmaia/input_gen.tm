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
* -> Function Call (input)
  4:     IN  0,0,0 	read input
  5:     ST  0,-2(2) 	assign: store value
* <- assign
* <- End Function
* End of execution.
  6:   HALT  0,0,0 	
