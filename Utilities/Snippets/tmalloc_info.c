//snipped to extrract memory information from tmalloc_slots
//1. chceck location of tmalloc_slots and tmalloc_filenames, compiler might change them
//2. export dtcm memory via gbd command 'dump memory dtcm.bin 0x20000000 0x20020000'
//3. run the program
//optional parameter (address) will indicate rows that contains the address
//zero size means that the slot was freed, file and lineno was changed to tfree location, goot for hunt double free
//iter increment with every memory operation tmalloc or tfree


#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

typedef struct
{
    uint32_t ptr;
    uint32_t size;
    uint32_t last_size;
    uint32_t itter;
    uint16_t file;
    uint16_t lineno;
} mem_slot_t;

#define FILE_NAME_SIZE  16
#define NUMBER_OF_SLOTS 3225
#define NUMBER_OF_FILES 64

mem_slot_t tmalloc_slots[NUMBER_OF_SLOTS] = {0};
char tmalloc_filenames[NUMBER_OF_FILES][FILE_NAME_SIZE] = {0};

#define INFO(...) {printf(__VA_ARGS__); printf("\n");}

typedef struct
{
    uint16_t file;
    uint16_t line;
    uint32_t cnt;
    uint32_t total;
} slot_sumary_t;

#define SUMARY_SLOTS    32

slot_sumary_t * tmalloc_find_sumary_slot(slot_sumary_t * slots, uint32_t file, uint16_t line)
{
    for (uint16_t i = 0; i < SUMARY_SLOTS; i++)
    {
        if (line == slots[i].line)
        {
            if (slots[i].file == file)
            {
                return &slots[i];
            }
        }

        if (slots[i].file == 0xFFFF)
        {
            slots[i].file = file;
            slots[i].line = line;

            return &slots[i];
        }
    }

    return NULL;
}

void tmalloc_summary_info_unlocked()
{
    slot_sumary_t slots[SUMARY_SLOTS] = {0};

    for (uint16_t i = 0; i < SUMARY_SLOTS; i++)
    {
        slots[i].file = 0xFFFF;
    }

    for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
    {
        if (tmalloc_slots[i].size != 0)
        {
            slot_sumary_t * slot = tmalloc_find_sumary_slot(slots, tmalloc_slots[i].file, tmalloc_slots[i].lineno);

            if (slot != NULL)
            {
                slot->cnt++;
                slot->total += tmalloc_slots[i].size;
            }

        }
    }

    uint32_t total_slots = 0;
    uint32_t total_allocated = 0;

    INFO(" slot    cnt       size");
    for (uint16_t i = 0; i < SUMARY_SLOTS; i++)
    {
        if (slots[i].file != 0xFFFF)
        {
            INFO(" %4u %6u %10u %s:%u", i, slots[i].cnt, slots[i].total, tmalloc_filenames[slots[i].file], slots[i].line);
            total_slots += slots[i].cnt;
            total_allocated += slots[i].total; 
        }
        else
        {
            break;
        }
    }


    INFO("total %u %u", total_slots, total_allocated);
    INFO("");
}

int cmpfunc (const void * a, const void * b)
{
    mem_slot_t * ma = a;
    mem_slot_t * mb = b;
    
    if (ma->ptr == mb->ptr)
    {
        if (ma->size != ma->size)
        {
            if (ma->size == 0)
                return -1;
            if (mb->size == 0)
                return 1;

            return ma->size - mb->size;            
        }
        else
        {
            return ma->itter - mb->itter;            
        }
    }
    
    return ma->ptr - mb->ptr;
}

int main(int argc, char *argv[])
{
    uint32_t locate = 0;
    
    if (argc == 2)
    {
        locate = strtol(argv[1], NULL, 16);
        INFO("looking for %08X", locate);
    }

    FILE * f = fopen("dtcm.bin", "r");
    
    if (f > 0)
    {
        INFO("Reading file");

        int rd;

        rd = fread(tmalloc_slots, sizeof(mem_slot_t), NUMBER_OF_SLOTS, f);
        INFO("rd = %d", rd);

        
        rd = fread(tmalloc_filenames, FILE_NAME_SIZE, NUMBER_OF_FILES, f);
        INFO("rd = %d", rd);
        
/*
        for (uint16_t i = 0; i < NUMBER_OF_FILES; i++)
        {
            INFO(" %u '%s'", i, tmalloc_filenames[i]);
        }
*/
        
        qsort(tmalloc_slots, NUMBER_OF_SLOTS, sizeof(mem_slot_t), cmpfunc);
       
        INFO("  iteration    memory range         size (     was) location of tmalloc or tfree ");
        for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
        {
            if (tmalloc_slots[i].ptr != 0)
            {
                uint32_t size = tmalloc_slots[i].size ? tmalloc_slots[i].size : tmalloc_slots[i].last_size;
            
                char p = ' ';
                if (tmalloc_slots[i].ptr <= locate && tmalloc_slots[i].ptr + size >= locate)
                    p = '>';
            
                INFO("%c%10u %08X - %08X %8u (%8u) %s:%u", 
                    p, tmalloc_slots[i].itter, 
                    tmalloc_slots[i].ptr, tmalloc_slots[i].ptr + size,
                    tmalloc_slots[i].size, tmalloc_slots[i].last_size, 
                    tmalloc_filenames[tmalloc_slots[i].file], tmalloc_slots[i].lineno);
            }
        }
        
        INFO("\n");
        tmalloc_summary_info_unlocked();
    }
    else
    {
        INFO("Unable to open file");
    }
    
    fclose(f);
    
    return 0;
}
