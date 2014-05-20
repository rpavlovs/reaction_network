#include "BvpPde.hpp"
using namespace Eigen;
using namespace std;

BvpPde::BvpPde(SecondOrderOde *pOde, BoundaryConditions *pBcs, double dt, double tau, double timer, int numNodes, double (*uj0)(double)) : BvpOde(pOde, pBcs, numNodes){
    mtau = tau;
    mdt = dt;
    mUj0Func = uj0;
    T = timer;
}

void BvpPde::SolvePde(){
    PopulateOperators();
    PopulateInitCondiotions();
    SolvePdeInTime();
}

void BvpPde::SolvePdeInTime(){
    int columns = T/mdt;
    solutionInTime.resize(BvpOde::mNumNodes, columns+1);
    solutionInTime.col(0) = mb;
    // Applying BC
    mb(0) = (*BvpOde::mpRhsVec)(0);
    mb(mNumNodes-1) = (*BvpOde::mpRhsVec)(mNumNodes-1);
    cout << mb;
    for (int i = 1; i <2; ++i) {
        VectorXd temp = mUj0Operator*mb;
        cout << temp;
        temp(0) = (*BvpOde::mpRhsVec)(0);
        temp(mNumNodes-1) = (*BvpOde::mpRhsVec)(mNumNodes-1);
        // cout << mb;
        mpLinearSolver = new LinearSolver(mUj1Operator, temp);
        solution = mpLinearSolver->SolveLinearSystem();
        mb = solution;
        solutionInTime.col(i) = mb;
        // cout << mb;
        mb(0) = (*BvpOde::mpRhsVec)(0);
        mb(mNumNodes-1) = (*BvpOde::mpRhsVec)(mNumNodes-1);
    }
    // cout << solutionInTime;
}

void BvpPde::PopulateOperators(){
    int x = BvpOde::mNumNodes;
    SparseMatrix<double> I = MatrixXd::Identity(x, x).sparseView();
    SparseMatrix<double> F = *BvpOde::mpLhsMat;
    mUj0Operator = (I +     mtau*mdt*I*F);
    mUj1Operator = (I - (1-mtau)*mdt*I*F);
}

void BvpPde::PopulateInitCondiotions(){
    int numNodes = BvpOde::mNumNodes;
    mb.resize(mNumNodes);
    for (int i = 0; i < numNodes; ++i){
        double x = BvpOde::mpGrid->mNodes[i].C.x;
        if (mUj0Func!=0) {
            mb(i) = mUj0Func(x);
        } else {
            mb(i) = 0;
        }
    }
}


