//
// This file is a part of pomerol - a scientific ED code for obtaining 
// properties of a Hubbard model on a finite-size lattice 
//
// Copyright (C) 2010-2011 Andrey Antipov <antipov@ct-qmc.org>
// Copyright (C) 2010-2011 Igor Krivenko <igor@shg.ru>
//
// pomerol is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// pomerol is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with pomerol.  If not, see <http://www.gnu.org/licenses/>.


/** \file src/GreensFunctionPart.cpp
** \brief Part of a Green's function.
**
** \author Igor Krivenko (igor@shg.ru)
** \author Andrey Antipov (antipov@ct-qmc.org)
*/
#include "GreensFunctionPart.h"

GreensFunctionPart::Term::Term(ComplexType Residue, RealType Pole) : Residue(Residue), Pole(Pole) {};
ComplexType GreensFunctionPart::Term::operator()(ComplexType Frequency) const { return Residue/(Frequency - Pole); }

inline
GreensFunctionPart::Term& GreensFunctionPart::Term::operator+=(const Term& AnotherTerm)
{
    Residue += AnotherTerm.Residue;
    return *this;
}

inline
bool GreensFunctionPart::Term::isSimilarTo(const Term& AnotherTerm) const
{
    return (fabs(Pole - AnotherTerm.Pole) < ReduceResonanceTolerance);
}

std::ostream& operator<<(std::ostream& out, const GreensFunctionPart::Term& T)
{
    out << T.Residue << "/(z - " << T.Pole << ")";
    return out;
}

GreensFunctionPart::GreensFunctionPart( AnnihilationOperatorPart& C, CreationOperatorPart& CX, 
                                        HamiltonianPart& HpartInner, HamiltonianPart& HpartOuter,
                                        DensityMatrixPart& DMpartInner, DensityMatrixPart& DMpartOuter) :
                                        ComputableObject(),
                                        Thermal(DMpartInner),
                                        HpartInner(HpartInner), HpartOuter(HpartOuter),
                                        DMpartInner(DMpartInner), DMpartOuter(DMpartOuter),
                                        C(C), CX(CX)
{}

void GreensFunctionPart::compute(void)
{
    Terms.clear();

    // Blocks (submatrices) of C and CX
    RowMajorMatrixType& Cmatrix = C.getRowMajorValue();
    ColMajorMatrixType& CXmatrix = CX.getColMajorValue();
    QuantumState outerSize = Cmatrix.outerSize();

    // Iterate over all values of the outer index.
    // TODO: should be optimized - skip empty rows of Cmatrix and empty columns of CXmatrix.
    for(QuantumState index1=0; index1<outerSize; ++index1){
        // <index1|C|Cinner><CXinner|CX|index1>
        RowMajorMatrixType::InnerIterator Cinner(Cmatrix,index1);
        ColMajorMatrixType::InnerIterator CXinner(CXmatrix,index1);

        // While we are not at the last column of Cmatrix or at the last row of CXmatrix.
        while(Cinner && CXinner){
            QuantumState C_index2 = Cinner.index();
            QuantumState CX_index2 = CXinner.index();

            // A meaningful matrix element
            if(C_index2 == CX_index2){
                ComplexType Residue = Cinner.value() * CXinner.value() * 
                                      (DMpartOuter.weight(index1) + DMpartInner.weight(C_index2));
                if(abs(Residue) > MatrixElementTolerance) // Is the residue relevant?
                {
                    // Create a new term and append it to the list.
                    RealType Pole = HpartInner.reV(C_index2) - HpartOuter.reV(index1);
                    Terms.push_back(Term(Residue,Pole));
                };
                ++Cinner;   // The next non-zero element
                ++CXinner;  // The next non-zero element
            }else{
                // Chasing: one index runs down the other index
                if(CX_index2 < C_index2) for(;QuantumState(CXinner.index())<C_index2; ++CXinner);
                else for(;QuantumState(Cinner.index())<CX_index2; ++Cinner);
            }
        }
    }

    reduceTerms(ReduceTolerance/Terms.size(),Terms);
}

ComplexType GreensFunctionPart::operator()(long MatsubaraNumber) const
{
    // TODO: Place this variable to a wider scope?
    ComplexType MatsubaraSpacing = I*M_PI/beta;

    ComplexType G = 0;
    for(std::list<Term>::const_iterator pTerm = Terms.begin(); pTerm != Terms.end(); ++pTerm)
        G += (*pTerm)(MatsubaraSpacing*RealType(2*MatsubaraNumber+1));

    return G;
}

void GreensFunctionPart::reduceTerms(const RealType Tolerance, std::list<Term> &Terms)
{
    // Sieve reduction of the terms
    for(std::list<Term>::iterator it1 = Terms.begin(); it1 != Terms.end();){
        std::list<Term>::iterator it2 = it1;
        for(it2++; it2 != Terms.end();){
            if(it1->isSimilarTo(*it2)){
                *it1 += *it2;
                it2 = Terms.erase(it2);
            }else
                it2++;
        }

        if(abs(it1->Residue) < Tolerance)
            it1 = Terms.erase(it1);
        else
            it1++;
    }
}
