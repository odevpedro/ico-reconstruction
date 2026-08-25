	.text
	.p2align 3
	.globl	boyAI_sub_1623B8
	.ent	boyAI_sub_1623B8
boyAI_sub_1623B8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,0x1fc($4)
	mtlo	$3
	madd	$2,$2,$5
	lw	$3,0($2)
	beqz	$3,loc_001624a0
	nop	
	dsrl32	$2,$6,0x1c
	andi	$2,$2,1
	beqz	$2,loc_001624a0
	addiu	$22,$0,0x6b
	lw	$16,0($17)
	addiu	$21,$0,0x50
	jal	0x0013f808
	move	$4,$16
	ld	$3,0x20($17)
	lui	$2,0x56
	sw	$0,0x33c($17)
	addiu	$18,$2,-0x3028
	or	$3,$3,$30
	sw	$0,0x110($17)
	sd	$3,0x20($17)
	sw	$0,0x114($17)
	sw	$0,0x118($17)
	sw	$23,0x32c($17)
	sw	$23,0x328($17)
	lw	$2,-0x6e08($28)
loc_00162420:
	beqz	$2,loc_00162448
	addiu	$5,$0,0xf2
	lw	$3,0x164($2)
	lw	$2,0x30($3)
	bne	$2,$22,loc_00162448
	nop	
	lw	$3,0x134($3)
	lw	$2,0($29)
	beql	$3,$2,loc_00162480
	ld	$2,0x18($17)
loc_00162448:
	lw	$2,0($29)
	lw	$4,0x164($2)
	lw	$3,0x30($4)
	mult	$2,$3,$21
	addu	$3,$2,$18
	lw	$2,0x4c($3)
	srl	$2,$2,8
	andi	$2,$2,1
	beql	$2,$0,loc_00162480
	ld	$2,0x18($17)
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
	ld	$2,0x18($17)
loc_00162480:
	dsrl32	$2,$2,0x1b
	andi	$2,$2,1
	.word	0x1440000c
	addiu	$4,$0,1
	jal	0x00203aa0
	nop	
	b	loc_00162420
	lw	$2,-0x6e08($28)
loc_001624a0:
	lw	$4,0($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_1623B8
