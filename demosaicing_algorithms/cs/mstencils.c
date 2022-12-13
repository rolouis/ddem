/* Generated 2022-12-09 12:58:20 */

/**
 * @file mstencils.c
 * @brief Mosaiced contour stencils
 * @author Pascal Getreuer <getreuer@gmail.com>
 */

#include <math.h>

/** @brief Compute the absolute difference between (x1,y1) and (x2,y2) */
#define TVEDGE(x1,y1,x2,y2)                               \
    fabs(Input[HorizontalOffset[x1] + VerticalOffset[y1]] \
    - Input[HorizontalOffset[x2] + VerticalOffset[y2]])

/** @brief Number of contour stencils */
#define NUMSTENCILS             8

/** @brief Radius of the stencil neighborhood */
#define NEIGHRADIUS             2

/* Contour stencil weights */
#define WEIGHT_AXIAL            4.545454545454546e-02
#define WEIGHT_PI_8             1.181325147609750e-02
#define WEIGHT_GREEN_MU         1.111167799007432e+00
#define WEIGHT_GREEN_DIAGONAL   5.050762722761053e-02
#define WEIGHT_REDBLUE_MU       1.111167799007432e+00
#define WEIGHT_REDBLUE_DIAGONAL 5.050762722761053e-02

/**
 * @brief Estimate the contour orientations of a mosaiced image
 * @param Stencil array to store the selected stencils
 * @param Input the mosaiced image
 * @param Width, Height dimensions of the image
 * @param RedX, RedY the coordinates of the upper-leftmost red pixel
 * 
 * Mosaiced contour stencils are applied to estimate the contour orientation
 * at each pixel.  The output array Stencil holds the index of the selected
 * stencil at each point.  The orientation estimate at the ith pixel is
 * Stencil[i]*M_PI/8 radians:
@verbatim
       5  4  3
     6,   |   ,2 
    7  `, | ,`  1
    0-----+-----0
Orientations described
    by Stencil[i]
@endverbatim
 * The same enumeration of orientations is used internally for the TV array.
 */
void FitMosaicedStencils(int *Stencil, const float *Input,
    int Width, int Height, int RedX, int RedY)
{
    const int Green = 1 - ((RedX + RedY) & 1);
    double TV[NUMSTENCILS], MinTV;
    int Offsets[2*(2*NEIGHRADIUS + 1)];
    int *HorizontalOffset = Offsets + NEIGHRADIUS;
    int *VerticalOffset = Offsets + (3*NEIGHRADIUS + 1);
    int x, y, k, S;

    if(!Stencil || !Input
        || Width < NEIGHRADIUS + 1 || Height < NEIGHRADIUS + 1)
        return;

    HorizontalOffset[0] = VerticalOffset[0] = 0;

    for(y = 0; y < Height; y++)
    {
        for(k = 1; k <= NEIGHRADIUS; k++)
        {
            VerticalOffset[-k] = (y >= k) ? -k*Width : (k - 2*y)*Width;
            VerticalOffset[k]  = (y < Height - k) ?
                k*Width : 2*(Height - k - y)*Width;
        }

        for(x = 0; x < Width; x++, Input++, Stencil++)
        {
            for(k = 1; k <= NEIGHRADIUS; k++)
            {
                HorizontalOffset[-k] = (x >= k) ? -k : (k - 2*x);
                HorizontalOffset[k]  = (x < Width - k) ?
                    k : 2*(Width - k - x);
            }

            TV[0] = TVEDGE(-1,-2,   1,-2)
                  + TVEDGE(-2,-1,   0,-1)
                  + TVEDGE(-1,-1,   1,-1)
                  + TVEDGE( 0,-1,   2,-1)
                  + TVEDGE(-2, 0,   0, 0)
                  + TVEDGE(-1, 0,   1, 0)
                  + TVEDGE( 0, 0,   2, 0)
                  + TVEDGE(-2, 1,   0, 1)
                  + TVEDGE(-1, 1,   1, 1)
                  + TVEDGE( 0, 1,   2, 1)
                  + TVEDGE(-1, 2,   1, 2);
            TV[4] = TVEDGE(-1, 0,  -1,-2)
                  + TVEDGE( 0, 0,   0,-2)
                  + TVEDGE( 1, 0,   1,-2)
                  + TVEDGE(-2, 1,  -2,-1)
                  + TVEDGE(-1, 1,  -1,-1)
                  + TVEDGE( 0, 1,   0,-1)
                  + TVEDGE( 1, 1,   1,-1)
                  + TVEDGE( 2, 1,   2,-1)
                  + TVEDGE(-1, 2,  -1, 0)
                  + TVEDGE( 0, 2,   0, 0)
                  + TVEDGE( 1, 2,   1, 0);

            if(((x + y) & 1) == Green)  /* Center pixel is green */
            {
                TV[2] = TVEDGE(-1,-1,   0,-2)
                      + TVEDGE(-2, 0,  -1,-1)
                      + TVEDGE(-1, 0,   1,-2)
                      + TVEDGE( 0, 0,   1,-1)
                      + TVEDGE(-2, 1,   0,-1)
                      + TVEDGE(-1, 1,   0, 0)
                      + TVEDGE( 0, 1,   2,-1)
                      + TVEDGE( 1, 1,   2, 0)
                      + TVEDGE(-1, 2,   1, 0)
                      + TVEDGE( 0, 2,   1, 1);
                TV[6] = TVEDGE( 1,-1,   0,-2)
                      + TVEDGE( 0, 0,  -1,-1)
                      + TVEDGE( 1, 0,  -1,-2)
                      + TVEDGE( 2, 0,   1,-1)
                      + TVEDGE(-1, 1,  -2, 0)
                      + TVEDGE( 0, 1,  -2,-1)
                      + TVEDGE( 1, 1,   0, 0)
                      + TVEDGE( 2, 1,   0,-1)
                      + TVEDGE( 0, 2,  -1, 1)
                      + TVEDGE( 1, 2,  -1, 0);

                /* Compute TVs for odd multiples of pi/8 as linear
                   combinations of axial and diagonal TVs.          */
                TV[1] = TV[0] + WEIGHT_GREEN_MU*TV[2];
                TV[3] = TV[4] + WEIGHT_GREEN_MU*TV[2];
                TV[5] = TV[4] + WEIGHT_GREEN_MU*TV[6];
                TV[7] = TV[0] + WEIGHT_GREEN_MU*TV[6];

                TV[2] *= WEIGHT_GREEN_DIAGONAL;
                TV[6] *= WEIGHT_GREEN_DIAGONAL;
            }
            else    /* Center pixel is red or blue */
            {
                TV[2] = TVEDGE(-2,-1,  -1,-2)
                      + TVEDGE( 0,-1,   1,-2)
                      + TVEDGE(-2, 0,   0,-2)
                      + TVEDGE(-1, 0,   0,-1)
                      + TVEDGE( 1, 0,   2,-1)
                      + TVEDGE(-2, 1,  -1, 0)
                      + TVEDGE(-1, 1,   1,-1)
                      + TVEDGE( 0, 1,   1, 0)
                      + TVEDGE(-1, 2,   0, 1)
                      + TVEDGE( 0, 2,   2, 0)
                      + TVEDGE( 1, 2,   2, 1);
                TV[6] = TVEDGE( 0,-1,  -1,-2)
                      + TVEDGE( 2,-1,   1,-2)
                      + TVEDGE(-1, 0,  -2,-1)
                      + TVEDGE( 1, 0,   0,-1)
                      + TVEDGE( 2, 0,   0,-2)
                      + TVEDGE( 0, 1,  -1, 0)
                      + TVEDGE( 1, 1,  -1,-1)
                      + TVEDGE( 2, 1,   1, 0)
                      + TVEDGE(-1, 2,  -2, 1)
                      + TVEDGE( 0, 2,  -2, 0)
                      + TVEDGE( 1, 2,   0, 1);

                TV[1] = TV[0] + WEIGHT_REDBLUE_MU*TV[2];
                TV[3] = TV[4] + WEIGHT_REDBLUE_MU*TV[2];
                TV[5] = TV[4] + WEIGHT_REDBLUE_MU*TV[6];
                TV[7] = TV[0] + WEIGHT_REDBLUE_MU*TV[6];

                TV[2] *= WEIGHT_REDBLUE_DIAGONAL;
                TV[6] *= WEIGHT_REDBLUE_DIAGONAL;
            }

            TV[0] *= WEIGHT_AXIAL;
            TV[1] *= WEIGHT_PI_8;
            TV[3] *= WEIGHT_PI_8;
            TV[4] *= WEIGHT_AXIAL;
            TV[5] *= WEIGHT_PI_8;
            TV[7] *= WEIGHT_PI_8;

            /* Select the best-fitting stencil */
            MinTV = TV[S = 0];

            for(k = 1; k < NUMSTENCILS; k++)
                if(TV[k] < MinTV)
                {
                    MinTV = TV[k];
                    S = k;
                }

            /* Store the selected stencil for the current pixel */
            *Stencil = S;
        }
    }
}
