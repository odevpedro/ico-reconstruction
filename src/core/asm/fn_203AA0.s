	.text
	.p2align 3
	.globl	fn_203AA0
	.ent	fn_203AA0
fn_203AA0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lui       $3, 0x27
	addiu     $7, $0, 0xa
	lw        $2, 0x4ec0($3)
	addiu     $29, $29, -0x20
	.word	0xFFBF0010
	addiu     $3, $3, 0x4ec0
	.word	0x00471018
	.word	0xFFB00000
	addiu     $6, $0, 0x3c
	lw        $5, 0x4($3)
	addiu     $3, $0, 0x3c
	subu      $2, $6, $2
	.word	0x50A00001
	break     0x0, 0x7
	.word	0x0045001A
	mflo      $2
	.word	0x00821018
	.word	0x0046001A
	mflo      $2
	.word	0x10800003
	.word	0x0040182D
	addiu     $2, $0, 0x1
	.word	0x0043180A
	.word	0x14600006
	nop
	nop
	.word	0x0C04F4FC
	nop
	.word	0x1000FFFD
	nop
	.word	0x18600005
	.word	0x0060802D
	.word	0x0C04F4FC
	addiu     $16, $16, -0x1
	.word	0x1600FFFD
	nop
	.word	0xDFBF0010
	.word	0xDFB00000
	.word	0x03E00008
	addiu     $29, $29, 0x20
	.set	macro
	.set	reorder
	.set	at
	.end	fn_203AA0
