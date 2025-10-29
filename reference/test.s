	# head
	LOD R2,STACK
	STO (R2),0
	LOD R4,EXIT
	STO (R2+4),R4

	# label main
main:

	# begin

	# var i

	# input i
	LOD R5,(R2+8)
	ITI
	LOD R5,R15

	# label L1
	STO (R2+8),R5
L1:

	# var t0

	# t0 = (i < 10)
	LOD R5,(R2+8)
	LOD R6,10
	SUB R5,R6
	TST R5
	LOD R3,R1+40
	JLZ R3
	LOD R5,0
	LOD R3,R1+24
	JMP R3
	LOD R5,1

	# ifz t0 goto L2
	STO (R2+12),R5
	TST R5
	JEZ L2

	# output i
	LOD R7,(R2+8)
	LOD R15,R7
	OTI

	# var t1

	# t1 = i + 1
	LOD R8,1
	ADD R7,R8

	# i = t1
	STO (R2+16),R7

	# goto L1
	STO (R2+8),R7
	JMP L1

	# label L2
L2:

	# output L3
	LOD R5,L3
	LOD R15,R5
	OTS

	# end
	LOD R3,(R2+4)
	LOD R2,(R2)
	JMP R3

	# tail
EXIT:
	END
L3:
	DBS 10,0
STATIC:
	DBN 0,0
STACK:
