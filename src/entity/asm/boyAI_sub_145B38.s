	.text
	.p2align 3
	.globl	boyAI_sub_145B38
	.ent	boyAI_sub_145B38
boyAI_sub_145B38:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$3,0x4a0($2)
	addiu	$4,$4,0x5060
	mult	$3,$3,$5
	addu	$3,$3,$4
	lw	$2,0x188($3)
	srl	$2,$2,0xd
	andi	$2,$2,1
	beqz	$2,loc_00145b88
	move	$6,$0
	lw	$3,0x164($16)
	lui	$1,0x4080
	mtc1	$1,$f0
	lw	$2,0x678($3)
	lwc1	$f1,0x400($2)
	.word	0x46000834
	nop	
	bc1tl	loc_00145b8c
	addiu	$6,$0,1
	b	loc_00145b8c
	nop	
loc_00145b88:
	lw	$3,0x164($16)
loc_00145b8c:
	beql	$6,$0,loc_00145ba8
	lw	$2,0x678($3)
	lw	$3,0x678($3)
	lw	$2,0x410($3)
	addiu	$2,$2,1
	b	loc_00145bac
	sw	$2,0x410($3)
loc_00145ba8:
	sw	$0,0x410($2)
loc_00145bac:
	lui	$2,0x27
	addiu	$6,$0,0xa
	lw	$4,0x4ec0($2)
	addiu	$3,$0,0x3c
	addiu	$2,$2,0x4ec0
	.word	0x00862018
	lw	$5,4($2)
	lw	$2,0x164($16)
	beql	$5,$0,loc_00145bd4
	break	0,7
loc_00145bd4:
	lw	$7,0x678($2)
	subu	$3,$3,$4
	div	$0,$3,$5
	lw	$6,0x410($7)
	mflo	$3
	sll	$2,$3,2
	addu	$2,$2,$3
	slt	$2,$2,$6
	beqz	$2,loc_00145c0c
	ld	$2,0x410($7)
	ori	$3,$0,0x8000
	dsll	$3,$3,0x11
	.word	0x10000007
	or	$2,$2,$3
loc_00145c0c:
	addiu	$3,$0,-2
	dsll	$3,$3,0x10
	ori	$3,$3,0xffff
	dsll	$3,$3,0x10
	ori	$3,$3,0xffff
	and	$2,$2,$3
	.set	macro
	.set	reorder
	.end	boyAI_sub_145B38
