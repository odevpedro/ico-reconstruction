	.text
	.p2align 3
	.globl	boyAI_sub_142F1C
	.ent	boyAI_sub_142F1C
boyAI_sub_142F1C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sll	$4,$7,6
	addu	$4,$4,$3
	lui	$2,0xfdff
	lw	$3,4($4)
	ori	$2,$2,0xffff
	move	$22,$7
	and	$3,$3,$2
	sw	$3,4($4)
	bgez	$22,loc_00142f50
	lui	$3,0x6b
	lui	$4,0x55
	.word	0x1000005e
	addiu	$4,$4,0x7e50
loc_00142f50:
	sll	$19,$22,6
	addiu	$2,$3,-0x6750
	beqz	$17,loc_00142f64
	addu	$16,$19,$2
	sw	$16,0($17)
loc_00142f64:
	sb	$20,6($16)
	lui	$4,0xfeff
	mtc1	$0,$f0
	ori	$4,$4,0xffff
	lbu	$3,0x38($18)
	lw	$2,4($16)
	.word	0x4600a034
	dsrl	$3,$3,7
	sw	$18,0x38($16)
	and	$2,$2,$4
	sll	$3,$3,0x18
	or	$2,$2,$3
	bc1f	loc_00142fa8
	sw	$2,4($16)
	lwc1	$f0,0x24($18)
	b	loc_00142fac
	swc1	$f0,0x18($16)
loc_00142fa8:
	swc1	$f20,0x18($16)
loc_00142fac:
	addiu	$7,$0,-1
	lui	$4,0xdfff
	sh	$7,4($16)
	ori	$4,$4,0xffff
	lui	$3,0xfbff
	lui	$1,0x447a
	mtc1	$1,$f3
	lw	$2,4($16)
	ori	$3,$3,0xffff
	lui	$1,0x43fa
	mtc1	$1,$f1
	lui	$6,0x1000
	.set	macro
	.set	reorder
	.end	boyAI_sub_142F1C
