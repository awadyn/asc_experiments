#ifndef __ASC_IO__
#define __ASC_IO__


#define CONIO_PAGESIZE 4096
#define CONIO_NUM_OUTPUTPAGES 16

#ifdef ASCIO_NO_OUTPUT
static char PRINTF_BUFFER[4096];
#define printf(...) snprintf(PRINTF_BUFFER, sizeof(PRINTF_BUFFER), __VA_ARGS__)
#endif

#ifdef ASCIO_STD_INPUT
//#ifdef ASCIO_INPUT
#define asc_getc getchar
#else
static char asc_conio_input_data[] = ASCIO_INPUT;
static int asc_conio_input_cursor=0;
#define asc_conio_input_len (sizeof(asc_conio_input_data))
static inline int asc_getc() {
  if (asc_conio_input_cursor >= asc_conio_input_len) return EOF;
  char c = asc_conio_input_data[asc_conio_input_cursor];
  asc_conio_input_cursor++;
  return c;
}
#endif

#if 0
static asc_conio_output_data[CONIO_PAGESIZE * CONIO_NUM_OUTPUTPAGES];
static char asc_conio_output_cursor=0;
static inline asc_putc(int c)
{
  asc_conio_output_data[asc_conio_output_cursor];
  asc_conio_output_cursor = (asc_conio_output_cursor + 1) % sizeof(asc_conio_output_data);
}
#endif
#endif
