	.text
	.p2align 3
	.globl	fn_14A100
	.ent	fn_14A100
fn_14A100:
	.frame	$sp,48,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu     $29, $29, -0x30
	.word	0xFFB10010
	.word	0xFFB00000
	.word	0x0080882D
	.word	0x00A0802D
	.word	0xFFBF0020
	.word	0x0200202D
	.word	0x0C0427C4
	.word	0x00C0282D
	lw        $4, 0x15c($16)
	sll       $2, $2, 0x6
	.word	0xDFBF0020
	lw        $3, 0xc($4)
	addu      $3, $2, $3
	.word	0xC4600030
	.word	0xE6200000
	lw        $4, 0x15c($16)
	lw        $3, 0xc($4)
	addu      $3, $2, $3
	.word	0xC4600034
	.word	0xE6200004
	lw        $3, 0x15c($16)
	.word	0xDFB00000
	lw        $4, 0xc($3)
	addu      $2, $2, $4
	.word	0xC4400038
	.word	0xE6200008
	.word	0xDFB10010
	.word	0x03E00008
	.set	macro
	.set	reorder
	.set	at
	.end	fn_14A100
