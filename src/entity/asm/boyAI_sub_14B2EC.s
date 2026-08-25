	.text
	.p2align 3
	.globl	boyAI_sub_14B2EC
	.ent	boyAI_sub_14B2EC
boyAI_sub_14B2EC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	lw	$3,0x164($4)
	beqz	$3,loc_0014b310
	move	$2,$0
	addiu	$2,$0,1
	ld	$3,0x58($3)
	sllv	$4,$2,$5
	and	$3,$4,$3
	movz	$2,$0,$3
loc_0014b310:
	jr	$31
	nop	
	jr	$31
	nop	
	lw	$2,0x164($4)
	lw	$3,0x678($2)
	jr	$31
	swc1	$f12,0x334($3)
	lw	$3,0x164($4)
	lw	$3,0x670($3)
	lw	$2,0x54($3)
	sltu	$2,$5,$2
	bnez	$2,loc_0014b350
	nop	
	sw	$5,0x54($3)
	swc1	$f12,0x58($3)
loc_0014b350:
	jr	$31
	nop	
	lui	$2,0x29
	sll	$4,$4,2
	addiu	$2,$2,0x24b0
	addu	$4,$4,$2
	jr	$31
	lwc1	$f0,0($4)
	lw	$3,0x15c($4)
	addiu	$6,$0,0x190
	.set	macro
	.set	reorder
	.end	boyAI_sub_14B2EC
