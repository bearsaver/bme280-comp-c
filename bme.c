/*
uses compensation functions from bme280 datasheet 
https://www.mouser.com/datasheet/2/783/BST-BME280-DS002-1509607.pdf

todo: get values dig_H1-dig_H6 and implement printing humidity    
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

#define dig_T1      0x6f8c
#define dig_T2      0x6976
#define dig_T3      0x0032

#define dig_P1      0x8e38
#define dig_P2      0xd70e
#define dig_P3      0x0bd0
#define dig_P4      0x18fa
#define dig_P5      0x001f
#define dig_P6      0xfff9
#define dig_P7      0x26ac
#define dig_P8      0xd80a
#define dig_P9      0x10bd

#define dig_H1      0
#define dig_H2      0
#define dig_H3      0
#define dig_H4      0
#define dig_H5      0
#define dig_H6      0

typedef int32_t BME280_S32_t;

// Returns temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC.
// t_fine carries fine temperature as global value
static BME280_S32_t t_fine;

double BME280_compensate_T_double(BME280_S32_t adc_T)
{
    double var1, var2, T;
    var1 = (((double)adc_T)/16384.0 - ((double)dig_T1)/1024.0) * ((double)dig_T2);
    var2 = ((((double)adc_T)/131072.0 - ((double)dig_T1)/8192.0) *
    (((double)adc_T)/131072.0 - ((double) dig_T1)/8192.0)) * ((double)dig_T3);
    t_fine = (BME280_S32_t)(var1 + var2);
    T = (var1 + var2) / 5120.0;
    return T;
}

// Returns pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa
double BME280_compensate_P_double(BME280_S32_t adc_P)
{
    double var1, var2, p;
    var1 = ((double)t_fine/2.0) - 64000.0;
    var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)dig_P5) * 2.0;
    var2 = (var2/4.0)+(((double)dig_P4) * 65536.0);
    var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0)*((double)dig_P1);
    if (var1 == 0.0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576.0 - (double)adc_P;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)dig_P9) * p * p / 2147483648.0;
    var2 = p * ((double)dig_P8) / 32768.0;
    p = p + (var1 + var2 + ((double)dig_P7)) / 16.0;
    return p;
}
// Returns humidity in %rH as as double. Output value of “46.332” represents 46.332 %rH
double bme280_compensate_H_double(BME280_S32_t adc_H)
{
    double var_H;
    var_H = (((double)t_fine) - 76800.0);
    var_H = (adc_H - (((double)dig_H4) * 64.0 + ((double)dig_H5) / 16384.0 *
            var_H)) * (((double)dig_H2) / 65536.0 * (1.0 + ((double)dig_H6) /
            67108864.0 * var_H *
            (1.0 + ((double)dig_H3) / 67108864.0 * var_H)));
    var_H = var_H * (1.0 - ((double)dig_H1) * var_H / 524288.0);
    if (var_H > 100.0) {
        var_H = 100.0;
    } else if (var_H < 0.0) {
        var_H = 0.0;
    }
    return var_H;
}

int main(int argc, char *argv[]) {

    BME280_S32_t adc_T = 0;
    BME280_S32_t adc_P = 0;

    char *arg_error = "invalid parameters. use \"bme -t <adc_T> -p <adc_P>";

    // validate input
    if (argc != 5) {
        printf("%s\n", arg_error);
        return 1;
    }

    if (!strcasecmp(argv[1], "-t")) {
        adc_T = (BME280_S32_t) strtoll(argv[2], NULL, 0);
    } else {
        printf("%s\n", arg_error);
        return 1;
    }

    if (!strcasecmp(argv[3], "-p")) {
        adc_P = (BME280_S32_t) strtoll(argv[4], NULL, 0);
    } else {
        printf("%s\n", arg_error);
        return 1;
    }


    double temp = BME280_compensate_T_double(adc_T);
    double pres = BME280_compensate_P_double(adc_P);

    printf("Temperature: %f\n", temp);
    printf("Humidity: %f\n", pres);

    return 0;
}