	# Program Header
	LOD R2,STACK
	STO (R2),0
	LOD R4,EXIT
	STO (R2+4),R4

	# label main
main:

	# begin

	# output L1
	LOD R5,L1
	LOD R15,R5
	OTS

	# output L2
	LOD R6,L2
	LOD R15,R6
	OTS

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
L2:
	DBS 10,0
L1:
	DBS 72,101,108,108,111,0
STATIC:
	DBN 0,0
STACK:
