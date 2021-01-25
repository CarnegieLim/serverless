#include <uapi/linux/ptrace.h>

//BPF_HASH(clone_start, u32);
BPF_HASH(dup_mm_start, u32);

BPF_HISTOGRAM(dup_mm_dist);
//BPF_HISTOGRAM(clone_dist);
/*
int do_clone_entry(struct pt_regs *ctx)
{
	char comm[32];
    	bpf_get_current_comm(&comm, sizeof(comm));
	if(comm[0] != 'm' || comm[1] != 'y' || comm[2] != '_' || comm[3] != 'c' || comm[4] != 'l' || comm[5] != 'o' || comm[6] != 'n' || comm[7] != 'e')
		return 0;

	u32 pid;
	u64 ts, *val;

	pid = bpf_get_current_pid_tgid();
	ts = bpf_ktime_get_ns();
	clone_start.update(&pid, &ts);
	return 0;
}


int do_clone_return(struct pt_regs *ctx)
{
	u32 pid;
	u64 *tsp, delta;

	pid = bpf_get_current_pid_tgid();
	tsp = clone_start.lookup(&pid);

	if (tsp != 0) {
		delta = bpf_ktime_get_ns() - *tsp;
		clone_dist.increment(delta / 1000);
		clone_start.delete(&pid);
	}

	return 0;
}
*/

int do_dup_mm_entry(struct pt_regs *ctx)
{
        char comm[32];
        bpf_get_current_comm(&comm, sizeof(comm));
        if(comm[0] != 'm' || comm[1] != 'y' || comm[2] != '_' || comm[3] != 'c' || comm[4] != 'l' || comm[5] != 'o' || comm[6] != 'n' || comm[7] != 'e')
                return 0;

        u32 pid;
        u64 ts, *val;

        pid = bpf_get_current_pid_tgid();
	ts = bpf_ktime_get_ns();
        dup_mm_start.update(&pid, &ts);
        return 0;
}

int do_dup_mm_return(struct pt_regs *ctx)
{
        u32 pid;
        u64 *tsp, delta;

        pid = bpf_get_current_pid_tgid();
	tsp = dup_mm_start.lookup(&pid);

        if (tsp != 0) {
                delta = bpf_ktime_get_ns() - *tsp;
                dup_mm_dist.increment(delta / 1000);
                dup_mm_start.delete(&pid);
        }

        return 0;
}

