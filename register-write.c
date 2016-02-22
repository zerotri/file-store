#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// PMU register info
#define PMU_BASE_ADDRESS ( 0x01C25400 )
#define PMU_DVFS_CTRL_REG0 ( 0x00 )
#define PMU_DVFS_CTRL_REG1 ( 0x0004 )
#define PMU_DVFS_CTRL_REG2 ( 0x000C )
#define PMU_DVFS_CTRL_REG3 ( 0x0018 )
#define PMU_DVFS_TIMEOUT_CTRL_REG ( 0x001C)
#define PMU_VF_TABLE_REG0 ( 0x0080)
#define PMU_VF_TABLE_INDEX_REG ( 0xD0 )
#define PMU_IRQ_STATUS_REG ( 0x44 )

// SID register info
#define SID_BASE_ADDRESS ( 0x01c23800 )

static int mem_fd = -1;
static void* mem_ptr = 0;
static unsigned int mem_offset = 0;

int mem_init( int base_address ) {
	unsigned int pages_size = sysconf(_SC_PAGESIZE);
   	unsigned int page_mask = (~(pages_size-1));
   	unsigned int addr_start = base_address & page_mask;
   	unsigned int addr_offset = base_address & ~page_mask;
	/* Open /dev/mem */
	if ((mem_fd = open ("/dev/mem", O_RDWR | O_SYNC)) == -1)
		fprintf(stderr, "Cannot open /dev/mem\n"), exit(1);

	mem_ptr = mmap ( 0, pages_size*2, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, addr_start );
	mem_offset = addr_offset;

	if(mem_ptr == (void *) -1) {
		printf("Memory map failed. error %i\n", mem_ptr);
		perror("mmap");
		return -1;
	}

	return 0;
}

int mem_close() {
	if( mem_fd < 0 ) {
		return -1;
	}
	close( mem_fd );
	mem_ptr = 0;
	mem_offset = 0;
}

int mem_read_register( unsigned int reg, unsigned int* out ) {
	void *ptr = mem_ptr;
	if( mem_ptr == (void *) -1 )
		return -1;

	*out = *(unsigned int *)(mem_ptr + mem_offset + reg);
	return 0;
}

int mem_write_register( unsigned int reg, unsigned int value ) {
	void *ptr = mem_ptr;
	if( mem_ptr == (void *) -1 )
		return -1;

	*(unsigned int *)(mem_ptr + mem_offset + reg) = value;
	return 0;
}


int main( int argc, int argv ) {
	unsigned int value = 0;

	/*mem_init( SID_BASE_ADDRESS );

	mem_read_register( 0x00, &value );
	printf("SID_KEY0 = 0x%08X\n", value);

	mem_read_register( 0x0C, &value );
	printf("SID_KEY3 = 0x%08X\n", value);

	mem_close();*/


	mem_init( PMU_BASE_ADDRESS );


	mem_write_register( PMU_DVFS_CTRL_REG0, 0x00);

	mem_read_register( PMU_VF_TABLE_INDEX_REG, &value );
	printf("PMU_VF_TABLE_INDEX_REG = 0x%08X\n", value);


	mem_read_register( PMU_DVFS_CTRL_REG0, &value );
	printf("PMU_DVFS_CTRL_REG0 = 0x%08X\n", value);

	mem_read_register( PMU_VF_TABLE_REG0, &value );
	printf("PMU_VF_TABLE_REG0 = 0x%08X\n", value);

	mem_read_register( PMU_DVFS_CTRL_REG1, &value );
	printf("PMU_DVFS_CTRL_REG1 = 0x%08X\n", value);

	mem_read_register( PMU_DVFS_CTRL_REG2, &value );
	printf("PMU_DVFS_CTRL_REG2 = 0x%08X\n", value);

	mem_read_register( PMU_DVFS_CTRL_REG3, &value );
	printf("PMU_DVFS_CTRL_REG3 = 0x%08X\n", value);

	mem_close();

	return 0;
}