WeChat: cstutorcs
QQ: 749389476
Email: tutorcs@163.com
	XOR	R0 R0 R0
	ADD	R1 R0 R0 
        ADDI	R2 R0 6 
T1:	LW	R3 0(R1)
	ADD	R3 R3 R5
	SUBI	R2 R2 1
	SW	R3 0(R1)
	ADDI 	R1 R1 4
	BNEZ	R2 T1	
	EOP
