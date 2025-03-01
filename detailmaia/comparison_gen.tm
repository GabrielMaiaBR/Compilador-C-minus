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
  4:    LDC  0,5(0) 	load const
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
  8:    LDC  0,10(0) 	load const
* <- Const
  9:     LD  1,-3(2) 	op: load left
 10:    SUB  0,1,0 	op <
 11:    JLT  0,2(7) 	br if true
 12:    LDC  0,0(0) 	false case
 13:    LDA  7,1(7) 	unconditional jmp
 14:    LDC  0,1(0) 	true case
* <- Op
* if: jump to else belongs here
* -> Function Call (output)
* -> Const
 16:    LDC  0,1(0) 	load const
* <- Const
 17:    OUT  0,0,0 	print value
* if: jump to end belongs here
 15:    JEQ  0,3(7) 	if: jmp to else
 18:    LDA  7,0(7) 	jmp to end
* <- if
* -> if
* -> Op
* -> Id
 19:     LD  0,-2(2) 	load id value
* <- Id
 20:     ST  0,-3(2) 	op: push left
* -> Const
 21:    LDC  0,5(0) 	load const
* <- Const
 22:     LD  1,-3(2) 	op: load left
 23:    SUB  0,1,0 	op <=
 24:    JLE  0,2(7) 	br if true
 25:    LDC  0,0(0) 	false case
 26:    LDA  7,1(7) 	unconditional jmp
 27:    LDC  0,1(0) 	true case
* <- Op
* if: jump to else belongs here
* -> Function Call (output)
* -> Const
 29:    LDC  0,1(0) 	load const
* <- Const
 30:    OUT  0,0,0 	print value
* if: jump to end belongs here
 28:    JEQ  0,3(7) 	if: jmp to else
 31:    LDA  7,0(7) 	jmp to end
* <- if
* -> if
* -> Op
* -> Id
 32:     LD  0,-2(2) 	load id value
* <- Id
 33:     ST  0,-3(2) 	op: push left
* -> Const
 34:    LDC  0,3(0) 	load const
* <- Const
 35:     LD  1,-3(2) 	op: load left
 36:    SUB  0,1,0 	op >
 37:    JGT  0,2(7) 	br if true
 38:    LDC  0,0(0) 	false case
 39:    LDA  7,1(7) 	unconditional jmp
 40:    LDC  0,1(0) 	true case
* <- Op
* if: jump to else belongs here
* -> Function Call (output)
* -> Const
 42:    LDC  0,1(0) 	load const
* <- Const
 43:    OUT  0,0,0 	print value
* if: jump to end belongs here
 41:    JEQ  0,3(7) 	if: jmp to else
 44:    LDA  7,0(7) 	jmp to end
* <- if
* -> if
* -> Op
* -> Id
 45:     LD  0,-2(2) 	load id value
* <- Id
 46:     ST  0,-3(2) 	op: push left
* -> Const
 47:    LDC  0,5(0) 	load const
* <- Const
 48:     LD  1,-3(2) 	op: load left
 49:    SUB  0,1,0 	op >=
 50:    JGE  0,2(7) 	br if true
 51:    LDC  0,0(0) 	false case
 52:    LDA  7,1(7) 	unconditional jmp
 53:    LDC  0,1(0) 	true case
* <- Op
* if: jump to else belongs here
* -> Function Call (output)
* -> Const
 55:    LDC  0,1(0) 	load const
* <- Const
 56:    OUT  0,0,0 	print value
* if: jump to end belongs here
 54:    JEQ  0,3(7) 	if: jmp to else
 57:    LDA  7,0(7) 	jmp to end
* <- if
* -> if
* -> Op
* -> Id
 58:     LD  0,-2(2) 	load id value
* <- Id
 59:     ST  0,-3(2) 	op: push left
* -> Const
 60:    LDC  0,5(0) 	load const
* <- Const
 61:     LD  1,-3(2) 	op: load left
 62:    SUB  0,1,0 	op ==
 63:    JEQ  0,2(7) 	br if true
 64:    LDC  0,0(0) 	false case
 65:    LDA  7,1(7) 	unconditional jmp
 66:    LDC  0,1(0) 	true case
* <- Op
* if: jump to else belongs here
* -> Function Call (output)
* -> Const
 68:    LDC  0,1(0) 	load const
* <- Const
 69:    OUT  0,0,0 	print value
* if: jump to end belongs here
 67:    JEQ  0,3(7) 	if: jmp to else
 70:    LDA  7,0(7) 	jmp to end
* <- if
* -> if
* -> Op
* -> Id
 71:     LD  0,-2(2) 	load id value
* <- Id
 72:     ST  0,-3(2) 	op: push left
* -> Const
 73:    LDC  0,6(0) 	load const
* <- Const
 74:     LD  1,-3(2) 	op: load left
 75:    SUB  0,1,0 	op !=
 76:    JNE  0,2(7) 	br if true
 77:    LDC  0,0(0) 	false case
 78:    LDA  7,1(7) 	unconditional jmp
 79:    LDC  0,1(0) 	true case
* <- Op
* if: jump to else belongs here
* -> Function Call (output)
* -> Const
 81:    LDC  0,1(0) 	load const
* <- Const
 82:    OUT  0,0,0 	print value
* if: jump to end belongs here
 80:    JEQ  0,3(7) 	if: jmp to else
 83:    LDA  7,0(7) 	jmp to end
* <- if
* <- End Function
* End of execution.
 84:   HALT  0,0,0 	
