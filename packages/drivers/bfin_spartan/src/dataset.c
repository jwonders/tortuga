/*
 * Copyright (C) 2008 Robotics at Maryland
 * Copyright (C) 2008 Steve Moskovchenko <stevenm@umd.edu>
 * All rights reserved.
 *
 * Author: Steve Moskovchenko <stevenm@umd.edu>
 * File:  packages/drivers/bfin_spartan/src/dataset.c
 */

// STD Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// UNIX Includes
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Project Includes
#include "drivers/bfin_spartan/include/dataset.h"

struct dataset * createDataset(int size)
{
    if(size < 0)
    {
        fprintf(stderr, "Cannot create dataset of negative size\n");
        return NULL;
    }

    if(MAX_SEGMENTS * ALLOC_UNIT_SIZE < size)
    {
        fprintf(stderr, "Requested dataset too large for current allocation unit size and count\n");
        return NULL;
    }


    struct dataset * s = (struct dataset *)  malloc(sizeof(struct dataset));
    memset(s, 0, sizeof(struct dataset));

    int i, j;
    fprintf(stderr, "Using allocation units of %d samples\n", ALLOC_UNIT_SIZE);
    int numUnits = size / ALLOC_UNIT_SIZE;
    if(size > numUnits * ALLOC_UNIT_SIZE)
        numUnits++;

    for(i=0; i<numUnits; i++)
    {
        for(j=0; j<4; j++)
        {
            s->data[i][j] = (signed short *) malloc(ALLOC_UNIT_SIZE * sizeof(signed short));
            if(s->data[i][j] == NULL)
            {
                fprintf(stderr, "Could not allocate inot %d for channel %d\nEPIC FAIL\n", i, j);
                exit(-1);
            }
        }
    }

    s->size = size;
    s->numUnits = numUnits;

    return s;
}


int destroyDataset(struct dataset * s)
{
    if(s == NULL)
        return -1;

    int i, j;
    for(i=0; i<s->numUnits; i++)
        for(j=0; j<4; j++)
            free(s->data[i][j]);

    free(s);
    return 0;
}

signed short getSample(struct dataset* s, int ch, int index)
{
    if(s == NULL)
    {
        fprintf(stderr, "Bad dataset!\n");
        return -1;
    }

    if(ch < 0 || ch > 3)
    {
        fprintf(stderr, "Bad channel number: %d\n", ch);
        return -1;
    }

    if(index < 0 || index >= s->size)
    {
        fprintf(stderr, "Sample index out of range! Max: %d, requested: %d\n", s->size-1, index);
        return -1;
    }
    int unit = index >> ALLOC_UNIT_NUMBITS;
    int offset = index & ALLOC_UNIT_MASK;

    return *(s->data[unit][ch]+offset);
}

int putSample(struct dataset* s, int ch, int index, signed short value)
{
    if(s == NULL)
    {
        fprintf(stderr, "Bad dataset!\n");
        return -1;
    }

    if(ch < 0 || ch > 3)
    {
        fprintf(stderr, "Bad channel number: %d\n", ch);
        return -1;
    }

    if(index < 0 || index >= s->size)
    {
        fprintf(stderr, "Sample index out of range! Max: %d\n, requested: %d\n", s->size-1, index);
        return -1;
    }
    int unit = index >> ALLOC_UNIT_NUMBITS;
    int offset = index & ALLOC_UNIT_MASK;

    *(s->data[unit][ch]+offset) = value;

    return 0; //added by ML.  Otherwise, no return for non-void
}

struct dataset * loadDataset(const char * filename)
{
    struct stat fileStat;
    if(stat(filename, &fileStat) != 0)
    {
        fprintf(stderr, "Could not stat file\n");
        return NULL;
    }
    fprintf(stderr, "Loading a dataset of %d bytes\n", (int)fileStat.st_size); //gives warning about %d.  Check that this is right?
    struct dataset * s = createDataset(fileStat.st_size / 8);

    if(!s)
    {
        fprintf(stderr, "Could not allocate memory for dataset\n");
        exit(-1);
    }

    FILE * f = fopen(filename, "rb");

    if(!f)
    {
        fprintf(stderr, "Could not open dataset\n");
        destroyDataset(s);
        return NULL;
    }

    int i, j;
    for(i=0; i<fileStat.st_size / 8; i++)
        for(j=0; j<4; j++)
            putSample(s, j, i, (signed short) (fgetc(f) | (fgetc(f) << 8)));
    return s;
}
