	.text
	.p2align 3
	.globl	subEnemyCollision
	.ent	subEnemyCollision
subEnemyCollision:
	.frame	$sp,288,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu     $29, $29, -0x120
	lui       $2, 0x56
	.word	0xE7B40110
	.word	0xFFB200F0
	.word	0xFFB100E0
	addiu     $18, $2, 0x5060
	.word	0xFFBF0100
	addiu     $17, $0, 0x190
	.word	0xFFB000D0
	sw        $4, ($29)
	lui       $1, 0x40a0
	.word	0x4481A000
	lw        $16, ($29)
	addiu     $4, $29, 0x10
	addiu     $6, $0, 0x2c
	lw        $2, 0x15c($16)
	lw        $3, 0x4a0($2)
	.word	0x00711018
	addu      $3, $2, $18
	lw        $2, 0x188($3)
	andi      $2, $2, 0x1
	.word	0x1040000F
	.word	0x0200282D
	.word	0x0C052840
	nop
	addiu     $4, $29, 0x20
	addiu     $6, $0, 0x33
	.word	0x0C052840
	.word	0x0200282D
	.word	0xC7A00024
	addiu     $4, $29, 0x10
	.word	0x46140001
	.word	0x0C05A14E
	.word	0xE7A00024
	lw        $2, 0xa4($29)
	.word	0x14400002
	addiu     $3, $0, 0x1
	.word	0x0000182D
	.word	0x10600004
	addiu     $5, $0, 0x9d
	lw        $4, ($29)
	.word	0x0C056F32
	nop
	.word	0x0C080EA8
	addiu     $4, $0, 0x1
	.word	0x1000FFDE
	.set	macro
	.set	reorder
	.set	at
	.end	subEnemyCollision
