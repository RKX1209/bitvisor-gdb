
#ifndef _CORE_GMM_ACCESS_H
#define _CORE_GMM_ACCESS_H

#include <core/types.h>

void read_gphys_b (u64 phys, void *data, u32 attr);
void write_gphys_b (u64 phys, u32 data, u32 attr);
void read_gphys_w (u64 phys, void *data, u32 attr);
void write_gphys_w (u64 phys, u32 data, u32 attr);
void read_gphys_l (u64 phys, void *data, u32 attr);
void write_gphys_l (u64 phys, u32 data, u32 attr);
void read_gphys_q (u64 phys, void *data, u32 attr);
void write_gphys_q (u64 phys, u64 data, u32 attr);

void read_gvirt_b (u64 virt, void *data, u32 attr);
void write_gvirt_b (u64 virt, void *data, u32 attr);
void read_gvirt_w (u64 virt, void *data, u32 attr);
void write_gvirt_w (u64 virt, void *data, u32 attr);
void read_gvirt_l (u64 virt, void *data, u32 attr);
void write_gvirt_l (u64 virt, void *data, u32 attr);
void read_gvirt_q (u64 virt, void *data, u32 attr);
void write_gvirt_q (u64 virt, void *data, u32 attr);

bool cmpxchg_gphys_l (u64 phys, u32 *olddata, u32 data, u32 attr);
bool cmpxchg_gphys_q (u64 phys, u64 *olddata, u64 data, u32 attr);

static inline void
read_phys_l (u64 phys, void *data)
{
	read_gphys_l (phys, data, 0);
}

static inline void
read_phys_b (u64 phys, void *data)
{
	read_gphys_b (phys, data, 0);
}

static inline void
write_phys_b (u64 phys, u32 data)
{
	write_gphys_b (phys, data, 0);
}

static inline void
write_phys_l (u64 phys, u32 data)
{
	write_gphys_l (phys, data, 0);
}

static inline void
read_phys_q (u64 phys, void *data)
{
	read_gphys_q (phys, data, 0);
}

static inline void
write_phys_q (u64 phys, u64 data)
{
	write_gphys_q (phys, data, 0);
}

#endif
