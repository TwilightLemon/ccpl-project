	# Program Header
	LOD R2,STACK
	STO (R2),0
	LOD R4,EXIT
	STO (R2+4),R4

	# label func
func:

	# begin

	# formal a

	# formal b

	# output a
	LOD R5,(R2-4)
	LOD R15,R5
	OTI

	# output b
	LOD R6,(R2-8)
	LOD R15,R6
	OTI

	# output L1
	LOD R7,L1
	LOD R15,R7
	OTS

	# var t0 : int

	# t0 = (a > 0)
	LOD R8,0
	SUB R5,R8
	TST R5
	LOD R3,R1+40
	JGZ R3
	LOD R5,0
	LOD R3,R1+24
	JMP R3
	LOD R5,1

	# ifz t0 goto L2
	STO (R2+8),R5
	TST R5
	JEZ L2

	# return b
	LOD R4,(R2-8)
	LOD R3,(R2+4)
	LOD R2,(R2)
	JMP R3

	# label L2
L2:

	# return '?'
	LOD R4,63
	LOD R3,(R2+4)
	LOD R2,(R2)
	JMP R3

	# end
	LOD R3,(R2+4)
	LOD R2,(R2)
	JMP R3

	# label func2
func2:

	# begin

	# formal b

	# end
	LOD R3,(R2+4)
	LOD R2,(R2)
	JMP R3

	# label main
main:

	# begin

	# var a : int

	# var b : int

	# var c : int

	# var d : int

	# input a
	LOD R5,(R2+8)
	ITI
	LOD R5,R15

	# var t1 : int

	# t1 = a + 1
	STO (R2+8),R5
	LOD R6,1
	ADD R5,R6

	# b = t1
	STO (R2+24),R5

	# var t2 : int

	# t2 = b + 2
	STO (R2+12),R5
	LOD R7,2
	ADD R5,R7

	# c = t2
	STO (R2+28),R5

	# var t3 : int

	# t3 = -c
	LOD R8,0
	STO (R2+16),R5
	SUB R8,R5

	# d = t3
	STO (R2+32),R8

	# output d
	STO (R2+20),R8
	LOD R15,R8
	OTI

	# output L1
	LOD R9,L1
	LOD R15,R9
	OTS

	# var r : char

	# var t4 : char

	# actual a
	LOD R10,(R2+8)
	STO (R2+44),R10

	# actual 'x'
	LOD R11,120
	STO (R2+48),R11

	# t4 = call func
	STO (R2+52),R2
	LOD R4,R1+32
	STO (R2+56),R4
	LOD R2,R2+52
	JMP func
	LOD R5,(R2+40)
	LOD R5,R4

	# r = t4
	STO (R2+40),R5

	# output r
	STO (R2+36),R5
	LOD R15,R5
	OTI

	# return 0
	LOD R4,0
	LOD R3,(R2+4)
	LOD R2,(R2)
	JMP R3

	# end
	LOD R3,(R2+4)
	LOD R2,(R2)
	JMP R3

	# Program Exit
EXIT:
	END
L1:
	DBS 10,0
STATIC:
	DBN 0,0
STACK:
