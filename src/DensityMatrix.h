#ifndef ____DEFINE_DENSITY_MATRIX____
#define ____DEFINE_DENSITY_MATRIX____

#include "StatesClassification.h"
#include "BitClassification.h"
#include "Hamiltonian.h"
#include "DensityMatrixPart.h"

class DensityMatrix
{
    StatesClassification& S;
    
    BlockNumber NumOfBlocks;
    DensityMatrixPart** parts;

public:
    DensityMatrix(StatesClassification& S, Hamiltonian& H, RealType beta);
    ~DensityMatrix();
    
    DensityMatrixPart& block(const QuantumNumbers &in);
    DensityMatrixPart& block(BlockNumber in);  
    
    RealType operator()( QuantumState &state );
};

#endif // endif :: #ifndef ____DEFINE_DENSITY_MATRIX____