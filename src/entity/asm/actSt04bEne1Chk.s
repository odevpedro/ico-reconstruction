	.text
	.p2align 3
	.globl	actSt04bEne1Chk
	.ent	actSt04bEne1Chk
actSt04bEne1Chk:
	.frame	$sp,16,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu     $29, $29, -0x10
	.word	0x0080382D
	.word	0xFFBF0000
	.word	0x00A0402D
	lw        $3, 0x164($7)
	lw        $2, 0x12c($3)
	.word	0x14400007
	.word	0x00C0282D
	sw        $5, 0x130($3)
	.word	0x0100302D
	.word	0x0C04FFE2
	sw        $7, 0x12c($3)
	.word	0x10000002
	addiu     $2, $0, 0x1
	.word	0x0000102D
	.word	0xDFBF0000
	.word	0x03E00008
	addiu     $29, $29, 0x10
	.set	macro
	.set	reorder
	.set	at
	.end	actSt04bEne1Chk
