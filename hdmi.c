#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#define CH7035B

/// BEGIN CHRONTEL 7053A
// I2C_ADDR
const char CH7053A_I2C_ADDR = 0x76;

// Page selection register
const char CH7053A_PAGE_SELECT = 0x03;

// Test Pattern Register 3
const char CH7053A_TST3 = 0x7F;
const char CH7053A_TST3_TSYNC = ( 1 << 5 );
const char CH7053A_TST3_TEST  = ( 1 << 5 );
const char CH7053A_TST3_TSTP = 0x0F;
const char CH7053A_TST3_TSTP_EXT = 0x00;
const char CH7053A_TST3_TSTP_WHITE = 0x01;
const char CH7053A_TST3_TSTP_VRAMP = 0x02;
const char CH7053A_TST3_TSTP_HRAMP = 0x03;
const char CH7053A_TST3_TSTP_COLBAR = 0x04;
const char CH7053A_TST3_TSTP_PRDTSTPAT = 0x07;
const char CH7053A_TST3_TSTP_DACSINE = 0x0B;
const char CH7053A_TST3_TSTP_DACRAMP = 0x0D;

// Power State Register 1
const char CH7053A_PWRST1 = 0x04;
const char CH7053A_PWRST1_FPD = ( 1 << 5 );

// Power State Register 3
const char CH7053A_PWRST3 = 0x09;
const char CH7053A_PWRST3_DIGITAL_PWRDWN = ( 1 << 7 );
const char CH7053A_PWRST3_GCKOFF = ( 1 << 6 );
const char CH7053A_PWRST3_TV_BYPASS = ( 1 << 5 );
const char CH7053A_PWRST3_SCALER_PWRDWN = ( 1 << 4 );
const char CH7053A_PWRST3_SDRAM_PWRDWN = ( 1 << 3 );
const char CH7053A_PWRST3_VGA_PWRDWN = ( 1 << 2 );
const char CH7053A_PWRST3_HDTV_PWRDWN = ( 1 << 1 );
const char CH7053A_PWRST3_DVI_PWRDWN = ( 1 );

// HDTV Output format
const char CH7053A_HDTV_OUTPUT_FORMAT = 0x2C;
const char CH7053A_HDTV_OUTPUT_FORMAT_HDTV_EN = ( 1 << 5 );
const char CH7053A_HDTV_OUTPUT_FORMAT_RESMASK = 0x1F;
const char CH7053A_HDTV_OUTPUT_FORMAT_480P = 0;
const char CH7053A_HDTV_OUTPUT_FORMAT_480P_DS = 1;
const char CH7053A_HDTV_OUTPUT_FORMAT_576P = 2;
const char CH7053A_HDTV_OUTPUT_FORMAT_576P_DS = 3;
const char CH7053A_HDTV_OUTPUT_FORMAT_720P_60 = 4;
const char CH7053A_HDTV_OUTPUT_FORMAT_720P_50 = 5;
const char CH7053A_HDTV_OUTPUT_FORMAT_1080I_60_274M = 6;
const char CH7053A_HDTV_OUTPUT_FORMAT_1080I_50_274M = 7;
const char CH7053A_HDTV_OUTPUT_FORMAT_1080I_50_295M = 8;
const char CH7053A_HDTV_OUTPUT_FORMAT_720P_60_DS = 9;
const char CH7053A_HDTV_OUTPUT_FORMAT_720P_50_DS = 10;
const char CH7053A_HDTV_OUTPUT_FORMAT_1080P_30 = 11;
const char CH7053A_HDTV_OUTPUT_FORMAT_1080P_25 = 12;
const char CH7053A_HDTV_OUTPUT_FORMAT_1080P_24 = 13;
const char CH7053A_HDTV_OUTPUT_FORMAT_1080P_60 = 14;
const char CH7053A_HDTV_OUTPUT_FORMAT_1080P_50_274M = 15;
const char CH7053A_HDTV_OUTPUT_FORMAT_1080P_50_295M = 16;
const char CH7053A_HDTV_OUTPUT_FORMAT_1035i_60_240M = 17;

// Crystal Frequency 1
const char CH7053A_CRYSTAL_FREQ1 = 0x1D;

// Crystal Frequency 2
const char CH7053A_CRYSTAL_FREQ2 = 0x1E;

const char CH7035B_OUTPUT_TIMING1 = 0x1F; //0b00011010 
const char CH7035B_OUTPUT_TIMING2 = 0x20; //0b10000000
const char CH7035B_OUTPUT_TIMING3 = 0x21; //0b00100000
const char CH7035B_OUTPUT_TIMING7 = 0x25; //0b00010001
const char CH7035B_OUTPUT_TIMING8 = 0x26; //0b11100000
const char CH7035B_OUTPUT_TIMING9 = 0x27; //0b00001101


int ch7053a_write(int file, char reg, char byte) {
    if( i2c_smbus_write_byte_data(file, reg, byte) < 0 )
        printf("Error: %s\n", strerror( errno ) );
}

int ch7053a_read(int file, char reg) {
    int data = i2c_smbus_read_byte_data(file, reg);
    printf("Read [0x%02X]: 0x%02X\n", reg, data );
    return data;
}

int ch7053a_write_and_read(int file, char reg, char byte) {
    ch7053a_write(file, reg, byte);
    // sleep for a short time
    struct timespec halfsec = { .tv_sec = 0, .tv_nsec = 7812500 };
    nanosleep( &halfsec );
    // read back test function select register
    return ch7053a_read(file, reg);
}

int ch7053a_set_page( int file, int page ) {
    const char page_map[4] = { 0, 1, 3, 4 };

    // only allow pages 1-4
    if( ( page > 0) && ( page < 5 ) )
        return ch7053a_write_and_read(file, CH7053A_PAGE_SELECT, page_map[ page - 1 ] );
    else return -1;
}


int ch7035b_write_output_timings(int file, unsigned short hto, unsigned short hao, unsigned short vto, unsigned short vao) {
    ch7053a_set_page(file, 1);

    hto = hto & 0x7FF; // 11 bits
    hao = hao & 0x3FF; // 10 bits
    vto = vto & 0x3FF; // 10 bits
    vao = vao & 0x3FF; // 10 bits
    unsigned char timing1 = ( 0b01111000 & ( ( hto & 0x3C0 ) >> 3 ) ); // hto[11:8] into timing1
    timing1 |= ( 0b00000111 & ( ( hao & 0x1C0 ) >> 5 ) ); // hao[10:8] into timing1
    unsigned char timing2 = ( hao & 0x00FF ); // hao[7:0] into timing2
    unsigned char timing3 = ( hto & 0x00FF ); // hto[7:0] into timing2

    unsigned char timing7 = ( 0b00111000 & ( ( vto & 0x1C0 ) >> 3 ) ); // vto[10:8] into timing7
    timing7 |= ( 0b00000111 & ( ( vao & 0x1C0 ) >> 5 ) ); // vao[10:8] into timing7
    unsigned char timing8 = ( vao & 0x00FF ); // vao[7:0] into timing8
    unsigned char timing9 = ( vto & 0x00FF ); // vto[7:0] into timing9

    printf("\nBegin output timings\n");
    ch7053a_write_and_read(file, CH7035B_OUTPUT_TIMING1, timing1);
    ch7053a_write_and_read(file, CH7035B_OUTPUT_TIMING2, timing2);
    ch7053a_write_and_read(file, CH7035B_OUTPUT_TIMING3, timing3);
    ch7053a_write_and_read(file, CH7035B_OUTPUT_TIMING7, timing7);
    ch7053a_write_and_read(file, CH7035B_OUTPUT_TIMING8, timing8);
    ch7053a_write_and_read(file, CH7035B_OUTPUT_TIMING9, timing9);
    printf("End output timings\n\n");
}

// use -1 for any values that should not be set here
int ch7053a_set_test_pattern( int file, int tsync, int test, int tstp ) {
    ch7053a_set_page(file, 1);
    char test_pattern_value = 0;
    if( tsync > 0 )
        test_pattern_value |= CH7053A_TST3_TSYNC;

    if( test > 0 )
        test_pattern_value |= CH7053A_TST3_TEST;

    if( tstp > 0 )
        test_pattern_value |= ( tstp & CH7053A_TST3_TSTP );

    return ch7053a_write_and_read(file, CH7053A_TST3, test_pattern_value);
}

int ch7053a_set_power_state_3(  int file,
                                int digital_path,
                                int graphics_clock,
                                int tv_bypass,
                                int scaler,
                                int sdram,
                                int vga,
                                int hdtv,
                                int dvi ) {

    ch7053a_set_page(file, 1);
    char pwrstate_value = 0b00011111;

    if( digital_path > 0 )
        pwrstate_value |= CH7053A_PWRST3_DIGITAL_PWRDWN;

    if( graphics_clock > 0 )
        pwrstate_value |= CH7053A_PWRST3_GCKOFF;

    if( tv_bypass > 0 )
        pwrstate_value |= CH7053A_PWRST3_TV_BYPASS;

#ifdef CH7035B
    if( scaler > 0 )
        pwrstate_value |= CH7053A_PWRST3_SCALER_PWRDWN;

    if( sdram > 0 )
        pwrstate_value |= CH7053A_PWRST3_SDRAM_PWRDWN;
#endif

#ifndef CH7035B
    if( vga > 0 )
        pwrstate_value ^= CH7053A_PWRST3_VGA_PWRDWN;

    if( hdtv > 0 )
        pwrstate_value ^= CH7053A_PWRST3_HDTV_PWRDWN;
#endif

    if( dvi > 0 )
        pwrstate_value ^= CH7053A_PWRST3_DVI_PWRDWN;

    return ch7053a_write_and_read(file, CH7053A_PWRST3, pwrstate_value);
}
int ch7053a_set_crystal_frequency( int file, uint16_t frequency ) {
    uint8_t* freq_ptr = (uint8_t*)&frequency;
    ch7053a_set_page(file, 1);
    ch7053a_write_and_read(file, CH7053A_CRYSTAL_FREQ1, freq_ptr[0] );
    ch7053a_write_and_read(file, CH7053A_CRYSTAL_FREQ2, freq_ptr[1] );
}

int ch7053a_set_hdtv_output_format( int file, int enable, int format ) {
    char output_format = 0b10000000;
    if ( enable > 0 )
        output_format |= CH7053A_HDTV_OUTPUT_FORMAT_HDTV_EN;
    if ( ( format & CH7053A_HDTV_OUTPUT_FORMAT_RESMASK ) ) {
        ch7053a_set_page(file, 1);
        output_format |= ( format & CH7053A_HDTV_OUTPUT_FORMAT_RESMASK );
        ch7053a_write_and_read(file, CH7053A_HDTV_OUTPUT_FORMAT, output_format );
    
    }
}

/// END CHRONTEL 7053A


int i2c_dev_open( int address ) {
    int file;
    if ( (file = open("/dev/i2c-1", O_RDWR)) < 0 ) {
        printf("Failed to open the bus.");
        printf("Error: %s\n", strerror( errno ) );
        exit(1);
    }

    if ( ioctl(file, I2C_SLAVE, address) < 0 ) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        printf("Error: %s\n", strerror( errno ) );
        exit(1);
    }
    return file;
}


int main( int argc, char** argv ) {
    int file = i2c_dev_open( CH7053A_I2C_ADDR );

    ch7053a_set_crystal_frequency(file, 27000);
    ch7053a_set_power_state_3(file, 0, 0, 0, 0, 0, 1, 0, 0);
    //ch7053a_set_hdtv_output_format(file, 1, CH7053A_HDTV_OUTPUT_FORMAT_480P);
    ch7035b_write_output_timings(file, 1280,1280,720,720);
    ch7053a_set_test_pattern(file, 1, 1, CH7053A_TST3_TSTP_COLBAR);
    ch7053a_set_page(file, 4);
    ch7053a_read(file, 0x61);
}