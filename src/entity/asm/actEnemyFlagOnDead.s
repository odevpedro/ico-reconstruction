	.text
	.p2align 3
	.globl	actEnemyFlagOnDead
	.ent	actEnemyFlagOnDead
actEnemyFlagOnDead:
	.frame	$sp,32,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu     $29, $29, -0x20
	sw        $4, ($29)
	lui       $4, 0x56
	.word	0xFFBF0010
	.word	0x0C069B8A
	addiu     $4, $4, -0x7740
	.word	0x0C080EA8
	.word	0x0000202D
	.word	0xDFBF0010
	.word	0x03E00008
	addiu     $29, $29, 0x20
	.set	macro
	.set	reorder
	.set	at
	.end	actEnemyFlagOnDead
