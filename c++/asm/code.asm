WeChat: cstutorcs
QQ: 749389476
Email: tutorcs@163.com
	XOR	R0 R0 R0
	XOR	R1 R1 R1
	ADDI	R2 R0 0xA000
	ADDI	R3 R0 0xB020
T1:	LW	R4 0(R2)
	BLTZ	R4 T2	
	SW	R4 0(R3)
	SUBI	R3 R3 0x4 	
T2:	ADD	R1 R1 R4
     	ADDI	R2 R2 0x4
   	SUBI	R5 R2 0XA020
     	BNEZ	R5 T1
     	SW  	R1 0(R2)
	SW	R3 4(R2)
	EOP