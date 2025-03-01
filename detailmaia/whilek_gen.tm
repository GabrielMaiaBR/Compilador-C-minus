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
  4:    LDC  0,0(0) 	load const
* <- Const
  5:     ST  0,-2(2) 	assign: store value
* <- assign
* -> while
* repeat: jump after body comes back here
* -> Op
* -> Id
  6:     LD  0,-2(2) 	load id value
* <- Id
  7:     ST  0,-3(2) 	op: push left
* -> Const
  8:    LDC  0,5(0) 	load const
* <- Const
  9:     LD  1,-3(2) 	op: load left
 10:    SUB  0,1,0 	op <
 11:    JLT  0,2(7) 	br if true
 12:    LDC  0,0(0) 	false case
 13:    LDA  7,1(7) 	unconditional jmp
 14:    LDC  0,1(0) 	true case
* <- Op
* -> assign
* -> Op
* -> Id
 16:     LD  0,-2(2) 	load id value
* <- Id
 17:     ST  0,-3(2) 	op: push left
* -> Const
 18:    LDC  0,1(0) 	load const
* <- Const
 19:     LD  1,-3(2) 	op: load left
 20:    ADD  0,1,0 	op +
* <- Op
 21:     ST  0,-2(2) 	assign: store value
* <- assign
 22:    LDA  7,-17(7) 	jump to savedLoc1
 15:    JEQ  0,7(7) 	repeat: jmp to end
* <- repeat
* <- End Function
* End of execution.
 23:   HALT  0,0,0 	
