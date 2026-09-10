	.text
	.p2align 3
	.globl	HoldRope
	.ent	HoldRope
HoldRope:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x45000002
	.word	0xE780AC5C
	.word	0xE782AC5C
	.word	0x0C079260
	.word	0xC78CAC5C
	.word	0x0000202D
	.word	0x0000282D
	.word	0x0C09395E
	addiu     $6, $29, 0x100
	lw        $3, 0x5250($17)
	andi      $2, $3, 0x8
	.word	0x1040000F
	lbu       $2, 0x111($29)
	lui       $1, 0x437f
	.word	0x44811000
	.word	0x44820000
	.word	0x46800020
	lui       $1, 0x3f80
	.word	0x44810800
	nop
	nop
	.word	0x46020003
	.word	0x46000841
	.word	0x46000B06
	.word	0x0C079260
	.word	0xE781AC5C
	lw        $3, 0x5250($18)
	andi      $2, $3, 0x2
	.word	0x1040000D
	lbu       $2, 0x113($29)
	lui       $1, 0x3c00
	.word	0x44811000
	.word	0x44820000
	.word	0x46800020
	lui       $1, 0x3f80
	.word	0x44810800
	lw        $4, -0x53b0($28)
	.word	0x46020002
	lw        $2, 0x15c($4)
	.word	0x46000841
	.word	0x10000006
	.word	0xE44100F4
	lw        $4, -0x53b0($28)
	lui       $1, 0x3f80
	.word	0x44810000
	lw        $2, 0x15c($4)
	.word	0xE44000F4
	lw        $3, 0x5250($17)
	andi      $2, $3, 0x8000
	.word	0x10400009
	lbu       $2, 0x109($29)
	lui       $1, 0x437f
	.word	0x44811000
	.word	0x44820000
	.word	0x46800020
	lui       $1, 0x4600
	.word	0x44810800
	.word	0x1000000B
	lw        $3, 0x15c($4)
	andi      $2, $3, 0x2000
	.word	0x10400010
	lbu       $2, 0x108($29)
	lui       $1, 0x437f
	.word	0x44811000
	.word	0x44820000
	.word	0x46800020
	lui       $1, 0xc600
	.word	0x44810800
	lw        $3, 0x15c($4)
	nop
	nop
	.word	0x46020003
	.word	0x46010002
	.word	0x46000064
	.word	0x44020800
	.word	0x10000003
	sh        $2, 0xf0($3)
	lw        $2, 0x15c($4)
	sh        $0, 0xf0($2)
	.word	0xDFBF0150
	.word	0xDFB20140
	.word	0xDFB10130
	.word	0xDFB00120
	.word	0x03E00008
	addiu     $29, $29, 0x160
	.set	macro
	.set	reorder
	.set	at
	.end	HoldRope
