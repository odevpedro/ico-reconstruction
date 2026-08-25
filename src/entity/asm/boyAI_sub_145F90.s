	.text
	.p2align 3
	.globl	boyAI_sub_145F90
	.ent	boyAI_sub_145F90
boyAI_sub_145F90:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	and	$2,$2,$3
	sd	$2,0x20($17)
	ld	$31,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	lwc1	$f20,0x30($29)
	jr	$31
	addiu	$29,$29,0x40
	addiu	$29,$29,-0x70
	sd	$20,0x50($29)
	sd	$18,0x30($29)
	move	$20,$0
	sd	$17,0x20($29)
	move	$18,$0
	sd	$16,0x10($29)
	move	$17,$4
	sd	$31,0x60($29)
	addiu	$4,$0,4
	sd	$19,0x40($29)
	jal	0x0013eb50
	lw	$19,0x164($17)
	b	loc_00145ffc
	move	$16,$2
	nop	
loc_00145ff0:
	jal	0x0013ebe0
	move	$4,$16
	move	$16,$2
loc_00145ffc:
	beqz	$16,loc_00146018
	nop	
	jal	0x001653a0
	move	$4,$16
	bne	$17,$2,loc_00145ff0
	nop	
	addiu	$20,$0,1
loc_00146018:
	beqz	$20,loc_0014603c
	lui	$5,0x800
	lw	$4,0x164($17)
	ld	$2,0x58($4)
	lw	$3,0x90($4)
	or	$2,$2,$5
	ori	$3,$3,1
	sd	$2,0x58($4)
	sw	$3,0x90($4)
loc_0014603c:
	lw	$4,0x164($17)
	beqz	$4,loc_00146058
	lui	$3,0x800
	ld	$2,0x58($4)
	and	$2,$2,$3
	bnez	$2,loc_0014605c
	addiu	$2,$0,1
loc_00146058:
	move	$2,$0
loc_0014605c:
	andi	$2,$2,0xff
	.word	0x50400017
	lw	$2,0x670($4)
	.set	macro
	.set	reorder
	.end	boyAI_sub_145F90
