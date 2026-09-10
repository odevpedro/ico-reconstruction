	.text
	.p2align 3
	.globl	GirlForceFieldGeo
	.ent	GirlForceFieldGeo
GirlForceFieldGeo:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x4600BB03
	.word	0x0220202D
	.word	0x0C091112
	.word	0x0220282D
	lw        $4, 0xb8($17)
	.word	0x0240282D
	lw        $2, 0x34($29)
	.word	0x0220302D
	.word	0x0C0417DE
	addu      $4, $4, $2
	lw        $5, 0xb8($17)
	.word	0x0220202D
	.word	0x0240302D
	.word	0x0C0417E4
	addu      $5, $5, $22
	.word	0x0C04180A
	.word	0x0220202D
	.word	0x4600D834
	nop
	.word	0x4500000F
	lw        $5, 0x30($29)
	.word	0x0C045F08
	.word	0x46000306
	nop
	nop
	.word	0x4600CB03
	.word	0x0220202D
	.word	0x0C091112
	.word	0x0220282D
	lw        $4, 0xb8($17)
	.word	0x0240282D
	.word	0x0220302D
	.word	0x0C0417DE
	addu      $4, $4, $30
	lw        $5, 0x30($29)
	.word	0x0220202D
	.word	0x0C090EBA
	.word	0x0240302D
	.word	0x0C04180A
	.word	0x0220202D
	.word	0x4600E034
	nop
	.word	0x45020011
	.word	0xC66C0000
	.word	0x0C045F08
	.word	0x46000306
	.word	0xC66C0040
	.word	0x0220202D
	nop
	nop
	.word	0x46006303
	.word	0x0C091112
	.word	0x0220282D
	.word	0x02E0202D
	.word	0x0240282D
	.word	0x0C090EB4
	.word	0x0220302D
	.word	0x10000002
	.word	0xC66C0000
	.word	0xC66C0000
	.word	0x0240202D
	lui       $1, 0x3f80
	.word	0x44810800
	.word	0x46006024
	.word	0x44020000
	lw        $5, 0xb8($17)
	addu      $6, $5, $22
	.word	0x44820000
	.word	0x46800020
	addu      $5, $5, $21
	.word	0x46006301
	.word	0x0C090EAA
	.word	0x460C0B01
	.word	0xDFBF00D0
	.word	0xDFBE00C0
	.word	0xDFB700B0
	.word	0xDFB600A0
	.word	0xDFB50090
	.word	0xDFB40080
	.word	0xDFB30070
	.word	0xDFB20060
	.word	0xDFB10050
	.word	0xDFB00040
	.word	0xC7BC0120
	.word	0xC7BB0118
	.word	0xC7BA0110
	.word	0xC7B90108
	.word	0xC7B80100
	.word	0xC7B700F8
	.word	0xC7B600F0
	.word	0xC7B500E8
	.word	0xC7B400E0
	.word	0x03E00008
	addiu     $29, $29, 0x130
	.set	macro
	.set	reorder
	.set	at
	.end	GirlForceFieldGeo
