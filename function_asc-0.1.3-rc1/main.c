#include <asc.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/param.h>
#include <sys/ptrace.h>
#include <uthash.h>



/* idx: index of excited bits in state vector */
long idx;

/* define hashable structure  */
typedef struct my_struct
{
	unsigned long boff;
	unsigned char byte;
	UT_hash_handle hh;
}my_struct;

/* ebhash: hashable structure to store bytes containing bits excited during execution */
struct my_struct *ebhash = NULL;

/* uthash_put: adds a struct my_struct to hash table
 * @param boff: key
 * @param *byte: pointer to the byte containing excited bits
 */
void uthash_put(unsigned long boff, unsigned char *byte)
{
	struct my_struct *b;
	b = (struct my_struct*)malloc(sizeof(struct my_struct));
	b->boff = boff;
	memcpy(&(b->byte), (void*)byte, sizeof(char)); 
	HASH_ADD(hh, ebhash, boff, sizeof(unsigned long), b);
}

/* uthash_get: returns item from hash table with key = *boff
 * @param boff: pointer to key *boff
 * @return struct my_struct with key = *boff
 */
struct my_struct *uthash_get(unsigned long *boff)
{
	struct my_struct *b;
	HASH_FIND(hh, ebhash, (void*)boff, sizeof(unsigned long), b);
	return b;
}




static struct machine m[1] = {{
    .metrics = "round,hash,pc,excited,bitrate,delta,wrong,error,mips",
    .rounds = LONG_MAX,
}};

static struct fann_train_data *D;


/*
 * x: input state vector
 * y: output state vector
 * u: predicted output state vector
 * e: excited bits state vector
 * d: vector representing difference between x and y (xor)
 */
static mpz_t x, y, d, e, u;
static mpz_t temp;

//b: pointer to buffer where hashing will take place
static unsigned char *b;


//n: dimension of m (i.e: number of bits that represent the entire machine state)
//a: number of correct predictions (hits)
static long a, n;


int main(int argc, char *argv[])
{
    sigset_t mask;
    FILE *stream;
  



        FILE *bytes;
        bytes = fopen("bytes.data", "w");

        FILE *bytes_offset;
        bytes_offset = fopen("bytes_offset.data", "w");
	
	FILE *stack_excited;
	stack_excited = fopen("stack_excited.data", "w");

        FILE *gpregs;
        gpregs = fopen("gpregs.data", "w");

        FILE *stack;
        stack = fopen("stack.data", "w");

    char *path;
    

    /*
     * r: rounds
     * c: number or retired instructions
     */
    long c, r;


    /* Parse command line arguments.  */
    if (options(m, argc, argv) < 0)
        goto failure;

    /* Print usage statement and exit if so configured.  */
    if (m->flags & HELP) {
        help();
        goto success;
    }




    /* Initialize vectors.  */
    mpz_inits(x, y, d, e, u, 0);

    /* Prepare execution context.  */
    if ((n = initial(y, x, m, argc - optind, argv + optind)) < 0)
        goto failure;       


    /* Allocate buffer for portable cryptographic hashes.  */
    if ((b = calloc(4 + n / 8, sizeof(unsigned char))) == 0)
        goto failure;

    /* Display output.  */
    if (status(r = 0, x, y, u, d, e, b, a = 0, m) < 0)
        goto failure;



    /* Initial transient integration loop.  */
    for (r = 1, mpz_set(x, y); r < MIN(3, m->rounds); r++, mpz_swap(x, y)) {

	if(m->mode == FUNCTION)
	{
        	if ((c = integrate(y, x, m, m->mode)) == 0)
	  		goto failure;

        	if (c < 0)
            		goto success;

		mpz_set(x, y);

        	if ((c = integrate(y, x, m, m->mode)) == 0)
          		goto failure;
	}
	else
	{
		if((c = integrate(y, x, m, m->mode)) == 0)
			goto failure;
	}
	if (c < 0)
            	goto success;

        mpz_xor(d, y, x);

        if (status(r, x, y, u, d, e, b, a = 0, m) < 0)
            goto failure;
    }




    if (m->excitations)
        mpz_set(e, m->excitations);

    if (m->data) {
        D = m->data;
    } else {
        D = calloc(1, sizeof(struct fann_train_data));
    }


    /* Master integrator loop.  */
    for ( ; r < m->rounds; r++, mpz_swap(x, y)) {

	if(m->mode == FUNCTION)
	{
	      	if ((c = integrate(y, x, m, m->mode)) == 0)
            		goto failure;

	        if (c < 0)
        		break;
	
		mpz_set(x, y);

		predict(u, &m->model, D, x, e);

        	if ((c = integrate(y, x, m, m->mode)) == 0)
            		goto failure;
	}
	else
	{
		predict(u, &m->model, D, x, e);

		if((c = integrate(y, x, m, m->mode)) == 0)
			goto failure;
	}

	if (c < 0)
		break;

	if (mpz_cmp(y, u) == 0)
		a ++;

        mpz_xor(d, y, x);

        if (status(r, x, y, u, d, e, b, a, m) < 0)
            goto failure;

    	mpz_ior(e, e, d);
	    
    	m->Bits += mpz_popcount(d);
	   
    	if (update(m->model, D, x, d, u, e) < 0)
      		goto failure;


 
	unsigned long currentBoff = -1;
	unsigned char byte;
	unsigned long boff;
	fprintf(stack_excited, "round %ld\n", r);
	for(idx = mpz_scan1(e, 0); idx> 0; idx = mpz_scan1(e, idx + 1))
	{
		boff = idx >> 3;
		if (boff != currentBoff)
		{
		        fwrite(&byte, 1, 1, bytes);

			if((boff >= 13336) && (boff < 148504))
			  {
			    fprintf(stack_excited, "map offset: %7ld\t", boff);
			    fprintf(stack_excited, "base offset: %7lx\t", (boff -13336)+0x7ffffffde000);
			    long val = ptrace(PTRACE_PEEKDATA, m->pid, (boff - 13336)+0x7ffffffde000);
			    if(errno)
			      {
				diagnostic("ptrace peek");
				goto failure;
			      }
			    fprintf(stack_excited, "value = %7lx\n", val);
			  }

			if(!uthash_get(&boff))
			{
				byte = 0;
				int ctr;
				for(ctr = 0; ctr < 8; ctr ++)
				  byte |= ((mpz_tstbit(x, boff*8+ctr))<<(7-ctr));
				uthash_put(boff, &byte);
			}
  			else
			  {
			    byte = uthash_get(&boff)->byte;
			    byte &= ~((1 << 7)>>(idx-boff*8));
			    byte |= (mpz_tstbit(x, idx) << 7)>>(idx-boff*8);
			    uthash_get(&boff)->byte = byte;
			  }
		       
			currentBoff = boff;
		}
		else
		  {
		    byte &= ~((1 << 7)>>(idx-boff*8));
		    byte |= (mpz_tstbit(x, idx) << 7)>>(idx-boff*8);
		    uthash_get(&boff)->byte = byte;
		  }
		  
	}
	fprintf(stack_excited, "\n");


        for(idx = 0; idx < 216; idx ++)
        {
                int ctr;
                byte = 0;
                for(ctr = 0; ctr < 8; ctr ++)
			if(mpz_tstbit(e, idx*8+ctr))
                		byte |= ((mpz_tstbit(y, idx*8+ctr)) << (7-ctr));
                fwrite(&byte, 1, 1, gpregs);
        }


        for(idx = 147900; idx < 148100; idx ++)
        {
                int ctr;
                byte = 0;
                for(ctr = 0; ctr < 8; ctr ++)
			if(mpz_tstbit(e, idx*8+ctr))
                        	byte |= ((mpz_tstbit(y, idx*8+ctr)) << (7-ctr));
                fwrite(&byte, 1, 1, stack);
        }

    }
   
    /* after all rounds are over, loop over hash table to get byte offsets of all bytes containing excited bits
     */
	struct my_struct *bhashed;
	for(bhashed = ebhash; bhashed != NULL; bhashed = bhashed->hh.next)
		fprintf(bytes_offset, "%lu " , bhashed->boff);
    




    if (sigemptyset(&mask) < 0) {
        diagnostic("sigemptyset");
        goto failure;
    }

    if (sigaddset(&mask, SIGINT) < 0) {
        diagnostic("sigaddset");
        goto failure;
    }

    /* Mask interrupts.  */
    if (sigprocmask(SIG_BLOCK, &mask, 0) < 0) {
        diagnostic("sigprocmask");
        goto failure;
    }

    /* Write out the excitation vector.  */
    if (mpz_popcount(e) > 0) {

        if (asprintf(&path, "%s.excite", argv[optind]) < 0)
            goto failure;

        if ((stream = fopen(path, "w")) == 0) {
            diagnostic("fopen");
            goto failure;
        }

        if (mpz_out_raw(stream, e) == 0) {
            diagnostic("mpz_out_raw");
            goto failure;
        }

        if (fclose(stream)) {
            diagnostic("fclose");
            goto failure;
        }
    }

    /* Write out the training set.  */
    if (D->num_data > 0) {

        if (asprintf(&path, "%s.train", argv[optind]) < 0)
            goto failure;

        if (fann_save_train(D, path) < 0)
            goto failure;
    }

    /* Write out the weak learner.  */
    if (m->model && fann_get_learning_rate(m->model) > 0) {

        if (asprintf(&path, "%s.net", argv[optind]) < 0)
            goto failure;

        if (fann_save(m->model, path) < 0)
            goto failure;
    }

    /* Unmask interrupts.  */
    if (sigprocmask(SIG_UNBLOCK, &mask, 0) < 0) {
        diagnostic("sigprocmask");
        goto failure;
    }



        fclose(bytes);
        fclose(bytes_offset);
	fclose(stack_excited);
	fclose(gpregs);
        fclose(stack);

  success:
    return 0;

  failure:
    return 1;
}
