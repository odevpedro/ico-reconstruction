	.text
	.p2align 3
	.globl	fn_15BCC8
	.ent	fn_15BCC8
fn_15BCC8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu     $2, $0, 0xa8
	.word	0x10A20004
	lw        $6, 0x164($4)
	addiu     $2, $0, 0xad
	.word	0x14A20019
	nop
	lw        $3, 0xc($4)
	addiu     $2, $0, 0x1
	.word	0x14620015
	nop
	.word	0xDCC30470
	.word	0x0003177E
	andi      $2, $2, 0x1
	.word	0x10400008
	nop
	.word	0xDCC20480
	.word	0x0002177E
	andi      $2, $2, 0x1
	.word	0x10400003
	nop
	.word	0x10000009
	addiu     $5, $0, 0xa9
	.word	0x000316FE
	andi      $2, $2, 0x1
	.word	0x10400005
	addiu     $3, $0, 0xaa
	.word	0xDCC20480
	.word	0x000216FE
	andi      $2, $2, 0x1
	.word	0x0062280B
	.word	0x0804FFE2
	.set	macro
	.set	reorder
	.set	at
	.end	fn_15BCC8
