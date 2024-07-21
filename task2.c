#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kallsyms.h>


MODULE_LICENSE("GPL");

typedef int (* syscall_wrapper)(struct pt_regs *);

//defining execve syscall
#define WATCHED_CALL __NR_execve

//because less than this can get syscall tbale directly from kallsymbols but now cannot do that as kernal does not allow
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,7,0)
#define KPROBE_LOOKUP 1
#include <linux/kprobes.h>
typedef unsigned long (* kallsyms_lookup_name_t)(const char* name);

static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};
#endif

unsigned long sys_call_table_addr;


//enabling writing of syscall table
int enable_page_rw(void *ptr){
    unsigned int level;
    pte_t *pte = lookup_address((unsigned long) ptr, &level);

    if(pte->pte & ~_PAGE_RW){
        pte->pte |= _PAGE_RW;
    }

    return 0;
}

//to make it again read only
int disable_page_rw(void *ptr){
    unsigned int level;
    pte_t *pte = lookup_address((unsigned long) ptr, &level);
    pte->pte = pte->pte & ~_PAGE_RW;
    return 0;
}

syscall_wrapper original_syscall;


//this is like the hooked execve , as we changes the address in syscall table
int log_syscall(struct pt_regs *regs) {
    pr_info("[monitor harsha] getexecve was called\n");

     char __user *filename = (char __user *)regs->di;
    char fname[256];
    char *new_fname;
    long ret;

    // Initialize fname to ensure it contains a valid string
    memset(fname, 0, sizeof(fname));

    // Copy the filename from user space to kernel space
    if (strncpy_from_user(fname, filename, sizeof(fname)) > 0) {
        // Check if the command starts with "/hidden"
        if (strncmp(fname, "/hidden", 7) == 0) {
            pr_info("Executing hidden command: %s\n", fname + 7);

            // Allocate memory for the new filename
            new_fname = kmalloc(strlen(fname + 7) + 1, GFP_KERNEL);
            if (!new_fname)
                return -ENOMEM;

            // Remove the "/hidden" prefix
            strcpy(new_fname, fname + 7);

            // Modify the filename in regs->di to point to the new filename
            regs->di = (unsigned long)new_fname;

    
            //here is the problem as regs->di even though i have changed , error still says as bad address
            //so i am able to hook the execve call and call it later again but when i want to strip from kernal part it seems some issue in user and kernal space , unable to figure till now


            return (*original_syscall)(regs);
        }
    }

    return (*original_syscall)(regs);
}


//starting of loader which runs first
static int __init logger_init(void) {
    pr_info("[monitor] module has been loaded\n");

    // Only run the kprobe search if defined
#ifdef KPROBE_LOOKUP
    pr_info("[monitor] setting up, looking for address of kallsyms_lookup_name...\n");

    // Register kprobe using kprobe structure defined above.using kprobe method to find syscall address
    int ret = register_kprobe(&kp);
    if (ret < 0) {
        pr_err("[monitor] register_kprobe failed, returned %d\n", ret);
        return ret;
    }

    pr_info("[monitor] kprobe registered. kallsyms_lookup_name found at 0x%px\n",
kp.addr);

    // Manually define kallsyms_lookup_name() function to point to the recovered address
    kallsyms_lookup_name_t kallsyms_lookup_name = (kallsyms_lookup_name_t) kp.addr;

    // Get rid of the kprobe, we don't need it anymore
    unregister_kprobe(&kp);

    pr_info("[monitor] kprobe unregistered. now to the meat and potatoes...\n");
#endif

    // We have kallsyms_lookup_name(). Get the sys_call_table address
    sys_call_table_addr = kallsyms_lookup_name("sys_call_table");

    
    pr_info("[monitor] sys_call_table@%lx\n", sys_call_table_addr);
    //now we have syscall_table address
    // Enable read/write of the syscall table
    enable_page_rw((void *)sys_call_table_addr);

    // Original syscall address (will change later to a different instruction)
    original_syscall = ((syscall_wrapper *)sys_call_table_addr)[WATCHED_CALL];
    pr_info("watching call address of execve",*original_syscall);


    if (!original_syscall) {
        pr_err("[monitor] Failed to find original syscall address\n");
        return -1;
    }

    // log_syscall is a modded version of the original function.
    //     It pr_info()s and returns
    ((syscall_wrapper *)sys_call_table_addr)[WATCHED_CALL] = log_syscall;

    // Disable read/write of the syscall table, we don't need it anymore
    disable_page_rw((void *)sys_call_table_addr);
    
    pr_info("[monitor] original_syscall = %p\n", original_syscall);
    return 0;
}

static void __exit logger_exit(void) {
    pr_info("[monitor] time to restore syscall...\n");
    
    // Enable read/write of the syscall table, so we can restore the instruction
    enable_page_rw((void *)sys_call_table_addr);

    // Restore syscall to original code . as we had changed the syscall before .
    ((syscall_wrapper *)sys_call_table_addr)[WATCHED_CALL] = original_syscall;

    // Disable read/write of the syscall table, we're done here
    disable_page_rw((void *)sys_call_table_addr);

    pr_info("[monitor] syscall restored. module has been unloaded\n");
}

module_init(logger_init);
module_exit(logger_exit);
