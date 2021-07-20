#include "common.h"

uint8_t hex_to_num(uint8_t c)
{
	if (c >= 'A')
		return c - 'A' + 10;
	else
		return c - '0';
}

uint8_t atoi_hex8(char * buffer)
{
	return hex_to_num(buffer[0]) << 4 | hex_to_num(buffer[1]);
}

uint32_t atoi_hex32(char * buffer)
{
	uint8_t c;
	uint32_t res = 0;

	for (uint8_t i = 0; i < 8; i++)
	{
		c = buffer[i];
		if (c == ',' || c == 0)
			break;

		res <<= 4;
		res |= hex_to_num(c);
	}

	return res;
}

bool start_with(char * s1, const char * s2)
{
	for (uint8_t i = 0; i < strlen(s2); i++)
	{
		if (s1[i] != s2[i])
			return false;
	}

	return true;
}

char * find_comma(char * str)
{
	return index(str, ',') + 1;
}

uint32_t pow_ten(uint8_t pow)
{
	uint32_t ret = 1;
	uint8_t i;

	for (i = 0; i < pow; i++)
		ret *= 10;

	return ret;
}

uint16_t atoi_c(char * str)
{
	uint16_t tmp = 0;
	uint8_t i = 0;

	while(ISDIGIT(str[i]))
	{
		tmp *= 10;
		tmp += str[i] - '0';
		i++;
	}

	return tmp;
}

uint32_t atoi_n(char * str, uint8_t n)
{
	uint32_t tmp = 0;
    uint8_t i;

	for (i=0; i < n; i++)
	{
		if (str[i] == ',')
			return tmp;
		if (str[i] == '.')
		{
			n++;
			continue;
		}

		tmp += (uint32_t)(str[i] - '0') * pow_ten(n - i - 1);
	}

	return tmp;
}

float atoi_f(char * str)
{
	float tmp = 0;
	uint8_t dot = 0;
	uint8_t i = 0;

	if (str[0] == '-')
		i++;

	while(ISDIGIT(str[i]) || str[i] == '.')
	{
		if (str[i] == '.')
		{
			dot = i;
			i++;
		}

		if (dot == 0)
		{
			tmp *= 10;
			tmp += str[i] - '0';
		}
		else
		{
			tmp += (str[i] - '0') / pow(10, i - dot);
		}

		i++;
	}

	if (str[0] == '-')
		tmp *= -1;

	return tmp;
}

int8_t complement2_7bit(uint8_t in)
{
#define C2_7bit_MASK 0b00111111

	if (in > C2_7bit_MASK)
		return (in & C2_7bit_MASK) - (C2_7bit_MASK + 1);

	return in;
}



int16_t complement2_16bit(uint16_t in)
{
#define C2_16bit_MASK 0b0111111111111111

    if (in > C2_16bit_MASK)
        return (in & C2_16bit_MASK) - (C2_16bit_MASK + 1);

    return in;
}

char * find_in_file(FIL * f, char * key, char * def, char * buff, uint16_t len)
{
	FSIZE_t start_pos = f_tell(f);
	bool loop = false;

	if (f_size(f) == 0)
	    return def;

	while (1)
	{
		while (f_gets(buff, len, f) != NULL)
		{
			if (strstr(buff, key) == buff && buff[strlen(key)] == '=')
			{
				//remove /n
				buff[strlen(buff) - 1] = 0;
				return buff + strlen(key) + 1;
			}

			if (loop && f_tell(f) >= start_pos)
			{
				return def;
			}
		}

		if (f_eof(f))
		{
			f_lseek(f, 0);
			loop = true;
		}
	}
}

uint8_t calc_crc(uint8_t crc, uint8_t key, uint8_t data)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        if ((data & 0x01) ^ (crc & 0x01))
        {
            crc = crc >> 1;
            crc = crc ^ key;
        }
        else
            crc = crc >> 1;
        data = data >> 1;
    }

    return crc;
}

uint32_t calc_crc32(uint32_t * data, uint32_t size)
{
    uint32_t crc = HAL_CRC_Calculate(&hcrc, data, size);
    return crc ^ 0xFFFFFFFF;
}

void get_tmp_filename(char * fname)
{
    static uint32_t counter = 0;

    sprintf(fname, "%s/%08X", PATH_TEMP_DIR, counter);
    counter++;
}

#define COPY_WORK_BUFFER_SIZE (1024 * 4)
bool copy_file(char * src, char * dst)
{
    FIL * f_src = (FIL *)malloc(sizeof(FIL));
    FIL * f_dst = (FIL *)malloc(sizeof(FIL));
    uint8_t * work_buffer = (uint8_t *) malloc(COPY_WORK_BUFFER_SIZE);

    if (f_src == NULL || f_dst == NULL || work_buffer == NULL)
    {
        if (f_src) free(f_src);
        if (f_dst) free(f_dst);
        if (work_buffer) free(work_buffer);

        return false;
    }

    bool ret = true;

    if (f_open(f_src, src, FA_READ) == FR_OK)
    {

        if (f_open(f_dst, dst, FA_WRITE | FA_CREATE_NEW) == FR_OK)
        {
            while (!f_eof(f_src))
            {
                UINT br, bw;

                f_read(f_src, work_buffer, COPY_WORK_BUFFER_SIZE, &br);
                f_write(f_dst, work_buffer, br, &bw);

                ASSERT(bw == br);
            }
            f_close(f_dst);
        }
        else
        {
            ret = false;
        }

        f_close(f_src);
    }
    else
    {
        ret = false;
    }

    free(f_src);
    free(f_dst);
    free(work_buffer);

    return ret;
}

FRESULT f_delete_node (
    TCHAR* path,    /* Path name buffer with the sub-directory to delete */
    UINT sz_buff  /* Size of path name buffer (items) */
)
{
    UINT i, j;
    FRESULT fr;
    DIR dir;

    FILINFO fno;    /* Name read buffer */

    fr = f_opendir(&dir, path); /* Open the directory */
    if (fr != FR_OK) return fr;

    for (i = 0; path[i]; i++) ; /* Get current path length */
    path[i++] = _T('/');

    for (;;) {
        fr = f_readdir(&dir, &fno);  /* Get a directory item */
        if (fr != FR_OK || !fno.fname[0]) break;   /* End of directory? */
        j = 0;
        do {    /* Make a path name */
            if (i + j >= sz_buff) { /* Buffer over flow? */
                fr = FR_NO_PATH; break;    /* Fails with 100 when buffer overflow */
            }
            path[i + j] = fno.fname[j];
        } while (fno.fname[j++]);
        if (fno.fattrib & AM_DIR) {    /* Item is a directory */
            fr = f_delete_node(path, sz_buff);
        } else {                        /* Item is a file */
            fr = f_unlink(path);
        }
        if (fr != FR_OK) break;
    }

    path[--i] = 0;  /* Restore the path name */
    f_closedir(&dir);

    if (fr == FR_OK) fr = f_unlink(path);  /* Delete the empty directory */
    return fr;
}

void clear_dir(char * path)
{
    char path_buff[PATH_LEN];
    strcpy(path_buff, path);
    f_delete_node(path_buff, sizeof(path_buff));
}

bool read_value(char * data, char * key, char * value, uint16_t value_len)
{
    char * pos = strstr(data, key);
    if (pos == NULL)
        return false;

    if ((pos == data || *(pos - 1) == '\n') && pos[strlen(key)] == '=')
    {
        char * value_start = pos + strlen(key) + 1;
        char * value_end = strchr(value_start, '\n');
        value_len -= 1;

        if (value_start == value_end)
        {
            return false;
        }

        if (value_end == NULL)
        {
            strncpy(value, value_start, value_len);
            value[value_len] = 0;
        }
        else
        {
            value_len = min(value_end - value_start, value_len);
            strncpy(value, value_start, value_len);
            value[value_len] = 0;
        }

        return true;
    }

    return false;
}

/**
 * This table holds sinus values for 0-90 degrees.
 * The value is multiplied by 255 to use a uint8_t * for storage.
 */
const float sin_table[91] = {
        0, 0.017452406437284, 0.034899496702501, 0.052335956242944, 0.069756473744125, 0.087155742747658,
       0.104528463267653, 0.121869343405147, 0.139173100960065, 0.156434465040231, 0.17364817766693,
       0.190808995376545, 0.207911690817759, 0.224951054343865, 0.241921895599668, 0.258819045102521,
       0.275637355816999, 0.292371704722737, 0.309016994374947, 0.325568154457157, 0.342020143325669,
       0.3583679495453, 0.374606593415912, 0.390731128489274, 0.4067366430758, 0.4226182617407,
       0.438371146789078, 0.453990499739547, 0.469471562785891, 0.484809620246337, 0.5, 0.515038074910054,
       0.529919264233205, 0.544639035015027, 0.559192903470747, 0.573576436351046, 0.587785252292473,
       0.601815023152049, 0.615661475325659, 0.629320391049838, 0.64278760968654, 0.656059028990508,
       0.669130606358859, 0.681998360062499, 0.694658370458998, 0.707106781186548, 0.719339800338652,
       0.731353701619171, 0.743144825477395, 0.754709580222772, 0.766044443118979, 0.777145961456971,
       0.788010753606722, 0.798635510047293, 0.809016994374948, 0.819152044288992, 0.829037572555042,
       0.838670567945425, 0.848048096156427, 0.857167300702113, 0.866025403784439, 0.874619707139396,
       0.882947592858927, 0.891006524188368, 0.898794046299167, 0.90630778703665, 0.913545457642601,
       0.920504853452441, 0.927183854566788, 0.933580426497202, 0.939692620785909, 0.945518575599317,
       0.951056516295154, 0.956304755963036, 0.961261695938319, 0.965925826289069, 0.970295726275997,
       0.974370064785236, 0.978147600733806, 0.981627183447664, 0.984807753012208, 0.987688340595138,
       0.990268068741571, 0.992546151641322, 0.994521895368274, 0.996194698091746, 0.997564050259824,
       0.998629534754574, 0.999390827019096, 0.999847695156391, 1};

float table_sin(uint16_t angle)
{
    angle = angle % 360;
    float value;

    if (angle < 90)
        value = sin_table[angle];
    else if (angle < 180)
        value = sin_table[90 - (angle - 90)];
    else if (angle < 270)
        value = -sin_table[angle - 180];
    else
        value = -sin_table[90 - (angle - 270)];

    return value;
}

float table_cos(uint16_t angle)
{
    angle += 270;
    return table_sin(angle);
}

bool file_exists(char * path)
{
    FILINFO fno;
    return (f_stat(path, &fno) == FR_OK);
}

bool file_isdir(char * path)
{
    FILINFO fno;
    if (f_stat(path, &fno) == FR_OK)
    {
    	return fno.fattrib & AM_DIR;
    }
    return false;
}
