	.text
	.p2align 3
	.globl	boyAI_sub_145ECC
	.ent	boyAI_sub_145ECC
boyAI_sub_145ECC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	mov.s	$f12,$f20
	mov.s	$f12,$f20
	move	$4,$16
	jal	0x001e09f8
	addiu	$5,$0,0x23
	lw	$4,0x164($16)
	lw	$6,-0x6e08($28)
	lw	$3,0x678($4)
	lw	$2,0x3a0($3)
	beqz	$2,loc_00145efc
	addiu	$2,$2,-1
	sw	$2,0x3a0($3)
loc_00145efc:
	bnel	$16,$6,loc_00145f10
	lw	$3,8($16)
	jal	0x001752d0
	move	$4,$16
	lw	$3,8($16)
loc_00145f10:
	addiu	$2,$0,0xd7d
	.word	0x14620021
	ld	$31,0x20($29)
	lui	$3,0x27
	addiu	$6,$0,0xa
	lw	$5,0x4ec0($3)
	addiu	$2,$0,0x3c
	addiu	$3,$3,0x4ec0
	.word	0x00a62818
	lw	$4,4($3)
	beql	$4,$0,loc_00145f40
	break	0,7
loc_00145f40:
	lw	$3,0x10($17)
	subu	$2,$2,$5
	div	$0,$2,$4
	mflo	$2
	sll	$2,$2,1
	slt	$2,$2,$3
	.word	0x10400010
	lw	$2,-0x6e08($28)
	beqz	$2,loc_00145f84
	addiu	$3,$0,0x6b
	lw	$4,0x164($2)
	lw	$2,0x30($4)
	bnel	$2,$3,loc_00145f88
	ld	$2,0x20($17)
	lw	$2,0x134($4)
	.word	0x50500008
	ld	$17,0x10($29)
loc_00145f84:
	ld	$2,0x20($17)
loc_00145f88:
	lui	$3,0xfffd
	ori	$3,$3,0xffff
	and	$2,$2,$3
	.set	macro
	.set	reorder
	.end	boyAI_sub_145ECC
