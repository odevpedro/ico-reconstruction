	.text
	.p2align 3
	.globl	boyAI_sub_159A80
	.ent	boyAI_sub_159A80
boyAI_sub_159A80:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	div	$0,$2,$3
	mflo	$2
	.word	0x7050001a
	.word	0x70001012
	slt	$2,$10,$2
	beqz	$2,loc_00159ad0
	addiu	$5,$0,0x127
	beqz	$23,loc_00159ab0
	nop	
	ld	$2,0x18($17)
	or	$2,$2,$19
	sd	$2,0x18($17)
loc_00159ab0:
	beqz	$30,loc_00159ad0
	nop	
	lw	$6,0($29)
	lw	$2,0($29)
	lw	$4,0x15c($6)
	lw	$3,0x15c($2)
	swc1	$f20,0x138($3)
	swc1	$f20,0x130($4)
loc_00159ad0:
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000ffe0
	lw	$10,0x48($17)
	nop	
	addiu	$29,$29,-0x80
	.set	macro
	.set	reorder
	.end	boyAI_sub_159A80
