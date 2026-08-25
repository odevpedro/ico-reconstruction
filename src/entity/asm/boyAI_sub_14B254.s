	.text
	.p2align 3
	.globl	boyAI_sub_14B254
	.ent	boyAI_sub_14B254
boyAI_sub_14B254:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jr	$31
	andi	$2,$2,1
	nop	
	lw	$2,0x164($4)
	sd	$0,0x60($2)
	jr	$31
	sd	$0,0x58($2)
	lw	$7,0x164($4)
	addiu	$2,$0,1
	sllv	$2,$2,$5
	addiu	$8,$5,-2
	ld	$3,0x58($7)
	sltiu	$4,$8,0xf
	or	$3,$2,$3
	beqz	$4,loc_0014b2e4
	sd	$3,0x58($7)
	lui	$2,0x56
	sll	$3,$8,2
	addiu	$2,$2,-0x7eb0
	addu	$3,$3,$2
	lw	$4,0($3)
	jr	$4
	nop	
	jr	$31
	swc1	$f12,0x68($7)
	jr	$31
	swc1	$f12,0x6c($7)
	jr	$31
	swc1	$f12,0x70($7)
	jr	$31
	sw	$6,0x74($7)
	jr	$31
	sw	$6,0x78($7)
	jr	$31
	sw	$6,0x7c($7)
	sw	$6,0x88($7)
loc_0014b2e4:
	jr	$31
	nop	
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
	.word	0x14400003
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_14B254
