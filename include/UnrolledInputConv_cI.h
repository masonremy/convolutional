////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  convolutional
//  File:     UnrolledInputChlMajInputConv_cI.h
//  Authors:  Ofer Dekel
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "BlasHelpers.h"
#include "ConvProperties.h"
#include "Tensor.h"

// Helper function that unrolls a channel-major input tensor into an unrolled input matrix
template <typename ElementType>
void ChlMajInputUnroll(const ElementType* X, 
    ElementType* U,
    int wRows,
    int wCols,
    int wChls,
    int vStride, 
    int yRows, 
    int yCols, 
    int uRows,
    int uCols)
{
    int copySize = yCols;
    int xRows = (yRows - 1) * vStride + wRows;
    int xCols = yCols + wCols - 1;
    int xChls = wChls;

    for(int wRow = 0; wRow < wRows; ++wRow) {
        for(int wCol = 0; wCol < wCols; ++wCol) {
            for(int wChl = 0; wChl < wChls; ++wChl) {
                for(int yRow = 0; yRow < yRows; ++yRow) {

                    // calculate copy source
                    int xRow = yRow * vStride + wRow;
                    int xCol = wCol;
                    int xChl = wChl;
                    const float* source = X + (xChl * xRows + xRow) * xCols + xCol;
                    
                    // calculate copy target
                    int uCol =  (wRow * wCols + wCol) * wChls + wChl;
                    ElementType* target = U + (uCol * yRows + yRow) * yCols;

                    // copy from X to U
                    assert(source >= X);
                    assert(source + copySize <= X + xRows * xCols * xChls);
                    assert(target >= U);
                    assert(target + copySize <= U + uRows * uCols);
                    std::copy(source, source + copySize, target);
                }   
            }  
        }   
    }   
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// 2D Tensor Convolution
// * supports only horizontal stride of 1
// * unrolled input 
// * filters in filter-major order
// * input tensor in channel-major order
// * output tensor in row-major order
// * requires temporary space of size (wRows * wCols * wChls * yRows * yCols)
//
// W: 4-dimensional weights tensor in filter-major order
// X: 3-dimensional input tensor in channel-major order
// Y: 3-dimensional output tensor in row-major order
// wCount: number of filters in W
// wRows: number of rows in each filter in W
// wCols: number of columns in each filter in W
// wChls: number of channels in each filter in W
// vStride: vertical stride
// yRows: number of rows in the output tensor Y
// yCols: number of columns in the output tensor Y
// space: pointer to temporary space of size at least (wRows * wCols * wChls * yRows * yCols)
template <typename ElementType>
void Convolution(ConvProperties<ChannelMajorInput, FilterMajorFilters, RowMajorOutput, UnitHorizontalStride, UnrolledInput>,
    const ElementType* W, 
    const ElementType* X, 
    ElementType* Y, 
    int wCount, 
    int wRows, 
    int wCols, 
    int wChls, 
    int vStride, 
    int yRows, 
    int yCols,
    ElementType* space)
{
    // use temp space to store the unrolled input matrix U in column-major order
    int uRows = yRows * yCols;
    int uCols = wRows * wCols * wChls;
    ElementType* U = space;

    // unroll the channel-major input
    ChlMajInputUnroll(X, U, wRows, wCols, wChls, vStride, yRows, yCols, uRows, uCols);

    // reshape the filters tensor W into a column-major matrix V
    int vCols = wCount;
    const ElementType* V = W;
    
    // reshape the output tensor Y into a row-major matrix Z
    ElementType* Z = Y;

    // matrix-matrix multiply
    Gemm(ColMaj, ColMaj, RowMaj, uRows, vCols, uCols, 1, U, V, 0, Z);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// 2D Tensor Convolution
// * supports only horizontal stride of 1
// * unrolled input 
// * filters in row-major order
// * input tensor in channel-major order
// * output tensor in row-major order
// * requires temporary space of size (wRows * wCols * wChls * yRows * yCols)
//
// W: 4-dimensional weights tensor in row-major order
// X: 3-dimensional input tensor in channel-major order
// Y: 3-dimensional output tensor in row-major order
// wCount: number of filters in W
// wRows: number of rows in each filter in W
// wCols: number of columns in each filter in W
// wChls: number of channels in each filter in W
// vStride: vertical stride
// yRows: number of rows in the output tensor Y
// yCols: number of columns in the output tensor Y
// space: pointer to temporary space of size at least (wRows * wCols * wChls * yRows * yCols)
template <typename ElementType>
void Convolution(ConvProperties<ChannelMajorInput, RowMajorFilters, RowMajorOutput, UnitHorizontalStride, UnrolledInput>,
    const ElementType* W, 
    const ElementType* X, 
    ElementType* Y, 
    int wCount, 
    int wRows, 
    int wCols, 
    int wChls, 
    int vStride, 
    int yRows, 
    int yCols,
    ElementType* space)
{
    // use temp space to store the unrolled input matrix U in column-major order
    int uRows = yRows * yCols;
    int uCols = wRows * wCols * wChls;
    ElementType* U = space;

    // unroll the channel-major input
    ChlMajInputUnroll(X, U, wRows, wCols, wChls, vStride, yRows, yCols, uRows, uCols);

    // reshape the filters tensor W into a row-major matrix V
    int vCols = wCount;
    const ElementType* V = W;
    
    // reshape the output tensor Y into a row-major matrix Z
    ElementType* Z = Y;

    // matrix-matrix multiply
    Gemm(ColMaj, RowMaj, RowMaj, uRows, vCols, uCols, 1, U, V, 0, Z);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// 2D Tensor Convolution
// * supports only horizontal stride of 1
// * unrolled input 
// * filters in filter-major order
// * input tensor in channel-major order
// * output tensor in channel-major order
// * requires temporary space of size (wRows * wCols * wChls * yRows * yCols)
//
// W: 4-dimensional weights tensor in filter-major order
// X: 3-dimensional input tensor in channel-major order
// Y: 3-dimensional output tensor in channel-major order
// wCount: number of filters in W
// wRows: number of rows in each filter in W
// wCols: number of columns in each filter in W
// wChls: number of channels in each filter in W
// vStride: vertical stride
// yRows: number of rows in the output tensor Y
// yCols: number of columns in the output tensor Y
// space: pointer to temporary space of size at least (wRows * wCols * wChls * yRows * yCols)
template <typename ElementType>
void Convolution(ConvProperties<ChannelMajorInput, FilterMajorFilters, ChannelMajorOutput, UnitHorizontalStride, UnrolledInput>,
    const ElementType* W, 
    const ElementType* X, 
    ElementType* Y, 
    int wCount, 
    int wRows, 
    int wCols, 
    int wChls, 
    int vStride, 
    int yRows, 
    int yCols,
    ElementType* space)
{
    // use temp space to store the unrolled input matrix U in column-major order
    int uRows = yRows * yCols;
    int uCols = wRows * wCols * wChls;
    ElementType* U = space;

    // unroll the channel-major input
    ChlMajInputUnroll(X, U, wRows, wCols, wChls, vStride, yRows, yCols, uRows, uCols);

    // reshape the filters tensor W into a column-major matrix V
    int vCols = wCount;
    const ElementType* V = W;
    
    // reshape the output tensor Y into a column-major matrix Z
    ElementType* Z = Y;

    // matrix-matrix multiply
    Gemm(ColMaj, ColMaj, ColMaj, uRows, vCols, uCols, 1, U, V, 0, Z);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// 2D Tensor Convolution
// * supports only horizontal stride of 1
// * unrolled input 
// * filters in row-major order
// * input tensor in channel-major order
// * output tensor in channel-major order
// * requires temporary space of size (wRows * wCols * wChls * yRows * yCols)
//
// W: 4-dimensional weights tensor in row-major order
// X: 3-dimensional input tensor in channel-major order
// Y: 3-dimensional output tensor in channel-major order
// wCount: number of filters in W
// wRows: number of rows in each filter in W
// wCols: number of columns in each filter in W
// wChls: number of channels in each filter in W
// vStride: vertical stride
// yRows: number of rows in the output tensor Y
// yCols: number of columns in the output tensor Y
// space: pointer to temporary space of size at least (wRows * wCols * wChls * yRows * yCols)
template <typename ElementType>
void Convolution(ConvProperties<ChannelMajorInput, RowMajorFilters, ChannelMajorOutput, UnitHorizontalStride, UnrolledInput>,
    const ElementType* W, 
    const ElementType* X, 
    ElementType* Y, 
    int wCount, 
    int wRows, 
    int wCols, 
    int wChls, 
    int vStride, 
    int yRows, 
    int yCols,
    ElementType* space)
{
    // use temp space to store the unrolled input matrix U in column-major order
    int uRows = yRows * yCols;
    int uCols = wRows * wCols * wChls;
    ElementType* U = space;

    // unroll the channel-major input
    ChlMajInputUnroll(X, U, wRows, wCols, wChls, vStride, yRows, yCols, uRows, uCols);

    // reshape the filters tensor W into a row-major matrix V
    int vCols = wCount;
    const ElementType* V = W;
    
    // reshape the output tensor Y into a column-major matrix Z
    ElementType* Z = Y;

    // matrix-matrix multiply
    Gemm(ColMaj, RowMaj, ColMaj, uRows, vCols, uCols, 1, U, V, 0, Z);
}

