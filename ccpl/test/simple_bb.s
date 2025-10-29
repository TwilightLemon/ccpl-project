	# Program Header
	LOD R2,STACK
	STO (R2),0
	LOD R4,EXIT
	STO (R2+4),R4

	# label add
add:

	# begin

	# formal a

	# formal b

	# var t0 : int

	# t0 = a + b
	LOD R5,(R2-4)
	LOD R6,(R2-8)
	ADD R5,R6

	# return t0
	STO (R2+8),R5
	LOD R4,(R2+8)
	LOD R3,(R2+4)
	LOD R2,(R2)
	JMP R3

	# end
	LOD R3,(R2+4)
	LOD R2,(R2)
	JMP R3

	# var x : int

	# var y : int

	# var z : int

	# label main
main:

	# begin

	# x = 5
	LOD R5,5

	# y = 10
	LOD R6,10

	# var t1 : int

	# t1 = (x > 3)
	LOD R4,STATIC
	STO (R4+0),R5
	LOD R7,3
	SUB R5,R7
	TST R5
	LOD R3,R1+40
	JGZ R3
	LOD R5,0
	LOD R3,R1+24
	JMP R3
	LOD R5,1

	# ifz t1 goto L1
	STO (R2+8),R5
	LOD R4,STATIC
	STO (R4+4),R6
	TST R5
	JEZ L1

	# var t2 : int

	# actual x
	LOD R4,STATIC
	LOD R8,(R4+0)
	STO (R2+16),R8

	# actual y
	STO (R2+20),R6

	# t2 = call add
	STO (R2+24),R2
	LOD R4,R1+32
	STO (R2+28),R4
	LOD R2,R2+24
	JMP add
	LOD R5,(R2+12)
	LOD R5,R4

	# z = t2
	STO (R2+12),R5

	# goto L2
	LOD R4,STATIC
	STO (R4+8),R5
	JMP L2

	# label L1
L1:

	# z = 0
	LOD R5,0

	# label L2
	LOD R4,STATIC
	STO (R4+8),R5
L2:

	# output z
	LOD R4,STATIC
	LOD R5,(R4+8)
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
STATIC:
	DBN 0,12
STACK:
