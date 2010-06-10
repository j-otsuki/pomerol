#include "Vertex4Part.h"

inline bool chaseIndices(RowMajorMatrixType::InnerIterator& index1_iter, 
                         ColMajorMatrixType::InnerIterator& index2_iter)
{
    InnerQuantumState index1 = index1_iter.index();
    InnerQuantumState index2 = index2_iter.index();
  
    if(index1 == index2) return true;
    
    if(index1 < index2) 
        for(;InnerQuantumState(index1_iter.index())<index2 && index1_iter; ++index1_iter);
    else 
        for(;InnerQuantumState(index2_iter.index())<index1 && index2_iter; ++index2_iter);
        
    return false;
}

//
// Vertex4Part::Vertex4TermType1
//
inline
Vertex4Part::Vertex4TermType1::Vertex4TermType1(ComplexType Coeff, 
                                                RealType E2MinusE1, RealType E3MinusE2, RealType E4MinusE3,
                                                Permutation3& Permutation) : Coeff(Coeff)
{
    Poles[Permutation.perm[0]] = E2MinusE1;
    Poles[Permutation.perm[1]] = E3MinusE2;
    Poles[Permutation.perm[2]] = E4MinusE3;
}

inline 
ComplexType Vertex4Part::Vertex4TermType1::operator()
            (ComplexType Frequency1, ComplexType Frequency2, ComplexType Frequency3) const
{
    return Coeff/((Frequency1 - Poles[0])*(Frequency2 - Poles[1])*(-Frequency3 - Poles[2]));
}

//
// Vertex4Part::Vertex4TermType2
//
inline
Vertex4Part::Vertex4TermType2::Vertex4TermType2(ComplexType CoeffA, ComplexType CoeffB, ComplexType CoeffC, 
                                                RealType E4MinusE1, RealType E4MinusE2, 
                                                RealType E4MinusE3, RealType E2MinusE1,
                                                Permutation3& Permutation) :
CoeffA(CoeffA), CoeffB(CoeffB), CoeffC(CoeffC),
E4MinusE1(E4MinusE1), E4MinusE2(E4MinusE2), E4MinusE3(E4MinusE3), E2MinusE1(E2MinusE1)
{
    z1 = (Var)Permutation.perm[0];
    z2 = (Var)Permutation.perm[1];
    z3 = (Var)Permutation.perm[2];
                       
    IsE4MinusE2Vanishes = abs(E4MinusE2) < 1e-16;
}

inline
ComplexType Vertex4Part::Vertex4TermType2::operator()
            (long MatsubaraNumberOdd1, long MatsubaraNumberOdd2, long MatsubaraNumberOdd3,
             ComplexType Frequency1, ComplexType Frequency2, ComplexType Frequency3) const
{
    long MatsubaraNumbersOdd[3] = {MatsubaraNumberOdd1, MatsubaraNumberOdd2, -MatsubaraNumberOdd3};
    ComplexType Frequencies[3] = {Frequency1,Frequency2,-Frequency3};
    
    if(IsE4MinusE2Vanishes && MatsubaraNumbersOdd[z2] + MatsubaraNumbersOdd[z3] == 0)
        return (CoeffA/(Frequencies[z1] - E2MinusE1) + CoeffB)
                /((Frequencies[z3] - E4MinusE3)
                *(Frequencies[z1] - E2MinusE1));
    else
        return (CoeffA/(Frequencies[z1] - E2MinusE1) + 
                CoeffC/(Frequencies[z1]+Frequencies[z2]+Frequencies[z3] - E4MinusE1))
               /((Frequencies[z3] - E4MinusE3)
               *(Frequencies[z2]+Frequencies[z3] - E4MinusE2));
}

//
// Vertex4Part:;Vertex4TermType3
//
inline
Vertex4Part::Vertex4TermType3::Vertex4TermType3(ComplexType CoeffResonant, ComplexType CoeffNonResonant,
                                                RealType E3MinusE1, RealType E3MinusE2, RealType E4MinusE3,
                                                Permutation3& Permutation) :
CoeffResonant(CoeffResonant), CoeffNonResonant(CoeffNonResonant),
E3MinusE1(E3MinusE1), E3MinusE2(E3MinusE2), E4MinusE3(E4MinusE3)
{
    z1 = (Var)Permutation.perm[0];
    z2 = (Var)Permutation.perm[1];
    z3 = (Var)Permutation.perm[2];
                   
    IsE3MinusE1Vanishes = abs(E3MinusE1) < 1e-16;
}

inline
ComplexType Vertex4Part::Vertex4TermType3::operator()
            (long MatsubaraNumberOdd1, long MatsubaraNumberOdd2, long MatsubaraNumberOdd3,
             ComplexType Frequency1, ComplexType Frequency2, ComplexType Frequency3) const
{
    long MatsubaraNumbersOdd[3] = {MatsubaraNumberOdd1, MatsubaraNumberOdd2, -MatsubaraNumberOdd3};
    ComplexType Frequencies[3] = {Frequency1,Frequency2,-Frequency3};
    
    if(IsE3MinusE1Vanishes && MatsubaraNumbersOdd[z1] + MatsubaraNumbersOdd[z2] == 0)
        return CoeffResonant
              /((Frequencies[z3] - E4MinusE3)
              *(Frequencies[z2] - E3MinusE2));
    else
        return CoeffNonResonant
              /((Frequencies[z3] - E4MinusE3)
              *(Frequencies[z2] - E3MinusE2)
              *(Frequencies[z1]+Frequencies[z2] - E3MinusE1));
}

//
// Vertex4Part
//
Vertex4Part::Vertex4Part(
                FieldOperatorPart& O1, FieldOperatorPart& O2, FieldOperatorPart& O3, CreationOperatorPart& CX4,
                HamiltonianPart& Hpart1, HamiltonianPart& Hpart2, HamiltonianPart& Hpart3, HamiltonianPart& Hpart4,
                DensityMatrixPart& DMpart1, DensityMatrixPart& DMpart2, DensityMatrixPart& DMpart3, DensityMatrixPart& DMpart4,
                Permutation3 Permutation) :
O1(O1), O2(O2), O3(O3), CX4(CX4), 
Hpart1(Hpart1), Hpart2(Hpart2), Hpart3(Hpart3), Hpart4(Hpart4),
DMpart1(DMpart1), DMpart2(DMpart2), DMpart3(DMpart3), DMpart4(DMpart4),
Permutation(Permutation)
{}

void Vertex4Part::compute(Vertex4Part::ComputationMethod method)
{
    TermsType1.clear();
    TermsType2.clear();
    TermsType3.clear();
    
    switch(method){
        case ChasingIndices1: computeChasing1(); break;
        case ChasingIndices2: computeChasing2(); break;
        default: assert(0);
    }
};

void Vertex4Part::computeChasing1(void)
	// I don't have any pen now, so I'm writing here:
	// <1 | O1 | 2> <2 | O2 | 3> <3 | O3 |4> <4| CX4 |1>
{
    RowMajorMatrixType& O1matrix = O1.getRowMajorValue();
    RowMajorMatrixType& O2matrix = O2.getRowMajorValue();    
    ColMajorMatrixType& O3matrix = O3.getColMajorValue();
    ColMajorMatrixType& CX4matrix = CX4.getColMajorValue();
    
    InnerQuantumState index1;
    InnerQuantumState index1Max = CX4matrix.outerSize();
    
    for(index1=0; index1<index1Max; ++index1){
        ColMajorMatrixType::InnerIterator index4bra(CX4matrix,index1);       
        while(index4bra){
            InnerQuantumState index4ket = index4bra.index();
            RowMajorMatrixType::InnerIterator index2ket(O1matrix,index1);       
            while (index2ket){
                InnerQuantumState index2bra = index2ket.index();
                RowMajorMatrixType::InnerIterator index3ket_iter(O2matrix,index2bra);
            	ColMajorMatrixType::InnerIterator index3bra_iter(O3matrix,index4ket);
            	while(index3bra_iter && index3ket_iter){
                    if(chaseIndices(index3ket_iter,index3bra_iter)){
                        // TODO
                
                        ++index3ket_iter;
                        ++index3bra_iter;
                    }
                };
             	++index2ket;
            };
            ++index4bra;
        };
    };
};

void Vertex4Part::computeChasing2(void)
{
    RealType beta = DMpart1.getBeta();
       
	// I don't have any pen now, so I'm writing here:
	// <1 | O1 | 2> <2 | O2 | 3> <3 | O3 |4> <4| CX4 |1>
    RowMajorMatrixType& O1matrix = O1.getRowMajorValue();
    ColMajorMatrixType& O2matrix = O2.getColMajorValue();    
    RowMajorMatrixType& O3matrix = O3.getRowMajorValue();
    ColMajorMatrixType& CX4matrix = CX4.getColMajorValue();
      
    InnerQuantumState index1;
    InnerQuantumState index1Max = CX4matrix.outerSize();
    
    InnerQuantumState index3;
    InnerQuantumState index3Max = O2matrix.outerSize();
    
    std::list<InnerQuantumState> Index4List;

    for(index1=0; index1<index1Max; ++index1){
    for(index3=0; index3<index3Max; ++index3){
        ColMajorMatrixType::InnerIterator index4bra_iter(CX4matrix,index1);       
        RowMajorMatrixType::InnerIterator index4ket_iter(O3matrix,index3);
        Index4List.clear();
		while (index4bra_iter && index4ket_iter){
            if(chaseIndices(index4ket_iter,index4bra_iter)){
				Index4List.push_back(index4bra_iter.index());
				++index4bra_iter;
				++index4ket_iter;
			}
 		};

		if (!Index4List.empty())
		{
            // !!! //
            RealType E1 = Hpart1.reV(index1);
            RealType E3 = Hpart3.reV(index3);
            RealType weight1 = DMpart1.weight(index1);
            RealType weight3 = DMpart3.weight(index3);
                        
            ColMajorMatrixType::InnerIterator index2bra_iter(O2matrix,index3);
            RowMajorMatrixType::InnerIterator index2ket_iter(O1matrix,index1);       
            while (index2bra_iter && index2ket_iter){
                if (chaseIndices(index2ket_iter,index2bra_iter)){
                  
                    // !!! //
                    InnerQuantumState index2 = index2ket_iter.index();
                    RealType E2 = Hpart2.reV(index2);
                    RealType weight2 = DMpart2.weight(index2);
                                        
                    for (std::list<InnerQuantumState>::iterator pIndex4 = Index4List.begin(); pIndex4!=Index4List.end(); ++pIndex4) 
					{
                        // !!! //
                        InnerQuantumState index4 = *pIndex4;
                        RealType E4 = Hpart4.reV(index4);                       
                        RealType weight4 = DMpart4.weight(index4);
                         
                        // !!!!! //
                        ComplexType MatrixElement = index2ket_iter.value()*
                                                    index2bra_iter.value()*
                                                    O3matrix.coeff(index3,index4)*
                                                    CX4matrix.coeff(index4,index1);
                        
                        if (abs(MatrixElement) <= MatrixElementTolerance) continue;
                          
                        MatrixElement *= Permutation.sign;
                         
                        RealType E2MinusE1 = E2 - E1;
                        RealType E3MinusE1 = E3 - E1;
                        RealType E3MinusE2 = E3 - E2;
                        RealType E4MinusE1 = E4 - E1;
                        RealType E4MinusE2 = E4 - E2;
                        RealType E4MinusE3 = E4 - E3;
                        
                        TermsType1.push_back(Vertex4TermType1(-MatrixElement,E2MinusE1,E3MinusE2,E4MinusE3,Permutation));
                        TermsType2.push_back(Vertex4TermType2(MatrixElement*(weight1 + weight2),
                                                              MatrixElement*(-beta*weight2),
                                                              -MatrixElement*(weight1 + weight4),
                                                              E4MinusE1,E4MinusE2,E4MinusE3,E2MinusE1,Permutation));
                        TermsType3.push_back(Vertex4TermType3(MatrixElement*(-beta*weight1),
                                                              MatrixElement*(weight1 - weight3),
                                                              E3MinusE1,E3MinusE2,E4MinusE3,Permutation));                                       
                    }
                    ++index2bra_iter;
                    ++index2ket_iter;
                };
            }
		};
    }};
}

ComplexType Vertex4Part::operator()(long MatsubaraNumber1, long MatsubaraNumber2, long MatsubaraNumber3)
{
    ComplexType MatsubaraSpacing = I*M_PI/DMpart1.getBeta();
    long MatsubaraNumberOdd1 = 2*MatsubaraNumber1 + 1;
    long MatsubaraNumberOdd2 = 2*MatsubaraNumber2 + 1;
    long MatsubaraNumberOdd3 = 2*MatsubaraNumber3 + 1;
    ComplexType Frequency1 = MatsubaraSpacing * RealType(MatsubaraNumberOdd1);
    ComplexType Frequency2 = MatsubaraSpacing * RealType(MatsubaraNumberOdd2);
    ComplexType Frequency3 = MatsubaraSpacing * RealType(MatsubaraNumberOdd3);
    
    ComplexType Value = 0;
    for(std::list<Vertex4TermType1>::const_iterator pTerm = TermsType1.begin(); pTerm != TermsType1.end(); ++pTerm)
        Value += (*pTerm)(Frequency1,Frequency2,Frequency3);
    for(std::list<Vertex4TermType2>::const_iterator pTerm = TermsType2.begin(); pTerm != TermsType2.end(); ++pTerm)
        Value += (*pTerm)(MatsubaraNumberOdd1,MatsubaraNumberOdd2,MatsubaraNumberOdd3,Frequency1,Frequency2,Frequency3);
    for(std::list<Vertex4TermType3>::const_iterator pTerm = TermsType3.begin(); pTerm != TermsType3.end(); ++pTerm)
        Value += (*pTerm)(MatsubaraNumberOdd1,MatsubaraNumberOdd2,MatsubaraNumberOdd3,Frequency1,Frequency2,Frequency3);
    
    return Value;
}
