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
* -> Const
  4:    LDC  0,1(0) 	load const
* <- Const
  5:     ST  0,-2(2) 	assign: store value
* <- assign
* -> if
* -> Op
* -> Id
  6:     LD  0,-2(2) 	load id value
* <- Id
  7:     ST  0,-3(2) 	op: push left
* -> Const
  8:    LDC  0,2(0) 	load const
* <- Const
  9:     LD  1,-3(2) 	op: load left
 10:    SUB  0,1,0 	op <
 11:    JLT  0,2(7) 	br if true
 12:    LDC  0,0(0) 	false case
 13:    LDA  7,1(7) 	unconditional jmp
 14:    LDC  0,1(0) 	true case
* <- Op
* if: jump to else belongs here
* -> assign
* -> Const
 16:    LDC  0,3(0) 	load const
* <- Const
 17:     ST  0,-2(2) 	assign: store value
* <- assign
* if: jump to end belongs here
 15:    JEQ  0,3(7) 	if: jmp to else
 18:    LDA  7,0(7) 	jmp to end
* <- if
* <- End Function
* End of execution.
 19:   HALT  0,0,0 	
