/*
 * Copyright (c) 2007, 2008 University of Tsukuba
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Tsukuba nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* accessing memory by guest-physical address */

#include "cache.h"
#include "constants.h"
#include "current.h"
#include "cpu_mmu.h"
#include <core/gmm_access.h>
#include "mm.h"
#include "mmio.h"
#include "panic.h"
#include "pcpu.h"
#include "printf.h"
#include "vt_regs.h"

void
read_gphys_b (u64 phys, void *data, u32 attr)
{
	attr = cache_get_attr (phys, attr);
	mmio_lock ();
	if (!mmio_access_memory (phys, false, data, 1, attr)) {
		phys = current->gmm.gp2hp (phys, NULL);
		read_hphys_b (phys, data, attr);
	}
	mmio_unlock ();
}

void
write_gphys_b (u64 phys, u32 data, u32 attr)
{
	bool fakerom;

	attr = cache_get_attr (phys, attr);
	mmio_lock ();
	if (!mmio_access_memory (phys, true, &data, 1, attr)) {
		phys = current->gmm.gp2hp (phys, &fakerom);
		if (fakerom)
			panic ("write_gphys_b modifying VMM memory.");
		write_hphys_b (phys, data, attr);
	}
	mmio_unlock ();
}

void
read_gphys_w (u64 phys, void *data, u32 attr)
{
	if ((phys & 0xFFF) == 0xFFF) {
		read_gphys_b (phys + 0, ((u8 *)data) + 0, attr);
		read_gphys_b (phys + 1, ((u8 *)data) + 1, attr);
		return;
	}
	attr = cache_get_attr (phys, attr);
	mmio_lock ();
	if (!mmio_access_memory (phys, false, data, 2, attr)) {
		phys = current->gmm.gp2hp (phys, NULL);
		read_hphys_w (phys, data, attr);
	}
	mmio_unlock ();
}

void
write_gphys_w (u64 phys, u32 data, u32 attr)
{
	bool fakerom;

	if ((phys & 0xFFF) == 0xFFF) {
		write_gphys_b (phys + 0, *(((u8 *)&data) + 0), attr);
		write_gphys_b (phys + 1, *(((u8 *)&data) + 1), attr);
		return;
	}
	attr = cache_get_attr (phys, attr);
	mmio_lock ();
	if (!mmio_access_memory (phys, true, &data, 2, attr)) {
		phys = current->gmm.gp2hp (phys, &fakerom);
		if (fakerom)
			panic ("write_gphys_w modifying VMM memory.");
		write_hphys_w (phys, data, attr);
	}
	mmio_unlock ();
}

void
read_gphys_l (u64 phys, void *data, u32 attr)
{
	if ((phys & 0xFFF) >= 0xFFD) {
		read_gphys_w (phys + 0, ((u16 *)data) + 0, attr);
		read_gphys_w (phys + 2, ((u16 *)data) + 1, attr);
		return;
	}
	attr = cache_get_attr (phys, attr);
	mmio_lock ();
	if (!mmio_access_memory (phys, false, data, 4, attr)) {
		phys = current->gmm.gp2hp (phys, NULL);
		read_hphys_l (phys, data, attr);
	}
	mmio_unlock ();
}

void
write_gphys_l (u64 phys, u32 data, u32 attr)
{
	bool fakerom;

	if ((phys & 0xFFF) >= 0xFFD) {
		write_gphys_w (phys + 0, *(((u16 *)&data) + 0), attr);
		write_gphys_w (phys + 2, *(((u16 *)&data) + 1), attr);
		return;
	}
	attr = cache_get_attr (phys, attr);
	mmio_lock ();
	if (!mmio_access_memory (phys, true, &data, 4, attr)) {
		phys = current->gmm.gp2hp (phys, &fakerom);
		if (fakerom)
			printf ("write_gphys_l modifying VMM memory.");
		write_hphys_l (phys, data, attr);
	}
	mmio_unlock ();
}

void
read_gphys_q (u64 phys, void *data, u32 attr)
{
	if ((phys & 0xFFF) >= 0xFF9) {
		read_gphys_l (phys + 0, ((u32 *)data) + 0, attr);
		read_gphys_l (phys + 4, ((u32 *)data) + 1, attr);
		return;
	}
	attr = cache_get_attr (phys, attr);
	mmio_lock ();
	if (!mmio_access_memory (phys, false, data, 8, attr)) {
		phys = current->gmm.gp2hp (phys, NULL);
		read_hphys_q (phys, data, attr);
	}
	mmio_unlock ();
}

void
write_gphys_q (u64 phys, u64 data, u32 attr)
{
	bool fakerom;

	if ((phys & 0xFFF) >= 0xFF9) {
		write_gphys_l (phys + 0, *(((u32 *)&data) + 0), attr);
		write_gphys_l (phys + 4, *(((u32 *)&data) + 1), attr);
		return;
	}
	attr = cache_get_attr (phys, attr);
	mmio_lock ();
	if (!mmio_access_memory (phys, true, &data, 8, attr)) {
		phys = current->gmm.gp2hp (phys, &fakerom);
		if (fakerom)
			panic ("write_gphys_q modifying VMM memory.");
		write_hphys_q (phys, data, attr);
	}
	mmio_unlock ();
}

/* RKX: Read/Write method for guest virtual address */
phys_t
gvirt_to_phys (virt_t virt)
{
	ulong cr0, cr3, cr4;
	u64 efer;
	u64 ent[5];
	phys_t physaddr;
	int levels;
	enum vmmerr res;

	current->vmctl.read_control_reg (CONTROL_REG_CR0, &cr0);
	current->vmctl.read_control_reg (CONTROL_REG_CR3, &cr3);
	current->vmctl.read_control_reg (CONTROL_REG_CR4, &cr4);
	current->vmctl.read_msr (MSR_IA32_EFER, &efer);

	if ((res = cpu_mmu_get_pte (virt, cr0, cr3, cr4, efer,
											false, false, false, ent, &levels)) == VMMERR_SUCCESS) {
	  physaddr = (ent[0] & PTE_ADDR_MASK64) | ((virt) & 0xFFF);
	} else {
		printf("gvirt_to_phys failed: reason %d\n", res);
		return -1;
	}
	//printf("virt: 0x%016lx => phys: 0x%016lx\n", virt, physaddr);
	return physaddr;
}

int
read_gvirt_b (u64 virt, void *data, u32 attr)
{
	u64 phys = gvirt_to_phys (virt);
	if (phys == -1)
		return -1;
	read_gphys_b (phys, data, attr);
	return 0;
}

int
write_gvirt_b (u64 virt, u32 data, u32 attr)
{
	u64 phys = gvirt_to_phys (virt);
	if (phys == -1)
		return -1;
	write_gphys_b (phys, data, attr);
	return 0;
}
int
read_gvirt_w (u64 virt, void *data, u32 attr)
{
	u64 phys = gvirt_to_phys (virt);
	if (phys == -1)
		return -1;
	read_gphys_w (phys, data, attr);
	return 0;
}

int
write_gvirt_w (u64 virt, u32 data, u32 attr)
{
	u64 phys = gvirt_to_phys (virt);
	if (phys == -1)
		return -1;
	write_gphys_w (phys, data, attr);
	return 0;
}
int
read_gvirt_l (u64 virt, void *data, u32 attr)
{
	u64 phys = gvirt_to_phys (virt);
	if (phys == -1)
		return -1;
	read_gphys_l (phys, data, attr);
	return 0;
}

int
write_gvirt_l (u64 virt, u32 data, u32 attr)
{
	u64 phys = gvirt_to_phys (virt);
	if (phys == -1)
		return -1;
	write_gphys_l (phys, data, attr);
	return 0;
}

int
read_gvirt_q (u64 virt, void *data, u32 attr)
{
	u64 phys = gvirt_to_phys (virt);
	if (phys == -1)
		return -1;
	read_gphys_q (phys, data, attr);
	return 0;
}

int
write_gvirt_q (u64 virt, u64 data, u32 attr)
{
	u64 phys = gvirt_to_phys (virt);
	if (phys == -1)
		return -1;
	write_gphys_q (phys, data, attr);
	return 0;
}

bool
cmpxchg_gphys_l (u64 phys, u32 *olddata, u32 data, u32 attr)
{
	bool fakerom;

	if ((phys & 0xFFF) >= 0xFFD)
		panic ("cmpxchg_gphys_l: bad physical address");
	attr = cache_get_attr (phys, attr);
	mmio_lock ();
	if (mmio_access_memory (phys, false, olddata, 4, attr))
		panic ("CMPXCHG MMIO!");
	mmio_unlock ();
	phys = current->gmm.gp2hp (phys, &fakerom);
	if (fakerom)
		panic ("cmpxchg_gphys_l modifying VMM memory.");
	return cmpxchg_hphys_l (phys, olddata, data, attr);
}

bool
cmpxchg_gphys_q (u64 phys, u64 *olddata, u64 data, u32 attr)
{
	bool fakerom;

	if ((phys & 0xFFF) >= 0xFF9)
		panic ("cmpxchg_gphys_q: bad physical address");
	attr = cache_get_attr (phys, attr);
	mmio_lock ();
	if (mmio_access_memory (phys, false, olddata, 8, attr))
		panic ("CMPXCHG MMIO!");
	mmio_unlock ();
	phys = current->gmm.gp2hp (phys, &fakerom);
	if (fakerom)
		panic ("cmpxchg_gphys_q modifying VMM memory.");
	return cmpxchg_hphys_q (phys, olddata, data, attr);
}
