#include <asc.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/ptrace.h>
#include <asm/ptrace-abi.h>
#include <asm/unistd_64.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <uthash.h>

#define HAVE_HW_BREAKPOINT_H 1

#ifdef HAVE_HW_BREAKPOINT_H
#include <linux/hw_breakpoint.h>
#endif

static struct perf_event_attr pe;

long transition(struct machine *m, enum mode o)
{
    long i, c[3];
    int status;

    /* Reset error code.  */
    errno = 0;

    /* Fetch instruction count.  */
    if (read(m->fd, c + 0, sizeof(long)) < 0) {
        diagnostic("read");
        goto failure;
    }

    if (o == BASELINE) {
        /* Let the process fly until it makes a system call.  */
        if (ptrace(PTRACE_SYSEMU, m->pid, 0, 0) < 0) {
            diagnostic("ptrace sysemu");
            goto failure;
        }

        /* Re-attach to child process.  */
        if (waitpid(m->pid, &status, 0) == -1) {
            diagnostic("waitpid");
            goto failure;
        }
    } else if (o == SINGLESTEP) {
        /* Execute one instruction.  */
        if (ptrace(PTRACE_SYSEMU_SINGLESTEP, m->pid, 0, 0) < 0) {
            diagnostic("ptrace singlestep");
            goto failure;
        }

        /* Re-attach to child process.  */
        if (waitpid(m->pid, &status, 0) == -1) {
            diagnostic("waitpid");
            goto failure;
        }
    } else if (o == BLOCKSTEP) {
        /* Execute one basic block.  */
        if (ptrace(PTRACE_SINGLEBLOCK, m->pid, 0, 0) < 0) {
            diagnostic("ptrace singleblock");
            goto failure;
        }

        /* Re-attach to child process.  */
        if (waitpid(m->pid, &status, 0) == -1) {
            diagnostic("waitpid");
            goto failure;
        }
    } else if (o == BREAKPOINT) {
      
	int fd;

        pe.size = sizeof(struct perf_event_attr);
        pe.type = PERF_TYPE_BREAKPOINT;
        pe.bp_type = HW_BREAKPOINT_X;
        pe.bp_len = sizeof(long);

        pe.bp_addr = m->address;
        pe.sample_period = m->period;

        if ((fd = syscall(__NR_perf_event_open, &pe, m->pid, -1, -1, 0)) < 0) {
            diagnostic("perf_event_open");
            goto failure;
        }

        if (fcntl(fd, F_SETFL, O_ASYNC) < 0) {
            diagnostic("fcntl");
            goto failure;
        }

        if (fcntl(fd, F_SETSIG, SIGSTOP) < 0) {
            diagnostic("fcntl");
            goto failure;
        }

        if (fcntl(fd, F_SETOWN, m->pid) < 0) {
            diagnostic("fcntl");
            goto failure;
        }

        // Let the process fly until it trips the breakpoint K times.  
        if (ptrace(PTRACE_SYSEMU, m->pid, 0, 0) < 0) {
            diagnostic("ptrace sysemu");
            goto failure;
        }

        // Re-attach to child process.  
        if (waitpid(m->pid, &status, 0) == -1) {
            diagnostic("waitpid");
            goto failure;
        }

        if (close(fd) < 0) {
            diagnostic("close");
            goto failure;
        }
	} else if (o == INSTRUCTIONS) {
        /* Let the process fly until overflow interrupt fires.  */
        if (ptrace(PTRACE_SYSEMU, m->pid, 0, 0) < 0) {
            diagnostic("ptrace sysemu");
            goto failure;
        }

        /* Re-attach to child process.  */
        if (waitpid(m->pid, &status, 0) == -1) {
            diagnostic("waitpid");
            goto failure;
        }
    } else if (o == FUNCTION) {

      /* NEW MODE
       * FUNCTION mode traces program execution accross a function call by breaking at two points:
       *     breakpoint 1 : address of first instruction of the function
       *     breakpoint 2 : return address of function
       * FUNCTION mode supports recursive functions:
       *     machine *m has a stack 'rtn_stack' of struct rtn items; rtn_stack stores all return addresses of a function (f); the top of the stack is the most recent return address
       *     machine *m has a variable rtn_address; rtn_address stores the address at the top of m->rtn_stack
       * At every execution of the master integrator loop, a FUNCTION mode transition step occurs. 
       * At every transition step, two breakpoints are set. 
       * Breakpoint one is set as m->address (i.e: function address). Breakpoint 2 is set as the top of m->rtn_stack (i.e: most recent return address).
       * When execution halts at a breakpoint, the breakpoint is examined: 
       *     If breakpoint is the function address, then execution of the function just started; in this case, store the return address of the function as top of m->rtn_stack; continue execution.
       *     If breakpoint is the most recent return address, then the specified function just returned; in this case, pop the top of m->rtn_stack (i.e: function has returned at this address; no use of this address anymore); continue execution.
       */


      /* file descriptors to trace program execution
       * fd: traces breakpoint 1
       * fd2: traces breakpoint 2
       */
        int fd;
	int fd2;

        pe.size = sizeof(struct perf_event_attr);
        pe.type = PERF_TYPE_BREAKPOINT;
        pe.bp_type = HW_BREAKPOINT_X;
        pe.bp_len = sizeof(long);
	pe.sample_period = m->period;
	


	/* set breakpoint 1 */        //printf("breakpoint 1 = %lx\n", m->address);
	pe.bp_addr = m->address;

        if ((fd = syscall(__NR_perf_event_open, &pe, m->pid, -1, -1, 0)) < 0) {
            diagnostic("perf_event_open");
            goto failure;
        }

	
	
	/* set breakpoint 2 */        //printf("breakpoint 2 = %lx\n", m->rtn_address);
	pe.bp_addr = m->rtn_address;

       if ((fd2 = syscall(__NR_perf_event_open, &pe, m->pid, -1, -1, 0)) < 0) {
            diagnostic("perf_event_open");
            goto failure;
        }


       /* set status flags for fd and fd2 */
        if (fcntl(fd, F_SETFL, O_ASYNC) < 0) {
            diagnostic("fcntl");
            goto failure;
        }
	if (fcntl(fd2, F_SETFL, O_ASYNC) < 0) {
            diagnostic("fcntl");
            goto failure;
        }
	/* set the signal sent when breakpoint 1 or breakpoint 2 is hit */
        if (fcntl(fd, F_SETSIG, SIGSTOP) < 0) {
            diagnostic("fcntl");
            goto failure;
        }
        if (fcntl(fd2, F_SETSIG, SIGSTOP) < 0) {
            diagnostic("fcntl");
            goto failure;
        }
	/* set this process as process to receive signals for events on fd and fd2 */
        if (fcntl(fd, F_SETOWN, m->pid) < 0) {
            diagnostic("fcntl");
            goto failure;
        }
       if (fcntl(fd2, F_SETOWN, m->pid) < 0) {
            diagnostic("fcntl");
            goto failure;
        }


        /* Let the process fly until it trips the breakpoint K times.  */
        if (ptrace(PTRACE_SYSEMU, m->pid, 0, 0) < 0) {
            diagnostic("ptrace sysemu");
            goto failure;
        }
        /* Re-attach to child process.  */
        if (waitpid(m->pid, &status, 0) == -1) {
            diagnostic("waitpid");
            goto failure;
        }


        if (close(fd) < 0) {
            diagnostic("close");
            goto failure;
        }
	if (close(fd2) < 0) {
            diagnostic("close");
            goto failure;
        }


	/* get pc value at this point in execution */      
        int err;
        struct user_regs_struct gpr;
        if((err = ptrace(PTRACE_GETREGS, m->pid, 0, &gpr)) < 0)
          {
            diagnostic("cannot grab general purpose registers");
            goto failure;
          }
	/* rip : stores pc value */
        long rip = (long)gpr.rip;
	

	/* if we are at the start of the function, add return address to m->rtn_stack */
	if(rip == m->address)
	  {
    	    long rsp = ptrace(PTRACE_PEEKUSER, m->pid, 8*RSP, 0);
	    long rtn_addr = ptrace(PTRACE_PEEKDATA, m->pid, rsp, 0);
	    
    	    add(&(m->rtn_stack), rtn_addr);
	    	    
   	    if(m->rtn_address != rtn_addr)
		m->rtn_address = rtn_addr;
	  }

	/* if we are at a return address, pop that return address from m->rtn_stack and set m->rtn_address to new top of m->rtn_stack */
	else
	  {
            if(m->rtn_stack != NULL)
	      {
	        pop(&(m->rtn_stack));

		if(m->rtn_stack != NULL)
			if(m->rtn_stack->addr != m->rtn_address)
				m->rtn_address = m->rtn_stack->addr;
	       }
	  }

    } else if (o == (INSTRUCTIONS | BREAKPOINT)) {
        /* Let the process fly until overflow interrupt fires.  */
        if (ptrace(PTRACE_SYSEMU, m->pid, 0, 0) < 0) {
            diagnostic("ptrace sysemu");
            goto failure;
        }

        /* Re-attach to child process.  */
        if (waitpid(m->pid, &status, 0) == -1) {
            diagnostic("waitpid");
            goto failure;
        }

        /* Fetch virtual register containing system call number if any.  */
        i = ptrace(PTRACE_PEEKUSER, m->pid, 8 * ORIG_RAX, 0);

        /* Bail out on system call or error.  */
        if (i != -1 || errno)
            goto count;

        /* Calculate offset of debug register 0.  */
        i = offsetof(struct user, u_debugreg[0]);

        /* Store designated breakpoint address in debug register 0.  */
        if (ptrace(PTRACE_POKEUSER, m->pid, i, m->address) < 0) {
            diagnostic("ptrace pokeuser");
            goto failure;
        }

        /* Calcul = offsetof(struct user, u_debugreg[7]);*/
        i = offsetof(struct user, u_debugreg[7]);

        /* Enable designated hardware breakpoint.  */
        if (ptrace(PTRACE_POKEUSER, m->pid, i, 1) < 0) {
            diagnostic("ptrace pokeuser");
            goto failure;
        }

        /* Let the process fly until it hits the breakpoint.  */
        if (ptrace(PTRACE_SYSEMU, m->pid, 0, 0) < 0) {
            diagnostic("ptrace sysemu");
            goto failure;
        }

        /* Re-attach to child process.  */
        if (waitpid(m->pid, &status, 0) == -1) {
            diagnostic("waitpid");
            goto failure;
        }

        /* Disable designated hardware breakpoint.  */
        if (ptrace(PTRACE_POKEUSER, m->pid, i, 0) < 0) {
            diagnostic("ptrace pokeuser");
            goto failure;
        }
    } else {
        diagnostic("illegal transition rule mode combination: 0x%x", o);
        goto failure;
    }

    /* Fetch new instruction count.  */
  count:
    if (read(m->fd, c + 1, sizeof(long)) < 0) {
        diagnostic("read");
        goto failure;
    }

    /* Compute number of instructions executed in this invocation.  */
    c[2] = c[1] - c[0];

    /* Compute cumulative instruction count.  */
    m->retired += c[2];

    /* Fetch virtual register containing system call number if any.  */
    i = ptrace(PTRACE_PEEKUSER, m->pid, 8 * ORIG_RAX, 0);

    /* Bail out if ptrace error.  */
    if (errno) {
        diagnostic("ptrace peekuser");
        goto failure;
    }

    /* Check for system call.  */
    switch (i) {
    case -1:                   /* No system call attempted.  */
        goto success;
    case __NR_exit:            /* Emulate exit system call.  */
	goto halting;
    default:                   /* Reject every other system call.  */
        diagnostic("illegal system call: 0x%lx", i);
        goto failure;
    }

  success:
    return c[2];

  halting:
    return MIN(-1, c[2]);

  failure:
    return 0;
}


/* adds a struct rtn to list passed as argument
 * @param **rtn_stack: the list to add struct rtn to
 * @param rtn_addr: return address of struct rtn to be added to list
 */
void add(struct rtn **rtn_stack, long rtn_addr)
{
  struct rtn *new_rtn;
  new_rtn = (struct rtn*)malloc(sizeof(struct rtn));
  new_rtn->addr = rtn_addr;
  if(rtn_stack != NULL)
      new_rtn->next = *rtn_stack;
  else
      new_rtn->next = NULL;
  *rtn_stack = new_rtn;
}

/* removes the top (i.e: head) item from list passed as argument
 * updates list
 * @param **rtn_stack: list to remove top from
 * @return struct rtn: the item removed
 */
struct rtn* pop(struct rtn **rtn_stack)
{
  struct rtn *top_rtn;
  if(rtn_stack != NULL)
    {
      top_rtn = *rtn_stack;
      *rtn_stack = (*rtn_stack)->next;
    }
    else
      top_rtn = NULL;
  return top_rtn;
}
